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
#define MINNEUTE	2.0	// MeV
#define MAXNEUTE	15.0	// MeV
#define NEUTN		3	// number of hits
#define MINVETOE	4.0	// MeV
#define VETON		2	// number of hits
#define DANSSVETOE	20.0	// Make veto if VETO counters are silent from Pmt or SiPM
#define BOTTOMVETOE	3.0	// Make veto from 2 bottom strip layers
#define RSHIFT		5000.0	// us
#define NRANDOM		16	// increase random statistics
#define SHOWERMIN	800	// 800 MeV shower event threshold
#define TISOLATIONM	140	// isolation us for random positron windows minus, 90 + 50 us
#define TISOLATIONP	80	// isolation us for random positron windows plus, 80 us
//	No correction
#define CORR_P0		0	// Positron energy correction from MC 
#define CORR_P1		1.0	// Positron energy correction from MC
#define CORR_PMT_P0	0	// Positron energy correction from MC
#define CORR_PMT_P1	1.0	// Positron energy correction from MC
#define CORR_SIPM_P0	0	// Positron energy correction from MC
#define CORR_SIPM_P1	1.0	// Positron energy correction from MC

#define iMaxDataElements 3000
#define masterTrgRandom 2

#define FLAG_IGNORE	1
#define FLAG_NEUTRON	2
#define FLAG_POSITRON	4
#define FLAG_VETO	8
#define FLAG_SHOWER	16

struct TrigArrayStruct {
	long long globalTime;
	int flags;
};

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

int IsIgnore(struct DanssEventStruct7 *DanssEvent, struct RawHitInfoStruct *RawHits)
//	"(PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3"
{
	if (DanssEvent->trigType == masterTrgRandom) return 1;		// ignore 1 Hz triggers
	if (DanssEvent->VetoCleanHits > 0) return 0;	// never kill VETO trigger
	if (!RawHits) return 0;
	if ((RawHits->PmtCnt > 0 && 1.0 * DanssEvent->PmtCleanHits / RawHits->PmtCnt < 0.3) ||
		1.0 * DanssEvent->SiPmHits / RawHits->SiPmCnt < 0.3) return 1;
	return 0;
}

int IsNeutron(struct DanssEventStruct7 *DanssEvent)
{
	float E;
	int rc;
	
	E = DanssEvent->NeutronEnergy;
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
	DanssPair->NeutronEnergy = DanssEvent->NeutronEnergy;
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
//	Copy DANSSEvent tree from the original MC
	const char MCLeafList[] = 
		"EventID/D:"
		"ParticleEnergy/D:"
		"EnergyLoss/D:"
		"DetectorEnergyLoss/D:"
		"CopperEnergyLoss/D:"
		"GdCoverEnergyLoss/D:"
		"X/D:Y/D:Z/D:"
		"DirX/D:DirY/D:DirZ/D:"
		"TimelineShift/D:"
		"FluxFlag/B";
	
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
	struct MCEventStruct		MCEvent;
	struct MCEventStruct		MCEventCopy;
	struct TrigArrayStruct		*TrigArray = NULL;
	TChain *EventChain = NULL;
	TChain *InfoChain = NULL;
	TChain *RawChain = NULL;
	TTree *tOut = NULL;
	TTree *tRandom = NULL;
	TTree *InfoOut = NULL;
	TFile *fOut = NULL;
	FILE *fList;
	char str[1024];
	char strl[1600];
	long long iEvt, nEvt, rEvt;
	int PairCnt[2], WinCnt[3];
	int PickUpCnt;
	int i;
	int iLoop;
	float tShift;
	char *ptr;
	int IsMC = 0;
	int LoopCnt, inCnt, outCnt, gtDiff;
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
	RawChain = new TChain("RawHits");
	InfoChain = new TChain("DanssInfo");
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
		return 30;
	}
//			Check RawHits
	nEvt = EventChain->GetEntries();
	rEvt = RawChain->GetEntries();
	if (rEvt > 0 && rEvt != nEvt) {
		printf("Event chain (%d) and RawHits chain (%d) do not match\n",  nEvt, rEvt); 
		return 40;
	} else if (rEvt == 0) {
		delete RawChain;
		RawChain = NULL;
	}
