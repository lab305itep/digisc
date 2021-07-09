#!/bin/bash
#PBS -N deadtime
#PBS -q mpi
#PBS -l nodes=51
#PBS -l walltime=120:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.err
date
cd /home/itep/alekseev/igor
export DEAD_PROG=deadtime
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root8n1/deadtime
mpirun --mca btl ^tcp run_dead_mpi 2210 2000 /home/clusters/rrcmpi/alekseev/igor/root8n1
date
exit 0
