#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <TFile.h>
#include <TTree.h>

int checkdigi(int fnum)
{
	const char pathpattern[][256] = {
		"digi_v3.0/%3.3dxxx/danss_data_%6.6d_phys.digi",
		"digi_rad/%3.3dxxx/danss_data_%6.6d_phys.digi",
		"digi_led/%3.3dxxx/danss_data_%6.6d_phys.digi"
	};
	int i, irc;
	char fname[1024];
//		Try possible file names:
	for (i=0; i < sizeof(pathpattern) / sizeof(pathpattern[0]); i++) {
		sprintf(fname, pathpattern[i], fnum/1000, fnum);
		irc = access(fname, R_OK);
		if (!irc) return true;
		strcat(fname, ".bz2");
		irc = access(fname, R_OK);
		if (!irc) return true;
	}
	return false;
}

int checkroot(int fnum, const char *pattern)
{
	int irc;
	char fname[1024];
	const char *rnames[] = {"DanssEvent", "DanssInfo", "RawHits"};
	const char *pnames[] = {"DanssPair", "DanssRandom", "DanssMuRandom", "SumInfo"};
	const char *mnames[] = {"MuonPair", "MuonRandom", "SumInfo"};
	TFile *f;
	TTree *t;
	int i;
	
	sprintf(fname, pattern, fnum/1000, fnum);
	irc = access(fname, R_OK);
	if (irc) return false;
	f = new TFile(fname);
	if (f->IsZombie()) return false;
	if (!f->GetNkeys()) {
		fprintf(stderr, "Run %d: No keys found!\n", fnum);
		goto ret_err;
	}
	if (f->TestBit(TFile::kRecovered)) {
		fprintf(stderr, "Run %d: Was not closed propely!\n", fnum);
		goto ret_err;
	}
	t = (TTree *) f->Get(rnames[0]);
	if (t) {
		for (i = 1; i < sizeof(rnames) / sizeof(rnames[0]); i++) {
			t = (TTree *) f->Get(rnames[i]);
			if (!t) {
				fprintf(stderr, "Run %d: No tree %s!\n", fnum, rnames[i]);
				goto ret_err;
			}
		}
		goto ret_OK;
	}
	t = (TTree *) f->Get(pnames[0]);
	if (t) {
		for (i = 1; i < sizeof(pnames) / sizeof(pnames[0]); i++) {
			t = (TTree *) f->Get(pnames[i]);
			if (!t) {
				fprintf(stderr, "Run %d: No tree %s!\n", fnum, pnames[i]);
				goto ret_err;
			}
		}
		goto ret_OK;
	}
	t = (TTree *) f->Get(mnames[0]);
	if (t) {
		for (i = 1; i < sizeof(mnames) / sizeof(mnames[0]); i++) {
			t = (TTree *) f->Get(mnames[i]);
			if (!t) {
				fprintf(stderr, "Run %d: No tree %s!\n", fnum, mnames[i]);
				goto ret_err;
			}
		}
		goto ret_OK;
	}
	
ret_err:
	f->Close();
	return false;
ret_OK:
	f->Close();
	return true;
}

int checkroot(const char *rname, const char *pattern)
{
	int irc;
	char fname[1024];
	TFile *f;
	
	sprintf(fname, pattern, rname);
	irc = access(fname, R_OK);
	if (irc) return false;
	f = new TFile(fname);
	if (f->IsZombie()) return false;
	if (!f->GetNkeys()) {
		f->Close();
		return false;
	}
	irc = f->TestBit(TFile::kRecovered);
	if (irc) {
		f->Close();
		return false;
	}
	f->Close();
	return true;
}

void Usage(void)
{
	printf("Check validity of root files and print invalid or absent run numbers\n");
	printf("Usage: ./rootcheck patternA patternB [from to]\n");
}

//	Check validity of root files and print invalid or absent run numbers
//	argv[1] - patternA - reference file name pattern like "root6n/%3.3dxxx/danss_%6.6d.root". 
//		Could be "digi" to compare to digi set
//		Could be pos to check against positions.h
//	argv[2] - patternB - to be checked
//	[argv[3] - from - (1-100000) range assumed
//	argv[4]] - to
int main(int argc, char **argv)
{
#include "positions.h"
	int from, to;
	char *patternA;
	char *patternB;
	int i, irc;
	int Cnt[2];
	
	from = 1;
	to = 100000;
	
	if (argc < 3) {
		Usage();
		return 10;
	}
	patternA = argv[1];
	patternB = argv[2];
	if (argc > 3) from = strtol(argv[3], NULL, 10);
	if (argc > 4) to = strtol(argv[4], NULL, 10);
	
//	printf("%s[1] %s[2] %s[3] %s[4]\n", argv[1], argv[2], argv[3], argv[4]);
//	return 100;
	
	Cnt[0] = Cnt[1] = 0;
	if (strcasecmp(patternA, "pos")) {
		for (i = from; i <= to; i++) {
			irc = strcasecmp(patternA, "digi") ? checkroot(i, patternA) : checkdigi(i);
			if (!irc) continue;
			Cnt[0]++;
			irc = checkroot(i, patternB);
			if (!irc) {
				printf("%d\n", i);
			} else {
				Cnt[1]++;
			}
		}
	} else {
		for (i = from; i <= to; i++) {
			if (i < 1 || i > sizeof(positions) / sizeof(positions[0])) continue;
			Cnt[0]++;
			irc = checkroot(positions[i-1].name, patternB);
			if (!irc) {
				printf("%d\n", i);
			} else {
				Cnt[1]++;
			}
		}
	}
	printf("Checked: %d files; OK: %d files, range [%d - %d]\n", Cnt[0], Cnt[1], from, to);
	return 0;
}
