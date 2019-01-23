#!/bin/bash
#PBS -N xyz
#PBS -q long
#PBS -l nodes=1
#PBS -l walltime=24:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/xyz.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/xyz.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
./xyz 5907 36408 /home/clusters/rrcmpi/alekseev/igor/tmp/xyz.root
exit 0
