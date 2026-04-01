"""
compare_estimators.py
Comparison of KF, UKF, EKF, and Particle Filter

This is a testbed and R&D project for estimators and predictors.
Software description document is SDD.md.
"""

import argparse
import sqlite3
import json
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def load_run(db_path: str, run_id: int):
    """Load data for a single run from the database."""
    conn = sqlite3.connect(db_path)
    df = pd.read_sql_query("""
        SELECT step, true_state, est_state, est_cov
        FROM time_steps 
        WHERE run_id = ? 
        ORDER BY step
    """, conn, params=(run_id,))
    conn.close()

    df["true_state"] = df["true_state"].apply(json.loads)
    df["est_state"]  = df["est_state"].apply(json.loads)

    true = np.array(df["true_state"].tolist())
    est  = np.array(df["est_state"].tolist())

    return df["step"].values, true, est


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare KF, UKF, EKF, PF (SDD.md testbed)")
    parser.add_argument("--kf",  type=int, help="Run ID for Kalman Filter")
    parser.add_argument("--ukf", type=int, help="Run ID for Unscented Kalman Filter")
    parser.add_argument("--ekf", type=int, help="Run ID for Extended Kalman Filter")
    parser.add_argument("--pf",  type=int, help="Run ID for Particle Filter")
    parser.add_argument("--db", default="results.db", help="Database path")
    args = parser.parse_args()

    runs = {}
    labels = {}
    if args.kf is not None:
        runs["kf"] = load_run(args.db, args.kf)
        labels["kf"] = "KF"
    if args.ukf is not None:
        runs["ukf"] = load_run(args.db, args.ukf)
        labels["ukf"] = "UKF"
    if args.ekf is not None:
        runs["ekf"] = load_run(args.db, args.ekf)
        labels["ekf"] = "EKF"
    if args.pf is not None:
        runs["pf"] = load_run(args.db, args.pf)
        labels["pf"] = "PF"

    if not runs:
        print("Error: Provide at least one run ID with --kf, --ukf, --ekf, or --pf")
        exit(1)

    # Compute RMSE for each
    rmse = {}
    for name, (steps, true, est) in runs.items():
        rmse[name] = np.sqrt(np.mean(np.sum((true - est)**2, axis=1)))

    print("=== Estimator Comparison (SDD.md testbed) ===")
    for name, r in rmse.items():
        print(f"{labels[name]:>4s} RMSE: {r:.4f}")

    # Plotting
    n_states = next(iter(runs.values()))[1].shape[1]   # number of state dimensions
    fig, axs = plt.subplots(n_states + 2, 1, figsize=(14, 4*(n_states + 2)), sharex=True)

    colors = {"kf": "blue", "ukf": "green", "ekf": "orange", "pf": "red"}

    # State plots
    for i in range(n_states):
        ax = axs[i]
        steps, true, _ = next(iter(runs.values()))
        ax.plot(steps, true[:, i], 'k-', linewidth=2.5, label="True State")

        for name, (steps, _, est) in runs.items():
            ax.plot(steps, est[:, i], '--', color=colors[name], label=labels[name])

        ax.set_ylabel(f"State {i}")
        ax.grid(True, alpha=0.3)
        ax.legend()

    # Absolute error (summed)
    ax_err = axs[-2]
    for name, (steps, true, est) in runs.items():
        err = np.sum(np.abs(true - est), axis=1)
        ax_err.plot(steps, err, '--', color=colors[name], label=f"{labels[name]} (RMSE={rmse[name]:.3f})")

    ax_err.set_ylabel("Summed Absolute Error")
    ax_err.grid(True, alpha=0.3)
    ax_err.legend()

    # RMSE bar chart
    ax_bar = axs[-1]
    names = list(rmse.keys())
    values = [rmse[n] for n in names]
    bars = ax_bar.bar([labels[n] for n in names], values, color=[colors[n] for n in names])
    ax_bar.set_ylabel("RMSE")
    ax_bar.set_title("RMSE Comparison (lower is better)")
    ax_bar.grid(True, axis='y', alpha=0.3)

    # Annotate bars
    for bar in bars:
        height = bar.get_height()
        ax_bar.text(bar.get_x() + bar.get_width()/2., height + 0.01,
                    f'{height:.3f}', ha='center', va='bottom')

    plt.suptitle("Estimator Comparison: KF vs UKF vs EKF vs PF\nSDD.md State Estimation Testbed", fontsize=14)
    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.show()

    print("\nPlot generated. Use --kf, --ukf, --ekf, --pf flags to select runs.")