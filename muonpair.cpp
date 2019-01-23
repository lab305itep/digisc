/********************************************************************************************************
 *	DANSS data analysis - build time correlated pairs and random pairs for muon driven events	*
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

#include "danssGlobals.h"
#include "evtbuilder.h"

#define GFREQ2US	(GLOBALFREQ / 1000000.0)
#define MAXPHITS	10
#define NRANDOM		16	// increase random statistics
#define CORR_P0		0.179	// cluster energy correction from MC
#define CORR_P1		0.929	// cluster energy correction from MC
#define CORR_PMT_P0	0.165	// cluster energy correction from MC
#define CORR_PMT_P1	0.929	// cluster energy correction from MC
#define CORR_SIPM_P0	0.187	// cluster energy correction from MC + 34 keV from SiPM to PMT comparison
#define CORR_SIPM_P1	0.920	// cluster energy correction from MC

#define iMaxDataElements 3000

struct HitStruct {
	float			E[iMaxDataElements];
	float			T[iMaxDataElements];
	struct HitTypeStruct 	type[iMaxDataElements];
};

struct PHits {
	int xy;
	int z;
};

struct CriteriaStruct {
	float TimeWindow;		// us
	float MuonEnergy;		// minimum energy for muon flash
	float VetoEnergy;		// minimum energy for veto counters
	float ClusterEnergy;		// mimimum cluster energy
	float ProtonEnergy;		// minimum single hit energy for knocked out proton
	float ExtraEnergy;		// energy out of delayed cluster
	int CheckHits;			// check hit pattern
} Criteria;

void CopyHits(struct HitStruct *to, struct HitStruct *from, int N)
{
	memcpy(to->E, from->E, N * sizeof(float));
	memcpy(to->T, from->T, N * sizeof(float));
	memcpy(to->type, from->type, N * sizeof(struct HitTypeStruct));
}

int IsVeto(struct DanssEventStruct6 *DanssEvent)
{
	float E;
	int rc;
	
	E = (DanssEvent->SiPmCleanEnergy + DanssEvent->PmtCleanEnergy) / 2;
	rc = (E >= 20) || (DanssEvent->VetoCleanEnergy >= 4) || (DanssEvent->VetoCleanHits >= 2);
	
	return rc;
}

int IsMuon(struct DanssEventStruct6 *DanssEvent, struct HitStruct *Hits, int *NPHits, struct PHits *PHits)
{
	float E;
	int i, j, rc;
	
	E = (DanssEvent->SiPmCleanEnergy + DanssEvent->PmtCleanEnergy) / 2;
	if (E < Criteria.MuonEnergy && DanssEvent->VetoCleanEnergy < Criteria.VetoEnergy) return 0;
	if (DanssEvent->PmtCleanEnergy < Criteria.ProtonEnergy) return 0;
	if (!Criteria.CheckHits) return 1;
	*NPHits = 0;
//		find isolated SiPM hits
	for (i=0; i<DanssEvent->NHits; i++) if (Hits->type[i].flag >= 0 && Hits->type[i].type == bSiPm) {
		rc = 0;
		for (j=0; j<DanssEvent->NHits; j++) if (Hits->type[j].flag >= 0 && Hits->type[j].type == bSiPm) { 
			if (Hits->type[j].z == Hits->type[i].z && abs(Hits->type[j].xy - Hits->type[i].xy) == 1) rc++;
			if (abs(Hits->type[j].z - Hits->type[i].z) == 2 && abs(Hits->type[j].xy - Hits->type[i].xy) <= 1) rc++;
		}
		if (!rc && Hits->E[i] >= Criteria.ProtonEnergy) {
			if (*NPHits < MAXPHITS) {
				PHits[*NPHits].xy = Hits->type[i].xy;
				PHits[*NPHits].z  = Hits->type[i].z;
				(*NPHits)++;
			} else {
				return 0;
			}
		}
	}
	return *NPHits;
}

/*	check for 12B production/decay pattern - production recoil proton must be separate 
and decay electron cluster must contain this strip.  */
int PatternCheck(int N0, struct HitStruct *Hits, int N1, struct PHits *PHits)
{
	int i, j, rc;
	rc = 0;
	for (i=0; i<N0; i++) if (Hits->type[i].flag > 0 && Hits->type[i].type == bSiPm) for (j=0; j<N1; j++) 
		if (PHits[j].xy == Hits->type[i].xy && PHits[j].z == Hits->type[i].z) rc++;
	return rc;
}

