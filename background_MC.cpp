#include <stdio.h>
#include <TChain.h>
#include <TCut.h>
#include <TFile.h>
#include <TH1.h>

/****************************************
 *	Calculate various cuts for MC	*
 ****************************************/

#define NHISTS 24
void background_MC(TChain *chain, const char *fname, TCut cAux = (TCut) "")
{
	char strs[128];
	char strl[1024];
	const char *titles[] = {
		"gtDiff", "R1",  "R2",  "RZ", "PX", "PY", "PZ",  "NX",   "NY",  "NZ", 
		"NE",     "NH",  "PH",  "AH", "AE", "AM", "AMO", "P2AZ", "AH1", "AE1", 
		"P2AZ1",  "PX1", "PY1", "PZ1"};
	const char *titlel[] = {
		"MC: Time from positron to neutron;us;mHz/us",
		"MC: Distance between positron and neutron, 2D;cm;mHz/4cm", 
		"MC: Distance between positron and neutron, 3D;cm;mHz/4cm", 
		"MC: Distance between positron and neutron, Z;cm;mHz/cm", 
		"MC: Positron vertex X;cm;mHz/4cm", "MC: Positron vertex Y;cm;mHz/4cm", "MC: Positron vertex Z;cm;mHz/cm", 
		"MC: Neutron vertex X;cm;mHz/4cm", "MC: Neutron vertex Y;cm;mHz/4cm", "MC: Neutron vertex Z;cm;mHz/cm", 
		"MC: Neutron capture energy;MeV;mHz/200keV", "MC: Neutron capture SiPM Hits;hits;mHz/hit",
		"MC: Number of SiPM hits in positron cluster;Hits;mHz/hit", 
		"MC: Number of SiPM hits out of the cluster;Hits;mHz/hit", 
		"MC: Energy out of the cluster;MeV;mHz/200keV", 
		"MC: The most energetic hit out of the cluster;MeV;mHz/200keV",
		"MC: The most energetic hit out of the cluster - other cuts applied;MeV;mHz/200keV", 
		"MC: Distance from the cluster to the closest hit outside the cluster, Z;cm;mHz/cm",
		"MC: Number of SiPM hits out of the single hit cluster;Hits;mHz/hit", 
		"MC: Energy out of the single hit cluster;MeV;mHz/200keV", 
		"MC: Distance from the cluster to the closest hit outside the single hit cluster, Z;cm;mHz/cm",
		"MC: Positron vertex X, single hit cluster;cm;mHz/4cm", 
		"MC: Positron vertex Y, single hit cluster;cm;mHz/4cm", 
		"MC: Positron vertex Z, single hit cluster;cm;mHz/cm"
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
	TCut c20("gtDiff > 2");
	TCut cGamma("AnnihilationEnergy < 1.8 && AnnihilationGammas < 9");
	TCut cGammaMax("AnnihilationMax < 0.8");
	TCut cPe("PositronEnergy > 0.75");
	TCut cPh("PositronHits < 7");
	TCut cR1("Distance < 45");
	TCut cR2("Distance < 55");
	TCut cR = cR2 && (cRXY || cR1);
	TCut cN("NeutronEnergy > 3.5 && NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
        TCut cNZ("1");
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))");
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
		case 4:		// PX, PY, NX, NY, PX1, PY1
		case 5:
		case 7:
		case 8:
		case 21:
		case 22:
			h[i] = new TH1D(strs, titlel[i], 25, 0, 100.0);
			break;
		case 6:		// PZ, NZ, PZ1
		case 9:
		case 23:
			h[i] = new TH1D(strs, titlel[i], 100, 0, 100.0);
			break;
		case 10:	// NE
			h[i] = new TH1D(strs, titlel[i], 45, 3.0, 12.0);
			break;
		case 11:	// NH
			h[i] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 12:	// PH
			h[i] = new TH1D(strs, titlel[i], 10, 0, 10.0);
			break;
		case 13:	// AH, AH1
		case 18:
			h[i] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 14:	// AE, AM, AMO, AE1
		case 15:
		case 16:
		case 19:
			h[i] = new TH1D(strs, titlel[i], 20, 0, 4.0);
			break;
		case 17:	// P2AZ, P2AZ1
		case 20:
			h[i] = new TH1D(strs, titlel[i], 30, 0, 30.0);
			break;
		}
	}

	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cGamma && cGammaMax && cN && cNZ && cSingle;
	chain->Project(h[0]->GetName(), "gtDiff", ct && cAux);
	ct = cIso && cX && cY && cZ && cPe && cPh && cGamma && cGammaMax && cN && cNZ && !cRXY && cSingle;
	chain->Project(h[1]->GetName(), "Distance", ct && cAux);
	ct = cIso && cX && cY && cZ && cPe && cPh && cGamma && cGammaMax && cN && cNZ && cRXY && cSingle;
	chain->Project(h[2]->GetName(), "Distance", ct && cAux);
	ct = cIso && cX && cY && cZ && cPe && cPh && cGamma && cGammaMax && cN && cNZ && cSingle;
	chain->Project(h[3]->GetName(), "DistanceZ", ct && cAux);
	ct = cIso && cY && cZ && cR && cPe && cPh && cGamma && cGammaMax && cN && cNZ && "PositronX[0] >= 0" && cSingle;
	chain->Project(h[4]->GetName(), "PositronX[0] + 2.0", ct && cAux);
	ct = cIso && cX && cZ && cR && cPe && cPh && cGamma && cGammaMax && cN && cNZ && "PositronX[1] >= 0" && cSingle;
	chain->Project(h[5]->GetName(), "PositronX[1] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cR && cPe && cPh && cGamma && cGammaMax && cN && cNZ && "PositronX[2] >= 0" && cSingle;
	chain->Project(h[6]->GetName(), "PositronX[2] + 0.5", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cGamma && cGammaMax && cN && cNZ && "NeutronX[0] >= 0" && cSingle;
	chain->Project(h[7]->GetName(), "NeutronX[0] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cGamma && cGammaMax && cN && cNZ && "NeutronX[1] >= 0" && cSingle;
	chain->Project(h[8]->GetName(), "NeutronX[1] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cGamma && cGammaMax && cN && "NeutronX[2] >= 0" && cSingle;
	chain->Project(h[9]->GetName(), "NeutronX[2] + 0.5", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPh && cPe && cGamma && cGammaMax && cNZ && cSingle;
	chain->Project(h[10]->GetName(), "NeutronEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPh && cPe && cGamma && cGammaMax && cNZ && cSingle;
	chain->Project(h[11]->GetName(), "NeutronHits", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cGamma && cGammaMax && cN && cNZ && cSingle;
	chain->Project(h[12]->GetName(), "PositronHits", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cNZ && cSingle && "AnnihilationEnergy < 1.8";
	chain->Project(h[13]->GetName(), "AnnihilationGammas", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cNZ && cSingle && "AnnihilationGammas <= 10";
	chain->Project(h[14]->GetName(), "AnnihilationEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cNZ && cSingle;
	chain->Project(h[15]->GetName(), "AnnihilationMax", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cNZ && cGamma && cSingle;
	chain->Project(h[16]->GetName(), "AnnihilationMax", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cPh && cN && cNZ && cGamma && cGammaMax;
	chain->Project(h[17]->GetName(), "MinPositron2GammaZ", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cN && cNZ && "PositronHits == 1";
	chain->Project(h[18]->GetName(), "AnnihilationGammas", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cN && cNZ && "PositronHits == 1";
	chain->Project(h[19]->GetName(), "AnnihilationEnergy", ct && cAux);
	ct = cIso && cX && cY && cZ && cR && cPe && cN && cNZ && cGamma && cGammaMax && "PositronHits == 1";
	chain->Project(h[20]->GetName(), "MinPositron2GammaZ", ct && cAux);
	ct = cIso && cY && cZ && cR && cPe && cGamma && cNZ && cGammaMax && cN && "PositronX[0] >= 0 && PositronHits == 1";
	chain->Project(h[21]->GetName(), "PositronX[0] + 2.0", ct && cAux);
	ct = cIso && cX && cZ && cR && cPe && cGamma && cNZ && cGammaMax && cN && "PositronX[1] >= 0 && PositronHits == 1";
	chain->Project(h[22]->GetName(), "PositronX[1] + 2.0", ct && cAux);
	ct = cIso && cX && cY && cR && cPe && cGamma && cNZ && cGammaMax && cN && "PositronX[2] >= 0 && PositronHits == 1";
	chain->Project(h[23]->GetName(), "PositronX[2] + 0.5", ct && cAux);
	
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
