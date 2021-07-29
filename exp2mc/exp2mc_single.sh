#!/bin/bash
#PBS -N exp2mc-1
#PBS -q short
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/exp2mc-1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/exp2mc-1.err
#PBS -l nodes=1
#PBS -l walltime=2:00:00
cd /home/itep/alekseev/igor/exp2mc

for ((r=96;$r<110;r=$r+1)) ; do
	rr=`printf "%d.%2.2d0" $((r/100)) $((r%100))`
	root -l -b -q "exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_apr21/base_single-calc.root\", \"hSum_Main\", 32.7, $rr, 0, \"PositronHits == 1\", \"one\")"
done
exit 0
