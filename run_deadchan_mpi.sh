#!/bin/bash
#PBS -N deadchan
#PBS -q mpi
#PBS -l nodes=10
#PBS -l walltime=40:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadchan_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadchan_calc.err
cd /home/itep/alekseev/igor
export DEAD_PROG=deadchannels
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root6n9/deadchan
mpirun --mca btl ^tcp run_dead_mpi 74721 352 /home/clusters/rrcmpi/alekseev/igor/root6n9
exit 0
