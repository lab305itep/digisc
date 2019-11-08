#!/bin/bash
#PBS -N hittree_all4
#PBS -q mpi
#PBS -l nodes=1
#PBS -l walltime=24:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_all4.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_all4.err
cd /home/itep/alekseev/igor
export SRC_DIR=/home/clusters/rrcmpi/alekseev/igor/root6n5 
export TGT_DIR=/home/clusters/rrcmpi/alekseev/igor/root6n6
export HITS_DIR=/home/clusters/rrcmpi/alekseev/igor/hitcheck
# We run at ~120 files/hour
mpirun --mca btl ^tcp run_hits_mpi 62000 50
exit 0
