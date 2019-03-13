#!/bin/sh

if [ x$1 == x ] ; then
	echo Usage $0 dirname
else 
	NAME=`basename $1`.root
	root -l -b -q "join_periods.C(\"${NAME}\", \"$1\")"
	root -l -b -q "danss_draw_report.C(\"${NAME}\",2.64)"
	root -l -b -q "danss_calc_ratio_v5.C(\"${NAME}\", 2.64)"
fi
