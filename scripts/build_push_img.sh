#!/bin/bash

# Variables
IMAGE_NAME="wisso/bitcrack:improved"
DOCKERFILE="Dockerfile.${1:-cuda}"
DOCKER_UNAME="wisso"

# Build the Docker image
echo "Building Docker image: $IMAGE_NAME..."
docker build -t "$IMAGE_NAME" . -f "$DOCKERFILE"

# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "Docker build failed."
    exit 1
fi


# Check for DOCKER_PASSWORD environment variable
if [ -z "$DOCKER_PASSWORD" ]; then
    # Prompt for Docker Hub password
    echo -n "Enter your Docker Hub password: "
    read -s DOCKER_PASSWORD
    echo
fi

# Log in to Docker
echo "Logging in to Docker..."
echo "$DOCKER_PASSWORD" | docker login -u $DOCKER_UNAME --password-stdin

# Check if login was successful
if [ $? -ne 0 ]; then
    echo "Docker login failed."
    exit 1
fi

# Push the Docker image
echo "Pushing Docker image: $IMAGE_NAME..."
docker push "$IMAGE_NAME"

# Check if push was successful
if [ $? -eq 0 ]; then
    echo "Docker image pushed successfully."
else
    echo "Failed to push Docker image."
    exit 1
fi
