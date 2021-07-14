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
	static inline double Gaus(double mean = 0, double sigma = 1) 
	{
		return rnd.Gaus(mean, sigma);
	};
	static inline double GausAdd(double val, double sigma = 0, double csigma = 0) 
	{
		return rnd.Gaus(val, sqrt(val*sigma*sigma + val*val*csigma*csigma));
	};
};


void makeMC_12B(const char *mcname, double scale)
{
	const double RndmSqe = 0.12;
	const double RndmC = 0.04;
	const double SiPMRndmSqe = 0.12;
	const double SiPMRndmC = 0.125;
	const double PMTRndmSqe = 0.12;
	const double PMTRndmC = 0.06;
	char str[1024];

	TFile *fMC = new TFile(mcname);
	if (!fMC->IsOpen()) return;
	TTree *tMC = (TTree *) fMC->Get("DanssEvent");
	
	sprintf(str, "MC_12B_v8.1_%5.3f.root", scale);
	TFile *fOut = new TFile(str, "RECREATE");
	
	TH1D *hMC = new TH1D("hMC12B", "MC of ^{12}B decay, SiPM+PMT;MeV", 80, 0, 20);
	TH1D *hMCSiPM = new TH1D("hMC12BSiPM", "MC of ^{12}B decay, SiPM;MeV", 80, 0, 20);
	TH1D *hMCPMT = new TH1D("hMC12BPMT", "MC of ^{12}B decay, PMT;MeV", 80, 0, 20);
	TH1D *hMCT = new TH1D("hMC12BT", "Time from muon, MC;ms", 99, 1, 100);
	
	TCut mccut("TimelineShift > 500000 && AnnihilationEnergy*1.08 < 0.25 && PositronEnergy*1.04 > 3.0");	// UGLY !

	sprintf(str, "MyRandom::GausAdd(PositronEnergy*1.04*%5.3f, %6.4f, %6.4f)", scale, RndmSqe, RndmC);		// UGLY !
	tMC->Project(hMC->GetName(), str, mccut);
	sprintf(str, "MyRandom::GausAdd(PositronSiPmEnergy*1.08*%5.3f, %6.4f, %6.4f)", scale, SiPMRndmSqe, SiPMRndmC);		// UGLY !
	tMC->Project(hMCSiPM->GetName(), str, mccut);				// UGLY !
	sprintf(str, "MyRandom::GausAdd(PositronPmtEnergy*%5.3f, %6.4f, %6.4f)", scale, PMTRndmSqe, PMTRndmC);		// UGLY !
	tMC->Project(hMCPMT->GetName(), str, mccut);
	tMC->Project(hMCT->GetName(), "TimelineShift / 1000000.0", mccut);

	hMC->Sumw2();
	hMCSiPM->Sumw2();
	hMCPMT->Sumw2();
	hMCT->Sumw2();
	
	fOut->cd();
	hMC->Write();
	hMCSiPM->Write();
	hMCPMT->Write();
	hMCT->Write();
	fOut->Close();
}
