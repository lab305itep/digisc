#!/bin/bash
#PBS -N hittree_11
#PBS -q mpi
#PBS -l nodes=20
#PBS -l walltime=1:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_11.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_11.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
mpirun --mca btl ^tcp run_hits_mpi 11000 100
exit 0
