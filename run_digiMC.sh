#!/bin/bash
#PBS -N run_digiMC
#PBS -q long
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.err
#PBS -l nodes=1
#PBS -l walltime=199:00:00
cd /home/itep/alekseev/igor

OVERWRITE=NO
MCRAW=/home/clusters/rrcmpi/danss/MC_RAW/v9
OUTDIR=/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/
DIGI=digi_MC
EXE=./evtbuilder5
DEAD="-deadlist main_dead.list"
##EXE=echo

do_sources()
{
	declare -a SRCLIST
	declare -a RAWLIST
#	SRCLIST=(12B/mc_12B_indLY_transcode_rawProc_pedSim_DBspectrum1.digi \
#		12B/mc_12B_indLY_transcode_rawProc_pedSim_DBspectrum2.digi \
#		12B/mc_12B_indLY_transcode_rawProc_pedSim_G4spectrum1.digi \
#		12B/mc_12B_indLY_transcode_rawProc_pedSim_G4spectrum2.digi \
#		22Na/mc_22Na_indLY_transcode_rawProc_pedSim_92_5_cm1.digi \
#		22Na/mc_22Na_indLY_transcode_rawProc_pedSim_Center1.digi \
#		22Na/mc_22Na_indLY_transcode_rawProc_pedSim_Full_decay_92_5_cm1.digi \
#		22Na/mc_22Na_indLY_transcode_rawProc_pedSim_Full_decay_Center1.digi \
#		248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm1.digi \
#		248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm2.digi \
#		248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_Center1.digi \
#		248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_Center2.digi \
#		60Co/mc_60Co_indLY_transcode_rawProc_pedSim_92_5_cm1.digi \
#		60Co/mc_60Co_indLY_transcode_rawProc_pedSim_Center1.digi)
#	SRCLIST=(mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm1.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm2.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm3.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm4.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm5.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm6.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm7.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm8.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_Center1.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_Center2.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_Center3.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_Center4.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_Center5.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_Center6.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_Center7.digi \
#		mc_248Cm_indLY_transcode_rawProc_pedSim_Center8.digi)
#	SRCLIST=(/12B/mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum1.digi \
#		/12B/mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum2.digi \
#		/12B/mc_12B-DB_indLY_transcode_rawProc_pedSim_G4spectrum1.digi \
#		/12B/mc_12B-DB_indLY_transcode_rawProc_pedSim_G4spectrum2.digi \
#		/22Na/mc_22Na_indLY_transcode_rawProc_pedSim_92_5_cm1.digi \
#		/22Na/mc_22Na_indLY_transcode_rawProc_pedSim_Center1.digi \
#		/22Na/mc_22Na_indLY_transcode_rawProc_pedSim_Full_decay_92_5_cm1.digi \
#		/22Na/mc_22Na_indLY_transcode_rawProc_pedSim_Full_decay_Center1.digi \
#		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm1.digi \
#		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm2.digi \
#		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm3.digi \
#		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm4.digi \
#		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm5.digi \
#		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm6.digi \
#		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm7.digi \
#		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm8.digi \
#		/60Co/mc_60Co_indLY_transcode_rawProc_pedSim_92_5_cm1.digi \
#		/60Co/mc_60Co_indLY_transcode_rawProc_pedSim_Center1.digi)
	SRCLIST=(/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_Center1.digi \
		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_Center2.digi \
		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_Center3.digi \
		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_Center4.digi \
		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_Center5.digi \
		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_Center6.digi \
		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_Center7.digi \
		/248Cm/mc_248Cm_indLY_transcode_rawProc_pedSim_Center8.digi \
		/248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm2.digi \
		/248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_Center2.digi)
#	RAWLIST=(12B/DB_spectrum_new_model/DANSS0_1.root \
#		12B/DB_spectrum_new_model/DANSS0_2.root \
#		12B/G4_spectrum_new_model/DANSS0_1.root \
#		12B/G4_spectrum_new_model/DANSS0_2.root \
#		22Na/92_5_cm_pos_new_model/DANSS0_1.root \
#		22Na/Center_new_model/DANSS0_1.root \
#		22Na/Full_decay_92_5_cm_pos_new_model/DANSS0_1.root \
#		22Na/Full_decay_center_new_model/DANSS0_1.root \
#		248Cm_neutron/92_5_cm_pos_new_model/DANSS0_1.root \
#		248Cm_neutron/92_5_cm_pos_new_model/DANSS0_2.root \
#		248Cm_neutron/Center_new_model/DANSS0_1.root \
#		248Cm_neutron/Center_new_model/DANSS0_2.root \
#		60Co/92_5_cm_pos_new_model/DANSS0_1.root
#		60Co/Center_new_model/DANSS0_1.root)
#	RAWLIST=(248Cm/92_5_cm_pos_new_model/Ready/DANSS0_1.root \
#		248Cm/92_5_cm_pos_new_model/Ready/DANSS0_2.root \
#		248Cm/92_5_cm_pos_new_model/Ready/DANSS0_3.root \
#		248Cm/92_5_cm_pos_new_model/Ready/DANSS0_4.root \
#		248Cm/92_5_cm_pos_new_model/Ready/DANSS0_5.root \
#		248Cm/92_5_cm_pos_new_model/Ready/DANSS0_6.root \
#		248Cm/92_5_cm_pos_new_model/Ready/DANSS0_7.root \
#		248Cm/92_5_cm_pos_new_model/Ready/DANSS0_8.root \
#		248Cm/Center_new_model/Ready/DANSS0_1.root \
#		248Cm/Center_new_model/Ready/DANSS0_2.root \
#		248Cm/Center_new_model/Ready/DANSS0_3.root \
#		248Cm/Center_new_model/Ready/DANSS0_4.root \
#		248Cm/Center_new_model/Ready/DANSS0_5.root \
#		248Cm/Center_new_model/Ready/DANSS0_6.root \
#		248Cm/Center_new_model/Ready/DANSS0_7.root \
#		248Cm/Center_new_model/Ready/DANSS0_8.root)
#	RAWLIST=(/12B/DB_spectrum_new_Cherenkov/DANSS0_1.root \
#		/12B/DB_spectrum_new_Cherenkov/DANSS0_2.root \
#		/12B/G4_spectrum_new_Cherenkov/DANSS0_1.root \
#		/12B/G4_spectrum_new_Cherenkov/DANSS0_2.root \
#		/22Na/92_5_cm_pos_new_Cherenkov/DANSS0_1.root \
#		/22Na/Center_new_Cherenkov/DANSS0_1.root \
#		/22Na/Full_decay_92_5_cm_pos_new_Cherenkov/DANSS0_1.root \
#		/22Na/Full_decay_center_new_Cherenkov/DANSS0_1.root \
#		/248Cm/92_5_cm_pos_new_Cherenkov/DANSS0_1.root
#		/248Cm/92_5_cm_pos_new_Cherenkov/DANSS0_2.root \
#		/248Cm/92_5_cm_pos_new_Cherenkov/DANSS0_3.root \
#		/248Cm/92_5_cm_pos_new_Cherenkov/DANSS0_4.root \
#		/248Cm/92_5_cm_pos_new_Cherenkov/DANSS0_5.root \
#		/248Cm/92_5_cm_pos_new_Cherenkov/DANSS0_6.root \
#		/248Cm/92_5_cm_pos_new_Cherenkov/DANSS0_7.root \
#		/248Cm/92_5_cm_pos_new_Cherenkov/DANSS0_8.root \
#		/60Co/92_5_cm_pos_new_Cherenkov/DANSS0_1.root \
#		/60Co/Center_new_Cherenkov/DANSS0_1.root)
	RAWHIST=(/248Cm/Center_new_Cherenkov/DANSS0_1.root
		/248Cm/Center_new_Cherenkov/DANSS0_2.root \
		/248Cm/Center_new_Cherenkov/DANSS0_3.root \
		/248Cm/Center_new_Cherenkov/DANSS0_4.root \
		/248Cm/Center_new_Cherenkov/DANSS0_5.root \
		/248Cm/Center_new_Cherenkov/DANSS0_6.root \
		/248Cm/Center_new_Cherenkov/DANSS0_7.root \
		/248Cm/Center_new_Cherenkov/DANSS0_8.root \
		/248Cm_neutron/92_5_cm_pos_new_Cherenkov/DANSS0_2.root \
		/248Cm_neutron/Center_new_Cherenkov/DANSS0_2.root)
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v9/RadSources/
#	for ((i=0;$i<${#SRCLIST[@]};i=$i+1)) ; do
	for ((i=0;$i<8;i=$i+1)) ; do
		${EXE} ${DIGIN}/${SRCLIST[$i]} 0x70000 ${OUTDIR}/RadSources_v9 -mcfile ${MCRAW}/${RAWLIST[$i]} ${DEAD}
	done
	for ((i=8;$i<10;i=$i+1)) ; do
		${EXE} ${DIGIN}/${SRCLIST[$i]} 0x70000 ${OUTDIR}/RadSources_v9/248Cm_neutron -mcfile ${MCRAW}/${RAWLIST[$i]} ${DEAD}
	done
}

