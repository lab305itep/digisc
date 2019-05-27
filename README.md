# digisc
Analysis on super computer
Igor's data analysis.
Main analysis flow:
1) digi_evtbuilder - make root files from Ira's digi files
danss_submit run_digi_mpi <run_from> <run_to> <queue> <number_of_cpus> "OUT_DIR=<path_to_roots>"
2) run_digiMC.sh - script to run digi_evtbuilder on MC
3) rootcheck - check that root files were created.
4) hitcheck/hitcheck_NNNNN_MMMMM.sh - create raw hit information
5) run_hits_mpi.sh - add tree with raw hit information to root files
6) pairbuilder - make trigger pairs
danss_submit run_pair_mpi <run_from> <run_to> <queue> <number_of_cpus> "SRC_DIR=<path_to_roots> TGT_DIR=<path_to_pairs>"
7) run_stat_mpi.sh - calculate file statistics
8) run_dead_mpi.sh - calculate dead times. Tha same script can be used to run dead channels scan.
9) 