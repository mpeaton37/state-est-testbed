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

    // Load control matrix (optional, default to zero)
    if (dynamics["control_matrix"]) {
        auto control_matrix = dynamics["control_matrix"];
        int control_dim = control_matrix[0].size();  // Assume rows == state_dim, cols == control_dim
        c.B = Eigen::MatrixXd(c.state_dim, control_dim);
        for (int i = 0; i < c.state_dim; ++i) {
            for (int j = 0; j < control_dim; ++j) {
                c.B(i, j) = control_matrix[i][j].as<double>();
            }
        }
    } else {
        c.B = Eigen::MatrixXd::Zero(c.state_dim, 1);  // Default to zero matrix with minimal control dim
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

    // Load estimator/model type (optional)
    if (node["estimator_type"])
        c.estimator_type = node["estimator_type"].as<std::string>();
    else
        c.estimator_type = "kf";

    if (node["model_type"])
        c.model_type = node["model_type"].as<std::string>();
    else
        c.model_type = "linear";

    // IMM support
    if (node["imm"]) {
        auto imm = node["imm"];
        // Sub-models
        if (imm["submodels"]) {
            for (const auto& sub : imm["submodels"]) {
                Config sub_cfg = c; // inherit base config, override below
                if (sub["F"]) {
                    for (int i = 0; i < c.state_dim; ++i)
                        for (int j = 0; j < c.state_dim; ++j)
                            sub_cfg.F(i, j) = sub["F"][i][j].as<double>();
                }
                if (sub["Q"]) {
                    for (int i = 0; i < c.state_dim; ++i)
                        for (int j = 0; j < c.state_dim; ++j)
                            sub_cfg.Q(i, j) = sub["Q"][i][j].as<double>();
                }
                if (sub["H"]) {
                    for (int i = 0; i < c.H.rows(); ++i)
                        for (int j = 0; j < c.H.cols(); ++j)
                            sub_cfg.H(i, j) = sub["H"][i][j].as<double>();
                }
                if (sub["R"]) {
                    for (int i = 0; i < c.R.rows(); ++i)
                        for (int j = 0; j < c.R.cols(); ++j)
                            sub_cfg.R(i, j) = sub["R"][i][j].as<double>();
                }
                if (sub["model_type"])
                    sub_cfg.model_type = sub["model_type"].as<std::string>();
                if (sub["estimator_type"])
                    sub_cfg.estimator_type = sub["estimator_type"].as<std::string>();
                c.imm_submodels.push_back(sub_cfg);
            }
        }
        // Transition matrix
        if (imm["transition_matrix"]) {
            int n = imm["transition_matrix"].size();
            c.imm_transition_matrix = Eigen::MatrixXd(n, n);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    c.imm_transition_matrix(i, j) = imm["transition_matrix"][i][j].as<double>();
        }
        // Initial probabilities
        if (imm["initial_probs"]) {
            for (size_t i = 0; i < imm["initial_probs"].size(); ++i)
                c.imm_initial_probs.push_back(imm["initial_probs"][i].as<double>());
        }
    }

    // UKF parameters
    if (node["ukf"]) {
        auto ukf = node["ukf"];
        if (ukf["alpha"]) c.ukf_alpha = ukf["alpha"].as<double>();
        if (ukf["beta"])  c.ukf_beta  = ukf["beta"].as<double>();
        if (ukf["kappa"]) c.ukf_kappa = ukf["kappa"].as<double>();
    }

    // EKF parameters (new - added for ExtendedKalmanFilter support)
    if (node["ekf"]) {
        auto ekf = node["ekf"];
        // Currently EKF uses the same matrices as base config (F, Q, H, R, B)
        // Add specific parameters here in the future if needed (e.g. tuning, numerical settings)
        if (ekf["alpha"]) c.ekf_alpha = ekf["alpha"].as<double>();           // example placeholder
//        if (ekf["beta"])  c.ekf_beta  = ekf["beta"].as<double>();            // example placeholder
    }

    // Particle Filter parameters
    if (node["pf"]) {
        auto pf = node["pf"];
        if (pf["num_particles"])     c.pf_num_particles     = pf["num_particles"].as<int>();
        if (pf["resample_threshold"]) c.pf_resample_threshold = pf["resample_threshold"].as<double>();
    }

    // MHT parameters
    if (node["mht"]) {
        auto mht = node["mht"];
        if (mht["gating_threshold"]) c.mht_gating_threshold = mht["gating_threshold"].as<double>();
        if (mht["max_hypotheses"])   c.mht_max_hypotheses   = mht["max_hypotheses"].as<int>();
    }

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