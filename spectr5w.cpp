#include <stdio.h>
#include <stdlib.h>

#include <TCut.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>

#include "HPainter2.h"

#define NEBINS 128
const char *NeutronCorrN = "0.004800-0.000225*x";
const char *NeutronCorrC = "0.01773-0.000792*x";

void make_cuts(TCut &cSig, TCut &cBgnd, TCut cAux)
{
//		Set cuts
	TCut cVeto("gtFromVeto > 60");
	TCut cMuonA("gtFromVeto == 0");
	TCut cMuonB("gtFromVeto > 0 && gtFromVeto <= 60");
	TCut cIso("((gtFromPrevious > 45  || gtFromPrevious == gtFromVeto) && gtToNext > 80 && EventsBetween == 0)");
	TCut cShower("gtFromShower > 200 || ShowerEnergy < 800");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut c20("gtDiff > 2");
        TCut cGamma("AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10");
	TCut cGammaMax("AnnihilationMax < 0.8");
        TCut cPe("PositronEnergy > 1");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
        TCut cR1("Distance < 45");
        TCut cR2("Distance < 55");
        TCut cR = cR2 && (cRXY || cR1);
        TCut cN("NeutronEnergy > 3.5 && NeutronEnergy < 15.0 && NeutronHits >= 3");
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))");
	TCut cSel = cX && cY && cZ && cR && c20 && cGamma && cGammaMax && cN && cPe && cIso && cShower && cSingle && cAux;
	cSig = cSel && cVeto;
	cBgnd = cSel && (!cVeto);
}

void spectr5(const char *name, int mask, int run_from, int run_to, double bgnd)
{
	char str[256];
	TCut cSig;
	TCut cBgnd;
	TCut cAux;
	int nSect;
	double bgScale;
	char *ptr;
	char fname[1024];
	const char *pairdir = "/home/clusters/rrcmpi/alekseev/igor/pair7n/";
	const char *outdir  = "/home/clusters/rrcmpi/alekseev/igor/period7n/";
	
//	Environment
	nSect = 1;
	ptr = getenv("SPECTR_NSECT");
	if (ptr) nSect = strtol(ptr, NULL, 10);
	
	bgScale = 2.24;
	ptr = getenv("SPECTR_BGSCALE");
	if (ptr) bgScale = strtod(ptr, NULL);
	
	cAux = (TCut)"";
	ptr = getenv("SPECTR_AUXCUT");
	if (ptr) cAux = (TCut)ptr;

	ptr = getenv("PAIR_DIR");
	if (ptr) pairdir = ptr;

	ptr = getenv("OUT_DIR");
	if (ptr) outdir = ptr;
	
	sprintf(fname, "%s/%s.root", outdir, name);
	TFile *fRoot = new TFile (fname, "RECREATE");
	make_cuts(cSig, cBgnd, cAux);
//		Background tail correction (mHz)
	TF1 fBgndN("fBgndN", NeutronCorrN, 0, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0, 100);

	TH1D *hSig  = new TH1D("hSig",  "Positron spectrum;MeV;mHz/125 keV", NEBINS, 0, 16);
	TH1D *hBgnd = new TH1D("hCosm", "Positron spectrum;MeV;mHz/125 keV", NEBINS, 0, 16);
	TH1D *hRes  = new TH1D("hRes",  "Positron spectrum;MeV;mHz/125 keV", NEBINS, 0, 16);
	TH1D *hConst = new TH1D("hConst", "Various period parameters", 10, 0, 10);
	
	HPainter2 *ptr2 = new HPainter2(mask, run_from, run_to, pairdir);
	ptr2->SetFile(fRoot);
	ptr2->Project(hSig,  "PositronEnergy", cSig);
	ptr2->Project(hBgnd, "PositronEnergy", cBgnd);
	hConst->Fill("UpTime", ptr2->GetUpTime());

	hSig->Add(&fBgndN, -1.0/nSect);
	hBgnd->Add(&fBgndC, -1.0/nSect);
	hRes->Add(hSig, hBgnd, 1, -bgnd * bgScale);
	fRoot->cd();
	hSig->Write();
	hBgnd->Write();
	hRes->Write();
	hConst->Write();
	fRoot->Close();
	delete ptr2;
}

int main(int argc, char **argv)
// void spectr_all(int nSect, const char *fname = "danss_report_v4.root", TCut cAux = (TCut)"", double bgScale = 2.24)	// 5.6% from reactor OFF data
{
#include "positions.h"
	const int mask = 0x801E;
	int i;
	int N;
	

	if (argc < 2) {
		printf("Usage: %s period_number (from 1)\n", argv[0]);
		return 100;
	}
	i = strtol(argv[1], NULL, 10);
	N = sizeof(positions) / sizeof(positions[0]);
	if (i<1 || i > N) {
		printf("Illegal period number %d (max = %d)\n", i, N);
		return 110;
	}
	
	spectr5(positions[i-1].name, mask, positions[i-1].first, positions[i-1].last, positions[i-1].bgnd);
	return 0;
}
