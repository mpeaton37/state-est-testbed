#pragma once
#include <Eigen/Dense>
#include "Estimator.hpp"

class KalmanFilter : public Estimator {
public:
    KalmanFilter(const Eigen::MatrixXd& F,
                 const Eigen::MatrixXd& Q,
                 const Eigen::MatrixXd& H,
                 const Eigen::MatrixXd& R,
                 const Eigen::MatrixXd& B);

    void init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0);

    void predict(const Eigen::VectorXd& u = {}) override;
    void update(const Eigen::VectorXd& z) override;

    Eigen::VectorXd state() const override;
    Eigen::MatrixXd covariance() const override;

private:
    Eigen::MatrixXd F_, Q_, H_, R_, B_;
    Eigen::VectorXd x_;
    Eigen::MatrixXd P_;
};
