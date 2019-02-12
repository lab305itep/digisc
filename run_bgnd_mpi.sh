#!/bin/bash
#PBS -N background_calc
#PBS -q mpi
#PBS -l nodes=245
#PBS -l walltime=2:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/background_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/background_calc.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
export OUTPUT_DIR="/home/clusters/rrcmpi/alekseev/igor/bgnd_v3"
# export AUX_CUT="PositronEnergy > 3"
mpirun --mca btl ^tcp run_bgnd_mpi
exit 0
