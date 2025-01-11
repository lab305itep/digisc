#!/bin/bash
#PBS -N IBD-n1
#PBS -q medium
#PBS -l nodes=1
#PBS -l walltime=23:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n1.err
cd /home/itep/alekseev/igor/IBD-n
date
#./IBD-n Chikuma_paint_0_15.txt mc_ibd_n_Chikuma_paint_0_15.root
./IBD-n Chikuma_paint_0_45.txt mc_ibd_n_Chikuma_paint_0_45.root
date
exit 0
