#!/bin/sh

if [ x$1 == x ] ; then
	echo Usage $0 dirname
else 
	NAME=`basename $1`.root
	root -l -b -q "add_periods.C(\"${NAME}\", \"$1\")"
	root -l -b -q "background_draw.C(\"${NAME}\")"
fi
