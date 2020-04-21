#!/bin/bash
#PBS -N run_pairMC
#PBS -q medium
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.err
#PBS -l nodes=1
#PBS -l walltime=23:30:00
cd /home/itep/alekseev/igor
RVER="6n11"
PVER="7n14"

LIST=`find /home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC/DataTakingPeriod01/Fuel* -name "*.root" -print`
for f in $LIST ; do
	OF=${f/root${RVER}/pair${PVER}}
	./pairbuilder8 $f $OF
done
exit 0

PDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/DataTakingPeriod01/Fuel

./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U.root     MC_raw/Fuel_IBD/235U_fuel/DANSS.root             $PDIR/mc_IBD_info_235U.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_00.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS0.root  $PDIR/mc_IBD_info_235U_00.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_01.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS1.root  $PDIR/mc_IBD_info_235U_01.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_02.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS2.root  $PDIR/mc_IBD_info_235U_02.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_03.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS3.root  $PDIR/mc_IBD_info_235U_03.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_04.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS4.root  $PDIR/mc_IBD_info_235U_04.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_05.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS5.root  $PDIR/mc_IBD_info_235U_05.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_06.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS6.root  $PDIR/mc_IBD_info_235U_06.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_07.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS7.root  $PDIR/mc_IBD_info_235U_07.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_08.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS8.root  $PDIR/mc_IBD_info_235U_08.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_09.root  MC_raw/Fuel_IBD_largeStat/235U_fuel/DANSS9.root  $PDIR/mc_IBD_info_235U_09.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_238U.root     MC_raw/Fuel_IBD/238U_fuel/DANSS.root             $PDIR/mc_IBD_info_238U.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_00.root  MC_raw/Fuel_IBD_largeStat/238U_fuel/DANSS0.root  $PDIR/mc_IBD_info_238U_00.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_01.root  MC_raw/Fuel_IBD_largeStat/238U_fuel/DANSS1.root  $PDIR/mc_IBD_info_238U_01.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_02.root  MC_raw/Fuel_IBD_largeStat/238U_fuel/DANSS2.root  $PDIR/mc_IBD_info_238U_02.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu.root    MC_raw/Fuel_IBD/239Pu_fuel/DANSS.root            $PDIR/mc_IBD_info_239Pu.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_00.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS0.root $PDIR/mc_IBD_info_239Pu_00.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_01.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS1.root $PDIR/mc_IBD_info_239Pu_01.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_02.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS2.root $PDIR/mc_IBD_info_239Pu_02.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_03.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS3.root $PDIR/mc_IBD_info_239Pu_03.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_04.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS4.root $PDIR/mc_IBD_info_239Pu_04.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_05.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS5.root $PDIR/mc_IBD_info_239Pu_05.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_06.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS6.root $PDIR/mc_IBD_info_239Pu_06.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_07.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS7.root $PDIR/mc_IBD_info_239Pu_07.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_08.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS8.root $PDIR/mc_IBD_info_239Pu_08.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_09.root MC_raw/Fuel_IBD_largeStat/239Pu_fuel/DANSS9.root $PDIR/mc_IBD_info_239Pu_09.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu.root    MC_raw/Fuel_IBD/241Pu_fuel/DANSS.root            $PDIR/mc_IBD_info_241Pu.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_00.root MC_raw/Fuel_IBD_largeStat/241Pu_fuel/DANSS0.root $PDIR/mc_IBD_info_241Pu_00.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_01.root MC_raw/Fuel_IBD_largeStat/241Pu_fuel/DANSS1.root $PDIR/mc_IBD_info_241Pu_01.root
./getMCinfo $PDIR/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_02.root MC_raw/Fuel_IBD_largeStat/241Pu_fuel/DANSS2.root $PDIR/mc_IBD_info_241Pu_02.root

exit 0