int IsDelayed(struct DanssEventStruct6 *DanssEvent, struct HitStruct *HitArray, int NPHits, struct PHits *PHits)
{
	if (IsVeto(DanssEvent)) return 0;
	if (DanssEvent->PositronEnergy < Criteria.ClusterEnergy) return 0;
	if (DanssEvent->AnnihilationEnergy > Criteria.ExtraEnergy) return 0;
	if (Criteria.CheckHits && !PatternCheck(DanssEvent->NHits, HitArray, NPHits, PHits)) return 0;
	return 1;
}

void MakePair(
	struct DanssEventStruct6 *DelayedEvent,	// Delayed
	struct DanssEventStruct6 *PromptEvent,	// Muon
	struct DanssMuonStruct   *DanssPair)
{
	double tmp;
	int i;
	
	memset(DanssPair, 0, sizeof(struct DanssMuonStruct));
	
	DanssPair->number[0] = PromptEvent->number;
	DanssPair->number[1] = DelayedEvent->number;
	DanssPair->globalTime[0] = PromptEvent->globalTime;
	DanssPair->globalTime[1] = DelayedEvent->globalTime;
	DanssPair->unixTime = PromptEvent->unixTime;
	DanssPair->runNumber = PromptEvent->runNumber;
	
	DanssPair->SiPmHits[0] = PromptEvent->SiPmCleanHits;
	DanssPair->SiPmEnergy[0] = PromptEvent->SiPmCleanEnergy;
	DanssPair->PmtHits[0] = PromptEvent->PmtCleanHits;
	DanssPair->PmtEnergy[0] = PromptEvent->PmtCleanEnergy;
	DanssPair->VetoHits[0] = PromptEvent->VetoCleanHits;
	DanssPair->VetoEnergy[0] = PromptEvent->VetoCleanEnergy;
	DanssPair->SiPmHits[1] = DelayedEvent->SiPmCleanHits;
	DanssPair->SiPmEnergy[1] = DelayedEvent->SiPmCleanEnergy;
	DanssPair->PmtHits[1] = DelayedEvent->PmtCleanHits;
	DanssPair->PmtEnergy[1] = DelayedEvent->PmtCleanEnergy;
	DanssPair->VetoHits[1] = DelayedEvent->VetoCleanHits;
	DanssPair->VetoEnergy[1] = DelayedEvent->VetoCleanEnergy;

	DanssPair->TotalEnergy = DelayedEvent->TotalEnergy;
	DanssPair->ClusterHits = DelayedEvent->PositronHits;
	DanssPair->ClusterEnergy = DelayedEvent->PositronEnergy;
	memcpy(DanssPair->ClusterX, DelayedEvent->PositronX, sizeof(DelayedEvent->PositronX));
	DanssPair->ClusterSiPmEnergy = DelayedEvent->PositronSiPmEnergy;
	DanssPair->ClusterPmtEnergy = DelayedEvent->PositronPmtEnergy;
	DanssPair->OffClusterHits = DelayedEvent->AnnihilationGammas;
	DanssPair->OffClusterEnergy = DelayedEvent->AnnihilationEnergy;
	
	DanssPair->MuonEnergy = (PromptEvent->SiPmCleanEnergy + PromptEvent->PmtCleanEnergy) / 2;
	
	DanssPair->gtDiff = (DelayedEvent->globalTime - PromptEvent->globalTime) / GFREQ2US;

	DanssPair->NPHits = PromptEvent->NHits;
	DanssPair->NDHits = DelayedEvent->NHits;
	
	DanssPair->ClusterEnergy = (DanssPair->ClusterEnergy - CORR_P0) / CORR_P1;
	DanssPair->ClusterPmtEnergy = (DanssPair->ClusterPmtEnergy - CORR_PMT_P0) / CORR_PMT_P1;
	DanssPair->ClusterSiPmEnergy = (DanssPair->ClusterSiPmEnergy - CORR_SIPM_P0) / CORR_SIPM_P1;
}

