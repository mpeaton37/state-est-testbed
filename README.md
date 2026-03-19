# State Estimation Testbed

A modular, simulation-based C++ framework for developing and evaluating state estimation algorithms, starting with the linear Kalman Filter (KF).

---

## 🚀 Overview

This project provides a reproducible, extensible environment for:

* Simulating linear dynamical systems
* Running Kalman Filter-based estimators
* Performing Monte Carlo experiments
* Evaluating statistical consistency (RMSE, NEES)
* Visualizing results via Python tools

Designed for rapid experimentation and future extension to nonlinear filters (EKF, UKF, particle filters).

---

## ✨ Features (MVP)

* Linear time-invariant Gaussian models
* Kalman Filter (1D, 2D, 3D, extensible to N-D via Eigen)
* YAML-based configuration
* Monte Carlo batch execution
* SQLite result storage + CSV export
* Python visualization scripts (matplotlib, seaborn)
* Reproducible runs via deterministic seeding
* Unit & integration tests (GoogleTest)

---

## 🧱 Project Structure

```
state-est-testbed/
├── CMakeLists.txt
├── environment.yml
├── README.md
├── configs/
├── src/
│   ├── main.cpp
│   ├── core/
│   ├── utils/
├── tests/
├── scripts/
└── docs/
    └── SDD.md
```

---

## ⚙️ Setup

### 1. Create Environment

```bash
conda env create -f environment.yml
conda activate state-est
```

### 2. Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

---

## ▶️ Running an Experiment

```bash
./state_est_testbed ../configs/example.yaml
```

---

## 📊 Output

* SQLite database (`results.db`)
* Optional CSV per run
* Python plots:

  * Trajectories (true vs estimated)
  * Error + ±3σ bounds
  * RMSE and NEES statistics

---

## 📈 Metrics

* **RMSE** — Root Mean Square Error
* **NEES** — Normalized Estimation Error Squared

  * Used for consistency validation
  * Expected mean ≈ state dimension

---

## 🧪 Testing

### Running Tests

After building the project, run the full test suite with:

```bash
ctest
```

For more control:

```bash
ctest --verbose                    # Show detailed test output
ctest -R KalmanFilterTest          # Run only KalmanFilter unit tests
ctest -R IntegrationTest           # Run the end-to-end integration test
ctest -N                           # List all available tests
```

To run the unit test executable directly:

```bash
./build/test_kalman_filter
```

**Current test coverage includes:**
* KalmanFilter predict/update logic
* Basic state and covariance checks
* Integration test that runs the full pipeline with `configs/test.yaml`

To improve coverage further, additional tests for `Config`, `Database`, and `Experiment` are recommended.

---

## 🔧 Configuration Example

```yaml
state_dimension: 2
num_time_steps: 300
dt: 0.05

initial:
  true_state: [0.0, 5.0]
  est_state:  [0.0, 0.0]
  initial_cov: 100.0

dynamics:
  state_transition: [[1.0, 0.05], [0.0, 1.0]]
  process_noise_diag: [0.0025, 0.0001]

measurement:
  observation_matrix: [1.0, 0.0]
  measurement_noise: 0.25

monte_carlo:
  runs: 200
  base_seed: 42
```

---

## 🧩 Extensibility

New estimators can be added by implementing:

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

Planned extensions:

* Extended Kalman Filter (EKF)
* Unscented Kalman Filter (UKF)
* Particle filters
* Parallel Monte Carlo execution

---

## ⚠️ Limitations (MVP)

* Linear systems only
* No real-time constraints
* No GUI
* Single-threaded execution

---

## 📚 Documentation

See [`docs/SDD.md`](docs/SDD.md) for full system design.

---

## 📄 License

MIT (or TBD)

---

## 👤 Author

mpeaton

