#!/bin/bash
cd /home/itep/alekseev/igor
MY_DIR="/home/clusters/rrcmpi/alekseev/igor/apr16_apr21/"

for var in ALL 05_1 1_2 2_3 3_4 4_5 5_6 6_8 ; do
	NAME=${MY_DIR}bgnd_${var}.root
	MCNAME=${MY_DIR}mc_ibd_v8.1_${var}.root
	root -l -b -q "background_draw.C(\"${NAME}\", \"${MCNAME}\", 0.05)"
done

exit 0
