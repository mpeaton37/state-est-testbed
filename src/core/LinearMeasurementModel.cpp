#include "LinearMeasurementModel.hpp"

LinearMeasurementModel::LinearMeasurementModel(const Eigen::MatrixXd& H, const Eigen::MatrixXd& R)
    : H_(H), R_(R) {}

Eigen::VectorXd LinearMeasurementModel::measure(const Eigen::VectorXd& x) const {
    return H_ * x;
}

Eigen::MatrixXd LinearMeasurementModel::getHJacobian(const Eigen::VectorXd& /*x*/) const {
    return H_;
}

Eigen::MatrixXd LinearMeasurementModel::getR() const {
    return R_;
}

void LinearMeasurementModel::setR(const Eigen::MatrixXd& newR) {
    R_ = newR;
}

int LinearMeasurementModel::getMeasurementDimension() const {
    return static_cast<int>(H_.rows());
}
