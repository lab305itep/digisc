#include <stdio.h>
#include <TCut.h>
#include <TFile.h>
#include <TH1.h>
#include <TTree.h>
#include "HPainter2.h"

/****************************************
 *	Calculate various cuts		*
 ****************************************/

#define NHISTS 31
#define THISTS 5
void background_calc(const char *name, int run_first, int run_last, TCut cAux = (TCut) "")
{
	char strs[128];
	char strl[1024];
	const char *titles[] = {
		"gtDiff", "R1",   "R2",   "RZ",   "PX",    "PY",  "PZ",  "NX",  "NY",  "NZ", 
		"NE",     "NE10", "NE20", "NE30", "NE40",  "NH",  "PH",  "AH",  "AE",  "AM", 
		"AMO",    "P2AZ", "AH1",  "AE1",  "P2AZ1", "PX1", "PY1", "PZ1", "PXN", "PYN", 
		"PZN"};
	const char *titlel[] = {
		"Time from positron to neutron;us;mHz/us",
		"Distance between positron and neutron, 2D;cm;mHz/4cm", 
		"Distance between positron and neutron, 3D;cm;mHz/4cm", 
		"Distance between positron and neutron, Z;cm;mHz/cm", 
		"Positron vertex X;cm;mHz/4cm", "Positron vertex Y;cm;mHz/4cm", "Positron vertex Z;cm;mHz/cm", 
		"Neutron vertex X;cm;mHz/4cm", "Neutron vertex Y;cm;mHz/4cm", "Neutron vertex Z;cm;mHz/cm", 
		"Neutron capture energy;MeV;mHz/100keV", 
		"Neutron capture energy, > 10 cm from the edge;MeV;mHz/100keV", 
		"Neutron capture energy, > 20 cm from the edge;MeV;mHz/100keV", 
		"Neutron capture energy, > 30 cm from the edge;MeV;mHz/100keV", 
		"Neutron capture energy, > 40 cm from the edge;MeV;mHz/100keV", 
		"Neutron capture hits;hits;mHz/hit",
		"Number of SiPM hits in positron cluster;Hits;mHz/hit", 
		"Number of hits out of the cluster;Hits;mHz/hit", 
		"Energy out of the cluster;MeV;mHz/100keV", 
		"The most energetic hit out of the cluster;MeV;mHz/100keV",
		"The most energetic hit out of the cluster - other cuts applied;MeV;mHz/100keV", 
		"Distance from the cluster to the closest hit outside the cluster, Z;cm;mHz/cm",
		"Number of hits out of the single hit cluster;Hits;mHz/hit", 
		"Energy out of the single hit cluster;MeV;mHz/100keV", 
		"Distance from the cluster to the closest hit outside the single hit cluster, Z;cm;mHz/cm",
		"Positron vertex X, single hit cluster;cm;mHz/4cm", 
		"Positron vertex Y, single hit cluster;cm;mHz/4cm", 
		"Positron vertex Z, single hit cluster;cm;mHz/cm",
		"Positron vertex X, multi hit cluster;cm;mHz/4cm", 
		"Positron vertex Y, multi hit cluster;cm;mHz/4cm", 
		"Positron vertex Z, multi hit cluster;cm;mHz/cm"
	};
	const char *ttitles[] = {"TSHOWER", "TMUON", "TBEFORE", "TAFTER", "TAFTERP"};
	const char *ttitlel[] = {
		"Time from showering event;us;mHz/2us", 
		"Time from muon event;us;mHz/2us", 
		"Time from non-muon event;us;mHz/2us", 
		"Time after neutron;us;mHz/2us",
		"Time after positron;us;mHz/2us",
	};
	TH1D *h[NHISTS][3];
	TH1D *hT[THISTS];
	TH1D *hConst;
	int i, j;
	const char *ptr;
	const char *what;
//		Main cuts
	TCut cVeto("gtFromVeto > 90");
	TCut cMuonA("gtFromVeto == 0");
	TCut cMuonB("gtFromVeto > 0 && gtFromVeto <= 90");
	TCut cIso("((gtFromPrevious > 50  || gtFromPrevious == gtFromVeto) && gtToNext > 80 && EventsBetween == 0)");
	TCut cShower("gtFromShower > 120 || ShowerEnergy < 800");
	TCut c20("gtDiff > 1");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
	TCut cPe("PositronEnergy > 0.5");
	TCut cPh("PositronHits < 8");
	TCut cR2("Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR3("Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR = cR3 && (cRXY || cR2);
	TCut cNH("NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
	TCut cNE("NeutronEnergy > 1.5 + 2.6 * exp(-0.15 * PositronEnergy*PositronEnergy)");
	TCut cN = cNH && cNE;
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))");
        TCut cNX10("NeutronX[0] > 8 && NeutronX[0] < 88");
        TCut cNY10("NeutronX[1] > 8 && NeutronX[1] < 88");
        TCut cNZ10("NeutronX[2] > 9.5 && NeutronX[2] < 89.5");
        TCut cN10 = cNX10 && cNY10 && cNZ10;
        TCut cNX20("NeutronX[0] > 18 && NeutronX[0] < 78");
        TCut cNY20("NeutronX[1] > 18 && NeutronX[1] < 78");
        TCut cNZ20("NeutronX[2] > 19.5 && NeutronX[2] < 79.5");
        TCut cN20 = cNX20 && cNY20 && cNZ20;
        TCut cNX30("NeutronX[0] > 28 && NeutronX[0] < 68");
        TCut cNY30("NeutronX[1] > 28 && NeutronX[1] < 68");
        TCut cNZ30("NeutronX[2] > 29.5 && NeutronX[2] < 69.5");
        TCut cN30 = cNX30 && cNY30 && cNZ30;
        TCut cNX40("NeutronX[0] > 38 && NeutronX[0] < 58");
        TCut cNY40("NeutronX[1] > 38 && NeutronX[1] < 58");
        TCut cNZ40("NeutronX[2] > 39.5 && NeutronX[2] < 59.5");
        TCut cN40 = cNX40 && cNY40 && cNZ40;
        TCut ct;
	TCut cv[3];

	cv[0] = cVeto;	// neutrino
	cv[1] = cMuonA; // fast neutron
	cv[2] = cMuonB; // pair of neutrons

	ptr = getenv("OUT_DIR");
	if (!ptr) ptr = "/home/clusters/rrcmpi/alekseev/igor/bgnd";
	sprintf(strl, "%s/%s.root", ptr, name);
	TFile *fRoot = new TFile(strl, "RECREATE");
//	printf("%s ==>> %s\n", strl, fRoot->GetName());
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
		case 4:		// PX, PY, NX, NY, PX1, PY1, PXN, PYN
		case 5:
		case 7:
		case 8:
		case 25:
		case 26:
		case 28:
		case 29:
			h[i][j] = new TH1D(strs, titlel[i], 25, 0, 100.0);
			break;
		case 6:		// PZ, NZ, PZ1, PZN
		case 9:
		case 27:
		case 30:
			h[i][j] = new TH1D(strs, titlel[i], 100, 0, 100.0);
			break;
		case 10:	// NE
		case 11:	// NE10
		case 12:	// NE20
		case 13:	// NE30
		case 14:	// NE40
			h[i][j] = new TH1D(strs, titlel[i], 105, 1.5, 12.0);
			break;
		case 15:	// NH
			h[i][j] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 16:	// PH
			h[i][j] = new TH1D(strs, titlel[i], 10, 0, 10.0);
			break;
		case 17:	// AH, AH1
		case 22:
			h[i][j] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 18:	// AE, AM, AMO, AE1
		case 19:
		case 20:
		case 23:
			h[i][j] = new TH1D(strs, titlel[i], 40, 0, 4.0);
			break;
		case 21:	// P2AZ, P2AZ1
		case 24:
			h[i][j] = new TH1D(strs, titlel[i], 30, 0, 30.0);
			break;
		}
	}
	for (i=0; i<THISTS; i++) hT[i] = new TH1D(ttitles[i], ttitlel[i], 250, 0, 500);
	hConst = new TH1D("hConst", "Run constants", 10, 0, 10);
	hConst->GetXaxis()->SetBinLabel(1, "gTime");

	ptr = getenv("PAIR_DIR");
	if (!ptr) ptr = "/home/clusters/rrcmpi/alekseev/igor/pair7n";

	HPainter2 *hp = new HPainter2(0x801E, run_first, run_last, ptr);
	if (hp->GetUpTime() < 10) {
		printf("%s:%d - %d: no runs !\n", ptr, run_first, run_last);
		fRoot->Close();
		return;
	}
	hp->SetFile(fRoot);
	hConst->SetBinContent(1, hp->GetUpTime());

	for (j=0; j<3; j++) {
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cGamma && cN && cSingle;
		hp->Project(h[0][j], "gtDiff", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cPh && cGamma && cN && !cRXY && cSingle;
		hp->Project(h[1][j], "Distance", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cPh && cGamma && cN && cRXY && cSingle;
		hp->Project(h[2][j], "Distance", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cPh && cGamma && cN && cSingle;
		hp->Project(h[3][j], "DistanceZ", ct && cv[j] && cAux);
		ct = cIso && cShower && cY && cZ && cR && cPe && cPh && cGamma && cN && "PositronX[0] >= 0" && cSingle;
		hp->Project(h[4][j], "PositronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cZ && cR && cPe && cPh && cGamma && cN && "PositronX[1] >= 0" && cSingle;
		hp->Project(h[5][j], "PositronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cR && cPe && cPh && cGamma && cN && "PositronX[2] >= 0" && cSingle;
		hp->Project(h[6][j], "PositronX[2] + 0.5", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cGamma && cN && "NeutronX[0] >= 0" && cSingle;
		hp->Project(h[7][j], "NeutronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cGamma && cN && "NeutronX[1] >= 0" && cSingle;
		hp->Project(h[8][j], "NeutronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cGamma && cN && "NeutronX[2] >= 0" && cSingle;
		hp->Project(h[9][j], "NeutronX[2] + 0.5", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle;
		hp->Project(h[10][j], "NeutronEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle && cN10;
		hp->Project(h[11][j], "NeutronEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle && cN20;
		hp->Project(h[12][j], "NeutronEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle && cN30;
		hp->Project(h[13][j], "NeutronEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle && cN40;
		hp->Project(h[14][j], "NeutronEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle;
		hp->Project(h[15][j], "NeutronHits", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cN && cSingle;
		hp->Project(h[16][j], "PositronHits", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cN && cSingle && "AnnihilationEnergy < 1.8";
		hp->Project(h[17][j], "AnnihilationGammas", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cN && cSingle && "AnnihilationGammas <= 10";
		hp->Project(h[18][j], "AnnihilationEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cN && cSingle;
		hp->Project(h[19][j], "AnnihilationMax", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cN && cGamma && cSingle;
		hp->Project(h[20][j], "AnnihilationMax", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cPh && cN && cGamma;
		hp->Project(h[21][j], "MinPositron2GammaZ", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && "PositronHits == 1";
		hp->Project(h[22][j], "AnnihilationGammas", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && "PositronHits == 1";
		hp->Project(h[23][j], "AnnihilationEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && cGamma && "PositronHits == 1";
		hp->Project(h[24][j], "MinPositron2GammaZ", ct && cv[j] && cAux);
		ct = cIso && cShower && cY && cZ && cR && cPe && cGamma && cN && "PositronX[0] >= 0 && PositronHits == 1";
		hp->Project(h[25][j], "PositronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cZ && cR && cPe && cGamma && cN && "PositronX[1] >= 0 && PositronHits == 1";
		hp->Project(h[26][j], "PositronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cR && cPe && cGamma && cN && "PositronX[2] >= 0 && PositronHits == 1";
		hp->Project(h[27][j], "PositronX[2] + 0.5", ct && cv[j] && cAux);
		ct = cIso && cShower && cY && cZ && cR && cPe && cGamma && cN && "PositronX[0] >= 0 && PositronHits > 1";
		hp->Project(h[28][j], "PositronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cZ && cR && cPe && cGamma && cN && "PositronX[1] >= 0 && PositronHits > 1";
		hp->Project(h[29][j], "PositronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cR && cPe && cGamma && cN && "PositronX[2] >= 0 && PositronHits > 1";
		hp->Project(h[30][j], "PositronX[2] + 0.5", ct && cv[j] && cAux);
	}
	
	for (i=0; i<THISTS; i++) {
		ct = cX && cY && cZ && cR && cPe && cGamma && cN && cSingle && "EventsBetween == 0";
		switch(i) {
		case 0:
			what = "gtFromShower";
			ct = ct && TCut("gtToNext > 80");
			break;
		case 1:
			what = "gtFromVeto";
			ct = ct && TCut("gtToNext > 80 && gtFromShower > 200");
			break;
		case 2:
			what = "gtFromPrevious";
			ct = ct && TCut("gtToNext > 80 && gtFromShower > 200 && gtFromVeto > gtFromPrevious");
			break;
		case 3:
			what = "gtToNext - gtDiff";
			ct = ct && TCut("gtFromShower > 200 && gtFromVeto > 60 && gtFromPrevious > 45");
			break;
		case 4:
			what = "gtToNext";
			ct = ct && TCut("gtFromShower > 200 && gtFromVeto > 60 && gtFromPrevious > 45");
			break;
		}
		hp->Project(hT[i], what, ct);
	}
	
	fRoot->cd();
	for (i=0; i<NHISTS; i++) for (j=0; j<3; j++) h[i][j]->Write();
	for (i=0; i<THISTS; i++) hT[i]->Write();
	hConst->Write();
	delete hp;
	fRoot->Close();
}

int main(int argc, char **argv)
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
