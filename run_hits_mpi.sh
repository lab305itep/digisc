#!/bin/bash
#PBS -N hittree_all1
#PBS -q mpi
#PBS -l nodes=7
#PBS -l walltime=3:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_all1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_all1.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
#mpirun --mca btl ^tcp run_hits_mpi 41385 43
mpirun --mca btl ^tcp run_hits_mpi 45700 41
exit 0
