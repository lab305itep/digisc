#!/bin/bash
#PBS -N run_digiMC
#PBS -q long
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.err
#PBS -l nodes=1
#PBS -l walltime=100:30:00
cd /home/itep/alekseev/igor
. /home/clusters/rrcmpi/danss/bin/danss_profile.sh

LIST=`find digi_MC/ -name "*.digi.bz2" -print`
#LIST=`find digi_MC/DataTakingPeriod01/Shielding/ -name "mc_*.digi.bz2" -print`

for f in $LIST ; do
	OD=`dirname ${f/"digi_MC/"/}`
	mkdir -p /home/clusters/rrcmpi/alekseev/igor/root6n5/MC/$OD
	./evtbuilder5 $f 0x70000 /home/clusters/rrcmpi/alekseev/igor/root6n5/MC/$OD
done

exit 0
