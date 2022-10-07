#!/bin/bash
#PBS -N run_phe_stripJ
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_stripJ.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_stripJ.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=100:00:00

cd /home/itep/alekseev/igor/PlasticEvolution
date
./phe_hist_strip /home/clusters/rrcmpi/alekseev/igor/dvert/phex_103000_121999.root 103000 121999 50
date

exit 0
