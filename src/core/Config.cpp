#include "Config.hpp"
#include <yaml-cpp/yaml.h>

Config Config::fromFile(const std::string& path) {
YAML::Node node = YAML::LoadFile(path);

```
Config c;
c.state_dim = node["state_dimension"].as<int>();
c.num_steps = node["num_time_steps"].as<int>();
c.dt = node["dt"].as<double>();

c.x0_true = Eigen::VectorXd::Zero(c.state_dim);
c.x0_est  = Eigen::VectorXd::Zero(c.state_dim);
c.P0 = Eigen::MatrixXd::Identity(c.state_dim, c.state_dim);

c.F = Eigen::MatrixXd::Identity(c.state_dim, c.state_dim);
c.Q = Eigen::MatrixXd::Identity(c.state_dim, c.state_dim) * 0.01;
c.H = Eigen::MatrixXd::Identity(1, c.state_dim);
c.R = Eigen::MatrixXd::Identity(1, 1) * 0.1;

return c;
```

}
