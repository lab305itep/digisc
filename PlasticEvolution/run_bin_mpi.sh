#!/bin/bash
#PBS -N run_bin_mpi
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_bin_mpi.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_bin_mpi.err
#PBS -l nodes=48
#PBS -l walltime=100:00:00
cd /home/itep/alekseev/igor/PlasticEvolution
mpirun --mca btl ^tcp run_bin_mpi
exit 0
