#include "Config.hpp"
#include <yaml-cpp/yaml.h>

Config Config::fromFile(const std::string& path) {
    YAML::Node node = YAML::LoadFile(path);

    Config c;
    c.state_dim = node["state_dimension"].as<int>();
    c.num_steps = node["num_time_steps"].as<int>();
    c.dt = node["dt"].as<double>();

    // Load initial conditions
    auto initial = node["initial"];
    c.x0_true = Eigen::VectorXd(c.state_dim);
    for (int i = 0; i < c.state_dim; ++i) {
        c.x0_true(i) = initial["true_state"][i].as<double>();
    }

    c.x0_est = Eigen::VectorXd(c.state_dim);
    for (int i = 0; i < c.state_dim; ++i) {
        c.x0_est(i) = initial["est_state"][i].as<double>();
    }

    double initial_cov = initial["initial_cov"].as<double>();
    c.P0 = Eigen::MatrixXd::Identity(c.state_dim, c.state_dim) * initial_cov;

    // Load dynamics
    auto dynamics = node["dynamics"];
    c.F = Eigen::MatrixXd(c.state_dim, c.state_dim);
    for (int i = 0; i < c.state_dim; ++i) {
        for (int j = 0; j < c.state_dim; ++j) {
            c.F(i, j) = dynamics["state_transition"][i][j].as<double>();
        }
    }

    auto process_noise_diag = dynamics["process_noise_diag"];
    c.Q = Eigen::MatrixXd::Zero(c.state_dim, c.state_dim);
    for (int i = 0; i < c.state_dim; ++i) {
        c.Q(i, i) = process_noise_diag[i].as<double>();
    }

    // Load measurement
    auto measurement = node["measurement"];
    int meas_dim = measurement["observation_matrix"].size();
    c.H = Eigen::MatrixXd(meas_dim, c.state_dim);
    for (int i = 0; i < meas_dim; ++i) {
        for (int j = 0; j < c.state_dim; ++j) {
            c.H(i, j) = measurement["observation_matrix"][i][j].as<double>();
        }
    }

    double meas_noise = measurement["measurement_noise"].as<double>();
    c.R = Eigen::MatrixXd::Identity(meas_dim, meas_dim) * meas_noise;

    // Load Monte Carlo settings (if present)
    if (node["monte_carlo"]) {
        auto mc = node["monte_carlo"];
        c.num_runs = mc["runs"].as<int>();
        c.base_seed = mc["base_seed"].as<int>();
    } else {
        c.num_runs = 1;
        c.base_seed = 42;
    }

    return c;
}
