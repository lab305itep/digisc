#!/bin/bash
#PBS -N run_digiMC1
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC1.err
#PBS -l nodes=1
#PBS -l walltime=23:30:00
cd /home/itep/alekseev/igor
. /home/clusters/rrcmpi/danss/bin/danss_profile.sh

#LIST=`find digi_MC/DataTakingPeriod0*/Fuel -name "*.digi.bz2" -print`
LIST=`find digi_MC/DataTakingPeriod01/Muons -name "mc_*.digi.bz2" -print`

for f in $LIST ; do
	OD=`dirname ${f/"digi_MC/"/}`
	mkdir -p /home/clusters/rrcmpi/alekseev/igor/root6n1/MC/$OD
	./evtbuilder5 $f 0x71000 /home/clusters/rrcmpi/alekseev/igor/root6n1/MC/$OD
	exit 0
done

exit 0
