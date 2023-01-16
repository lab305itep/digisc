#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

#define TMPDIR "/home/clusters/rrcmpi/alekseev/igor/tmp"
#define HITINFODIR "/home/clusters/rrcmpi/alekseev/igor/hitcheck"

//	argv[1] - the first digi file number
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	char clist[1024];
	char fname[1024];
	char hitinfo[1024];
	FILE *flist;
	int fnum;
	time_t t0, t1;
	int irc, iver;
	char *suffix;
	char *tdir;
	char *tcalib;
	char *runlist;
	char *runnum;
	char *ptr;
	FILE *rfile;
	int i;
	int SetVer = 0;
	
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
	tdir = getenv("OUT_DIR");
	if (!tdir) tdir = "root6n";
	//		Check if run from list is required and get its number
	runlist = getenv("DIGI_LIST");
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
	ptr = getenv("DIGI_VERSION");
	if (ptr) SetVer = strtol(ptr, NULL, 10);
//		Try possible file names:
	iver = 0;
//		V3
	if (!SetVer || SetVer == 3) {
		iver = 3;
		sprintf(fname, "digi_v3.0/%3.3dxxx/danss_data_%6.6d_phys.digi", fnum/1000, fnum);
		irc = access(fname, R_OK);
		if (!irc) goto found;
		strcat(fname, ".bz2");
		irc = access(fname, R_OK);
		if (!irc) goto found;
		iver = 3;
		sprintf(fname, "digi_rad/%3.3dxxx/danss_data_%6.6d_phys.digi", fnum/1000, fnum);
		irc = access(fname, R_OK);
		if (!irc) goto found;
		strcat(fname, ".bz2");
		irc = access(fname, R_OK);
		if (!irc) goto found;
	}
//		V2
	if (!SetVer || SetVer == 2) {
		iver = 2;
		sprintf(fname, "digi_v2.1/%3.3dxxx/danss_data_%6.6d_phys_rawrec.digi", fnum/1000, fnum);
		irc = access(fname, R_OK);
		if (!irc) goto found;
		strcat(fname, ".bz2");
		irc = access(fname, R_OK);
		if (!irc) goto found;
//		No digi file
		printf("Run %6.6d not found.\n", fnum);
	}
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
		setenv("DANSSRAWREC_HOME", "lib_v3.2", 1);
	} else {
		setenv("DANSSRAWREC_HOME", "lib_v2.1", 1);
	}
	tcalib = getenv("TCALIB");
	if (!tcalib) tcalib = "tcalib_sorted.txt";
	
	sprintf(hitinfo, "%s/%3.3dxxx/hits_%6.6d.txt.bz2", HITINFODIR, fnum/1000, fnum);
	
	sprintf(str, "./digi_evtbuilder6_v%d -no_hit_tables -file %s -output %s/%3.3dxxx/danss_%6.6d.root "
		"-flag 0x40002 -tcalib %s -hitinfo %s -deadlist main_dead.list", iver, clist, tdir, fnum/1000, fnum, tcalib, hitinfo);
//	Old flag=0x250002
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
