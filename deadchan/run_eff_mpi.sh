#!/bin/bash
#PBS -N efficiency
#PBS -q mpi
#####PBS -l nodes=173
#####PBS -l nodes=77
#PBS -l nodes=45
#PBS -l walltime=50:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.err
cd /home/itep/alekseev/igor/deadchan
export ROOT_DIR=/home/clusters/rrcmpi/alekseev/igor/root6n11/MC/DataTakingPeriod01/EffGd/
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair7n14/MC/DataTakingPeriod01/EffGd/
export FUEL_LIST=fuelGd.list
export VAR_DIR=varlists_2210_078234

for ((i=0; $i<173; i=$i + 45 )) ; do
	mpirun --mca btl ^tcp run_eff_mpi $i
done

root -l -q -b "calc_eff.C(173, \"_2210_078234\", \"$VAR_DIR\", 15000000)"
exit 0
