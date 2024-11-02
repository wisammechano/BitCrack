#!/bin/bash

# rclone copy function
copy() {
    local src="$1"
    local dst="$2"
    rclone copy "$src" "$dst" --update 
    if [[ $? -ne 0 ]]; then
        echo "ERROR: rclone failed to copy from $src to $dst"
        exit 1
    fi
}

# Environment variable checks
if [[ -z "${G_BUCKET}" ]]; then
    echo "ERROR: G_BUCKET environment variable is not set."
    exit 1
fi

if [[ -z "${DATA_DIR}" || ! -d "${DATA_DIR}" ]]; then
    echo "ERROR: DATA_DIR environment variable is not set or is not a directory."
    exit 1
fi

# Check if operation argument is provided
if [ $# -lt 1 ]; then
    echo "ERROR: Please provide an operation argument."
    exit 1
fi

OPERATION=$1
BUCKET_DIR='data'

case $OPERATION in
    "upload"|"u")
        echo "[$(date)] | Uploading files..."
        copy "$DATA_DIR" "gcs:$G_BUCKET/$BUCKET_DIR"
        echo "[$(date)] | Upload completed successfully."
        ;;
    
    "download"|"d")
        echo "[$(date)] | Downloading files..."
        copy "gcs:$G_BUCKET/$BUCKET_DIR" "$DATA_DIR"
        echo "[$(date)] | Download completed successfully."
        ;;
    
    "both"|"bi"|"b")
        echo "[$(date)] | Downloading then uploading files (syncing)..."
        copy "gcs:$G_BUCKET/$BUCKET_DIR" "$DATA_DIR"
        sleep 5
        copy "$DATA_DIR" "gcs:$G_BUCKET/$BUCKET_DIR"
        echo "[$(date)] | Syncing completed successfully."
        ;;
    
    *)
        echo "ERROR: Unknown operation $OPERATION"
        exit 1
        ;;
esac
