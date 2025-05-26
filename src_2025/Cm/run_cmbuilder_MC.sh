#!/bin/bash
#PBS -N build_248MC
#PBS -q long
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/build_248MC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/build_248MC.err
#PBS -l nodes=1
#PBS -l walltime=150:00:00
cd /home/itep/alekseev/igor/src_2025/Cm
CMBUILDER=/home/itep/alekseev/igor/cmbuilder

for f in cm_MC_8.2_Center_Chikuma_Birks_el_0_0108 \
	cm_MC_8.2_Center_Chikuma_Birks_el_0_0308 \
	cm_MC_8.2_Center_Chikuma_Cher_coeff_0_033 \
	cm_MC_8.2_Center_Chikuma_Cher_coeff_0_233 \
	cm_MC_8.2_Center_Chikuma \
	cm_MC_8.2_Center_Chikuma_main_Birks_0_0108 \
	cm_MC_8.2_Center_Chikuma_main_Birks_0_0308 \
	cm_MC_8.2_Center_Chikuma_paint_0_15 \
	cm_MC_8.2_Center_Chikuma_paint_0_45 \
	cm_MC_8.2_Center_Chikuma_xzmap \
	cm_MC_8.6_Center_Chikuma_Birks_el_0_0108 \
	cm_MC_8.6_Center_Chikuma_Birks_el_0_0308 \
	cm_MC_8.6_Center_Chikuma_Cher_coeff_0_033 \
	cm_MC_8.6_Center_Chikuma_Cher_coeff_0_233 \
	cm_MC_8.6_Center_Chikuma \
	cm_MC_8.6_Center_Chikuma_main_Birks_0_0108 \
	cm_MC_8.6_Center_Chikuma_main_Birks_0_0308 \
	cm_MC_8.6_Center_Chikuma_paint_0_15 \
	cm_MC_8.6_Center_Chikuma_paint_0_45 \
	cm_MC_8.6_Center_Chikuma_xzmap ; do
	${CMBUILDER} ${f}.list ${f}.root
done

exit 0
