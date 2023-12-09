#!/bin/bash

# Define default values
nodes=4
cores=16
output_dir="OUTPUT_DIR"
job_to_run="grid_mult"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -n|--nodes)
            nodes="$2"
            shift 2
            ;;
        -c|--cores)
            cores="$2"
            shift 2
            ;;
        -o|--output)
            output_dir="$2"
            shift 2
            ;;
        -j|--job)
            job_to_run="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Create the job script dynamically
job_script_2="#!/bin/bash
#SBATCH -p rome16q # partition (queue)
#SBATCH -N $nodes # number of nodes
#SBATCH --ntasks-per-node $cores  # number of cores
#SBATCH -t 2-12:00 # time (D-HH:MM)
#SBATCH -o results/n${nodes}/c${cores}/${job_to_run}.txt
#SBATCH --open-mode=append
#SBATCH --exclusive

module load openmpi-4.0.5
srun build/debug/${job_to_run}
"

# Save the job script to a temporary file
job_script_file2="job_script_temp2.sh"
echo "$job_script_2" > "$job_script_file2"

# Submit the job script to Slurm
sbatch "$job_script_file2"

# Clean up the temporary job script file
rm "$job_script_file2"

