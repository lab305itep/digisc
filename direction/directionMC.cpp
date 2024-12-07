#include <stdio.h>
#include <stdlib.h>

#include <TCut.h>
#include <TFile.h>
#include <TH1.h>
#include <TTree.h>

void direction(const char *name, int num, TCut cAux)
{
	char str[256];
	TCut cSig;
	TCut cBgnd;
	TCut Cuts;
	double bgnd;
	char *ptr;
	char fname[1024];
	const char *pairdir = "/home/clusters/rrcmpi/alekseev/igor/pair8n2/MC/Eff_ind_v0";
	const char *outdir = "./";
	int i;
	
//	Environment
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

	sprintf(fname, "%s/var_%d_fuel.root", pairdir, num);
	TFile *fIn = new TFile(fname);
	if (!fIn->IsOpen()) return;
	TTree *tEvent = (TTree *) fIn->Get("DanssPair");
	if (!tEvent) {
		printf("No DanssPair tree in %s\n", fname);
		return;
	}

	sprintf(fname, "%s/%s_%d.root", outdir, name, num);
	TFile *fRoot = new TFile (fname, "RECREATE");

	TH1D *hResX  = new TH1D("hResX",  "Shift X;cm;Events", 16, -32, 32);		// signal - background events
	TH1D *hResY  = new TH1D("hResY",  "Shift Y;cm;Events", 16, -32, 32);
	TH1D *hResZ  = new TH1D("hResZ",  "Shift Z;cm;Events", 64, -32, 32);
	

	tEvent->Project(hResX->GetName(),  "NeutronX[0] - PositronX[0]", cSig);
	printf("Projection X done.\n");
	tEvent->Project(hResY->GetName(),  "NeutronX[1] - PositronX[1]", cSig);
	printf("Projection Y done.\n");
	tEvent->Project(hResZ->GetName(),  "NeutronX[2] - PositronX[2]", cSig);
	printf("Projection Z done.\n");
//	Save files
	fRoot->cd();
	hResX->Write();
	hResY->Write();
	hResZ->Write();
	fRoot->Close();
}

int main(int argc, char **argv)
{
	int num;
	const char *name = "MC";
	const char *cut = "";
	
	if (argc < 2) {
		printf("Usage: %s number [name [aux cut]]\n", argv[0]);
		return 100;
	}
	num = strtol(argv[1], NULL, 10);
	if (argc > 2) name = argv[2];
	if (argc > 3) cut = argv[3];

	direction(name, num, TCut(cut));
	return 0;
}
