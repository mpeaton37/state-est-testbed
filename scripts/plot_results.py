#!/usr/bin/env python3
"""
Plotting script for state estimation results from SQLite database.
Assumes database has tables: experiments, runs, time_steps, summary_stats.
Run with: python plot_results.py results.db <run_id>
"""

import sys
import sqlite3
import json
import numpy as np
import matplotlib.pyplot as plt

def main():
    if len(sys.argv) != 3:
        print("Usage: python plot_results.py <results.db> <run_id>")
        sys.exit(1)

    db_file = sys.argv[1]
    run_id = int(sys.argv[2])

    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()

    # Query time_steps for the run
    cursor.execute("SELECT step, true_state, est_state FROM time_steps WHERE run_id = ? ORDER BY step", (run_id,))
    rows = cursor.fetchall()

    if not rows:
        print(f"No data found for run_id {run_id}")
        sys.exit(1)

    steps = []
    true_states = []
    est_states = []

    for row in rows:
        step, true_state_json, est_state_json = row
        steps.append(step)
        true_states.append(json.loads(true_state_json))
        est_states.append(json.loads(est_state_json))

    # Convert to numpy arrays
    true_states = np.array(true_states)
    est_states = np.array(est_states)

    # Query RMSE
    cursor.execute("SELECT rmse FROM summary_stats WHERE run_id = ?", (run_id,))
    rmse_row = cursor.fetchone()
    rmse = rmse_row[0] if rmse_row else None

    conn.close()

    # Plot
    fig, axs = plt.subplots(2, 2, figsize=(12, 8))

    # Assuming 2D state; adjust if needed
    if true_states.shape[1] >= 2:
        # True vs Estimated X
        axs[0, 0].plot(steps, true_states[:, 0], label='True X', color='blue')
        axs[0, 0].plot(steps, est_states[:, 0], label='Est X', color='red', linestyle='--')
        axs[0, 0].set_title('X Position')
        axs[0, 0].legend()
        axs[0, 0].grid(True)

        # True vs Estimated Y
        axs[0, 1].plot(steps, true_states[:, 1], label='True Y', color='blue')
        axs[0, 1].plot(steps, est_states[:, 1], label='Est Y', color='red', linestyle='--')
        axs[0, 1].set_title('Y Position')
        axs[0, 1].legend()
        axs[0, 1].grid(True)

        # Error in X
        error_x = true_states[:, 0] - est_states[:, 0]
        axs[1, 0].plot(steps, error_x, color='green')
        axs[1, 0].set_title('Error in X')
        axs[1, 0].grid(True)

        # Error in Y
        error_y = true_states[:, 1] - est_states[:, 1]
        axs[1, 1].plot(steps, error_y, color='green')
        axs[1, 1].set_title('Error in Y')
        axs[1, 1].grid(True)
    else:
        # 1D case
        axs[0, 0].plot(steps, true_states[:, 0], label='True', color='blue')
        axs[0, 0].plot(steps, est_states[:, 0], label='Est', color='red', linestyle='--')
        axs[0, 0].set_title('State')
        axs[0, 0].legend()
        axs[0, 0].grid(True)

        error = true_states[:, 0] - est_states[:, 0]
        axs[1, 0].plot(steps, error, color='green')
        axs[1, 0].set_title('Error')
        axs[1, 0].grid(True)

        # Hide unused subplots
        axs[0, 1].axis('off')
        axs[1, 1].axis('off')

    if rmse is not None:
        fig.suptitle(f'Run {run_id} - RMSE: {rmse:.4f}')

    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    main()
