ROOTLIB = $(shell root-config --libs)
ROOTINC = $(shell root-config --cflags)

direction: direction.cpp ../HPainter2.cpp
	g++ $^ -o $@ ${ROOTINC} ${ROOTLIB}

directionMC: directionMC.cpp
	g++ $^ -o $@ ${ROOTINC} ${ROOTLIB}
