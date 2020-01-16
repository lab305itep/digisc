/****************************************************************************************
 *	DANSS data analysis - build time correlated pairs and random pairs		*
 ****************************************************************************************/

#include <libgen.h>
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
#define MAXTDIFF	50.0	// us
#define MINPOSE		0.5	// MeV
#define MAXPOSE		20.0	// MeV
#define AGAMMAN		0	// number of annihilation gamma hits (0 no requirement)
#define MINNEUTE	3.0	// MeV
#define MAXNEUTE	15.0	// MeV
#define NEUTN		3	// number of hits
#define MINVETOE	4.0	// MeV
#define VETON		2	// number of hits
#define DANSSVETOE	20.0	// Make veto if VETO counters are silent from Pmt or SiPM
#define BOTTOMVETOE	3.0	// Make veto from 2 bottom strip layers
#define RSHIFT		5000.0	// us
#define NRANDOM		16	// increase random statistics
//#define ATTENUATION	0.00342	// Signal attenuation for positron energy correction
//	Old calibration
//#define CORR_P0		0.179	// Positron energy correction from MC
//#define CORR_P1		0.929	// Positron energy correction from MC
//#define CORR_PMT_P0	0.165	// Positron energy correction from MC
//#define CORR_PMT_P1	0.929	// Positron energy correction from MC
//#define CORR_SIPM_P0	0.187	// Positron energy correction from MC + 34 keV from SiPM to PMT comparison
//#define CORR_SIPM_P1	0.920	// Positron energy correction from MC
//	New calibration
#define CORR_P0		0.130	// Positron energy correction from MC 
#define CORR_P1		0.976	// Positron energy correction from MC
#define CORR_PMT_P0	0.125	// Positron energy correction from MC
#define CORR_PMT_P1	0.948	// Positron energy correction from MC
#define CORR_SIPM_P0	0.104	// Positron energy correction from MC
#define CORR_SIPM_P1	0.997	// Positron energy correction from MC
#define SHOWERMIN	800	// 800 MeV shower event threshold

#define iMaxDataElements 3000

struct HitStruct {
	float			E[iMaxDataElements];
	float			T[iMaxDataElements];
	struct HitTypeStruct 	type[iMaxDataElements];
};

double PMTYAverageLightColl(double x)
{
    //<func(x)=1>
	const double FuncAverage = 1.00147;
	double rez;
	rez = (0.987387*exp(-0.0016*(x-48)) + 0.023973*exp(-0.0877*(x-48)) - 0.0113581*exp(-0.1042*(x-48))
	        -2.30972E-6*exp(0.2214*(x-48))) / FuncAverage;
	return rez;
}

double SiPMYAverageLightColl(double x)
{
    //<func(x)=1>
	const double FuncAverage = 1.02208;
	double rez;
	rez = (0.00577381*exp(-0.1823*(x-48)) + 0.999583*exp(-0.0024*(x-48)) - 8.095E-13*exp(0.5205*(x-48))
	        -0.00535714*exp(-0.1838*(x-48))) / FuncAverage;
	return rez;
}

//	Correction based on the neutron position if this was not done before based on the positron position
void NeutronCorr(struct DanssPairStruct7 *DanssPair) 
{
	double CSiPm, CPmt;
	if (DanssPair->PositronX[0] < 0 && DanssPair->NeutronX[0] >= 0) {
		CSiPm = SiPMYAverageLightColl(DanssPair->NeutronX[0]);
		CPmt = PMTYAverageLightColl(DanssPair->NeutronX[0]);
	} else if (DanssPair->PositronX[1] < 0 && DanssPair->NeutronX[1] >= 0) {
		CSiPm = SiPMYAverageLightColl(DanssPair->NeutronX[1]);
		CPmt = PMTYAverageLightColl(DanssPair->NeutronX[1]);
	} else {
		CSiPm = 1.0;
		CPmt = 1.0;
	}
	DanssPair->PositronEnergy *= (CSiPm + CPmt) / 2;
	DanssPair->PositronSiPmEnergy *= CSiPm;
	DanssPair->PositronPmtEnergy *= CPmt;
}

void CopyHits(struct HitStruct *to, struct HitStruct *from, int N)
{
	memcpy(to->E, from->E, N * sizeof(float));
	memcpy(to->T, from->T, N * sizeof(float));
	memcpy(to->type, from->type, N * sizeof(struct HitTypeStruct));
}

