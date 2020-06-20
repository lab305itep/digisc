#!/bin/bash
#PBS -N run_digiMC
#PBS -q short
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_digiMC.err
#PBS -l nodes=1
#PBS -l walltime=4:00:00
cd /home/itep/alekseev/igor

MCRAW=MC_raw
MCRAWGD=MC_rawGd
OUTDIR=/home/clusters/rrcmpi/alekseev/igor/root6n11/MC/DataTakingPeriod01
DIGI=digi_MC/DataTakingPeriod01
DIGIGD=digi_MCGd/DataTakingPeriod01
EXE=./evtbuilder5
DEAD="-deadlist deadlist_12850.txt"
#EXE=echo

# We need just a list...
#${EXE}  input.digi flags output_dir -mcfile mc.root
do_monopositrons() 
{
#	Monopositrons
	for ((E=625; $E<120000; E=$E+1250)) ; do
		let EM=$E/10000
		let EF=$E-10000*$EM
		FN=`printf "mc_MonoPositrons_glbLY_transcode_rawProc_pedSim_%2.2d-%4.4d.digi.bz2" $EM $EF`
		DN=`printf "%d.%4.4dMeV" $EM $EF`
		${EXE} ${DIGI}/MonoPositronsLargeStat/${FN} 0x71000 ${OUTDIR}/MonoPositronsLargeStat -mcfile ${MCRAW}/Positrons/${DN}/DANSS.root ${DEAD}
	done  
}

#	Fuel
do_fuel()
{
	for f in 235U 238U 239Pu 241Pu ; do 
		${EXE} ${DIGI}/Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_${f}.digi.bz2 0x870000 ${OUTDIR}/Fuel -mcfile ${MCRAW}/Fuel_IBD/${f}_fuel/DANSS.root ${DEAD}
	done
	for f in 235U 238U 239Pu 241Pu ; do 
		for ((n=0; $n<10; n=$n+1)) ; do
			fn=`printf "mc_IBD_glbLY_transcode_rawProc_pedSim_%s_%2.2d.digi.bz2" $f $n`
			fm=`printf "%s_fuel/DANSS%d.root" $f $n`
			if [ -f ${DIGI}/Fuel_largeStat/${fn} ] ; then 
				${EXE} ${DIGI}/Fuel_largeStat/${fn} 0x870000 ${OUTDIR}/Fuel -mcfile ${MCRAW}/Fuel_IBD_largeStat/${fm} ${DEAD}
			fi
		done
	done
}

do_fuelGd()
{
	for f in 235U 238U 239Pu 241Pu ; do 
		${EXE} ${DIGIGD}/Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_${f}.digi.bz2 0x870000 ${OUTDIR}/FuelGd -mcfile ${MCRAWGD}/Fuel/${f}_fuel/DANSS.root ${DEAD}
	done
	for f in 235U 238U 239Pu 241Pu ; do 
		for ((n=0; $n<10; n=$n+1)) ; do
			fn=`printf "mc_IBD_glbLY_transcode_rawProc_pedSim_%s_%2.2d.digi.bz2" $f $n`
			fm=`printf "%s_fuel/DANSS%d.root" $f $n`
			if [ -f ${DIGIGD}/Fuel_largeStat/${fn} ] ; then 
				${EXE} ${DIGIGD}/Fuel_largeStat/${fn} 0x870000 ${OUTDIR}/FuelGd -mcfile ${MCRAWGD}/Fuel_new_Gd_big_stat/${fm} ${DEAD}
			fi
		done
	done
}

do_flatGd()
{
#	${EXE} ${DIGIGD}/Flat_spectrum/mc_FlatSpectrum_glbLY_transcode_rawProc_pedSim.digi.bz2 0x870000 ${OUTDIR}/FlatGd -mcfile ${MCRAWGD}/Flat_spectrum/DANSS.root ${DEAD}
	for i in 0 1 2 ; do
		for j in A B C D E ; do
			${EXE} ${DIGIGD}/Flat_spectrum_new_Gd_big_stat/Ready_2/mc_NeutrinoFlat_glbLY_transcode_rawProc_pedSim_${i}_${j}.digi.bz2 0x870000 \
				${OUTDIR}/FlatGd/2 -mcfile ${MCRAWGD}/Flat_spectrum_new_Gd_big_stat/Ready_2/DANSS${i}_${j}.root ${DEAD}
		done
	done
}

