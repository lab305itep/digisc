ROOTLIB = $(shell root-config --libs)
ROOTINC = $(shell root-config --cflags)

All: do_hists do_muons

do_hists: do_hists.cpp
	g++ $^ -o $@ ${ROOTINC} ${ROOTLIB}

do_muons: do_muons.cpp
	g++ $^ -o $@ ${ROOTINC} ${ROOTLIB}

clean:
	-rm -f do_hists do_muons
