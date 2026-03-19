#pragma once
#include <Eigen/Dense>

class LinearGaussianModel {
public:
LinearGaussianModel(const Eigen::MatrixXd& F,
const Eigen::MatrixXd& H);

Eigen::VectorXd propagate(const Eigen::VectorXd& x);
Eigen::VectorXd measure(const Eigen::VectorXd& x);

private:
Eigen::MatrixXd F_, H_;
};
