#!/bin/bash

# Define the root directory where the tree structure is located
root_directory="results/"

# Find all files named col.txt, grid.txt, and row.txt recursively
# and process them one by one
find "$root_directory" -type f \( -name "par_a.txt" -o -name "par_d.txt" \) -print0 |
while IFS= read -r -d '' file; do
    # Use awk to filter lines that start with a number and overwrite the file
    awk '!/^\[/' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
done

echo "Lines not starting with a number have been removed."

