#include <stdio.h>
#include <TChain.h>
#include <TCut.h>
#include <TFile.h>
#include <TH1.h>
#include <TROOT.h>

/****************************************************************
 *	Scan over scale with neutrons from IBD - MC variation	*
 ****************************************************************/

#define NHISTS 21

TH1D *mc_hist(TChain *chain, const char *suffix, const char *what, TCut ct, double scale, double RndmSqe, double RndmC)
{
	char strs[128];
	char strl[1024];
	
	sprintf(strs, "hGdn%s_S%5.3f_R%5.3f_C%5.3f", suffix, scale, RndmSqe, RndmC);
	sprintf(strl, "Neutron capture energy %s MC * %5.3f rndm=%5.3f/#sqrt{E} #oplus %5.3f;MeV", suffix, scale, RndmSqe, RndmC);
	TH1D *h = new TH1D(strs, strl, 105, 1.5, 12.0);
	sprintf(strl, "MyRandom::GausAdd(%5.3f*%s, %6.4f, %6.4f)", scale, what, RndmSqe, RndmC);
	chain->Project(h->GetName(), strl, ct);
	return h;
}

void background_MC(TChain *chain, const char *fname, TCut cAux = (TCut) "")
{
	char strs[128];
	char strl[1024];
	TH1D *h[4][NHISTS];
	TH1D *hSiPM[4][NHISTS];
	TH1D *hPMT[4][NHISTS];
	int i, j;
	double scale, RndmSqe, RndmC;
//		Main cuts
	TCut cVeto("gtFromVeto > 90");
	TCut cIso("(gtFromPrevious > 50 && gtToNext > 80 && EventsBetween == 0)");
	TCut cShower("gtFromShower > 120 || ShowerEnergy < 800");
	TCut cX("PositronX[0] > 28 && PositronX[0] < 68");		// 30 cm from all edges
	TCut cY("PositronX[1] > 28 && PositronX[1] < 68");
	TCut cZ("PositronX[2] > 29.5 && PositronX[2] < 69.5");
	TCut c20("gtDiff > 1");
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
	TCut cPe("PositronEnergy > 3.0");	// large enough
	TCut cPh("PositronHits < 8");
	TCut cR("Distance < 40 && NeutronX[0] > 0 && NeutronX[1] > 0");		// both coordinates in neutron
        TCut ct;

	TFile *fRoot = new TFile(fname, "RECREATE");
	ct = cVeto && cIso && cShower && cX && cY && cZ && c20 && cGamma && cPe && cPh && cR && cAux;
	for (i=0; i<NHISTS; i++) {
		scale = 0.9 + 0.01 * i;
		RndmSqe = 0.02 + 0.01 * i;
		RndmC = 0.005 * i;
		
		h[0][i] = mc_hist(chain, "", "NeutronEnergy", ct, scale, 0.12, 0.04);
		hSiPM[0][i] = mc_hist(chain, "SiPM", "SiPmCleanEnergy[1]", ct, scale, 0.12, 0.04);
		hPMT[0][i] = mc_hist(chain, "PMT", "PmtCleanEnergy[1]", ct, scale, 0.12, 0.04);
		h[1][i] = mc_hist(chain, "", "NeutronEnergy", ct, 1.0, RndmSqe, 0.04);
		hSiPM[1][i] = mc_hist(chain, "SiPM", "SiPmCleanEnergy[1]", ct, 1.0, RndmSqe, 0.04);
		hPMT[1][i] = mc_hist(chain, "PMT", "PmtCleanEnergy[1]", ct, 1.0, RndmSqe, 0.04);
		h[2][i] = mc_hist(chain, "", "NeutronEnergy", ct, 1.0, 0.12, RndmC);
		hSiPM[2][i] = mc_hist(chain, "SiPM", "SiPmCleanEnergy[1]", ct, 1.0, 0.12, RndmC);
		hPMT[2][i] = mc_hist(chain, "PMT", "PmtCleanEnergy[1]", ct, 1.0, 0.12, RndmC);
		h[3][i] = mc_hist(chain, "", "NeutronEnergy", ct, scale, 0.0, 0.0);
		hSiPM[3][i] = mc_hist(chain, "SiPM", "SiPmCleanEnergy[1]", ct, scale, 0.0, 0.0);
		hPMT[3][i] = mc_hist(chain, "PMT", "PmtCleanEnergy[1]", ct, scale, 0.0, 0.0);
	}
	fRoot->cd();
	for (j=0; j<4; j++) for (i=0; i<NHISTS; i++) {
		h[j][i]->Write();
		hSiPM[j][i]->Write();
		hPMT[j][i]->Write();
	}
	fRoot->Close();
}

int main(int argc, char **argv)
{
	const char * auxstr;
	char str[1024];
	TChain *chain;
	FILE *fIn;

	gROOT->LoadMacro("MyRandom.C");

	chain = new TChain("DanssPair");

	if (argc < 3) {
		printf("Usage: %s mc_file_list.txt output_file.root\n", argv[0]);
		return 100;
	}
	fIn = fopen(argv[1], "rt");
	if (!fIn) {
		printf("Can not open file %s : %m\n", argv[1]);
		return 10;
	}
	for(;;) {
		if (!fgets(str, sizeof(str), fIn)) break;
		if (strlen(str) < 2) continue;
		str[strlen(str)-1] = '\0';
		chain->AddFile(str);
	}
	fclose(fIn);
	if (!chain->GetEntries()) {
		printf("No events found !\n");
		return 20;
	}
	
	auxstr = getenv("AUX_CUT");
	if (!auxstr) auxstr = "";
	
	background_MC(chain, argv[2], (TCut) auxstr);
	return 0;
}
