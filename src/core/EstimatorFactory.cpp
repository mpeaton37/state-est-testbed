#include "EstimatorFactory.hpp"
#include "KalmanFilter.hpp"
#include "UnscentedKalmanFilter.hpp"
#include "ExtendedKalmanFilter.hpp"
#include "ParticleFilter.hpp"
#include "LinearGaussianModel.hpp"
// Future: #include "InteractingMultipleModel.hpp"
// Future: #include "MultiHypothesisTracker.hpp"

std::unique_ptr<Estimator> EstimatorFactory::create(const Config& config) {
    if (config.estimator_type == "kf") {
        return std::make_unique<KalmanFilter>(
            config.F, config.Q, config.H, config.R, config.B
        );
    }
    else if (config.estimator_type == "ukf") {
        // UKF requires a DynamicsModel pointer (LinearGaussianModel now derives from DynamicsModel)
        static LinearGaussianModel ukf_model(config.F, config.H, config.B);

        return std::make_unique<UnscentedKalmanFilter>(
            config.Q,
            config.R,
            &ukf_model,                    // raw pointer - UKF does NOT take ownership
            config.ukf_alpha,
            config.ukf_beta,
            config.ukf_kappa
        );
    }
    else if (config.estimator_type == "ekf") {
        // EKF also requires a DynamicsModel pointer for nonlinear propagation + Jacobians
        static LinearGaussianModel ekf_model(config.F, config.H, config.B);

        return std::make_unique<ExtendedKalmanFilter>(
            config.F,
            config.Q,
            config.H,
            config.R,
            config.B,
            &ekf_model                     // raw pointer - EKF does NOT take ownership
        );
    }
    else if (config.estimator_type == "pf") {
        ParticleFilter::Params pf_params;
        pf_params.num_particles      = config.pf_num_particles;
        pf_params.resample_threshold = config.pf_resample_threshold;

        static LinearGaussianModel pf_model(config.F, config.H, config.B);

        return std::make_unique<ParticleFilter>(
            config.Q,
            config.R,
            &pf_model,
            pf_params          // must pass explicitly now
        );
    }
    // Future extensions:
    // else if (config.estimator_type == "imm") { ... }
    // else if (config.estimator_type == "mht") { ... }

    throw std::runtime_error("Unknown estimator_type: " + config.estimator_type);
}