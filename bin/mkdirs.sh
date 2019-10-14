#!/bin/bash
# Make run directories

if [ "x"$2 == "x" ] ; then
	echo Usage $0 from to
else
	for (( i=$1; $i<=$2 ; i=$i+1 )) ; do
		mkdir `printf "%3.3dxxx" $i`
	done
fi
