#include "core/Config.hpp"
#include "core/Experiment.hpp"
#include "core/Database.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./state_est_testbed <config.yaml> [--tag <tag>]\n";
        return 1;
    }

    std::string config_path = argv[1];
    std::string experiment_tag = "";

    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--tag" && i + 1 < argc) {
            experiment_tag = argv[i + 1];
            ++i;
        }
    }

    Config config = Config::fromFile(config_path);
    Database db("results.db");

    int experiment_id = db.insertExperiment(config_path);

    std::cout << "Running " << config.estimator_type 
              << " estimator with tag: " << (experiment_tag.empty() ? "(none)" : experiment_tag) << std::endl;

    static int global_run_counter = 0;

    for (int i = 0; i < config.num_runs; ++i) {
        int run_id = db.insertRun(
            experiment_id,
            global_run_counter++,
            config.base_seed + i,
            config.estimator_type,
            experiment_tag
        );

        Experiment experiment(config, &db, run_id);
        experiment.run();
    }

    std::cout << "Completed " << config.num_runs << " run(s) for estimator: " 
              << config.estimator_type << std::endl;
    return 0;
}
