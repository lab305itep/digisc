#include <stdio.h>
#include <stdlib.h>

#include <TCut.h>
#include <TFile.h>
#include <TH3.h>

#include "HPainter2.h"

void make_cuts(TCut &cSig, TCut cAux)
{
//		Set cuts
	TCut cVeto("gtFromVeto > 60");
	TCut cMuonA("gtFromVeto == 0");
	TCut cMuonB("gtFromVeto > 0 && gtFromVeto <= 60");
	TCut cIso("(gtFromPrevious > 45 && gtToNext > 80 && EventsBetween == 0) || (gtFromPrevious == gtFromVeto)");
	TCut cShower("gtFromVeto > 200 || DanssEnergy < 300");
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
//        TCut cRZ("fabs(DistanceZ) < 40");
        TCut cR = cR2 && (cRXY || cR1);
        TCut cN("NeutronEnergy > 3.5");
	TCut cSel = cX && cY && cZ && cR && c20 && cGamma && cGammaMax && cN && cPe && cIso && cShower && cAux;
	cSig = cSel && cVeto;
}

int main(int argc, char **argv)
{
	const int mask = 0xE;
	int i_begin, i_end;
	char *nameOut;
	TH3D *hXZ;
	TH3D *hYZ;
	TCut cSig;
	TCut cAux;

	if (argc < 3) {
		printf("Usage: %s run_begin run_end [file_out [aux_cuts]]\n", argv[0]);
		return 100;
	}
	i_begin = strtol(argv[1], NULL, 10);
	i_end = strtol(argv[2], NULL, 10);
	if (argc > 3) {
		nameOut = argv[3];
	} else {
		nameOut = (char *)malloc(1024);
		sprintf(nameOut, "xyz_%6.6d_%6.6d.root", i_begin, i_end);
	}
	if (argc > 4) {
		cAux = (TCut)argv[4];
	} else {
		cAux = (TCut)"";
	}
	
	TFile *fRoot = new TFile (nameOut, "RECREATE");
	if (!fRoot->IsOpen()) return 200;
	hXZ = new TH3D("hXZ", "XZ-energy distribution;X, cm;Z, cm;E, MeV", 25, 0, 100, 50, 0, 100, 24, 1, 7);
	hYZ = new TH3D("hYZ", "YZ-energy distribution;Y, cm;Z, cm;E, MeV", 25, 0, 100, 50, 0, 100, 24, 1, 7);
	make_cuts(cSig, cAux);

	HPainter2 *ptr2 = new HPainter2(mask, i_begin, i_end, "/home/clusters/rrcmpi/alekseev/igor/pair7_dead/");
	ptr2->SetFile(fRoot);
	ptr2->Project(hXZ, "PositronEnergy:PositronX[2]+0.5:PositronX[0]+2", cSig && "PositronX[0] >= 0");
	ptr2->Project(hYZ, "PositronEnergy:PositronX[2]+0.5:PositronX[1]+2", cSig && "PositronX[1] >= 0");
	
	hXZ->Write();
	hYZ->Write();
	
	fRoot->Close();
	delete ptr2;
	return 0;
}
