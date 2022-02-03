/***
 *
 * Version:       2.0
 *
 * Package:       DANSS time calibration pass 1
 *
 * Description:   Writes out a tree with muon events
 *                We select muons which have single hits only
 *                in each of top and bottom layers and 80-150 hits total
 *
 ***/
#include <libgen.h>
#include <stdio.h>
#include <string.h>

#include "Riostream.h"
#include "TROOT.h"
#include "TMath.h"
#include "TFile.h"
#include "TChain.h"
#include "TNetFile.h"
#include "TRandom2.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TStyle.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TTreeCacheUnzip.h"
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

#include "readDigiData.h"
#include "danssGlobals.h"

/***********************	Definitions	****************************/
#define MYVERSION	"2.00"
#define SIPMMINENERGY	0.5	// MeV
#define PMTMINENERGY	2.0	// MeV
#define VETOMINENERGY	2.0	// MeV
#define MUONMINHITS	80	// All detectors
#define MUONMAXHITS	150	// All detectors


using namespace std;

// Globals:
ReadDigiDataUser	*user;
long long		iNevtTotal;
long long		iNevtSelected;
long long		MaxEvents;

TFile *			OutputFile;
TTree *			OutputTree;
struct MuonEventStruct {
	long long number;
	int unixTime;
	int topX;
	int topY;
	int bottomX;
	int bottomY;
	int NHits;
} MuonEvent;

struct HitStruct {
	int	type[iMaxDataElements];	// 0 - SiPM, 1 - PMT, 2 - Veto
	int	chan[iMaxDataElements];	// ADC*100 + chan
	int	z[iMaxDataElements];	// odd - X, even - Y
	int	xy[iMaxDataElements];	// X or Y
	float	t[iMaxDataElements];
}	HitArray;


// Stores hit parameters
void StoreHits(void)
{
	int i, j, N;

	j = 0;
	N = user->nhits();
	for (i=0; i<N; i++) switch (user->type(i)) {
	case bSiPm:
		if (user->e(i) < SIPMMINENERGY) break;
		HitArray.type[j] = bSiPm;
		HitArray.chan[j] = 100 * user->adc(i) + user->adcChan(i);
		HitArray.z[j]    = user->zCoord(i);
		HitArray.xy[j]   = user->firstCoord(i);
		HitArray.t[j]    = user->t_raw(i);
		j++;
		break;
	case bPmt:
		if (user->e(i) < PMTMINENERGY) break;
		HitArray.type[j] = bPmt;
		HitArray.chan[j] = 100 * user->adc(i) + user->adcChan(i);
		HitArray.z[j]    = 2 * user->zCoord(i);
		if (user->side(i) == 'X') HitArray.z[j]++;
		HitArray.xy[j] = user->firstCoord(i);
		HitArray.t[j] = user->t_raw(i);
		j++;
		break;
	case bVeto:
		if (user->e(i) < VETOMINENERGY) break;
		HitArray.type[j] = bVeto;
		HitArray.chan[j] = 100 * user->adc(i) + user->adcChan(i);
		HitArray.z[j]    = user->adcChan(i);
		HitArray.xy[j]   = 0;
		HitArray.t[j]    = user->t_raw(i);
		j++;
		break;
	}
	MuonEvent.NHits = j;
}

// Check if this is a good muon
int GoodMuon(void)
{
	int i, cnt, N;
	int topXcnt, topXval;
	int topYcnt, topYval;
	int bottomXcnt, bottomXval;
	int bottomYcnt, bottomYval;
	
	N = user->nhits();
	cnt = 0;
	topXcnt = topYcnt = bottomXcnt = bottomYcnt = 0;
	for (i=0; i<N; i++) switch (user->type(i)) {
	case bSiPm:
		if (user->e(i) < SIPMMINENERGY) break;
		cnt++;
		break;
	case bPmt:
		if (user->e(i) < PMTMINENERGY) break;
		if (user->zCoord(i) == 0) {
			if (user->side(i) == 'X') {
				bottomXcnt++;
				bottomXval = user->firstCoord(i);
			} else {
				bottomYcnt++;
				bottomYval = user->firstCoord(i);
			}
		} else if (user->zCoord(i) == 4) {
			if (user->side(i) == 'X') {
				topXcnt++;
				topXval = user->firstCoord(i);
			} else {
				topYcnt++;
				topYval = user->firstCoord(i);
			}
		}
		cnt++;
		break;
	case bVeto:
		if (user->e(i) < VETOMINENERGY) break;
		cnt++;
		break;
	}
	if (cnt >= MUONMINHITS && cnt <= MUONMAXHITS && 
		topXcnt == 1 && topYcnt == 1 && bottomXcnt == 1 && bottomYcnt == 1) {
		MuonEvent.number = user->nevt();
		MuonEvent.topX = topXval;
		MuonEvent.topY = topYval;
		MuonEvent.bottomX = bottomXval;
		MuonEvent.bottomY = bottomYval;
		return 1;
	}
	return 0;
}

