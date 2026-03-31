#pragma once
#include <Eigen/Dense>
#include <vector>
#include <memory>
#include "Estimator.hpp"

// Represents a single hypothesis in the MHT tree
struct MHTHypothesis {
    Eigen::VectorXd state;
    Eigen::MatrixXd covariance;
    double log_prob;
    int parent_idx; // Index of parent hypothesis in previous step (-1 for root)
    int step;       // Time step of this hypothesis
};

class MultiHypothesisTracker : public Estimator {
public:
    struct Params {
        double gating_threshold; // Mahalanobis distance squared
        size_t max_hypotheses;   // Pruning: max number of hypotheses to keep
    };

    MultiHypothesisTracker(std::unique_ptr<Estimator> base_filter,
                          const Params& params);

    void init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0);

    void predict(const Eigen::VectorXd& u = Eigen::VectorXd()) override;
    void update(const Eigen::VectorXd& z) override;

    Eigen::VectorXd state() const override;      // MAP hypothesis state
    Eigen::MatrixXd covariance() const override; // MAP hypothesis covariance

    // Access all active hypotheses at current step
    std::vector<MHTHypothesis> getHypotheses() const;

private:
    std::unique_ptr<Estimator> base_filter_prototype_; // Used to clone new filters
    Params params_;

    // Hypothesis tree: each step holds a vector of hypotheses
    std::vector<std::vector<MHTHypothesis>> hypothesis_tree_;
    int current_step_;

    // Helper: prune to top-N by log_prob
    void prune();

    // Helper: compute Mahalanobis distance squared
    static double mahalanobis(const Eigen::VectorXd& innov, const Eigen::MatrixXd& S);

    // Helper: clone base filter (assumes Estimator has copy constructor or clone method)
    std::unique_ptr<Estimator> cloneFilter(const Eigen::VectorXd& x, const Eigen::MatrixXd& P) const;
};
