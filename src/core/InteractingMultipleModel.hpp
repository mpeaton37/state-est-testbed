#pragma once
#include <Eigen/Dense>
#include <vector>
#include <memory>
#include "Estimator.hpp"

// Interacting Multiple Model (IMM) estimator
class InteractingMultipleModel : public Estimator {
public:
    // Each sub-estimator is owned by IMM
    InteractingMultipleModel(std::vector<std::unique_ptr<Estimator>>&& models,
                            const Eigen::MatrixXd& transition_matrix,
                            const std::vector<double>& initial_probs);

    void init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0);

    void predict(const Eigen::VectorXd& u = Eigen::VectorXd()) override;
    void update(const Eigen::VectorXd& z) override;

    Eigen::VectorXd state() const override;
    Eigen::MatrixXd covariance() const override;

    // Returns the current model probabilities (length = num_models)
    std::vector<double> getModelProbabilities() const;

    // Returns the state/covariance of each hypothesis (for logging/analysis)
    std::vector<std::pair<Eigen::VectorXd, Eigen::MatrixXd>> getModelStates() const;

private:
    size_t num_models_;
    std::vector<std::unique_ptr<Estimator>> models_;
    Eigen::MatrixXd transition_matrix_; // Markov transition matrix (num_models x num_models)
    std::vector<double> model_probs_;   // Current model probabilities

    // Mixing step helpers
    std::vector<Eigen::VectorXd> mixed_states_;
    std::vector<Eigen::MatrixXd> mixed_covs_;

    // Helper: normalize probabilities
    void normalizeProbs(std::vector<double>& probs) const;
};
