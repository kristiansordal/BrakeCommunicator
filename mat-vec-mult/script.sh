#!/bin/bash

# Check for the correct number of arguments
if [ $# -ne 1 ]; then
  echo "Usage: $0 <input_file>"
  exit 1
fi

input_file="$1"

# Check if the input file exists
if [ ! -f "$input_file" ]; then
  echo "Input file '$input_file' does not exist."
  exit 1
fi

# Get the last line from the input file
last_line=$(tail -n 1 "$input_file")

# Delete the original input file
rm "$input_file"

# Write the last line to a new file with the same name as the original file
echo "$last_line" > "$input_file"

echo "Last line from '$input_file' written to a new file with the same name."
