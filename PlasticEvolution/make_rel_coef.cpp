/****************************************************************
 *  Take graph with absolute coefficients and make relative	*
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TFile.h>
#include <TH1D.h>

#define PHEDIR "/home/clusters/rrcmpi/alekseev/igor/dvert/phe"

int main(int argc, char **argv)
{
	struct FileListStruct {
		const char *name;
		int begin;
		int end;
	} FileList[] = {
		{"phe_2000_12999",      2000,  12999},
		{"phe_13000_22999",    13000,  22999},
		{"phe_23000_32999",    23000,  32999},
		{"phe_33000_42999",    33000,  42999},
		{"phe_43000_52999",    43000,  52999},
		{"phe_53000_62999",    53000,  62999},
		{"phe_63000_82999",    63000,  82999},
		{"phe_83000_102999",   83000, 102999},
		{"phe_103000_121999", 103000, 121999}
	};
	struct HeadStruct {
		int adc;
		int chan;
		int num;
		int run_begin;
		int run_step;
		int run_end;
	} head;
	FILE *fOut;
	int i, j, k, m, N, ind;
	int cnt;
	double sum, val;
	TH1D *h;
	char str[1280];
	double *coef;
	int adc, chan;
	TFile *fIn;

	int NF = sizeof(FileList) / sizeof(FileList[0]);
	int NT = (FileList[NF-1].end - FileList[0].begin) / 50 + 1;

	if (argc<3) {
		printf("Usage: %s adc chan\n", argv[0]);
		return 100;
	}
	adc = strtol(argv[1], NULL, 10);
	chan = strtol(argv[2], NULL, 10);

	sprintf(str, "%s/relcoef_%d_%2.2d.bin", PHEDIR, adc, chan);
	fOut = fopen(str, "wb");
	if (!fOut) {
		printf("Can not open the output file %s - %m\n", str);
		return 10;
	}


	coef = (double *) malloc(NT * sizeof(double));
	if (!coef) {
		printf("No memory !\n");
		return 60;
	}

	i = adc;
	j = chan;
	sum = 0;
	cnt = 0;
	memset(coef, 0, NT * sizeof(double));
	for (k=0; k<NF; k++) {
		sprintf(str, "%s/%s.root", PHEDIR, FileList[k].name);
//		printf("%s\n", str);
		fIn = new TFile(str);
		if (!fIn->IsOpen()) return 20;
		N = (FileList[k].end - FileList[k].begin) / 50 + 1;
		for (m=0; m<N; m++) {
			sprintf(str, "hCoef_%6.6d_%d_%2.2d", FileList[k].begin + m * 50, i, j);
			h = (TH1D *) fIn->Get(str);
			if (!h) continue;
			val = h->GetMean();
			delete h;
			if (val < 10) continue;			// invalid
			ind = (FileList[k].begin - FileList[0].begin) / 50 + m;
//			if (k==0) printf("%s: %f[%d]\n", str, val, ind);
			coef[ind] = val;
			sum += val;
			cnt++;
		}
		delete fIn;
	}
	if (cnt) {
		sum /= cnt;
		if (sum > 40) {
			for (k=0; k<NT; k++) {
				if (!coef[k]) {
					coef[k] = -1000;		// invalid
				} else {
					coef[k] = (coef[k] - sum) / sum;
				}
			}
			head.adc = i;
			head.chan = j;
			head.num = NT;
			head.run_begin = FileList[0].begin;
			head.run_end = FileList[NF-1].end;
			head.run_step = 50;
			fwrite(&head, sizeof(head), 1, fOut);
			fwrite(coef, NT * sizeof(double), 1, fOut);
			printf("%d.%2.2d: %d points\n", i, j, NT);
		}
	}
	fclose(fOut);
	
	return 0;
}
