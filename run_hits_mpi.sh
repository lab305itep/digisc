#!/bin/bash
#PBS -N hittree_all
#PBS -q mpi
#PBS -l nodes=523
#PBS -l walltime=3:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_all.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_all.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
mpirun --mca btl ^tcp run_hits_mpi 2210 100
exit 0
