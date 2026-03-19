#include <gtest/gtest.h>
#include "KalmanFilter.hpp"
#include <Eigen/Dense>

// Test Kalman Filter predict and update steps
TEST(KalmanFilterTest, PredictUpdate) {
    // Simple 1D constant velocity model
    Eigen::MatrixXd F(1, 1);
    F << 1.0;
    Eigen::MatrixXd Q(1, 1);
    Q << 0.01;
    Eigen::MatrixXd H(1, 1);
    H << 1.0;
    Eigen::MatrixXd R(1, 1);
    R << 0.1;

    KalmanFilter kf(F, Q, H, R);

    Eigen::VectorXd x0(1);
    x0 << 0.0;
    Eigen::MatrixXd P0(1, 1);
    P0 << 1.0;
    kf.init(x0, P0);

    // Initial state
    EXPECT_NEAR(kf.state()(0), 0.0, 1e-6);
    EXPECT_NEAR(kf.covariance()(0, 0), 1.0, 1e-6);

    // Predict step
    kf.predict();
    EXPECT_NEAR(kf.state()(0), 0.0, 1e-6);
    EXPECT_NEAR(kf.covariance()(0, 0), 1.01, 1e-6);  // P + Q

    // Update step with measurement z = 0.5
    Eigen::VectorXd z(1);
    z << 0.5;
    kf.update(z);

    // Check updated state and covariance
    double expected_x = 0.5 * 1.01 / (1.01 + 0.1);  // Kalman gain * residual
    EXPECT_NEAR(kf.state()(0), expected_x, 1e-6);
    EXPECT_NEAR(kf.covariance()(0, 0), (1.01 * 0.1) / (1.01 + 0.1), 1e-6);
}

TEST(KalmanFilterTest, MultiStepPrediction) {
    Eigen::MatrixXd F(2, 2);
    F << 1.0, 0.1, 0.0, 1.0;
    Eigen::MatrixXd Q(2, 2);
    Q << 0.01, 0.0, 0.0, 0.005;
    Eigen::MatrixXd H(1, 2);
    H << 1.0, 0.0;
    Eigen::MatrixXd R(1, 1);
    R << 0.1;

    KalmanFilter kf(F, Q, H, R);

    Eigen::VectorXd x0(2);
    x0 << 0.0, 1.0;
    Eigen::MatrixXd P0(2, 2);
    P0 << 1.0, 0.0, 0.0, 1.0;
    kf.init(x0, P0);

    kf.predict();
    EXPECT_NEAR(kf.state()(0), 0.1, 1e-5);
    EXPECT_NEAR(kf.state()(1), 1.0, 1e-5);

    Eigen::VectorXd z(1);
    z << 0.2;
    kf.update(z);

    EXPECT_GT(kf.state()(0), 0.0);
    EXPECT_LT(kf.covariance()(0, 0), 1.0);
}

TEST(KalmanFilterTest, ZeroMeasurementNoise) {
    Eigen::MatrixXd F(1, 1);
    F << 1.0;
    Eigen::MatrixXd Q(1, 1);
    Q << 0.0;
    Eigen::MatrixXd H(1, 1);
    H << 1.0;
    Eigen::MatrixXd R(1, 1);
    R << 0.0;

    KalmanFilter kf(F, Q, H, R);

    Eigen::VectorXd x0(1);
    x0 << 5.0;
    Eigen::MatrixXd P0(1, 1);
    P0 << 1.0;
    kf.init(x0, P0);

    Eigen::VectorXd z(1);
    z << 10.0;
    kf.update(z);

    EXPECT_NEAR(kf.state()(0), 10.0, 1e-6);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
