#include "LinearGaussianModel.hpp"

LinearGaussianModel::LinearGaussianModel(const Eigen::MatrixXd& F,
const Eigen::MatrixXd& H,
const Eigen::MatrixXd& B)
: F_(F), H_(H), B_(B) {}

Eigen::VectorXd LinearGaussianModel::propagate(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
return F_ * x + B_ * u;
}

Eigen::VectorXd LinearGaussianModel::measure(const Eigen::VectorXd& x) {
return H_ * x;
}
