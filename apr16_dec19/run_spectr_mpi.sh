#!/bin/bash
#PBS -N digi_spectr_base
#PBS -q mpi
#PBS -l walltime=0:30:00
#PBS -l nodes=433
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/spectr.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/spectr.err
cd /home/itep/alekseev/igor
export SPECTR_BGSCALE=2.7
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair7n10
export OUT_DIR=/home/clusters/rrcmpi/alekseev/igor/apr16_dec19/base.data
export SPECTR_WHAT="((PositronHits == 1) ? (PositronEnergy+0.0292)/1.043 : (PositronEnergy-0.1779)/0.9298)"
export SPECTR_MUCUT="gtFromVeto > 60"
export SPECTR_CUT00="((gtFromPrevious > 45  || gtFromPrevious == gtFromVeto) && gtToNext > 80 && EventsBetween == 0)"
export SPECTR_CUT01="gtFromShower > 200 || ShowerEnergy < 800"
export SPECTR_CUT02="PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)"
export SPECTR_CUT03="PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)"
export SPECTR_CUT04="PositronX[2] > 3.5 && PositronX[2] < 95.5"
export SPECTR_CUT05="gtDiff > 2"
export SPECTR_CUT06="AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10 && AnnihilationMax < 0.8"
export SPECTR_CUT07="((PositronHits == 1) ? (PositronEnergy+0.0292)/1.043 : (PositronEnergy-0.1779)/0.9298) > 0.5"
export SPECTR_CUT08="((PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || Distance < 45) && Distance < 55"
export SPECTR_CUT09="NeutronEnergy > 3.5 && NeutronEnergy < 15.0 && NeutronHits >= 3"
export SPECTR_CUT10="!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))"

# mkdir -p $OUT_DIR
# rm -f ${OUT_DIR}/*.root
# mpirun --mca btl ^tcp run_spectr_mpi

NAME=${OUT_DIR/.data/.root}
# root -l -b -q "join_periods.C(\"${NAME}\", \"$OUT_DIR\")"
root -l -b -q "danss_draw_report.C(\"${NAME}\", $SPECTR_BGSCALE)"
root -l -b -q "apr16_dec19/danss_calc_ratio_v6.C(\"${NAME}\", $SPECTR_BGSCALE)"

exit 0
#//##PBS -l pvmem=2Gb
