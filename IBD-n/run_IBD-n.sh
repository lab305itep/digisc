#!/bin/bash
#PBS -N IBD-n
#PBS -q long
#PBS -l nodes=1
#PBS -l walltime=100:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n.err
cd /home/itep/alekseev/igor/IBD-n
date
#for f in Birks_0_005 Cher_coeff_0_05 paint_0_2 paint_0_3 ; do
for f in paint_0_2 paint_0_3 ; do
	./IBD-n Fuso_${f}.txt mc_ibd_n_Fuso_${f}.root
done
date
exit 0
