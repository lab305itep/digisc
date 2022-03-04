#!/bin/bash
#PBS -N run_digiMC
#PBS -q long
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.err
#PBS -l nodes=1
#PBS -l walltime=99:00:00
cd /home/itep/alekseev/igor

OVERWRITE=NO
MCRAW=MC_RAW
OUTDIR=/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/
DIGI=digi_MC
EXE=./evtbuilder5
DEAD="-deadlist dch_002210_102856.list"
##EXE=echo

do_sources()
{
	declare -a SRCLIST
	declare -a RAWLIST
	SRCLIST=(mc_12B_indLY_transcode_rawProc_pedSim.digi \
		 mc_22Na_indLY_transcode_rawProc_pedSim.digi \
		 mc_22Na_92_5_cmPos_indLY_transcode_rawProc_pedSim.digi \
		 mc_248Cm_indLY_transcode_rawProc_pedSim.digi \
		 mc_248Cm_92_5_cmPos_indLY_transcode_rawProc_pedSim.digi \
		 mc_60Co_indLY_transcode_rawProc_pedSim.digi \
		 mc_60Co_92_5_cmPos_indLY_transcode_rawProc_pedSim.digi\
		 mc_12B-DB_indLY_transcode_rawProc_pedSim.digi)
	RAWLIST=(12B_v2 22Na_v2 22Na_92_5_cm_pos_v2 248Cm_v2 248Cm_92_5_cm_pos_v2 60Co_v2 60Co_92_5_cm_pos_v2 12B-DB)
	
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/RadSources_v2
	if [ "x$1" == "x" ] ; then
		for ((i=0;$i<${#SRCLIST[@]};i=$i+1)) ; do
			${EXE} ${DIGIN}/${SRCLIST[$i]} 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/${RAWLIST[$i]}/DANSS0_1.root ${DEAD}
		done
	else
		${EXE} ${DIGIN}/${SRCLIST[$1]} 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/${RAWLIST[$1]}/DANSS0_1.root ${DEAD} -events 500000
	fi
}

do_neutrons()
{
	nbdir=${DIGI}/Neutron_background
	for ((i=0;$i<5;i=$i+1)) ; do
		for ((j=1;$j<=16;j=$j+1)) ; do
			fname=`printf "${nbdir}/mc_NeutronBgr_indLY_transcode_rawProc_pedSim%d_%d.digi.bz2" $i $j`
			mname=`printf "${MCRAW}/Neutron_background/Ready/DANSS%d_%d.root" $i $j`
			${EXE} $fname 0x870000 ${OUTDIR}/Neutron_background/ ${DEAD} -mcfile $mname
		done
	done
}

do_test()
{
	${EXE} /home/clusters/rrcmpi/danss/DANSS/digi_MC/newLY/DataTakingPeriod02/mc_Muons_indLY_transcode_rawProc_pedSim.digi \
		0x70000 ${OUTDIR}/test/ -mcfile ${MCRAW}/NewTimeLine/Muonmod.root ${DEAD}
}

do_stoppedmuons()
{
	for ((i=0;$i<2;i=$i+1)) ; do
		${EXE} ${DIGI}/Muons/mc_MuonsStopped_indLY_transcode_rawProc_pedSim_0${i}.digi.bz2 0x70000 \
			${OUTDIR}/Stopped_muons_central_part_ind_coeff/ \
			-mcfile ${MCRAW}/Stopped_muons_central_part_ind_coeff/DANSS_0${i}.root ${DEAD}
	done
}

do_muons()
{
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/Muons_v2
	for ((i=0;$i<2;i=$i+1)) ; do
		for ((j=1;$j<17;j=$j+1)) ; do
			ser=`printf "%2.2d_%2.2d" $i $j`
			${EXE} ${DIGIN}/mc_Muons_indLY_transcode_rawProc_pedSim_${ser}.digi.bz2 0x70000 \
				${OUTDIR}/Muons/ -mcfile ${MCRAW}/Muons_v2/OldTimeLine/DANSS${i}_${j}.root ${DEAD}
		done
	done
}

do_mudecay()
{
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/Muons_v2
#	${EXE} ${DIGIN}/mc_MuonsStopped_indLY_transcode_rawProc_pedSim_01.digi.bz2 0x70000 \
#		${OUTDIR}/MuonsStopped/ -mcfile ${MCRAW}/Stopped_muons_central_part_ind_coeff_v2/DANSS0_2.root ${DEAD}
	${EXE} ${DIGIN}/mc_MuonsMuMinus_indLY_transcode_rawProc_pedSim_01.digi.bz2 0x70000 \
		${OUTDIR}/MuonsStopped/ -mcfile ${MCRAW}/muminus/DANSS0_1.root ${DEAD}
}

# Usage: do_IBDdir templatein templateraw outdir nser ninser
do_IBDdir()
{
	tplin=$1
	tplraw=$2
	outdir=$3
	nser=$4
	ninser=$5
	for ((i=0;$i<nser;i=$i+1)) ; do
		for ((j=1;$j<=ninser;j=$j+1)) ; do
			namein=`printf "$tplin" $i $j`
			nameraw=`printf "$tplraw" $i $j`
			if [ $OVERWRITE == "NO" ] ; then
				nameout=$outdir/`basename $namein`
				nameout=${nameout/.digi/.root}
				if [ -f $nameout ] ; then
					continue
				fi
			fi
			if [ -f $namein ] ; then
				${EXE} $namein 0x870000 $outdir -mcfile $nameraw ${DEAD}
			fi
		done
	done
}

do_IBD()
{
	digi_ibd=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/IBD/
	raw_ibd=/home/clusters/rrcmpi/danss/MC_RAW/IBD/
	do_IBDdir "${digi_ibd}235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_%2.2d_%2.2d.digi" "${raw_ibd}235U/DANSS%d_%d.root" ${OUTDIR}/IBD/235U 3 4
	do_IBDdir "${digi_ibd}238U/mc_IBD_indLY_transcode_rawProc_pedSim_238U_%2.2d_%2.2d.digi" "${raw_ibd}238U/DANSS%d_%d.root" ${OUTDIR}/IBD/238U 1 4
	do_IBDdir "${digi_ibd}239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_%2.2d_%2.2d.digi" "${raw_ibd}239Pu/DANSS%d_%d.root" ${OUTDIR}/IBD/239Pu 3 4
	do_IBDdir "${digi_ibd}241Pu/mc_IBD_indLY_transcode_rawProc_pedSim_241Pu_%2.2d_%2.2d.digi" "${raw_ibd}241Pu/DANSS%d_%d.root" ${OUTDIR}/IBD/241Pu 1 4
	do_IBDdir "${digi_ibd}FS/mc_IBD_indLY_transcode_rawProc_pedSim_FS_%2.2d_%2.2d.digi" "${raw_ibd}Flat_spectrum/DANSS%d_%d.root" ${OUTDIR}/IBD/FS 10 16
}

do_test()
{
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/Muons_v2
	i=0
	j=1
	ser=`printf "%2.2d_%2.2d" $i $j`
	${EXE} ${DIGIN}/mc_Muons_indLY_transcode_rawProc_pedSim_${ser}.digi.bz2 0x70000 \
		${HOME}/test/ -mcfile ${MCRAW}/Muons_v2/OldTimeLine/DANSS${i}_${j}.root ${DEAD}
}

do_no_tr_profile()
{
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/no_transv_profile/DataTakingPeriod02
	MCRAW=/home/clusters/rrcmpi/danss/MC_RAW/No_transv_profile
	OUTDIR=/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/no_transv_profile
#		Muons
	for ((i=0;$i<2;i=$i+1)) ; do
		for ((j=1;$j<17;j=$j+1)) ; do
			ser=`printf "%2.2d_%2.2d" $i $j`
			FILEIN=${DIGIN}/Muons/mc_Muons_indLY_transcode_rawProc_pedSim_${ser}.digi
			if [ -f $FILEIN ] ; then
				${EXE} $FILEIN 0x70000 ${OUTDIR}/Muons -mcfile ${MCRAW}/Muons/DANSS${i}_${j}.root ${DEAD}
			fi
		done
	done
#		Sources
	${EXE} ${DIGIN}/RadSources/mc_22Na_no_transv_profile_indLY_transcode_rawProc_pedSim.digi  0x70000 ${OUTDIR}/RadSources \
		-mcfile ${MCRAW}/22Na/DANSS0_1.root ${DEAD}
	${EXE} ${DIGIN}/RadSources/mc_248Cm_no_transv_profile_indLY_transcode_rawProc_pedSim.digi 0x70000 ${OUTDIR}/RadSources \
		-mcfile ${MCRAW}/248Cm_neutron/DANSS0_1.root ${DEAD}
	${EXE} ${DIGIN}/RadSources/mc_60Co_no_transv_profile_indLY_transcode_rawProc_pedSim.digi  0x70000 ${OUTDIR}/RadSources \
		-mcfile ${MCRAW}/60Co/DANSS0_1.root ${DEAD}
}

do_IBD

exit 0