int IsPickUp(struct DanssEventStruct7 *DanssEvent, struct RawHitInfoStruct *RawHits)
//	"(PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3"
{
	if (DanssEvent->VetoCleanHits > 0) return 0;	// never kill VETO trigger
	if ((RawHits->PmtCnt > 0 && 1.0 * DanssEvent->PmtCleanHits / RawHits->PmtCnt < 0.3) ||
		1.0 * DanssEvent->SiPmHits / RawHits->SiPmCnt < 0.3) return 1;
	return 0;
}

int IsNeutron(struct DanssEventStruct7 *DanssEvent)
{
	float E;
	int rc;
	
	E = (DanssEvent->SiPmCleanEnergy + DanssEvent->PmtCleanEnergy) / 2;
	rc = (E >= MINNEUTE && E < MAXNEUTE && DanssEvent->SiPmCleanHits >= NEUTN);

	return rc;
}

int IsPositron(struct DanssEventStruct7 *DanssEvent)
{
	float E;
	int rc;

	E = DanssEvent->PositronEnergy;
	rc = (E >= MINPOSE && E < MAXPOSE && DanssEvent->AnnihilationGammas >= AGAMMAN);

	return rc;
}

int IsVeto(struct DanssEventStruct7 *Event)
{
	if (Event->VetoCleanEnergy > MINVETOE || Event->VetoCleanHits >= VETON || 
		Event->PmtCleanEnergy + Event->SiPmCleanEnergy > 2*DANSSVETOE ||
		Event->BottomLayersEnergy > BOTTOMVETOE) return 1;
	return 0;
}

int IsShower(struct DanssEventStruct7 *Event)
{
	if (Event->PmtCleanEnergy + Event->SiPmCleanEnergy > 2*SHOWERMIN) return 1;
	return 0;
}

void MakePair(
    struct DanssEventStruct7 *DanssEvent,	// Neutron
    struct DanssEventStruct7 *SavedEvent,	// Positron
    struct DanssEventStruct7 *VetoEvent, 	// Veto
    struct DanssEventStruct7 *ShowerEvent, 	// Shower
    struct DanssPairStruct7 *DanssPair)
{
	double tmp;
	int i;
	
	memset(DanssPair, 0, sizeof(struct DanssPairStruct7));

	DanssPair->number[0] = SavedEvent->number;
	DanssPair->number[1] = DanssEvent->number;
	DanssPair->globalTime[0] = SavedEvent->globalTime;
	DanssPair->globalTime[1] = DanssEvent->globalTime;
	DanssPair->unixTime = DanssEvent->unixTime;
//	DanssPair->runNumber = DanssEvent->runNumber;
	DanssPair->SiPmCleanEnergy[0] = SavedEvent->SiPmCleanEnergy;
	DanssPair->PmtCleanEnergy[0] = SavedEvent->PmtCleanEnergy;
	DanssPair->SiPmCleanEnergy[1] = DanssEvent->SiPmCleanEnergy;
	DanssPair->PmtCleanEnergy[1] = DanssEvent->PmtCleanEnergy;
	
	DanssPair->PositronHits = SavedEvent->PositronHits;
	DanssPair->PositronEnergy = SavedEvent->PositronEnergy;
	memcpy(DanssPair->PositronX, SavedEvent->PositronX, sizeof(SavedEvent->PositronX));
	DanssPair->TotalEnergy = SavedEvent->TotalEnergy;
	DanssPair->PositronSiPmEnergy = SavedEvent->PositronSiPmEnergy;
	DanssPair->PositronPmtEnergy = SavedEvent->PositronPmtEnergy;
	DanssPair->AnnihilationGammas = SavedEvent->AnnihilationGammas;
	DanssPair->AnnihilationEnergy = SavedEvent->AnnihilationEnergy;
	DanssPair->AnnihilationMax = SavedEvent->AnnihilationMax;
	DanssPair->MinPositron2GammaZ = SavedEvent->MinPositron2GammaZ;
	
	DanssPair->NeutronHits = DanssEvent->SiPmCleanHits;
	DanssPair->NeutronEnergy = (DanssEvent->SiPmCleanEnergy + DanssEvent->PmtCleanEnergy) / 2;
	memcpy(DanssPair->NeutronX, DanssEvent->NeutronX, sizeof(DanssEvent->NeutronX));
	
	DanssPair->gtDiff = (DanssEvent->globalTime - SavedEvent->globalTime) / GFREQ2US;
	
