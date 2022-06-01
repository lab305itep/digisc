#include <stdio.h>
#include <TCut.h>
#include <TFile.h>
#include <TH1.h>
#include <TTree.h>
#include "../HPainter2.h"

/****************************************
 *	Calculate various cuts		*
 ****************************************/

void background_calc(const char *name, int run_first, int run_last, TCut cAux = (TCut) "")
{
	char strs[128];
	char strl[1024];
	TH1D *hExp;
	TH1D *hExpSiPM;
	TH1D *hExpPMT;
	TH1D *hConst;
	int i, j;
	const char *ptr;
	const char *what;
//		Main cuts

//		Main cuts
	TCut cVeto("gtFromVeto > 90");
	TCut cIso("(gtFromPrevious > 50 && gtToNext > 80 && EventsBetween == 0)");
	TCut cShower("gtFromShower > 120 || ShowerEnergy < 800");
	TCut c20("gtDiff > 1");
	TCut cX("PositronX[0] > 28 && PositronX[0] < 68");		// 30 cm from all edges
	TCut cY("PositronX[1] > 28 && PositronX[1] < 68");
	TCut cZ("PositronX[2] > 29.5 && PositronX[2] < 69.5");
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
	TCut cPe("PositronEnergy > 3.0");	// large enough
	TCut cPh("PositronHits < 8");
	TCut cR("Distance < 40 && NeutronX[0] > 0 && NeutronX[1] > 0");		// both coordinates in neutron
        TCut ct = cIso && cVeto && cShower && c20 && cX && cY && cZ && cGamma && cPe && cPh && cR && cAux;

	sprintf(strl, "exp/%s.root", name);
	TFile *fRoot = new TFile(strl, "RECREATE");
	hExp = new TH1D("hn", "Delayed event energy;MeV;events", 105, 1.5, 12);
	hExpSiPM = new TH1D("hnSiPM", "Delayed event energy, SiPM;MeV;events", 105, 1.5, 12);
	hExpPMT = new TH1D("hnPMT", "Delayed event energy, PMT;MeV;events", 105, 1.5, 12);
	hConst = new TH1D("hConst", "Run constants", 10, 0, 10);
	hConst->GetXaxis()->SetBinLabel(1, "gTime");

	ptr = getenv("PAIR_DIR");
	if (!ptr) ptr = "/home/clusters/rrcmpi/alekseev/igor/pair8n2";

	HPainter2 *hp = new HPainter2(0x801E, run_first, run_last, ptr);
	if (hp->GetUpTime() < 10) {
		printf("%s:%d - %d: no runs !\n", ptr, run_first, run_last);
		fRoot->Close();
		return;
	}
	hp->SetFile(fRoot);
	hConst->SetBinContent(1, hp->GetUpTime());
	hp->Project(hExp, "NeutronEnergy", ct);
	hp->Project(hExpSiPM, "SiPmCleanEnergy[1]", ct);
	hp->Project(hExpPMT, "PmtCleanEnergy[1]", ct);

	fRoot->cd();
	hExp->Write();
	hExpSiPM->Write();
	hExpPMT->Write();
	hConst->Write();
	delete hp;
	fRoot->Close();
}

int main(int argc, char **argv)
{
#include "../positions.h"
	int i;
	int N;
	const char * auxstr;

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
	
	auxstr = getenv("AUX_CUT");
	if (!auxstr) auxstr = "";
	
	background_calc(positions[i-1].name, positions[i-1].first, positions[i-1].last, (TCut) auxstr);
	return 0;
}