//			Is this MC ?
	if(EventChain->GetBranch("MCEvent")) IsMC = 1;
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
//			Create output Chains
	tOut = new TTree("DanssPair", "Time Correlated events");
	tOut->Branch("Pair", &DanssPair, LeafList);
	tOut->Branch("PHitE", HitArray[0].E, "PHitE[NPHits]/F");
	tOut->Branch("PHitT", HitArray[0].T, "PHitT[NPHits]/F");
	tOut->Branch("PHitType", HitArray[0].type, "PHitType[NPHits]/I");
	tOut->Branch("NHitE", HitArray[1].E, "NHitE[NNHits]/F");
	tOut->Branch("NHitT", HitArray[1].T, "NHitT[NNHits]/F");
	tOut->Branch("NHitType", HitArray[1].type, "NHitType[NNHits]/I");
	if (IsMC) tOut->Branch("MCEvent", &MCEventCopy, MCLeafList);

	tRandom = new TTree("DanssRandom", "Random coincidence events");
	tRandom->Branch("Pair", &DanssPair, LeafList);
	tRandom->Branch("PHitE", HitArray[0].E, "PHitE[NPHits]/F");
	tRandom->Branch("PHitT", HitArray[0].T, "PHitT[NPHits]/F");
	tRandom->Branch("PHitType", HitArray[0].type, "PHitType[NPHits]/I");
	tRandom->Branch("NHitE", HitArray[1].E, "NHitE[NNHits]/F");
	tRandom->Branch("NHitT", HitArray[1].T, "NHitT[NNHits]/F");
	tRandom->Branch("NHitType", HitArray[1].type, "NHitType[NNHits]/I");
	if (IsMC) tRandom->Branch("MCEvent", &MCEventCopy, MCLeafList);

	InfoOut = new TTree("SumInfo", "Summary information");
	InfoOut->Branch("Info", &SumInfo,  
		"gTime/L:"		// running time in terms of 125 MHz
		"startTime/I:"		// linux start time, seconds
		"stopTime/I:"		// linux stop time, seconds
		"events/L"		// number of events
	);
	memset(&SumInfo, 0, sizeof(struct DanssInfoStruct));
//			Set input branch addresses
	EventChain->SetBranchAddress("Data", &DanssEvent);
	EventChain->SetBranchAddress("HitE", &HitArray[2].E);
	EventChain->SetBranchAddress("HitT", &HitArray[2].T);
	EventChain->SetBranchAddress("HitType", &HitArray[2].type);
	if (IsMC) EventChain->SetBranchAddress("MCEvent", &MCEvent);
	if (RawChain) RawChain->SetBranchAddress("RawHits", &RawHits);
	InfoChain->SetBranchAddress("Info", &DanssInfo);
//			Create trigger arryay to facilitate searches
	TrigArray = (struct TrigArrayStruct *) malloc(nEvt * sizeof(struct TrigArrayStruct));
	if (!TrigArray) {
		printf("TrigArray[%d] memery allocation failed: %m\n", nEvt);
		goto fin;
	}
	for (iEvt =0; iEvt < nEvt; iEvt++) {
		EventChain->GetEntry(iEvt);
		if (RawChain) RawChain->GetEntry(iEvt);
		TrigArray[iEvt].globalTime = DanssEvent.globalTime;
		TrigArray[iEvt].flags = 0;
		if ((RawChain) ? IsIgnore(&DanssEvent, &RawHits) : IsIgnore(&DanssEvent, NULL)) TrigArray[iEvt].flags |= FLAG_IGNORE;
		if (IsNeutron(&DanssEvent)) TrigArray[iEvt].flags |= FLAG_NEUTRON;
		if (IsPositron(&DanssEvent)) TrigArray[iEvt].flags |= FLAG_POSITRON;
		if (IsVeto(&DanssEvent)) TrigArray[iEvt].flags |= FLAG_VETO;
		if (IsShower(&DanssEvent)) TrigArray[iEvt].flags |= FLAG_SHOWER;
	}

