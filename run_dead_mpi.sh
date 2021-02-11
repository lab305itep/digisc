#!/bin/bash
#PBS -N deadtime
#PBS -q short
#PBS -l nodes=1
#PBS -l walltime=1:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.err
date
cd /home/itep/alekseev/igor
export DEAD_PROG=deadtime
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root6n12/deadtime
mpirun --mca btl ^tcp run_dead_mpi 81739 1 /home/clusters/rrcmpi/alekseev/igor/root6n12
date
exit 0
