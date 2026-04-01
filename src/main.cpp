#include "core/Config.hpp"
#include "core/Experiment.hpp"
#include "core/Database.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./state_est_testbed <config.yaml> [--tag <experiment_tag>]\n";
        std::cerr << "Example: ./state_est_testbed configs/test_pf.yaml --tag linear_case_v1\n";
        return 1;
    }

    std::string config_path = argv[1];
    std::string experiment_tag = "";

    // Simple command-line parsing for --tag
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
    std::cout << "Config: " << config_path << "\n";
    std::cout << "Estimator: " << config.estimator_type << "\n";
    if (!experiment_tag.empty()) {
        std::cout << "Tag: " << experiment_tag << "\n";
    }
    std::cout << "Running " << config.num_runs << " Monte Carlo run(s)\n\n";

    for (int i = 0; i < config.num_runs; ++i) {
        int run_id = db.insertRun(
            experiment_id, 
            i, 
            config.base_seed + i, 
            config.estimator_type,
            experiment_tag
        );

        Experiment experiment(config, &db, run_id);
        experiment.run();

        std::cout << "Completed run " << i 
                  << " (run_id=" << run_id 
                  << ", estimator=" << config.estimator_type << ")\n";
    }

    std::cout << "\nAll runs completed. Results saved to results.db\n";
    if (!experiment_tag.empty()) {
        std::cout << "Use tag '" << experiment_tag << "' in compare_estimators.py\n";
    }

    return 0;
}