# python/stateest/kalman.py
"""
Kalman Filter Python Bindings Interface

This module provides Python bindings for the C++ KalmanFilter class and related state estimation tools.

Classes:
    KalmanFilter
        - Methods:
            __init__(F, Q, H, R, B)
            init(x0, P0)
            predict([u])
            update(z)
            getR()
            setR(newR)
            update_price(price)
            get_prediction_and_variance()
        - Properties:
            state
            covariance

Usage Example:
    import kalman
    import numpy as np

    F = np.eye(2)
    Q = 0.01 * np.eye(2)
    H = np.array([[1.0, 0.0]])
    R = np.array([[0.1]])
    B = np.zeros((2, 1))
    kf = kalman.KalmanFilter(F, Q, H, R, B)
    kf.init(np.array([0.0, 1.0]), np.eye(2))
    kf.predict()
    kf.update(np.array([0.05]))
    print(kf.state)
    print(kf.covariance)
    kf.setR(np.array([[0.5]]))  # Dynamically change measurement noise

Notes:
    - The 'kalman' submodule is deprecated. Use 'import stateest' directly instead.
    - All matrix arguments must be numpy arrays.
"""
# python/stateest/kalman.py
from ._stateest import *
import warnings
warnings.warn(
    "The 'kalman' submodule is deprecated. Use 'import stateest' directly instead.",
    DeprecationWarning,
    stacklevel=2
)

