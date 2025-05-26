#!/bin/bash
#PBS -N run_sources6
#PBS -q long
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_sources6.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_sources6.err
#PBS -l nodes=1
#PBS -l walltime=199:00:00
cd /home/itep/alekseev/igor/src_2025/NaCo

echo MC
date

echo Na
for f in Full_decay_center_Chikuma \
	Full_decay_center_Chikuma_Birks_el_0_0108 \
	Full_decay_center_Chikuma_Birks_el_0_0308 \
	Full_decay_center_Chikuma_Cher_coeff_0_033 \
	Full_decay_center_Chikuma_Cher_coeff_0_233 \
	Full_decay_center_Chikuma_main_Birks_0_0108 \
	Full_decay_center_Chikuma_main_Birks_0_0308 \
	Full_decay_center_Chikuma_paint_0_15 \
	Full_decay_center_Chikuma_paint_0_45 \
	Full_decay_center_Chikuma_xzmap ; do
	for ((j=0; $j<41; j=$j+1)) ; do
		scale=`echo "0.9 + 0.005*$j" | bc`
		root -l -b -q "draw_Sources6.C(1031, \"$f\", $scale, 30)"
		root -l -b -q "draw_Sources6.C(1032, \"$f\", $scale, 30)"
	done
done

echo Co
for f in Center_Chikuma \
	Center_Chikuma_Birks_el_0_0108 \
	Center_Chikuma_Birks_el_0_0308 \
	Center_Chikuma_Cher_coeff_0_033 \
	Center_Chikuma_Cher_coeff_0_233 \
	Center_Chikuma_main_Birks_0_0108 \
	Center_Chikuma_main_Birks_0_0308 \
	Center_Chikuma_paint_0_15 \
	Center_Chikuma_paint_0_45 \
	Center_Chikuma_xzmap ; do
	for ((j=0; $j<41; j=$j+1)) ; do
		scale=`echo "0.9 + 0.005*$j" | bc`
		root -l -b -q "draw_Sources6.C(1131, \"$f\", $scale, 30)"
		root -l -b -q "draw_Sources6.C(1132, \"$f\", $scale, 30)"
	done
done
date

exit 0
