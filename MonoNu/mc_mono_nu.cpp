#include <stdio.h>
#include <TChain.h>
#include <TCut.h>
#include <TFile.h>
#include <TH2.h>

/************************************************************************
 *	Calculate positron response on neutrinos from flat spectrum	*
 * A 2d histogram E_e+ : E_nu is created                                *
 ************************************************************************/

int main(int argc, char **argv)
{
	const double ENuMin = 1.81;
	const double deltaM = 0.5109989461 + 939.5654133 - 938.2720813;
	const double BinWidth = 0.125;
	const int BinsNu = 64;
	const int BinsE = 96;
	char str[1024];
	char *ptr;
	const char *shift;

	TChain *chain = new TChain("DanssPair");
	TChain *mc = new TChain("FromMC");

	if (argc < 3) {
		printf("Usage: %s mc_file_list.txt output_file.root [auxcuts [shift]]\n", argv[0]);
		return 100;
	}
	FILE *fIn = fopen(argv[1], "rt");
	if (!fIn) {
		printf("Can not open file %s : %m\n", argv[1]);
		return 10;
	}
	for(;;) {
		if (!fgets(str, sizeof(str), fIn)) break;
		if (strlen(str) < 2) continue;
		if (str[0] == '#') continue;
		str[strlen(str)-1] = '\0';
		ptr = strtok(str, " \t\n");
		chain->AddFile(ptr);
		ptr = strtok(NULL, " \t\n");
		mc->AddFile(ptr);
	}
	fclose(fIn);
	if (!chain->GetEntries()) {
		printf("No events found !\n");
		return 20;
	}
	if (chain->GetEntries() != mc->GetEntries()) {
		printf("Different number of entries: %d (pairs) %d (McTruth)\n", chain->GetEntries(), mc->GetEntries());
		return 30;
	}
	chain->AddFriend(mc);

	TFile *fOut = new TFile(argv[2], "RECREATE");
	if (!fOut->IsOpen()) return 40;
	
	TH2D *h = new TH2D("h2PE", "Reconstructed positron energy versus neutrino energy;E_{#nu}, MeV;E_{e+}, MeV", 
		BinsNu, ENuMin, ENuMin + BinsNu * BinWidth, BinsE, 0, BinsE * BinWidth);
	
//		Cuts
	TCut cAux = (argc > 3) ? argv[3] : "1";
	TCut cVeto("gtFromVeto > 60");
	TCut cIso("EventsBetween == 0");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut c20("gtDiff > 1");
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
//	TCut cPe("PositronEnergy > 0.75");
	TCut cPh("PositronHits < 8");
	TCut cR2("Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR3("Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR = cR3 && (cRXY || cR2);
	TCut cNH("NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
	TCut cNE("NeutronEnergy > 1.5 + 3 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cN = cNH && cNE;
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))");
        TCut ct = c20 && cIso && cX && cY && cZ && cR && cPh && cGamma && cN && cSingle && cAux;
        
        shift = (argc > 4) ? argv[4] : "0.0";
	
	sprintf(str, "PositronEnergy + %s:ParticleEnergy+MCPositronEnergy + %f", shift, deltaM);
	chain->Project(h->GetName(), str, ct);
	
	h->Write();
	fOut->Close();
	return 0;
}
