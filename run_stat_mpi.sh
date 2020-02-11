#!/bin/bash
#PBS -N digi_stat
#PBS -q mpi
#PBS -l nodes=70
#PBS -l walltime=50:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/stat.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/stat.err
cd /home/itep/alekseev/igor
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root6n8/stat
mpirun --mca btl ^tcp run_stat_mpi 2210 1000
exit 0
