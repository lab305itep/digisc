DIGI_V3 = ../lib_v3.2
ROOTLIB = $(shell root-config --libs)
ROOTINC = $(shell root-config --cflags)
DIGILIB = -lReadDigiData

digi_tcalib: digi_tcalib.cpp
	g++ -O3 -o $@ $^ ${ROOTINC} -I${DIGI_V3} ${ROOTLIB} -L${DIGI_V3} ${DIGILIB}

run_tcalib_mpi: run_tcalib_mpi.c
	mpicc -o $@ $^

ana_tcalib: ana_tcalib.cpp
	g++ -O3 -o $@ $^ ${ROOTINC} ${ROOTLIB}

