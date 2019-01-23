#!/bin/bash
#PBS -N hello_mpi
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/test1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/test1.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
mpirun --mca btl ^tcp run_digi_mpi
exit 0
