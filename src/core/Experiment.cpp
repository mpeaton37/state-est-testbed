#include "Experiment.hpp"
#include "KalmanFilter.hpp"
#include "LinearGaussianModel.hpp"
#include "Database.hpp"
#include <iostream>
#include <random>
#include <vector>

Experiment::Experiment(const Config& config, Database* db, int run_id)
    : config_(config), db_(db), run_id_(run_id) {}

void Experiment::run() {
    LinearGaussianModel model(config_.F, config_.H);

    KalmanFilter kf(config_.F, config_.Q, config_.H, config_.R);
    kf.init(config_.x0_est, config_.P0);

    Eigen::VectorXd x_true = config_.x0_true;

    // Random number generator for noise
    std::mt19937 gen(config_.base_seed + run_id_);
    std::normal_distribution<double> process_noise(0.0, 1.0);
    std::normal_distribution<double> meas_noise(0.0, 1.0);

    // For RMSE calculation
    std::vector<Eigen::VectorXd> true_states;
    std::vector<Eigen::VectorXd> est_states;

    for (int k = 0; k < config_.num_steps; ++k) {
        // Propagate true state with process noise
        Eigen::VectorXd w(config_.state_dim);
        for (int i = 0; i < config_.state_dim; ++i) {
            w(i) = process_noise(gen) * std::sqrt(config_.Q(i, i));
        }
        x_true = model.propagate(x_true) + w;

        // Generate measurement with measurement noise
        Eigen::VectorXd v(config_.H.rows());
        for (int i = 0; i < config_.H.rows(); ++i) {
            v(i) = meas_noise(gen) * std::sqrt(config_.R(i, i));
        }
        Eigen::VectorXd z = model.measure(x_true) + v;

        kf.predict();
        kf.update(z);

        // Store for RMSE
        true_states.push_back(x_true);
        est_states.push_back(kf.state());

        // Log to database
        db_->insertTimeStep(run_id_, k, x_true, kf.state(), kf.covariance());

        std::cout << "Step " << k
                  << " | True: " << x_true.transpose()
                  << " | Est: " << kf.state().transpose()
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
