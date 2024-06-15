#!/bin/bash
#PBS -N IBD-n
#PBS -q medium
#PBS -l nodes=1
#PBS -l walltime=10:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n.err
cd /home/itep/alekseev/igor/IBD-n
date
./IBD-n Chikuma_FIFRELIN2.txt mc_ibd_n_FIFRELIN2.root
date
exit 0
