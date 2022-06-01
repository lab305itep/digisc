#!/bin/bash
#PBS -N IBD-n
#PBS -q medium
#PBS -l nodes=1
#PBS -l walltime=10:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n.err
cd /home/itep/alekseev/igor/IBD-n
date
./IBD-n ibd_list.txt mc_ibd_n.root
date
exit 0
