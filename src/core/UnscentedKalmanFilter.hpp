#pragma once
#include <Eigen/Dense>
#include <memory>
#include "Estimator.hpp"
#include "DynamicsModel.hpp"

// Unscented Kalman Filter implementation
class UnscentedKalmanFilter : public Estimator {
public:
    // Constructor: takes system matrices for compatibility, and a DynamicsModel pointer (not owned)
    UnscentedKalmanFilter(const Eigen::MatrixXd& Q,
                          const Eigen::MatrixXd& R,
                          const DynamicsModel* model,
                          double alpha = 1e-3,
                          double beta = 2.0,
                          double kappa = 0.0);

    void init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0);

    void predict(const Eigen::VectorXd& u = Eigen::VectorXd()) override;
    void update(const Eigen::VectorXd& z) override;

    Eigen::VectorXd state() const override;
    Eigen::MatrixXd covariance() const override;

    std::pair<Eigen::VectorXd, Eigen::MatrixXd> get_state_and_covariance() const;

private:
    // UKF parameters
    double alpha_;
    double beta_;
    double kappa_;
    int n_; // state dimension
    int L_; // 2n+1

    Eigen::VectorXd x_;
    Eigen::MatrixXd P_;

    Eigen::MatrixXd Q_;
    Eigen::MatrixXd R_;

    const DynamicsModel* model_; // Not owned

    // Precomputed weights
    Eigen::VectorXd wm_;
    Eigen::VectorXd wc_;

    // Helper functions
    void computeSigmaPoints(const Eigen::VectorXd& x, const Eigen::MatrixXd& P, Eigen::MatrixXd& Xsig) const;
    void computeWeights();
};
