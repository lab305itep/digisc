#!/bin/bash
#PBS -N digi_stat1
#PBS -q mpi
#PBS -l nodes=1
#PBS -l walltime=10:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/stat1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/stat1.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
mpirun --mca btl ^tcp run_stat_mpi 45700 287
exit 0
