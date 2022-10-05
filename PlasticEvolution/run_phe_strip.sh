#!/bin/bash
#PBS -N run_phe_stripI
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_stripI.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_phe_stripI.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=100:00:00

cd /home/itep/alekseev/igor/PlasticEvolution
date
./phe_hist_strip /home/clusters/rrcmpi/alekseev/igor/dvert/phex_83000_102999.root 83000 102999 50
date

exit 0
