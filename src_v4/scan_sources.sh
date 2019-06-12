#!/bin/bash
#PBS -N scan_sources
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/scan_sources.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/scan_sources.err
#PBS -l nodes=86
#PBS -l walltime=10:00:00
cd /home/itep/alekseev/igor/src_v4

mpirun --mca btl ^tcp run_many 'root -l -b -q'\
	'"draw_Sources2.C(11, 0.880)"' '"draw_Sources2.C(11, 0.885)"' '"draw_Sources2.C(11, 0.890)"' '"draw_Sources2.C(11, 0.895)"' \
	'"draw_Sources2.C(11, 0.900)"' '"draw_Sources2.C(11, 0.905)"' '"draw_Sources2.C(11, 0.910)"' '"draw_Sources2.C(11, 0.915)"' \
	'"draw_Sources2.C(11, 0.920)"' '"draw_Sources2.C(11, 0.925)"' '"draw_Sources2.C(11, 0.930)"' '"draw_Sources2.C(11, 0.935)"' \
	'"draw_Sources2.C(11, 0.940)"' '"draw_Sources2.C(11, 0.945)"' '"draw_Sources2.C(11, 0.950)"' '"draw_Sources2.C(11, 0.955)"' \
	'"draw_Sources2.C(11, 0.960)"' '"draw_Sources2.C(11, 0.965)"' '"draw_Sources2.C(11, 0.970)"' '"draw_Sources2.C(11, 0.975)"' \
	'"draw_Sources2.C(11, 0.980)"' '"draw_Sources2.C(11, 0.985)"' '"draw_Sources2.C(11, 0.990)"' '"draw_Sources2.C(11, 0.995)"' \
	'"draw_Sources2.C(11, 1.000)"' '"draw_Sources2.C(11, 1.005)"' \
	'"draw_Sources2.C(111, 0.880)"' '"draw_Sources2.C(111, 0.885)"' '"draw_Sources2.C(111, 0.890)"' '"draw_Sources2.C(111, 0.895)"' \
	'"draw_Sources2.C(111, 0.900)"' '"draw_Sources2.C(111, 0.905)"' '"draw_Sources2.C(111, 0.910)"' '"draw_Sources2.C(111, 0.915)"' \
	'"draw_Sources2.C(111, 0.920)"' '"draw_Sources2.C(111, 0.925)"' '"draw_Sources2.C(111, 0.930)"' '"draw_Sources2.C(111, 0.935)"' \
	'"draw_Sources2.C(111, 0.940)"' '"draw_Sources2.C(111, 0.945)"' '"draw_Sources2.C(111, 0.950)"' '"draw_Sources2.C(111, 0.955)"' \
	'"draw_Sources2.C(111, 0.960)"' '"draw_Sources2.C(111, 0.965)"' '"draw_Sources2.C(111, 0.970)"' '"draw_Sources2.C(111, 0.975)"' \
	'"draw_Sources2.C(111, 0.980)"' '"draw_Sources2.C(111, 0.985)"' '"draw_Sources2.C(111, 0.990)"' '"draw_Sources2.C(111, 0.995)"' \
	'"draw_Sources2.C(111, 1.000)"' '"draw_Sources2.C(111, 1.005)"' \
	'"draw_Sources2.C(1001, 0.00)"' '"draw_Sources2.C(1001, 0.02)"' '"draw_Sources2.C(1001, 0.04)"' '"draw_Sources2.C(1001, 0.06)"' \
	'"draw_Sources2.C(1001, 0.08)"' '"draw_Sources2.C(1001, 0.10)"' '"draw_Sources2.C(1001, 0.12)"' '"draw_Sources2.C(1001, 0.14)"' \
	'"draw_Sources2.C(1001, 0.16)"' '"draw_Sources2.C(1001, 0.18)"' '"draw_Sources2.C(1001, 0.20)"' '"draw_Sources2.C(1001, 0.22)"' \
	'"draw_Sources2.C(1001, 0.24)"' '"draw_Sources2.C(1001, 0.26)"' '"draw_Sources2.C(1001, 0.28)"' '"draw_Sources2.C(1001, 0.30)"' \
	'"draw_Sources2.C(1001, 0.32)"' \
	'"draw_Sources2.C(1101, 0.00)"' '"draw_Sources2.C(1101, 0.02)"' '"draw_Sources2.C(1101, 0.04)"' '"draw_Sources2.C(1101, 0.06)"' \
	'"draw_Sources2.C(1101, 0.08)"' '"draw_Sources2.C(1101, 0.10)"' '"draw_Sources2.C(1101, 0.12)"' '"draw_Sources2.C(1101, 0.14)"' \
	'"draw_Sources2.C(1101, 0.16)"' '"draw_Sources2.C(1101, 0.18)"' '"draw_Sources2.C(1101, 0.20)"' '"draw_Sources2.C(1101, 0.22)"' \
	'"draw_Sources2.C(1101, 0.24)"' '"draw_Sources2.C(1101, 0.26)"' '"draw_Sources2.C(1101, 0.28)"' '"draw_Sources2.C(1101, 0.30)"' \
	'"draw_Sources2.C(1101, 0.32)"' 
exit 0
