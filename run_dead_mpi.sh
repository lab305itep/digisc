#!/bin/bash
#PBS -N deadtime
#PBS -q mpi
#PBS -l nodes=9
#PBS -l walltime=100:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc8n2.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc8n2.err
date
cd /home/itep/alekseev/igor
export DEAD_PROG=deadtime
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root8n2/deadtime
mpirun --mca btl ^tcp run_dead_mpi 154798 1000 /home/clusters/rrcmpi/alekseev/igor/root8n2
date
exit 0
