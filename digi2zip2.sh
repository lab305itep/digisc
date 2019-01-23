#!/bin/bash
#PBS -N digi_zip2
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/zip2.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/zip2.err
#PBS -l nodes=66
#PBS -l walltime=24:00:00
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
export DANSSRAWREC_HOME=/home/itep/alekseev/igor/digi.v2/
mpirun --mca btl ^tcp digi_zip 2000
exit 0
