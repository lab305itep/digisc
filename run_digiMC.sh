#!/bin/bash
#PBS -N run_digiMC
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.err
#PBS -l nodes=1
#PBS -l walltime=12:00:00
cd /home/itep/alekseev/igor

MCRAW=MC_RAW
OUTDIR=/home/clusters/rrcmpi/alekseev/igor/root8n1/MC/
DIGI=digi_MC
EXE=./evtbuilder5
DEAD="-deadlist dch_002210_089039.list"
##EXE=echo

do_sources()
{
	declare -a SRCLIST
	SRCLIST=(mc_12B_indLY_transcode_rawProc_pedSim.digi.bz2 mc_248Cm_indLY_transcode_rawProc_pedSim.digi.bz2\
		 mc_22Na_90cmPos_indLY_transcode_rawProc_pedSim.digi.bz2 mc_60Co_90cmPos_indLY_transcode_rawProc_pedSim.digi.bz2\
		 mc_22Na_indLY_transcode_rawProc_pedSim.digi.bz2 mc_60Co_indLY_transcode_rawProc_pedSim.digi.bz2\
		 mc_248Cm_90cmPos_indLY_transcode_rawProc_pedSim.digi.bz2)
	declare -a RAWLIST
	RAWLIST=(12B 248Cm 22Na_90_cm_pos 22Na 60Co_90_cm_pos 60Co 248Cm_90_cm_pos)

	for ((i=0;$i<${#SRCLIST[@]};i=$i+1)) ; do
		${EXE} ${DIGI}/RadSources/${SRCLIST[$i]} 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/${RAWLIST[$i]}/DANSS.root ${DEAD}
	done
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

do_muons()
{
	for ((i=0;$i<2;i=$i+1)) ; do
		${EXE} ${DIGI}/Muons/mc_MuonsStopped_indLY_transcode_rawProc_pedSim_0${i}.digi.bz2 0x70000 \
			${OUTDIR}/Stopped_muons_central_part_ind_coeff/ \
			-mcfile ${MCRAW}/Stopped_muons_central_part_ind_coeff/DANSS_0${i}.root ${DEAD}
	done
}

do_muons

exit 0
