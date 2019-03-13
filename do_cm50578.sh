#!/bin/bash
#PBS -N cm_50578
#PBS -q short
#PBS -l nodes=1
#PBS -l walltime=4:30:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/cm_50578.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/cm_50578.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
./cmbuilder cm_50578_50647.list cm_50578_50647.root
exit 0
