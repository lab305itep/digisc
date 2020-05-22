#!/bin/bash
#PBS -N check_pair7n
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/check_pair7n.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/check_pair7n.err
#PBS -l nodes=1
#PBS -q short

cd /home/itep/alekseev/igor
echo IBD
./rootcheck "/home/clusters/rrcmpi/alekseev/igor/root6n11/%3.3dxxx/danss_%6.6d.root" "/home/clusters/rrcmpi/alekseev/igor/pair7n14/%3.3dxxx/pair_%6.6d.root" 2210 78234
#echo Muon
#./rootcheck "/home/clusters/rrcmpi/alekseev/igor/root6n8/%3.3dxxx/danss_%6.6d.root" "/home/clusters/rrcmpi/alekseev/igor/muon7n8/%3.3dxxx/muon_%6.6d.root" 71211 74720
exit 0