do_muons()
{
	${EXE} ${DIGI}/Muons/mc_Muons_glbLY_transcode_rawProc_pedSim.digi.bz2 0x70000 ${OUTDIR}/MuonsNoDead -mcfile ${MCRAW}/Muons/DANSS.root
	for f in 0 1 2 3 ; do
		${EXE} ${DIGI}/Muons/mc_Muons_glbLY_transcode_rawProc_pedSim_${f}.digi.bz2 0x70000 ${OUTDIR}/MuonsNoDead -mcfile ${MCRAW}/Muons/DANSS${f}.root
	done
	${EXE} ${DIGI}/Muons/mc_MuonsStoppedCenter_glbLY_transcode_rawProc_pedSim.digi.bz2   0x70000 ${OUTDIR}/MuonsNoDead -mcfile ${MCRAW}/Stopped_muons_central_part/DANSS.root
	${EXE} ${DIGI}/Muons/mc_MuonsStoppedCenter_glbLY_transcode_rawProc_pedSim_0.digi.bz2 0x70000 ${OUTDIR}/MuonsNoDead -mcfile ${MCRAW}/Stopped_muons_central_part/DANSS0.root
	${EXE} ${DIGI}/Muons/mc_Muons_glbLY_transcode_rawProc_pedSim_cutted_energy.digi.bz2  0x70000 ${OUTDIR}/MuonsNoDead -mcfile ${MCRAW}/Muons_cutted_energy/DANSS.root
}

do_radmuons()
{
	for f in 0 1 2 3 ; do
		${EXE} ${DIGIGD}/Muons/Positrons_from_muplus_decay_new_paint/mc_Muons_glbLY_transcode_rawProc_pedSim_positronsFromMuplusDecayNewPaint_${f}.digi.bz2 0x70000 \
		${OUTDIR}/MuonsNoDead -mcfile ${MCRAWGD}/Muons/Positrons_from_muplus_decay_new_paint/DANSS${f}.root
	done
}

do_sources()
{
	${EXE} ${DIGI}/12B/mc_12B_glbLY_transcode_rawProc_pedSim.digi.bz2                    0x70000 ${OUTDIR}/12B        -mcfile ${MCRAW}/12B/DANSS.root               ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_22Na_glbLY_transcode_rawProc_pedSim.digi.bz2            0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/22Na/DANSS.root              ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_22Na_90cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2    0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/22Na_90_cm_pos/DANSS.root    ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_60Co_glbLY_transcode_rawProc_pedSim.digi.bz2            0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/60Co/DANSS.root              ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_60Co_90cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2    0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/60Co_90_cm_pos/DANSS.root    ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_glbLY_transcode_rawProc_pedSim.digi.bz2           0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm/DANSS.root             ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_glbLY_transcode_rawProc_pedSim_0.digi.bz2         0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm/DANSS0.root            ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_glbLY_transcode_rawProc_pedSim_1.digi.bz2         0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm/DANSS1.root            ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_glbLY_transcode_rawProc_pedSim_2.digi.bz2         0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm/DANSS2.root            ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_50cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2   0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_50_cm_pos/DANSS.root   ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_50cmPos_glbLY_transcode_rawProc_pedSim_0.digi.bz2 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_50_cm_pos/DANSS0.root  ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_50cmPos_glbLY_transcode_rawProc_pedSim_1.digi.bz2 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_50_cm_pos/DANSS1.root  ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_50cmPos_glbLY_transcode_rawProc_pedSim_2.digi.bz2 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_50_cm_pos/DANSS2.root  ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_90cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2   0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_90_cm_pos/DANSS.root   ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_92cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2   0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_92_cm_pos/DANSS.root   ${DEAD}
	${EXE} ${DIGI}/RadSources/mc_248Cm_92_5cmPos_glbLY_transcode_rawProc_pedSim.digi.bz2 0x70000 ${OUTDIR}/RadSources -mcfile ${MCRAW}/248Cm_92_5_cm_pos/DANSS.root ${DEAD}
}

# do_sources
# do_monopositrons
# do_fuelGd
#do_flatGd
#do_fuel
do_radmuons

exit 0
