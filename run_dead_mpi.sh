#!/bin/bash
#PBS -N deadtime
#PBS -q mpi
#PBS -l nodes=50
#PBS -l walltime=4:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
mpirun --mca btl ^tcp run_dead_mpi 2000 1000 /home/clusters/rrcmpi/alekseev/igor/root6n1
exit 0
