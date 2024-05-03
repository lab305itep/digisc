#!/bin/bash
#PBS -N MonoF
#PBS -q medium
#PBS -l nodes=1
#PBS -l walltime=8:00:00
#PBS -l pmem=3Gb
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/MonoF.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/MonoF.err
cd /home/itep/alekseev/igor/MonoNu

#for DIRNAME in R0.120C0.040S1.020 R0.120C0.040S1.000 R0.120C0.040S0.980 R0.180C0.060S1.000 R0.060C0.020S1.000 R0.120C0.040S1.000_DMAX  R0.120C0.040S1.000_DMIN ; do
for DIRNAME in R0.000C0.000S1.000 ; do
	PAIRDIR=/home/clusters/rrcmpi/alekseev/igor/pair8n2/MC/IBD/160_files/${DIRNAME}
	LIST=${PAIRDIR}/run_list.txt
	HIST=${PAIRDIR}/${DIRNAME}_high
#	./mc_mono_nu_high $LIST ${HIST}.root
#	./mc_mono_nu_high $LIST ${HIST}_opt.root "NeutronEnergy > 2.8 && Distance < 30"
	./mc_mono_nu_high $LIST ${HIST}_p0.05.root "1" "0.05"
	root -l -b -q "mc_mono_neutrinos.C(\"${HIST}p0.05\")"
	./mc_mono_nu_high $LIST ${HIST}_n0.05.root "1" "(-0.05)"
	root -l -b -q "mc_mono_neutrinos.C(\"${HIST}n0.05\")"
	./mc_mono_nu_high $LIST ${HIST}_opt_p0.05.root "NeutronEnergy > 2.8 && Distance < 30" "0.05"
	root -l -b -q "mc_mono_neutrinos.C(\"${HIST}p0.05\")"
	./mc_mono_nu_high $LIST ${HIST}_opt_n0.05.root "NeutronEnergy > 2.8 && Distance < 30" "(-0.05)"
	root -l -b -q "mc_mono_neutrinos.C(\"${HIST}n0.05\")"
	for delta in 0.025 0.075 0.10 0.125; do
		./mc_mono_nu_high $LIST ${HIST}_p${delta}.root "1" "${delta}"
		root -l -b -q "mc_mono_neutrinos.C(\"${HIST}p${delta}\")"
		./mc_mono_nu_high $LIST ${HIST}_opt_p${delta}.root "NeutronEnergy > 2.8 && Distance < 30" "${delta}"
		root -l -b -q "mc_mono_neutrinos.C(\"${HIST}p${delta}\")"
	done 
done

exit 0
