#include "ParticleFilter.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <stdexcept>

ParticleFilter::ParticleFilter(const Eigen::MatrixXd& Q,
                               const Eigen::MatrixXd& R,
                               const DynamicsModel* model,
                               Params params)
    : model_(model),
      Q_(Q),
      R_(R),
      params_(params),
      rng_(std::random_device{}())
{
    if (!model_) {
        throw std::invalid_argument("DynamicsModel pointer must not be null for ParticleFilter");
    }
}

void ParticleFilter::init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) {
    particles_.resize(params_.num_particles);
    weights_.assign(params_.num_particles, 1.0 / params_.num_particles);

    // Initialize particles from N(x0, P0) using Cholesky
    std::normal_distribution<double> dist(0.0, 1.0);
    Eigen::MatrixXd L = P0.llt().matrixL();

    for (auto& p : particles_) {
        p = x0;
        for (int i = 0; i < x0.size(); ++i) {
            p(i) += L(i, i) * dist(rng_);
        }
    }
}

void ParticleFilter::predict(const Eigen::VectorXd& u) {
    std::normal_distribution<double> noise(0.0, 1.0);

    for (auto& p : particles_) {
        Eigen::VectorXd w(Q_.rows());
        for (int i = 0; i < Q_.rows(); ++i) {
            w(i) = noise(rng_) * std::sqrt(Q_(i, i));
        }
        p = model_->propagate(p, u) + w;
    }
}

void ParticleFilter::update(const Eigen::VectorXd& z) {
    // Update particle weights using measurement likelihood
    for (size_t i = 0; i < particles_.size(); ++i) {
        Eigen::VectorXd z_pred = model_->measure(particles_[i]);
        Eigen::VectorXd innov = z - z_pred;

        double mahal = innov.transpose() * R_.inverse() * innov;
        double likelihood = std::exp(-0.5 * mahal) / std::sqrt(2.0 * M_PI * R_.determinant());

        weights_[i] *= likelihood;
    }

    // Normalize weights
    double sum_w = std::accumulate(weights_.begin(), weights_.end(), 0.0);
    if (sum_w > 1e-12) {
        for (auto& w : weights_) w /= sum_w;
    } else {
        std::fill(weights_.begin(), weights_.end(), 1.0 / params_.num_particles);
    }

    // Resample if effective sample size is low
    if (effectiveSampleSize() < params_.resample_threshold * params_.num_particles) {
        resample();
    }
}

Eigen::VectorXd ParticleFilter::state() const {
    if (particles_.empty()) return Eigen::VectorXd();

    Eigen::VectorXd mean = Eigen::VectorXd::Zero(particles_[0].size());
    for (size_t i = 0; i < particles_.size(); ++i) {
        mean += weights_[i] * particles_[i];
    }
    return mean;
}

Eigen::MatrixXd ParticleFilter::covariance() const {
    if (particles_.empty()) return Eigen::MatrixXd();

    Eigen::VectorXd mean = state();
    Eigen::MatrixXd cov = Eigen::MatrixXd::Zero(mean.size(), mean.size());

    for (size_t i = 0; i < particles_.size(); ++i) {
        Eigen::VectorXd diff = particles_[i] - mean;
        cov += weights_[i] * (diff * diff.transpose());
    }
    return cov;
}

double ParticleFilter::effectiveSampleSize() const {
    double sum_sq = 0.0;
    for (double w : weights_) sum_sq += w * w;
    return (sum_sq > 1e-12) ? 1.0 / sum_sq : 0.0;
}

void ParticleFilter::resample() {
    std::vector<Eigen::VectorXd> new_particles(params_.num_particles);
    std::vector<double> new_weights(params_.num_particles, 1.0 / params_.num_particles);

    // Systematic resampling
    double u = std::uniform_real_distribution<double>(0.0, 1.0 / params_.num_particles)(rng_);
    double cum = 0.0;
    size_t j = 0;

    for (size_t i = 0; i < params_.num_particles; ++i) {
        double target = u + static_cast<double>(i) / params_.num_particles;
        while (target > cum + weights_[j]) {
            cum += weights_[j];
            ++j;
            if (j >= weights_.size()) j = 0;
        }
        new_particles[i] = particles_[j];
    }

    particles_ = std::move(new_particles);
    weights_ = std::move(new_weights);
}