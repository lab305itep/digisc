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
	
	t0 = time(NULL);
//		Get our run number
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
//		The run itself
	sprintf(str, "/home/itep/alekseev/igor/background_calc %d ", serial + 1);
	if (argc > 1) strcat(str, argv[1]);
	printf("%s\n", str);
	irc = system(str);
	if (irc) printf("Period %d: error %d returned: %m\n", serial + 1, irc);
//		time and print
	t1 = time(NULL);
	printf("Period %d: elapsed time %d s:\n", serial + 1, t1 - t0);
fin:
	MPI_Finalize();
}
