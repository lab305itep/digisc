#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <TFile.h>
#include <TLeaf.h>
#include <TTree.h>

//#define MASK ((1L<<42) - 1)
#define MASK ((1L<<45) - 1)
#define MAXTRIG		4000000

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
	int i, j, K, N;
	long long gtA;
	long long gtB;
	struct RawStruct {
		unsigned short PmtCnt;
		unsigned short VetoCnt;
		unsigned short SiPmCnt;
	};
	struct RawStruct Raw;
	struct RawArrayStruct {
		long long tag;
		struct RawStruct data;
	};
	struct RawArrayStruct *RawArray;
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
	
	RawArray = (struct RawArrayStruct *) malloc(MAXTRIG * sizeof(struct RawArrayStruct));
	if (!RawArray) {
		printf("No memory: %m\n");
		return 35;
	}
	
	FILE *fIn = OpenDataFile(argv[2]);
	if (!fIn) {
		printf("Something wrong with file: %s\n", argv[2]);
		free(RawArray);
		return 40;
	}
	
	K = 0;
	for (;;) {
		ptr = fgets(str, sizeof(str), fIn);
		if (!ptr) break;	// EOF
		strcpy(str_copy, str);
		ptr = strtok(str, " \t");
		if (!ptr) continue;
		if (!isdigit(ptr[0])) continue;
		RawArray[K].tag = MASK & strtoll(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Bad string at %d: %s [file %s]\n", i, str_copy, argv[2]);
			continue;
		}
		RawArray[K].data.PmtCnt = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Bad string at %d: %s [file %s]\n", i, str_copy, argv[2]);
			continue;
		}
		RawArray[K].data.VetoCnt = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Bad string at %d: %s [file %s]\n", i, str_copy, argv[2]);
			continue;
		}
		RawArray[K].data.SiPmCnt = strtol(ptr, NULL, 10);
		K++;
	}
	pclose(fIn);

	TFile *f = new TFile(argv[1], "UPDATE");
	TTree *evt = (TTree *) f->Get("DanssEvent");
	if (!evt) {
		printf("Something wrong with file: %s\n", argv[1]);
		return 50;
	}
	N = evt->GetEntries();
	TTree *t = new TTree("RawHits", "RawHits");
	t->Branch("RawHits", &Raw, "PmtCnt/s:VetoCnt/s:SiPmCnt/s");
	
	j = 0;
	
	for(i=0; i<N; i++) {
		evt->GetEntry(i);
		gtA = MASK & (evt->GetLeaf("globalTime"))->GetValueLong64();
		memset(&Raw, 0, sizeof(Raw));
		for (;j<K;j++) {
			gtB = RawArray[j].tag;
			if (gtB == gtA) break;
		} 
		if (gtB != gtA) {	// try to start from the beginning
			for (j=0;j<K;j++) {
				gtB = RawArray[j].tag;
				if (gtB == gtA) break;
			}
		}
		if (gtB == gtA) {		// trigger found
			memcpy(&Raw, &RawArray[j].data, sizeof(struct RawStruct));
		} else {			// trigger is somehow missing
			printf("Trigger %d at %Ld not found [file %s]\n", i, gtA, argv[2]);
		}
		t->Fill();
	}
	
	t->Write();
	f->Close();
	free(RawArray);
	return 0;
}
