#!/bin/bash

if [ "x$3" == "x" ] ; then
	exit 0
fi

k=$1
i=$2
j=$3

cd ${HOME}/igor
FNAME=${FLAT_DIGI}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.digi
MCNAME=${FLAT_RAW}/Ready_${k}/DANSS${i}_${j}.root
RNAME=${ROOTDIR}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.root
PNAME=${PAIRDIR}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.root
INAME=${PAIRDIR}/Ready_${k}/mc_NeutrinoFlat_info_${i}_${j}.root
DLIST=${RNAME/.root/.list}
if [ -f $FNAME ] && [ -f $MCNAME ] ; then
	echo "$FNAME" > $DLIST
	./digi_evtbuilder6_v3 -file $DLIST -output $RNAME -flag 0x870000 -mcdata -mcfile $MCNAME $FLAT_DEAD $FLAT_OPT
	./pairbuilder8 $RNAME $PNAME 
	./getMCinfo $PNAME $MCNAME $INAME
	rm -f $DLIST
fi

exit 0
