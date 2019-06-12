#!/bin/bash
#PBS -N digi_stat
#PBS -q mpi
#PBS -l nodes=500
#PBS -l walltime=10:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/stat.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/stat.err
cd /home/itep/alekseev/igor
mpirun --mca btl ^tcp run_stat_mpi 2000 110
exit 0