void SetCriteria(void)
{
	char *ptr;
	
	Criteria.TimeWindow = 100000;	// 0.1 s
	Criteria.MuonEnergy = 20;	// MeV
	Criteria.VetoEnergy = 4;	// MeV
	Criteria.ClusterEnergy = 4;	// MeV
	Criteria.ExtraEnergy = 0.25;	// MeV
	Criteria.ProtonEnergy = 1;	// MeV
	Criteria.CheckHits = 1;
	
	ptr = getenv("TIMEWINDOW");
	if (ptr) Criteria.TimeWindow = strtod(ptr, NULL);
	ptr = getenv("MUONENERGY");
	if (ptr) Criteria.MuonEnergy = strtod(ptr, NULL);
	ptr = getenv("VETOENERGY");
	if (ptr) Criteria.VetoEnergy = strtod(ptr, NULL);
	ptr = getenv("CLUSTERENERGY");
	if (ptr) Criteria.ClusterEnergy = strtod(ptr, NULL);
	ptr = getenv("CHECKHITS");
	if (ptr) Criteria.CheckHits = strtol(ptr, NULL, 0);
	ptr = getenv("EXTRAENERGY");
	if (ptr) Criteria.ExtraEnergy = strtod(ptr, NULL);
	ptr = getenv("PROTONENERGY");
	if (ptr) Criteria.ProtonEnergy = strtod(ptr, NULL);
	
	printf("Criteria.TimeWindow = %f us; Criteria.MuonEnergy = %f MeV; Criteria.ClusterEnergy = %f MeV; Criteria.ProtonEnergy = %f MeV\n",
		Criteria.TimeWindow, Criteria.MuonEnergy, Criteria.ClusterEnergy, Criteria.ProtonEnergy);
	printf("Criteria.VetoEnergy = %f MeV; Criteria.CheckHits = %d; Criteria.ExtraEnergy = %f MeV\n",
		Criteria.VetoEnergy, Criteria.CheckHits, Criteria.ExtraEnergy);
}

