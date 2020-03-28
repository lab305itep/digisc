#!/bin/bash
#PBS -N efficiency
#PBS -q mpi
#PBS -l nodes=173
#PBS -l walltime=2:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.err
cd /home/itep/alekseev/igor/deadchan
export ROOT_DIR=/home/clusters/rrcmpi/alekseev/igor/root6n9/MC/DataTakingPeriod01/Eff/
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair7n11/MC/DataTakingPeriod01/Eff/
export FUEL_LIST=fuel.list
export VAR_DIR=varlists_2210_078234

mpirun --mca btl ^tcp run_eff_mpi

root -l -q -b "calc_eff.C(173, \"_2210_078234\", \"$PAIR_DIR\")"
exit 0
