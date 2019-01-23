#include <stdio.h>
#include <stdlib.h>

#include <TCut.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include "HPainter2.h"


void make_cuts(TCut &cSig, TCut &cBgnd, TCut cAux)
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
	cBgnd = cSel && (!cVeto);
}

int main(int argc, char **argv)
{
	TCut cSig;
	TCut cBgnd;
	TCut cAux("SiPmCleanEnergy[0] > 1 && PmtCleanEnergy[0] > 1");
	char fname[1024];
	const int mask = 0x801E;
	int ifirst, ilast;

	if (argc < 4) {
		printf("Usage: %s first_run last_run outdir\n", argv[0]);
		return 100;
	}
	int run_from = strtol(argv[1], NULL, 10);
	int run_to   = strtol(argv[2], NULL, 10);
	
	snprintf(fname, sizeof(fname), "%s/pmt2sipm_%d_%d.root", argv[3], run_from, run_to);
	TFile *fRoot = new TFile (fname, "RECREATE");
	make_cuts(cSig, cBgnd, cAux);

	TH2D *hSig  = new TH2D("hPmt2Sipm",  "SiPm - Pmt difference;(SiPm+Pmt)/2, MeV;2(SiPm-Pmt)/(SiPm+Pmt)", 28, 1, 8, 100, -1, 1);
	
	HPainter2 *ptr2 = new HPainter2(mask, run_from, run_to, "/home/clusters/rrcmpi/alekseev/igor/pair");
	ptr2->SetFile(fRoot);
//	ptr2->Project(hSig, 
//		"2*(PositronSiPmEnergy-PositronPmtEnergy)/(PositronSiPmEnergy+PositronPmtEnergy):(PositronSiPmEnergy+PositronPmtEnergy)/2", 
//		cSig);
//		Ugly patch to process old files
//	Old coeffs: 0.929 0.179
//	New PMT:    0.929 0.165
//	New SiPM:   0.921 0.148
//	E_SiPM = E_SiPM_old * 1.0087 + 0.0337
//	E_PMT  = E_PMT_old           + 0.0151
//	Add 35 keV to SiPM
	if (ptr2->GetUpTime() > 0.1) ptr2->Project(hSig, "(1.0087*PositronSiPmEnergy + 0.054 - PositronPmtEnergy) / PositronEnergy:PositronEnergy", cSig);
	fRoot->cd();
	hSig->Write();
	fRoot->Close();
	delete ptr2;
}
