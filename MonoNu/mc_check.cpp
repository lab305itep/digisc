#include <stdio.h>
#include <string.h>
#include <TChain.h>
#include <TCut.h>
#include <TFile.h>
#include <TH2.h>

/************************************************************************
 *		Check MC to info files correspondence			*
 ************************************************************************/

int main(int argc, char **argv)
{
	char str[2048];
	char str1[2048];
	char *nameMC;
	char *nameInfo;
	TFile *fMC;
	TFile *fInfo;
	TTree *tMC;
	TTree *tInfo;
	long long nMC, nInfo;

	if (argc < 2) {
		printf("Usage: %s mc_file_list.txt\n", argv[0]);
		return 100;
	}
	FILE *fIn = fopen(argv[1], "rt");
	if (!fIn) {
		printf("Can not open file %s : %m\n", argv[1]);
		return 10;
	}
	for(;;) {
		if (!fgets(str, sizeof(str), fIn)) break;
		if (strlen(str) < 2) continue;
		if (str[0] == '#') continue;
		str[strlen(str)-1] = '\0';
		strcpy(str1, str);
		nameMC = strtok(str, " \t");
		nameInfo = strtok(NULL, " \t");
		fMC = new TFile(nameMC);
		fInfo = new TFile(nameInfo);
		tMC = (TTree *) fMC->Get("DanssPair");
		tInfo = (TTree *) fInfo->Get("FromMC");
		if (!tMC || !tInfo) {
			printf("Trees are not found in %s\n", str1);
		} else {
			nMC = tMC->GetEntries();
			nInfo = tInfo->GetEntries();
			if (nMC != nInfo) printf("MC(%Ld) != Info(%Ld) @ %s\n", nMC, nInfo, str1);
		}
		delete fMC;
		delete fInfo;
	}
	fclose(fIn);
	return 0;
}
