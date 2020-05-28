#!/bin/bash
#DBG="echo"
ROOTVER=${ROOTVER:-6n11}
PAIRVER=${PAIRVER:-7n14}

if [ "x$3" == "x" ] ; then
	echo "Usage: $0 smear_stohastic smear_constant scale"
	exit 10
fi
DIRNAME=`printf "R%5.3fC%5.3fS%5.3f" $1 $2 $3`
OPT="-smear $1 $2 -mcscale $3"
RAW=/home/itep/alekseev/igor/MC_rawGd/Flat_spectrum_new_Gd_big_stat
DIGI=/home/clusters/rrcmpi/alekseev/igor/MCFlat
ROOTDIR=/home/clusters/rrcmpi/alekseev/igor/root${ROOTVER}/MC/DataTakingPeriod01/FlatGd/${DIRNAME}
PAIRDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PAIRVER}/MC/DataTakingPeriod01/FlatGd/${DIRNAME}
DEAD="-deadlist deadlist_12850.txt"
LIST=${PAIRDIR}/run_list.txt
HIST=${PAIRDIR}/${DIRNAME}
DLIST=${ROOTDIR}/.tmp.list
export DANSSRAWREC_HOME=lib_v3.2

cd /home/itep/alekseev/igor
mkdir -p $ROOTDIR
mkdir -p $PAIRDIR
rm -f $LIST

for k in 1 2 3 ; do
	for i in 0 1 2 3 4 ; do
		for j in A B C D E ; do
			FNAME=${DIGI}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.digi
			MCNAME=${RAW}/Ready_${k}/DANSS${i}_${j}.root
			RNAME=${ROOTDIR}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.root
			PNAME=${PAIRDIR}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.root
			INAME=${PAIRDIR}/Ready_${k}/mc_NeutrinoFlat_info_${i}_${j}.root
			if [ -f $FNAME ] && [ -f $MCNAME ] ; then
				echo "$FNAME" > $DLIST
				$DBG ./digi_evtbuilder6_v3 -file $DLIST -output $RNAME -flag 0x870000 -mcdata -mcfile $MCNAME $DEAD $OPT
				$DBG ./pairbuilder8 $RNAME $PNAME 
				$DBG ./getMCinfo $PNAME $MCNAME $INAME
				echo $PNAME $INAME >> ${LIST}
			fi
		done
	done
done

$DBG MonoNu/mc_mono_nu $LIST ${HIST}.root
$DBG root -l -b -q "MonoNu/mc_mono_neutrinos.C(\"${HIST}\")"

exit 0
