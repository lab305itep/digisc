#!/bin/bash
#PBS -N hittree_43
#PBS -q short
#PBS -l nodes=1
#PBS -l walltime=4:30:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_43.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/hittree_43.err
cd /home/itep/alekseev/igor
. /opt/fairsoft/bin/thisroot.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gcc-5.3/lib64
SRCDIR=/home/clusters/rrcmpi/alekseev/igor/root6n1
TGTDIR=/home/clusters/rrcmpi/alekseev/igor/root6n2
HITDIR=/home/clusters/rrcmpi/alekseev/igor/hitcheck
FROM=43000
TO=43999
for ((i=$FROM; $i<=$TO; i=$i+1 )) ; do
	let j=i/1000
	ROOTNAME=`printf "%3.3dxxx/danss_%6.6d.root" $j $i`
	HITNAME=`printf "%3.3dxxx/hits_%6.6d.txt.bz2" $j $i`
	if [ -f $SRCDIR/$ROOTNAME ] ; then
		cp --remove-destination $SRCDIR/$ROOTNAME $TGTDIR/$ROOTNAME 
		./hittree $TGTDIR/$ROOTNAME $HITDIR/$HITNAME
	fi
done
exit 0
