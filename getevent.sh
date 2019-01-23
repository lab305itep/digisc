#!/bin/bash

if (($# < 2)) ; then
	echo "Usage $0 run_number|full_path event_global_time"
	exit 10
fi;

. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
export DANSSRAWREC_HOME=/home/clusters/rrcmpi/alekseev/igor/digi_home


RUN=$1

shift

if [ ${RUN:0:1} == "/" ] ; then
	FULLPATH=$RUN
	TMPFILE=list_file.tmp
	ARGS="-mcdata -flag 0x70000"
	
else
	DANSS_DIGI_DIR=/home/clusters/rrcmpi/alekseev/igor/digi/v2.1/
	RUNFILE=`printf "%3.3dxxx/danss_data_%6.6d_phys_rawrec.digi" $(($RUN / 1000)) $RUN`
	FULLPATH=${DANSS_DIGI_DIR}${RUNFILE}
	TMPFILE=${RUN}.tmp
	ARGS="-tcalib tcalib_5512_ss-d.calib -flag 0x50000"
fi

echo "${FULLPATH}" > ${TMPFILE}

for GT in $@ ; do
	./digi_evtbuilder6  ${ARGS} -no_hit_tables -file ${TMPFILE} -dump ${GT}
	if (($? == 0)) ; then
		root -l -q -b "drawevent.C(${GT})"
	fi
done

rm -f ${RUN}.tmp

exit 0