	tmp = (DanssEvent->NeutronX[2] - SavedEvent->PositronX[2]) * (DanssEvent->NeutronX[2] - SavedEvent->PositronX[2]);
	for (i=0; i<2; i++) if (DanssEvent->NeutronX[i] >= 0 && SavedEvent->PositronX[i] >= 0) 
		tmp += (DanssEvent->NeutronX[i] - SavedEvent->PositronX[i]) * (DanssEvent->NeutronX[i] - SavedEvent->PositronX[i]);
	DanssPair->Distance = sqrt(tmp);
	DanssPair->DistanceZ = DanssEvent->NeutronX[2] - SavedEvent->PositronX[2];

	DanssPair->gtFromVeto = (SavedEvent->globalTime - VetoEvent->globalTime) / GFREQ2US;
	DanssPair->VetoHits = VetoEvent->VetoCleanHits;
	DanssPair->VetoEnergy = VetoEvent->VetoCleanEnergy;
	DanssPair->DanssEnergy = (VetoEvent->SiPmCleanEnergy + VetoEvent->PmtCleanEnergy) / 2;
	DanssPair->gtFromShower = (SavedEvent->globalTime - ShowerEvent->globalTime) / GFREQ2US;
	DanssPair->ShowerEnergy = (ShowerEvent->SiPmCleanEnergy + ShowerEvent->PmtCleanEnergy) / 2;

	DanssPair->NNHits = DanssEvent->NHits;
	DanssPair->NPHits = SavedEvent->NHits;
	
	NeutronCorr(DanssPair);		// correct positron energy based on neutron position if only one coordinate of positron cluster is available
	DanssPair->PositronEnergy = (DanssPair->PositronEnergy - CORR_P0) / CORR_P1;
	DanssPair->PositronPmtEnergy = (DanssPair->PositronPmtEnergy - CORR_PMT_P0) / CORR_PMT_P1;
	DanssPair->PositronSiPmEnergy = (DanssPair->PositronSiPmEnergy - CORR_SIPM_P0) / CORR_SIPM_P1;
}

