#!/bin/bash
#PBS -N stat_n
#PBS -q short
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/statn.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/statn.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=/usr/lib64/openmpi/lib:/opt/fairsoft/lib/root:/opt/gcc-5.3/lib64
export DANSSRAWREC_HOME=/home/clusters/rrcmpi/alekseev/igor/digi_home
root -l -q -b "digi_stat.C(34900,34999)" > stat_norm.txt
exit 0
