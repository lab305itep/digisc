#!/bin/bash
#PBS -N efficiency
#PBS -q mpi
#PBS -l nodes=19
#PBS -l walltime=50:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/efficiency.err
date
cd /home/itep/alekseev/igor/deadchan
export ROOT_DIR=/home/clusters/rrcmpi/alekseev/igor/root6n12/MC/DataTakingPeriod01/EffGd/
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair7n15/MC/DataTakingPeriod01/EffGd/
export FUEL_LIST=/home/itep/alekseev/igor/deadchan/fuelGd.list
export VAR_DIR=var_fixed

### for ((i=0; $i<173; i=$i + 45 )) ; do
	mpirun --mca btl ^tcp run_eff_mpi
### done

## root -l -q -b "calc_eff.C(19, \"2210_78234\", \"$VAR_DIR\", 15000000)"
date
exit 0
