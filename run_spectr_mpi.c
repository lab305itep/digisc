#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int serial;
	char str[4096];
	time_t t0, t1;
	int irc;
	int begin;
	
	t0 = time(NULL);
	
	begin = (argc > 1) ? strtol(argv[1], NULL, 10) : 1;
//		Get our run number
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
//		The run itself
	sprintf(str, "./spectr6 %d", serial + begin);
	irc = system(str);
	if (irc) printf("Period %d: error %d returned: %m\n", serial + begin, irc);
//		time and print
	t1 = time(NULL);
	printf("Period %d: elapsed time %d s:\n", serial + begin, t1 - t0);
fin:
	MPI_Finalize();
}
