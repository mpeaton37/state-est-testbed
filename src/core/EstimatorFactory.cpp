#include "EstimatorFactory.hpp"
#include "KalmanFilter.hpp"
#include "UnscentedKalmanFilter.hpp"
#include "LinearGaussianModel.hpp"
// Future: #include "EKF.hpp", "IMM.hpp", "MHT.hpp"

// SDD.md §13: All estimator_type logic centralized here for extensibility

std::unique_ptr<Estimator> EstimatorFactory::create(const Config& config) {
    if (config.estimator_type == "kf") {
        return std::make_unique<KalmanFilter>(
            config.F, config.Q, config.H, config.R, config.B
        );
    } else if (config.estimator_type == "ukf") {
        // For UKF, create a LinearGaussianModel (could be extended for nonlinear)
        // Model is NOT owned by UKF, so we must keep it alive somewhere else if needed.
        // Here, we use a static to persist for the lifetime of the process (testbed is single-threaded).
        static LinearGaussianModel ukf_model(config.F, config.H, config.B);
        return std::make_unique<UnscentedKalmanFilter>(
            config.Q,
            config.R,
            &ukf_model,
            config.ukf_alpha,
            config.ukf_beta,
            config.ukf_kappa
        );
    }
    // Future: add EKF, IMM, MHT, etc.
    // else if (config.estimator_type == "ekf") { ... }
    // else if (config.estimator_type == "imm") { ... }
    // else if (config.estimator_type == "mht") { ... }

    throw std::runtime_error("Unknown estimator_type: " + config.estimator_type);
}
