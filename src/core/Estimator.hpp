#pragma once
#include <Eigen/Dense>

class Estimator {
public:
    virtual ~Estimator() = default;
    virtual void predict() = 0;
    virtual void update(const Eigen::VectorXd& z) = 0;
    virtual Eigen::VectorXd state() const = 0;
    virtual Eigen::MatrixXd covariance() const = 0;
};
