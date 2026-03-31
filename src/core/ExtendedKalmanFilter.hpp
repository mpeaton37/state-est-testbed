#pragma once

#include <Eigen/Dense>

#include <memory>

#include "Estimator.hpp"

#include "DynamicsModel.hpp"



// Extended Kalman Filter implementation

class ExtendedKalmanFilter : public Estimator {

public:

    // Constructor: takes system matrices for compatibility, and a DynamicsModel pointer (not owned)

    ExtendedKalmanFilter(const Eigen::MatrixXd& F,

                         const Eigen::MatrixXd& Q,

                         const Eigen::MatrixXd& H,

                         const Eigen::MatrixXd& R,

                         const Eigen::MatrixXd& B,

                         const DynamicsModel* model);



    void init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0);



    void predict(const Eigen::VectorXd& u = Eigen::VectorXd()) override;

    void update(const Eigen::VectorXd& z) override;



    Eigen::VectorXd state() const override;

    Eigen::MatrixXd covariance() const override;



    std::pair<Eigen::VectorXd, Eigen::MatrixXd> get_state_and_covariance() const;



private:

    Eigen::VectorXd x_;

    Eigen::MatrixXd P_;



    Eigen::MatrixXd F_;

    Eigen::MatrixXd Q_;

    Eigen::MatrixXd H_;

    Eigen::MatrixXd R_;

    Eigen::MatrixXd B_;



    const DynamicsModel* model_; // Not owned

};