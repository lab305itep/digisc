#!/bin/bash
#PBS -N mk_tcalib
#PBS -q xxl
#PBS -l nodes=1
#PBS -l walltime=500:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/mk_tcalib.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/mk_tcalib.err
cd /home/itep/alekseev/igor/tcalib

let run_first=2210
declare -a BLIST
BLIST=(	5000   6836   7639   9247   11579  15999  17444  19257  25474  26552 \
	26958  27865  29522  31117  32184  36999  64045  69764  70770  98999 \
	99797  100303 100600 102850 104996 105474 105650 105979 106300 107999 \
	109600 110801 113968 134000 135484)
let BLAST=${#BLIST[@]}-1
let IFIRST=$BLAST

MUTDIR=/home/clusters/rrcmpi/alekseev/igor/mut8n2

for i in ${!BLIST[@]}; do
	if (($i == 0)) ; then
		let A=$run_first
	else
		let j=$i-1
		let A=${BLIST[$j]}+1
	fi
	let B=${BLIST[$i]}
	if (($i >= $IFIRST)) ; then
		CALIB=`printf "$MUTDIR/hist/calib_%6.6d_%6.6d.bin" $A $B`
		./ana_tcalib -p $A $B $MUTDIR
		for ((j=0;$j<4;j=$j+1)) ; do
			./ana_tcalib -p $A $B $MUTDIR $CALIB
		done
		./ana_tcalib $A $B $MUTDIR $CALIB
	fi
done

exit 0
