"""
compare_estimators.py
Compare multiple estimators using experiment_tag (SDD.md testbed)

Usage:
    python compare_estimators.py --tag "linear_case_v1"
    python compare_estimators.py --tag "maneuver_test"
"""

import argparse
import sqlite3
import json
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def load_run(db_path: str, run_id: int):
    """Load data for a single run."""
    conn = sqlite3.connect(db_path)
    df = pd.read_sql_query("""
        SELECT step, true_state, est_state 
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


def get_runs_by_tag(db_path: str, tag: str):
    """Get all run_ids that match a given experiment_tag."""
    conn = sqlite3.connect(db_path)
    df = pd.read_sql_query("""
        SELECT run_id, estimator_type 
        FROM runs 
        WHERE experiment_tag = ?
        ORDER BY run_id
    """, conn, params=(tag,))
    conn.close()
    return df


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare estimators by experiment tag (SDD.md testbed)")
    parser.add_argument("--tag", type=str, required=True, 
                        help="Experiment tag to compare (e.g. 'linear_case_v1')")
    parser.add_argument("--db", default="results.db", help="Database path")
    args = parser.parse_args()

    # Get all runs with this tag
    runs_df = get_runs_by_tag(args.db, args.tag)

    if runs_df.empty:
        print(f"Error: No runs found with tag '{args.tag}'")
        print("Make sure you ran the estimators with --tag", args.tag)
        exit(1)

    print(f"Found {len(runs_df)} runs with tag '{args.tag}':")
    for _, row in runs_df.iterrows():
        print(f"  Run {row['run_id']:2d} → {row['estimator_type']}")

    # Load data for each run
    runs = {}
    labels = {}
    colors = {"kf": "blue", "ukf": "green", "ekf": "orange", "pf": "red"}

    for _, row in runs_df.iterrows():
        name = row['estimator_type']
        run_id = row['run_id']
        steps, true, est = load_run(args.db, run_id)
        
        runs[name] = (steps, true, est)
        labels[name] = name.upper()

    # Compute RMSE
    rmse = {}
    for name, (_, true, est) in runs.items():
        rmse[name] = np.sqrt(np.mean(np.sum((true - est)**2, axis=1)))

    print("\n=== Estimator Comparison Results ===")
    for name, r in sorted(rmse.items(), key=lambda x: x[1]):
        print(f"{labels[name]:>4s} RMSE: {r:.4f}")

    # Plotting
    n_states = next(iter(runs.values()))[1].shape[1]
    fig, axs = plt.subplots(n_states + 2, 1, figsize=(14, 4 * (n_states + 2)), sharex=True)

    # State plots
    for i in range(n_states):
        ax = axs[i]
        steps, true, _ = next(iter(runs.values()))
        ax.plot(steps, true[:, i], 'k-', linewidth=2.5, label="True State")

        for name, (_, _, est) in runs.items():
            ax.plot(steps, est[:, i], '--', color=colors.get(name, "gray"), 
                    label=f"{labels[name]} (RMSE={rmse[name]:.3f})")

        ax.set_ylabel(f"State {i}")
        ax.grid(True, alpha=0.3)
        ax.legend()

    # Error plot
    ax_err = axs[-2]
    for name, (steps, true, est) in runs.items():
        err = np.sum(np.abs(true - est), axis=1)
        ax_err.plot(steps, err, '--', color=colors.get(name, "gray"),
                    label=f"{labels[name]} (RMSE={rmse[name]:.3f})")

    ax_err.set_ylabel("Summed Absolute Error")
    ax_err.grid(True, alpha=0.3)
    ax_err.legend()

    # RMSE bar chart
    ax_bar = axs[-1]
    names = list(rmse.keys())
    values = [rmse[n] for n in names]
    bars = ax_bar.bar([labels[n] for n in names], values, 
                      color=[colors.get(n, "gray") for n in names])
    ax_bar.set_ylabel("RMSE")
    ax_bar.set_title(f"RMSE Comparison - Tag: {args.tag}")
    ax_bar.grid(True, axis='y', alpha=0.3)

    for bar in bars:
        height = bar.get_height()
        ax_bar.text(bar.get_x() + bar.get_width()/2., height + 0.005,
                    f'{height:.3f}', ha='center', va='bottom', fontsize=10)

    plt.suptitle(f"Estimator Comparison\nExperiment Tag: {args.tag}", fontsize=14)
    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.show()

    print(f"\nPlot generated for tag '{args.tag}'.")