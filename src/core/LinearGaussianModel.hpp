#pragma once
#include <Eigen/Dense>

class LinearGaussianModel {
public:
LinearGaussianModel(const Eigen::MatrixXd& F,
const Eigen::MatrixXd& H,
const Eigen::MatrixXd& B);

Eigen::VectorXd propagate(const Eigen::VectorXd& x, const Eigen::VectorXd& u);
Eigen::VectorXd measure(const Eigen::VectorXd& x);

private:
Eigen::MatrixXd F_, H_, B_;
};
