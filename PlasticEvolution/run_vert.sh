#!/bin/bash
#PBS -N run_vert
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/run_vert.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/run_vert.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=180:00:00

let thefirst=2210
let thelast=121985
SRC=/home/clusters/rrcmpi/alekseev/igor/root8n2
TGT=/home/clusters/rrcmpi/alekseev/igor/vert8n2

cd /home/itep/alekseev/igor/PlasticEvolution
date
for ((n=$thefirst;$n<=$thelast;n=$n+1)) ; do
	let nn=$n/1000
	filein=`printf "%s/%3.3dxxx/danss_%6.6d.root" $SRC $nn $n`
	fileout=`printf "%s/%3.3dxxx/danss_%6.6d.root" $TGT $nn $n`
	dirout=`printf "%s/%3.3dxxx" $TGT $nn`
	mkdir -p $dirout
	./vert_muon $filein $fileout
done
date

exit 0
