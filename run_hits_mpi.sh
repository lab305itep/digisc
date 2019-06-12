#!/bin/bash
#PBS -N hittree_all
#PBS -q mpi
#PBS -l nodes=220
#PBS -l walltime=24:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_all.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_all.err
cd /home/itep/alekseev/igor
export SRC_DIR=/home/clusters/rrcmpi/alekseev/igor/root6n3 
export TGT_DIR=/home/clusters/rrcmpi/alekseev/igor/root6n4
# We run at ~120 files/hour
mpirun --mca btl ^tcp run_hits_mpi 2000 250
exit 0
