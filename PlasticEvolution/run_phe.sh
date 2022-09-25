#!/bin/bash
#PBS -N run_phe
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=100:00:00

cd /home/itep/alekseev/igor/PlasticEvolution
date
./phe_hist phe_2000_121999.root 2000 121999 200
for adc in 9 10 11 18 19 29 30 31 32 ; do
	./phe_hist phe_2000_121999_${adc}.root 2000 121999 200 $adc
done
date

exit 0
