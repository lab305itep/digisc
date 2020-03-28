#!/bin/bash
#PBS -N background_calc
#PBS -q mpi
#PBS -l nodes=433
#PBS -l walltime=2:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/background_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/background_calc.err
cd /home/itep/alekseev/igor
export OUT_DIR="/home/clusters/rrcmpi/alekseev/igor/apr16_dec19/bgnd.data"
mkdir -p $OUT_DIR
rm -f $OUT_DIR/*.root
export PAIR_DIR="/home/clusters/rrcmpi/alekseev/igor/pair7n11"
mpirun --mca btl ^tcp run_bgnd_mpi

NAME=${OUT_DIR/.data/.root}
MCNAME=${OUT_DIR/bgnd.data/mc_ibd.root}
root -l -b -q "add_periods.C(\"${NAME}\", \"$OUT_DIR\")"
root -l -b -q "background_draw.C(\"${NAME}\", \"${MCNAME}\")"

exit 0
