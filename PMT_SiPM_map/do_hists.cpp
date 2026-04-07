/***********************************************************************************
 * Analyze PMT to SiPM sensitivity ratio due to the photcathode nonuniformity and  *
 * different fiber lengths. Create maps. Use events with single SiPM hit in a PMT. *
 ***********************************************************************************/
#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#include <TChain.h>
#include <TF1.h>
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

int GetNextFromStat(FILE *fStat)
{
	char str[4096];
	char *ptr;
	int run;
	int type;
	
	for(;;) {
		ptr = fgets(str, sizeof(str), fStat);
		if (!ptr) return 1000000;	// like EOF
		ptr = strtok(str, " \t");
		if (!ptr || !isdigit(ptr[0])) continue;
		run = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr || !isdigit(ptr[0])) continue;
		type = strtol(ptr, NULL, 10);
		if (type == 2 || type == 3 || type == 4) return run;
	}
}

int main(int argc, char **argv)
{
	const char *stat_all = "/home/itep/alekseev/igor/stat_all.txt";
	struct DanssEventStruct7 DanssEvent;
	struct HitStruct HitArray;
	char *ptr;
	char str[1024];
	char strl[4096];
	FILE *fList;
	TChain *EventChain;
	long iEvt, nEvt;
	TFile *fOut;
	int iFirst, iLast, iStat;
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
	TH2D *hMapSiPMY;
	TH2D *hEFit[2][5][5];	// Gaus fit values per PMT
	TH2D *hAvrEFit;		// Average Gaus fit values
	TH2D *hEAvr[2][5][5];	// Average ratio values per PMT
	TH2D *hAvrEAvr;		// Average map of average values
	double mean, sigma, low, high;

//			Check number of arguments
	if (argc < 3) {
		printf("Usage: %s list_file.txt|input_file.root|first:last output_file.root\n", argv[0]);
		printf("Will process file(s) and create root-file\n");
		printf("list_file.txt - take files from this list.\n");
		printf("input_file.root - process just this file.\n");
		printf("first:last - process files in the range. Only regular\n");
		printf("files according to stat_all.txt are taken.\n");
		return 10;
	}
//			The first argument must be a single root file or a list or a range
	iStat = iFirst = iLast = -1;
	ptr = strrchr(argv[1], '.');
	if (!ptr) {
		ptr = strchr(argv[1], ':');
		if (!ptr || !isdigit(argv[1][0]) || !isdigit(ptr[1])) {
			printf("Strange argument: .txt, .root or range are expected\n");
			return 15;
		}
		iFirst = strtol(argv[1], NULL, 10);
		iLast = strtol(&ptr[1], NULL, 10);
		if (iLast < iFirst) {
			printf("Bad file range: %d:%d\n", iFirst, iLast);
			return 16;
		}
	}
//			Create Input chains
	EventChain = new TChain("DanssEvent");
//			Add files to input chains
	if (iFirst > 0) {
		fList = fopen(stat_all, "rt");
		if (!fList) {
			printf("Can not open stat_all: %s : %m\n", stat_all);
			return 17;
		}
		for (i = iFirst; i <= iLast; i++) {
			while (iStat < i) iStat = GetNextFromStat(fList);
			if (i == iStat) {
				sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n7/%3.3dxxx/danss_%6.6d.root", i / 1000, i);
				EventChain->Add(str);
			}
		}
		fclose(fList);
	} else if (!strcmp(ptr, ".txt")) {
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
		printf("Strange argument: .txt, .root or range are expected\n");
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
		h[j][k][l][m][n] = new TH1D(str, strl, 116, 0.1, 3);
	}
	for (m=0; m<10; m++) for (n=0; n<5; n++) {
		sprintf(str, "hRatio_Avr_z%dxy%d", m , n);
		sprintf(strl, "Average PMT/SiPM SiPM z%dxy%d", m , n);
		hAvr[m][n] = new TH1D(str, strl, 116, 0.1, 3);
	}

	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) {
		sprintf(str, "hEAvr_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
		sprintf(strl, "Average Ratio map for side %c PMT z%dxy%d", (j) ? 'X' : 'Y', k, l);
		hEAvr[j][k][l] = new TH2D(str, strl, 5, 0, 5, 10, 0, 10);
		sprintf(str, "hEFit_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
		sprintf(strl, "Fit Ratio map for side %c PMT z%dxy%d", (j) ? 'X' : 'Y', k, l);
		hEFit[j][k][l] = new TH2D(str, strl, 5, 0, 5, 10, 0, 10);
	}
	
	hMapPMTX = new TH2D("hMapPMTX", "PMT fill map XZ", 5, 0, 5, 5, 0, 5);
	hMapPMTY = new TH2D("hMapPMTY", "PMT fill map YZ", 5, 0, 5, 5, 0, 5);
	hMapSiPM = new TH2D("hMapSiPM", "SiPM fill map XYZ", 5, 0, 5, 10, 0, 10);
	hMapSiPMX = new TH2D("hMapSiPMX", "SiPM fill map XZ", 25, 0, 25, 50, 0, 50);
	hMapSiPMY = new TH2D("hMapSiPMY", "SiPM fill map YZ", 25, 0, 25, 50, 0, 50);
	hAvrEFit = new TH2D("hAvrEFit", "Fit ratio map over all PMTs", 5, 0, 5, 10, 0, 10);
	hAvrEAvr = new TH2D("hAvrEAvr", "Averge ratio map over all PMTs", 5, 0, 5, 10, 0, 10);

	for (iEvt =0; iEvt < nEvt; iEvt++) {
		EventChain->GetEntry(iEvt);
	// Remove muons
		if (DanssEvent.SiPmCleanEnergy + DanssEvent.PmtCleanEnergy > 40) continue;
		if (DanssEvent.VetoCleanEnergy > 4 || DanssEvent.VetoCleanHits > 1) continue;
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
				PMTHits[side][z_PMT][xy_PMT].E > 1.5 && PMTHits[side][z_PMT][xy_PMT].E < 4) {
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
		if (h[j][k][l][m][n]->GetEntries() > 1000) {
		mean = h[j][k][l][m][n]->GetMean();
		sigma = h[j][k][l][m][n]->GetMeanError();
		hEAvr[j][k][l]->SetBinContent(n+1, m+1, mean);
		hEAvr[j][k][l]->SetBinError(n+1, m+1, sigma);
		h[j][k][l][m][n]->Fit("gaus", "0", "", 0.2, 1.6);
		mean = h[j][k][l][m][n]->GetFunction("gaus")->GetParameter(1);
		sigma = h[j][k][l][m][n]->GetFunction("gaus")->GetParameter(2);
		low  = mean - sigma;
		high = mean + sigma;
		if (low < 0.1) low = 0.1;
		if (high > 2) high = 2;
		h[j][k][l][m][n]->Fit("gaus", "0", "", low, high);
		mean = h[j][k][l][m][n]->GetFunction("gaus")->GetParameter(1);
		sigma = h[j][k][l][m][n]->GetFunction("gaus")->GetParError(1);
		hEFit[j][k][l]->SetBinContent(n+1, m+1, mean);
		hEFit[j][k][l]->SetBinError(n+1, m+1, sigma);
	}

	for (m=0; m<10; m++) for (n=0; n<5; n++) if (hAvr[m][n]->GetEntries() > 1000) {
		mean = hAvr[m][n]->GetMean();
		sigma = hAvr[m][n]->GetMeanError();
		hAvrEAvr->SetBinContent(n+1, m+1, mean);
		hAvrEAvr->SetBinError(n+1, m+1, sigma);
		hAvr[m][n]->Fit("gaus", "0", "", 0.2, 1.6);
		mean = hAvr[m][n]->GetFunction("gaus")->GetParameter(1);
		sigma = hAvr[m][n]->GetFunction("gaus")->GetParameter(2);
		low  = mean - sigma;
		high = mean + sigma;
		if (low < 0.1) low = 0.1;
		if (high > 2) high = 2;
		hAvr[m][n]->Fit("gaus", "0", "", low, high);
		mean = hAvr[m][n]->GetFunction("gaus")->GetParameter(1);
		sigma = hAvr[m][n]->GetFunction("gaus")->GetParError(1);
		hAvrEFit->SetBinContent(n+1, m+1, mean);
		hAvrEFit->SetBinError(n+1, m+1, sigma);
	}
	
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<10; m++) for (n=0; n<5; n++) 
		if (h[j][k][l][m][n]->GetEntries()) h[j][k][l][m][n]->Write();
	for (m=0; m<10; m++) for (n=0; n<5; n++) 
		if (hAvr[m][n]->GetEntries()) hAvr[m][n]->Write();
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) hEFit[j][k][l]->Write();
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) hEAvr[j][k][l]->Write();
	hMapPMTX->Write();
	hMapPMTY->Write();
	hMapSiPM->Write();
	hMapSiPMX->Write();
	hMapSiPMY->Write();
	hAvrEAvr->Write();
	hAvrEFit->Write();
	
	fOut->Close();
	
	return 0;
}
