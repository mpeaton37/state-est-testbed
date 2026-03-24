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

# Install AWS CLI v2 (required for Secrets Manager)
echo "Installing AWS CLI v2..."
curl -s "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip"
unzip -q awscliv2.zip
./aws/install
rm -rf aws awscliv2.zip

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

# === Pull password from AWS Secrets Manager ===
echo "Retrieving Jupyter password from AWS Secrets Manager..."

SECRET_ID="state-est-testbed/jupyter-password"   
REGION="us-east-1"                               

# Retrieve the value of the key "jupyter_password"
PASSWORD=$(/usr/local/bin/aws secretsmanager get-secret-value \
    --secret-id "$SECRET_ID" \
    --region "$REGION" \
    --query "SecretString" \
    --output text 2>/dev/null | python3 -c "

import sys, json
try:
    data = json.loads(sys.stdin.read())
    print(data.get("jupyter_password", ""))
except:
    print("")
" ) || PASSWORD=""

# Fallback if secret retrieval failed
if [ -z "$PASSWORD" ]; then
    PASSWORD="ChangeMe123!"
    echo "WARNING: Could not retrieve password from Secrets Manager."
    echo "         Using fallback password: ChangeMe123!"
    echo "         Please update it after login using: jupyter lab password"
fi

echo "Password length retrieved: ${#PASSWORD} characters"

# Set the JupyterLab password
echo "Setting JupyterLab password..."
/opt/miniconda/bin/conda run -n stateest python -c "
from jupyter_server.auth import passwd
import json, os
config_dir = '/home/ubuntu/.jupyter'
os.makedirs(config_dir, exist_ok=True)
config_path = os.path.join(config_dir, 'jupyter_server_config.json')
with open(config_path, 'w') as f:
    json.dump({'ServerApp': {'password': passwd('$PASSWORD')}}, f, indent=2)
"
# Clone your repository
echo "Cloning your repository..."
cd /home/ubuntu
git clone https://github.com/mpeaton37/state-est-testbed.git
cd state-est-testbed

# Create jl wrapper
cat << 'EOF' > /usr/local/bin/jl
#!/bin/bash
CONDA_PLUGINS_AUTO_ACCEPT_TOS=yes \
/opt/miniconda/bin/conda run -n stateest \
jupyter lab --ip=0.0.0.0 \
--no-browser \
--allow-root \
--ServerApp.allow_remote_access=True \
--ServerApp.open_browser=False \
--ServerApp.allow_origin='*' \
"\$@"
EOF
chmod +x /usr/local/bin/jl

# Welcome message
cat << EOF > /home/ubuntu/WELCOME.txt
================================================================
          state-est-testbed AWS Demo Environment Ready!
================================================================

JupyterLab is password protected.

Start the server with:
    jl

Access URL: http://YOUR-EC2-PUBLIC-IP:8888

Setup completed on: $(date)
================================================================
EOF

chown ubuntu:ubuntu /home/ubuntu/WELCOME.txt

echo "=== Setup completed successfully! ==="
echo "Run 'jl' to start JupyterLab."

