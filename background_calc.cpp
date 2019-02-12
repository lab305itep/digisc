#include <stdio.h>
#include <TCut.h>
#include <TFile.h>
#include <TH1.h>
#include <TTree.h>
#include "HPainter2.h"

/****************************************
 *	Calculate various cuts		*
 ****************************************/

#define NHISTS 24
void background_calc(const char *name, int run_first, int run_last, TCut cAux = (TCut) "")
{
	char strs[128];
	char strl[1024];
	const char titles[NHISTS][16] = {
		"gtDiff", "R1",  "R2",  "RZ", "PX", "PY", "PZ",  "NX",   "NY",  "NZ", 
		"NE",     "NH",  "PH",  "AH", "AE", "AM", "AMO", "P2AZ", "AH1", "AE1", 
		"P2AZ1",  "PX1", "PY1", "PZ1"};
	const char titlel[NHISTS][256] = {
		"Time from positron to neutron;us",
		"Distance between positron and neutron, 2D;cm", "Distance between positron and neutron, 3D;cm", "Distance between positron and neutron, Z;cm", 
		"Positron vertex X;cm", "Positron vertex Y;cm", "Positron vertex Z;cm", 
		"Neutron vertex X;cm", "Neutron vertex Y;cm", "Neutron vertex Z;cm", 
		"Neutron capture energy;MeV", "Neutron capture SiPM hits",
		"Number of SiPM hits in positron cluster", 
		"Number of SiPM hits out of the cluster", "Energy out of the cluster;MeV", "The most energetic hit out of the cluster;MeV",
		"The most energetic hit out of the cluster - other cuts applied;MeV", "Distance from the cluster to the closest hit outside the cluster, Z;cm",
		"Number of SiPM hits out of the single hit cluster", "Energy out of the single hit cluster;MeV", 
		"Distance from the cluster to the closest hit outside the single hit cluster, Z;cm",
		"Positron vertex X, single hit cluster;cm", "Positron vertex Y, single hit cluster;cm", "Positron vertex Z, single hit cluster;cm"
	};
	TH1D *h[NHISTS][3];
	int i, j;
	const char *ptr;
//		Main cuts
	TCut cVeto("gtFromVeto > 60");
	TCut cMuonA("gtFromVeto == 0");
	TCut cMuonB("gtFromVeto > 0 && gtFromVeto <= 60");
	TCut cIso("((gtFromPrevious > 45  || gtFromPrevious == gtFromVeto) && gtToNext > 80 && EventsBetween == 0)");
	TCut cShower("gtFromShower > 200 || ShowerEnergy < 800");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut c20("gtDiff > 2");
        TCut cGamma("AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10");
	TCut cGammaMax("AnnihilationMax < 0.8");
        TCut cPe("PositronEnergy > 1");
        TCut cR1("Distance < 45");
        TCut cR2("Distance < 55");
        TCut cR = cR2 && (cRXY || cR1);
        TCut cN("NeutronEnergy > 3.5 && NeutronEnergy < 15.0 && NeutronHits >= 3");
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))");
        TCut ct;
	TCut cv[3];

	cv[0] = cVeto;	// neutrino
	cv[1] = cMuonA; // fast neutron
	cv[2] = cMuonB; // pair of neutrons

	ptr = getenv("OUTPUT_DIR");
	if (!ptr) ptr = "/home/clusters/rrcmpi/alekseev/igor/bgnd";
	sprintf(strl, "%s/%s.root", ptr, name);
	TFile *fRoot = new TFile(strl, "RECREATE");
	printf("%s ==>> %s\n", strl, fRoot->GetName());
	for (i=0; i<NHISTS; i++) for (j=0; j<3; j++) {
		sprintf(strs, "h%s%c", titles[i], 'A' + j);
		switch(i) {
		case 0:		// gtDiff
			h[i][j] = new TH1D(strs, titlel[i], 50, 0, 50.0);
			break;
		case 1:		// R1, R2
		case 2:
			h[i][j] = new TH1D(strs, titlel[i], 40, 0, 160.0);
			break;
		case 3:		// RZ
			h[i][j] = new TH1D(strs, titlel[i], 100, -50.0, 50.0);
			break;
		case 4:		// PX, PY, NX, NY, PX1, PY1
		case 5:
		case 7:
		case 8:
		case 21:
		case 22:
			h[i][j] = new TH1D(strs, titlel[i], 25, 0, 100.0);
			break;
		case 6:		// PZ, NZ, PZ1
		case 9:
		case 23:
			h[i][j] = new TH1D(strs, titlel[i], 100, 0, 100.0);
			break;
		case 10:	// NE
			h[i][j] = new TH1D(strs, titlel[i], 45, 3.0, 12.0);
			break;
		case 11:	// NH
			h[i][j] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 12:	// PH
			h[i][j] = new TH1D(strs, titlel[i], 10, 0, 10.0);
			break;
		case 13:	// AH, AH1
		case 18:
			h[i][j] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 14:	// AE, AM, AMO, AE1
		case 15:
		case 16:
		case 19:
			h[i][j] = new TH1D(strs, titlel[i], 20, 0, 4.0);
			break;
		case 17:	// P2AZ, P2AZ1
		case 20:
			h[i][j] = new TH1D(strs, titlel[i], 30, 0, 30.0);
			break;
		}
	}

	HPainter2 *hp = new HPainter2(0x801E, run_first, run_last, "/home/clusters/rrcmpi/alekseev/igor/pair7");
	if (hp->GetUpTime() < 10) {
		printf("%d - %d: no runs !\n", run_first, run_last);
		fRoot->Close();
		return;
	}
	hp->SetFile(fRoot);

	for (j=0; j<3; j++) {
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cGammaMax && cN && cSingle;
		hp->Project(h[0][j], "gtDiff", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cGamma && cGammaMax && cN && !cRXY && cSingle;
		hp->Project(h[1][j], "Distance", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cGamma && cGammaMax && cN && cRXY && cSingle;
		hp->Project(h[2][j], "Distance", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cGamma && cGammaMax && cN && cSingle;
		hp->Project(h[3][j], "DistanceZ", ct && cv[j] && cAux);
		ct = cIso && cShower && cY && cZ && cR && cPe && cGamma && cGammaMax && cN && "PositronX[0] >= 0" && cSingle;
		hp->Project(h[4][j], "PositronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cZ && cR && cPe && cGamma && cGammaMax && cN && "PositronX[1] >= 0" && cSingle;
		hp->Project(h[5][j], "PositronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cR && cPe && cGamma && cGammaMax && cN && "PositronX[2] >= 0" && cSingle;
		hp->Project(h[6][j], "PositronX[2] + 0.5", ct && cv[j] && cAux);
		ct = cIso && cShower && cY && cZ && cR && cPe && cGamma && cGammaMax && cN && "NeutronX[0] >= 0" && cSingle;
		hp->Project(h[7][j], "NeutronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cZ && cR && cPe && cGamma && cGammaMax && cN && "NeutronX[1] >= 0" && cSingle;
		hp->Project(h[8][j], "NeutronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cR && cPe && cGamma && cGammaMax && cN && "NeutronX[2] >= 0" && cSingle;
		hp->Project(h[9][j], "NeutronX[2] + 0.5", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cGammaMax && cSingle;
		hp->Project(h[10][j], "NeutronEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cGammaMax && cSingle;
		hp->Project(h[11][j], "NeutronHits", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cGammaMax && cSingle;
		hp->Project(h[12][j], "PositronHits", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && cSingle;
		hp->Project(h[13][j], "AnnihilationGammas", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && cSingle;
		hp->Project(h[14][j], "AnnihilationEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && cSingle;
		hp->Project(h[15][j], "AnnihilationMax", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && cGamma && cSingle;
		hp->Project(h[16][j], "AnnihilationMax", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && cGamma && cGammaMax;
		hp->Project(h[17][j], "MinPositron2GammaZ", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && "PositronHits == 1";
		hp->Project(h[18][j], "AnnihilationGammas", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && "PositronHits == 1";
		hp->Project(h[19][j], "AnnihilationEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && cGamma && cGammaMax && "PositronHits == 1";
		hp->Project(h[20][j], "MinPositron2GammaZ", ct && cv[j] && cAux);
		ct = cIso && cShower && cY && cZ && cR && cPe && cGamma && cGammaMax && cN && "PositronX[0] >= 0 && PositronHits == 1";
		hp->Project(h[21][j], "PositronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cZ && cR && cPe && cGamma && cGammaMax && cN && "PositronX[1] >= 0 && PositronHits == 1";
		hp->Project(h[22][j], "PositronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cR && cPe && cGamma && cGammaMax && cN && "PositronX[2] >= 0 && PositronHits == 1";
		hp->Project(h[23][j], "PositronX[2] + 0.5", ct && cv[j] && cAux);
	}
	
	fRoot->cd();
	for (i=0; i<NHISTS; i++) for (j=0; j<3; j++) h[i][j]->Write();
	delete hp;
	fRoot->Close();
}

int main(int argc, char **argv)
// void spectr_all(int nSect, const char *fname = "danss_report_v4.root", TCut cAux = (TCut)"", double bgScale = 2.24)	// 5.6% from reactor OFF data
{
#include "positions.h"
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
