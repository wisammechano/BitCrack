#!/bin/bash

FILE_PATH=${1:-$CREDENTIALS_PATH}

# G_SA_TOKEN: is the file download link token, acquire from firebase storage
if [[ -z "${G_SA_TOKEN}" ]]; then
  echo "ERROR: G_SA_TOKEN environment variable is not set."
  exit 1
fi

if [[ -z "${G_BUCKET}" ]]; then
  echo "ERROR: G_BUCKET environment variable is not set."
  exit 1
fi

if [[ -z "${FILE_PATH}" ]]; then
  echo "ERROR: Provide a file path arg or define CREDENTIALS_PATH variable"
  exit 1
fi

# Download service account FILE
echo "Downloading service account credentials..."
G_SA_PATH="secure%2Fsa_user.json"
G_TOKEN_LINK="https://firebasestorage.googleapis.com/v0/b/${G_BUCKET}/o/${G_SA_PATH}?alt=media&token=${G_SA_TOKEN}"
curl -sS -o $FILE_PATH $G_TOKEN_LINK
if [ $? -ne 0 ]; then
  echo "ERROR: Failed to download service account file."
  exit 1
else
  echo "Service account file downloaded successfully. Path: ${FILE_PATH}"
fi