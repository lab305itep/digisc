all: vert_muon digi_vert_sipm run_vert_mpi phe_hist phe_hist_strip

vert_muon: vert_muon.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

phe_hist: phe_hist.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

phe_hist_strip: phe_hist_strip.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

digi_vert_sipm: digi_vert_sipm.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs) -I../lib_v3.2 -L../lib_v3.2 -lReadDigiData

run_vert_mpi: run_vert_mpi.c
	mpicc -o $@ $^

