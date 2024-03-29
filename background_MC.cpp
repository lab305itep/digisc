#include <stdio.h>
#include <TChain.h>
#include <TCut.h>
#include <TFile.h>
#include <TH1.h>

/****************************************
 *	Calculate various cuts for MC	*
 ****************************************/

#define NHISTS 31
void background_MC(TChain *chain, const char *fname, TCut cAux = (TCut) "")
{
	char strs[128];
	char strl[1024];
	const char *titles[] = {
		"gtDiff", "R1",  "R2",  "RZ", "PX", "PY", "PZ",  "NX",   "NY",  "NZ", 
		"NE",     "NE10", "NE20", "NE30", "NE40",  "NH",  "PH",  "AH",  "AE",  "AM", 
		"AMO",    "P2AZ", "AH1",  "AE1",  "P2AZ1", "PX1", "PY1", "PZ1", "PXN", "PYN", 
		"PZN"};
	const char *titlel[] = {
		"MC: Time from positron to neutron;us;mHz/us",
		"MC: Distance between positron and neutron, 2D;cm;mHz/4cm", 
		"MC: Distance between positron and neutron, 3D;cm;mHz/4cm", 
		"MC: Distance between positron and neutron, Z;cm;mHz/cm", 
		"MC: Positron vertex X;cm;mHz/4cm", "MC: Positron vertex Y;cm;mHz/4cm", "MC: Positron vertex Z;cm;mHz/cm", 
		"MC: Neutron vertex X;cm;mHz/4cm", "MC: Neutron vertex Y;cm;mHz/4cm", "MC: Neutron vertex Z;cm;mHz/cm", 
		"Neutron capture energy;MeV;mHz/100keV", 
		"Neutron capture energy, > 4 cm from the edge;MeV;mHz/100keV", 
		"Neutron capture energy, > 12 cm from the edge;MeV;mHz/100keV", 
		"Neutron capture energy, > 24 cm from the edge;MeV;mHz/100keV", 
		"Neutron capture energy, > 36 cm from the edge;MeV;mHz/100keV", 
		"Neutron capture hits;hits;mHz/hit",
		"MC: Number of SiPM hits in positron cluster;Hits;mHz/hit", 
		"MC: Number of hits out of the cluster;Hits;mHz/hit", 
		"MC: Energy out of the cluster;MeV;mHz/100keV", 
		"MC: The most energetic hit out of the cluster;MeV;mHz/100keV",
		"MC: The most energetic hit out of the cluster - other cuts applied;MeV;mHz/100keV", 
		"MC: Distance from the cluster to the closest hit outside the cluster, Z;cm;mHz/cm",
		"MC: Number of hits out of the single hit cluster;Hits;mHz/hit", 
		"MC: Energy out of the single hit cluster;MeV;mHz/100keV", 
		"MC: Distance from the cluster to the closest hit outside the single hit cluster, Z;cm;mHz/cm",
		"MC: Positron vertex X, single hit cluster;cm;mHz/4cm", 
		"MC: Positron vertex Y, single hit cluster;cm;mHz/4cm", 
		"MC: Positron vertex Z, single hit cluster;cm;mHz/cm",
		"MC: Positron vertex X, multi hit cluster;cm;mHz/4cm", 
		"MC: Positron vertex Y, multi hit cluster;cm;mHz/4cm", 
		"MC: Positron vertex Z, multi hit cluster;cm;mHz/cm"
	};
	TH1D *h[NHISTS];
	int i;
//		Main cuts
	TCut cVeto("gtFromVeto > 60");
	TCut cIso("EventsBetween == 0");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut c20("gtDiff > 1");
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
	TCut cPe("PositronEnergy > 0.75");
	TCut cPh("PositronHits < 8");
	TCut cR2("Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR3("Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR = cR3 && (cRXY || cR2);
	TCut cNH("NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
	TCut cNE("NeutronEnergy > 1.5 + 2.6 * exp(-0.15 * PositronEnergy*PositronEnergy)");
	TCut cN = cNH && cNE;
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))");
        TCut cNX10("PositronX[0] > 2 && PositronX[0] < 94");
        TCut cNY10("PositronX[1] > 2 && PositronX[1] < 94");
        TCut cNZ10("PositronX[2] > 3.5 && PositronX[2] < 95.5");
        TCut cN10 = cNX10 && cNY10 && cNZ10;
        TCut cNX20("PositronX[0] > 10 && PositronX[0] < 86");
        TCut cNY20("PositronX[1] > 10 && PositronX[1] < 86");
        TCut cNZ20("PositronX[2] > 11.5 && PositronX[2] < 87.5");
        TCut cN20 = cNX20 && cNY20 && cNZ20;
        TCut cNX30("PositronX[0] > 22 && PositronX[0] < 74");
        TCut cNY30("PositronX[1] > 22 && PositronX[1] < 74");
        TCut cNZ30("PositronX[2] > 23.5 && PositronX[2] < 75.5");
        TCut cN30 = cNX30 && cNY30 && cNZ30;
        TCut cNX40("PositronX[0] > 34 && PositronX[0] < 62");
        TCut cNY40("PositronX[1] > 34 && PositronX[1] < 62");
        TCut cNZ40("PositronX[2] > 35.5 && PositronX[2] < 63.5");
        TCut cN40 = cNX40 && cNY40 && cNZ40;
        TCut ct;

	TFile *fRoot = new TFile(fname, "RECREATE");
	for (i=0; i<NHISTS; i++) {
		sprintf(strs, "h%s_MC", titles[i]);
		switch(i) {
		case 0:		// gtDiff
			h[i] = new TH1D(strs, titlel[i], 50, 0, 50.0);
			break;
		case 1:		// R1, R2
		case 2:
			h[i] = new TH1D(strs, titlel[i], 40, 0, 160.0);
			break;
		case 3:		// RZ
			h[i] = new TH1D(strs, titlel[i], 100, -50.0, 50.0);
			break;
		case 4:		// PX, PY, NX, NY, PX1, PY1, PXN, PYN
		case 5:
		case 7:
		case 8:
		case 25:
		case 26:
		case 28:
		case 29:
			h[i] = new TH1D(strs, titlel[i], 25, 0, 100.0);
			break;
		case 6:		// PZ, NZ, PZ1, PZN
		case 9:
		case 27:
		case 30:
			h[i] = new TH1D(strs, titlel[i], 100, 0, 100.0);
			break;
		case 10:	// NE
		case 11:	// NE10
		case 12:	// NE20
		case 13:	// NE30
		case 14:	// NE40
			h[i] = new TH1D(strs, titlel[i], 105, 1.5, 12.0);
			break;
		case 15:	// NH
			h[i] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 16:	// PH
			h[i] = new TH1D(strs, titlel[i], 10, 0, 10.0);
			break;
		case 17:	// AH, AH1
		case 22:
			h[i] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 18:	// AE, AM, AMO, AE1
		case 19:
		case 20:
		case 23:
			h[i] = new TH1D(strs, titlel[i], 40, 0, 4.0);
			break;
		case 21:	// P2AZ, P2AZ1
		case 24:
			h[i] = new TH1D(strs, titlel[i], 30, 0, 30.0);
			break;
		}
	}

	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cGamma && cN && cSingle;
	chain->Project(h[0]->GetName(), "gtDiff", ct && cAux);
	ct = cIso && cX && cY && cZ && cPe && cPh && cGamma && cN && !cRXY && cSingle;
	chain->Project(h[1]->GetName(), "Distance", ct && cAux);
	ct = cIso && cX && cY && cZ && cPe && cPh && cGamma && cN && cRXY && cSingle;
	chain->Project(h[2]->GetName(), "Distance", ct && cAux);
	ct = cIso && cX && cY && cZ && cPe && cPh && cGamma && cN && cSingle;
	chain->Project(h[3]->GetName(), "DistanceZ", ct && cAux);
	ct = cIso && cY && cZ && cR && cPe && cPh && cGamma && cN && "PositronX[0] >= 0" && cSingle;
	chain->Project(h[4]->GetName(), "PositronX[0] + 2.0", ct && cAux);
	ct = cIso && cX && cZ && cR && cPe && cPh && cGamma && cN && "PositronX[1] >= 0" && cSingle;
	chain->Project(h[5]->GetName(), "PositronX[1] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cR && cPe && cPh && cGamma && cN && "PositronX[2] >= 0" && cSingle;
	chain->Project(h[6]->GetName(), "PositronX[2] + 0.5", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cGamma && cN && "NeutronX[0] >= 0" && cSingle;
	chain->Project(h[7]->GetName(), "NeutronX[0] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cGamma && cN && "NeutronX[1] >= 0" && cSingle;
	chain->Project(h[8]->GetName(), "NeutronX[1] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cGamma && cN && "NeutronX[2] >= 0" && cSingle;
	chain->Project(h[9]->GetName(), "NeutronX[2] + 0.5", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle;
	chain->Project(h[10]->GetName(), "NeutronEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle && cN10;
	chain->Project(h[11]->GetName(), "NeutronEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle && cN20;
	chain->Project(h[12]->GetName(), "NeutronEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle && cN30;
	chain->Project(h[13]->GetName(), "NeutronEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle && cN40;
	chain->Project(h[14]->GetName(), "NeutronEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPh && cPe && cGamma && cSingle;
	chain->Project(h[15]->GetName(), "NeutronHits", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cGamma && cN && cSingle;
	chain->Project(h[16]->GetName(), "PositronHits", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cSingle && "AnnihilationEnergy < 1.8";
	chain->Project(h[17]->GetName(), "AnnihilationGammas", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cSingle && "AnnihilationGammas <= 10";
	chain->Project(h[18]->GetName(), "AnnihilationEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cSingle;
	chain->Project(h[19]->GetName(), "AnnihilationMax", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cGamma && cSingle;
	chain->Project(h[20]->GetName(), "AnnihilationMax", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cGamma;
	chain->Project(h[21]->GetName(), "MinPositron2GammaZ", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cN && "PositronHits == 1";
	chain->Project(h[22]->GetName(), "AnnihilationGammas", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cN && "PositronHits == 1";
	chain->Project(h[23]->GetName(), "AnnihilationEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cN && cGamma && "PositronHits == 1";
	chain->Project(h[24]->GetName(), "MinPositron2GammaZ", ct && cAux);
	ct = cIso && cY && cZ && cR && cPe && cGamma && cN && "PositronX[0] >= 0 && PositronHits == 1";
	chain->Project(h[25]->GetName(), "PositronX[0] + 2.0", ct && cAux);
	ct = cIso && cX && cZ && cR && cPe && cGamma &&  cN && "PositronX[1] >= 0 && PositronHits == 1";
	chain->Project(h[26]->GetName(), "PositronX[1] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cR && cPe && cGamma && cN && "PositronX[2] >= 0 && PositronHits == 1";
	chain->Project(h[27]->GetName(), "PositronX[2] + 0.5", ct && cAux);
	ct = cIso && cY && cZ && cR && cPe && cGamma &&  cN && "PositronX[0] >= 0 && PositronHits > 1";
	chain->Project(h[28]->GetName(), "PositronX[0] + 2.0", ct && cAux);
	ct = cIso && cX && cZ && cR && cPe && cGamma && cN && "PositronX[1] >= 0 && PositronHits > 1";
	chain->Project(h[29]->GetName(), "PositronX[1] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cR && cPe && cGamma && cN && "PositronX[2] >= 0 && PositronHits > 1";
	chain->Project(h[30]->GetName(), "PositronX[2] + 0.5", ct && cAux);
	
	fRoot->cd();
	for (i=0; i<NHISTS; i++) h[i]->Write();
	fRoot->Close();
}

int main(int argc, char **argv)
{
	const char * auxstr;
	char str[1024];
	TChain *chain;
	FILE *fIn;

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
