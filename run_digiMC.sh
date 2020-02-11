#!/bin/bash
#PBS -N run_digiMC
#PBS -q long
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.err
#PBS -l nodes=1
#PBS -l walltime=100:00:00
cd /home/itep/alekseev/igor

MCRAW=MC_raw
OUTDIR=/home/clusters/rrcmpi/alekseev/igor/root6n8/MC/DataTakingPeriod01
DIGI=digi_MC/DataTakingPeriod01

# We need just a list...
#./evtbuilder5  input.digi flags output_dir -mcfile mc.root

for ((E=625; $E<120000; E=$E+1250)) ; do
	let EM=$E/10000
	let EF=$E-10000*$EM
	FN=`printf "mc_MonoPositrons_glbLY_transcode_rawProc_pedSim_%2.2d-%4.4d.digi.bz2" $EM $EF`
	DN=`printf "%d.%4.4dMeV" $EM $EF`
	./evtbuilder5 ${DIGI}/MonoPositronsLargeStat/${FN} 0x71000 ${OUTDIR}/MonoPositronsLargeStat -mcfile ${MCRAW}/Positrons/${DN}/DANSS.root
done  

./evtbuilder5 ${DIGI}/12B/mc_12B_glbLY_transcode_rawProc_pedSim.digi.bz2 0x70000 ${OUTDIR}/12B -mcfile ${MCRAW}/12B/DANSS.root
for f in 235U 238U 239Pu 241Pu ; do 
	./evtbuilder5 ${DIGI}/Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_${f}.digi.bz2 0x70000 ${OUTDIR}/Fuel -mcfile ${MCRAW}/Fuel_IBD/${f}_fuel/DANSS.root
done


./evtbuilder5 ${DIGI}/Muons/mc_Muons_glbLY_transcode_rawProc_pedSim.digi.bz2   0x70000 ${OUTDIR}/Muons -mcfile ${MCRAW}/Muons/DANSS.root
./evtbuilder5 ${DIGI}/Muons/mc_Muons_glbLY_transcode_rawProc_pedSim_0.digi.bz2 0x70000 ${OUTDIR}/Muons -mcfile ${MCRAW}/Muons/DANSS0.root
./evtbuilder5 ${DIGI}/Muons/mc_Muons_glbLY_transcode_rawProc_pedSim_1.digi.bz2 0x70000 ${OUTDIR}/Muons -mcfile ${MCRAW}/Muons/DANSS1.root
./evtbuilder5 ${DIGI}/Muons/mc_Muons_glbLY_transcode_rawProc_pedSim_2.digi.bz2 0x70000 ${OUTDIR}/Muons -mcfile ${MCRAW}/Muons/DANSS2.root
./evtbuilder5 ${DIGI}/Muons/mc_Muons_glbLY_transcode_rawProc_pedSim_3.digi.bz2 0x70000 ${OUTDIR}/Muons -mcfile ${MCRAW}/Muons/DANSS3.root

./evtbuilder5 ${DIGI}/Muons/mc_MuonsStoppedCenter_glbLY_transcode_rawProc_pedSim.digi.bz2   0x70000 ${OUTDIR}/Muons -mcfile ${MCRAW}/Stopped_muons_central_part/DANSS.root
./evtbuilder5 ${DIGI}/Muons/mc_MuonsStoppedCenter_glbLY_transcode_rawProc_pedSim_0.digi.bz2 0x70000 ${OUTDIR}/Muons -mcfile ${MCRAW}/Stopped_muons_central_part/DANSS0.root

./evtbuilder5 ${DIGI}/12B/mc_12B_glbLY_transcode_rawProc_pedSim.digi.bz2                    0x70000 ${OUTDIR}/12B        -mcfile ${MCRAW}/12B/DANSS.root
./evtbuilder5 ${DIGI}/RadSources/mc_22Na_glbLY_transcode_rawProc_pedSim.digi.bz2            0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/22Na/DANSS.root
./evtbuilder5 ${DIGI}/RadSources/mc_22Na_90cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2    0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/22Na_90_cm_pos/DANSS.root
./evtbuilder5 ${DIGI}/RadSources/mc_60Co_glbLY_transcode_rawProc_pedSim.digi.bz2            0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/60Co/DANSS.root
./evtbuilder5 ${DIGI}/RadSources/mc_60Co_90cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2    0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/60Co_90_cm_pos/DANSS.root
./evtbuilder5 ${DIGI}/RadSources/mc_248Cm_glbLY_transcode_rawProc_pedSim.digi.bz2           0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm/DANSS.root
./evtbuilder5 ${DIGI}/RadSources/mc_248Cm_90cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2   0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_90_cm_pos/DANSS.root
./evtbuilder5 ${DIGI}/RadSources/mc_248Cm_92cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2   0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_92_cm_pos/DANSS.root
./evtbuilder5 ${DIGI}/RadSources/mc_248Cm_92_5cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_92_5_cm_pos/DANSS.root

exit 0
