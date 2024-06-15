#!/bin/bash
#PBS -N run_pairMC
#PBS -q long
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_pairMC.err
#PBS -l nodes=1
#PBS -l walltime=100:00:00
cd /home/itep/alekseev/igor
RVER="8n2"
PVER="8n2"

ROOTDIR=/home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC
PAIRDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC
RAWDIR=/home/clusters/rrcmpi/danss/MC_RAW

# Usage: do_IBDdir name nser ninser
do_IBDdir()
{
	name=$1
	nser=$2
	ninser=$3
	fuel=$4
	mkdir -p $PAIRDIR/$name
	for ((i=0;$i<nser;i=$i+1)) ; do
		for ((j=1;$j<=ninser;j=$j+1)) ; do
			namein=$ROOTDIR/$name/`printf "mc_IBD_indLY_transcode_rawProc_pedSim_%s_%2.2d_%2.2d.root" $fuel $i $j`
			nameraw=$RAWDIR/$name/Ready/`printf "DANSS%d_%d.root" $i $j`
			nameout=$PAIRDIR/$name/`basename $namein`
			nameinfo=${nameout/.root/_info.root}
			if [ -f $namein ] ; then
				./pairbuilder8 $namein $nameout
				./getMCinfo $nameout $nameraw $nameinfo
			fi
		done
	done
}

do_IBD()
{
	do_IBDdir "235U"   3  4 "235U" 
	do_IBDdir "238U"   1  4 "238U" 
	do_IBDdir "239Pu"  3  4 "239Pu"
	do_IBDdir "241Pu"  1  4 "241Pu"
	do_IBDdir "FS"    10 16 "FS"   
}

do_neutrons()
{
	RDIR=/home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC/Akagi/Neutron_background
	PDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/Akagi/Neutron_background
	MDIR=/home/clusters/rrcmpi/danss/MC_RAW/Akagi/Neutron_background/
	mkdir -p /home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/Akagi/Neutron_background
	for ((i=1;$i<17;i=$i+1)) ; do
		for ((j=0;$j<=10;j=$j+1)) ; do 
			rname=`printf "${RDIR}/mc_NeutronBgr_indLY_transcode_rawProc_pedSim0_%dp%d.root" $i $j`
			pname=`printf "${PDIR}/mc_NeutronBgr_indLY_transcode_rawProc_pedSim_pair0_%dp%d.root" $i $j`
			iname=`printf "${PDIR}/mc_NeutronBgr_indLY_transcode_rawProc_pedSim_info0_%dp%d.root" $i $j`
			mname=`printf "${MDIR}/DANSS0_%dp%d.root" $i $j`
			./pairbuilder8 $rname $pname
			./getMCinfo $pname $mname $iname
		done
	done
}

do_gamma()
{
	RDIR=/home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC/Gamma_v9
	PDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/Gamma_v9
	MDIR=/home/clusters/rrcmpi/danss/MC_RAW/v9/Gamma/MeasuredSpectrum
	mkdir -p ${PDIR}
	for ((j=1; $j<=16; j=$j+1)) ; do
		rname=`printf "${RDIR}/mc_GammaMeasuredSpectrum_indLY_transcode_rawProc_pedSim_DBspectrum%d.root" $j`
		pname=`printf "${PDIR}/mc_GammaMeasuredSpectrum_indLY_transcode_rawProc_pedSim_DBspectrum_pair%d.root" $j`
		iname=`printf "${PDIR}/mc_GammaMeasuredSpectrum_indLY_transcode_rawProc_pedSim_DBspectrum_info%d.root" $j`
		mname=`printf "${MDIR}/DANSS0_%d.root" $j`
		./pairbuilder8 $rname $pname
		./getMCinfo $pname $mname $iname
	done
}

date
do_IBDdir "Chikuma/IBD/235U_fuel_Chikuma_Cher_coeff_0_233" 6 16 "235U"
do_IBDdir "Chikuma/IBD/235U_fuel_Chikuma_FIFRELIN2" 6 16 "235U"
date

exit 0
