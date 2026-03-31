#pragma once
#include <memory>
#include "Config.hpp"
#include "Estimator.hpp"
#include "KalmanFilter.hpp"
// Future: #include "EKF.hpp", "UKF.hpp", "IMM.hpp", "MHT.hpp"

class EstimatorFactory {
public:
    // Factory method to create the appropriate estimator based on config
    static std::unique_ptr<Estimator> create(const Config& config);
};
