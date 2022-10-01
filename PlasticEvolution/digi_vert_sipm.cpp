/***
 *
 * Version:       3.0
 *
 * Package:       DANSS SiPm/PMT Signal Processing and Calibration
 *
 * Description:   find vertical muons and get sipm hits 
 *
 ***/

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

#include "readDigiData.h"
#include "danssGlobals.h"
using namespace std;

// Parameters
#define MINPIX	1
#define MINHITS	90
#define MAXHITS	300
#define MINE	0.5
#define MAXR2	400
#define MAXDIST	6.0
#define MINEK	0.6
#define MAXEK	3.0
#define MAXADC	52
#define MAXCHAN	64

// Types

struct HitOutStruct {
	float E;
	float phe;
	float pix;
	float signal;
	float dist;
	int adc;
	int chan;
	int ovf;
} SelectedHit;

struct MuonEventStruct {
	int MCNum;
	int NHits;
	float xUp;
	float yUp;
	float xDown;
	float yDown;
	float r2;
} SelectedEvent;

// Globals:

TFile *OutputFile;
TTree *OutputHit;
TTree *OutputEvent;
TH2D  *hXZ;
TH2D  *hYZ;
TH1D  *hCoef[MAXADC][MAXCHAN];

int EventCnt;
int SelectedCnt;
int HitCnt;

//**************** Longitudinal correction

double SiPMYAverageLightColl(double x)
{
    //<func(x)=1>
	const double FuncAverage = 1.02208;
	double rez;
	if (x < 0) x = 0;
	if (x > 100) x = 100;
	rez = (0.00577381*exp(-0.1823*(x-50)) + 0.999583*exp(-0.0024*(x-48)) - 8.095E-13*exp(0.5205*(x-50))
	        -0.00535714*exp(-0.1838*(x-50))) / FuncAverage;
	return rez;
}

/************************	class ReadDigiDataUser user functions			*****************************/

/***
 *
 * A function to initialize calibration delays
 *
 * Input parameters: -
 *
 * Return value: -
 *
 ***/

