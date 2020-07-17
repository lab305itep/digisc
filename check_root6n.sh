#!/bin/bash
#PBS -N check_root6n
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/check_root6n.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/check_root6n.err
#PBS -l nodes=1
#PBS -q medium

cd /home/itep/alekseev/igor
./rootcheck digi "/home/clusters/rrcmpi/alekseev/igor/root6n12/%3.3dxxx/danss_%6.6d.root" 2210 78234

exit 0
