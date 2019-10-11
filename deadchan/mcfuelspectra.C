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
	static inline double GausAdd(double val, double sigma)
	{
		return rnd.Gaus(val, sqrt(val)*sigma);
	};
};

void mcfuelspectra(const char *pfile, const char *hfile)
{
	const double kScale = 1.0;
	const double kRndm = 0.17;
	char str[1024];
	
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cT("gtDiff > 2");
	TCut cA("AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10 && AnnihilationMax < 0.8");
	TCut cPE("PositronEnergy > 1");
	TCut cR("((PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || Distance < 45) && Distance < 55");
	TCut cN("NeutronEnergy > 3.5 && NeutronEnergy < 15.0 && NeutronHits >= 3");
	TCut cOneHit("!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))");
	TCut cSel = cX && cY && cZ && cA && cR && cN && cOneHit;
	
	TFile *fMC = new TFile(pfile);
	TTree *tMC = (TTree *) fMC->Get("DanssPair");
	if (!tMC) {
		printf("Tree DanssPair not found in %s\n", pfile);
		return 20;
	}
	
	TFile *fSp = new TFile(hfile, "RECREATE");
	TH1D *hMC = new TH1D("hMC", "Fuel mixture MC event positron spectra;E_{e^{+}}, MeV;Events/day/125 keV", 128, 0, 16);
	sprintf(str, "%6.3f*MyRandom::GausAdd(PositronEnergy, %8.5f)", kScale, kRndm);
	tMC->Project(hMC->GetName(), str, cSel);
	fSp->cd();
	hMC->Write();
	fSp->Close();
	fMC->Close();
}
