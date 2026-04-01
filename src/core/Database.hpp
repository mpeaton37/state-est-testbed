#pragma once
#include <Eigen/Dense>
#include <string>
#include <vector>

class Database {
public:
    Database(const std::string& db_path);
    ~Database();

    void createTables();
    int insertExperiment(const std::string& config_path);
  int insertRun(int experiment_id, int run_id, int seed, const std::string& estimator_type = "unknown");
    void insertTimeStep(int run_id, int step,
                        const Eigen::VectorXd& true_state,
                        const Eigen::VectorXd& est_state,
                        const Eigen::MatrixXd& est_cov,
                        const std::string& model_probs_json = "null",
                        const std::string& hypotheses_json = "null");
    void insertSummary(int run_id, double rmse);

private:
    std::string serializeVector(const Eigen::VectorXd& vec) const;
    std::string serializeMatrix(const Eigen::MatrixXd& mat) const;
    void* db_;  // sqlite3* db;
};
