ROOTINC = -I/opt/fairsoft/include/root 
DIGI_V2 = lib_v2.1
DIGI_V3 = lib_v3.0
CLIB = -L/opt/gcc-5.3/lib64 -lgcc -lstdc++
ROOTLIB = -L/opt/fairsoft/lib/root -lCint -lCore -lRIO -lNet -lHist -lMinuit -lGraf -lGraf3d -lGpad -lTree -lRint \
    -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lSpectrum -lpthread -lm -ldl -rdynamic
DIGILIB = -lReadDigiData

All: digi_evtbuilder6_v2 digi_evtbuilder6_v3 pairbuilder7 muonpair pmt2sipm spectr5w background_calc xyz deadtime run_dead_mpi\
    run_digi_mpi run_pair_mpi run_stat_mpi run_spectr_mpi  run_bgnd_mpi run_pmt2sipm_mpi run_muon_mpi rootcheck

digi_evtbuilder6_v2: digi_evtbuilder6.cpp
	gcc -DDIGI_V2 -O3 -o $@ $^ ${ROOTINC} -I${DIGI_V2} ${CLIB} ${ROOTLIB} -L${DIGI_V2} ${DIGILIB}

digi_evtbuilder6_v3: digi_evtbuilder6.cpp
	gcc -DDIGI_V3 -O3 -o $@ $^ ${ROOTINC} -I${DIGI_V3} ${CLIB} ${ROOTLIB} -L${DIGI_V3} ${DIGILIB}

run_digi_mpi: run_digi_mpi.c
	mpicc -o $@ $^

run_pair_mpi: run_pair_mpi.c
	mpicc -o $@ $^

digi_zip: digi_zip.c
	mpicc -o $@ $^

run_muon_mpi: run_muon_mpi.c
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

pairbuilder6: pairbuilder6.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

pairbuilder7: pairbuilder7.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

deadtime: deadtime.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

rootcheck: rootcheck.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

muonpair: muonpair.cpp
	gcc $^ -o $@ -I${DIGI_V3} ${ROOTINC} ${DIGIINC} ${CLIB} ${ROOTLIB}

rawbgnd: rawbgnd.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

spectr5w: spectr5w.cpp HPainter2.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

xyz: xyz.cpp HPainter2.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

background_calc: background_calc.cpp HPainter2.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

pmt2sipm: pmt2sipm.cpp HPainter2.cpp
	gcc $^ -o $@ ${ROOTINC} ${CLIB} ${ROOTLIB}

clean:
	-rm -f digi_evtbuilder5 pairbuilder6 run_digi_mpi run_pair_mpi run_stat_mpi run_spectr_mpi spectr5w *.o
