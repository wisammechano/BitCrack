FROM jupyter/minimal-notebook:latest

# Set environment variables for paths and SUDO
ENV BUILD_DIR=/app_build \
    APP_DIR=/app \
    DATA_DIR=/app/data \
    BIN_PATH=/app/bin/cuBitCrack \
    GRANT_SUDO=yes

# Elevate access to install and build
USER root

# Step 1: Install Cuda Keyring
RUN curl https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb -o pkg.deb && dpkg -i pkg.deb && rm -f pkg.deb

# Step 2: Install necessary packages
RUN apt-get update && apt-get install -y \
  build-essential \
  g++ \
  make \
  cuda-toolkit-12-6 \
  && rm -rf /var/lib/apt/lists/*

# Step 3: Add Library to PATH
ENV LD_LIBRARY_PATH="/usr/local/cuda/lib64:${LD_LIBRARY_PATH}" \
    PATH="/usr/local/cuda/bin:${PATH}"

# Step 4 build app
# Set the working directory to BUILD_DIR and copy source code
WORKDIR $BUILD_DIR
COPY ./src .

# Build the project with CUDA support
RUN make BUILD_CUDA=1

# Step 5: Initialize Jupyter environment

# Move to APP_DIR and set up application files and scripts
WORKDIR $APP_DIR
COPY ./runner .
COPY ./scripts ./scripts
# Copy binaries from builder
RUN cp -r $BUILD_DIR/bin ./bin

# Make runner and scripts executable and move ownership to default user while elevating access
RUN chmod +x ./run ./scripts/* ./bin/* && chown -R ${NB_UID}:${NB_GID} $APP_DIR && echo "$NB_USER ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/notebook
# Install Google Cloud Storage FUSE
RUN ./scripts/install_gcsfuse

USER ${NB_UID}

# Set the entrypoint to start Jupyter Notebook
ENTRYPOINT ["start-notebook.sh"]