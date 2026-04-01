#include "core/Config.hpp"
#include "core/Experiment.hpp"
#include "core/Database.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./state_est_testbed <config.yaml>\n";
        return 1;
    }

    Config config = Config::fromFile(argv[1]);
    Database db("results.db");
    int experiment_id = db.insertExperiment(argv[1]);

    for (int i = 0; i < config.num_runs; ++i) {
        int run_id = db.insertRun(experiment_id, i, config.base_seed + i, config.estimator_type);
        Experiment experiment(config, &db, run_id);
        experiment.run();
    }

    return 0;
}
