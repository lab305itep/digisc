#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	argv[1] - max runs in one shot
//	argv[2] - directory
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	int nRuns, i, j;
	time_t t0, t1;
	int irc;
	const char *dir;
	
	t0 = time(NULL);

//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 3) {
		printf("Usage %s runs_per_series directory\n", argv[0]);
		goto fin;
	}
	nRuns = strtol(argv[1], NULL, 0);
	dir = argv[2];
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	i = serial / nRuns;
	j = (serial % nRuns) + 1;
//		The run itself
	sprintf(str, "./mcConvert %s/DANSSmod%d_%d.root %s/DANSS%d_%d.root", dir, i, j, dir, i, j);
	irc = system(str);
	if (irc) printf("Run %d_%d: error [%s]: %m\n", i , j, str);
//		time and print
	t1 = time(NULL);
	printf("Run %d_%d: elapsed time %d s:\n", i, j, t1 - t0);
fin:
	MPI_Finalize();
}
