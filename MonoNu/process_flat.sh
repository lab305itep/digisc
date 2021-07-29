#!/bin/bash
# Process one MC file
# 160 files named 
# ${FLAT_DIGI}/mc_IBD_indLY_transcode_rawProc_pedSim_FS_NN_MM.digi
# etc. NN=0..10, MM=1..16

if [ "x$1" == "x" ] ; then
	echo "Nothing to do"
	exit 0
fi

if [ $1 == 25 ] ; then
	echo "Bad MC_RAW" 
	exit 0
fi
if [ $1 == 37 ] ; then
	echo "Bad MC_RAW" 
	exit 0
fi
if [ $1 == 70 ] ; then
	echo "Bad digi" 
	exit 0
fi
if [ $1 == 118 ] ; then
	echo "Bad digi" 
	exit 0
fi

k=$(($1/16))
i=$(($1%16))

cd ${HOME}/igor
FNAME=`printf "${FLAT_DIGI}/mc_IBD_indLY_transcode_rawProc_pedSim_FS_%2.2d_%2.2d.digi" $k $i`
MCNAME=${FLAT_RAW}/DANSS${k}_${i}.root
RNAME=`printf "${ROOTDIR}/mc_IBD_indLY_transcode_rawProc_pedSim_FS_%2.2d_%2.2d.root" $k $i`
PNAME=`printf "${PAIRDIR}/mc_IBD_indLY_transcode_rawProc_pedSim_FS_%2.2d_%2.2d.root" $k $i`
INAME=`printf "${PAIRDIR}/mc_IBD_indLY_transcode_rawProc_pedSim_FS_%2.2d_%2.2d_info.root" $k $i`
DLIST=${RNAME/.root/.list}
if [ -f $FNAME ] && [ -f $MCNAME ] ; then
	echo "$FNAME" > $DLIST
	./digi_evtbuilder6_v3 -file $DLIST -output $RNAME -flag 0x870000 -mcdata -mcfile $MCNAME $FLAT_DEAD $FLAT_OPT
	./pairbuilder8 $RNAME $PNAME 
	./getMCinfo $PNAME $MCNAME $INAME
	rm -f $DLIST
else
	echo "No file $FNAME and/or $MCNAME"
fi

exit 0
