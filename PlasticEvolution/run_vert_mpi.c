#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	argv[1] - the first run number
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	char fin[1024];
	char fout[1024];
	int fnum;
	time_t t0, t1;
	int irc;
	FILE *rfile;
	char *runlist;
	char *runnum;
	int i;
	const char *ptr;
	
	t0 = time(NULL);
//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 2) {
		printf("Usage %s run_number\n", argv[0]);
		goto fin;
	}
	fnum = strtol(argv[1], NULL, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	fnum += serial;
	//		Check if run from list is required and get its number
	runlist = getenv("PAIR_LIST");
	if (runlist) {
		rfile = fopen(runlist, "rt");
		if (!rfile) {
			fprintf(stderr, "Can not open the list file %s!\n", runlist);
			goto fin;
		}
		for (i=0; i<fnum; i++) {
			runnum = fgets(str, sizeof(str), rfile);
			if (!runnum) goto fin;
		}
		fnum = strtol(str, NULL, 10);
	}

//		check that file exists
//		The run itself
	sprintf(str, "./run_digi_vert.sh %d", fnum);
	irc = system(str);
	if (irc) printf("Run %d: error %d returned: %m\n", fnum, irc);
//		time and print
	t1 = time(NULL);
	printf("Run %d: elapsed time %d s:\n", fnum, t1 - t0);
fin:
	MPI_Finalize();
}
