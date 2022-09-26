#!/bin/bash
#PBS -N run_phe_strip
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_strip.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_strip.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=100:00:00

cd /home/itep/alekseev/igor/PlasticEvolution
date
./phe_hist_strip /home/clusters/rrcmpi/alekseev/igor/dvert/phe_2000_121999.root 2000 121999 500
date

exit 0
