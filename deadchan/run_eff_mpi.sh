#!/bin/bash
#PBS -N efficiency
#PBS -q mpi
#PBS -l nodes=36
#PBS -l walltime=150:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.err
date
cd /home/itep/alekseev/igor/deadchan
export ROOT_DIR=/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Eff_ind/
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair8n2/MC/Eff_ind/
## 60 files
export FUEL_LIST=/home/itep/alekseev/igor/deadchan/fuel_largeA.list
export VAR_DIR=varlist_2210_163748

mpirun --mca btl ^tcp run_eff_mpi

date
exit 0
