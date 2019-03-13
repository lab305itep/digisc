#!/bin/bash
#PBS -N run_digiMC
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.err
#PBS -l nodes=1
#PBS -l walltime=23:30:00
cd /home/itep/alekseev/igor
LIST=`find digi_MC/DataTakingPeriod01/MonoPositronsLargeStat -name "*_05-*.digi.bz2" -print`

for f in $LIST ; do
	OD=`dirname ${f/"digi_MC/"/}`
	
	./evtbuilder5 $f 0x71000 /home/clusters/rrcmpi/alekseev/igor/root6n1/MC/$OD
done

exit 0
