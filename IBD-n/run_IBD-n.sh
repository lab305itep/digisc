#!/bin/bash
#PBS -N IBD-n1
#PBS -q medium
#PBS -l nodes=1
#PBS -l walltime=10:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n1.err
cd /home/itep/alekseev/igor/IBD-n
date
#./IBD-n Chikuma_FIFRELIN2.txt mc_ibd_n_Chikuma_FIFRELIN2.root
./IBD-n Chikuma_main_Birks_0_0308.txt mc_ibd_n_Chikuma_main_Birks_0_0308.root
date
exit 0
