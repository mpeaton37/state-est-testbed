#!/usr/bin/env python3
"""
Simple plotting script for state estimation results.
Assumes data is logged to a CSV file with columns: step, true_x, true_y, est_x, est_y
Run with: python plot_results.py results.csv
"""

import sys
import pandas as pd
import matplotlib.pyplot as plt

def main():
    if len(sys.argv) != 2:
        print("Usage: python plot_results.py <results.csv>")
        sys.exit(1)

    csv_file = sys.argv[1]
    data = pd.read_csv(csv_file)

    # Assuming 2D state for simplicity
    steps = data['step']
    true_x = data['true_x']
    true_y = data['true_y']
    est_x = data['est_x']
    est_y = data['est_y']

    fig, axs = plt.subplots(2, 2, figsize=(12, 8))

    # True vs Estimated X
    axs[0, 0].plot(steps, true_x, label='True X', color='blue')
    axs[0, 0].plot(steps, est_x, label='Est X', color='red', linestyle='--')
    axs[0, 0].set_title('X Position')
    axs[0, 0].legend()
    axs[0, 0].grid(True)

    # True vs Estimated Y
    axs[0, 1].plot(steps, true_y, label='True Y', color='blue')
    axs[0, 1].plot(steps, est_y, label='Est Y', color='red', linestyle='--')
    axs[0, 1].set_title('Y Position')
    axs[0, 1].legend()
    axs[0, 1].grid(True)

    # Error in X
    error_x = true_x - est_x
    axs[1, 0].plot(steps, error_x, color='green')
    axs[1, 0].set_title('Error in X')
    axs[1, 0].grid(True)

    # Error in Y
    error_y = true_y - est_y
    axs[1, 1].plot(steps, error_y, color='green')
    axs[1, 1].set_title('Error in Y')
    axs[1, 1].grid(True)

    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    main()
