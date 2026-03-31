#include "InteractingMultipleModel.hpp"
#include <stdexcept>
#include <numeric>
#include <cmath>

InteractingMultipleModel::InteractingMultipleModel(std::vector<std::unique_ptr<Estimator>>&& models,
                                                  const Eigen::MatrixXd& transition_matrix,
                                                  const std::vector<double>& initial_probs)
    : num_models_(models.size()),
      models_(std::move(models)),
      transition_matrix_(transition_matrix),
      model_probs_(initial_probs)
{
    if (num_models_ < 2) {
        throw std::invalid_argument("IMM requires at least two models");
    }
    if (transition_matrix_.rows() != static_cast<int>(num_models_) || transition_matrix_.cols() != static_cast<int>(num_models_)) {
        throw std::invalid_argument("Transition matrix size mismatch");
    }
    if (model_probs_.size() != num_models_) {
        throw std::invalid_argument("Initial model probabilities size mismatch");
    }
    double sum = std::accumulate(model_probs_.begin(), model_probs_.end(), 0.0);
    if (std::abs(sum - 1.0) > 1e-6) {
        throw std::invalid_argument("Initial model probabilities must sum to 1");
    }
}

void InteractingMultipleModel::init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) {
    for (size_t i = 0; i < num_models_; ++i) {
        models_[i]->init(x0, P0);
    }
}

void InteractingMultipleModel::predict(const Eigen::VectorXd& u) {
    // 1. Mixing probabilities
    Eigen::MatrixXd mixing_probs(num_models_, num_models_);
    std::vector<double> c_j(num_models_, 0.0);

    for (size_t j = 0; j < num_models_; ++j) {
        for (size_t i = 0; i < num_models_; ++i) {
            mixing_probs(i, j) = transition_matrix_(i, j) * model_probs_[i];
            c_j[j] += mixing_probs(i, j);
        }
    }
    // Normalize columns
    for (size_t j = 0; j < num_models_; ++j) {
        if (c_j[j] > 0) {
            for (size_t i = 0; i < num_models_; ++i) {
                mixing_probs(i, j) /= c_j[j];
            }
        }
    }

    // 2. Mixing state and covariance
    mixed_states_.resize(num_models_);
    mixed_covs_.resize(num_models_);
    for (size_t j = 0; j < num_models_; ++j) {
        // Weighted mean
        Eigen::VectorXd x_mix = Eigen::VectorXd::Zero(models_[j]->state().size());
        for (size_t i = 0; i < num_models_; ++i) {
            x_mix += mixing_probs(i, j) * models_[i]->state();
        }
        mixed_states_[j] = x_mix;

        // Weighted covariance
        Eigen::MatrixXd P_mix = Eigen::MatrixXd::Zero(models_[j]->covariance().rows(), models_[j]->covariance().cols());
        for (size_t i = 0; i < num_models_; ++i) {
            Eigen::VectorXd dx = models_[i]->state() - x_mix;
            P_mix += mixing_probs(i, j) * (models_[i]->covariance() + dx * dx.transpose());
        }
        mixed_covs_[j] = P_mix;
    }

    // 3. Re-initialize each model with mixed state/cov
    for (size_t j = 0; j < num_models_; ++j) {
        models_[j]->init(mixed_states_[j], mixed_covs_[j]);
    }

    // 4. Predict step for each model
    for (size_t j = 0; j < num_models_; ++j) {
        models_[j]->predict(u);
    }
}

void InteractingMultipleModel::update(const Eigen::VectorXd& z) {
    // 1. Compute likelihood for each model
    std::vector<double> likelihoods(num_models_, 0.0);
    for (size_t j = 0; j < num_models_; ++j) {
        // Innovation: y = z - h(x)
        Eigen::VectorXd y = z - models_[j]->state(); // This is a placeholder; ideally, use measurement prediction
        Eigen::MatrixXd S = models_[j]->covariance(); // Placeholder; ideally, use innovation covariance

        // For now, assume Gaussian: p(z|model) ~ N(z; h(x), S)
        double detS = S.determinant();
        if (detS <= 0) detS = 1e-12;
        double norm_const = 1.0 / std::sqrt(std::pow(2 * M_PI, y.size()) * detS);
        double exponent = -0.5 * y.transpose() * S.inverse() * y;
        likelihoods[j] = norm_const * std::exp(exponent);
    }

    // 2. Update model probabilities
    std::vector<double> new_probs(num_models_, 0.0);
    double denom = 0.0;
    for (size_t j = 0; j < num_models_; ++j) {
        new_probs[j] = likelihoods[j] * model_probs_[j];
        denom += new_probs[j];
    }
    if (denom > 0) {
        for (size_t j = 0; j < num_models_; ++j) {
            new_probs[j] /= denom;
        }
    } else {
        // fallback: uniform
        for (size_t j = 0; j < num_models_; ++j) {
            new_probs[j] = 1.0 / num_models_;
        }
    }
    model_probs_ = new_probs;

    // 3. Update step for each model
    for (size_t j = 0; j < num_models_; ++j) {
        models_[j]->update(z);
    }
}

Eigen::VectorXd InteractingMultipleModel::state() const {
    // Weighted mean of model states
    Eigen::VectorXd x = Eigen::VectorXd::Zero(models_[0]->state().size());
    for (size_t j = 0; j < num_models_; ++j) {
        x += model_probs_[j] * models_[j]->state();
    }
    return x;
}

Eigen::MatrixXd InteractingMultipleModel::covariance() const {
    // Weighted sum of covariances + spread of means
    Eigen::VectorXd x = state();
    Eigen::MatrixXd P = Eigen::MatrixXd::Zero(models_[0]->covariance().rows(), models_[0]->covariance().cols());
    for (size_t j = 0; j < num_models_; ++j) {
        Eigen::VectorXd dx = models_[j]->state() - x;
        P += model_probs_[j] * (models_[j]->covariance() + dx * dx.transpose());
    }
    return P;
}

std::vector<double> InteractingMultipleModel::getModelProbabilities() const {
    return model_probs_;
}

std::vector<std::pair<Eigen::VectorXd, Eigen::MatrixXd>> InteractingMultipleModel::getModelStates() const {
    std::vector<std::pair<Eigen::VectorXd, Eigen::MatrixXd>> out;
    for (size_t j = 0; j < num_models_; ++j) {
        out.emplace_back(models_[j]->state(), models_[j]->covariance());
    }
    return out;
}

void InteractingMultipleModel::normalizeProbs(std::vector<double>& probs) const {
    double sum = std::accumulate(probs.begin(), probs.end(), 0.0);
    if (sum > 0) {
        for (auto& p : probs) p /= sum;
    }
}
