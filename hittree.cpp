#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <TFile.h>
#include <TLeaf.h>
#include <TTree.h>

//#define MASK ((1L<<45) - 1)
#define MASK ((1L<<60) - 1)

/*      Open data file either directly or via zcat etc, depending on the extension      */
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
	int i, N;
	long long gtA;
	long long gtB;
	struct RawStruct {
		unsigned short PmtCnt;
		unsigned short VetoCnt;
		unsigned short SiPmCnt;
	} Raw;
	char str[1024];
	char str_copy[1024];
	char *ptr;

	if (argc < 3) {
		printf("Add tree with hit information to a file\n");
		printf("Usage: %s root_file hit_file\n", argv[0]);
		return 10;
	}
	
	if (access(argv[1], W_OK | R_OK)) {
		printf("File %s either does not exist or can not be written: %m\n", argv[1]);
		return 20;
	}

	if (access(argv[2], R_OK)) {
		printf("File %s either does not exist or can not be read: %m\n", argv[2]);
		return 30;
	}
	
	FILE *fIn = OpenDataFile(argv[2]);
	if (!fIn) {
		printf("Something wrong with file: %s\n", argv[2]);
		return 40;
	}

	TFile *f = new TFile(argv[1], "UPDATE");
	TTree *evt = (TTree *) f->Get("DanssEvent");
	if (!evt) {
		printf("Something wrong with file: %s\n", argv[1]);
		return 50;
	}
	N = evt->GetEntries();
	TTree *t = new TTree("RawHits", "RawHits");
	t->Branch("RawHits", &Raw, "PmtCnt/s:VetoCnt/s:SiPmCnt/s");
	
	gtB = -1;
	for(i=0; i<N; i++) {
		evt->GetEntry(i);
		gtA = MASK & (evt->GetLeaf("globalTime"))->GetValueLong64();
		memset(&Raw, 0, sizeof(Raw));
		if (gtB < gtA) for (;;) {
			ptr = fgets(str, sizeof(str), fIn);
			if (!ptr) {
				printf("Unexpected EOF of %s @ %d: %m\n", argv[2], i);
				gtB = -1;
				break;
			}
			strcpy(str_copy, str);
			ptr = strtok(str, " \t");
			if (!ptr) continue;
			gtB = MASK & strtoll(ptr, NULL, 10);
			if (!gtB) continue;
			if (gtB >= gtA) break;
		} else {
			strcpy(str, str_copy);
			strtok(str, " \t");
		}
		if (gtB < 0) break;
		if (gtB == gtA) {		// trigger found
			ptr = strtok(NULL, " \t");
			if (!ptr) {
				printf("Bad string at %d: %s [file %s]\n", i, str_copy, argv[2]);
				goto fill;
			}
			Raw.PmtCnt = strtol(ptr, NULL, 10);
			ptr = strtok(NULL, " \t");
			if (!ptr) {
				printf("Bad string at %d: %s [file %s]\n", i, str_copy, argv[2]);
				goto fill;
			}
			Raw.VetoCnt = strtol(ptr, NULL, 10);
			ptr = strtok(NULL, " \t");
			if (!ptr) {
				printf("Bad string at %d: %s [file %s]\n", i, str_copy, argv[2]);
				goto fill;
			}
			Raw.SiPmCnt = strtol(ptr, NULL, 10);
		} else {			// trigger is somehow missing
			printf("Trigger %d at %Ld not found [file %s]\n", i, gtA, argv[2]);
		}
fill:
		t->Fill();
	}
	
	t->Write();
	f->Close();
	return 0;
}
