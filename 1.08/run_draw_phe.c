#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

// 121 run assumed
int main(int argc, char **argv)
{
	int serial;
	char str[4096];
	time_t t0, t1;
	int irc;
	int kr, kc;
	double r0SiPM = 1.000;
	double srSiPM = 0.005;
	double r0PMT  = 1.000;
	double srPMT  = 0.005;
	double c0SiPM = 0.100;
	double scSiPM = 0.005;
	double c0PMT  = 0.060;
	double scPMT  = 0.005;
	
	t0 = time(NULL);
//		Get our run number
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	kc = serial / 21 - 10;
	kr = serial % 21 - 10;
//		The run itself
	sprintf(str, "root -l -b -q -x \"draw_phe.C(%5.3f, %5.3f, 0.12, 0.12, %5.3f, %5.3f, 2000)\"", 
		r0SiPM + kr * srSiPM, r0PMT + kr * srPMT,
		c0SiPM + kc * scSiPM, c0PMT + kc * scPMT);
	irc = system(str);
	if (irc) printf("Run %s: error %d returned: %m\n", str, irc);
//		time and print
	t1 = time(NULL);
	printf("Run %s: elapsed time %d s:\n", str, t1 - t0);
fin:
	MPI_Finalize();
}
