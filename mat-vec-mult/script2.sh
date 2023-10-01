#!/bin/bash

# Check for the correct number of arguments
if [ $# -ne 1 ]; then
  echo "Usage: $0 <directory>"
  exit 1
fi

directory="$1"

# Check if the specified directory exists
if [ ! -d "$directory" ]; then
  echo "Directory '$directory' does not exist."
  exit 1
fi

# Loop through each file in the directory
for file in "$directory"/*; do
  # Check if the file is a regular file
  if [ -f "$file" ]; then
    # Get the last line from the file
    last_line=$(tail -n 1 "$file")

    # Delete the original file
    rm "$file"

    # Write the last line to a new file with the same name
    echo "$last_line" > "$file"

    echo "Last line from '$file' written to a new file with the same name."
  fi
done
