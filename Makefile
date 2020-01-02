DIGI_V2 = lib_v2.0
DIGI_V3 = lib_v3.2
#CLIB = -lgcc -lstdc++
CLIB = 
ROOTLIB = $(shell root-config --libs)
ROOTINC = $(shell root-config --cflags)
DIGILIB = -lReadDigiData

All: digi_evtbuilder6_v3 pairbuilder7 muonpair pmt2sipm spectr5w background_calc xyz deadtime run_dead_mpi\
    run_digi_mpi run_pair_mpi run_stat_mpi run_spectr_mpi  run_bgnd_mpi run_pmt2sipm_mpi run_muon_mpi rootcheck deadchannels \
    hittree run_hits_mpi cmbuilder spectr6 background_calc_72g

digi_evtbuilder6_v2: digi_evtbuilder6.cpp
	g++ -DDIGI_V2 -O3 -o $@ $^ ${ROOTINC} -I${DIGI_V2} ${CLIB} ${ROOTLIB} -L${DIGI_V2} ${DIGILIB} -lMinuit

digi_evtbuilder6_v3: digi_evtbuilder6.cpp
	g++ -DDIGI_V3 -O3 -o $@ $^ ${ROOTINC} -I${DIGI_V3} ${CLIB} ${ROOTLIB} -L${DIGI_V3} ${DIGILIB}

run_digi_mpi: run_digi_mpi.c
	mpicc -o $@ $^

run_pair_mpi: run_pair_mpi.c
	mpicc -o $@ $^

digi_zip: digi_zip.c
	mpicc -o $@ $^

run_muon_mpi: run_muon_mpi.c
	mpicc -o $@ $^

run_hits_mpi: run_hits_mpi.c
	mpicc -o $@ $^

run_stat_mpi: run_stat_mpi.c
	mpicc -o $@ $^

run_dead_mpi: run_dead_mpi.c
	mpicc -o $@ $^

run_spectr_mpi: run_spectr_mpi.c
	mpicc -o $@ $^

run_bgnd_mpi: run_bgnd_mpi.c
	mpicc -o $@ $^

run_pmt2sipm_mpi: run_pmt2sipm_mpi.c
	mpicc -o $@ $^

cmbuilder: cmbuilder.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

pairbuilder6: pairbuilder6.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

pairbuilder7: pairbuilder7.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

deadtime: deadtime.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

hittree: hittree.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

deadchannels: deadchannels.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

rootcheck: rootcheck.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

muonpair: muonpair.cpp
	g++ $^ -o $@ -I${DIGI_V3} ${ROOTINC} ${DIGIINC} ${CLIB} ${ROOTLIB}

rawbgnd: rawbgnd.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

spectr5w: spectr5w.cpp HPainter2.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

spectr6: spectr6.cpp HPainter2.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

xyz: xyz.cpp HPainter2.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

background_calc: background_calc.cpp HPainter2.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

background_calc_72g: background_calc_72g.cpp HPainter2.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

pmt2sipm: pmt2sipm.cpp HPainter2.cpp
	g++ $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

clean:
	-rm -f digi_evtbuilder5 digi_evtbuilder6_v* pairbuilder6 run_digi_mpi run_pair_mpi run_stat_mpi run_spectr_mpi spectr5w *.o
