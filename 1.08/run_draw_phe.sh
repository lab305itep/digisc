#!/bin/bash
#PBS -N mu-calib
#PBS -q mpi
#PBS -l nodes=441
#PBS -l walltime=2:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/calib_mu_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/calib_mu_calc.err
cd /home/itep/alekseev/igor/1.08
mpirun --mca btl ^tcp run_draw_phe

exit 0
