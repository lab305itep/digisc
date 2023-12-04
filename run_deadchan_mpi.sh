#!/bin/bash
#PBS -N deadchan
#PBS -q mpi
#PBS -l nodes=21
#PBS -l walltime=100:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/deadchan_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/deadchan_calc.err
date
cd /home/itep/alekseev/igor
export DEAD_PROG=deadchannels
mkdir -p /home/clusters/rrcmpi/alekseev/igor/root8n2/deadchan
mpirun --mca btl ^tcp run_dead_mpi 142816 300 /home/clusters/rrcmpi/alekseev/igor/root8n2
date
exit 0
