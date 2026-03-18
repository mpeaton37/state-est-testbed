# Software Design Document (SDD)

## Simulation-Based Testbed for State Estimation Algorithms

**Version:** 1.0
**Date:** March 18, 2026

---

## 1. Introduction

### 1.1 Purpose

This document defines the architecture and design of a modular C++ simulation framework for evaluating state estimation algorithms.

The system supports:

* Linear Kalman Filter experimentation
* Monte Carlo statistical evaluation
* Reproducible, configuration-driven runs

---

### 1.2 Scope

#### In Scope (MVP)

* Linear Gaussian systems
* Discrete-time Kalman Filter
* State dimension: N ≥ 1 (Eigen-based)
* Monte Carlo experiments
* SQLite + CSV output
* Python visualization

#### Out of Scope

* Nonlinear filters (EKF, UKF)
* Real sensor data
* Real-time systems
* GUI / cloud deployment

---

## 2. System Overview

### Execution Flow

```
main
 └── BatchRunner
      └── Experiment (per run)
            ├── propagate truth
            ├── predict
            ├── update
            └── log results
```

---

### Data Flow

```
YAML → Config → Experiment → Simulation Loop → Database → Python Plots
```

---

## 3. Mathematical Model

### State Space Model

* State transition:
  x = F x₋₁ + w
* Measurement:
  z = H x + v

Where:

* w ~ N(0, Q)
* v ~ N(0, R)

---

### Matrix Dimensions

| Symbol | Dimension |
| ------ | --------- |
| F      | n × n     |
| H      | m × n     |
| Q      | n × n     |
| R      | m × m     |
| x      | n × 1     |

---

## 4. Metrics

### RMSE

RMSE = sqrt(mean(||x_true - x_est||²))

---

### NEES

NEES = eᵀ P⁻¹ e

Expected:

* E[NEES] = n
* Used with χ² bounds for consistency

---

## 5. Architecture

### Components

* Config Parser
* Dynamics Model
* Estimator
* Experiment Runner
* Batch Runner
* Database Layer
* Visualization Scripts

---

### Key Interfaces

#### Estimator

```cpp
class Estimator {
public:
    virtual ~Estimator() = default;
    virtual void predict(const Eigen::VectorXd& u = {}) = 0;
    virtual void update(const Eigen::VectorXd& z) = 0;
    virtual Eigen::VectorXd state() const = 0;
    virtual Eigen::MatrixXd covariance() const = 0;
};
```

---

## 6. Configuration

### Validation Rules

* state_dimension > 0
* Matrix dimensions must match
* Q, R, P₀ must be positive definite

---

### Seeding Strategy

```
seed_run_i = base_seed + i
```

---

## 7. Database Design

### Key Design Choice

Vectors stored as JSON arrays:

```json
[1.0, 2.0, 3.0]
```

---

### Tables

* experiments
* runs
* time_steps
* summary_stats

Indexes:

* (run_id, step)

---

## 8. Noise Model

* Gaussian noise via `std::normal_distribution`
* Independent process and measurement noise

---

## 9. Performance Considerations

* KF complexity: O(n³)
* SQLite may become bottleneck at high MC counts
* Future: parallel execution

---

## 10. Logging

Levels:

* INFO — progress
* DEBUG — per-step
* ERROR — failures

---

## 11. Testing Strategy

* Unit tests
* Integration tests
* Statistical validation (NEES consistency)

---

## 12. Risks

* Numerical instability (covariance divergence)
* Poor tuning of Q/R
* Ill-conditioned matrices
* Data volume growth in DB

---

## 13. Future Work

* EKF / UKF support
* Parallel Monte Carlo
* Real data ingestion
* GUI dashboard

---

## 14. Implementation Plan

1. Config system
2. Dynamics + KF
3. End-to-end run
4. Database
5. Batch runner
6. Visualization
7. Testing

---

## 15. Non-Goals

* Production deployment
* Real-time systems
* Hardware integration

---

**End of Document**

