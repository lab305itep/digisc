#!/bin/bash
MCDIR=/mnt/MC/DANSSmod/Muons
for d in Hit_checker_cutted_Chikuma Hit_checker_cutted_Chikuma_Birks_el_0_0108 Hit_checker_cutted_Chikuma_Birks_el_0_0308 \
	Hit_checker_cutted_Chikuma_Cher_coeff_0_033 Hit_checker_cutted_Chikuma_Cher_coeff_0_233 Hit_checker_cutted_Chikuma_main_Birks_0_0108 \
	Hit_checker_cutted_Chikuma_main_Birks_0_0308 Hit_checker_cutted_Chikuma_paint_0_15 Hit_checker_cutted_Chikuma_paint_0_45 ; do
	for ((i=0; $i<5; i++)) ; do
		for ((j=1; $j<17; j++)) ; do
			prcname=`printf "mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d.root" $i $j`
			mcname=`printf "DANSSmod%d_%d.root" $i $j`
			./add_depth ${d}/${prcname} ${MCDIR}/${d}/Ready/${mcname}
			echo ${d}/${prcname} - done.
		done
	done
done
