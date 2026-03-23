from setuptools import setup

setup(
    name="pystate-est",
    version="0.1.0.dev0",
    packages=["pystate_est"],
    author="State Estimation Testbed Team",
    description="Python interface to state-est-testbed KalmanFilter bindings",
    long_description=open("README.md").read() if os.path.exists("README.md") else "",
    long_description_content_type="text/markdown",
    python_requires=">=3.8",
    classifiers=[
        "Development Status