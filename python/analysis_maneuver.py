# analysis_maneuver.py   or put this in a notebook
"""
Quick performance analysis of a linear KF on the simple.py trajectory
"""

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# ─── Adjust these paths ────────────────────────────────────────────────
import sys
sys.path.insert(0, "python")           # if not using pip install -e .
import stateest

DATA_PATH = "maneuver_kalman_test_data.csv"

# ─── Load data ─────────────────────────────────────────────────────────
df = pd.read_csv(DATA_PATH)

t = df["time_s"].values
z_range = df["meas_range_m"].values
z_az    = df["meas_az_rad"].values
z_el    = df["meas_el_rad"].values
z_rr    = df["meas_rr_mps"].values

# True states for comparison
x_true = df[["true_x_m", "true_y_m", "true_z_m"]].values      # (n,3)
v_true = df[["true_vx_mps", "true_vy_mps", "true_vz_mps"]].values

# Full 6-D truth state for NEES (position + velocity)
x_true_full = np.hstack([x_true, v_true])                     # (n,6)

n_steps = len(t)

# ─── Choose coordinate system for filter ───────────────────────────────
# Most common choice for radar: Cartesian (px, py, pz, vx, vy, vz)
# State = [px, py, pz, vx, vy, vz]
dim_x = 6

# Very simple near-constant-velocity model (you'll tune Q later)
dt = np.mean(np.diff(t))  # average dt (assume roughly constant)
F = np.eye(6)
F[0:3, 3:6] = dt * np.eye(3)

Qpos = 0.5      # m²/s⁴   process noise on acceleration (tune!)
Q = np.zeros((6,6))
Q[3:6, 3:6] = Qpos * np.eye(3)

# Measurement model: range, azimuth, elevation, range-rate
# Nonlinear → for linear KF we need to linearize or use EKF
# For quick start → let's do a simple linear approximation or use EKF if you have it

# ─── Placeholder: very basic linear KF in polar-ish frame ──────────────
# (not accurate — just to get something running; replace with proper model)

# For demonstration we'll pretend we observe [x,y,z,vx,vy,vz] directly
# (obviously wrong — replace with proper H / nonlinear measurement function)
H = np.eye(6)           # ← placeholder !!!
R = np.diag([15**2, 15**2, 15**2, 1.5**2, 0.1**2, 0.1**2])   # rough
B = np.zeros((6, 1))    # no control input

kf = stateest.KalmanFilter(F, Q, H, R, B)     # assuming your bindings accept np arrays

# Initial guess (can be noisy / uncertain)
x0 = np.array([800000., 0., 80000., -5500., 0., -30.])
P0 = np.diag([1000**2]*3 + [200**2]*3)

kf.init(x0, P0)

# ─── Run filter ────────────────────────────────────────────────────────
estimates = np.zeros((n_steps, dim_x))
covs = np.zeros((n_steps, dim_x, dim_x))
nees = np.zeros(n_steps)

for i in range(n_steps):
    # For linear case: predict + update
    kf.predict()               # no control → empty u

    # Fake measurement vector (replace with real conversion from range/az/el/rr)
    # This is just to get the loop running — you'll need proper measurement model
    z = np.concatenate([x_true[i], v_true[i]]) + np.random.randn(6)*np.sqrt(np.diag(R))

    kf.update(z)

    estimates[i] = kf.state
    covs[i] = kf.covariance

    # NEES (only if P invertible)
    err = estimates[i] - x_true_full[i]
    try:
        nees[i] = err @ np.linalg.solve(covs[i], err)
    except np.linalg.LinAlgError:
        nees[i] = np.nan

# ─── Plotting ──────────────────────────────────────────────────────────
fig, axs = plt.subplots(3, 1, figsize=(12, 10), sharex=True)

# Position
for j, coord in enumerate(['x', 'y', 'z']):
    axs[0].plot(t, x_true[:,j]/1000, label=f"true {coord}", lw=2)
    axs[0].plot(t, estimates[:,j]/1000, label=f"est {coord}", ls="--")
    axs[0].fill_between(t, (estimates[:,j]-3*np.sqrt(covs[:,j,j]))/1000,
                           (estimates[:,j]+3*np.sqrt(covs[:,j,j]))/1000,
                           alpha=0.15, label=f"±3σ {coord}")
axs[0].set_ylabel("Position [km]")
axs[0].legend()
axs[0].grid(True)

# Velocity
for j, coord in enumerate(['vx', 'vy', 'vz']):
    axs[1].plot(t, v_true[:,j], label=f"true {coord}")
    axs[1].plot(t, estimates[:,3+j], label=f"est {coord}", ls="--")
axs[1].set_ylabel("Velocity [m/s]")
axs[1].legend()
axs[1].grid(True)

# NEES
axs[2].plot(t, nees, label="NEES")
axs[2].axhline(6, color='gray', ls='--', label="expected value (dim=6)")
axs[2].set_ylabel("NEES")
axs[2].legend()
axs[2].grid(True)

axs[-1].set_xlabel("Time [s]")
plt.tight_layout()
plt.show()

print("Position RMSE [m]:", np.sqrt(np.mean((estimates[:,:3] - x_true)**2, axis=0)))
print("Velocity RMSE [m/s]:", np.sqrt(np.mean((estimates[:,3:] - v_true)**2, axis=0)))
