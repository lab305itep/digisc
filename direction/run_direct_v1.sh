#!/bin/bash
#PBS -N direction
#PBS -q xxl
#PBS -l walltime=480:00:00
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/direct.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/direct.err
cd /home/itep/alekseev/igor/direction

decalre -a fbegin 
decalre -a fend
fbegin=(7037 24731 56291 89360 95141 119274 147917)
fend=(23073 54629 87473 94957 117692 146668 154797)
decalre -a zbegin 
decalre -a zend
zbegin=(23074 54630 87474 94960 117693 146669)
zend=(24705 56290 89039 95140 119273 147916)

for ((n=0; $n<7; n++)) ; do
	./direction ${fbegin[n]} ${fend[n]} full
	./direction ${fbegin[n]} ${fend[n]} full_center "PositronX[0]>30 && PositronX[0]<66 && PositronX[1]>30 && PositronX[1]<66 && PositronX[2]>31.5 && PositronX[2]<67.5"
	./direction ${fbegin[n]} ${fend[n]} full_center_15mks "gtDiff < 15 && PositronX[0]>30 && PositronX[0]<66 && PositronX[1]>30 && PositronX[1]<66 && PositronX[2]>31.5 && PositronX[2]<67.5"
	./direction ${fbegin[n]} ${fend[n]} full_center_4MeV "PositronEnergy > 4 && PositronX[0]>30 && PositronX[0]<66 && PositronX[1]>30 && PositronX[1]<66 && PositronX[2]>31.5 && PositronX[2]<67.5"
	./direction ${fbegin[n]} ${fend[n]} full_center_n6MeV "NeutronEnergy > 6 && PositronX[0]>30 && PositronX[0]<66 && PositronX[1]>30 && PositronX[1]<66 && PositronX[2]>31.5 && PositronX[2]<67.5"
done

for ((n=0; $n<6; n++)) ; do
	./direction ${zbegin[n]} ${zend[n]} zero
	./direction ${zbegin[n]} ${zend[n]} zero_center "PositronX[0]>30 && PositronX[0]<66 && PositronX[1]>30 && PositronX[1]<66 && PositronX[2]>31.5 && PositronX[2]<67.5"
	./direction ${zbegin[n]} ${zend[n]} zero_center_15mks "gtDiff < 15 && PositronX[0]>30 && PositronX[0]<66 && PositronX[1]>30 && PositronX[1]<66 && PositronX[2]>31.5 && PositronX[2]<67.5"
	./direction ${zbegin[n]} ${zend[n]} zero_center_4MeV "PositronEnergy > 4 && PositronX[0]>30 && PositronX[0]<66 && PositronX[1]>30 && PositronX[1]<66 && PositronX[2]>31.5 && PositronX[2]<67.5"
	./direction ${zbegin[n]} ${zend[n]} zero_center_n6MeV "NeutronEnergy > 6 && PositronX[0]>30 && PositronX[0]<66 && PositronX[1]>30 && PositronX[1]<66 && PositronX[2]>31.5 && PositronX[2]<67.5"
done
