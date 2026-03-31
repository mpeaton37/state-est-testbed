#include "ExtendedKalmanFilter.hpp"

#include <stdexcept>



ExtendedKalmanFilter::ExtendedKalmanFilter(const Eigen::MatrixXd& F,

                                           const Eigen::MatrixXd& Q,

                                           const Eigen::MatrixXd& H,

                                           const Eigen::MatrixXd& R,

                                           const Eigen::MatrixXd& B,

                                           const DynamicsModel* model)

    : F_(F), Q_(Q), H_(H), R_(R), B_(B), model_(model)

{

    if (!model_) {

        throw std::invalid_argument("DynamicsModel pointer must not be null");

    }

}



void ExtendedKalmanFilter::init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) {

    x_ = x0;

    P_ = P0;

}



void ExtendedKalmanFilter::predict(const Eigen::VectorXd& u) {

    // Use nonlinear model if available, else fall back to linear F/B

    if (model_) {

        x_ = model_->propagate(x_, u);

        // Linearize at current state

        Eigen::MatrixXd Fk = model_->getFJacobian(x_, u);

        P_ = Fk * P_ * Fk.transpose() + Q_;

    } else {

        // Should not happen, but fallback for safety

        if (u.size() > 0) {

            x_ = F_ * x_ + B_ * u;

        } else {

            x_ = F_ * x_;

        }

        P_ = F_ * P_ * F_.transpose() + Q_;

    }

}



void ExtendedKalmanFilter::update(const Eigen::VectorXd& z) {

    // Linearize measurement at predicted state

    Eigen::MatrixXd Hk = model_ ? model_->getHJacobian(x_) : H_;

    Eigen::VectorXd z_pred = model_ ? model_->measure(x_) : H_ * x_;



    Eigen::MatrixXd S = Hk * P_ * Hk.transpose() + R_;

    Eigen::MatrixXd K = P_ * Hk.transpose() * S.inverse();



    x_ = x_ + K * (z - z_pred);

    P_ = (Eigen::MatrixXd::Identity(x_.size(), x_.size()) - K * Hk) * P_;

}



Eigen::VectorXd ExtendedKalmanFilter::state() const {

    return x_;

}



Eigen::MatrixXd ExtendedKalmanFilter::covariance() const {

    return P_;

}



std::pair<Eigen::VectorXd, Eigen::MatrixXd> ExtendedKalmanFilter::get_state_and_covariance() const {

    return {x_, P_};

}
