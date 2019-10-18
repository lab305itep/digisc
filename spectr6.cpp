#include <stdio.h>
#include <stdlib.h>

#include <TCut.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>

#include "HPainter2.h"

#define NEBINS 128
const char *NeutronCorrN = "0.003475-0.000152*x";
const char *NeutronCorrC = "0.01138-0.000437*x";
//const char *NeutronCorrN = "0.004800-0.000225*x";
//const char *NeutronCorrC = "0.01773-0.000792*x";

void spectr(const char *name, int mask, int run_from, int run_to, double bgnd)
{
	char str[256];
	TCut cSig;
	TCut cBgnd;
	TCut Cuts;
	double bgScale;
	char *ptr;
	char fname[1024];
	const char *pairdir = "/home/clusters/rrcmpi/alekseev/igor/pair7n/";
	const char *outdir  = "/home/clusters/rrcmpi/alekseev/igor/period7n/";
	const char *what = "PositronEnergy";
	const char *mucut = "gtFromVeto > 60";
	int i;
	
//	Environment
	bgScale = 2.64;
	ptr = getenv("SPECTR_BGSCALE");
	if (ptr) bgScale = strtod(ptr, NULL);
	
	ptr = getenv("PAIR_DIR");
	if (ptr) pairdir = ptr;

	ptr = getenv("OUT_DIR");
	if (ptr) outdir = ptr;

	ptr = getenv("SPECTR_WHAT");
	if (ptr) what = ptr;

	ptr = getenv("SPECTR_MUCUT");
	if (ptr) mucut =  ptr;

	Cuts = (TCut)"";
	for (i=0; i<100; i++) {
		sprintf(str, "SPECTR_CUT%2.2d", i);
		ptr = getenv(str);
		if (ptr) Cuts = Cuts && (TCut)ptr;
	}

	cSig = Cuts && (TCut) mucut;
	cBgnd = Cuts && !((TCut) mucut);

	sprintf(fname, "%s/%s.root", outdir, name);
	TFile *fRoot = new TFile (fname, "RECREATE");

//		Background tail correction (mHz)
	TF1 fBgndN("fBgndN", NeutronCorrN, 0, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0, 100);

	TH1D *hSig  = new TH1D("hSig",  "Positron spectrum;MeV;mHz/125 keV", NEBINS, 0, 16);
	TH1D *hBgnd = new TH1D("hCosm", "Positron spectrum;MeV;mHz/125 keV", NEBINS, 0, 16);
	TH1D *hRes  = new TH1D("hRes",  "Positron spectrum;MeV;mHz/125 keV", NEBINS, 0, 16);
	TH1D *hConst = new TH1D("hConst", "Various period parameters", 10, 0, 10);
	
	HPainter2 *ptr2 = new HPainter2(mask, run_from, run_to, pairdir);
	ptr2->SetFile(fRoot);
	ptr2->Project(hSig,  what, cSig);
	ptr2->Project(hBgnd, what, cBgnd);
	hConst->Fill("UpTime", ptr2->GetUpTime());

	hSig->Add(&fBgndN, -1.0);
	hBgnd->Add(&fBgndC, -1.0);
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
{
#include "positions.h"
	const int mask = 0x803E;
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
	
	spectr(positions[i-1].name, mask, positions[i-1].first, positions[i-1].last, positions[i-1].bgnd);
	return 0;
}
