#!/bin/bash
#PBS -N efficiency241Pu
#PBS -q mpi
#PBS -l nodes=23
#PBS -l walltime=50:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency241Pu.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency241Pu.err
date
cd /home/itep/alekseev/igor/deadchan
export ROOT_DIR=/home/clusters/rrcmpi/alekseev/igor/root8n1/MC/Eff/241Pu
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair8n1/MC/Eff/241Pu
export FUEL_LIST=/home/itep/alekseev/igor/deadchan/fuel_241Pu_v8.1.list
export VAR_DIR=var_fixed_241Pu_v8.1

mpirun --mca btl ^tcp run_eff_mpi

date
exit 0
