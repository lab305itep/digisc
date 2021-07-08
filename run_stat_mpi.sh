#!/bin/bash
#PBS -N digi_stat
#PBS -q mpi
#PBS -l nodes=202
#PBS -l walltime=12:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/stat.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/stat.err
date
cd /home/itep/alekseev/igor
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root8n1/stat
mpirun --mca btl ^tcp run_stat_mpi 2210 500
date
exit 0
