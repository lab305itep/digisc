#!/bin/bash
#PBS -N stat_b
#PBS -q short
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/statb.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/statb.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=/usr/lib64/openmpi/lib:/opt/fairsoft/lib/root:/opt/gcc-5.3/lib64
export DANSSRAWREC_HOME=/home/clusters/rrcmpi/alekseev/igor/digi_home
root -l -q -b "digi_stat.C(34900,34999, \"0349xx_t0fixed_vetoT0_45-350\")" > stat_b.txt
exit 0
