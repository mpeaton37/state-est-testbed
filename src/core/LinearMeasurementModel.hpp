#pragma once
#include "MeasurementModel.hpp"
#include <Eigen/Dense>

/**
 * @brief Linear measurement model: z = Hx + v, v ~ N(0, R)
 */
class LinearMeasurementModel : public MeasurementModel {
public:
    LinearMeasurementModel(const Eigen::MatrixXd& H, const Eigen::MatrixXd& R);

    Eigen::VectorXd measure(const Eigen::VectorXd& x) const override;
    Eigen::MatrixXd getHJacobian(const Eigen::VectorXd& x) const override;
    Eigen::MatrixXd getR() const override;
    void setR(const Eigen::MatrixXd& newR) override;
    int getMeasurementDimension() const override;

    // Access to H for advanced use
    const Eigen::MatrixXd& getH() const { return H_; }

private:
    Eigen::MatrixXd H_;
    Eigen::MatrixXd R_;
};
