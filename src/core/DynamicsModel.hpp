#pragma once
#include <Eigen/Dense>

// Abstract base class for system dynamics and measurement models
class DynamicsModel {
public:
    virtual ~DynamicsModel() = default;

    // Propagate state: x_k+1 = f(x_k, u_k)
    virtual Eigen::VectorXd propagate(const Eigen::VectorXd& x, const Eigen::VectorXd& u) const = 0;

    // Measurement: z_k = h(x_k)
    virtual Eigen::VectorXd measure(const Eigen::VectorXd& x) const = 0;

    // Jacobian of f with respect to x at (x, u)
    virtual Eigen::MatrixXd getFJacobian(const Eigen::VectorXd& x, const Eigen::VectorXd& u) const = 0;

    // Jacobian of h with respect to x at x
    virtual Eigen::MatrixXd getHJacobian(const Eigen::VectorXd& x) const = 0;
};
