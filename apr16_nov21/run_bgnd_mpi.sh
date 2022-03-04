#!/bin/bash
#PBS -N background_calc
#PBS -q mpi
#PBS -l nodes=643
#PBS -l pmem=3Gb
#PBS -l walltime=150:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/background_calc.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/background_calc.err
cd /home/itep/alekseev/igor
MY_DIR="/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/"
MY_PE="PositronEnergy"
export STAT_ALL=stat_all.txt
export PAIR_DIR="/home/clusters/rrcmpi/alekseev/igor/pair8n2"

for var in ALL 05_1 1_2 2_3 3_4 4_5 5_6 6_8 ; do
	case $var in
	ALL )  AUX_CUT="$MY_PE > 0.75 && $MY_PE < 8"	;;
	05_1 ) AUX_CUT="$MY_PE > 0.5 && $MY_PE < 1"	;;
	1_2 )  AUX_CUT="$MY_PE > 1 && $MY_PE < 2"	;;
	2_3 )  AUX_CUT="$MY_PE > 2 && $MY_PE < 3"	;;
	3_4 )  AUX_CUT="$MY_PE > 3 && $MY_PE < 4"	;;
	4_5 )  AUX_CUT="$MY_PE > 4 && $MY_PE < 5"	;;
	5_6 )  AUX_CUT="$MY_PE > 5 && $MY_PE < 6"	;;
	6_8 )  AUX_CUT="$MY_PE > 6 && $MY_PE < 8"	;;
	esac
	OUT_DIR=${MY_DIR}bgnd_${var}.data
	NAME=${MY_DIR}bgnd_${var}.root
	MCNAME=${MY_DIR}mc_ibd_v8.2_${var}.root
	mkdir -p $OUT_DIR
	rm -f $OUT_DIR/*.root
	export OUT_DIR AUX_CUT
	mpirun --mca btl ^tcp run_bgnd_mpi
	./background_MC apr16_nov21/ibd_list.txt $MCNAME
	root -l -b -q "add_periods.C(\"${NAME}\", \"$OUT_DIR\")"
	root -l -b -q "background_draw.C(\"${NAME}\", \"${MCNAME}\", 0.04)"
done

exit 0
