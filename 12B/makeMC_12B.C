#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <TFile.h>
#include <TH1D.h>
#include <TRandom2.h>
#include <TTree.h>

TRandom2 rnd;

class MyRandom {
    public:
	inline MyRandom(void) {;};
	inline ~MyRandom(void) {;};
	static inline double Gaus(double mean = 0, double sigma = 1) {
		return rnd.Gaus(mean, sigma);
	};
	static inline double GausAdd(double val, double sigma) {
		return rnd.Gaus(val, sqrt(val)*sigma);
	};
	static inline double GausAdd2(double val, double sigma) {
		return rnd.Gaus(val, val*sigma);
	};
};

void makeMC_12B(const char *mcname, double rndm = 0)
{
	char str[1024];

	TFile *fMC = new TFile(mcname);
	if (!fMC->IsOpen()) return;
	TTree *tMC = (TTree *) fMC->Get("DanssEvent");
	
	sprintf(str, "MC_12B_rndm_%4.2f.root", rndm);
	TFile *fOut = new TFile(str, "RECREATE");

	sprintf(str, "MC for ^{12}B decay, PositronEnergy + Random (%6.4f/#sqrt{E});MeV", rndm);
	TH1D *hMC = new TH1D("hMC12B", str, 80, 0, 20);
	
	sprintf(str, "MyRandom::GausAdd(PositronEnergy, %6.4f)", rndm);
	tMC->Project(hMC->GetName(), str);
	
	hMC->Sumw2();
	
	fOut->cd();
	hMC->Write();
	fOut->Close();
}
