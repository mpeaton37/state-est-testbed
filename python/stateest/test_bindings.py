#!/usr/bin/env python3
"""
Test script for pybind11 KalmanFilter bindings.
Verifies import of kalman.so, instantiation, init/predict/update, and properties.
"""

import sys
import os
import argparse
import numpy as np


def main():
    parser = argparse.ArgumentParser(description="Test kalman Python bindings")
    parser.add_argument(
        "--build-dir",
        default="build",
        help="Path to CMake build directory (default: 'build')"
    )
    args = parser.parse_args()

    # Dynamically add build/python/ to sys.path
    python_path = os.path.abspath(os.path.join(args.build_dir, "python"))
    if not os.path.exists(python_path):
        raise FileNotFoundError(
            f"Build Python module directory not found: {python_path}\n"
            f"Run 'cmake -B {args.build_dir} .. && cmake --build {args.build_dir}' first."
        )
    sys.path.insert(0, python_path)
    print(f"✓ Added {python_path} to PYTHONPATH")

    # Test 1: Import
    import kalman
    print("✓ Successfully imported 'kalman' module!")

    # Test 2: Instantiate KalmanFilter (2D state, 1D measurement)
    F = np.eye(2)  # Identity transition
    Q = 0.01 * np.eye(2)  # Process noise
    H = np.array([[1.0, 0.0]])  # Position observation
    R = np.array([[0.1]])  # Measurement noise
    B = np.zeros((2, 1))  # Control input matrix (no control used)
    kf = kalman.KalmanFilter(F, Q, H, R, B)
    print("✓ KalmanFilter instantiated with matrices F,Q,H,R,B")

    # Test 3: init()
    x0 = np.array([0.0, 1.0])
    P0 = np.eye(2)
    kf.init(x0, P0)
    print("✓ init() called with x0, P0")
    print(f"  Initial state: {kf.state}")
    assert np.allclose(kf.state, x0)
    print("  ✓ state matches x0")

    # Test 4: update() + predict()
    z = np.array([0.05])  # Noisy position measurement
    kf.update(z)
    kf.predict()
    final_state = kf.state
    final_cov = kf.covariance
    print(f"  After update({z[0]}) + predict():")
    print(f"    state: {final_state}")
    print(f"    cov[0,0]: {final_cov[0,0]:.6f}")
    assert final_state.shape == (2,)
    assert final_cov.shape == (2, 2)
    print("✓ predict()/update() succeeded; shapes correct")

    # Test 5: Convenience methods (from bindings)
    pred_price = kf.update_price(0.1)  # 1D price update + predict
    print(f"  update_price(0.1) → predicted price: {pred_price:.6f}")
    pred_var = kf.get_prediction_and_variance()[1]
    print(f"  get_prediction_and_variance() → var: {pred_var:.6f}")
    print("✓ Convenience methods work")

    print("\n🎉 All tests passed! Python bindings are fully functional.")


def test_kalmanfilter_setR():
    """
    Test that KalmanFilter.setR and getR work as expected.
    """
    import numpy as np
    import kalman

    # 1D Kalman Filter
    F = np.array([[1.0]])
    Q = np.array([[0.01]])
    H = np.array([[1.0]])
    R = np.array([[0.1]])
    B = np.array([[0.0]])

    kf = kalman.KalmanFilter(F, Q, H, R, B)
    kf.init(np.array([0.0]), np.array([[1.0]]))

    # Check initial R
    np.testing.assert_allclose(kf.getR(), R)

    # Change R
    new_R = np.array([[0.5]])
    kf.setR(new_R)
    np.testing.assert_allclose(kf.getR(), new_R)

    # Run a predict/update step to ensure filter still works
    kf.predict()
    kf.update(np.array([1.0]))
    print("✓ KalmanFilter.setR/getR test passed.")

# Optionally, call the test at the end of the script:
if __name__ == "__main__":
    main()
    test_kalmanfilter_setR()
