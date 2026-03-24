#!/bin/bash
# =============================================================================
# setup_instance.sh
# Fully idempotent setup for state-est-testbed demo on AWS EC2 (Ubuntu)
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
# 2. AWS CLI v2
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
# 3. Miniconda
# ------------------------------------------------------------------
if [ ! -d "/opt/miniconda" ]; then
    echo "Installing Miniconda..."
    wget -q https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O /tmp/miniconda.sh
    bash /tmp/miniconda.sh -b -p /opt/miniconda
    rm /tmp/miniconda.sh
else
    echo "Miniconda already installed."
fi

# ------------------------------------------------------------------
# 4. ToS Acceptance
# ------------------------------------------------------------------
echo "Ensuring Anaconda Terms of Service are accepted..."
export CONDA_PLUGINS_AUTO_ACCEPT_TOS=yes
/opt/miniconda/bin/conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main || true

# ------------------------------------------------------------------
# 5. Create conda environment
# ------------------------------------------------------------------
if ! /opt/miniconda/bin/conda env list | grep -q "stateest"; then
    echo "Creating conda environment 'stateest'..."
    /opt/miniconda/bin/conda create -n stateest python=3.12 -y
else
    echo "Conda environment 'stateest' already exists."
fi

# ------------------------------------------------------------------
# 6. Install base Python packages + Eigen3 (critical for CMake)
# ------------------------------------------------------------------
echo "Installing base scientific packages + Eigen3..."
/opt/miniconda/bin/conda run -n stateest CONDA_PLUGINS_AUTO_ACCEPT_TOS=yes \
    conda install -y \
    numpy pandas matplotlib plotly pyyaml tqdm ipykernel \
    eigen cmake pybind11

/opt/miniconda/bin/conda run -n stateest pip install --upgrade jupyterlab

# ------------------------------------------------------------------
# 7. Clone / Update Repository
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
# 8. Build + Install stateest package
# ------------------------------------------------------------------
echo "Building and installing stateest package..."
cd /home/ubuntu/state-est-testbed

# Clean old build artifacts
rm -rf build/ *.egg-info/ dist/ 2>/dev/null || true

# Build C++ extension with Eigen from conda
echo "Building pybind11 extension..."
/opt/miniconda/bin/conda run -n stateest cmake -B build -S . \
    -DPython3_EXECUTABLE=/opt/miniconda/envs/stateest/bin/python \
    -DCMAKE_PREFIX_PATH=/opt/miniconda/envs/stateest \
    -DCMAKE_BUILD_TYPE=Release

/opt/miniconda/bin/conda run -n stateest cmake --build build --config Release -j2

# Install in editable mode
echo "Installing stateest in editable mode..."
/opt/miniconda/bin/conda run -n stateest pip install -e .

echo "✅ stateest package installed successfully."
# ------------------------------------------------------------------
# 9. Register Jupyter kernel
# ------------------------------------------------------------------
echo "Registering Jupyter kernel 'stateest'..."
/opt/miniconda/bin/conda run -n stateest \
    python -m ipykernel install --user --name stateest --display-name "Python (stateest)"

# ------------------------------------------------------------------
# 10. Set JupyterLab password from AWS Secrets Manager
# ------------------------------------------------------------------
echo "Retrieving Jupyter password from AWS Secrets Manager..."

SECRET_ID="state-est-testbed/jupyter-password"
REGION="us-east-1"

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
' ) || PASSWORD="ChangeMe123!"

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
# 11. Create jl wrapper
# ------------------------------------------------------------------
cat > /usr/local/bin/jl << 'EOF'
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
# 12. Final Welcome Message
# ------------------------------------------------------------------
cat << EOF > /home/ubuntu/WELCOME.txt
================================================================
          state-est-testbed AWS Demo Environment Ready!
================================================================

Repository: ~/state-est-testbed
stateest package: installed (editable)
Jupyter kernel: registered as "Python (stateest)"

Start JupyterLab:
    jl

Access: http://YOUR-EC2-PUBLIC-IP:8888

Setup completed on: $(date)
================================================================
EOF

chown -R ubuntu:ubuntu /home/ubuntu

echo "=== Setup Completed Successfully! ==="
echo "You can safely re-run this script anytime."
echo "Run 'jl' to start JupyterLab."

