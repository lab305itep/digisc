#!/bin/bash
#PBS -N run_pairMC
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.err
#PBS -l nodes=1
#PBS -l walltime=23:30:00
cd /home/itep/alekseev/igor
#LIST=`find /home/clusters/rrcmpi/alekseev/igor/root6n3/MC/DataTakingPeriod01/Shielding -name "*.root" -print`
LIST=`find /home/clusters/rrcmpi/alekseev/igor/root6n3/MC/DataTakingPeriod01/Fuel -name "*.root" -print`

for f in $LIST ; do
	OF=${f/root6n3/pair7n4}
	mkdir -p `dirname $OF`
	./pairbuilder7 $f $OF
done

exit 0
