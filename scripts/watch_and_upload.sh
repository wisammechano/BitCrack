#!/bin/bash

# Directory to watch
WATCH_DIR=$DATA_DIR
BUCKET_DIR="data"

# Remote destination path for rclone
REMOTE_PATH="gcs:$G_BUCKET/$BUCKET_DIR"


# Check if inotifywait is installed
if ! command -v inotifywait &> /dev/null; then
    echo "inotifywait is required but not installed. Install with: sudo apt install inotify-tools"
    exit 1
fi

# Run a loop that listens for changes
inotifywait -m -r -e close_write "$WATCH_DIR" --format '%w%f' |
while read FILE; do
    # Display which file was changed

    # Extract the filename and path relative to $WATCH_DIR
    RELATIVE_PATH=$(dirname "$FILE" | sed "s|^$WATCH_DIR/||")
    BASE_NAME=$(basename "$FILE")
    echo -n "[$(date)] | File changed: $RELATIVE_PATH/$BASE_NAME .. Uploading .. "

    # Upload the changed file to the remote destination
    # --update only uploads if source is newer than destination
    rclone copy "$FILE" "${REMOTE_PATH}/${RELATIVE_PATH}" --update

    if [ $? -eq 0 ]; then
        echo "Success."
    else
        echo "Failed."
    fi
done
