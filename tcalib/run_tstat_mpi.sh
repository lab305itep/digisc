#!/bin/bash
#PBS -N tstat
#PBS -q mpi
#PBS -l nodes=51
#PBS -l walltime=200:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/tstat.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/tstat.err
cd /home/itep/alekseev/igor/tcalib

mpirun --mca btl ^tcp run_tstat_mpi 74000 78234 hDT
mpirun --mca btl ^tcp run_tstat_mpi 74000 78234 hDTP

exit 0