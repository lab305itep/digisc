#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

#define TMPDIR "/home/clusters/rrcmpi/alekseev/igor/tmp/"

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
	int irc, iver;
	char *suffix;
	char *tdir;
	char *tcalib;
	
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
	tdir = getenv("DIGI_TARGETDIR");
	if (!tdir) tdir = "root6n";
//		Try possible file names:
//		V3
	iver = 3;
	sprintf(fname, "digi_v3.0/%3.3dxxx/danss_data_%6.6d_phys.digi", fnum/1000, fnum);
	irc = access(fname, R_OK);
	if (!irc) goto found;
	strcat(fname, ".bz2");
	irc = access(fname, R_OK);
	if (!irc) goto found;
//		V2
	iver = 2;
	sprintf(fname, "digi_v2.1/%3.3dxxx/danss_data_%6.6d_phys_rawrec.digi", fnum/1000, fnum);
	irc = access(fname, R_OK);
	if (!irc) goto found;
	strcat(fname, ".bz2");
	irc = access(fname, R_OK);
	if (!irc) goto found;
//		No digi file
	printf("Run %6.6d not found.\n", fnum);
	goto fin;
found:
//		create the list file
	sprintf(clist, "%s/%6.6d.list", TMPDIR, fnum);
	flist = fopen(clist, "wt");
	if (!flist) {
		printf("Can not open list file %s: %m\n", clist);
		goto fin;
	}
	fprintf(flist, "%s\n", fname);
	fclose(flist);
//		The run itself
	if (iver == 3) {
		setenv("DANSSRAWREC_HOME", "lib_v3.0", 1);
	} else {
		setenv("DANSSRAWREC_HOME", "lib_v2.1", 1);
	}
	tcalib = (fnum < 5469) ? "tcalib_cmnew_ss-d.calib" : "tcalib_5512_ss-d.calib";
	
	sprintf(str, "./digi_evtbuilder6_v%d -no_hit_tables -file %s -output %s/%3.3dxxx/danss_%6.6d.root "
		"-flag 0x50002 -tcalib %s", iver, clist, tdir, fnum/1000, fnum, tcalib);
	irc = system(str);
	if (irc) printf("Run %d: error %d returned: %m\n", fnum, irc);
//		delete list file
	unlink(clist);
//		time and print
	t1 = time(NULL);
	printf("Run %d: elapsed time %d s:\n", fnum, t1 - t0);
fin:
	MPI_Finalize();
	return 0;
}
