ROOTLIB = $(shell root-config --libs)
ROOTINC = $(shell root-config --cflags)

all: stopped_muons run_stopped_mpi

stopped_muons: stopped_muons.cpp
	g++ $^ -o $@ ${ROOTINC} ${ROOTLIB}

run_stopped_mpi: run_stopped_mpi.c
	mpicc -o $@ $^
