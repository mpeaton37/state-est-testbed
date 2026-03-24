#!/bin/bash
# =============================================================================
# setup-miniconda-jupyter.sh
# One-shot setup for state-est-testbed demo on AWS EC2 (Ubuntu)
# Uses full conda paths — no 'conda init' 
# =============================================================================

set -e

echo "=== Starting Miniconda + JupyterLab setup for state-est-testbed (Ubuntu) ==="

# Update system
echo "Updating system packages..."
apt-get update -qq && apt-get upgrade -y -qq
apt-get install -y -qq curl wget git build-essential

# Install Miniconda
echo "Installing Miniconda..."
MINICONDA_URL="https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh"
wget -q $MINICONDA_URL -O /tmp/miniconda.sh

bash /tmp/miniconda.sh -b -p /opt/miniconda
rm /tmp/miniconda.sh



# === ROBUST ToS ACCEPTANCE ===
echo "Accepting Anaconda Terms of Service automatically..."
export CONDA_PLUGINS_AUTO_ACCEPT_TOS=yes

# Create environment using full path (no init)
echo "Creating conda environment 'stateest'..."
/opt/miniconda/bin/conda create -n stateest python=3.12 -y

echo "Accepting Anaconda Terms of Service (non-interactive)..."
/opt/miniconda/bin/conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main || true
/opt/miniconda/bin/conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r   || true

# Also accept the defaults channel (good practice)
 /opt/miniconda/bin/conda tos accept --override-channels --channel defaults || true

# Install packages
echo "Installing scientific packages..."
/opt/miniconda/bin/conda run -n stateest conda install -y \
    numpy pandas matplotlib plotly pyyaml tqdm ipykernel

/opt/miniconda/bin/conda run -n stateest pip install pybind11 cmake jupyterlab

# Register Jupyter kernel
/opt/miniconda/bin/conda run -n stateest python -m ipykernel install --user --name stateest --display-name "Python (stateest)"

# Create convenient wrapper script
cat << 'EOF' > /usr/local/bin/jl
#!/bin/bash
/opt/miniconda/bin/conda run -n stateest jupyter lab --ip=0.0.0.0 --no-browser --allow-root "$@"
EOF
chmod +x /usr/local/bin/jl

# Create welcome message for ubuntu user
cat << 'EOF' > /home/ubuntu/WELCOME.txt
================================================================
          state-est-testbed AWS Demo Environment Ready!
================================================================

To start JupyterLab, simply run:
    jl

Then open in your browser:
    http://YOUR-EC2-PUBLIC-IP:8888/lab

Recommended workflow:
    cd ~/state-est-testbed
    jl

Useful commands:
    /opt/miniconda/bin/conda run -n stateest python -c "import stateest; print('stateest ready!')"

Remember: STOP the instance when you're not using it to save costs!

Created on: $(date)
================================================================
EOF

chown ubuntu:ubuntu /home/ubuntu/WELCOME.txt

echo "=== Setup completed successfully! ==="
echo "Read /home/ubuntu/WELCOME.txt for next steps."
echo "You can now run:   jl"



