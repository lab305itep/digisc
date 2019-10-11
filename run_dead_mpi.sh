#!/bin/bash
#PBS -N deadtime
#PBS -q mpi
#PBS -l nodes=10
#PBS -l walltime=40:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.err
cd /home/itep/alekseev/igor
export DEAD_PROG=deadtime
mpirun --mca btl ^tcp run_dead_mpi 59260 152 /home/clusters/rrcmpi/alekseev/igor/root6n4
exit 0
