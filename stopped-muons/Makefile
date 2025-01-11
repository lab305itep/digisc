ROOTLIB = $(shell root-config --libs)
ROOTINC = $(shell root-config --cflags)

stopped_muons: stopped_muons.cpp
	g++ $^ -o $@ ${ROOTINC} ${ROOTLIB}
