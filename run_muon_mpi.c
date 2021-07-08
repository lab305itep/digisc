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
	char *ptr;
	char *src_dir = "/home/clusters/rrcmpi/alekseev/igor/root8n1/";
	char *tgt_dir = "/home/clusters/rrcmpi/alekseev/igor/muon8n1/";
	int fnum;
	time_t t0, t1;
	int irc;
	
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
	
	ptr = getenv("SRC_DIR");
	if (ptr) src_dir = ptr;
	ptr = getenv("TGT_DIR");
	if (ptr) tgt_dir = ptr;
//		check that file exists
	sprintf(fin, "%s/%3.3dxxx/danss_%6.6d.root", src_dir, fnum/1000, fnum);
	sprintf(fout, "%s/%3.3dxxx/muon_%6.6d.root", tgt_dir, fnum/1000, fnum);
	irc = access(fin, R_OK);
	if (irc) {
		printf("Run %6.6d not found at %s\n", fnum, fin);
		goto fin;
	}
//		The run itself
	sprintf(str, "mkdir -p %s/%3.3dxxx/", tgt_dir, fnum/1000);
	irc = system(str);
	if (irc) printf("%s - %d: error %d returned: %m\n", str, fnum, irc);
	sprintf(str, "./muonpair %s %s", fin, fout);
	irc = system(str);
	if (irc) printf("Run %d: error %d returned: %m\n", fnum, irc);
//		time and print
	t1 = time(NULL);
	printf("Run %d: elapsed time %d s:\n", fnum, t1 - t0);
fin:
	MPI_Finalize();
}
