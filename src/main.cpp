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

    std::cout << "=== Starting Experiment ===\n";
    std::cout << "Config      : " << config_path << "\n";
    std::cout << "Estimator   : " << config.estimator_type << "\n";
    if (!experiment_tag.empty()) std::cout << "Tag         : " << experiment_tag << "\n";
    std::cout << "Num runs    : " << config.num_runs << "\n\n";

    static int global_run_counter = 0;

    for (int i = 0; i < config.num_runs; ++i) {
        int run_id = db.insertRun(
            experiment_id,
            global_run_counter++,
            config.base_seed + i,
            config.estimator_type,
            experiment_tag
        );

        std::cout << "→ Starting run " << i << " (global run_id = " << run_id << ")\n";

        Experiment experiment(config, &db, run_id);
        experiment.run();
    }

    std::cout << "\nAll runs completed for tag: " << (experiment_tag.empty() ? "(none)" : experiment_tag) << "\n";
    return 0;
}
