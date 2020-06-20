#!/bin/bash
#PBS -N exp2mc
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/exp2mc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/exp2mc.err
#PBS -l nodes=23
#PBS -l walltime=1:00:00
cd /home/itep/alekseev/igor/exp2mc

mpirun --mca btl ^tcp run_many 'root -l -b -q'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.050)"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.125, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.100, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.075, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.050, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.025, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.000, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.025, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.050, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.075, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.100, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_single-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.125, \"PositronHits == 1\", \"1\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.125, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.100, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.075, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.050, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.025, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.000, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.025, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.050, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.075, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.100, \"PositronHits > 1\", \"N\")"'\
	'"exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mult-calc.root\", \"hSum_Main\", 34.0, 1.000,  0.120, \"PositronHits > 1\", \"N\")"'
exit 0
