# python/stateest/__init__.py
"""stateest – lightweight Python interface for the state estimation testbed."""

from .kalman import KalmanFilter
from .config import Config, load_config
from .experiment import ExperimentRunner, run_monte_carlo

__version__ = "0.1.0-dev"
__all__ = [
    "KalmanFilter",
    "Config",
    "load_config",
    "ExperimentRunner",
    "run_monte_carlo",
]