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

echo "Initializing conda..."
/opt/miniconda/bin/conda init bash

# Source so the current shell knows about conda
source /ubuntu/.bashrc

echo "Creating and activating stateest environment..."
conda create -n stateest python=3.12 -y
conda activate stateest

echo "Installing packages..."
conda install -y numpy pandas matplotlib plotly pyyaml tqdm ipykernel
pip install pybind11 cmake jupyterlab

# Register the kernel
python -m ipykernel install --user --name stateest --display-name "Python (stateest)"


# Create convenient aliases and environment activation
cat << EOF >> /ubuntu/.bashrc

# Convenience for state-est-testbed
alias jl='conda run -n stateest jupyter lab --ip=0.0.0.0 --no-browser --allow-root'
export PATH="/opt/miniconda/bin:\$PATH"
conda activate stateest 2>/dev/null || true
EOF

# Create a welcome message
cat << EOF > /ubuntu/WELCOME.txt
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
echo "Read /ubuntu/WELCOME.txt for next steps."
