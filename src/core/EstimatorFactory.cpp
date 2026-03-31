#include "EstimatorFactory.hpp"
#include "KalmanFilter.hpp"
// Future: #include "EKF.hpp", "UKF.hpp", "IMM.hpp", "MHT.hpp"

std::unique_ptr<Estimator> EstimatorFactory::create(const Config& config) {
    // For now, only support "kf"
    if (config.estimator_type == "kf") {
        return std::make_unique<KalmanFilter>(
            config.F, config.Q, config.H, config.R, config.B
        );
    }
    // Future: add EKF, UKF, IMM, MHT, etc.
    // else if (config.estimator_type == "ekf") { ... }
    // else if (config.estimator_type == "ukf") { ... }
    // else if (config.estimator_type == "imm") { ... }
    // else if (config.estimator_type == "mht") { ... }

    throw std::runtime_error("Unknown estimator_type: " + config.estimator_type);
}
