#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	argv[1] - the first run number
//	argv[2] - runs per process
//	argv[3] - root directory
//	We will process runs argv[1] + k*argv[2], ..., argv[1] + (k+1)*argv[2] - 1
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	int fnum, pnum;
	time_t t0, t1;
	int irc;
	const char *prog = "deadtime";
	char *ptr;
	
	t0 = time(NULL);
//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 4) {
		printf("Usage %s first_run_number runs_per_process root_directory\n", argv[0]);
		goto fin;
	}
	fnum = strtol(argv[1], NULL, 0);
	pnum = strtol(argv[2], NULL, 0);
	
	ptr = getenv("DEAD_PROG");
	if (ptr) prog = ptr;
	
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	fnum += serial*pnum;
//		The run itself
	sprintf(str, "./%s %d %d %s", prog, fnum, fnum+pnum-1, argv[3]);
	irc = system(str);
	if (irc) printf("Run %d: error %d returned: %m\n", fnum, irc);
//		time and print
	t1 = time(NULL);
	printf("Run %d: elapsed time %d s:\n", fnum, t1 - t0);
fin:
	MPI_Finalize();
}
