#!/bin/bash
#PBS -N digi_processing_a
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/digia.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/digia.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=/usr/lib64/openmpi/lib:/opt/fairsoft/lib/root:/opt/gcc-5.3/lib64
export DANSSRAWREC_HOME=/home/clusters/rrcmpi/alekseev/igor/digi_home
export DIGI_SUFFIX=0349xx_t0fixed
mpirun --mca btl ^tcp run_digi_mpi 34900
exit 0
