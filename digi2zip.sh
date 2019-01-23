#!/bin/bash
#PBS -N digi_zip
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/zip.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/zip.err
#PBS -l nodes=106
#PBS -l walltime=24:00:00
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
export DANSSRAWREC_HOME=/home/itep/alekseev/igor/digi.v2/
mpirun --mca btl ^tcp digi_zip 5000
exit 0
