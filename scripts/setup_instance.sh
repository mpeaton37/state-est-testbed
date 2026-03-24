#!/bin/bash
# =============================================================================
# setup-miniconda-jupyter.sh
# Fully idempotent setup for state-est-testbed demo on AWS EC2 (Ubuntu)
# Safe to run multiple times (AWS CLI, Miniconda, repo, password, notebook)
# =============================================================================

set -e

echo "=== Starting idempotent setup for state-est-testbed demo (Ubuntu) ==="

# ------------------------------------------------------------------
# 1. System Update + Basic Tools
# ------------------------------------------------------------------
echo "Updating system packages..."
apt-get update -qq && apt-get upgrade -y -qq
apt-get install -y -qq curl wget git build-essential unzip

# ------------------------------------------------------------------
# 2. AWS CLI v2 (idempotent)
# ------------------------------------------------------------------
if ! command -v aws >/dev/null 2>&1 || ! aws --version | grep -q "aws-cli/2"; then
    echo "Installing AWS CLI v2..."
    curl -s "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip"
    unzip -q awscliv2.zip
    ./aws/install --update
    rm -rf aws awscliv2.zip
else
    echo "AWS CLI v2 already installed."
fi

# ------------------------------------------------------------------
# 3. Miniconda (idempotent)
# ------------------------------------------------------------------
if [ ! -d "/opt/miniconda" ]; then
    echo "Installing Miniconda..."
    MINICONDA_URL="https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh"
    wget -q $MINICONDA_URL -O /tmp/miniconda.sh
    bash /tmp/miniconda.sh -b -p /opt/miniconda
    rm /tmp/miniconda.sh
else
    echo "Miniconda already installed."
fi

# ------------------------------------------------------------------
# 4. Robust ToS Acceptance
# ------------------------------------------------------------------
echo "Ensuring Anaconda Terms of Service are accepted..."
export CONDA_PLUGINS_AUTO_ACCEPT_TOS=yes
/opt/miniconda/bin/conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main || true
/opt/miniconda/bin/conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r   || true

# ------------------------------------------------------------------
# 5. Create conda environment (idempotent)
# ------------------------------------------------------------------
if ! /opt/miniconda/bin/conda env list | grep -q "stateest"; then
    echo "Creating conda environment 'stateest'..."
    /opt/miniconda/bin/conda create -n stateest python=3.12 -y
else
    echo "Conda environment 'stateest' already exists."
fi

# ------------------------------------------------------------------
# 6. Install Python packages (idempotent)
# ------------------------------------------------------------------
echo "Installing/updating scientific packages..."
/opt/miniconda/bin/conda run -n stateest CONDA_PLUGINS_AUTO_ACCEPT_TOS=yes \
    conda install -y numpy pandas matplotlib plotly pyyaml tqdm ipykernel

/opt/miniconda/bin/conda run -n stateest CONDA_PLUGINS_AUTO_ACCEPT_TOS=yes \
    pip install --upgrade pybind11 cmake jupyterlab

# ------------------------------------------------------------------
# 7. Jupyter Password from AWS Secrets Manager
# ------------------------------------------------------------------
echo "Retrieving Jupyter password from AWS Secrets Manager..."

SECRET_ID="state-est-testbed/jupyter-password"   # ← CHANGE IF NEEDED
REGION="us-east-1"                               # ← CHANGE TO YOUR REGION

PASSWORD=$(/usr/local/bin/aws secretsmanager get-secret-value \
    --secret-id "$SECRET_ID" \
    --region "$REGION" \
    --query "SecretString" \
    --output text 2>/dev/null | python3 -c '
import sys, json
try:
    data = json.loads(sys.stdin.read())
    print(data.get("jupyter_password", ""))
except:
    print("")
' ) || PASSWORD=""

if [ -z "$PASSWORD" ]; then
    PASSWORD="ChangeMe123!"
    echo "WARNING: Could not retrieve password from Secrets Manager. Using fallback."
fi

echo "Setting JupyterLab password..."
/opt/miniconda/bin/conda run -n stateest python -c "
from jupyter_server.auth import passwd
import json, os
config_dir = '/home/ubuntu/.jupyter'
os.makedirs(config_dir, exist_ok=True)
with open(os.path.join(config_dir, 'jupyter_server_config.json'), 'w') as f:
    json.dump({'ServerApp': {'password': passwd('$PASSWORD')}}, f, indent=2)
"

# ------------------------------------------------------------------
# 8. Clone / Update Repository (graceful)
# ------------------------------------------------------------------
echo "Cloning / updating repository..."
cd /home/ubuntu

if [ -d "state-est-testbed" ]; then
    echo "Repository exists → pulling latest changes..."
    cd state-est-testbed
    git pull --rebase --autostash || echo "Warning: git pull failed (continuing...)"
else
    echo "Cloning fresh repository..."
    git clone https://github.com/mpeaton37/state-est-testbed.git
    cd state-est-testbed
fi

# ------------------------------------------------------------------
# 9. Create demo notebook if missing
# ------------------------------------------------------------------
if [ ! -f demo_notebook.ipynb ]; then
    echo "Creating demo_notebook.ipynb..."
    cat > demo_notebook.ipynb << 'EON'
{
 "cells": [
  {
   "cell_type": "markdown",
   "source": [
    "# state-est-testbed Live Demo\n",
    "**Simulation-based R&D framework for state estimators and predictors**\n",
    "\n",
    "Repository: https://github.com/mpeaton37/state-est-testbed"
   ]
  },
  {
   "cell_type": "code",
   "source": [
    "import pandas as pd\n",
    "import stateest\n",
    "print('stateest version:', getattr(stateest, '__version__', 'unknown'))\n",
    "df = pd.read_csv('maneuver_kalman_test_data.csv')\n",
    "print(f'Loaded {len(df)} timesteps from simple.py trajectory')"
   ]
  }
 ],
 "metadata": {
  "kernelspec": {"name": "stateest", "display_name": "Python (stateest)"}
 },
 "nbformat": 4,
 "nbformat_minor": 5
}
EON
fi

# ------------------------------------------------------------------
# 10. Create jl wrapper
# ------------------------------------------------------------------
cat << 'EOF' > /usr/local/bin/jl
#!/bin/bash
CONDA_PLUGINS_AUTO_ACCEPT_TOS=yes \
/opt/miniconda/bin/conda run -n stateest \
jupyter lab --ip=0.0.0.0 --no-browser --allow-root \
--ServerApp.allow_remote_access=True \
--ServerApp.open_browser=False \
--ServerApp.allow_origin='*' \
"\$@"
EOF
chmod +x /usr/local/bin/jl

# ------------------------------------------------------------------
# 11. Final Welcome Message
# ------------------------------------------------------------------
cat << EOF > /home/ubuntu/WELCOME.txt
================================================================
          state-est-testbed AWS Demo Environment Ready!
================================================================

Repository: ~/state-est-testbed
JupyterLab: password protected (from AWS Secrets Manager)

To start:
    jl

Access: http://YOUR-EC2-PUBLIC-IP:8888

Setup completed on: $(date)
================================================================
EOF

chown -R ubuntu:ubuntu /home/ubuntu

echo "=== Idempotent Setup Completed Successfully! ==="
echo "You can safely re-run this script anytime."
echo "Run 'jl' to start JupyterLab."

