#!/bin/bash
#PBS -N run_sourcesC
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_sourcesC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_sourcesC.err
#PBS -l nodes=1
#PBS -l walltime=23:00:00
cd /home/itep/alekseev/igor/src_2025/NaCo

for n in 1 11 21 31 101 111 121; do
	root -l -b -q "draw_Sources6.C(${n}, \"root8n2\")"
	root -l -b -q "draw_Sources6.C(${n}, \"root8n6\")"
done
exit 0
