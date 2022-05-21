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
//#include "TSpectrum.h"

#include "evtbuilder.h"

#define GFREQ2US	(GLOBALFREQ / 1000000)
#define MAXTDIFF	100.0	// us
#define MINTRIGE	5.0	// MeV
#define MAXTRIGE	20.0	// MeV
#define MINVETOE	4.0	// MeV
#define VETON		2	// number of hits
#define DANSSVETOE	20.0	// Make veto if VETO counters are silent from Pmt or SiPM
#define VETOBLK		100.0	// us
#define DELTA		15.0	// cm

int IsFission(struct DanssEventStruct7 *DanssEvent)
{
	if (DanssEvent->PmtCleanEnergy + DanssEvent->SiPmCleanEnergy < 2*MINTRIGE ||
		DanssEvent->PmtCleanEnergy + DanssEvent->SiPmCleanEnergy > 2*MAXTRIGE) return 0;
	return 1;
}

int IsVeto(struct DanssEventStruct7 *Event)
{
	if (Event->VetoCleanEnergy > MINVETOE || Event->VetoCleanHits >= VETON || 
		Event->PmtCleanEnergy + Event->SiPmCleanEnergy > 2*DANSSVETOE) return 1;
	return 0;
}

void Add2Cm(struct DanssEventStruct7 *DanssEvent, struct DanssCmStruct *DanssCm, int num, long long globalTime)
{
	if (num >= 10) return;
	DanssCm->number[num] = DanssEvent->number;
	if (!num) DanssCm->unixTime = DanssEvent->unixTime;
	DanssCm->SiPmCleanEnergy[num] = DanssEvent->SiPmCleanEnergy;
	DanssCm->PmtCleanEnergy[num] = DanssEvent->PmtCleanEnergy;

	DanssCm->Hits[num] = DanssEvent->SiPmCleanHits;
	DanssCm->NeutronEnergy[num] = (DanssEvent->SiPmCleanEnergy + DanssEvent->PmtCleanEnergy) / 2;
	memcpy(DanssCm->NeutronX[num], DanssEvent->NeutronX, sizeof(DanssEvent->NeutronX));
	memcpy(DanssCm->PositronX[num], DanssEvent->PositronX, sizeof(DanssEvent->PositronX));
	DanssCm->NeutronRadius[num] = -1;
	DanssCm->PositronEnergy[num] = DanssEvent->PositronEnergy;
	
	DanssCm->gtDiff[num] = DanssEvent->globalTime - globalTime;
	DanssCm->Distance[num] = sqrt(
		(DanssCm->NeutronX[num][0] - DanssCm->NeutronX[0][0]) * (DanssCm->NeutronX[num][0] - DanssCm->NeutronX[0][0]) +
		(DanssCm->NeutronX[num][1] - DanssCm->NeutronX[0][1]) * (DanssCm->NeutronX[num][1] - DanssCm->NeutronX[0][1]) +
		(DanssCm->NeutronX[num][2] - DanssCm->NeutronX[0][2]) * (DanssCm->NeutronX[num][2] - DanssCm->NeutronX[0][2])
	);
	DanssCm->DistanceZ[num] = DanssCm->NeutronX[num][2] - DanssCm->NeutronX[0][2];
}

