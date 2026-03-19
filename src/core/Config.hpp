#pragma once
#include <Eigen/Dense>
#include <string>

struct Config {
int state_dim;
int num_steps;
int base_seed;
int num_runs;
double dt;

Eigen::VectorXd x0_true;
Eigen::VectorXd x0_est;
Eigen::MatrixXd P0;

Eigen::MatrixXd F, Q, H, R;

static Config fromFile(const std::string& path);

};
