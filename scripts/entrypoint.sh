#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

env >> /etc/environment

# Set  terminal to bash
ln -sf /bin/bash /bin/sh
export SHELL=/bin/bash

echo "Mounting data dir"
${SCRIPTS_DIR}/init_data_bucket rclone

# Create the .ssh directory and add the public key
mkdir ~/.ssh && chmod 700 ~/.ssh

# Copy the SSH public key from the environment variable into the authorized keys
echo "$SSH_PUBLIC_KEY" > ~/.ssh/authorized_keys && \
    chmod 600 ~/.ssh/authorized_keys && \
    chown -R root:root ~/.ssh

# Start ssh server
echo "Starting ssh daemon..."
service ssh start

export JUPYTER_TOKEN=${USE_JUPYTER_TOKEN:-$JUPYTER_TOKEN}

# Start JupyterLab
echo "Starting JupyterLab..."
exec jupyter lab --ip=0.0.0.0 --port=8080 --no-browser --allow-root