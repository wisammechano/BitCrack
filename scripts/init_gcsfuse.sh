#!/bin/bash

echo "Initializing gcsfuse..."

if [[ -z "${G_BUCKET}" ]]; then
  echo "ERROR: G_BUCKET environment variable is not set."
  exit 1
fi

if [[ ! -d "${DATA_DIR}" ]]; then
  echo "ERROR: DATA_DIR is not a directory."
  exit 1
fi

CREDENTIALS=${1:-$CREDENTIALS_PATH}

# Check if service account file exists; if not, exit
if [[ ! -f "$CREDENTIALS" ]]; then
  echo "Service account file not provided. Exiting..."
  exit 1
else
  echo "Using service account file ${CREDENTIALS}"
fi

BUCKET_DIR='data'

# Mount
gcsfuse --key-file=$CREDENTIALS --only-dir $BUCKET_DIR $G_BUCKET $DATA_DIR


# Verify mounting
if mount | grep "gcsfuse" > /dev/null; then
  echo "Bucket mounted successfully at $DATA_DIR"
else
  echo "ERROR: Bucket mounting failed."
  exit 1
fi
