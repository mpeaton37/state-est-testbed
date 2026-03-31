#include "ParticleFilter.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

ParticleFilter::ParticleFilter(const Eigen::MatrixXd& Q,
                               const Eigen::MatrixXd& R,
                               const DynamicsModel* model,
                               const Params& params)
    : model_(model), Q_(Q), R_(R), params_(params), rng_(std::random_device{}()) {
    if (!model_) {
        throw std::invalid_argument("DynamicsModel pointer must not be null for ParticleFilter");
    }
}

void ParticleFilter::init(const Eigen::VectorXd& x0, const Eigen::MatrixXd& P0) {
    int n = x0.size();
    particles_.resize(params_.num_particles);
    weights_.assign(params_.num_particles, 1.0 / params_.num_particles);

    // Initialize particles from N(x0, P0)
    std::normal_distribution<double> dist(0.0, 1.0);
    Eigen::MatrixXd L = P0.llt().matrixL();  // Cholesky

    for (auto& p : particles_) {
        p = x0;
        for (int i = 0; i < n; ++i) {
            p(i) += L(i,i) * dist(rng_);   // simple diagonal for now; improve later
        }
    }
}

void ParticleFilter::predict(const Eigen::VectorXd& u) {
    std::normal_distribution<double> noise(0.0, 1.0);

    for (auto& p : particles_) {
        Eigen::VectorXd w(Q_.rows());
        for (int i = 0; i < Q_.rows(); ++i) {
            w(i) = noise(rng_) * std::sqrt(Q_(i,i));
        }
        p = model_->propagate(p, u) + w;
    }
}

void ParticleFilter::update(const Eigen::VectorXd& z) {
    // Compute likelihood weights (assume Gaussian measurement noise for now)
    for (size_t i = 0; i < particles_.size(); ++i) {
        Eigen::VectorXd z_pred = model_->measure(particles_[i]);
        Eigen::VectorXd innov = z - z_pred;

        // Simple Gaussian likelihood (can be extended)
        double mahal = innov.transpose() * R_.inverse() * innov;
        weights_[i] *= std::exp(-0.5 * mahal) / std::sqrt((2 * M_PI * R_.determinant()));
    }

    // Normalize weights
    double sum_w = std::accumulate(weights_.begin(), weights_.end(), 0.0);
    if (sum_w > 0.0) {
        for (auto& w : weights_) w /= sum_w;
    }

    // Resample if effective sample size is too low
    if (effectiveSampleSize() < params_.resample_threshold * params_.num_particles) {
        resample();
    }
}

Eigen::VectorXd ParticleFilter::state() const {
    Eigen::VectorXd mean = Eigen::VectorXd::Zero(particles_[0].size());
    for (size_t i = 0; i < particles_.size(); ++i) {
        mean += weights_[i] * particles_[i];
    }
    return mean;
}

Eigen::MatrixXd ParticleFilter::covariance() const {
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
    return 1.0 / sum_sq;
}

void ParticleFilter::resample() {
    // Systematic resampling (simple and effective)
    std::vector<Eigen::VectorXd> new_particles(params_.num_particles);
    std::vector<double> new_weights(params_.num_particles, 1.0 / params_.num_particles);

    double u = std::uniform_real_distribution<double>(0.0, 1.0 / params_.num_particles)(rng_);
    double cum = 0.0;
    size_t j = 0;

    for (size_t i = 0; i < params_.num_particles; ++i) {
        double target = u + static_cast<double>(i) / params_.num_particles;
        while (target > cum + weights_[j]) {
            cum += weights_[j];
            ++j;
        }
        new_particles[i] = particles_[j];
    }

    particles_ = std::move(new_particles);
    weights_ = std::move(new_weights);
}