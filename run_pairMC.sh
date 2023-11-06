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
	mkdir -p $PAIRDIR/IBD/$name
	for ((i=0;$i<nser;i=$i+1)) ; do
		for ((j=1;$j<=ninser;j=$j+1)) ; do
			namein=$ROOTDIR/IBD/$name/`printf "mc_IBD_indLY_transcode_rawProc_pedSim_%s_%2.2d_%2.2d.root" $name $i $j`
			nameraw=$RAWDIR/IBD/$name/`printf "DANSS%d_%d.root" $i $j`
			nameout=$PAIRDIR/IBD/$name/`basename $namein`
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
	do_IBDdir "235U"   3  4
	do_IBDdir "238U"   1  4
	do_IBDdir "239Pu"  3  4
	do_IBDdir "241Pu"  1  4
	do_IBDdir "FS"    10 16
}

do_neutrons()
{
	RDIR=/home/clusters/rrcmpi/alekseev/igor/root${RVER}/MC/Neutron_background
	PDIR=/home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/Neutron_background
	MDIR=/home/clusters/rrcmpi/danss/MC_RAW/Neutron_background/Ready/
	mkdir -p /home/clusters/rrcmpi/alekseev/igor/pair${PVER}/MC/Neutron_background
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
do_gamma
date

exit 0
