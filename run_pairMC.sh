#!/bin/bash
#PBS -N run_pairMC
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.err
#PBS -l nodes=1
#PBS -l walltime=23:30:00
cd /home/itep/alekseev/igor
LIST=`find /home/clusters/rrcmpi/alekseev/igor/root6n10/MC/DataTakingPeriod01/Fuel* -name "*.root" -print`

for f in $LIST ; do
	OF=${f/root6n10/pair7n13}
	./pairbuilder8 $f $OF
done

exit 0
