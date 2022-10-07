/****************************************************************
 *  Get absolute coefs from histograms, calculate relative 	*
 *  and save in binary file.					*
 ****************************************************************/
#include <stdio.h>

#include <TFile.h>
#include <TH1D.h>
#include <TString.h>

#define MAXADC	55
#define MAXCHAN	64

int main(int argc, char **argv)
{
	int run_begin, run_end, run_step;
	int i, j, k, N, nsum;
	double sum, val;
	TH1D *hIn;
	char strs[128];
	double *coef;

	if (argc < 2) {
		printf("Usage: %s fname run_begin run_end run_step\n", argv[0]);
		return 10;
	}
	TFile *fIn = new TFile(argv[1]);
	if (!fIn->IsOpen()) return 20;
	run_begin = strtol(argv[2], NULL, 10);
	run_end = strtol(argv[3], NULL, 10);
	run_step = strtol(argv[4], NULL, 10);
	N = (run_end - run_begin) / run_step + 1;
	coef = (double *) malloc(N * sizeof(double));
	if (!coef) {
		printf("No memory for %d bytes - %m\n", N * sizeof(double));
		return 40;
	}
	
	TString fname(argv[1]);
	fname.ReplaceAll(".root", "_relcoef.bin");
	FILE *fOut = fopen(fname.Data(), "wb");
	if (!fOut) {
		printf("Can not open the output file %s\n", fname.Data());
		return 30;
	}

	for (i=0; i<MAXADC; i++) for (j=0; j<MAXCHAN; j++) {
		printf("Begin %d.%2.2d\n", i, j);
		sum = 0;
		nsum = 0;
		memset(coef, 0, N * sizeof(double));
		for (k=0; k<N; k++) {
			sprintf(strs, "hCoef_%6.6d_%d_%2.2d", run_begin + k * run_step, i, j);
			hIn = (TH1D *) fIn->Get(strs);
			if (!hIn) continue;
			val = hIn->GetMean();
			if (val < 10) continue;			// invalid
			coef[k] = val;
			sum += val;
			nsum++;
		}
		if (!nsum) continue;
		sum /= nsum;
		for (k=0; k <N; k ++) {
			if (!coef[k]) {
				coef[k] = -1000;		// invalid
			} else {
				coef[k] = (coef[k] - sum) / sum;
			}
		}
		fwrite(coef, N * sizeof(double), 1, fOut);
		printf("End %d.%2.2d\n", i, j);
	}
	fclose(fOut);
	fIn->Close();
	
	return 0;
}