int main(int argc, char **argv)
{
	const char LeafList[] = 
		"number[2]/L:"		// event numbers in the file
		"globalTime[2]/L:"	// global times
		"unixTime/I:"		// linux time, seconds
		"runNumber/I:"		// run number
		
		"SiPmHits[2]/I:"	// SiPm clean hits
		"SiPmEnergy[2]/F:"	// Full Clean energy SiPm
		"PmtHits[2]/I:"		// Pmt clean hits
		"PmtEnergy[2]/F:"	// Full Clean energy Pmt
		"VetoHits[2]/I:"	// hits in Veto counters
		"VetoEnergy[2]/F:"	// Energy in Veto counters

		"TotalEnergy/F:"	// Total energy long. corrected
		"ClusterHits/I:"	// hits in the cluster
		"ClusterEnergy/F:"	// Energy sum of the cluster (SiPM)
		"ClusterSiPmEnergy/F:"	// SiPM energy in the cluster, corrected
		"ClusterPmtEnergy/F:"	// PMT energy in the cluster, corrected
		"ClusterX[3]/F:"	// cluster position
		"OffClusterHits/I:"	// number of possible annihilation gammas
		"OffClusterEnergy/F:"	// Energy in annihilation gammas
//		"neutron" parameters
		"MuonEnergy/F:"		// Energy sum of above (SiPM)
//		Pair parameters
		"gtDiff/F:"		// time difference in us (from 125 MHz clock)
//		Hits
		"NPHits/I:"		// Number of hits in "positron event"
		"NDHits/I";		// Number of hits in "neutron event"
	
	struct DanssMuonStruct		DanssPair;
	struct DanssEventStruct6	DanssEvent;
	struct DanssEventStruct6	Muon;
	struct DanssInfoStruct4		DanssInfo;
	struct DanssInfoStruct		SumInfo;
	struct HitStruct		HitArray[2];	// 0 - muon, 1 - decay
	struct PHits			PHits[10];
	int NPHits;

	TChain *EventChain;
	TChain *InfoChain;
	TTree *tOut;
	TTree *tRandom;
	TTree *InfoOut;
	TFile *fOut;
	FILE *fList;
	char str[1024];
	long long iEvt, nEvt;
	int PairCnt[2];
	int i;
	int iLoop;
	float tShift;
	char *ptr;
	
	SetCriteria();
	if (argc < 3) {
		printf("Usage: %s list_file.txt|input_file.root output_file.root\n", argv[0]);
		printf("Will process files in the list_file and create root-file\n");
		return 10;
	}

	fOut = new TFile(argv[2], "RECREATE");
	if (!fOut->IsOpen()) {
		printf("Can not open the output file %s: %m\n", argv[2]);
		return -10;
	}

	tOut = new TTree("MuonPair", "Time Correlated muon driven events");
	tOut->Branch("Pair", &DanssPair, LeafList);
	tOut->Branch("PHitE", HitArray[0].E, "PHitE[NPHits]/F");
	tOut->Branch("PHitT", HitArray[0].T, "PHitT[NPHits]/F");
	tOut->Branch("PHitType", HitArray[0].type, "PHitType[NPHits]/I");
	tOut->Branch("DHitE", HitArray[1].E, "DHitE[NDHits]/F");
	tOut->Branch("DHitT", HitArray[1].T, "DHitT[NDHits]/F");
	tOut->Branch("DHitType", HitArray[1].type, "DHitType[NDHits]/I");

	tRandom = new TTree("MuonRandom", "Random coincidence events");
	tRandom->Branch("Pair", &DanssPair, LeafList);
	tRandom->Branch("PHitE", HitArray[0].E, "PHitE[NPHits]/F");
	tRandom->Branch("PHitT", HitArray[0].T, "PHitT[NPHits]/F");
	tRandom->Branch("PHitType", HitArray[0].type, "PHitType[NPHits]/I");
	tRandom->Branch("DHitE", HitArray[1].E, "DHitE[NDHits]/F");
	tRandom->Branch("DHitT", HitArray[1].T, "DHitT[NDHits]/F");
	tRandom->Branch("DHitType", HitArray[1].type, "DHitType[NDHits]/I");

	InfoOut = new TTree("SumInfo", "Summary information");
	InfoOut->Branch("Info", &SumInfo,  
		"gTime/L:"		// running time in terms of 125 MHz
		"startTime/I:"		// linux start time, seconds
		"stopTime/I:"		// linux stop time, seconds
		"events/L"		// number of events
	);
	memset(&SumInfo, 0, sizeof(struct DanssInfoStruct));

	EventChain = new TChain("DanssEvent");
	EventChain->SetBranchAddress("Data", &DanssEvent);
	EventChain->SetBranchAddress("HitE", &HitArray[1].E);
	EventChain->SetBranchAddress("HitT", &HitArray[1].T);
	EventChain->SetBranchAddress("HitType", &HitArray[1].type);
	InfoChain = new TChain("DanssInfo");
	InfoChain->SetBranchAddress("Info", &DanssInfo);

	ptr = strrchr(argv[1], '.');
	if (!ptr) {
		printf("Strange file extention: .txt or .root expected\n");
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
			InfoChain->Add(str);
		}
		fclose(fList);
	} else if (!strcmp(ptr, ".root")) {
		EventChain->Add(argv[1]);
		InfoChain->Add(argv[1]);
	} else {
		printf("Strange file extention: .txt or .root expected\n");
		goto fin;
	}

	nEvt = EventChain->GetEntries();
	memset(PairCnt, 0, sizeof(PairCnt));
	Muon.globalTime = -GLOBALFREQ * 1000;	// some large number
	for (iEvt =0; iEvt < nEvt; iEvt++) {
//	We try to get all available pairs of Muon-Delayed event in the requested time slot
//	1. Look for muon 
//	2. Look for all possible delayed events
//	3. Go to the next muon
		EventChain->GetEntry(iEvt);
//	Get Muon
		if (IsMuon(&DanssEvent, &HitArray[1], &NPHits, PHits)) {
			memcpy(&Muon, &DanssEvent, sizeof(struct DanssEventStruct6));
			CopyHits(&HitArray[0], &HitArray[1], DanssEvent.NHits);
//	Look for delayed event
			for (i = iEvt + 1; i < nEvt; i++) {
				EventChain->GetEntry(i);
				if (DanssEvent.globalTime - Muon.globalTime >= Criteria.TimeWindow * GFREQ2US
					|| DanssEvent.globalTime - Muon.globalTime <= 0) break;
				if (IsDelayed(&DanssEvent, &HitArray[1], NPHits, PHits)) {
					MakePair(&DanssEvent, &Muon, &DanssPair);
					tOut->Fill();
					PairCnt[0]++;
				}
			}
//	Look for random event - delayed event before
			for (i = iEvt - 1; i >= 0; i--) {
				EventChain->GetEntry(i);
				if (Muon.globalTime - DanssEvent.globalTime >= Criteria.TimeWindow * NRANDOM * GFREQ2US
					|| Muon.globalTime - DanssEvent.globalTime <= 0) break;
				if (IsDelayed(&DanssEvent, &HitArray[1], NPHits, PHits)) {
					MakePair(&DanssEvent, &Muon, &DanssPair);
					iLoop = -DanssPair.gtDiff / Criteria.TimeWindow;
					DanssPair.gtDiff += (iLoop + 1) * Criteria.TimeWindow;
					tRandom->Fill();
					PairCnt[1]++;
				}
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

	printf("%Ld events processed with %d randomizing loops - %d/%d pairs found. Aquired time %f7.0 s\n", 
		iEvt, NRANDOM, PairCnt[0], PairCnt[1], SumInfo.upTime / GLOBALFREQ);
fin:
	delete EventChain;
	delete InfoChain;

	InfoOut->Write();
	tOut->Write();
	tRandom->Write();
	fOut->Close();
	return 0;
}
