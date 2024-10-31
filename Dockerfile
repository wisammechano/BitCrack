# Use the official CUDA image from NVIDIA
FROM nvidia/cuda:11.4.2-devel-ubuntu20.04

# Install necessary packages
RUN apt-get update && apt-get install -y \
  build-essential \
  g++ \
  make \
  google-cloud-sdk \
  gcsfuse \
  && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the current directory contents into the container at /app
COPY . /app

# Build the project
RUN make BUILD_CUDA=1

# Make Runner Script Executable
RUN chmod +x /app/runner/run

# -- Mount GCloud Bucket for Data -- #

# Make mounting script executable
RUN chmod +x /app/runner/init_gcloud_and_mount

# Run the mounting script
RUN /app/runner/init_gcloud_and_mount.sh

# Set the entrypoint to bash
ENTRYPOINT ["/bin/bash"]