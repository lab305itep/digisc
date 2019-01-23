#!/bin/bash
#PBS -N pmt2sipm
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/pmt2sipm.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/pmt2sipm.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
mpirun --mca btl ^tcp run_pmt2sipm_mpi 5600 500 /home/clusters/rrcmpi/alekseev/igor/pmt2sipm
exit 0
