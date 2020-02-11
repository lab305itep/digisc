#!/bin/bash
#PBS -N deadtime
#PBS -q mpi
#PBS -l nodes=70
#PBS -l walltime=100:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.err
cd /home/itep/alekseev/igor
export DEAD_PROG=deadtime
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root6n8/deadtime
mpirun --mca btl ^tcp run_dead_mpi 2210 1000 /home/clusters/rrcmpi/alekseev/igor/root6n8
exit 0
