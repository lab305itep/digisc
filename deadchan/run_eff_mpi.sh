#!/bin/bash
#PBS -N efficiency
#PBS -q mpi
#PBS -l nodes=173
#PBS -l walltime=24:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.err
cd /home/itep/alekseev/igor/deadchan
export ROOT_DIR=/home/clusters/rrcmpi/alekseev/igor/root6n7/MC/DataTakingPeriod01/Eff/
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair7n8/MC/DataTakingPeriod01/Eff/
export FUEL_LIST=fuel.list
export VAR_DIR=vars_4.0

mpirun --mca btl ^tcp run_eff_mpi
exit 0
