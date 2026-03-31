#pragma once
#include <Eigen/Dense>
#include <string>
#include <vector>

struct Config {
    int state_dim;
    int num_steps;
    int base_seed;
    int num_runs;
    double dt;

    Eigen::VectorXd x0_true;
    Eigen::VectorXd x0_est;
    Eigen::MatrixXd P0;

    Eigen::MatrixXd F, Q, H, R, B;

    // New fields for estimator/model selection
    std::string estimator_type; // "kf", "ekf", "ukf", "imm", "mht"
    std::string model_type;     // "linear", "nonlinear", etc.

    // IMM: sub-model configs (for IMM, each sub-model can have its own config)
    std::vector<Config> imm_submodels;
    Eigen::MatrixXd imm_transition_matrix;
    std::vector<double> imm_initial_probs;

    // UKF parameters
    double ukf_alpha = 1e-3;
    double ukf_beta = 2.0;
    double ukf_kappa = 0.0;

    // MHT parameters
    double mht_gating_threshold = 9.21; // Default: chi2(2) 99% ≈ 9.21
    int mht_max_hypotheses = 10;

    static Config fromFile(const std::string& path);
};
