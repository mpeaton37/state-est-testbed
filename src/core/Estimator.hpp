#pragma once
#include <Eigen/Dense>

class Estimator {
public:
    virtual ~Estimator() = default;
    virtual void init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) = 0;
    virtual void predict(const Eigen::VectorXd& u = {}) = 0;
    virtual void update(const Eigen::VectorXd& z) = 0;
    virtual Eigen::VectorXd state() const = 0;
    virtual Eigen::MatrixXd covariance() const = 0;
};