do_sources_full()
{
	declare -a SRCLIST
	declare -a RAWLIST
	SRCLIST=(mc_22Na_full_decay_central_pos_indLY_transcode_rawProc_pedSim.digi \
		mc_26Al_full_decay_central_pos_indLY_transcode_rawProc_pedSim.digi)
	RAWLIST=(22Na_full_decay_central_pos 26Al_full_decay_central_pos)
	
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/RadSources_v2_1
	if [ "x$1" == "x" ] ; then
		for ((i=0;$i<${#SRCLIST[@]};i=$i+1)) ; do
			${EXE} ${DIGIN}/${SRCLIST[$i]} 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/${RAWLIST[$i]}/DANSS0_1.root ${DEAD}
		done
	else
		${EXE} ${DIGIN}/${SRCLIST[$1]} 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/${RAWLIST[$1]}/DANSS0_1.root ${DEAD} -events 500000
	fi
}

do_248Cm_FR2()
{
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/FR-2/DataTakingPeriod02/RadSources
	${EXE} ${DIGIN}/mc_248Cm_indLY_transcode_rawProc_pedSim.digi 0x70000 ${OUTDIR}/FR2\
		-mcfile ${MCRAW}/248Cm-FR-2/DANSS0_1.root ${DEAD}
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
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/Stopped_muons_central_part_ind_coeff_v2
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/Muons_v2
	for ((i=0;$i<2;i=$i+1)) ; do
		${EXE} ${DIGIN}/mc_MuonsStopped_indLY_transcode_rawProc_pedSim_0${i}.digi.bz2 0x70000 \
			${OUTDIR}/Muons/ \
			-mcfile ${_mc_raw}/DANSS0_$(($i+1)).root ${DEAD}
	done
}

do_cutmuons()
{
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/v9/Hit_checker_new_Cherenkov_cutted
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v9/Hit_checker_new_Cherenkov_cutted
	for ((k=0;$k<5;k=$k+1)) ; do
		for ((i=1;$i<=16;i=$i+1)) ; do
			j=`printf "%2.2d" $i`
			${EXE} ${DIGIN}/mc_Muons_indLY_transcode_rawProc_pedSim_0${k}_${j}.digi 0x70000 \
				${OUTDIR}/Hit_checker_new_model_cutted_v9 -mcfile ${_mc_raw}/DANSS${k}_${i}.root ${DEAD}
		done
	done
}

do_cutmuons_127()
{
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/Hit_checker_new_model_cutted_Birks_0.0127
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v6/Hit_checker_new_model_cutted_Birks_0.0127
	for ((k=0;$k<5;k=$k+1)) ; do
		for ((i=1;$i<=16;i=$i+1)) ; do
			j=`printf "%2.2d" $i`
			${EXE} ${DIGIN}/mc_Muons_indLY_transcode_rawProc_pedSim_0${k}_${j}.digi 0x70000 \
				${OUTDIR}/Hit_checker_new_model_cutted_Birks_0.0127 -mcfile ${_mc_raw}/DANSS${k}_${i}.root ${DEAD}
		done
	done
}

do_cutmuons_289()
{
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/Hit_checker_new_model_cutted_Birks_0.0289
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v6/Hit_checker_new_model_cutted_Birks_0.0289
	for ((k=0;$k<5;k=$k+1)) ; do
		for ((i=1;$i<=16;i=$i+1)) ; do
			j=`printf "%2.2d" $i`
			${EXE} ${DIGIN}/mc_Muons_indLY_transcode_rawProc_pedSim_0${k}_${j}.digi 0x70000 \
				${OUTDIR}/Hit_checker_new_model_cutted_Birks_0.0289 -mcfile ${_mc_raw}/DANSS${k}_${i}.root ${DEAD}
		done
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
#	${EXE} ${DIGIN}/mc_MuonsMuMinus_indLY_transcode_rawProc_pedSim_01.digi.bz2 0x70000 \
#		${OUTDIR}/MuonsStopped/ -mcfile ${MCRAW}/muminus/DANSS0_1.root ${DEAD}
#	${EXE} ${DIGIN}/mc_MuonsMuPlus_indLY_transcode_rawProc_pedSim_01.digi.bz2 0x70000 \
#		${OUTDIR}/MuonsStopped/ -mcfile ${MCRAW}/muplus/DANSS0_1.root ${DEAD}
	${EXE} ${DIGIN}/mc_MuonsMuPlus_indLY_transcode_rawProc_pedSim_02.digi.bz2 0x70000 \
		${OUTDIR}/MuonsStopped/ -mcfile ${MCRAW}/muplus/DANSS0_2.root ${DEAD}
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

do_Edik_n()
{
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/newNewLY/DataTakingPeriod02/Edik_profile
	for ((i=0; $i<10; i=$i+1)) ; do 
		for ((j=1; $j<=16; j=$j+1)) ; do
			NAMEIN=`printf "mc_NeutronBgr_indLY_transcode_rawProc_pedSim0_%dp%d.digi" $j $i`
			MCIN=`printf "DANSS0_%dp%d.root" $j $i`
			${EXE} ${DIGIN}/${NAMEIN} 0x70000 ${OUTDIR}/Edik_profile -mcfile ${MCRAW}/Edik_profile/Ready/${MCIN} ${DEAD}
		done
	done
}

do_DB_new()
{
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v6/DB_spectrum_new_model
	mkdir -p ${OUTDIR}/DB_spectrum_new_model
	${EXE} ${DIGIN}/mc_12B-DB_indLY_transcode_rawProc_pedSim.digi 0x70000 \
		${OUTDIR}/DB_spectrum_new_model -mcfile ${_mc_raw}/DANSS0_1.root ${DEAD}
}

do_Posi_new()
{
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v6/Positrons_from_muplus_decay_new_model
	mkdir -p ${OUTDIR}/Positrons_from_muplus_decay_new_model
	${EXE} ${DIGIN}/mc_MuonsMuPlus_indLY_transcode_rawProc_pedSim_01.digi 0x70000 \
		${OUTDIR}/Positrons_from_muplus_decay_new_model -mcfile ${_mc_raw}/DANSS0_1.root ${DEAD}
	${EXE} ${DIGIN}/mc_MuonsMuPlus_indLY_transcode_rawProc_pedSim_02.digi 0x70000 \
		${OUTDIR}/Positrons_from_muplus_decay_new_model -mcfile ${_mc_raw}/DANSS0_2.root ${DEAD}
}

do_StMu_new()
{
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v6/Stopped_muons_new_model
	mkdir -p ${OUTDIR}/Stopped_muons_new_model
	${EXE} ${DIGIN}/mc_MuonsStopped_indLY_transcode_rawProc_pedSim_01.digi 0x70000 \
		${OUTDIR}/Stopped_muons_new_model -mcfile ${_mc_raw}/DANSS0_1.root ${DEAD}
	${EXE} ${DIGIN}/mc_MuonsStopped_indLY_transcode_rawProc_pedSim_02.digi 0x70000 \
		${OUTDIR}/Stopped_muons_new_model -mcfile ${_mc_raw}/DANSS0_2.root ${DEAD}
}

do_StMuCenter_new()
{
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v6/Stopped_muons_central_part_new_model
	mkdir -p ${OUTDIR}/Stopped_muons_central_part_new_model
	${EXE} ${DIGIN}/mc_MuonsStopped_central_part_indLY_transcode_rawProc_pedSim_01.digi 0x70000 \
		${OUTDIR}/Stopped_muons_central_part_new_model -mcfile ${_mc_raw}/DANSS0_1.root ${DEAD}
	${EXE} ${DIGIN}/mc_MuonsStopped_central_part_indLY_transcode_rawProc_pedSim_02.digi 0x70000 \
		${OUTDIR}/Stopped_muons_central_part_new_model -mcfile ${_mc_raw}/DANSS0_2.root ${DEAD}
}

do_mu_e_C_decay()
{
	_mc_raw=/home/clusters/rrcmpi/danss/MC_RAW/New/Muons/Electrons_from_muminus_decay_in_C_orbit_new_model
	DIGIN=/home/clusters/rrcmpi/danss/DANSS/digi_MC/new_model_newProfiles_v6/Muons/Electrons_from_muminus_decay_in_C_orbit_new_model
	mkdir -p ${OUTDIR}/Electrons_from_muminus_decay_in_C_orbit_new_model
	${EXE} ${DIGIN}/mc_MuonsMuMinus_indLY_transcode_rawProc_pedSim_01.digi 0x70000 \
		${OUTDIR}/Electrons_from_muminus_decay_in_C_orbit_new_model -mcfile ${_mc_raw}/DANSS0_1.root ${DEAD}
}

date
do_cutmuons
date
exit 0
