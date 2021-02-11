vert_muon : vert_muon.cpp
	g++ $^ -o $@ $(shell root-config --cflags --libs)

run_draw_phe: run_draw_phe.c
	mpicc -o $@ $^
