#include "LinearGaussianModel.hpp"

LinearGaussianModel::LinearGaussianModel(const Eigen::MatrixXd& F,
const Eigen::MatrixXd& H)
: F_(F), H_(H) {}

Eigen::VectorXd LinearGaussianModel::propagate(const Eigen::VectorXd& x) {
return F_ * x;
}

Eigen::VectorXd LinearGaussianModel::measure(const Eigen::VectorXd& x) {
return H_ * x;
}
