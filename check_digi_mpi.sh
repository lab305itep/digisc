#!/bin/bash
#PBS -N check_digi_rad
#PBS -q mpi
#PBS -l nodes=100
#PBS -l walltime=100:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/check_digi_rad.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/check_digi_rad.err
mpirun --mca btl ^tcp check_digi_mpi /home/clusters/rrcmpi/danss/DANSS/digi_Real/digi_fullProc/v3.0/radSources
exit 0
