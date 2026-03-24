#!/bin/bash
# =============================================================================
# setup-miniconda-jupyter.sh
# One-shot setup for state-est-testbed demo on AWS EC2
# Installs Miniconda + JupyterLab + common scientific packages
# =============================================================================

set -e  # exit on any error

echo "=== Starting Miniconda + JupyterLab setup for state-est-testbed ==="

# Update system
echo "Updating system packages..."
if command -v apt-get >/dev/null 2>&1; then
    apt-get update -qq && apt-get upgrade -y -qq
    apt-get install -y -qq curl wget git build-essential
else
    dnf update -y -q
    dnf install -y -q curl wget git gcc gcc-c++ make
fi

# Install Miniconda (latest)
echo "Installing Miniconda..."
MINICONDA_URL="https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh"
wget -q $MINICONDA_URL -O /tmp/miniconda.sh

bash /tmp/miniconda.sh -b -p /opt/miniconda
rm /tmp/miniconda.sh

# Initialize conda for root and add to PATH
/opt/miniconda/bin/conda init bash
source /root/.bashrc

# Create a dedicated environment for the testbed
echo "Creating conda environment 'stateest'..."
/opt/miniconda/bin/conda create -n stateest python=3.12 -y

# Activate and install core packages
echo "Installing scientific packages..."
/opt/miniconda/bin/conda run -n stateest conda install -y \
    numpy pandas matplotlib plotly pyyaml tqdm ipykernel

/opt/miniconda/bin/conda run -n stateest pip install pybind11 cmake jupyterlab

# Make the environment kernel available to Jupyter
/opt/miniconda/bin/conda run -n stateest python -m ipykernel install --user --name stateest --display-name "Python (stateest)"

# Create convenient aliases and environment activation
cat << EOF >> /root/.bashrc

# Convenience for state-est-testbed
alias jl='conda run -n stateest jupyter lab --ip=0.0.0.0 --no-browser --allow-root'
export PATH="/opt/miniconda/bin:\$PATH"
conda activate stateest 2>/dev/null || true
EOF

# Create a welcome message
cat << EOF > /root/WELCOME.txt
================================================================
          state-est-testbed AWS Demo Environment Ready!
================================================================

To start JupyterLab:
    jl

Then open in your browser:
    http://YOUR-EC2-PUBLIC-IP:8888

Working directory: /home/ec2-user/state-est-testbed  (create it if needed)

Useful commands:
    conda activate stateest
    jupyter lab
    python -c "import stateest; print('stateest ready!')"

Remember to STOP the instance when not in use!
================================================================
EOF

echo "=== Setup completed successfully! ==="
echo "Read /root/WELCOME.txt for next steps."
