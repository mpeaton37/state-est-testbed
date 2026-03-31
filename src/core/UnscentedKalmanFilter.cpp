#include "UnscentedKalmanFilter.hpp"
#include <stdexcept>
#include <cmath>

UnscentedKalmanFilter::UnscentedKalmanFilter(const Eigen::MatrixXd& Q,
                                             const Eigen::MatrixXd& R,
                                             const DynamicsModel* model,
                                             double alpha,
                                             double beta,
                                             double kappa)
    : Q_(Q), R_(R), model_(model), alpha_(alpha), beta_(beta), kappa_(kappa)
{
    if (!model_) {
        throw std::invalid_argument("DynamicsModel pointer must not be null");
    }
    // n_ will be set in init()
}

void UnscentedKalmanFilter::init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) {
    x_ = x0;
    P_ = P0;
    n_ = x0.size();
    L_ = 2 * n_ + 1;
    computeWeights();
}

void UnscentedKalmanFilter::computeWeights() {
    double lambda = alpha_ * alpha_ * (n_ + kappa_) - n_;
    wm_ = Eigen::VectorXd(L_);
    wc_ = Eigen::VectorXd(L_);
    wm_(0) = lambda / (n_ + lambda);
    wc_(0) = wm_(0) + (1 - alpha_ * alpha_ + beta_);
    for (int i = 1; i < L_; ++i) {
        wm_(i) = 1.0 / (2 * (n_ + lambda));
        wc_(i) = wm_(i);
    }
}

void UnscentedKalmanFilter::computeSigmaPoints(const Eigen::VectorXd& x, const Eigen::MatrixXd& P, Eigen::MatrixXd& Xsig) const {
    double lambda = alpha_ * alpha_ * (n_ + kappa_) - n_;
    Eigen::MatrixXd A = P.llt().matrixL();
    Xsig.col(0) = x;
    double scaling = std::sqrt(n_ + lambda);
    for (int i = 0; i < n_; ++i) {
        Xsig.col(i + 1) = x + scaling * A.col(i);
        Xsig.col(i + 1 + n_) = x - scaling * A.col(i);
    }
}

void UnscentedKalmanFilter::predict(const Eigen::VectorXd& u) {
    // Generate sigma points
    Eigen::MatrixXd Xsig(n_, L_);
    computeSigmaPoints(x_, P_, Xsig);

    // Propagate through process model
    Eigen::MatrixXd Xsig_pred(n_, L_);
    for (int i = 0; i < L_; ++i) {
        Xsig_pred.col(i) = model_->propagate(Xsig.col(i), u);
    }

    // Predicted mean
    Eigen::VectorXd x_pred = Eigen::VectorXd::Zero(n_);
    for (int i = 0; i < L_; ++i) {
        x_pred += wm_(i) * Xsig_pred.col(i);
    }

    // Predicted covariance
    Eigen::MatrixXd P_pred = Eigen::MatrixXd::Zero(n_, n_);
    for (int i = 0; i < L_; ++i) {
        Eigen::VectorXd dx = Xsig_pred.col(i) - x_pred;
        P_pred += wc_(i) * (dx * dx.transpose());
    }
    P_pred += Q_;

    x_ = x_pred;
    P_ = P_pred;
}

void UnscentedKalmanFilter::update(const Eigen::VectorXd& z) {
    int m = z.size();

    // Generate sigma points
    Eigen::MatrixXd Xsig(n_, L_);
    computeSigmaPoints(x_, P_, Xsig);

    // Transform sigma points through measurement model
    Eigen::MatrixXd Zsig(m, L_);
    for (int i = 0; i < L_; ++i) {
        Zsig.col(i) = model_->measure(Xsig.col(i));
    }

    // Predicted measurement mean
    Eigen::VectorXd z_pred = Eigen::VectorXd::Zero(m);
    for (int i = 0; i < L_; ++i) {
        z_pred += wm_(i) * Zsig.col(i);
    }

    // Innovation covariance
    Eigen::MatrixXd S = Eigen::MatrixXd::Zero(m, m);
    for (int i = 0; i < L_; ++i) {
        Eigen::VectorXd dz = Zsig.col(i) - z_pred;
        S += wc_(i) * (dz * dz.transpose());
    }
    S += R_;

    // Cross covariance
    Eigen::MatrixXd Pxz = Eigen::MatrixXd::Zero(n_, m);
    for (int i = 0; i < L_; ++i) {
        Eigen::VectorXd dx = Xsig.col(i) - x_;
        Eigen::VectorXd dz = Zsig.col(i) - z_pred;
        Pxz += wc_(i) * (dx * dz.transpose());
    }

    // Kalman gain
    Eigen::MatrixXd K = Pxz * S.inverse();

    // Update state and covariance
    x_ = x_ + K * (z - z_pred);
    P_ = P_ - K * S * K.transpose();
}

Eigen::VectorXd UnscentedKalmanFilter::state() const {
    return x_;
}

Eigen::MatrixXd UnscentedKalmanFilter::covariance() const {
    return P_;
}

std::pair<Eigen::VectorXd, Eigen::MatrixXd> UnscentedKalmanFilter::get_state_and_covariance() const {
    return {x_, P_};
}
