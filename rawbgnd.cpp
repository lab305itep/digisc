/********************************************************************************************************
 *	DANSS data analysis - understanding of backgrounds with account to veto				*
 ********************************************************************************************************/

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "Riostream.h"
#include "TROOT.h"
#include "TMath.h"
#include "TFile.h"
#include "TChain.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TStyle.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TTreeCacheUnzip.h"
#include "TRandom.h"
#include "TDirectory.h"
#include "TProcessID.h"
#include "TObject.h"
#include "TClonesArray.h"
#include "TRefArray.h"
#include "TRef.h"
#include "TKey.h"
#include "TGraph.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"

#include "evtbuilder.h"

#define GFREQ2US	(GLOBALFREQ / 1000000.0)
#define TVETO		((long long)(60 * GFREQ2US))		// us
#define MINVETOE	4
#define MINVETON	2
#define MINDANSSE	20

#define TYPE_SIPM	0

#define iMaxDataElements 3000

struct HitStruct {
	float			E[iMaxDataElements];
	float			T[iMaxDataElements];
	struct HitTypeStruct 	type[iMaxDataElements];
};

void CheckSignatures(struct DanssEventStruct6 *Event, struct HitStruct *Array, long long *lastTime)
{
	int i;
	int top2hits, edge2hits;
	
	lastTime[0] = Event->globalTime;
	
	if (Event->VetoCleanEnergy >= MINVETOE || Event->VetoCleanHits >= MINVETON) lastTime[1] = Event->globalTime;
	if (Event->SiPmCleanEnergy + Event->PmtCleanEnergy >= 2 * MINDANSSE) lastTime[2] = Event->globalTime;
	
	top2hits = edge2hits = 0;
	for (i=0; i<Event->NHits; i++) if (Array->type[i].type == TYPE_SIPM) {
		if (Array->type[i].z >= 98) top2hits++;
		if (Array->type[i].z >= 98 || Array->type[i].z <= 1 || Array->type[i].xy == 0 || Array->type[i].xy == 24 ) edge2hits++;
	}
	
	if (top2hits) lastTime[3] = Event->globalTime;
	if (edge2hits) lastTime[4] = Event->globalTime;
}

int main(int argc, char **argv)
{
	struct DanssEventStruct6	DanssEvent;
	struct HitStruct 		HitArray; 
	long long lastTime[5];	// 0 - previous trigger; 1 - previous hit VETO; 2 - previous 20 MeV; 3 - previous 2 top layers; 4 - previous outer 2 layers
	TChain *EventChain;
	TFile *fOut;
	FILE *fList;
	char str[1024];
	long long iEvt, nEvt;
	int i;
	char *ptr;
	TH1D *h[18];
	
	if (argc < 3) {
		printf("Usage: %s list_file.txt|input_file.root output_file.root\n", argv[0]);
		printf("Will process file(s in the list_file) and create fixed set of histogramms\n");
		return 10;
	}

	fOut = new TFile(argv[2], "RECREATE");
	if (!fOut->IsOpen()) {
		printf("Can not open the output file %s: %m\n", argv[2]);
		return -10;
	}

	h[0] = new TH1D("hAllc", "All events;E_{clust}", 100, 0, 20);
	h[1] = new TH1D("hAllt", "All events;E_{total}", 100, 0, 20);
	h[2] = new TH1D("hVetoc", "Events tagged by veto counters;E_{clust}", 100, 0, 20);
	h[3] = new TH1D("hVetot", "Events tagged by veto counters;E_{total}", 100, 0, 20);
	h[4] = new TH1D("hNoVetoc", "Events NOT tagged by veto counters;E_{clust}", 100, 0, 20);
	h[5] = new TH1D("hNoVetot", "Events NOT tagged by veto counters;E_{total}", 100, 0, 20);
	h[6] = new TH1D("h20c", "Events tagged by 20 Mev in DANSS;E_{clust}", 100, 0, 20);
	h[7] = new TH1D("h20t", "Events tagged by 20 Mev in DANSS counters;E_{total}", 100, 0, 20);
	h[8] = new TH1D("hNo20c", "Events NOT tagged by  20 Mev in DANSS;E_{clust}", 100, 0, 20);
	h[9] = new TH1D("hNo20t", "Events NOT tagged by  20 Mev in DANSS;E_{total}", 100, 0, 20);
	h[10] = new TH1D("h2topc", "Events tagged by two top layers;E_{clust}", 100, 0, 20);
	h[11] = new TH1D("h2topt", "Events tagged by two top layers;E_{total}", 100, 0, 20);
	h[12] = new TH1D("hNo2topc", "Events NOT tagged by two top layers;E_{clust}", 100, 0, 20);
	h[13] = new TH1D("hNo2topt", "Events NOT tagged by two top layers;E_{total}", 100, 0, 20);
	h[14] = new TH1D("h2edgec", "Events tagged by two edge layers;E_{clust}", 100, 0, 20);
	h[15] = new TH1D("h2edget", "Events tagged by two edge layers;E_{total}", 100, 0, 20);
	h[16] = new TH1D("hNo2edgec", "Events NOT tagged by two edge layers;E_{clust}", 100, 0, 20);
	h[17] = new TH1D("hNo2edget", "Events NOT tagged by two edge layers;E_{total}", 100, 0, 20);

	EventChain = new TChain("DanssEvent");
	EventChain->SetBranchAddress("Data", &DanssEvent);
	EventChain->SetBranchAddress("HitE", &HitArray.E);
	EventChain->SetBranchAddress("HitT", &HitArray.T);
	EventChain->SetBranchAddress("HitType", &HitArray.type);

	ptr = strrchr(argv[1], '.');
	if (!ptr) {
		printf("No file extention: .txt or .root expected\n");
		goto fin;
	}

	if (!strcmp(ptr, ".txt")) {

		fList = fopen(argv[1], "rt");
		if (!fList) {
			printf("Can not open list of files %s: %m\n", argv[1]);
			goto fin;
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
		printf("Strange file extention %s: .txt or .root expected\n");
		goto fin;
	}

	nEvt = EventChain->GetEntries();
	memset(lastTime, -1, sizeof(lastTime));

	for (iEvt =0; iEvt < nEvt; iEvt++) {
		EventChain->GetEntry(iEvt);
		CheckSignatures(&DanssEvent, &HitArray, lastTime);
		h[0]->Fill(DanssEvent.PositronEnergy);
		h[1]->Fill(DanssEvent.TotalEnergy);
		for (i=0; i<4; i++) {
			if (DanssEvent.globalTime - lastTime[i+1] > TVETO) {
				h[4*i+4]->Fill(DanssEvent.PositronEnergy);
				h[4*i+5]->Fill(DanssEvent.TotalEnergy);
			} else if (DanssEvent.globalTime != lastTime[i+1]) {
				h[4*i+2]->Fill(DanssEvent.PositronEnergy);
				h[4*i+3]->Fill(DanssEvent.TotalEnergy);
			}
		}
	}
fin:
	delete EventChain;
	fOut->cd();
	for (i=0; i<sizeof(h) / sizeof(h[0]); i++) h[i]->Write();
	fOut->Close();
	return 0;
}
