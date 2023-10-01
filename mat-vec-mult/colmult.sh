#!/bin/bash

# Specify the output directory
output_directory="colmult_res"

# Check for the correct number of arguments
if [ $# -ne 1 ]; then
  echo "Usage: $0 <number_of_repetitions>"
  exit 1
fi

number_of_repetitions="$1"

# Run sbatch multiple times
for ((i = 1; i <= number_of_repetitions; i++)); do
  sbatch colmult
done

sleep "$((number_of_repetitions * 2))"
# Run the sh command with the specified output directory
sh script3.sh "$output_directory/outputs" "$output_directory/times.txt"

rm -rf "$output_directory/outputs"
mkdir "$output_directory/outputs"

