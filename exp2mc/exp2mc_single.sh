#!/bin/bash
#PBS -N exp2mc-1
#PBS -q short
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/exp2mc-1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/exp2mc-1.err
#PBS -l nodes=1
#PBS -l walltime=2:00:00
cd /home/itep/alekseev/igor/exp2mc

root -l -b -q "exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2-calc.root\", \"hSum_Main\", 34.0, 1.005, -0.050)"
root -l -b -q "exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2-calc.root\", \"hSum_Main\", 34.0, 1.000, -0.050)"
root -l -b -q "exp2mc.C(\"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2-calc.root\", \"hSum_Main\", 34.0, 0.995, -0.050)"

exit 0