int main(int argc, char **argv)
{
	struct DanssEventStruct7		DanssEvent;
	struct DanssCmStruct			DanssCm;
	struct DanssEventStruct7		SavedEvent;
	struct DanssInfoStruct4			DanssInfo;
	struct DanssInfoStruct			SumInfo;

	TChain *EventChain;
	TChain *InfoChain;
	TTree *tOut;
	TTree *InfoOut;
	TFile *fOut;
	FILE *fList;
	char str[1024];
	long long iEvt, nEvt;
	long long lastgTime, lastVeto;
	int CmCnt;
	int i;
	char *ptr;
	int nCnt;
	
	if (argc < 3) {
		printf("Usage: ./cmbuilder list_file.txt output_file.root\n");
		printf("Will process files in the list_file and create root-file\n");
		return 10;
	}

	fOut = new TFile(argv[2], "RECREATE");
	if (!fOut->IsOpen()) {
		printf("Can not open the output file %s: %m\n", argv[2]);
		return -10;
	}

	tOut = new TTree("DanssCm", "Time Correlated events");
	tOut->Branch("Cm", &DanssCm,
		"number[10]/L:"		// event numbers in the file
		"unixTime/I:"		// linux time, seconds
		"N/I:"			// number of neutrons + 1
		"SiPmCleanEnergy[10]/F:"	// Full Clean energy SiPm
		"PmtCleanEnergy[10]/F:"	// Full Clean energy Pmt
//		"neutron" parameters
		"Hits[10]/I:"		// SiPm clean hits
//		"NeutronHits[10]/I:"	// number of hits considered as neutron capture gammas
		"NeutronEnergy[10]/F:"	// Energy sum of above (SiPM)
		"NeutronX[10][3]/F:"	// center of gammas position
		"PositronX[10][3]/F:"	// maximum hit clusters
//		"NeutronGammaEnergy[10][5]/F:"	// sorted list of the 5 most energetic gammas
//		"NeutronGammaDistance[10][5]/F:"	// distances for the gammas above to the "neutron" center
		"PositronEnergy[10]/F:"	// maximum hit clusters
		"NeutronRadius[10]/F:"	// average distance between hits and the center
//		Pair parameters
		"gtDiff[10]/F:"		// time difference in us (from 125 MHz clock)
		"Distance[10]/F:"		// distance between neutron and positron, cm
		"DistanceZ[10]/F"		// in Z, cm
	);
	
	InfoOut = new TTree("SumInfo", "Summary information");
	InfoOut->Branch("Info", &SumInfo,  
		"gTime/L:"		// running time in terms of 125 MHz
		"startTime/I:"		// linux start time, seconds
		"stopTime/I:"		// linux stop time, seconds
		"events/I"		// number of events
	);
	memset(&SumInfo, 0, sizeof(struct DanssInfoStruct4));
	memset(&DanssCm, 0, sizeof(struct DanssCmStruct));

	EventChain = new TChain("DanssEvent");
	EventChain->SetBranchAddress("Data", &DanssEvent);
	InfoChain = new TChain("DanssInfo");
	InfoChain->SetBranchAddress("Info", &DanssInfo);

	fList = fopen(argv[1], "rt");
	if (!fList) {
		printf("Can not open list of files %s: %m\n", argv[1]);
		goto fin;
	}
	
	for(i=0;;i++) {
		if (!fgets(str, sizeof(str), fList)) break;
		ptr = strchr(str, '\n');
		if (ptr) *ptr = '\0';
		EventChain->Add(str);
		InfoChain->Add(str);
	}
	fclose(fList);

	nEvt = EventChain->GetEntries();
	printf("%d files with %d triggers\n", i, nEvt);
	CmCnt = 0;
	nCnt = 0;
	lastVeto = lastgTime = -GLOBALFREQ;
	for (iEvt =0; iEvt < nEvt; iEvt++) {
		EventChain->GetEntry(iEvt);
		if (IsVeto(&DanssEvent)) {
			lastVeto = DanssEvent.globalTime;
			lastgTime = -GLOBALFREQ;
			nCnt = 0;
		}
		if (DanssEvent.globalTime - lastVeto < VETOBLK * GFREQ2US) {
			continue;	// Veto is active
		}
		if (DanssEvent.globalTime - lastgTime < MAXTDIFF * GFREQ2US) {
			Add2Cm(&DanssEvent, &DanssCm, nCnt, lastgTime);
			nCnt++;
			continue;
		} else {
			if (nCnt) {
				DanssCm.N = nCnt;
				tOut->Fill();
				if (nCnt > 0) CmCnt++;
				nCnt = 0;
				memset(&DanssCm, 0, sizeof(struct DanssCmStruct));			
			}
			if (IsFission(&DanssEvent)) {
				lastgTime = DanssEvent.globalTime;
				Add2Cm(&DanssEvent, &DanssCm, nCnt, lastgTime);
				nCnt++;
			}
		}
	}
	
	for(i=0; i<InfoChain->GetEntries(); i++) {
		InfoChain->GetEntry(i);
		SumInfo.upTime += DanssInfo.upTime;
		SumInfo.stopTime = DanssInfo.stopTime;
		SumInfo.events += DanssInfo.events;
		if (!i) SumInfo.startTime = DanssInfo.startTime;
	}
	InfoOut->Fill();

	printf("%Ld events processed - %d fissions found. Aquired time %f7.0 s\n", iEvt, CmCnt, SumInfo.upTime / GLOBALFREQ);
	printf("Trigger freq = %7.1f Hz\n", 1.0 * iEvt * GLOBALFREQ / SumInfo.upTime);
	printf("Fission freq = %7.1f Hz\n", 1.0 * CmCnt * GLOBALFREQ / SumInfo.upTime);
fin:
	delete EventChain;
	delete InfoChain;

	InfoOut->Write();
	tOut->Write();
	fOut->Close();
	return 0;
}

