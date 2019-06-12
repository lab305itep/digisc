#!/bin/bash
#PBS -N scan_12B
#PBS -q mpi
#PBS -o /home/clusters/rrcmpi/alekseev/igor/tmp/scan_12B.out
#PBS -e /home/clusters/rrcmpi/alekseev/igor/tmp/scan_12B.err
#PBS -l nodes=17
#PBS -l walltime=10:00:00
cd /home/itep/alekseev/igor/12B

mpirun --mca btl ^tcp run_many 'root -l -b -q'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.02)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.04)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.06)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.08)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.10)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.12)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.14)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.16)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.18)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.20)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.22)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.24)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.26)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.28)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.30)"'\
	'"makeMC_12B.C(\"/home/clusters/rrcmpi/alekseev/igor/root6n1/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root\", 0.32)"'
exit 0


#	'"makeExp_12B.C+O(2000, 55000, 0.88)"' '"makeExp_12B.C+O(2000, 55000, 0.885)"' '"makeExp_12B.C+O(2000, 55000, 0.89)"' '"makeExp_12B.C+O(2000, 55000, 0.895)"'\
#	'"makeExp_12B.C+O(2000, 55000, 0.90)"' '"makeExp_12B.C+O(2000, 55000, 0.905)"' '"makeExp_12B.C+O(2000, 55000, 0.91)"' '"makeExp_12B.C+O(2000, 55000, 0.915)"'\
#	'"makeExp_12B.C+O(2000, 55000, 0.92)"' '"makeExp_12B.C+O(2000, 55000, 0.925)"' '"makeExp_12B.C+O(2000, 55000, 0.93)"' '"makeExp_12B.C+O(2000, 55000, 0.935)"'\
#	'"makeExp_12B.C+O(2000, 55000, 0.94)"' '"makeExp_12B.C+O(2000, 55000, 0.945)"' '"makeExp_12B.C+O(2000, 55000, 0.95)"' '"makeExp_12B.C+O(2000, 55000, 0.955)"'\
#	'"makeExp_12B.C+O(2000, 55000, 0.96)"' '"makeExp_12B.C+O(2000, 55000, 0.965)"' '"makeExp_12B.C+O(2000, 55000, 0.97)"' '"makeExp_12B.C+O(2000, 55000, 0.975)"'\
#	'"makeExp_12B.C+O(2000, 55000, 0.98)"' '"makeExp_12B.C+O(2000, 55000, 0.985)"' '"makeExp_12B.C+O(2000, 55000, 0.99)"' '"makeExp_12B.C+O(2000, 55000, 0.995)"'\
#	'"makeExp_12B.C+O(2000, 55000, 1.00)"' '"makeExp_12B.C+O(2000, 55000, 1.005)"' \
