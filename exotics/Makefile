all: exotics run_exotics_mpi

exotics: exotics.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

run_exotics_mpi: run_exotics_mpi.c
	mpicc -o $@ $^
