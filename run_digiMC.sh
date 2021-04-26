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

${EXE} /home/clusters/rrcmpi/danss/DANSS/digi_MC/newLY/DataTakingPeriod02/mc_IBD_glbLY_transcode_rawProc_pedSim.digi \
	0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/NewTimeLine/IBD_small.root ${DEAD}

exit 0
