#pragma once
#include <Eigen/Dense>
#include "DynamicsModel.hpp"

class LinearGaussianModel : public DynamicsModel {
public:
    LinearGaussianModel(const Eigen::MatrixXd& F,
                       const Eigen::MatrixXd& H,
                       const Eigen::MatrixXd& B);

    Eigen::VectorXd propagate(const Eigen::VectorXd& x, const Eigen::VectorXd& u) const override;
    Eigen::VectorXd measure(const Eigen::VectorXd& x) const override;
    Eigen::MatrixXd getFJacobian(const Eigen::VectorXd& x, const Eigen::VectorXd& u) const override;
    Eigen::MatrixXd getHJacobian(const Eigen::VectorXd& x) const override;

private:
    Eigen::MatrixXd F_, H_, B_;
};
