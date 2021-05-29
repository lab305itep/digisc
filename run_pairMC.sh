#!/bin/bash
#PBS -N run_pairMC
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.err
#PBS -l nodes=1
#PBS -l walltime=12:00:00
cd /home/itep/alekseev/igor
RVER="8n1"
PVER="8n1"

do_fuel()
{
	LIST=`find /home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC/DataTakingPeriod01/FuelGd -name "*.root" -print`
	for f in $LIST ; do
		OF=${f/root${RVER}/pair${PVER}}
		./pairbuilder8 $f $OF
	done
	PDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/DataTakingPeriod01/FuelGd

	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U.root     MC_rawGd/Fuel/235U_fuel/DANSS.root                   $PDIR/mc_IBD_info_235U.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_00.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS0.root  $PDIR/mc_IBD_info_235U_00.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_01.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS1.root  $PDIR/mc_IBD_info_235U_01.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_02.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS2.root  $PDIR/mc_IBD_info_235U_02.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_03.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS3.root  $PDIR/mc_IBD_info_235U_03.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_04.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS4.root  $PDIR/mc_IBD_info_235U_04.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_05.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS5.root  $PDIR/mc_IBD_info_235U_05.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_06.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS6.root  $PDIR/mc_IBD_info_235U_06.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_07.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS7.root  $PDIR/mc_IBD_info_235U_07.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_08.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS8.root  $PDIR/mc_IBD_info_235U_08.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_09.root  MC_rawGd/Fuel_new_Gd_big_stat/235U_fuel/DANSS9.root  $PDIR/mc_IBD_info_235U_09.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_238U.root     MC_rawGd/Fuel/238U_fuel/DANSS.root                   $PDIR/mc_IBD_info_238U.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_00.root  MC_rawGd/Fuel_new_Gd_big_stat/238U_fuel/DANSS0.root  $PDIR/mc_IBD_info_238U_00.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_01.root  MC_rawGd/Fuel_new_Gd_big_stat/238U_fuel/DANSS1.root  $PDIR/mc_IBD_info_238U_01.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_02.root  MC_rawGd/Fuel_new_Gd_big_stat/238U_fuel/DANSS2.root  $PDIR/mc_IBD_info_238U_02.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu.root    MC_rawGd/Fuel/239Pu_fuel/DANSS.root                  $PDIR/mc_IBD_info_239Pu.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_00.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS0.root $PDIR/mc_IBD_info_239Pu_00.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_01.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS1.root $PDIR/mc_IBD_info_239Pu_01.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_02.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS2.root $PDIR/mc_IBD_info_239Pu_02.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_03.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS3.root $PDIR/mc_IBD_info_239Pu_03.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_04.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS4.root $PDIR/mc_IBD_info_239Pu_04.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_05.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS5.root $PDIR/mc_IBD_info_239Pu_05.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_06.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS6.root $PDIR/mc_IBD_info_239Pu_06.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_07.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS7.root $PDIR/mc_IBD_info_239Pu_07.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_08.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS8.root $PDIR/mc_IBD_info_239Pu_08.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_09.root MC_rawGd/Fuel_new_Gd_big_stat/239Pu_fuel/DANSS9.root $PDIR/mc_IBD_info_239Pu_09.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu.root    MC_rawGd/Fuel/241Pu_fuel/DANSS.root                  $PDIR/mc_IBD_info_241Pu.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_00.root MC_rawGd/Fuel_new_Gd_big_stat/241Pu_fuel/DANSS0.root $PDIR/mc_IBD_info_241Pu_00.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_01.root MC_rawGd/Fuel_new_Gd_big_stat/241Pu_fuel/DANSS1.root $PDIR/mc_IBD_info_241Pu_01.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_02.root MC_rawGd/Fuel_new_Gd_big_stat/241Pu_fuel/DANSS2.root $PDIR/mc_IBD_info_241Pu_02.root
}

do_flat()
{
	LIST=`find /home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC/DataTakingPeriod01/FlatGd/2 -name "*.root" -print`
	for f in $LIST ; do
		OF=${f/root${RVER}/pair${PVER}}
		./pairbuilder8 $f $OF
	done

	PDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/DataTakingPeriod01/FlatGd/2

#	./getMCinfo $PDIR/mc_FlatSpectrum_glbLY_transcode_rawProc_pedSim.root MC_rawGd/Flat_spectrum/DANSS.root $PDIR/mc_Flat_info.root
	for i in 0 1 2 ; do
		for j in A B C D E ; do
			./getMCinfo ${PDIR}/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.root \
				MC_rawGd/Flat_spectrum_new_Gd_big_stat/Ready_2/DANSS${i}_${j}.root ${PDIR}/mc_Flat_info_${i}_${j}.root
		done
	done
}

do_fuelg4()
{
	LIST=`find /home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC/DataTakingPeriod01/FuelNewG4v4 -name "*.root" -print`
	for f in $LIST ; do
		OF=${f/root${RVER}/pair${PVER}}
		./pairbuilder8 $f $OF
	done
	PDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/DataTakingPeriod01/FuelNewG4v4
	RAWDIR=/home/clusters/rrcmpi/danss/DANSS/MC_raw/newG4_test4/Fuel/
	mkdir -p $PDIR
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U.root  $RAWDIR/235U_fuel/DANSS.root  $PDIR/mc_IBD_info_235U.root
	./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu.root  $RAWDIR/239Pu_fuel/DANSS.root  $PDIR/mc_IBD_info_239Pu.root
	for (( i=0; $i<4; i=$i+1 )) ; do
		./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_0${i}.root  $RAWDIR/235U_fuel/DANSS${i}.root  $PDIR/mc_IBD_info_235U_0${i}.root
		./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_0${i}.root  $RAWDIR/239Pu_fuel/DANSS${i}.root  $PDIR/mc_IBD_info_239Pu_0${i}.root
	done
}

do_neutrons()
{
	RDIR=/home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC/Neutron_background
	PDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/Neutron_background
	MDIR=/home/clusters/rrcmpi/danss/MC_RAW/Neutron_background/Ready/
	for ((i=0;$i<5;i=$i+1)) ; do
		for ((j=1;$j<=16;j=$j+1)) ; do
			rname=`printf "${RDIR}/mc_NeutronBgr_indLY_transcode_rawProc_pedSim%d_%d.root" $i $j`
			pname=`printf "${PDIR}/mc_NeutronBgr_indLY_transcode_rawProc_pedSim_pair%d_%d.root" $i $j`
			iname=`printf "${PDIR}/mc_NeutronBgr_indLY_transcode_rawProc_pedSim_info%d_%d.root" $i $j`
			mname=`printf "${MDIR}/DANSS%d_%d.root" $i $j`
			./pairbuilder8 $rname $pname
			./getMCinfo $pname $mname $iname
		done
	done
}

date
mkdir -p /home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/Neutron_background
do_neutrons
date

exit 0
