all: vert_muon digi_vert_sipm run_vert_mpi phe_hist phe_hist_strip make_rel_coef join_rel_coef run_bin_mpi phe_hist_corr

make_rel_coef: make_rel_coef.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

bin_hists: bin_hists.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

make_rel_coef_bin: make_rel_coef_bin.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

join_rel_coef: join_rel_coef.cpp
	g++ $^ -o $@

vert_muon: vert_muon.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

phe_hist: phe_hist.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

phe_hist_strip: phe_hist_strip.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

phe_hist_corr: phe_hist_corr.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

digi_vert_sipm: digi_vert_sipm.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs) -I../lib_v3.2 -L../lib_v3.2 -lReadDigiData

run_vert_mpi: run_vert_mpi.c
	mpicc -o $@ $^

run_bin_mpi: run_bin_mpi.c
	mpicc -o $@ $^

run_rel_mpi: run_rel_mpi.c
	mpicc -o $@ $^

