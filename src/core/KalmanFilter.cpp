#include "KalmanFilter.hpp"

KalmanFilter::KalmanFilter(const Eigen::MatrixXd& F,
const Eigen::MatrixXd& Q,
const Eigen::MatrixXd& H,
const Eigen::MatrixXd& R)
: F_(F), Q_(Q), H_(H), R_(R) {}

void KalmanFilter::init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) {
x_ = x0;
P_ = P0;
}

void KalmanFilter::predict() {
x_ = F_ * x_;
P_ = F_ * P_ * F_.transpose() + Q_;
}

void KalmanFilter::update(const Eigen::VectorXd& z) {
Eigen::MatrixXd S = H_ * P_ * H_.transpose() + R_;
Eigen::MatrixXd K = P_ * H_.transpose() * S.inverse();
x_ = x_ + K * (z - H_ * x_);
P_ = (Eigen::MatrixXd::Identity(x_.size(), x_.size()) - K * H_) * P_;
}

Eigen::VectorXd KalmanFilter::state() const { return x_; }

Eigen::MatrixXd KalmanFilter::covariance() const { return P_; }
