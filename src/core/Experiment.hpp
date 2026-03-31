#pragma once
#include "Config.hpp"

class Database;
class EstimatorFactory; // Forward declaration

class Experiment {
public:
    explicit Experiment(const Config& config, Database* db, int run_id);
    void run();

private:
    Config config_;
    Database* db_;
    int run_id_;
    EstimatorFactory* estimator_factory_; // Declaration of EstimatorFactory pointer
};
