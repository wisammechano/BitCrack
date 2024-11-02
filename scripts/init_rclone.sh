#!/bin/bash

echo "Initializing rclone..."


if [[ -z "${G_BUCKET}" ]]; then
  echo "ERROR: G_BUCKET environment variable is not set."
  exit 1
fi

if [[ ! -d "${DATA_DIR}" ]]; then
  echo "ERROR: DATA_DIR is not a directory."
  exit 1
fi

CREDENTIALS="${1:-$CREDENTIALS_PATH}"

# Check if service account file exists; if not, exit
if [[ ! -f "$CREDENTIALS" ]]; then
  echo "Service account file not provided. Exiting..."
  exit 1
else
  echo "Using service account file ${CREDENTIALS}"
fi


BUCKET_DIR='data'

CONFIG_FILE_PATH="/root/.config/rclone"
mkdir -p "${CONFIG_FILE_PATH}"

# Create config
CONFIG_CONTENT=$(cat <<EOF
[gcs]
type = google cloud storage
service_account_file = ${CREDENTIALS}
object_acl = private
bucket_acl = private
location = europe-central2
no_check_bucket = true
bucket_policy_only = true
EOF
)

# Write the content to the file
echo "$CONFIG_CONTENT" > "$CONFIG_FILE_PATH"/rclone.conf

# IF mount

# If copy
echo "Downloading remote files"

rclone copy gcs:$G_BUCKET/$BUCKET_DIR "$DATA_DIR" --update 

# Check exit status of the last command executed
if [[ $? -ne 0 ]]; then
  echo "ERROR: Failed to init rclone."
  exit 1
fi

# Setup a cron job that uploads data every 2 minutes
# Create a cron job entry
SCRIPT_DIR=${SCRIPTS_DIR:-"$(dirname "$(realpath "$0")")"}

CRON_COMMAND="${SCRIPT_DIR}/upload_data_rclone.sh $CREDENTIALS >> ${DATA_DIR}/log.log 2>&1"

CRON_JOB="*/1 * * * * $CRON_COMMAND"

# Check if the cron job already exists
if ! crontab -l | grep -Fxq "$CRON_JOB"; then
    # Add the cron job
    (crontab -l; echo "$CRON_JOB") | crontab -
    echo "Cron job added: $CRON_JOB"
else
    echo "Cron job already exists: $CRON_JOB"
fi