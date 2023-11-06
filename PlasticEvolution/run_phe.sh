#!/bin/bash
#PBS -N run_phe
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=120:00:00

cd /home/itep/alekseev/igor/PlasticEvolution
date
for alpha in -1.0 -0.8 -0.6 -0.4 -0.2 0 0.2 0.4 0.6 0.8 1.0 ; do
	./phe_hist phe_2200_142799a${alpha}.root 2200 142799 100 $alpha
done
date

exit 0
