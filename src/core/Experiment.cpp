#include "Experiment.hpp"
#include "KalmanFilter.hpp"
#include "LinearGaussianModel.hpp"
#include <iostream>

Experiment::Experiment(const Config& config)
: config_(config) {}

void Experiment::run() {
LinearGaussianModel model(config_.F, config_.H);

```
KalmanFilter kf(config_.F, config_.Q, config_.H, config_.R);
kf.init(config_.x0_est, config_.P0);

Eigen::VectorXd x_true = config_.x0_true;

for (int k = 0; k < config_.num_steps; ++k) {
    x_true = model.propagate(x_true);
    Eigen::VectorXd z = model.measure(x_true);

    kf.predict();
    kf.update(z);

    std::cout << "Step " << k
              << " | True: " << x_true.transpose()
              << " | Est: " << kf.state().transpose()
              << std::endl;
}
```

}
