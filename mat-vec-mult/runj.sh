#!/bin/bash

# Initialize variables with default values
n_value=""
c_value=""
last_arg=""
script_choice=""

while getopts "n:c:s:" opt; do
  case $opt in
    n)
      n_value="$OPTARG"
      ;;
    c)
      c_value="$OPTARG"
      ;;
    s)
      script_choice="$OPTARG"
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

shift $((OPTIND-1))

# Validate the script_choice argument
if [ "$script_choice" != "1" ] && [ "$script_choice" != "2" ]; then
  echo "Invalid script choice. Use 1 for runjob1 or 2 for runjob2." >&2
  exit 1
fi

# Determine the script to run based on script_choice
if [ "$script_choice" == "1" ]; then
  script_to_run="runjob1.sh"
elif [ "$script_choice" == "2" ]; then
  script_to_run="runjob2.sh"
fi

# Loop through the last arguments you want to run
for last_arg in "col" "row" "grid"
do
  sh "$script_to_run" -n "$n_value" -c "$c_value" -o "rowmult_res" -j "$last_arg"
done
