ROOTLIB = $(shell root-config --libs)
ROOTINC = $(shell root-config --cflags)

all: stopped_muons run_stopped_mpi add_depth bragg_MC fit_depth

stopped_muons: stopped_muons.cpp
	g++ $^ -o $@ ${ROOTINC} ${ROOTLIB}

run_stopped_mpi: run_stopped_mpi.c
	mpicc -o $@ $^

add_depth: add_depth.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

fit_depth: fit_depth.cpp
	g++ $^ -o $@ -lMinuit $(shell root-config --cflags --libs)

bragg_MC: bragg_MC.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)
