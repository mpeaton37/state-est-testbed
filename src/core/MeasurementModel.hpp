#pragma once
#include <Eigen/Dense>

/**
 * @brief Abstract base class for measurement models.
 * Supports dynamic measurement covariance R and nonlinear h(x).
 */
class MeasurementModel {
public:
    virtual ~MeasurementModel() = default;

    // Measurement function: z = h(x)
    virtual Eigen::VectorXd measure(const Eigen::VectorXd& x) const = 0;

    // Jacobian of h(x) with respect to x
    virtual Eigen::MatrixXd getHJacobian(const Eigen::VectorXd& x) const = 0;

    // Get current measurement noise covariance R (may be dynamic)
    virtual Eigen::MatrixXd getR() const = 0;

    // Set measurement noise covariance R (for dynamic/adaptive R)
    virtual void setR(const Eigen::MatrixXd& newR) = 0;

    // Get measurement dimension
    virtual int getMeasurementDimension() const = 0;
};
