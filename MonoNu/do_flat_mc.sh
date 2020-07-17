#!/bin/bash
#DBG="echo"
ROOTVER=${ROOTVER:-6n12}
PAIRVER=${PAIRVER:-7n15}
REBUILD=${REBUILD:-NO}

if [ "x$3" == "x" ] ; then
	echo "Usage: $0 smear_stohastic smear_constant scale [dead option:MIN NORM MAX]"
	exit 10
fi

DIRNAME=`printf "R%5.3fC%5.3fS%5.3f" $1 $2 $3`

case "x"$4 in 
	xMIN ) 	
		DEAD="-deadlist dch_002210_078234_min.list" 
		DIRNAME=${DIRNAME}_DMIN
		;;
	xMAX ) 
		DEAD="-deadlist dch_002210_078234_max.list"
		DIRNAME=${DIRNAME}_DMAX
		;;
	* )
		DEAD="-deadlist dch_002210_078234.list" 
		;;
esac

OPT="-smear $1 $2 -mcscale $3"
RAW=/home/itep/alekseev/igor/MC_rawGd/Flat_spectrum_new_Gd_big_stat
DIGI=/home/clusters/rrcmpi/alekseev/igor/MCFlat
ROOTDIR=/home/clusters/rrcmpi/alekseev/igor/root${ROOTVER}/MC/DataTakingPeriod01/FlatGd/${DIRNAME}
PAIRDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PAIRVER}/MC/DataTakingPeriod01/FlatGd/${DIRNAME}
DEAD=${DEAD_FILE:-"-deadlist dch_002210_078234.list"}
LIST=${PAIRDIR}/run_list.txt
HIST=${PAIRDIR}/${DIRNAME}
DLIST=${ROOTDIR}/.tmp.list
export DANSSRAWREC_HOME=lib_v3.2

cd /home/itep/alekseev/igor
mkdir -p $ROOTDIR
mkdir -p $PAIRDIR
rm -f $LIST

for k in 1 2 3 4 5 6 7 8 ; do
	for i in 0 1 2 3 4 ; do
		for j in A B C D E ; do
			FNAME=${DIGI}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.digi
			MCNAME=${RAW}/Ready_${k}/DANSS${i}_${j}.root
			RNAME=${ROOTDIR}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.root
			PNAME=${PAIRDIR}/Ready_${k}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.root
			INAME=${PAIRDIR}/Ready_${k}/mc_NeutrinoFlat_info_${i}_${j}.root
			if [ -f $FNAME ] && [ -f $MCNAME ] ; then
				if [ $REBUILD == "YES" ] || ! [ -f $PNAME ] ; then
					echo "$FNAME" > $DLIST
					$DBG ./digi_evtbuilder6_v3 -file $DLIST -output $RNAME -flag 0x870000 -mcdata -mcfile $MCNAME $DEAD $OPT
					$DBG ./pairbuilder8 $RNAME $PNAME 
					$DBG ./getMCinfo $PNAME $MCNAME $INAME
				fi
				echo $PNAME $INAME >> ${LIST}
			fi
		done
	done
done

$DBG MonoNu/mc_mono_nu $LIST ${HIST}.root
$DBG root -l -b -q "MonoNu/mc_mono_neutrinos.C(\"${HIST}\")"

$DBG MonoNu/mc_mono_nu $LIST ${HIST}p0.05.root "1" "0.05"
$DBG root -l -b -q "MonoNu/mc_mono_neutrinos.C(\"${HIST}p0.05\")"

$DBG MonoNu/mc_mono_nu $LIST ${HIST}n0.05.root "1" "(-0.05)"
$DBG root -l -b -q "MonoNu/mc_mono_neutrinos.C(\"${HIST}n0.05\")"

exit 0
