#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

# Set  terminal to bash
ln -sf /bin/bash /bin/sh
export SHELL=/bin/bash

echo "Mounting data dir"
./scripts/init_data_bucket

# Start JupyterLab
echo "Starting JupyterLab..."
exec jupyter lab --ip=0.0.0.0 --port=8888 --no-browser --allow-root