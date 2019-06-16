#!/bin/bash
#PBS -N digi_spectr_base_1.00
#PBS -q mpi
#PBS -l walltime=0:30:00
#PBS -l nodes=378
#PBS -l pvmem=2Gb
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/spectr_base_1.00.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/spectr_base_1.00.err
cd /home/itep/alekseev/igor
export SPECTR_BGSCALE=2.46
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair7n4
export OUT_DIR=/home/clusters/rrcmpi/alekseev/igor/apr16_mar19_new_1.00
export SPECTR_WHAT="PositronEnergy"
export SPECTR_MUCUT="gtFromVeto > 60"
export SPECTR_CUT00="((gtFromPrevious > 45  || gtFromPrevious == gtFromVeto) && gtToNext > 80 && EventsBetween == 0)"
export SPECTR_CUT01="gtFromShower > 200 || ShowerEnergy < 800"
export SPECTR_CUT02="PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)"
export SPECTR_CUT03="PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)"
export SPECTR_CUT04="PositronX[2] > 3.5 && PositronX[2] < 95.5"
export SPECTR_CUT05="gtDiff > 2"
export SPECTR_CUT06="AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10 && AnnihilationMax < 0.8"
export SPECTR_CUT07="PositronEnergy > 1"
export SPECTR_CUT08="((PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || Distance < 45) && Distance < 55"
export SPECTR_CUT09="NeutronEnergy > 3.5 && NeutronEnergy < 15.0 && NeutronHits >= 3"
export SPECTR_CUT10="!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))";

mkdir -p $OUT_DIR
rm -f ${OUT_DIR}/*.root
mpirun --mca btl ^tcp run_spectr_mpi

NAME=`basename $OUT_DIR`.root
	root -l -b -q "join_periods.C(\"${NAME}\", \"$OUT_DIR\")"
	root -l -b -q "danss_draw_report.C(\"${NAME}\", $SPECTR_BGSCALE)"
	root -l -b -q "danss_calc_ratio_v5.C(\"${NAME}\", $SPECTR_BGSCALE)"

exit 0
