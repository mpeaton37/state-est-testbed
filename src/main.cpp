#include "core/Config.hpp"
#include "core/Experiment.hpp"
#include <iostream>

int main(int argc, char** argv) {
if (argc < 2) {
std::cerr << "Usage: ./state_est_testbed <config.yaml>\n";
return 1;
}

Config config = Config::fromFile(argv[1]);
Experiment experiment(config);
experiment.run();

}
