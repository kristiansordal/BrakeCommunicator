#!/bin/bash

# Loop through files starting with "slurm"
for file in slurm*; do
    # Check if the file is a regular file
    if [ -f "$file" ]; then
        # Use grep to filter lines starting with a capital letter and overwrite the file
        grep '^[[:upper:]]' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
    fi
done
