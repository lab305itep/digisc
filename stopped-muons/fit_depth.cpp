//	Fit depth for each event using MC bragg curve
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TFile.h>
#include <TMinuit.h>
#include <TProfile.h>
#include <TSpline.h>
#include <TString.h>
#include <TTree.h>

#define MAXZ	100

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
#pragma pack(pop)

TSpline *Bragg;
struct StoppedMuonStruct Muon;

void FitFunction(int &npar, double *gin, double &f, double *x, int iFlag)
{
	double sum;
	int i;
	double scale;
	double L;
	double dedx;
	double diff;

	if (iFlag == 1) return;
	scale = sqrt(1 + tan(Muon.thetaX)*tan(Muon.thetaX) + tan(Muon.thetaY)*tan(Muon.thetaY));
	sum = 0;
	double d = x[0];
	for (i=0; i<Muon.NHits; i++) if (Muon.Ehit[i] > 0) {
		if (i) {
			L = (d + i - 0.5) * scale;
			dedx = Muon.Ehit[i] / scale;
		} else {
			L = 0.5 * d * scale;
			dedx = Muon.Ehit[i] / (d * scale);
		}
		diff = dedx - Bragg->Eval(L);
		sum += diff * diff;
	}
	printf("%f %f %f\n", scale, d, sum);
	f = sum;
}

float FitDepth(void)
{
	double d, ed;
	TMinuit *mn = new TMinuit();
	mn->mninit(5, 6, 7);
	mn->SetFCN(FitFunction);
        mn->DefineParameter(0, "Depth", 0.5, 0.005, 0.001, 1.1);
	mn->Migrad();
	mn->GetParameter(0, d, ed);
	delete mn;
	return d;
}

int main(int argc, char ** argv)
{
	float Depth;
	int i, N;
	
	if (argc < 3) {
		printf("Usage: %s prc_file bragg_file\n", argv[0]);
		return 10;
	}
	
	TFile *fIn = new TFile(argv[1]);
	TString fname(argv[1]);
	fname.ReplaceAll(".root", "");
	TFile *fBragg = new TFile(argv[2]);
	TFile *fOut = new TFile((fname + "-fdepth.root").Data(), "RECREATE");
	if (!fIn->IsOpen() || !fOut->IsOpen() || !fBragg->IsOpen()) return 20;
	
	TTree *tIn = (TTree *) fIn->Get("StoppedMuons");
	TProfile *hBragg = (TProfile *) fBragg->Get("hEL_profX");
	if (!tIn || !hBragg) {
		printf("Can not find all trees/histogramms\n");
		return 30;
	}
	Bragg = new TSpline3(hBragg);
	
	tIn->SetBranchAddress("Stopped", &Muon);
	TTree *tOut = new TTree("Depth", "Depth");
	tOut->Branch("Depth", &Depth, "Depth/F");
	
	N = tIn->GetEntries();
	N = 10;
	for (i=0; i<N; i++) {
		tIn->GetEntry(i);
		Depth = FitDepth();
		tOut->Fill();
	}
	
	fOut->cd();
	tOut->Write();
	fOut->Close();
	fIn->Close();
	fBragg->Close();
	return 0;
}