/************************	class ReadDigiDataUser user functions			*****************************/

/***
 *
 * A function to initialize calibration delays with all zeroes
 *
 ***/

void ReadDigiDataUser::init_Tds()
{
	FILE *f;
	char str[1024];
	char *ptr;
	int i, k;
	int iAdcNum, iAdcChan;
	int iFirst, iLast;
	double val;

//	Set all zeroes
	for(i = 100; i < iNElements; i++) {
    		iAdcNum = i / 100;
    		iAdcChan = i % 100;
    		if(!isAdcChannelExist(iAdcNum, iAdcChan)) continue;
    		setTd(i, 0); // set all td = 0
  	}
}

//------------------------------->

void Help(void)
{
	printf("\tDANSS time calibration pass 1. Version %s\n", MYVERSION);
	printf("Process events and create root-tree with muon events.\n");
	printf("\tOptions:\n");
	printf("-events number          --- stop after processing this number of events. Default - do not stop.\n");
	printf("-file filename.txt      --- file with a list of files for processing. No default.\n");
	printf("-help                   --- print this message and exit.\n");
	printf("-output filename.root   --- output file name. Without this key output file is not written.\n");
}

/***
 *
 * A function which is called to initialized user data
 *
 * Input parameters: standard command line parameters argc, argv
 *
 * Return value: -
 *
 ***/

void ReadDigiDataUser::initUserData(int argc, const char **argv)
{
	int i, j;
	char strs[1024];
	char strl[1024];
	char *chOutputFile = NULL;
	
	user = this;
	OutputFile = NULL;
	OutputTree = NULL;
	MaxEvents = 0;

	for (i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-output")) {
			i++;
			chOutputFile = (char *)argv[i];
		} else if (!strcmp(argv[i], "-events")) {
			i++;
			MaxEvents = strtol(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-help")) {
			Help();
			exit(0);
		}
	}

	init_Tds();

	if (chOutputFile) {
		strncpy(strs, chOutputFile, sizeof(strs));
		sprintf(strl, "mkdir -p %s", dirname(strs));
		if (system(strl)) {
			printf("Can not crete target directory: %m\n");
			exit(10);
		}
		OutputFile = new TFile(chOutputFile, "RECREATE");
		if (!OutputFile->IsOpen()) throw "Panic - can not open output file!";
		OutputFile->SetCompressionSettings(ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose);
		OutputTree = new TTree("MuonEvent", "Muon event tree");
		OutputTree->Branch("Data", &MuonEvent, 
//		Common parameters
			"number/L:"		// event number in the file
			"unixTime/I:"		// linux time, seconds
//		Muon track
			"topX/I:"		// top X hit in PMT (0-4)
			"topY/I:"		// top Y hit in PMT (0-4)
			"bottomX/I:"		// bottom X hit in PMT (0-4)
			"bottomY/I:"		// bottom Y hit in PMT (0-4)
//		Hit array
			"NHits/I"		// Number of hits
		);
		OutputTree->Branch("Type", HitArray.type, "Type[NHits]/I");
		OutputTree->Branch("Chan", HitArray.chan, "Chan[NHits]/I");
		OutputTree->Branch("Z", HitArray.z, "Z[NHits]/I");
		OutputTree->Branch("XY", HitArray.xy, "XY[NHits]/I");
		OutputTree->Branch("T", HitArray.t, "t[NHits]/F");
	}
	iNevtTotal = 0;
	iNevtSelected = 0;
}

//------------------------------->

/***
 *
 * A function which is called once per each event
 *
 * Input parameters: -
 *
 * Return value: - 0 - OK, -1 - stop
 *
 ***/

int ReadDigiDataUser::processUserEvent()
{
	float fineTime;
	int McNum;
	long long gt;

	if( ttype() != 1 ) return 0;
	
	memset(&MuonEvent, 0, sizeof(struct MuonEventStruct));

	if (GoodMuon()) {
		StoreHits();
		iNevtSelected++;
		if (OutputTree) OutputTree->Fill();
	}

	iNevtTotal++;
	if (MaxEvents > 0 && iNevtTotal >= MaxEvents) return -1;
	return 0;
}

/***
 *
 * A function which is called to finish user data processing
 *
 * Input parameters: -
 *
 * Return value: -
 *
 ***/

void ReadDigiDataUser::finishUserProc()
{
	int i, j;

	if (OutputFile) OutputFile->cd();
	if (OutputTree) OutputTree->Write();
	if (OutputFile) OutputFile->Close();
  
	printf("Total muon events %Ld / total events %Ld\n", iNevtSelected, iNevtTotal);
	return;
}

//--------------------->


int ReadDigiDataUser::userActionAtFileChange()
{
	return 0;
}