//	printf("EventChain: %d   RawHits: %d\n", nEvt, rEvt);
	memset(PairCnt, 0, sizeof(PairCnt));
	memset(WinCnt, 0, sizeof(WinCnt));
	memset(&Veto, 0, sizeof(Veto));
	memset(&Shower, 0, sizeof(Shower));
	PickUpCnt = 0;
	for (iEvt =0; iEvt < nEvt; iEvt++) {
		if (TrigArray[iEvt].flags & FLAG_IGNORE) {
			PickUpCnt++;
			continue;	// ignore PickUp and 1 Hz events
		}
		EventChain->GetEntry(iEvt);
//	Shower
		if (TrigArray[iEvt].flags & FLAG_SHOWER) memcpy(&Shower, &DanssEvent, sizeof(struct DanssEventStruct7));
//	Veto
		if (TrigArray[iEvt].flags & FLAG_VETO) {
			memcpy(&Veto, &DanssEvent, sizeof(struct DanssEventStruct7));
			continue;
		}
//	Get Neutron
		if (TrigArray[iEvt].flags & FLAG_NEUTRON) {
			memcpy(&Neutron, &DanssEvent, sizeof(struct DanssEventStruct7));
			CopyHits(&HitArray[1], &HitArray[2], DanssEvent.NHits);
			LoopCnt = 0;
			WinCnt[0]++;
			for (iLoop = 0; LoopCnt <= NRANDOM; iLoop++) {
				tShift = iLoop * RSHIFT;
				if (iLoop) {	// check the isolation window - looking backward from neutron and around the signal/background window
					WinCnt[1]++;
					inCnt = outCnt = 0;
					for (i=iEvt-1; i>=0; i--) {
						if (TrigArray[i].flags & FLAG_IGNORE) continue;
						gtDiff = TrigArray[i].globalTime + tShift * GFREQ2US - Neutron.globalTime;
						if (gtDiff >= -TISOLATIONM * GFREQ2US && gtDiff < -MAXTDIFF * GFREQ2US) {
							outCnt++;		// outer count
						} else if (gtDiff >= -MAXTDIFF * GFREQ2US && gtDiff < 0) {
							if (TrigArray[i].flags & FLAG_POSITRON) {
								inCnt++;
							} else {
								outCnt++;
							}
						} else if (gtDiff >= 0 && gtDiff <= TISOLATIONP * GFREQ2US) {
							outCnt++;
						} else if (gtDiff < -TISOLATIONM * GFREQ2US) {
							break;		// end of search
						}
					}
					if (inCnt > 1 || outCnt > 0) {
						WinCnt[2]++;
						continue;		// don't count this window because of isolation cut
					}
				}
//	Now look backward for positron in the region ([-50, 0] - iLoop*RSHIFT) us
				inCnt = 0;
				for (i=iEvt-1; i>=0; i--) {
					if (TrigArray[i].flags & FLAG_IGNORE) continue;
					gtDiff = TrigArray[i].globalTime + tShift * GFREQ2US - Neutron.globalTime;
					if (gtDiff >= -MAXTDIFF * GFREQ2US && gtDiff < 0 && (TrigArray[i].flags & FLAG_POSITRON)) {
						inCnt++;
						break;
					}
					if (gtDiff < -MAXTDIFF * GFREQ2US) break;
				}
				if (inCnt) {	// Positron found
					EventChain->GetEntry(i);
					memcpy(&Positron, &DanssEvent, sizeof(struct DanssEventStruct7));
					CopyHits(&HitArray[0], &HitArray[2], DanssEvent.NHits);
					Positron.globalTime += tShift * GFREQ2US;	// assume it here !!!
					MakePair(&Neutron, &Positron, &Veto, &Shower, &DanssPair);
					if (IsMC) memcpy(&MCEventCopy, &MCEvent, sizeof(MCEvent)); // Copy MC DANSSEvent for positron
//				Isolation for Neutron position
//	look backward
					for (i=iEvt-1;i>=0;i--) {
						if (TrigArray[i].flags & FLAG_IGNORE) continue;
						if (TrigArray[i].globalTime > Positron.globalTime) {
							DanssPair.EventsBetween++;
						} else if (TrigArray[i].globalTime < Positron.globalTime && !(TrigArray[i].flags & FLAG_VETO)) {
							EventChain->GetEntry(i);
							DanssPair.gtFromPrevious = (Positron.globalTime - DanssEvent.globalTime) / GFREQ2US;
							DanssPair.PreviousEnergy = (DanssEvent.SiPmCleanEnergy + DanssEvent.PmtCleanEnergy) / 2;
							break;
						}
					}
					if (i < 0) DanssPair.gtFromPrevious = RSHIFT;	// something large
//	look forward
					for (i=iEvt+1;i<nEvt;i++) { 
						if (TrigArray[i].flags & FLAG_IGNORE) continue;
						EventChain->GetEntry(i);
						DanssPair.gtToNext = (DanssEvent.globalTime - Positron.globalTime) / GFREQ2US;
						DanssPair.NextEnergy = (DanssEvent.SiPmCleanEnergy + DanssEvent.PmtCleanEnergy) / 2;
						break;
					}
					if (i == nEvt) {
						DanssPair.gtToNext = RSHIFT;	// something large
						DanssPair.NextEnergy = 0;
					}
//	Fill proper tree
					if (iLoop) {
						tRandom->Fill();
						PairCnt[1]++;
					} else {
						tOut->Fill();
						PairCnt[0]++;
					}
				}
				LoopCnt++;
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
	printf("%d neutron candidates; %d windows checked; %d windows rejected\n", WinCnt[0], WinCnt[1], WinCnt[2]);

fin:
	if (EventChain) delete EventChain;
	if (InfoChain) delete InfoChain;
	if (RawChain) delete RawChain;
	if (InfoOut) InfoOut->Write();
	if (tOut) tOut->Write();
	if (tRandom) tRandom->Write();
	if (fOut) fOut->Close();
	if (TrigArray) free(TrigArray);
	return 0;
}
