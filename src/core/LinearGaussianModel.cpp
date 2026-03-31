#include "LinearGaussianModel.hpp"

LinearGaussianModel::LinearGaussianModel(const Eigen::MatrixXd& F,
                                         const Eigen::MatrixXd& H,
                                         const Eigen::MatrixXd& B)
    : F_(F), H_(H), B_(B) {}

Eigen::VectorXd LinearGaussianModel::propagate(const Eigen::VectorXd& x, const Eigen::VectorXd& u) const {
    return F_ * x + B_ * u;
}

Eigen::VectorXd LinearGaussianModel::measure(const Eigen::VectorXd& x) const {
    return H_ * x;
}

Eigen::MatrixXd LinearGaussianModel::getFJacobian(const Eigen::VectorXd& x, const Eigen::VectorXd& u) const {
    (void)x; (void)u;
    return F_;
}

Eigen::MatrixXd LinearGaussianModel::getHJacobian(const Eigen::VectorXd& x) const {
    (void)x;
    return H_;
}
