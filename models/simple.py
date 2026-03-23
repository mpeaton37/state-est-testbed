import numpy as np
import pandas as pd

# ───────────────────────────────────────────────
# Time & discretization
dt = 0.5               # time step (s)
t_max = 200.0
t = np.arange(0, t_max + dt/2, dt)
n = len(t)

# ───────────────────────────────────────────────
# Initial conditions (realistic start)
pos = np.zeros((n, 3))
vel = np.zeros((n, 3))
pos[0] = [800000.0, 0.0, 80000.0]      # x (downrange), y (cross), z (alt) [m]
vel[0] = [-5500.0, 0.0, -30.0]         # vx, vy, vz [m/s]

# ───────────────────────────────────────────────
# Maneuver parameters (sinusoidal lateral like your example)
amp_lat = 3000.0          # amplitude of lateral displacement [m] → ~6.5 km peak-to-peak here
period = 70.0             # maneuver period [s] → agile but realistic
omega = 2 * np.pi / period

# Precompute ideal sinusoidal lateral motion (for reference / optional override)
y_ideal = amp_lat * np.sin(omega * t)
vy_ideal = amp_lat * omega * np.cos(omega * t)
# Peak lateral accel = amp_lat * omega² ≈ 24 m/s² (~2.4 g)

# ───────────────────────────────────────────────
# Dynamics integration loop
for i in range(1, n):
    time = t[i-1]
    v = vel[i-1]
    speed = np.linalg.norm(v)
    
    # Quadratic drag
    drag_coeff = 0.00025
    drag = -drag_coeff * speed**2 if speed > 0 else 0.0
    a_drag = drag * v / speed if speed > 0 else np.zeros(3)
    
    # Lateral maneuver acceleration (simple harmonic → matches displacement/velocity)
    a_y = amp_lat * omega**2 * np.sin(omega * time)   # = -d²y/dt² for y = amp sin(ωt)
    
    # Vertical: gravity + small lift variation
    lift = 9.65 + 0.15 * np.cos(2 * np.pi * time / 55.0)  # ~1 m/s² oscillation
    a_z = -9.81 + lift
    
    # Total acceleration
    a = a_drag + np.array([0.0, a_y, a_z])
    
    # Integrate velocity & position (semi-implicit Euler for stability)
    vel[i] = vel[i-1] + a * dt
    pos[i] = pos[i-1] + vel[i-1] * dt + 0.5 * a * dt**2

# ───────────────────────────────────────────────
# Simulate  measurements (same as before)
np.random.seed(42)
sigma_range = 15.0
sigma_angle = 0.00025  # ~0.014°
sigma_rr = 1.5

meas = np.zeros((n, 4))
for i in range(n):
    p = pos[i]
    v = vel[i]
    r_true = np.linalg.norm(p)
    r = r_true if r_true > 1e-6 else 1e-6
    az = np.arctan2(p[1], p[0])
    el = np.arcsin(p[2] / r)
    rr = np.dot(v, p) / r
    
    meas[i,0] = r + np.random.normal(0, sigma_range)
    meas[i,1] = az + np.random.normal(0, sigma_angle)
    meas[i,2] = el + np.random.normal(0, sigma_angle)
    meas[i,3] = rr + np.random.normal(0, sigma_rr)

# ───────────────────────────────────────────────
# Build and save DataFrame
df = pd.DataFrame({
    'time_s':      t.round(1),
    'true_x_m':    pos[:,0].round(1),
    'true_y_m':    pos[:,1].round(1),
    'true_z_m':    pos[:,2].round(1),
    'true_vx_mps': vel[:,0].round(2),
    'true_vy_mps': vel[:,1].round(2),
    'true_vz_mps': vel[:,2].round(2),
    'meas_range_m': meas[:,0].round(1),
    'meas_az_rad':  meas[:,1].round(6),
    'meas_el_rad':  meas[:,2].round(6),
    'meas_rr_mps':  meas[:,3].round(2)
})

df.to_csv('maneuver_kalman_test_data.csv', index=False)
print("Full dataset (401 timesteps) saved to: maneuver_kalman_test_data.csv")

# Quick summary
print(f"Altitude range: {df['true_z_m'].min()/1000:.2f} – {df['true_z_m'].max()/1000:.2f} km")
print(f"Max lateral displacement (y): {df['true_y_m'].abs().max():.1f} m")
print(f"Peak lateral velocity (vy): {df['true_vy_mps'].abs().max():.1f} m/s")