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
inotifywait -m -e close_write "$WATCH_DIR" --format '%w%f' |
while read FILE; do
    # Display which file was changed
    echo -n "[$(date)] | File changed: $FILE .. Uploading .. "

    # Upload the changed file to the remote destination
    # --update only uploads if source is newer than destination
    rclone copy "$FILE" "$REMOTE_PATH" --update

    if [ $? -eq 0 ]; then
        echo "Success."
    else
        echo "Failed."
    fi
done
