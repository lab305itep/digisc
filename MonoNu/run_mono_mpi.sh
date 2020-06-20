#!/bin/bash
#PBS -N MonoNu
#PBS -q mpi
#PBS -l nodes=9
#PBS -l walltime=72:00:00
#PBS -l pmem=3Gb
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/MonoNu.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/MonoNu.err
cd /home/itep/alekseev/igor/MonoNu

###export REBUILD=YES
export ROOTVER=6n11
export PAIRVER=7n14
export MONO_ARG00="0.06 0.02 0.98"
export MONO_ARG01="0.12 0.04 0.98"
export MONO_ARG02="0.18 0.06 0.98"
export MONO_ARG03="0.06 0.02 1.00"
export MONO_ARG04="0.12 0.04 1.00"
export MONO_ARG05="0.18 0.06 1.00"
export MONO_ARG06="0.06 0.02 1.02"
export MONO_ARG07="0.12 0.04 1.02"
export MONO_ARG08="0.18 0.06 1.02"

mpirun --mca btl ^tcp run_mono_mpi

exit 0
