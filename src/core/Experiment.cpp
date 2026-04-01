#include "Experiment.hpp"
#include "EstimatorFactory.hpp"
#include "LinearGaussianModel.hpp"
#include "Database.hpp"
#include <iostream>
#include <random>
#include <vector>

Experiment::Experiment(const Config& config, Database* db, int run_id)
    : config_(config), db_(db), run_id_(run_id) {}

void Experiment::run() {
    auto estimator = EstimatorFactory::create(config_);
    LinearGaussianModel truth_model(config_.F, config_.H, config_.B);

    estimator->init(config_.x0_est, config_.P0);

    Eigen::VectorXd x_true = config_.x0_true;

    std::mt19937 gen(config_.base_seed + run_id_);
    std::normal_distribution<double> pn(0.0, 1.0);
    std::normal_distribution<double> mn(0.0, 1.0);

    int control_dim = config_.B.cols();
    Eigen::VectorXd u = Eigen::VectorXd::Ones(control_dim) * 0.1;

    for (int k = 0; k < config_.num_steps; ++k) {
        // Propagate true state
        Eigen::VectorXd w(config_.state_dim);
        for (int i = 0; i < config_.state_dim; ++i) {
            w(i) = pn(gen) * std::sqrt(config_.Q(i, i));
        }
        x_true = truth_model.propagate(x_true, u) + w;

        // Generate measurement
        Eigen::VectorXd v(config_.H.rows());
        for (int i = 0; i < config_.H.rows(); ++i) {
            v(i) = mn(gen) * std::sqrt(config_.R(i, i));
        }
        Eigen::VectorXd z = truth_model.measure(x_true) + v;

        estimator->predict(u);
        estimator->update(z);

        // Log to database
        db_->insertTimeStep(run_id_, k, x_true, estimator->state(), estimator->covariance());
    }
}
