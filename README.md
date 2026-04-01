# State Estimation Testbed

A modular, simulation-based C++ framework for developing and evaluating state estimation algorithms.

---

## Overview

This project provides a reproducible, extensible environment for:

* Simulating linear dynamical systems
* Running Kalman Filter-based estimators (KF, UKF, EKF, PF)
* Performing Monte Carlo experiments
* Evaluating statistical consistency (RMSE)
* Visualizing results via Python tools

---

## Features

* Linear time-invariant Gaussian models
* Kalman Filter (KF)
* Unscented Kalman Filter (UKF)
* Extended Kalman Filter (EKF)
* Particle Filter (PF)
* YAML-based configuration
* Monte Carlo batch execution
* SQLite result storage
* Python visualization scripts
* Reproducible runs via deterministic seeding

---

## Project Structure
```
state-est-testbed/
├── CMakeLists.txt
├── README.md
├── configs/
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Config.cpp
│   │   ├── Database.cpp
│   │   ├── EstimatorFactory.cpp
│   │   ├── Experiment.cpp
│   │   ├── KalmanFilter.cpp
│   │   ├── UnscentedKalmanFilter.cpp
│   │   ├── ExtendedKalmanFilter.cpp
│   │   ├── ParticleFilter.cpp
│   │   └── LinearGaussianModel.cpp
├── python/
│   └── compare_estimators.py
└── docs/
    └── SDD.md
```
---

## Quick Start

### 1. Build

mkdir build && cd build
cmake ..
cmake --build . --parallel

### 2. Run a single estimator

./build/state_est_testbed ./configs/test_kf.yaml --tag "my_test"

### 3. Run all estimators

./run_all_estimators.sh

### 4. Compare results

python ./python/compare_estimators.py --tag "my_test"

---

## Available Estimators

Estimator | Type                        | Best For
----------|-----------------------------|------------------------
kf        | Linear Kalman Filter        | Linear Gaussian systems
ukf       | Unscented Kalman Filter     | Mildly nonlinear systems
ekf       | Extended Kalman Filter      | Differentiable nonlinearities
pf        | Particle Filter             | Highly nonlinear / non-Gaussian systems

---

## Configuration Example
```yaml
state_dimension: 2
num_time_steps: 100
dt: 0.1
initial:
  true_state: [0.0, 1.0]
  est_state: [0.0, 0.0]
  initial_cov: 1.0
dynamics:
  state_transition:
    - [1.0, 0.1]
    - [0.0, 1.0]
  process_noise_diag: [0.01, 0.005]
measurement:
  observation_matrix: [[1.0, 0.0]]
  measurement_noise: 0.1
estimator_type: kf
```
---

## Comparison

python ./python/compare_estimators.py --tag "my_test"

This generates plots showing trajectories, errors, and RMSE for all estimators under the same tag.

---

## Development Notes

- All estimator selection logic is centralized in EstimatorFactory.cpp (SDD.md §13)
- New estimators should inherit from Estimator and be added to the factory
- Use --tag <name> when running to group related experiments

Current status: KF, UKF, EKF, and PF are fully functional with proper database logging.

---

## License

MIT

---

## Author

mpeaton