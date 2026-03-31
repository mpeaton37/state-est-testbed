#include "Experiment.hpp"
#include "KalmanFilter.hpp"
#include "LinearGaussianModel.hpp"
#include "Database.hpp"
#include "EstimatorFactory.hpp"
#include <iostream>
#include <random>
#include <vector>
#include <memory>

Experiment::Experiment(const Config& config, Database* db, int run_id)
    : config_(config), db_(db), run_id_(run_id) {}

void Experiment::run() {
    // Create the appropriate estimator using the factory
    std::unique_ptr<Estimator> estimator = EstimatorFactory::create(config_);

    // Create the appropriate dynamics model (for truth propagation)
    // For now, always use LinearGaussianModel for truth (could extend for nonlinear)
    LinearGaussianModel model(config_.F, config_.H, config_.B);

    estimator->init(config_.x0_est, config_.P0);

    Eigen::VectorXd x_true = config_.x0_true;

    // Random number generator for noise
    std::mt19937 gen(config_.base_seed + run_id_);
    std::normal_distribution<double> process_noise(0.0, 1.0);
    std::normal_distribution<double> meas_noise(0.0, 1.0);

    // For RMSE calculation
    std::vector<Eigen::VectorXd> true_states;
    std::vector<Eigen::VectorXd> est_states;

    // Assume a simple constant control input for demonstration; can be made configurable
    int control_dim = config_.B.cols();
    Eigen::VectorXd u = Eigen::VectorXd::Ones(control_dim) * 0.1;  // Example: small constant input

    for (int k = 0; k < config_.num_steps; ++k) {
        // Propagate true state with process noise
        Eigen::VectorXd w(config_.state_dim);
        for (int i = 0; i < config_.state_dim; ++i) {
            w(i) = process_noise(gen) * std::sqrt(config_.Q(i, i));
        }
        x_true = model.propagate(x_true, u) + w;

        // Generate measurement with measurement noise
        Eigen::VectorXd v(config_.H.rows());
        for (int i = 0; i < config_.H.rows(); ++i) {
            v(i) = meas_noise(gen) * std::sqrt(config_.R(i, i));
        }
        Eigen::VectorXd z = model.measure(x_true) + v;

        estimator->predict(u);
        estimator->update(z);

        // Store for RMSE
        true_states.push_back(x_true);
        est_states.push_back(estimator->state());

        // Log to database
        db_->insertTimeStep(run_id_, k, x_true, estimator->state(), estimator->covariance());

        std::cout << "Step " << k
                  << " | True: " << x_true.transpose()
                  << " | Est: " << estimator->state().transpose()
                  << std::endl;
    }

    // Compute and log RMSE
    double rmse = 0.0;
    for (size_t k = 0; k < true_states.size(); ++k) {
        Eigen::VectorXd error = true_states[k] - est_states[k];
        rmse += error.squaredNorm();
    }
    rmse = std::sqrt(rmse / true_states.size());
    db_->insertSummary(run_id_, rmse);
    std::cout << "Run " << run_id_ << " RMSE: " << rmse << std::endl;
}
