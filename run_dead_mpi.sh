#!/bin/bash
#PBS -N deadtime
#PBS -q mpi
#PBS -l nodes=1
#PBS -l walltime=40:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadtime_calc.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
mpirun --mca btl ^tcp run_dead_mpi 45700 287 /home/clusters/rrcmpi/alekseev/igor/root6n2
exit 0