int main(int argc, char **argv)
{
	const char LeafList[] = 
		"number[2]/L:"		// event numbers in the file
		"globalTime[2]/L:"	// global times
		"unixTime/I:"		// linux time, seconds
//		"runNumber/I:"		// run number
		"SiPmCleanEnergy[2]/F:"	// Full Clean energy SiPm
		"PmtCleanEnergy[2]/F:"	// Full Clean energy Pmt
//		"positron cluster" parameters
		"PositronHits/I:"	// hits in the cluster
		"PositronEnergy/F:"	// Energy sum of the cluster (SiPM)
		"TotalEnergy/F:"	// Total energy long. corrected
		"PositronSiPmEnergy/F:"	// SiPM energy in the cluster, corrected
		"PositronPmtEnergy/F:"	// PMT energy in the cluster, corrected
		"PositronX[3]/F:"	// cluster position
		"AnnihilationGammas/I:"	// number of possible annihilation gammas
		"AnnihilationEnergy/F:"	// Energy in annihilation gammas
		"AnnihilationMax/F:"	// Max hit energy beyond the cluster
		"MinPositron2GammaZ/F:"	// Z-distance to the closest gamma
//		"neutron" parameters
		"NeutronHits/I:"	// number of hits considered as neutron capture gammas
		"NeutronEnergy/F:"	// Energy sum of above (SiPM)
		"NeutronX[3]/F:"	// center of gammas position
//		Pair parameters
		"gtDiff/F:"		// time difference in us (from 125 MHz clock)
		"Distance/F:"		// distance between neutron and positron, cm
		"DistanceZ/F:"		// in Z, cm
//		Environment
		"gtFromPrevious/F:"	// time from the previous hit before positron, us
		"PreviousEnergy/F:"	// energy of the previous event
		"gtToNext/F:"		// time to the next hit after neutron, counted from positron, us
		"NextEnergy/F:"		// energy of the next event
		"EventsBetween/I:"	// Events between positron and neutron
//		Veto
		"gtFromVeto/F:"		// time from the last Veto event
		"VetoHits/I:"		// hits in Veto counters
		"VetoEnergy/F:"		// Energy in Veto counters
		"DanssEnergy/F:"	// Veto Energy in Danss (Pmt + SiPm)/2
		"gtFromShower/F:"	// time from large energy shower in DANSS
		"ShowerEnergy/F:"	// shower event energy in DANSS (Pmt + SiPm)/2
//		Hits
		"NPHits/I:"		// Number of hits in "positron event"
		"NNHits/I";		// Number of hits in "neutron event"
	
	struct DanssPairStruct7		DanssPair;
	struct DanssEventStruct7	DanssEvent;
	struct DanssEventStruct7	Neutron;
	struct DanssEventStruct7	Positron;
	struct DanssEventStruct7	Veto;
	struct DanssEventStruct7	Shower;
	struct DanssInfoStruct4		DanssInfo;
	struct DanssInfoStruct		SumInfo;
	struct HitStruct 		HitArray[3];	// 0 - positron, 1 - neutron, 2 - place for input
	struct RawHitInfoStruct		RawHits;

	TChain *EventChain = NULL;
	TChain *InfoChain = NULL;
	TChain *RawChain = NULL;
	TTree *tOut;
	TTree *tRandom;
	TTree *InfoOut;
	TFile *fOut;
	FILE *fList;
	char str[1024];
	char strl[1600];
	long long iEvt, nEvt, rEvt;
	int PairCnt[2];
	int PickUpCnt;
	int i;
	int iLoop;
	float tShift;
	char *ptr;
	
	if (argc < 3) {
		printf("Usage: %s list_file.txt|input_file.root output_file.root\n", argv[0]);
		printf("Will process files in the list_file and create root-file\n");
		return 10;
	}

	strncpy(str, argv[2], sizeof(str));
	sprintf(strl, "mkdir -p %s", dirname(str));
	if (system(strl)) {
		printf("Can not crete target directory: %m\n");
		return -5;
	}

	fOut = new TFile(argv[2], "RECREATE");
	if (!fOut->IsOpen()) {
		printf("Can not open the output file %s: %m\n", argv[2]);
		return -10;
	}

	tOut = new TTree("DanssPair", "Time Correlated events");
	tOut->Branch("Pair", &DanssPair, LeafList);
	tOut->Branch("PHitE", HitArray[0].E, "PHitE[NPHits]/F");
	tOut->Branch("PHitT", HitArray[0].T, "PHitT[NPHits]/F");
	tOut->Branch("PHitType", HitArray[0].type, "PHitType[NPHits]/I");
	tOut->Branch("NHitE", HitArray[1].E, "NHitE[NNHits]/F");
	tOut->Branch("NHitT", HitArray[1].T, "NHitT[NNHits]/F");
	tOut->Branch("NHitType", HitArray[1].type, "NHitType[NNHits]/I");

	tRandom = new TTree("DanssRandom", "Random coincidence events");
	tRandom->Branch("Pair", &DanssPair, LeafList);
	tRandom->Branch("PHitE", HitArray[0].E, "PHitE[NPHits]/F");
	tRandom->Branch("PHitT", HitArray[0].T, "PHitT[NPHits]/F");
	tRandom->Branch("PHitType", HitArray[0].type, "PHitType[NPHits]/I");
	tRandom->Branch("NHitE", HitArray[1].E, "NHitE[NNHits]/F");
	tRandom->Branch("NHitT", HitArray[1].T, "NHitT[NNHits]/F");
	tRandom->Branch("NHitType", HitArray[1].type, "NHitType[NNHits]/I");

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
	EventChain->SetBranchAddress("HitE", &HitArray[2].E);
	EventChain->SetBranchAddress("HitT", &HitArray[2].T);
	EventChain->SetBranchAddress("HitType", &HitArray[2].type);
	RawChain = new TChain("RawHits");
	RawChain->SetBranchAddress("RawHits", &RawHits);
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
			RawChain->Add(str);
			InfoChain->Add(str);
		}
		fclose(fList);
	} else if (!strcmp(ptr, ".root")) {
		EventChain->Add(argv[1]);
		if (RawChain) RawChain->Add(argv[1]);
		InfoChain->Add(argv[1]);
	} else {
		printf("Strange file extention: .txt or .root expected\n");
		goto fin;
	}

	nEvt = EventChain->GetEntries();
	rEvt = RawChain->GetEntries();
	if (rEvt > 0 && rEvt != nEvt) {
		printf("Event chain (%d) and RawHits chain (%d) do not match\n",  nEvt, rEvt); 
		goto fin;
	} else if (rEvt == 0) {
		delete RawChain;
		RawChain = NULL;
	}
