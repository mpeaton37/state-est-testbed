#pragma once
#include "Config.hpp"

class Database;

class Experiment {
public:
    explicit Experiment(const Config& config, Database* db, int run_id);
    void run();

private:
    Config config_;
    Database* db_;
    int run_id_;
};
