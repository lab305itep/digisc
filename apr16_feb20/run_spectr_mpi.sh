#!/bin/bash
#PBS -N digi_spectr
#PBS -q mpi
#PBS -l walltime=10:00:00
#PBS -l nodes=450
####PBS -l pvmem=2Gb
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/spectr.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/spectr.err
cd /home/itep/alekseev/igor
export STAT_ALL=apr16_feb20/stat_all_n2.txt
export SPECTR_BGSCALE=2.0
export PAIR_DIR=/home/clusters/rrcmpi/alekseev/igor/pair7n14
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
# v1,2: export SPECTR_CUT08="(PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || (Distance < 40 && Distance < 21 + 4.3 * PositronEnergy)"
export SPECTR_CUT08="(PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || (Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy))"
# v1: export SPECTR_CUT09="Distance < 48 && Distance < 29 + 4.3 * PositronEnergy && Distance < 70 - 4.8 * PositronEnergy"
# v2: export SPECTR_CUT09="Distance < 48 && Distance < 29 + 4.3 * PositronEnergy"
export SPECTR_CUT09="Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)"
# v1: export SPECTR_CUT10="NeutronEnergy > 4.8 - 0.75 * PositronEnergy && NeutronEnergy > -0.33 + 0.33 * PositronEnergy && NeutronEnergy > 1.5"
# v2: export SPECTR_CUT10="NeutronEnergy > 4.8 - 0.75 * PositronEnergy && NeutronEnergy > 1.5"
export SPECTR_CUT10="NeutronEnergy > 1.5 + 3 * exp(-0.13 * PositronEnergy*PositronEnergy)"
export SPECTR_CUT11="NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20"
export SPECTR_CUT12="!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))"
export SPECTR_CUT13="PositronHits < 8"

export OUT_DIR=/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v14f2.data

date
mkdir -p $OUT_DIR
rm -f ${OUT_DIR}/*.root
mpirun --mca btl ^tcp run_spectr_mpi

NAME=${OUT_DIR/.data/.root}
root -l -b -q "join_periods.C(\"${NAME}\", \"$OUT_DIR\")"
root -l -b -q "danss_draw_report.C(\"${NAME}\", $SPECTR_BGSCALE)"
root -l -b -q "apr16_feb20/danss_calc_ratio_v6.C(\"${NAME}\", $SPECTR_BGSCALE)"
date
exit 0
