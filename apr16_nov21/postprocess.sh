#!/bin/bash
cd /home/itep/alekseev/igor

if [ "x$2" == "x" ] ; then
	echo "Usage $0 name bg_scale"
	exit 10
fi

OUT_DIR=/home/clusters/rrcmpi/alekseev/igor/apr16_nov21/

NAMEIN=${OUT_DIR}/$1
NAMEOUT=${NAMEIN/.root/${2}.root}
cp $NAMEIN $NAMEOUT
root -l -b -q "danss_draw_report.C(\"${NAMEOUT}\", $2)"
root -l -b -q "apr16_nov21/danss_calc_ratio_v6.C(\"${NAMEOUT}\", $2)"
exit 0