void ReadDigiDataUser::init_Tds()
{
	int i;
	int iAdcNum, iAdcChan;

//	Set all zeroes
	for(i = 0; i < iNElements; i++) {
    		iAdcNum = i / 100;
    		iAdcChan = i % 100;
    		if(!isAdcChannelExist(iAdcNum, iAdcChan)) continue;
    		setTd(i, 0); // set all td = 0
  	}
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
	int i;
	char *chOutputFile;

	chOutputFile = NULL;

	for (i=1; i<argc; i++) if (!strcmp(argv[i], "-output")) {
		i++;
		chOutputFile = (char *)argv[i];
	}

	if (!chOutputFile) {
		chOutputFile = (char *) malloc(strlen(argv[0]) + 6);
		sprintf(chOutputFile, "%s%s", argv[0], ".root");
	}
	
	OutputFile = new TFile(chOutputFile, "RECREATE");
	if (!OutputFile->IsOpen()) throw "Panic - can not open output file!";
	OutputHit = new TTree("Hit", "Hit");
	OutputHit->Branch("Data", &SelectedHit, "E/F:phe/F:pix/F:signal/F:dist/F:adc/I:chan/I:ovf/I");
	OutputEvent = new TTree("Event", "Event");
	OutputEvent->Branch("Data", &SelectedEvent, "MCNum/I:NHits/I:xUP/F:yUP/F:xDown/F:yDown/F:r2/F");
	hXZ = new TH2D("hXZ", "XZ 2D plot;X;Z,N", 25, 0, 100, 50, 0, 100);
	hYZ = new TH2D("hYZ", "YZ 2D plot;Y;Z,N", 25, 0, 100, 50, 0, 100);
	memset(hCoef, 0, sizeof(hCoef));

	EventCnt = SelectedCnt = HitCnt = 0;
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
	int i, j, k, N, NHits;
	double XZ[50][25], YZ[50][25];
	double xUp, yUp, xDown, yDown;
	double ExUp, EyUp, ExDown, EyDown;
	double x, z;
	double r2;
	double Lcorr, Ccorr;
	char strs[128];
	char strl[1024];

	if( ttype() != 1 ) return 0;
	EventCnt++;
	memset(XZ, 0, sizeof(XZ));
	memset(YZ, 0, sizeof(YZ));
	N = nhits();
	
	// Fill channel amplification histogram
	for(i=0; i<N; i++) if (type(i) == bSiPm && e(i) > MINEK && e(i) < MAXEK && npe(i) > 0) {
		if (!hCoef[adc(i)][adcChan(i)]) {
			sprintf(strs, "hCoef_%d_%2.2d", adc(i), adcChan(i));
			sprintf(strl, "signal/phe for channel %d.%2.2d;signal/phe;N", adc(i), adcChan(i));
			hCoef[adc(i)][adcChan(i)] = new TH1D(strs, strl, 500, 40, 140);
		}
		hCoef[adc(i)][adcChan(i)]->Fill(signal(i) / npe(i));
	}
	
	if (N < MINHITS || N > MAXHITS) return 0;

	// Fill XZ and YZ with SiPM hit energy. Ignore hits with less than MINPIX pixels
	for(i=0; i<N; i++) if (type(i) == bSiPm && npix(i) >= MINPIX) {
		switch(side(i)) {
		case 'X': 
			XZ[zCoord(i)/2][firstCoord(i)] = e(i); 
			hXZ->Fill(firstCoord(i)*4+2.0, zCoord(i)+0.5, e(i));
			break;
		case 'Y': 
			YZ[zCoord(i)/2][firstCoord(i)] = e(i); 
			hYZ->Fill(firstCoord(i)*4+2.0, zCoord(i)+0.5, e(i));
			break;
		}
	}
	
	// Find the track up and down crossings
	xUp = yUp = xDown = yDown = ExUp = EyUp = ExDown = EyDown = 0;
	for(k=0; k<4; k++) for(j=0; j<25; j++) {
		if (XZ[k][j] > MINE) {
			xDown += (4*j + 2.0) * XZ[k][j];
			ExDown += XZ[k][j];
		}
		if (YZ[k][j] > MINE) {
			yDown += (4*j + 2.0) * YZ[k][j];
			EyDown += YZ[k][j];
		}
		if (XZ[k+46][j] > MINE) {
			xUp += (4*j + 2.0) * XZ[k+46][j];
			ExUp += XZ[k+46][j];
		}
		if (YZ[k+46][j] > MINE) {
			yUp += (4*j + 2.0) * YZ[k+46][j];
			EyUp += YZ[k+46][j];
		}
	}
	if (ExUp < MINE || EyUp < MINE || ExDown < MINE || EyDown < MINE) return 0;
	xUp /= ExUp;
	xDown /= ExDown;
	yUp /= EyUp;
	yDown /= EyDown;
	r2 = (xUp - xDown) * (xUp - xDown) + (yUp - yDown) * (yUp - yDown);
	if (r2 > MAXR2) return 0;
	
	// Select and store hits
	NHits = 0;
	for(i=0; i<N; i++) if (type(i) == bSiPm && npix(i) >= MINPIX) {
		j = firstCoord(i);
		k = zCoord(i)/2;
		z = k - 1.5;
		switch(side(i)) {
		case 'X': 
			// cut for proper position
			x = (xUp - xDown) * z / 46 + xDown;
			if (4*j + 2.0 < x - MAXDIST || 4*j + 2.0 > x + MAXDIST) continue;
			// cuts for no leakage
			if (j > 0 && XZ[k][j-1] > 0) continue;
			if (j < 24 && XZ[k][j+1] > 0) continue;
			// cuts for strips above and below
			if (k > 0 && XZ[k-1][j] < MINE) continue;
			if (k < 49 && XZ[k+1][j] < MINE) continue;
			// Longitudinal correction
			Lcorr = yDown + (yUp - yDown) * k / 46.0;
			break;
		case 'Y':
			// cut for proper position
			x = (yUp - yDown) * z / 46 + yDown;
			if (4*j + 2.0 < x - MAXDIST || 4*j + 2.0 > x + MAXDIST) continue;
			// cuts for no leakage
			if (j > 0 && YZ[k][j-1] > 0) continue;
			if (j < 24 && YZ[k][j+1] > 0) continue;
			// cuts for strips above and below
			if (k > 0 && YZ[k-1][j] < MINE) continue;
			if (k < 49 && YZ[k+1][j] < MINE) continue;
			// Longitudinal correction
			Lcorr = xDown + (xUp - xDown) * (k - 1) / 46.0;
			break;
		}
		Ccorr = SiPMYAverageLightColl(Lcorr);
		// Good hit - fill
		HitCnt++;
		NHits++;
		SelectedHit.E = e(i) / Ccorr;
		SelectedHit.phe = npe(i) / Ccorr;
		SelectedHit.pix = npix(i) / Ccorr;
		SelectedHit.dist = Lcorr;
		SelectedHit.adc = adc(i);
		SelectedHit.signal = signal(i) / Ccorr;
		SelectedHit.chan = adcChan(i);
		SelectedHit.ovf = wasOvfl(i);
		OutputHit->Fill();
	}
	
	
	// Store muon event information
	SelectedCnt++;
	SelectedEvent.MCNum = absTime();
	SelectedEvent.NHits = NHits;
	SelectedEvent.xUp = xUp;
	SelectedEvent.yUp = yUp;
	SelectedEvent.xDown = xDown;
	SelectedEvent.yDown = yDown;
	SelectedEvent.r2 = r2;
	OutputEvent->Fill();
	
	if ((EventCnt%1000) == 0) {
		printf(".");
		fflush(stdout);
	}
	return 0;
}

//------------------------------->

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
	for (i=0; i<MAXADC; i++) for (j=0; j<MAXCHAN; j++) if (hCoef[i][j]) hCoef[i][j]->Write();
	OutputEvent->Write();
	OutputHit->Write();
	hXZ->Write();
	hYZ->Write();
	OutputFile->Close();
	printf("%d events processed. %d muon tracks found. %d hits stored\n", EventCnt, SelectedCnt, HitCnt);
	return;
}

//--------------------->


int ReadDigiDataUser::userActionAtFileChange()
{
	return 0;
}

