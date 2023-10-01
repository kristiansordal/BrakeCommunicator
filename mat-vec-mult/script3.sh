#!/bin/bash

# Check for the correct number of arguments
if [ $# -ne 2 ]; then
  echo "Usage: $0 <directory> <output_file>"
  exit 1
fi

directory="$1"
output_file="$2"

# Check if the specified directory exists
if [ ! -d "$directory" ]; then
  echo "Directory '$directory' does not exist."
  exit 1
fi

# Initialize the output file (create or clear it)
> "$output_file"

# Loop through each file in the directory
for file in "$directory"/*; do
  # Check if the file is a regular file
  if [ -f "$file" ]; then
    # Get the last line from the file
    last_line=$(tail -n 1 "$file")

    # Append the last line to the output file
    echo "$last_line" >> "$output_file"

    echo "Last line from '$file' appended to '$output_file'."
  fi
done
