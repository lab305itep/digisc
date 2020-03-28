#include <stdio.h>

#include <TCut.h>
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
	static inline double GausAdd(double val, double sigma = 0, double csigma = 0) {
		return rnd.Gaus(val, sqrt(val*sigma*sigma + val*val*csigma*csigma));
	};
};

void mcfuelspectra(const char *pairname, const char *spname, const double kRndm = 0.12, const double kRndmC = 0.04)
{
	const double kScale = 1.0;
	char str[1024];
	
	TCut cVeto("gtFromVeto > 90");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cT("gtDiff > 2");
	TCut cA("AnnihilationEnergy < 1.8 && AnnihilationGammas < 9 && AnnihilationMax < 0.8");
	TCut cPE("PositronEnergy > 0.75 && PositronHits < 7");
	TCut cR("((PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || Distance < 45) && Distance < 55");
	TCut cN("NeutronEnergy > 3.5 && NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
	TCut cOneHit("!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))");
	TCut cSel = cVeto && cX && cY && cZ && cA && cR && cN && cOneHit;
	
	TFile *fMC = new TFile(pairname);
	TTree *tMC = (TTree *) fMC->Get("DanssPair");
	if (!tMC) {
		printf("Tree DanssPair not found in %s\n", pairname);
		return;
	}
	
	TFile *fSp = new TFile(spname, "RECREATE");
	TH1D *hMC = new TH1D("hMC", "Fuel mixture MC event positron spectra;E_{e^{+}}, MeV;Events/day/125 keV", 128, 0, 16);
	sprintf(str, "%6.3f*MyRandom::GausAdd(PositronEnergy, %8.5f, %8.5f)", kScale, kRndm, kRndmC);
	tMC->Project(hMC->GetName(), str, cSel);
	fSp->cd();
	hMC->Write();
	fSp->Close();
	fMC->Close();
}
