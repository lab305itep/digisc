#!/bin/bash
#PBS -N run_phe_corr065
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_corr065.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_corr065.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=120:00:00

cd /home/itep/alekseev/igor/PlasticEvolution
date
./phe_hist phe_2000_121999_corr_0_65.root 2000 121999 100 -0.65
for adc in 9 10 11 18 19 29 30 31 32 ; do
	./phe_hist phe_2000_121999_${adc}_corr_0_65.root 2000 121999 200 -0.65 $adc
done
date

exit 0
