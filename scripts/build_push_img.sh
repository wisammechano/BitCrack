#!/bin/bash

# Variables
DOCKER_UNAME="wisso"
IMAGE_NAME=""

# Function to display usage information
show_usage() {
    echo "Usage: $0 -t <tag> -d <dockerfile>"
    echo "  -t <tag>          Image tag (required)"
    echo "  -d <dockerfile>   Dockerfile to use (required)"
    exit 1
}

# Parse command-line arguments
while getopts ":t:d:" opt; do
    case $opt in
        t) TAG="$OPTARG" ;;
        d) DOCKERFILE="$OPTARG" ;;
        \?) echo "Invalid option: -$OPTARG" >&2; show_usage ;;
        :) echo "Option -$OPTARG requires an argument." >&2; show_usage ;;
    esac
done

# Check if both TAG and DOCKERFILE are set
if [ -z "$TAG" ] || [ -z "$DOCKERFILE" ]; then
    echo "Error: Both -t <tag> and -d <dockerfile> are required."
    show_usage
fi

# Check for DOCKER_PASSWORD environment variable
if [ -z "$DOCKER_PASSWORD" ]; then
    # Prompt for Docker Hub password
    echo -n "Enter your Docker Hub password: "
    read -s DOCKER_PASSWORD
    echo
fi

# Set the image name using the tag
IMAGE_NAME="${DOCKER_UNAME}/bitcrack:${TAG}"

# Build the Docker image
echo "Building Docker image: $IMAGE_NAME with Dockerfile: $DOCKERFILE..."
docker build -t "$IMAGE_NAME" . -f "Dockerfile.$DOCKERFILE"

# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "Docker build failed."
    exit 1
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
