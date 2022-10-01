#!/bin/bash
#PBS -N run_phe_stripH
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_stripH.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_stripH.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=100:00:00

cd /home/itep/alekseev/igor/PlasticEvolution
date
./phe_hist_strip /home/clusters/rrcmpi/alekseev/igor/dvert/phe_103000_121999.root 103000 121999 50
date

exit 0
