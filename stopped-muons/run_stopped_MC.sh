#!/bin/bash
#PBS -N stopped_MC
#PBS -q long
#PBS -l walltime=100:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/stopped_MC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/stopped_MC.err
cd /home/itep/alekseev/igor/stopped-muons

BASE=/home/clusters/rrcmpi/alekseev/igor/root8n2
CHIKUMA=MC/Chikuma/Muons
LIST=(	Hit_checker_Chikuma \
	Hit_checker_cutted_Chikuma \
	Hit_checker_cutted_Chikuma_Birks_el_0_0108 \
	Hit_checker_cutted_Chikuma_Birks_el_0_0308 \
	Hit_checker_cutted_Chikuma_Cher_coeff_0_033 \
	Hit_checker_cutted_Chikuma_Cher_coeff_0_233 \
	Hit_checker_cutted_Chikuma_main_Birks_0_0108 \
	Hit_checker_cutted_Chikuma_main_Birks_0_0308 \
	Hit_checker_cutted_Chikuma_paint_0_15 \
	Hit_checker_cutted_Chikuma_paint_0_45)
	
for ((i=0; $i<9; i++)) ; do
	FLIST=`ls ${BASE}/${CHIKUMA}/${LIST[$i]}`
	for f in $FLIST ; do
		./stopped_muons ${CHIKUMA}/${LIST[$i]}/${f}
	done
done

exit 0
