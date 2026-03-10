#!/bin/bash
#PBS -N IBD-n-exp
#PBS -q mpi
#PBS -l nodes=783
#PBS -l walltime=6:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n-exp.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/IBD-n-exp.err
cd /home/itep/alekseev/igor/IBD-n
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair8n7
date
mpirun --mca btl ^tcp ./run_IBD-n-exp_mpi
root -l -b -q "../add_periods.C(\"n-exp_87_Apr16_Aug25.root\", \"exp_87\")"
date
exit 0
