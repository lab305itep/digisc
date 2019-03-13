//	Extract number of raw hits per tirgger
//	I.G. Alekseev & D.N. Svirida, ITEP, Moscow, 2019

#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "recformat.h"

#define MOD_PMT		1
#define MOD_VETO	3
#define GLOBALTIME_WRAP	(1LL << 45)
#define BSIZE		0x100000

int GetFileRecord(char *buf, int buf_size, FILE *f)
{
	int irc;
	int len;
//		Read length
	irc = fread(buf, sizeof(int), 1, f);
	if (irc <= 0) return irc;	// Error or _normal_ EOF

	len = *((int *)buf);
	if (len < sizeof(int) || len > buf_size) return -2;	// strange length

//		Read the rest
	irc = fread(&buf[sizeof(int)], len - sizeof(int), 1, f);
	return irc;	// Error and strange EOF and normal read
}


/*	Open data file either directly or via zcat etc, depending on the extension	*/
FILE* OpenDataFile(const char *fname) 
{
	char cmd[1024];
	FILE *f;

	if (strstr(fname, ".bz2")) {
		sprintf(cmd, "bzcat %s", fname);
		f = popen(cmd, "r");
	} else if (strstr(fname, ".gz")) {
		sprintf(cmd, "zcat %s", fname);
		f = popen(cmd, "r");
	} else if (strstr(fname, ".xz")) {
		sprintf(cmd, "xzcat %s", fname);
		f = popen(cmd, "r");
	} else {
		f = fopen(fname, "rb");
	}
	return f;
}

int main(int argc, char **argv)
{
	char *buf;
	struct rec_header_struct *head;
	struct hw_rec_struct *rec;
	int irc;
	int mod, chan;
	int iptr, len;
	int PmtCnt, VetoCnt, SiPmCnt;
	FILE *fIn;
	FILE *fOut;
	long long globalTime;
	long long lastGlobalTime;
	long long globalTimeWrap;

	if (argc < 3) {
		printf("Create list with number of raw hits per event\n");
		printf("Usage: ./%s data_file_name list_file_name\n", argv[0]);
		return 10;
	}
	buf = (char *)malloc(BSIZE);
	if (!buf) {
		printf("FATAL: no memory - %m (procassing %s)!\n", argv[1]);
		return 100;
	}
	head = (struct rec_header_struct *) buf;
	
	fIn = OpenDataFile(argv[1]);
	if (!fIn) {
		printf("Can not open input file %s (%m)\n", argv[1]);
		free(buf);
		return 20;
	}
	fOut = fopen(argv[2], "wt");
	if (!fOut) {
		printf("Can not open output file %s (%m)\n", argv[2]);
		free(buf);
		fclose(fIn);
		return 30;
	}
	//             123456789012345 1234567 1234567 1234567
	fprintf(fOut, "* globalTime    nPMT    nVeto   nSiPm\n");
	lastGlobalTime = 0;
	globalTimeWrap = 0;
	
	for(;;) {
		irc = GetFileRecord(buf, BSIZE, fIn);
		if (irc <= 0) break;
		
		if (head->type & REC_EVENT) {
			globalTime = -1;
			PmtCnt = VetoCnt = SiPmCnt = 0;
			for (iptr = sizeof(struct rec_header_struct); iptr < head->len; iptr += len) {		// record by record
				rec = (struct hw_rec_struct *) &buf[iptr];
				len = (rec->len + 1) * sizeof(short);
				if (iptr + len > head->len) {
					printf("Unexpected end of event: iptr = %d, len = %d, total len = %d.\n", iptr, len, head->len);
					break;
				}
				mod = rec->module;
				if (mod < 1 || mod > 50) {
					printf("Module %d is out of range.\n", mod);
					continue;
				}
				switch(rec->type) {
				case TYPE_MASTER:
					switch(mod) {
					case MOD_PMT:
						PmtCnt++;
						break;
					case MOD_VETO:
						VetoCnt++;
						break;
					default:
						SiPmCnt++;
					}
					break;
				case TYPE_TRIG:
					globalTime = rec->d[1] + (((long long) rec->d[2]) << 15) + (((long long) rec->d[3]) << 30);
					break;
				}
			}
			if (globalTime < 0) {
				printf("Trigger record (type = 2) was not found.\n");
				continue;
			}
			if (globalTime < lastGlobalTime) globalTimeWrap += GLOBALTIME_WRAP;
			lastGlobalTime = globalTime;
			globalTime += globalTimeWrap;
			fprintf(fOut, "%15Ld %7d %7d %7d\n", globalTime, PmtCnt, VetoCnt, SiPmCnt);
		}
	}
	fclose(fIn);
	fprintf(fOut, "*EOF\n");
	fclose(fOut);
	free(buf);
	return 0;
}
