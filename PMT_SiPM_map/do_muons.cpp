/***********************************************************************************
 * Analyze PMT to SiPM sensitivity ratio due to the photcathode nonuniformity and  *
 * different fiber lengths. Create maps. Make vertical projection using muons      *
 * located in a single vertical column.						   *
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

struct PMTStruct {
	double E_PMT;
	double E_SiPM;
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
	int i, j, k, l, m;
	int side, z, xy, z_PMT, xy_PMT;
	int muon_xy[2];
	struct PMTStruct PMT[2][5][5];	// hits in PMT, sum SiPM energy
	TH1D *hmu[2][5][5][5];		// side, z_PMT, xy_PMT, xy_SiPM in PMT
	TH1D *hmuAvr[5];		// xy_SiPM in PMT
	TH1D *hmuSiPMEvtX;		// Events per column in X
	TH1D *hmuSiPMEvtY;		// Events per column in Y
	TH1D *hmuEFit[2][5][5];		// Gaus fit values per PMT
	TH1D *hmuAvrEFit;		// Average Gaus fit values
	TH1D *hmuEAvr[2][5][5];		// Average ratio values per PMT
	TH1D *hmuAvrEAvr;		// Average map of average values
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
	
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<5; m++) {
		sprintf(str, "hmuRatio_%c_z%dxy%d_c%d", (j) ? 'X' : 'Y', k, l, m );
		sprintf(strl, "PMT/SiPM for side %c PMT z%dxy%d column %d", (j) ? 'X' : 'Y', k, l, m);
		hmu[j][k][l][m] = new TH1D(str, strl, 116, 0.1, 3);
	}
	for (m=0; m<5; m++) {
		sprintf(str, "hmuRatio_Avr_c%d", m);
		sprintf(strl, "Average PMT/SiPM column %d", m);
		hmuAvr[m] = new TH1D(str, strl, 116, 0.1, 3);
	}

	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) {
		sprintf(str, "hmuEAvr_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
		sprintf(strl, "Average muon Ratio map for side %c PMT z%dxy%d", (j) ? 'X' : 'Y', k, l);
		hmuEAvr[j][k][l] = new TH1D(str, strl, 5, 0, 5);
		sprintf(str, "hmuEFit_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
		sprintf(strl, "Fit muon Ratio map for side %c PMT z%dxy%d", (j) ? 'X' : 'Y', k, l);
		hmuEFit[j][k][l] = new TH1D(str, strl, 5, 0, 5);
	}
	
	hmuSiPMEvtX = new TH1D("hSiPMEvtX", "Column fill statistics X", 25, 0, 25);
	hmuSiPMEvtY = new TH1D("hSiPMEvtY", "Column fill statistics Y", 25, 0, 25);
	hmuAvrEFit = new TH1D("hmuAvrEFit", "Muon fit ratio map over all PMTs", 5, 0, 5);
	hmuAvrEAvr = new TH1D("hmuAvrEAvr", "Muon averge ratio map over all PMTs", 5, 0, 5);

	for (iEvt =0; iEvt < nEvt; iEvt++) {
		EventChain->GetEntry(iEvt);
	// We expect > 150 MeV energy deposit, cut @ 50 MeV
		if (DanssEvent.SiPmCleanEnergy + DanssEvent.PmtCleanEnergy < 100) continue;
	// We need exactly 10 hits in PMT
		if (DanssEvent.PmtCleanHits != 10) continue;
	// We need at least 90 hits in SiPM (we allow some amount of dead channels)
		if (DanssEvent.SiPmCleanHits < 90) continue;
	// Make hit patterns
		memset(PMT, 0, sizeof(PMT));
		muon_xy[0] = muon_xy[1] = -1;
		for (i=0; i<DanssEvent.NHits; i++) switch (HitArray.type[i].type) {
		case bSiPM:
			if (HitArray.E[i] < 0.3) break;		// muon should deposit ~1.6 MeV
			side = HitArray.type[i].z & 1;
			z = HitArray.type[i].z / 2;
			xy = HitArray.type[i].xy;
			z_PMT = HitArray.type[i].z / 20;
			xy_PMT = HitArray.type[i].xy / 5;
			PMT[side][z_PMT][xy_PMT].E_SiPM += HitArray.E[i];	// sum over SiPM energy
			if (muon_xy[side] < 0) {
				muon_xy[side] = xy;
			} else if (muon_xy[side] != xy) {
				muon_xy[side] = 1000;	// this is bad flag
			}
			break;
		case bPMT:
			side = HitArray.type[i].z & 1;
			z_PMT = HitArray.type[i].z / 2;
			xy_PMT = HitArray.type[i].xy;
			PMT[side][z_PMT][xy_PMT].E_PMT = HitArray.E[i];
			break;
		}
	// Find muons, which hit exactly single column in SiPM & PMT - check PMT column
		for (j=0; j<2; j++) if (muon_xy[j] >= 0 && muon_xy[j] < 50) {
			for (k = 0; k<5; k++) if (PMT[j][k][xy / 5].E_PMT < 1) break;	// Expected energy deposit ~18 MeV
			if (k < 5) muon_xy[j] = 2000;	// flag bad
		}
	// Fill Ratio PMT/SiPM for selected events (each side separately)
		for (j=0; j<2; j++) if (muon_xy[j] >= 0 && muon_xy[j] < 50) {
			for (k=0; k<5; k++) {
				hmu[j][k][muon_xy[j]/5][muon_xy[j]%5]->Fill(PMT[j][k][muon_xy[j]/5].E_PMT / PMT[j][k][muon_xy[j]/5].E_SiPM);
				hmuAvr[muon_xy[j]%5]->Fill(PMT[j][k][muon_xy[j]/5].E_PMT / PMT[j][k][muon_xy[j]/5].E_SiPM);
			}
			if (j) {
				hmuSiPMEvtX->Fill(muon_xy[j]);
			} else {
				hmuSiPMEvtY->Fill(muon_xy[j]);
			}
		}
	}
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<5; m++)
		if (hmu[j][k][l][m]->GetEntries() > 1000) {
		mean = hmu[j][k][l][m]->GetMean();
		sigma = hmu[j][k][l][m]->GetMeanError();
		hmuEAvr[j][k][l]->SetBinContent(m+1, mean);
		hmuEAvr[j][k][l]->SetBinError(m+1, sigma);
		hmu[j][k][l][m]->Fit("gaus", "0", "", 0.2, 1.6);
		mean = hmu[j][k][l][m]->GetFunction("gaus")->GetParameter(1);
		sigma = hmu[j][k][l][m]->GetFunction("gaus")->GetParameter(2);
		low  = mean - sigma;
		high = mean + sigma;
		if (low < 0.1) low = 0.1;
		if (high > 2) high = 2;
		hmu[j][k][l][m]->Fit("gaus", "0", "", low, high);
		mean = hmu[j][k][l][m]->GetFunction("gaus")->GetParameter(1);
		sigma = hmu[j][k][l][m]->GetFunction("gaus")->GetParError(1);
		hmuEFit[j][k][l]->SetBinContent(m+1, mean);
		hmuEFit[j][k][l]->SetBinError(m+1, sigma);
	}

	for (m=0; m<5; m++) if (hmuAvr[m]->GetEntries() > 1000) {
		mean = hmuAvr[m]->GetMean();
		sigma = hmuAvr[m]->GetMeanError();
		hmuAvrEAvr->SetBinContent(m+1, mean);
		hmuAvrEAvr->SetBinError(m+1, sigma);
		hmuAvr[m]->Fit("gaus", "0", "", 0.2, 1.6);
		mean = hmuAvr[m]->GetFunction("gaus")->GetParameter(1);
		sigma = hmuAvr[m]->GetFunction("gaus")->GetParameter(2);
		low  = mean - sigma;
		high = mean + sigma;
		if (low < 0.1) low = 0.1;
		if (high > 2) high = 2;
		hmuAvr[m]->Fit("gaus", "0", "", low, high);
		mean = hmuAvr[m]->GetFunction("gaus")->GetParameter(1);
		sigma = hmuAvr[m]->GetFunction("gaus")->GetParError(1);
		hmuAvrEFit->SetBinContent(m+1, mean);
		hmuAvrEFit->SetBinError(m+1, sigma);
	}
	
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<5; m++)
		if (hmu[j][k][l][m]->GetEntries()) hmu[j][k][l][m]->Write();
	for (m=0; m<5; m++) 
		if (hmuAvr[m]->GetEntries()) hmuAvr[m]->Write();
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) hmuEFit[j][k][l]->Write();
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) hmuEAvr[j][k][l]->Write();
	hmuSiPMEvtX->Write();
	hmuSiPMEvtY->Write();
	hmuAvrEAvr->Write();
	hmuAvrEFit->Write();
	
	fOut->Close();
	
	return 0;
}
