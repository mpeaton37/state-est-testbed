#include "MultiHypothesisTracker.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>

MultiHypothesisTracker::MultiHypothesisTracker(std::unique_ptr<Estimator> base_filter,
                                               const Params& params)
    : base_filter_prototype_(std::move(base_filter)), params_(params), current_step_(0)
{
    if (!base_filter_prototype_) {
        throw std::invalid_argument("Base filter prototype must not be null");
    }
}

void MultiHypothesisTracker::init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) {
    hypothesis_tree_.clear();
    current_step_ = 0;
    // Root hypothesis
    MHTHypothesis root;
    root.state = x0;
    root.covariance = P0;
    root.log_prob = 0.0;
    root.parent_idx = -1;
    root.step = 0;
    hypothesis_tree_.push_back({root});
}

void MultiHypothesisTracker::predict(const Eigen::VectorXd& u) {
    if (hypothesis_tree_.empty()) return;
    std::vector<MHTHypothesis> &prev_hyps = hypothesis_tree_.back();
    std::vector<MHTHypothesis> predicted_hyps;
    for (size_t i = 0; i < prev_hyps.size(); ++i) {
        // Clone base filter and set state/cov
        auto filter = cloneFilter(prev_hyps[i].state, prev_hyps[i].covariance);
        filter->predict(u);
        MHTHypothesis hyp;
        hyp.state = filter->state();
        hyp.covariance = filter->covariance();
        hyp.log_prob = prev_hyps[i].log_prob;
        hyp.parent_idx = static_cast<int>(i);
        hyp.step = current_step_ + 1;
        predicted_hyps.push_back(hyp);
    }
    hypothesis_tree_.push_back(predicted_hyps);
    ++current_step_;
}

void MultiHypothesisTracker::update(const Eigen::VectorXd& z) {
    if (hypothesis_tree_.empty()) return;
    std::vector<MHTHypothesis> &predicted_hyps = hypothesis_tree_.back();
    std::vector<MHTHypothesis> updated_hyps;

    for (size_t i = 0; i < predicted_hyps.size(); ++i) {
        // Gating: check Mahalanobis distance
        auto filter = cloneFilter(predicted_hyps[i].state, predicted_hyps[i].covariance);
        // Predict measurement and innovation covariance
        Eigen::VectorXd z_pred = filter->state(); // Placeholder: should use measurement model
        Eigen::MatrixXd S = filter->covariance(); // Placeholder: should use innovation covariance

        Eigen::VectorXd innov = z - z_pred;
        double md2 = mahalanobis(innov, S);
        if (md2 <= params_.gating_threshold) {
            // Accept this hypothesis
            filter->update(z);
            MHTHypothesis hyp;
            hyp.state = filter->state();
            hyp.covariance = filter->covariance();
            // Update log_prob with likelihood (Gaussian)
            double detS = S.determinant();
            if (detS <= 0) detS = 1e-12;
            double norm_const = 1.0 / std::sqrt(std::pow(2 * M_PI, innov.size()) * detS);
            double exponent = -0.5 * innov.transpose() * S.inverse() * innov;
            hyp.log_prob = predicted_hyps[i].log_prob + std::log(norm_const) + exponent;
            hyp.parent_idx = static_cast<int>(i);
            hyp.step = current_step_;
            updated_hyps.push_back(hyp);
        }
        // else: prune this branch
    }

    if (updated_hyps.empty()) {
        // If all pruned, keep the highest-prob predicted hypothesis (no update)
        auto max_it = std::max_element(predicted_hyps.begin(), predicted_hyps.end(),
            [](const MHTHypothesis& a, const MHTHypothesis& b) { return a.log_prob < b.log_prob; });
        if (max_it != predicted_hyps.end()) {
            updated_hyps.push_back(*max_it);
        }
    }

    hypothesis_tree_.back() = updated_hyps;
    prune();
}

Eigen::VectorXd MultiHypothesisTracker::state() const {
    // Return MAP hypothesis at current step
    if (hypothesis_tree_.empty() || hypothesis_tree_.back().empty())
        return Eigen::VectorXd();
    auto max_it = std::max_element(hypothesis_tree_.back().begin(), hypothesis_tree_.back().end(),
        [](const MHTHypothesis& a, const MHTHypothesis& b) { return a.log_prob < b.log_prob; });
    return max_it->state;
}

Eigen::MatrixXd MultiHypothesisTracker::covariance() const {
    // Return MAP hypothesis covariance
    if (hypothesis_tree_.empty() || hypothesis_tree_.back().empty())
        return Eigen::MatrixXd();
    auto max_it = std::max_element(hypothesis_tree_.back().begin(), hypothesis_tree_.back().end(),
        [](const MHTHypothesis& a, const MHTHypothesis& b) { return a.log_prob < b.log_prob; });
    return max_it->covariance;
}

std::vector<MHTHypothesis> MultiHypothesisTracker::getHypotheses() const {
    if (hypothesis_tree_.empty()) return {};
    return hypothesis_tree_.back();
}

void MultiHypothesisTracker::prune() {
    // Keep only top-N hypotheses by log_prob
    if (hypothesis_tree_.empty()) return;
    auto& hyps = hypothesis_tree_.back();
    if (hyps.size() > params_.max_hypotheses) {
        std::nth_element(hyps.begin(), hyps.begin() + params_.max_hypotheses, hyps.end(),
            [](const MHTHypothesis& a, const MHTHypothesis& b) { return a.log_prob > b.log_prob; });
        hyps.resize(params_.max_hypotheses);
    }
}

double MultiHypothesisTracker::mahalanobis(const Eigen::VectorXd& innov, const Eigen::MatrixXd& S) {
    // Mahalanobis distance squared
    return innov.transpose() * S.inverse() * innov;
}

std::unique_ptr<Estimator> MultiHypothesisTracker::cloneFilter(const Eigen::VectorXd& x, const Eigen::MatrixXd& P) const {
    // Assumes base_filter_prototype_ has a copy constructor or clone method
    // For now, just use the prototype and re-init
    auto filter = base_filter_prototype_->clone();
    filter->init(x, P);
    return filter;
}
