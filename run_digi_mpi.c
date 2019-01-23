#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	argv[1] - the first digi file number
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	char clist[1024];
	char fname[1024];
	FILE *flist;
	int fnum;
	time_t t0, t1;
	int irc;
	int WasZipped;
	char *suffix;
	char *tdir;
	
	t0 = time(NULL);
//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 2) {
		printf("Usage %s first_run_number\n", argv[0]);
		goto fin;
	}
	fnum = strtol(argv[1], NULL, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	fnum += serial;
//		check that file exists
	WasZipped = 0;
	
	suffix = getenv("DIGI_SUFFIX");
	if (!suffix) {
		sprintf(str, "v2.1/%3.3dxxx", fnum/1000);
		suffix = str;
	} 
	
	tdir = getenv("DIGI_TARGETDIR");
	if (!tdir) tdir = "root6n";

	sprintf(fname, "/home/clusters/rrcmpi/alekseev/igor/digi/%s/danss_data_%6.6d_phys_rawrec.digi", suffix, fnum);
	irc = access(fname, R_OK);
	if (irc) {
		sprintf(str, "%s.bz2", fname);
		irc = access(str, R_OK);
		if (irc) {
			printf("Run %6.6d not found at %s(%s)\n", fnum, fname, str);
			goto fin;
		}
		sprintf(str, "bzcat %s.bz2 > /home/clusters/rrcmpi/alekseev/igor/tmp/danss_data_%6.6d_phys_rawrec.digi", fname, fnum);
		irc = system(str);
		if (irc) {
			printf("Run %d decompression error: %d - %m\n", fnum, irc);
			goto fin;
		}
		sprintf(fname, "/home/clusters/rrcmpi/alekseev/igor/tmp/danss_data_%6.6d_phys_rawrec.digi", fnum);
		WasZipped = 1;
	}
//		create the list file
	sprintf(clist, "/home/clusters/rrcmpi/alekseev/igor/tmp/%6.6d.list", fnum);
	flist = fopen(clist, "wt");
	if (!flist) {
		printf("Can not open list file %s: %m\n", clist);
		goto fin;
	}
	fprintf(flist, "%s\n", fname);
	fclose(flist);
//		The run itself
	sprintf(str, "/home/itep/alekseev/igor/digi_evtbuilder6 -no_hit_tables -file %s -output /home/clusters/rrcmpi/alekseev/igor/%s/%3.3dxxx/danss_%6.6d.root "
		"-flag 0x50002 -tcalib tcalib_5512_ss-d.calib", clist, tdir, fnum/1000, fnum);
	irc = system(str);
	if (irc) printf("Run %d: error %d returned: %m\n", fnum, irc);
//		delete list file
	unlink(clist);
	if (WasZipped) unlink(fname);
//		time and print
	t1 = time(NULL);
	printf("Run %d: elapsed time %d s:\n", fnum, t1 - t0);
fin:
	MPI_Finalize();
}
