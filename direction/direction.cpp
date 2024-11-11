#include <stdio.h>
#include <stdlib.h>

#include <TCut.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>
#include <TROOT.h>

#include "../HPainter2.h"

void direction(const char *name, int run_from, int run_to, TCut cAux)
{
	const int mask = 0x803E;		// positions to be used - exclude rad. sources etc.
	char str[256];
	TCut cSig;
	TCut cBgnd;
	TCut Cuts;
	double bgnd;
	char *ptr;
	char fname[1024];
	const char *pairdir = "/home/clusters/rrcmpi/alekseev/igor/pair8n2/";
	const char *outdir = "./";
	int i;
	
//	Environment
	bgnd = 0.045;
	ptr = getenv("BGND_FRACT");
	if (ptr) bgnd = strtod(ptr, NULL);
	
	ptr = getenv("PAIR_DIR");
	if (ptr) pairdir = ptr;

	ptr = getenv("OUT_DIR");
	if (ptr) outdir = ptr;

//		Veto cuts
	TCut cVeto("gtFromVeto > 90");
	TCut cIso("((gtFromPrevious > 50  || gtFromPrevious == gtFromVeto) && gtToNext > 80 && EventsBetween == 0)");
	TCut cShower("gtFromShower > 120 || ShowerEnergy < 800");
//		minimum & maximum time cut
	TCut ct10("gtDiff > 1 && gtDiff < 50");
//		We require well defined IBD vertex to be 16 cm from the edge
	TCut cX("(PositronX[0] > 14 && PositronX[0] < 82)");
	TCut cY("(PositronX[0] > 14 && PositronX[0] < 82)");
	TCut cZ("(PositronX[0] > 15.5 && PositronX[0] < 83.5)");
//		Require well defined neutron vertex
	TCut cNXY("NeutronX[0] >= 0 && NeutronX[1] >= 0");
//		Cut for annihilation gammas
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
//		Cut for positron
	TCut cPos("PositronEnergy > 0.5 && PositronHits < 8");
//		Cut for neutron
	TCut cNH("NeutronEnergy > 1.5 && NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");

	Cuts = cIso && cShower && ct10 && cX && cY && cZ && cNXY && cGamma && cPos && cNH && cAux;

	cSig = Cuts && cVeto;
	cBgnd = Cuts && !cVeto;

	sprintf(fname, "%s/%s.root", outdir, name);
	TFile *fRoot = new TFile (fname, "RECREATE");

	TH1D *hSigX  = new TH1D("hSigX",  "Shift X;cm;Events", 16, -32, 32);		// signal events
	TH1D *hSigY  = new TH1D("hSigY",  "Shift Y;cm;Events", 16, -32, 32);
	TH1D *hSigZ  = new TH1D("hSigZ",  "Shift Z;cm;Events", 64, -32, 32);
	TH1D *hBgndX  = new TH1D("hBgndX",  "Shift X;cm;Events", 16, -32, 32);		// muon background
	TH1D *hBgndY  = new TH1D("hBgndY",  "Shift Y;cm;Events", 16, -32, 32);
	TH1D *hBgndZ  = new TH1D("hBgndZ",  "Shift Z;cm;Events", 64, -32, 32);
	TH1D *hRBgndX  = new TH1D("hRBgndX",  "Shift X;cm;Events", 16, -32, 32);		// ACB muon background
	TH1D *hRBgndY  = new TH1D("hRBgndY",  "Shift Y;cm;Events", 16, -32, 32);
	TH1D *hRBgndZ  = new TH1D("hRBgndZ",  "Shift Z;cm;Events", 64, -32, 32);
	TH1D *hResX  = new TH1D("hResX",  "Shift X;cm;Events", 16, -32, 32);		// signal - background events
	TH1D *hResY  = new TH1D("hResY",  "Shift Y;cm;Events", 16, -32, 32);
	TH1D *hResZ  = new TH1D("hResZ",  "Shift Z;cm;Events", 64, -32, 32);
	
	HPainter2 *ptr2 = new HPainter2(mask, run_from, run_to, pairdir);
	printf("Running for runs %d-%d with up time %10.0f seconds\n", run_from, run_to, ptr2->GetUpTime());
	if (ptr2->GetUpTime() < 1) goto fin;

	ptr2->Project(hSigX,  "NeutronX[0] - PositronX[0]", cSig, 0);
	ptr2->Project(hBgndX, "NeutronX[0] - PositronX[0]", cBgnd, 0);
	printf("Projection X done.\n");
	hRBgndX = (TH1D*) gROOT->FindObject("hBgndX-murand");
	if (!hBgndX) {
		printf("hBgndX-murand not found\n");
		goto fin;
	}
	ptr2->Project(hSigY,  "NeutronX[1] - PositronX[1]", cSig, 0);
	ptr2->Project(hBgndY, "NeutronX[1] - PositronX[1]", cBgnd, 0);
	printf("Projection Y done.\n");
	hRBgndY = (TH1D*) gROOT->FindObject("hBgndY-murand");
	if (!hRBgndY) {
		printf("hBgndY-murand not found\n");
		goto fin;
	}
	ptr2->Project(hSigZ,  "NeutronX[2] - PositronX[2]", cSig, 0);
	ptr2->Project(hBgndZ, "NeutronX[2] - PositronX[2]", cBgnd, 0);
	printf("Projection Z done.\n");
	hRBgndZ = (TH1D*) gROOT->FindObject("hBgndZ-murand");
	if (!hRBgndZ) {
		printf("hBgndZ-murand not found\n");
		goto fin;
	}	
//	Undo scale
	hSigX->Scale(ptr2->GetUpTime() / 1000);
	hSigY->Scale(ptr2->GetUpTime() / 1000);
	hSigZ->Scale(ptr2->GetUpTime() / 1000);
	hBgndX->Scale(ptr2->GetUpTime() / 1000);
	hBgndY->Scale(ptr2->GetUpTime() / 1000);
	hBgndZ->Scale(ptr2->GetUpTime() / 1000);
//	correct for ACB muon bgnd
	hBgndX->Add(hRBgndX, -1);
	hBgndY->Add(hRBgndY, -1);
	hBgndZ->Add(hRBgndZ, -1);
//	Subtract muon background
	hResX->Add(hSigX, hBgndX, 1, -bgnd);
	hResY->Add(hSigY, hBgndY, 1, -bgnd);
	hResZ->Add(hSigZ, hBgndZ, 1, -bgnd);
//	Save files
fin:
	fRoot->cd();
	hSigX->Write();
	hBgndX->Write();
	hResX->Write();
	hSigY->Write();
	hBgndY->Write();
	hResY->Write();
	hSigZ->Write();
	hBgndZ->Write();
	hResZ->Write();
	fRoot->Close();
	delete ptr2;
}

int main(int argc, char **argv)
{
	int iFirst, iLast;
	char str[1024];
	const char *name = "dir";
	const char *cut = "";
	
	if (argc < 3) {
		printf("Usage: %s run_first run_last [name [aux cut]]\n", argv[0]);
		return 100;
	}
	iFirst = strtol(argv[1], NULL, 10);
	iLast = strtol(argv[2], NULL, 10);
	if (argc > 3) name = argv[3];
	if (argc > 4) cut = argv[4];
	sprintf(str,"%s_%6.6d_%6.6d", name, iFirst, iLast);

	direction(str, iFirst, iLast, TCut(cut));
	return 0;
}
