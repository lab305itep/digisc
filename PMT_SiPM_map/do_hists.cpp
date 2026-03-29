#include <libgen.h>
#include <stdio.h>

#include <TChain.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>

#include "../evtbuilder.h"

#define iMaxDataElements 3000

enum HitType { bSiPM = 0, bPMT = 1, bVETO = 2};

struct HitStruct {
	float			E[iMaxDataElements];
	float			T[iMaxDataElements];
	struct HitTypeStruct 	type[iMaxDataElements];
};

struct PMTHitsStruct {
	int nHits;
	int z;
	int xy;
	double E;
};

int main(int argc, char **argv)
{
	struct DanssEventStruct7 DanssEvent;
	struct HitStruct HitArray;
	char *ptr;
	char str[1024];
	char strl[4096];
	FILE *fList;
	TChain *EventChain;
	long iEvt, nEvt;
	TFile *fOut;
	int side;
	int z_PMT, xy_PMT;
	int i, j, k, l, m, n;
	struct PMTHitsStruct PMTHits[2][5][5];
	TH1D *h[2][5][5][10][5];	// side, z_PMT, xy_PMT, z_SiPM in PMT, xy_SiPM in PMT
	TH1D *hAvr[10][5];		// z_SiPM in PMT, xy_SiPM in PMT
	TH2D *hMapPMTX;
	TH2D *hMapPMTY;
	TH2D *hMapSiPM;
	TH2D *hMapSiPMX;
	TH2D *hMapSiPMY ;

//			Check number of arguments
	if (argc < 3) {
		printf("Usage: %s list_file.txt|input_file.root output_file.root\n", argv[0]);
		printf("Will process files in the list_file and create root-file\n");
		return 10;
	}
//			The first argument must be a single root file or a list
	ptr = strrchr(argv[1], '.');
	if (!ptr) {
		printf("Strange file extention: .txt or .root expected\n");
		return 15;
	}
//			Create Input chains
	EventChain = new TChain("DanssEvent");
//			Add files to input chains
	if (!strcmp(ptr, ".txt")) {
		fList = fopen(argv[1], "rt");
		if (!fList) {
			printf("Can not open list of files %s: %m\n", argv[1]);
			return 20;
		}
	
		for(;;) {
			if (!fgets(str, sizeof(str), fList)) break;
			ptr = strchr(str, '\n');
			if (ptr) *ptr = '\0';
			EventChain->Add(str);
		}
		fclose(fList);
	} else if (!strcmp(ptr, ".root")) {
		EventChain->Add(argv[1]);
	} else {
		printf("Strange file extention: .txt or .root expected\n");
		return 30;
	}

	EventChain->SetBranchAddress("Data", &DanssEvent);
	EventChain->SetBranchAddress("HitE", HitArray.E);
	EventChain->SetBranchAddress("HitT", HitArray.T);
	EventChain->SetBranchAddress("HitType", HitArray.type);
	nEvt = EventChain->GetEntries();

//			Create output directory
	strncpy(str, argv[2], sizeof(str));
	sprintf(strl, "mkdir -p %s", dirname(str));
	if (system(strl)) {
		printf("Can not crete target directory: %m\n");
		return -5;
	}
//			Open output file
	fOut = new TFile(argv[2], "RECREATE");
	if (!fOut->IsOpen()) {
		printf("Can not open the output file %s: %m\n", argv[2]);
		return -10;
	}
	
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<10; m++) for (n=0; n<5; n++) {
		sprintf(str, "hRatio_%c_z%dxy%d_z%dxy%d", (j) ? 'X' : 'Y', k, l, m , n);
		sprintf(strl, "PMT/SiPM for side %c PMT z%dxy%d SiPM z%dxy%d", (j) ? 'X' : 'Y', k, l, m , n);
		h[j][k][l][m][n] = new TH1D(str, strl, 108, 0.3, 3);
	}
	for (m=0; m<10; m++) for (n=0; n<5; n++) {
		sprintf(str, "hRatio_Avr_z%dxy%d", m , n);
		sprintf(strl, "Average PMT/SiPM SiPM z%dxy%d", m , n);
		hAvr[m][n] = new TH1D(str, strl, 108, 0.3, 3);
	}
	
	hMapPMTX = new TH2D("hMapPMTX", "PMT fill map XZ", 5, 0, 5, 5, 0, 5);
	hMapPMTY = new TH2D("hMapPMTY", "PMT fill map YZ", 5, 0, 5, 5, 0, 5);
	hMapSiPM = new TH2D("hMapSiPM", "SiPM fill map XYZ", 5, 0, 5, 10, 0, 10);
	hMapSiPMX = new TH2D("hMapSiPMX", "SiPM fill map XZ", 25, 0, 25, 50, 0, 50);
	hMapSiPMY = new TH2D("hMapSiPMY", "SiPM fill map YZ", 25, 0, 25, 50, 0, 50);

	for (iEvt =0; iEvt < nEvt; iEvt++) {
		EventChain->GetEntry(iEvt);
	// Count SiPM hits in each PMT
		memset(PMTHits, 0, sizeof(PMTHits));
		for (i=0; i<DanssEvent.NHits; i++) if (HitArray.type[i].type == bSiPM) {
			side = HitArray.type[i].z & 1;
			z_PMT = HitArray.type[i].z / 20;
			xy_PMT = HitArray.type[i].xy / 5;
			PMTHits[side][z_PMT][xy_PMT].nHits++;
			PMTHits[side][z_PMT][xy_PMT].z = (HitArray.type[i].z - 20 * z_PMT) / 2;
			PMTHits[side][z_PMT][xy_PMT].xy = HitArray.type[i].xy - 5 * xy_PMT;
			PMTHits[side][z_PMT][xy_PMT].E = HitArray.E[i];	// we will use this number if it is a single number only
//			printf("%d %d => %c %d %d : %d %d\n", HitArray.type[i].z, HitArray.type[i].xy, (side) ? 'X' : 'Y', z_PMT, xy_PMT,
//				PMTHits[side][z_PMT][xy_PMT].z, PMTHits[side][z_PMT][xy_PMT].xy);
		}
	// Fill Ratio hist if the hit in PMT is single
		for (i=0; i<DanssEvent.NHits; i++) if (HitArray.type[i].type == bPMT) {
			side = HitArray.type[i].z & 1;
			z_PMT = HitArray.type[i].z / 2;
			xy_PMT = HitArray.type[i].xy;
			if (PMTHits[side][z_PMT][xy_PMT].nHits == 1 && 
				PMTHits[side][z_PMT][xy_PMT].E > 2 && PMTHits[side][z_PMT][xy_PMT].E < 6) {
				h[side][z_PMT][xy_PMT][PMTHits[side][z_PMT][xy_PMT].z][PMTHits[side][z_PMT][xy_PMT].xy]->Fill(HitArray.E[i] / PMTHits[side][z_PMT][xy_PMT].E);
				hAvr[PMTHits[side][z_PMT][xy_PMT].z][PMTHits[side][z_PMT][xy_PMT].xy]->Fill(HitArray.E[i] / PMTHits[side][z_PMT][xy_PMT].E);
				if (side) {
					hMapPMTX->Fill(z_PMT, xy_PMT);
					hMapSiPMX->Fill(5 * xy_PMT + PMTHits[side][z_PMT][xy_PMT].xy, 
						10 * z_PMT + PMTHits[side][z_PMT][xy_PMT].z);
				} else {
					hMapPMTY->Fill(z_PMT, xy_PMT);
					hMapSiPMY->Fill(5 * xy_PMT + PMTHits[side][z_PMT][xy_PMT].xy, 
						10 * z_PMT + PMTHits[side][z_PMT][xy_PMT].z);
				}
				hMapSiPM->Fill(PMTHits[side][z_PMT][xy_PMT].xy, PMTHits[side][z_PMT][xy_PMT].z);
//				printf("%d %d => %c %d %d : %d %d\n", HitArray.type[i].z, HitArray.type[i].xy, (side) ? 'X' : 'Y', z_PMT, xy_PMT,
//					PMTHits[side][z_PMT][xy_PMT].z, PMTHits[side][z_PMT][xy_PMT].xy);
			}
		}
	}
	
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<10; m++) for (n=0; n<5; n++) 
		if (h[j][k][l][m][n]->GetEntries()) h[j][k][l][m][n]->Write();
	for (m=0; m<10; m++) for (n=0; n<5; n++) 
		if (hAvr[m][n]->GetEntries()) hAvr[m][n]->Write();
	hMapPMTX->Write();
	hMapPMTY->Write();
	hMapSiPM->Write();
	hMapSiPMX->Write();
	hMapSiPMY->Write();
	fOut->Close();
	
	return 0;
}
