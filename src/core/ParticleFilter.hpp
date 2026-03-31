#pragma once
#include <Eigen/Dense>
#include <vector>
#include <random>
#include "Estimator.hpp"
#include "DynamicsModel.hpp"

class ParticleFilter : public Estimator {
public:
    struct Params {
        int    num_particles      = 1000;
        double resample_threshold = 0.5;   // ESS / N threshold
    };

    // Constructor - default argument removed to avoid initialization order issue
    ParticleFilter(const Eigen::MatrixXd& Q,
                   const Eigen::MatrixXd& R,
                   const DynamicsModel* model,
                   Params params = Params());   // Pass by value is fine and simpler here

    void init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) override;

    void predict(const Eigen::VectorXd& u = Eigen::VectorXd()) override;
    void update(const Eigen::VectorXd& z) override;

    Eigen::VectorXd state() const override;
    Eigen::MatrixXd covariance() const override;

private:
    const DynamicsModel* model_;   // not owned
    Eigen::MatrixXd Q_;
    Eigen::MatrixXd R_;

    Params params_;

    std::vector<Eigen::VectorXd> particles_;
    std::vector<double> weights_;

    std::mt19937 rng_;

    void resample();
    double effectiveSampleSize() const;
};