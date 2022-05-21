#!/bin/bash
#PBS -N exp2mc
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/exp2mc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/exp2mc.err
#PBS -l nodes=81
#PBS -l walltime=3:00:00
cd /home/itep/alekseev/igor/exp2mc

mpirun --mca btl ^tcp run_many 'root -l -b -q'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.980, -0.100)"'\
\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.985, -0.100)"'\
\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.990, -0.100)"'\
\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 0.995, -0.100)"'\
\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.000, -0.100)"'\
\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.005, -0.100)"'\
\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.010, -0.100)"'\
\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.015, -0.100)"'\
\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020,  0.100)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020,  0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020,  0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020,  0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020,  0.000)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020, -0.025)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020, -0.075)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/basen1.8-calc.root\", \"hSum_Main\", 32.9, 1.020, -0.100)"'

exit 0
