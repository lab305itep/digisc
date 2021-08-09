#!/bin/bash
#PBS -N digi_spectr_base_1
#PBS -q mpi
#PBS -l walltime=3:00:00
#PBS -l nodes=582
#PBS -l pvmem=2Gb
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/spectr_1.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/spectr_1.err
cd /home/itep/alekseev/igor
export STAT_ALL=stat_all.txt
export SPECTR_BGSCALE=1.9
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair8n1
export OUT_DIR=/home/clusters/rrcmpi/alekseev/igor/apr16_apr21/base_single.data
export SPECTR_WHAT="PositronEnergy"
export SPECTR_MUCUT="gtFromVeto > 90"
export SPECTR_CUT00="gtFromPrevious > 50 && gtToNext > 80 && EventsBetween == 0"
export SPECTR_CUT01="gtFromShower > 120 || ShowerEnergy < 800"
export SPECTR_CUT02="PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)"
export SPECTR_CUT03="PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)"
export SPECTR_CUT04="PositronX[2] > 3.5 && PositronX[2] < 95.5"
export SPECTR_CUT05="gtDiff > 1"
export SPECTR_CUT06="AnnihilationEnergy < 1.2 && AnnihilationGammas < 12"
export SPECTR_CUT07="PositronEnergy > 0.5"
export SPECTR_CUT08="(PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || (Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy))"
export SPECTR_CUT09="Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)"
export SPECTR_CUT10="NeutronEnergy > 1.5 + 3 * exp(-0.13 * PositronEnergy*PositronEnergy)"
export SPECTR_CUT11="NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20"
export SPECTR_CUT12="PositronHits == 1 && AnnihilationGammas >= 1 && AnnihilationEnergy >= 0.1"
export SPECTR_CUT13="PositronHits < 8"

mkdir -p $OUT_DIR
rm -f ${OUT_DIR}/*.root
mpirun --mca btl ^tcp run_spectr_mpi $i

NAME=${OUT_DIR/.data/.root}
root -l -b -q "join_periods.C(\"${NAME}\", \"$OUT_DIR\")"
root -l -b -q "danss_draw_report.C(\"${NAME}\", $SPECTR_BGSCALE)"
root -l -b -q "apr16_apr21/danss_calc_ratio_v6.C(\"${NAME}\", $SPECTR_BGSCALE, 0.5)"

exit 0
