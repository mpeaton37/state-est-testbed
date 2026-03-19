#include "Database.hpp"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <iomanip>

Database::Database(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), reinterpret_cast<sqlite3**>(&db_)) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }
    createTables();
}

Database::~Database() {
    sqlite3_close(reinterpret_cast<sqlite3*>(db_));
}

void Database::createTables() {
    const char* sql_experiments = R"(
        CREATE TABLE IF NOT EXISTS experiments (
            id INTEGER PRIMARY KEY,
            config_path TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    const char* sql_runs = R"(
        CREATE TABLE IF NOT EXISTS runs (
            id INTEGER PRIMARY KEY,
            experiment_id INTEGER,
            run_id INTEGER,
            seed INTEGER,
            FOREIGN KEY (experiment_id) REFERENCES experiments(id)
        );
    )";

    const char* sql_time_steps = R"(
        CREATE TABLE IF NOT EXISTS time_steps (
            run_id INTEGER,
            step INTEGER,
            true_state TEXT,
            est_state TEXT,
            est_cov TEXT,
            PRIMARY KEY (run_id, step),
            FOREIGN KEY (run_id) REFERENCES runs(id)
        );
    )";

    const char* sql_summary_stats = R"(
        CREATE TABLE IF NOT EXISTS summary_stats (
            run_id INTEGER PRIMARY KEY,
            rmse REAL,
            FOREIGN KEY (run_id) REFERENCES runs(id)
        );
    )";

    char* err_msg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(db_), sql_experiments, nullptr, nullptr, &err_msg);
    if (err_msg) { std::cerr << "Error creating experiments table: " << err_msg << std::endl; sqlite3_free(err_msg); }
    sqlite3_exec(reinterpret_cast<sqlite3*>(db_), sql_runs, nullptr, nullptr, &err_msg);
    if (err_msg) { std::cerr << "Error creating runs table: " << err_msg << std::endl; sqlite3_free(err_msg); }
    sqlite3_exec(reinterpret_cast<sqlite3*>(db_), sql_time_steps, nullptr, nullptr, &err_msg);
    if (err_msg) { std::cerr << "Error creating time_steps table: " << err_msg << std::endl; sqlite3_free(err_msg); }
    sqlite3_exec(reinterpret_cast<sqlite3*>(db_), sql_summary_stats, nullptr, nullptr, &err_msg);
    if (err_msg) { std::cerr << "Error creating summary_stats table: " << err_msg << std::endl; sqlite3_free(err_msg); }
}

int Database::insertExperiment(const std::string& config_path) {
    std::string sql = "INSERT INTO experiments (config_path) VALUES (?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db_), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, config_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    int id = sqlite3_last_insert_rowid(reinterpret_cast<sqlite3*>(db_));
    sqlite3_finalize(stmt);
    return id;
}

int Database::insertRun(int experiment_id, int run_id, int seed) {
    std::string sql = "INSERT INTO runs (experiment_id, run_id, seed) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db_), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, experiment_id);
    sqlite3_bind_int(stmt, 2, run_id);
    sqlite3_bind_int(stmt, 3, seed);
    sqlite3_step(stmt);
    int id = sqlite3_last_insert_rowid(reinterpret_cast<sqlite3*>(db_));
    sqlite3_finalize(stmt);
    return id;
}

void Database::insertTimeStep(int run_id, int step, const Eigen::VectorXd& true_state, const Eigen::VectorXd& est_state, const Eigen::MatrixXd& est_cov) {
    std::string sql = "INSERT INTO time_steps (run_id, step, true_state, est_state, est_cov) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db_), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, run_id);
    sqlite3_bind_int(stmt, 2, step);
    sqlite3_bind_text(stmt, 3, serializeVector(true_state).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, serializeVector(est_state).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, serializeMatrix(est_cov).c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void Database::insertSummary(int run_id, double rmse) {
    std::string sql = "INSERT INTO summary_stats (run_id, rmse) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db_), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, run_id);
    sqlite3_bind_double(stmt, 2, rmse);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::string Database::serializeVector(const Eigen::VectorXd& vec) const {
    std::stringstream ss;
    ss << "[";
    for (int i = 0; i < vec.size(); ++i) {
        ss << std::fixed << std::setprecision(6) << vec(i);
        if (i < vec.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

std::string Database::serializeMatrix(const Eigen::MatrixXd& mat) const {
    std::stringstream ss;
    ss << "[";
    for (int i = 0; i < mat.rows(); ++i) {
        ss << "[";
        for (int j = 0; j < mat.cols(); ++j) {
            ss << std::fixed << std::setprecision(6) << mat(i, j);
            if (j < mat.cols() - 1) ss << ",";
        }
        ss << "]";
        if (i < mat.rows() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}
