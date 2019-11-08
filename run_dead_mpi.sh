#!/bin/bash
#PBS -N deadtime
#PBS -q mpi
#PBS -l nodes=25
#PBS -l walltime=80:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.err
cd /home/itep/alekseev/igor
export DEAD_PROG=deadtime
mpirun --mca btl ^tcp run_dead_mpi 60000 200 /home/clusters/rrcmpi/alekseev/igor/root6n6
exit 0
