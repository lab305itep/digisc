#!/bin/bash
#PBS -N efficiency
#PBS -q mpi
#PBS -l nodes=27
#PBS -l walltime=100:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.err
date
cd /home/itep/alekseev/igor/deadchan
export ROOT_DIR=/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Eff/
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair8n2/MC/Eff/
export FUEL_LIST=/home/itep/alekseev/igor/deadchan/fuel_v8.list
export VAR_DIR=var_fixed_v8.2

mpirun --mca btl ^tcp run_eff_mpi

date
exit 0
