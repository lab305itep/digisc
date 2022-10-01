/****************************************************************
 *  Take graph with absolute coefficients and make relative	*
 ****************************************************************/
#include <stdio.h>

#include <TFile.h>
#include <TGraph.h>
#include <TString.h>

#define MAXADC	52
#define MAXCHAN	64

int main(int argc, char **argv)
{
	int i, j, k, N;
	double sum;
	TGraph *gIn;
	TGraph *gOut;
	char strs[128];

	if (argc < 2) {
		printf("Usage: %s fname\n", argv[0]);
		return 10;
	}
	TFile *fIn = new TFile(argv[1]);
	if (!fIn->IsOpen()) return 20;
	
	TString fname(argv[1]);
	fname.ReplaceAll(".root", "_relcoef.root");
	TFile *fOut = new TFile(fname.Data(), "RECREATE");
	if (!fOut->IsOpen()) return 30;

	for (i=0; i<MAXADC; i++) for (j=0; j<MAXCHAN; j++) {
		sprintf(strs, "gCoef_%d_%2.2d", i, j);
		gIn = (TGraph *) fIn->Get(strs);
		if (!gIn) continue;
		N = gIn->GetN();
		if (!N) continue;
		sum = 0;
		for (k=0; k<N; k++) sum += gIn->GetPointY(k);
		sum /= N;
		if (sum <= 40) continue;
		gOut = new TGraph(N);
		sprintf(strs, "gRCoef_%d_%2.2d", i, j);
		gOut->SetName(strs);
		gOut->SetTitle(gIn->GetTitle());
		for (k=0; k<N; k++) 
			gOut->SetPoint(k, gIn->GetPointX(k), (gIn->GetPointY(k) - sum) / sum);
		fOut->cd();
		gOut->Write();
		delete gOut;
	}
	fOut->Close();
	fIn->Close();
	
	return 0;
}

