#!/bin/bash
sbatch -N 1 --ntasks-per-node=1 jobscript
sbatch -N 1 --ntasks-per-node=2 jobscript
sbatch -N 1 --ntasks-per-node=4 jobscript
sbatch -N 1 --ntasks-per-node=8 jobscript
sbatch -N 1 --ntasks-per-node=16 jobscript
sbatch -N 1 --ntasks-per-node=32 jobscript
sbatch -N 1 --ntasks-per-node=64 jobscript
