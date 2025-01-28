//	Fit depth for each event using MC bragg curves
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TF1.h>
#include <TFile.h>
#include <TH1D.h>
#include <TMinuit.h>
#include <TString.h>
#include <TTree.h>

#define MAXZ	100
#define DEPTHBINS	11	// 0.0-1.1 mm
#define SCALEBINS	30	// 1.0-4.0
#define MAXLAYERS	25

#pragma pack(push,1)
struct StoppedMuonStruct {
        int index;              // Index in the DanssEvent tree
        long long globalTime;   // global time 125 MHz ticks
        int Z;                  // z of the stopping strip
        int XY;                 // xy of the stopping strip
        float thetaX;           // angle of the track in ZX, 0 - vertical
        float thetaY;           // angle of the track in ZY, 0 - vertical
        int NHits;              // number of hits including empty hits
        float Ehit[MAXZ];       // hit energy
};
struct DepthFitStruct {		// Fit results
	float depth;
	float escale;
};
#pragma pack(pop)

struct fit_one_struct {
	TH1D *hMCdE;
	struct StoppedMuonStruct Muon;
} FitOneData;

TH1D *hMCdE[SCALEBINS][DEPTHBINS];

void fit_one_fun(int &Npar, double *gin, double &f, double *x, int iflag)
{
	double d, sum;
	int i;
	
	if (iflag == 1) return;
	
	sum = 0;
	for (i=0; i<FitOneData.Muon.NHits && i<MAXLAYERS; i++) if (FitOneData.Muon.Ehit[i] > 0) {
		d = FitOneData.Muon.Ehit[i] - x[0] * FitOneData.hMCdE->GetBinContent(i+1);
		sum += d * d;
	}
	f = sum;
}

double FitOneFit(TH1D *hMC, double &escale)
{
	double kE, ekE;
	int n;
	double f;
	
	FitOneData.hMCdE = hMC;
	TMinuit *mn = new TMinuit(2);
	mn->mninit(5, 6, 7);
	mn->Command("set print -1");
	mn->SetFCN(fit_one_fun);
	mn->DefineParameter(0, "Escale", 1.0, 0.05, 0.1, 10.);	// Escale
	mn->Migrad();
	mn->GetParameter(0, kE, ekE);
	n = 1;
	fit_one_fun(n, NULL, f, &kE, 0);
	
	delete mn;
	escale = kE;
	return f;
}

//	Try to fit depth
void fit_one(DepthFitStruct *DepthFit)
{
	double scale;
	int i, j, m;
	char str[256];
	double chi2[DEPTHBINS];
	double escale[DEPTHBINS];
	double d;
	
	scale = sqrt(1 + tan(FitOneData.Muon.thetaX)*tan(FitOneData.Muon.thetaX) + 
		tan(FitOneData.Muon.thetaY)*tan(FitOneData.Muon.thetaY));
	m = (scale - 1.0) / 0.1;
	if (m < 0) m = 0;
	if (m >= SCALEBINS) m = SCALEBINS - 1;

	for (i=0; i<DEPTHBINS; i++) chi2[i] = FitOneFit(hMCdE[m][i], escale[i]);
	
	TH1D h("_h", "S^{2}", 11, 0, 1.1);
	for (i=0; i<DEPTHBINS; i++) h.SetBinContent(i+1, chi2[i]);
	TF1 fpar("fPar", "pol2");
	h.Fit(fpar.GetName(), "Q0", "");

	d = - 0.5 * fpar.GetParameter(1) / fpar.GetParameter(2);
	if (d < 0) d = 0;
	if (d > 1.1) d = 1.1;
	j = (d+0.05) / 0.1;
	if (j >= DEPTHBINS) j = DEPTHBINS-1;
	DepthFit->depth = d;
	DepthFit->escale = escale[j];
}

int main(int argc, char ** argv)
{
	int i, j, N;
	DepthFitStruct DepthFit;
	char str[1024];
	
	if (argc < 3) {
		printf("Usage: %s prc_file MC_hist_file\n", argv[0]);
		return 10;
	}
	
	TFile *fIn = new TFile(argv[1]);
	TString fname(argv[1]);
	fname.ReplaceAll(".root", "");
	TFile *fMC = new TFile(argv[2]);
	TFile *fOut = new TFile((fname + "-fdepth.root").Data(), "RECREATE");
	if (!fIn->IsOpen() || !fOut->IsOpen() || !fMC->IsOpen()) return 20;
	
	TTree *tIn = (TTree *) fIn->Get("StoppedMuons");
	if (!tIn) {
		printf("Can not find input tree\n");
		return 30;
	}
	tIn->SetBranchAddress("Stopped", &FitOneData.Muon);

	for (i=0; i<DEPTHBINS; i++) for (j=0; j<SCALEBINS; j++) {
		sprintf(str, "hdEprof_s%2.2dd%d", j, i);
		hMCdE[j][i] = (TH1D*) fMC->Get(str);
		if (!hMCdE[j][i]) {
			printf("No hist %s\n", str);
			return 40;
		}
	}

	TTree *tOut = new TTree("Depth", "Depth");
	tOut->Branch("Depth", &DepthFit, "Depth/F:EScale/F");
	
	N = tIn->GetEntries();
	for (i=0; i<N; i++) {
		tIn->GetEntry(i);
		fit_one(&DepthFit);
		tOut->Fill();
	}
	
	fOut->cd();
	tOut->Write();
	fOut->Close();
	fIn->Close();
	fMC->Close();
	return 0;
}
