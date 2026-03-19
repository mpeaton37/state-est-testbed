#!/usr/bin/env python3
"""
Utility module for state estimation analysis in Jupyter notebooks.
Provides functions to load data from SQLite database, compute metrics, and plot results.
"""

import sqlite3
import json
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

def load_run_data(db_file, run_id):
    """
    Load time step data for a specific run from the database.

    Args:
        db_file (str): Path to the SQLite database file.
        run_id (int): The run ID to load.

    Returns:
        dict: Dictionary with 'steps', 'true_states', 'est_states', 'est_covs' as numpy arrays.
    """
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()

    cursor.execute("SELECT step, true_state, est_state, est_cov FROM time_steps WHERE run_id = ? ORDER BY step", (run_id,))
    rows = cursor.fetchall()

    if not rows:
        conn.close()
        raise ValueError(f"No data found for run_id {run_id}")

    steps = []
    true_states = []
    est_states = []
    est_covs = []

    for row in rows:
        step, true_state_json, est_state_json, est_cov_json = row
        steps.append(step)
        true_states.append(json.loads(true_state_json))
        est_states.append(json.loads(est_state_json))
        est_covs.append(json.loads(est_cov_json))

    conn.close()

    return {
        'steps': np.array(steps),
        'true_states': np.array(true_states),
        'est_states': np.array(est_states),
        'est_covs': np.array(est_covs)
    }

def load_rmse(db_file, run_id):
    """
    Load RMSE for a specific run.

    Args:
        db_file (str): Path to the SQLite database file.
        run_id (int): The run ID.

    Returns:
        float: The RMSE value.
    """
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()
    cursor.execute("SELECT rmse FROM summary_stats WHERE run_id = ?", (run_id,))
    row = cursor.fetchone()
    conn.close()
    return row[0] if row else None

def compute_rmse(true_states, est_states):
    """
    Compute RMSE from true and estimated states.

    Args:
        true_states (np.ndarray): True states (n_steps, n_dim).
        est_states (np.ndarray): Estimated states (n_steps, n_dim).

    Returns:
        float: RMSE value.
    """
    errors = true_states - est_states
    mse = np.mean(np.sum(errors**2, axis=1))
    return np.sqrt(mse)

def compute_nees(true_states, est_states, est_covs):
    """
    Compute NEES (Normalized Estimation Error Squared) for each time step.

    Args:
        true_states (np.ndarray): True states (n_steps, n_dim).
        est_states (np.ndarray): Estimated states (n_steps, n_dim).
        est_covs (np.ndarray): Estimated covariances (n_steps, n_dim, n_dim).

    Returns:
        np.ndarray: NEES values (n_steps,).
    """
    n_steps = true_states.shape[0]
    nees = np.zeros(n_steps)
    for k in range(n_steps):
        error = true_states[k] - est_states[k]
        P_inv = np.linalg.inv(est_covs[k])
        nees[k] = error.T @ P_inv @ error
    return nees

def plot_run(data, run_id=None, rmse=None):
    """
    Plot true vs estimated states and errors for a run.

    Args:
        data (dict): Data from load_run_data.
        run_id (int, optional): Run ID for title.
        rmse (float, optional): RMSE for title.
    """
    steps = data['steps']
    true_states = data['true_states']
    est_states = data['est_states']

    fig, axs = plt.subplots(2, 2, figsize=(12, 8))

    n_dim = true_states.shape[1]
    if n_dim >= 2:
        # Plot X and Y
        axs[0, 0].plot(steps, true_states[:, 0], label='True X', color='blue')
        axs[0, 0].plot(steps, est_states[:, 0], label='Est X', color='red', linestyle='--')
        axs[0, 0].set_title('X Position')
        axs[0, 0].legend()
        axs[0, 0].grid(True)

        axs[0, 1].plot(steps, true_states[:, 1], label='True Y', color='blue')
        axs[0, 1].plot(steps, est_states[:, 1], label='Est Y', color='red', linestyle='--')
        axs[0, 1].set_title('Y Position')
        axs[0, 1].legend()
        axs[0, 1].grid(True)

        axs[1, 0].plot(steps, true_states[:, 0] - est_states[:, 0], color='green')
        axs[1, 0].set_title('Error in X')
        axs[1, 0].grid(True)

        axs[1, 1].plot(steps, true_states[:, 1] - est_states[:, 1], color='green')
        axs[1, 1].set_title('Error in Y')
        axs[1, 1].grid(True)
    else:
        # 1D
        axs[0, 0].plot(steps, true_states[:, 0], label='True', color='blue')
        axs[0, 0].plot(steps, est_states[:, 0], label='Est', color='red', linestyle='--')
        axs[0, 0].set_title('State')
        axs[0, 0].legend()
        axs[0, 0].grid(True)

        axs[1, 0].plot(steps, true_states[:, 0] - est_states[:, 0], color='green')
        axs[1, 0].set_title('Error')
        axs[1, 0].grid(True)

        axs[0, 1].axis('off')
        axs[1, 1].axis('off')

    title = 'State Estimation Results'
    if run_id is not None:
        title += f' - Run {run_id}'
    if rmse is not None:
        title += f' - RMSE: {rmse:.4f}'
    fig.suptitle(title)

    plt.tight_layout()
    plt.show()

def load_experiment_runs(db_file, experiment_id):
    """
    Load all run IDs for a given experiment.

    Args:
        db_file (str): Path to the SQLite database file.
        experiment_id (int): The experiment ID.

    Returns:
        list: List of run IDs.
    """
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()
    cursor.execute("SELECT id FROM runs WHERE experiment_id = ?", (experiment_id,))
    rows = cursor.fetchall()
    conn.close()
    return [row[0] for row in rows]

def monte_carlo_summary(db_file, experiment_id):
    """
    Compute Monte Carlo summary statistics for an experiment.

    Args:
        db_file (str): Path to the SQLite database file.
        experiment_id (int): The experiment ID.

    Returns:
        dict: Summary with mean RMSE, std RMSE, etc.
    """
    run_ids = load_experiment_runs(db_file, experiment_id)
    rmses = []
    for run_id in run_ids:
        rmse = load_rmse(db_file, run_id)
        if rmse is not None:
            rmses.append(rmse)
    rmses = np.array(rmses)
    return {
        'mean_rmse': np.mean(rmses),
        'std_rmse': np.std(rmses),
        'min_rmse': np.min(rmses),
        'max_rmse': np.max(rmses),
        'num_runs': len(rmses)
    }
