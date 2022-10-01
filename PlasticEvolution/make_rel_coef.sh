#!/bin/bash
#PBS -N rel_coef
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/rel_coef.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/rel_coef.err
#PBS -l nodes=1
#PBS -q long
#PBS -l walltime=80:00:00

DIR=/home/clusters/rrcmpi/alekseev/igor/dvert
cd /home/itep/alekseev/igor/PlasticEvolution
date
for f in phe_103000_121999.root phe_33000_42999.root phe_53000_62999.root phe_83000_102999.root phe_13000_22999.root phe_23000_32999.root phe_43000_52999.root phe_63000_82999.root ; do
	./make_rel_coef ${DIR}/$f
done
date

exit 0
