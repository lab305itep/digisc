#!/bin/bash
#PBS -N cm_14487
#PBS -q short
#PBS -l nodes=1
#PBS -l walltime=4:30:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/cm_14487.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/cm_14487.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
./cmbuilder cm_14487_14512.list cm_14487_14512.root
exit 0
