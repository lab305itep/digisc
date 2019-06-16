#!/bin/bash
#PBS -N background_calc
#PBS -q mpi
#PBS -l nodes=359
#PBS -l walltime=2:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/background_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/background_calc.err
cd /home/itep/alekseev/igor
NAME=bgnd_v4
export OUT_DIR="/home/clusters/rrcmpi/alekseev/igor/$NAME"
mkdir -p $OUT_DIR
export PAIR_DIR="/home/clusters/rrcmpi/alekseev/igor/pair7n4"
mpirun --mca btl ^tcp run_bgnd_mpi

root -l -b -q "add_periods.C(\"${NAME}.root\", \"$OUT_DIR\")"
root -l -b -q "background_draw.C(\"${NAME}.root\")"

exit 0
