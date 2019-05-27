#!/bin/bash
#PBS -N run_sources
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_sources.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_sources.err
#PBS -l nodes=1
#PBS -l walltime=23:30:00
cd /home/itep/alekseev/igor/src_v4
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64

#LIST="1 2 11 12 101 102 111 112 1001 1101"
LIST="12 112"

for f in $LIST ; do
	root -l -b -q "draw_Sources2.C+($f, 1.00)"
	root -l -b -q "draw_Sources2.C+($f, 0.98)"
	root -l -b -q "draw_Sources2.C+($f, 0.96)"
	root -l -b -q "draw_Sources2.C+($f, 0.94)"
	root -l -b -q "draw_Sources2.C+($f, 0.92)"
done

exit 0
