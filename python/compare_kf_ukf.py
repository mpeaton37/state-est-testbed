"""
compare_kf_ukf.py
Comparison of Linear Kalman Filter vs Unscented Kalman Filter

This is a testbed and R&D project for estimators and predictors.
Software description document is SDD.md.

Purpose (per SDD.md §4 Metrics & §13 Future Work):
- Load results from two separate Monte-Carlo runs stored in results.db
  (one run using estimator_type="kf", one using estimator_type="ukf")
- Compute and visualize RMSE, state trajectories, and estimation errors
- Highlight performance differences on the same ground-truth trajectory
"""

import argparse
import sqlite3
import json
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


def load_run_data(db_path: str, run_id: int):
    """Load true_state, est_state, and est_cov from the testbed database for a given run."""
    conn = sqlite3.connect(db_path)
    query = """
        SELECT step, true_state, est_state, est_cov
        FROM time_steps
        WHERE run_id = ?
        ORDER BY step
    """
    df = pd.read_sql_query(query, conn, params=(run_id,))
    conn.close()

    # JSON strings → numpy arrays
    df["true_state"] = df["true_state"].apply(json.loads)
    df["est_state"] = df["est_state"].apply(json.loads)
    df["est_cov"] = df["est_cov"].apply(json.loads)

    true = np.array(df["true_state"].tolist())
    est = np.array(df["est_state"].tolist())
    cov = np.array(df["est_cov"].tolist())  # shape (n_steps, state_dim, state_dim)

    return df["step"].values, true, est, cov


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Compare Linear KF vs UKF results from the state-estimation testbed (SDD.md)"
    )
    parser.add_argument("--kf_run", type=int, required=True, help="Run ID for Linear Kalman Filter (estimator_type=kf)")
    parser.add_argument("--ukf_run", type=int, required=True, help="Run ID for Unscented Kalman Filter (estimator_type=ukf)")
    parser.add_argument("--db", default="results.db", help="Path to SQLite database (default: results.db)")
    args = parser.parse_args()

    # Load both runs
    steps_kf, true_kf, est_kf, _ = load_run_data(args.db, args.kf_run)
    steps_ukf, true_ukf, est_ukf, _ = load_run_data(args.db, args.ukf_run)

    # Basic sanity check
    if not np.array_equal(steps_kf, steps_ukf):
        print("WARNING: Time steps differ between runs – using KF steps for plotting.")

    # Compute RMSE (per SDD.md §4)
    rmse_kf = np.sqrt(np.mean(np.sum((true_kf - est_kf) ** 2, axis=1)))
    rmse_ukf = np.sqrt(np.mean(np.sum((true_ukf - est_ukf) ** 2, axis=1)))

    print("=== KF vs UKF Performance Comparison (SDD.md testbed metrics) ===")
    print(f"RMSE (KF)  : {rmse_kf:.4f}")
    print(f"RMSE (UKF) : {rmse_ukf:.4f}")
    print(f"Improvement: {100 * (rmse_kf - rmse_ukf) / rmse_kf if rmse_kf > 0 else 0:.1f}%")

    # ─────────────────────────────────────────────────────────────────────
    # Visualization (state trajectories, errors, RMSE bar)
    n_states = true_kf.shape[1]
    fig, axs = plt.subplots(n_states + 2, 1, figsize=(12, 3 * (n_states + 2)), sharex=True)

    # State plots
    for i in range(n_states):
        axs[i].plot(steps_kf, true_kf[:, i], "k-", linewidth=2, label="True state")
        axs[i].plot(steps_kf, est_kf[:, i], "b--", label="Linear KF estimate")
        axs[i].plot(steps_ukf, est_ukf[:, i], "r-.", label="Unscented KF estimate")
        axs[i].set_ylabel(f"State {i}")
        axs[i].grid(True)
        axs[i].legend()

    # Absolute error (summed over states)
    err_kf = np.sum(np.abs(true_kf - est_kf), axis=1)
    err_ukf = np.sum(np.abs(true_ukf - est_ukf), axis=1)
    axs[-2].plot(steps_kf, err_kf, "b--", label="KF error")
    axs[-2].plot(steps_ukf, err_ukf, "r-.", label="UKF error")
    axs[-2].set_ylabel("Summed abs. error")
    axs[-2].grid(True)
    axs[-2].legend()

    # RMSE bar chart
    axs[-1].bar(["Linear KF", "Unscented KF"], [rmse_kf, rmse_ukf], color=["blue", "red"])
    axs[-1].set_ylabel("RMSE")
    axs[-1].set_title("RMSE Comparison (lower is better)")

    plt.suptitle(f"KF vs UKF Comparison – Run KF:{args.kf_run} | UKF:{args.ukf_run} (SDD.md testbed)")
    plt.xlabel("Time step")
    plt.tight_layout()
    plt.show()

    print("\nPlot generated. This script is ready for any pair of KF/UKF runs produced by the testbed.")
    print("Extend EstimatorFactory::create() to support estimator_type='ukf' (see UnscentedKalmanFilter) to populate UKF data.")