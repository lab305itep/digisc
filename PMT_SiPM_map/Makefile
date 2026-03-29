ROOTLIB = $(shell root-config --libs)
ROOTINC = $(shell root-config --cflags)

All: do_hists

do_hists: do_hists.cpp
	g++ $^ -o $@ ${ROOTINC} ${ROOTLIB}

clean:
	-rm -f do_hists
