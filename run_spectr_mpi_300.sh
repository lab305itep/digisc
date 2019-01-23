#!/bin/bash
#PBS -N digi_spectr_300
#PBS -q mpi
#PBS -l walltime=0:30:00
#PBS -l nodes=45
#PBS -l mem=4Gb
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/spectr_300.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/spectr_300.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
#export SPECTR_AUXCUT="!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))"
export SPECTR_BGSCALE=2.24
export SPECTR_NSECT=1
mpirun --mca btl ^tcp run_spectr_mpi 200
exit 0
