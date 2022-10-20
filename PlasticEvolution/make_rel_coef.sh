#!/bin/bash
#PBS -N rel_coef
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/rel_coef.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/rel_coef.err
#PBS -l nodes=1
#PBS -q mpi
#PBS -l walltime=199:00:00
#PBS -l vmem=8Gb

DIR=/home/clusters/rrcmpi/alekseev/igor/dvert
cd /home/itep/alekseev/igor/PlasticEvolution
date
#for ((n=2; $n<47; n=$n+1)) ; do
#	for ((k=0; k<63; k=$k+1)) ; do
#		./make_rel_coef $n $k
#	done
#done

./make_rel_coef 2 1

date

exit 0
