all : mcConvert run_convert_mpi

mcConvert : mcConvert.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

run_convert_mpi : run_convert_mpi.c
	mpicc $^ -o $@
