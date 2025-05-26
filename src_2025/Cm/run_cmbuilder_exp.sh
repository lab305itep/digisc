#!/bin/bash
#PBS -N build_248Cmd
#PBS -q long
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/build_248Cmd.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/build_248Cmd.err
#PBS -l nodes=1
#PBS -l walltime=150:00:00
cd /home/itep/alekseev/igor/src_2025/Cm
CMBUILDER=/home/itep/alekseev/igor/cmbuilder

for f in cm_127720_127772_8.2 cm_127720_127772_8.6 cm_14428_14485_8.2 cm_14428_14485_8.6 cm_50578_50647_8.2 cm_50578_50647_8.6 ; do
	${CMBUILDER} ${f}.list ${f}.root
done

exit 0
