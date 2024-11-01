#!/bin/bash


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


echo "[$(date)]: Uploading Data.."

rclone copy "$DATA_DIR" gcs:$G_BUCKET/$BUCKET_DIR --update 

# Check exit status of the last command executed
if [[ $? -ne 0 ]]; then
  echo "ERROR: Uploading files.."
  exit 1
fi