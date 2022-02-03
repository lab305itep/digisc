#!/bin/bash
#PBS -N ana_tcalib
#PBS -q long
#PBS -l nodes=1
#PBS -l walltime=100:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/ana_tcalib.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/ana_tcalib.err
cd /home/itep/alekseev/igor/tcalib

let run_first=2210
let run_last=110801
let step=50
MUTDIR=/home/clusters/rrcmpi/alekseev/igor/mut8n2

for ((run=$run_first;$run<=$run_last;run=$run+$step)) ; do
	let A=$run
	let B=$run+$step-1
	CALIB=`printf "$MUTDIR/hist/calib_%6.6d_%6.6d.bin" $A $B`
	./ana_tcalib $A $B $MUTDIR
	./ana_tcalib $A $B $MUTDIR $CALIB
	./ana_tcalib $A $B $MUTDIR $CALIB
done

exit 0
