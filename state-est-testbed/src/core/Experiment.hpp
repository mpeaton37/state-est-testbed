#pragma once
#include "Config.hpp"

class Experiment {
public:
explicit Experiment(const Config& config);
void run();

private:
Config config_;
};
