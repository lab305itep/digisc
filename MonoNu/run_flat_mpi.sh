#!/bin/bash
#PBS -N MonoNu6
#PBS -q mpi
#PBS -l nodes=160
#PBS -l walltime=5:00:00
#PBS -l pmem=3Gb
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/MonoNu6.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/MonoNu6.err
cd /home/itep/alekseev/igor/MonoNu

export ROOTVER=8n1
export PAIRVER=8n1
#export MONO_ARG="0.12 0.04 1.00"
#export MONO_ARG="0.12 0.04 0.98"
#export MONO_ARG="0.12 0.04 1.02"
#export MONO_ARG="0.06 0.02 1.00"
#export MONO_ARG="0.18 0.06 1.00"
#export MONO_ARG="0.12 0.04 1.00 MIN"
#export MONO_ARG="0.12 0.04 1.00 MAX"

ARG1=0.12
ARG2=0.04
ARG3=1.00
ARG4=MAX

DIRNAME=`printf "R%5.3fC%5.3fS%5.3f" $ARG1 $ARG2 $ARG3`

case $ARG4 in 
	"MIN" ) 	
		FLAT_DEAD="-deadlist dch_002210_078234_min.list" 
		DIRNAME=${DIRNAME}_DMIN
		;;
	"MAX" ) 
		FLAT_DEAD="-deadlist dch_002210_078234_max.list"
		DIRNAME=${DIRNAME}_DMAX
		;;
	* )
		FLAT_DEAD="-deadlist dch_002210_078234.list" 
		;;
esac
export FLAT_DEAD
export FLAT_OPT="-smear $ARG1 $ARG2 -mcscale $ARG3"

export FLAT_RAW=/home/clusters/rrcmpi/danss/MC_RAW/IBD/FS
export FLAT_DIGI=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/IBD/FS
export ROOTDIR=/home/clusters/rrcmpi/alekseev/igor/root${ROOTVER}/MC/IBD/${DIRNAME}
export PAIRDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PAIRVER}/MC/IBD/${DIRNAME}
LIST=${PAIRDIR}/run_list.txt
HIST=${PAIRDIR}/${DIRNAME}
DLIST=${ROOTDIR}/.tmp.list
export DANSSRAWREC_HOME=lib_v3.2

cd /home/itep/alekseev/igor/MonoNu
mkdir -p $ROOTDIR
mkdir -p $PAIRDIR
rm -f $LIST

mpirun --mca btl ^tcp run_flat_mpi

for ((k=0;$k<10;k=$k+1)) ; do
	for ((i=1;$i<=16;i=$i+1)) ; do
		PNAME=`printf "${PAIRDIR}/mc_IBD_indLY_transcode_rawProc_pedSim_FS_%2.2d_%2.2d.root" $k $i`
		INAME=`printf "${PAIRDIR}/mc_IBD_indLY_transcode_rawProc_pedSim_FS_%2.2d_%2.2d_info.root" $k $i`
		if [ -f $PNAME ] && [ -f $INAME ] ; then
			echo $PNAME $INAME >> ${LIST}
		fi
	done
done

$DBG ./mc_mono_nu $LIST ${HIST}.root
$DBG root -l -b -q "mc_mono_neutrinos.C(\"${HIST}\")"

exit 0

$DBG ./mc_mono_nu $LIST ${HIST}p0.05.root "1" "0.05"
$DBG root -l -b -q "mc_mono_neutrinos.C(\"${HIST}p0.05\")"

$DBG ./mc_mono_nu $LIST ${HIST}n0.05.root "1" "(-0.05)"
$DBG root -l -b -q "mc_mono_neutrinos.C(\"${HIST}n0.05\")"

exit 0