//	printf("EventChain: %d   RawHits: %d\n", nEvt, rEvt);
	memset(PairCnt, 0, sizeof(PairCnt));
	memset(&Veto, 0, sizeof(Veto));
	memset(&Shower, 0, sizeof(Shower));
	PickUpCnt = 0;
	for (iEvt =0; iEvt < nEvt; iEvt++) {
		EventChain->GetEntry(iEvt);
		if (RawChain) RawChain->GetEntry(iEvt);
		if (RawChain && IsPickUp(&DanssEvent, &RawHits)) {
			PickUpCnt++;
			continue;	// ignore PickUp events
		}
//	Shower
		if (IsShower(&DanssEvent)) memcpy(&Shower, &DanssEvent, sizeof(struct DanssEventStruct7));
//	Veto
		if (IsVeto(&DanssEvent)) {
			memcpy(&Veto, &DanssEvent, sizeof(struct DanssEventStruct7));
			continue;
		}
//	Get Neutron
		if (IsNeutron(&DanssEvent)) {
			memcpy(&Neutron, &DanssEvent, sizeof(struct DanssEventStruct7));
			CopyHits(&HitArray[1], &HitArray[2], DanssEvent.NHits);
			for (iLoop = 0; iLoop <= NRANDOM; iLoop++) {
				tShift = iLoop * RSHIFT;
//	Now look backward for positron in the region ([-50, 0] - iLoop*RSHIFT) us
				for (i=iEvt-1; i>=0; i--) {
					EventChain->GetEntry(i);
					if (RawChain) RawChain->GetEntry(i);
					if (RawChain && IsPickUp(&DanssEvent, &RawHits)) continue;	// ignore PickUp events
					if (Neutron.globalTime - DanssEvent.globalTime >= (MAXTDIFF + tShift) * GFREQ2US) break;		// not found
					if (Neutron.globalTime - DanssEvent.globalTime >= tShift * GFREQ2US && IsPositron(&DanssEvent)) break;	// found
					if (Neutron.globalTime - DanssEvent.globalTime < 0) break;
				}
				if (Neutron.globalTime - DanssEvent.globalTime < 0 || i < 0) break;
//	less than 50 us from neutron
				if (Neutron.globalTime - DanssEvent.globalTime < (MAXTDIFF + tShift) * GFREQ2US && i >= 0) {
					memcpy(&Positron, &DanssEvent, sizeof(struct DanssEventStruct7));
					CopyHits(&HitArray[0], &HitArray[2], DanssEvent.NHits);
					Positron.globalTime += tShift * GFREQ2US;	// assume it here !!!
					MakePair(&Neutron, &Positron, &Veto, &Shower, &DanssPair);
//	look backward
					for (i=iEvt-1;i>=0;i--) {
						EventChain->GetEntry(i);
						if (RawChain) RawChain->GetEntry(i);
						if (RawChain && IsPickUp(&DanssEvent, &RawHits)) continue;	// ignore PickUp events
						if (DanssEvent.globalTime > Positron.globalTime) {
							DanssPair.EventsBetween++;						
						} else if (DanssEvent.globalTime < Positron.globalTime) {
							DanssPair.gtFromPrevious = (Positron.globalTime - DanssEvent.globalTime) / GFREQ2US;
							DanssPair.PreviousEnergy = (DanssEvent.SiPmCleanEnergy + DanssEvent.PmtCleanEnergy) / 2;
							break;
						}
					}
					if (i == 0) DanssPair.gtFromPrevious = RSHIFT;	// something large
//	look forward
					for (i=iEvt+1;i < nEvt;i++) { 
						EventChain->GetEntry(i);
						if (RawChain) RawChain->GetEntry(i);
						if (RawChain && IsPickUp(&DanssEvent, &RawHits)) continue;	// ignore PickUp events
						DanssPair.gtToNext = (DanssEvent.globalTime - Positron.globalTime) / GFREQ2US;
						DanssPair.NextEnergy = (DanssEvent.SiPmCleanEnergy + DanssEvent.PmtCleanEnergy) / 2;
						break;
					}
					if (i == nEvt) DanssPair.gtToNext = RSHIFT;	// something large
					if (iLoop) {
						tRandom->Fill();
						PairCnt[1]++;
					} else {
						tOut->Fill();
						PairCnt[0]++;
					}
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

	printf("%Ld events processed with %d randomizing loops - %d/%d pairs found. Aquired time %7.0f s. PickUp count = %d\n", 
		iEvt, NRANDOM, PairCnt[0], PairCnt[1], SumInfo.upTime / GLOBALFREQ, PickUpCnt);
fin:
	if (EventChain) delete EventChain;
	if (InfoChain) delete InfoChain;
	if (RawChain) delete RawChain;

	InfoOut->Write();
	tOut->Write();
	tRandom->Write();
	fOut->Close();
	return 0;
}
