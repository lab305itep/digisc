#!/bin/bash
#PBS -N digi_spectr
#PBS -q mpi
#PBS -l walltime=0:30:00
#PBS -l nodes=323
#PBS -l pvmem=2Gb
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/spectr.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/spectr.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
#export SPECTR_AUXCUT="!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))"
export SPECTR_BGSCALE=2.64
export SPECTR_NSECT=1
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair7n1
export OUT_DIR=/home/clusters/rrcmpi/alekseev/igor/apr16_jan19
mpirun --mca btl ^tcp run_spectr_mpi
exit 0
