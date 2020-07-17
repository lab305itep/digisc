#!/bin/bash
#PBS -N digi_stat
#PBS -q short
#PBS -l nodes=1
#PBS -l walltime=4:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/stat.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/stat.err
date
cd /home/itep/alekseev/igor
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root6n12/stat
mpirun --mca btl ^tcp run_stat_mpi 45445 55
date
exit 0
