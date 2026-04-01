#!/bin/bash
# =============================================================================
# run_all_estimators.sh
#
# Simple script to run KF, UKF, EKF, and PF with the same experiment tag.
# Usage: ./run_all_estimators.sh [tag]
# =============================================================================

set -e

TAG=${1:-"experiment_$(date +%Y%m%d_%H%M)"}

echo "=== Running All Estimators ==="
echo "Tag: $TAG"
echo ""

# Clean previous results
rm -f results.db

echo "→ Running Kalman Filter (kf)..."
./build/state_est_testbed ./configs/test_kf.yaml --tag "$TAG"

echo "→ Running Unscented Kalman Filter (ukf)..."
./build/state_est_testbed ./configs/test_ukf.yaml --tag "$TAG"

echo "→ Running Extended Kalman Filter (ekf)..."
./build/state_est_testbed ./configs/test_ekf.yaml --tag "$TAG"

echo "→ Running Particle Filter (pf)..."
./build/state_est_testbed ./configs/test_pf.yaml --tag "$TAG"

echo ""
echo "=== All runs completed successfully! ==="
echo ""
echo "To compare the results, run:"
echo "   python ./python/compare_estimators.py --tag $TAG"
echo ""
echo "Done."