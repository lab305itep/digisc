/***
 *
 * Version:       4.0
 *
 * Package:       DANSS SiPm Signal Processing and Calibration
 *
 * Description:   Calculate different event parameters and put to root file
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
#include "evtbuilder.h"

/***********************	Definitions	****************************/
#define MYVERSION	"4.70"
//	Initial clean parameters
#define MINSIPMPIXELS	3			// Minimum number of pixels to consider SiPM hit for FineTime calculation
// #define MINSIPMPIXELS2	2		// Minimum number of pixels to consider SiPM hit without confirmation (method 2)
// #define MINPMTENERGY	0.1			// Minimum PMT energy for a hit
// #define MINVETOENERGY	0.1		// Minimum VETO energy for a hit
#define SIPMEARLYTIME	45			// ns - shift from fine time
#define SOMEEARLYTIME	130			// ns - absolute if fineTime is not defined
#define MAXPOSITRONENERGY	20		// Maximum Total clean energy to calculate positron parameters
#define MAXCLUSTITER		10		// Maximum number of iterations in cluster search
#define MCNEUTRONSIGGMA		20.0		// Sigma for neutron based longitudinal correction for MC
#define NBOTTOMLAYERS		2		// Use two bottom SiPM layers as additional VETO
//#define ENERGY_CORRECTION	0.95		// Energy correction to be applied for experimental data
//	fine time
#define MINENERGY4TIME	0.25			// Minimum energy to use for fine time averaging
#define MINAVRTIME	130			// Minimum time for hit to be used in fine time calculations
#define TCUT		10			// fine time cut, ns for SiPM
//#define TCUT		15.0			// fine time cut, ns for SiPM for experiment
#define TMCCUTMIN	(-15.0)			// fine time cut, ns for SiPM for MC
#define TMCCUTMAX	(15.0)			// fine time cut, ns for SiPM for MC
#define TCUTPMT		30			// fine time cut, ns for PMT and VETO
#define NOFINETIME	10000			// something out of range
#define MCPMTDELAY	(-13.2)			// PMT MC DELAY, ns (-13.2)
#define MCSIPMDELAY	15.2			// SiPM MC DELAY, ns (15.2)
//	Flags
#define FLG_PRINTALL		       1	// do large debuggging printout
#define FLG_DTHIST		       2	// create time delta histogramms
//#define FLG_EAMPLITUDE	       4	// put amplitude instead of energy to XXCleanEnergy cells - abandoned
//#define FLG_POSECORRECTIONA	   0x100	// do positron energy correction based on MC and NHITS
//#define FLG_POSECORRECTIONB	   0x200	// do positron energy correction based on average MC
#define FLG_SIMLONGCORR	  	  0x1000	// simulate "neutron" correction for MC events
//#define FLG_NOCLEANNOISE 	 0x10000	// do not clean low energy signals
#define FLG_NOTIMECUT		 0x20000	// do not clean signals by time
//#define FLG_NOCONFIRM		 0x40000	// do not search PMT confirmation for SiPM and vice versa
//#define FLG_NOCONFIRM2	 0x80000	// do not search PMT confirmation for 1 pixel SiPM signals
#define FLG_NOPMTCORR		0x100000	// do not correct PMT energy of cluster for out of cluster SiPM hits
#define FLG_PMTTIMECUT		0x200000	// Cut PMT and Veto by time
#define FLG_CONFIRMSIPM		0x400000	// do not confirm all SiPM hits
#define FLG_MCENERGYSMEAR	0x800000	// Do additional energy smear for MC events (IBD simulation assumed)

#define MAXADCBOARD	60
#define TAGMASK		((1L<<45) - 1)
#define TIMEHISTMINENERGY	3		// Minimum sum energy in SiPm + Pmt + Veto to fill time hists (not divided by 2)
#define TIMEHISTMINHITS		4		// Minimum number of hits in SiPm + Pmt + Veto to fill time hists

using namespace std;

// Globals:
ReadDigiDataUser		*user;
long long			iNevtTotal;
long long			upTime;
long long			fileFirstTime;
long long			fileLastTime;
long long			dumpgTime;
int				globalTimeWrapped;
int                             progStartTime;
char *				chTimeCalibration;
char *				chOutputFile;
int				iFlags;
int				MaxEvents;
int				IsMc;				// MC run flag
double				EnergyCorrection;		// energy correction based on 12B
double				SiPMEnergyCorrection;		// correct SiPM energy based on MC single pixel response
double				MCEnergyCorrection;		// MC energy correction

TRandom2 *			Random;
struct {	// additional energy smear for MC events
	double st;	// stokhastic term
	double ct;	// impurity term
} MCsmear;
TFile *				OutputFile;
TTree *				OutputTree;
TTree *				InfoTree;
TTree *				RawHitsTree;
TFile *				McFile;
TTree *				McEventTree;
int				McEntries;
struct DanssEventStruct7	DanssEvent;
struct DanssInfoStruct4		DanssInfo;
struct DanssMcStruct		DanssMc;
struct MCEventStruct		McEvent;
struct DanssExtraStruct	{
	float SiPmEnergy;
	int SiPmHits;
	float PmtEnergy;
	int PmtHits;
	float VetoEnergy;
	int VetoHits;
} 				DanssExtra;
int 				HitFlag[iMaxDataElements];	// array to flag out SiPM hits
	// Hitflag = -100 - early hit
	// HitFlag = -1   - bad hit
	// HitFlag = 0    - ordinary hit
	// Hitflag = 5    - Pmt Hit in the cluster
	// HitFlag = 10   - the most energetic hit in the cluster (SiPm)
	// HitFlag = 20   - other hit in the cluster (SiPm)
TH1D *				hTimeDelta[MAXADCBOARD][iNChannels_AdcBoard];
TH1D *				hPMTTimeDelta[MAXADCBOARD][iNChannels_AdcBoard];
double				PmtFineTime;
int				DeadList[MAXADCBOARD][iNChannels_AdcBoard];
struct HitStruct {
	float			E[iMaxDataElements];
	float			T[iMaxDataElements];
	struct HitTypeStruct 	type[iMaxDataElements];
}	HitArray;

struct RawStruct {
	unsigned short PmtCnt;
	unsigned short VetoCnt;
	unsigned short SiPmCnt;
} RawHits;
struct RawArrayStruct {
	long long tag;
	struct RawStruct data;
};
struct RawArrayStruct *RawHitsArray;
int RawHitsPtr;
int RawHitsCnt;

TH1D *hCrossTalk;
TH1D *hPMTAmpl[iNChannels_AdcBoard];
TH1D *hSiPMtime[6];
TH1D *hSiPMtimeClean[6];

TH1D *hEtoEMC;
TH1D *hNPEtoEMC;
/********************************************************************************************************************/
/************************	Raw hits - fight with the pickup			*****************************/
/********************************************************************************************************************/

/*      Open data file either directly or via zcat etc, depending on the extension      */
FILE* OpenTextDataFile(const char *fname) 
{
	char cmd[1024];
	FILE *f;

	if (strstr(fname, ".bz2")) {
		sprintf(cmd, "bzcat %s", fname);
    	        f = popen(cmd, "r");
        } else if (strstr(fname, ".gz")) {
                sprintf(cmd, "zcat %s", fname);
                f = popen(cmd, "r");
        } else if (strstr(fname, ".xz")) {
                sprintf(cmd, "xzcat %s", fname);
    	        f = popen(cmd, "r");
        } else {
                f = fopen(fname, "rb");
        }
        return f;
}

// Initialize Raw hits Array from file return number of triggers read
int RawHitsArrayInit(const char *RawHitsFileName)
{
	FILE *f;
	int size;
	char *ptr;
	char str[1024];
	char str_copy[1024];
	void *mptr;
	
	RawHitsCnt = 0;
	RawHitsPtr = 0;
	f = OpenTextDataFile(RawHitsFileName);
	if (!f) {
		printf("Can not open file %s: %m\n", RawHitsFileName);
		goto fin;
	}
	size = 2000000;
	RawHitsArray = (struct RawArrayStruct *) malloc(size * sizeof(struct RawArrayStruct));
	if (!RawHitsArray) {
		printf("Can not allocate memory: %m\n");
		goto fin;
	}
	for (;;) {
		ptr = fgets(str, sizeof(str), f);
		if (!ptr) break;	// EOF
		strcpy(str_copy, str);
		ptr = strtok(str, " \t");
		if (!ptr) continue;
		if (!isdigit(ptr[0])) continue;
		if (RawHitsCnt >= size) {	// we need larger array
			mptr = realloc(RawHitsArray, (size + 1000000) * sizeof(struct RawArrayStruct));
			if (!mptr) {
				printf("Can not allocate memory: %m\n");
				goto fin;
			}
			size += 1000000;
			RawHitsArray = (struct RawArrayStruct *)mptr;
		}
		RawHitsArray[RawHitsCnt].tag = TAGMASK & strtoll(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Bad string: %s [file %s]\n", str_copy, RawHitsFileName);
			continue;
		}
		RawHitsArray[RawHitsCnt].data.PmtCnt = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Bad string: %s [file %s]\n", str_copy, RawHitsFileName);
			continue;
		}
		RawHitsArray[RawHitsCnt].data.VetoCnt = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Bad string: %s [file %s]\n", str_copy, RawHitsFileName);
			continue;
		}
		RawHitsArray[RawHitsCnt].data.SiPmCnt = strtol(ptr, NULL, 10);
		RawHitsCnt++;
	}
fin:
	if (f) pclose(f);
	if (!RawHitsCnt && RawHitsArray) {
		free(RawHitsArray);
		RawHitsArray = NULL;
	}
	return RawHitsCnt;
}

// Find raw hits numbers for the trigger being processed
void FindRawHits(void)
{
	long long gtA;
	long long gtB;

	memset(&RawHits, 0, sizeof(RawHits));
	gtA = TAGMASK & DanssEvent.globalTime;
	for (; RawHitsPtr < RawHitsCnt; RawHitsPtr++) {
			gtB = RawHitsArray[RawHitsPtr].tag;
			if (gtB == gtA) break;
		} 
		if (gtB != gtA) {	// try to start from the beginning
			for (RawHitsPtr = 0; RawHitsPtr < RawHitsCnt; RawHitsPtr++) {
				gtB = RawHitsArray[RawHitsPtr].tag;
				if (gtB == gtA) break;
			}
		}
		if (gtB == gtA) {		// trigger found
			memcpy(&RawHits, &RawHitsArray[RawHitsPtr].data, sizeof(struct RawStruct));
		} else {			// trigger is somehow missing
			printf("Trigger %Ld not found.\n", gtA);
		}
}

// Check if the current trigger is associated with pickup noise. 
// We never consider VETO triggers as PickUp
// We consider PickUp either less than 30% of SiPM or PMT hits pass Ira's analysis
int IsPickUp(void)
{
	if (!RawHitsArray) return 0;		// no RawHits information
//	"(PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3"
	if (DanssEvent.VetoCleanHits > 0) return 0;	// never kill VETO trigger
	if ((RawHits.PmtCnt > 0 && 1.0 * DanssEvent.PmtCleanHits / RawHits.PmtCnt < 0.3) ||
		1.0 * DanssEvent.SiPmHits / RawHits.SiPmCnt < 0.3) return 1;
	return 0;
}


/********************************************************************************************************************/
/************************	Analysis functions					*****************************/
/********************************************************************************************************************/

// Get time from hit number. Apply MC correction
// i - hit number
double HitTime(int i)
{
	if (!IsMc) return user->t_raw(i);
	if (user->adc(i) == 1 || user->adc(i) == 3) return  user->t_raw(i) - MCPMTDELAY;
	return user->t_raw(i) - MCSIPMDELAY;
}

// Get energy from hit number. Apply global corrections
// i - hit number
double Energy(int i)
{
	double E;
	E = user->e(i);
	if (user->type(i) == bSiPm) E *= SiPMEnergyCorrection;
	if (IsMc) {
		E *= MCEnergyCorrection;
	} else {
		E *= EnergyCorrection;
	}
	return E;
}

//	int SiPm - hit number in SiPM
//	int Pmt  - hit number in PMT
//	return true if SiPM is read by this PMT
int IsInModule(int SiPm, int Pmt)
{
	int SiPmXY, PmtXY;
	int SiPmZ, PmtZ;

	if (user->side(SiPm) != user->side(Pmt)) return false;
	if (user->type(SiPm) != bSiPm || user->type(Pmt) != bPmt) return false;	// wrong request
	SiPmXY = user->firstCoord(SiPm);
	PmtXY  = user->firstCoord(Pmt);
	SiPmZ  = user->zCoord(SiPm);
	PmtZ   = user->zCoord(Pmt);
	if (SiPmXY / 5 != PmtXY || SiPmZ / 20 != PmtZ) return false;
	return true;
}

//	int hitA, hitB - hits in SiPM
//	return true if the SiPMs are neighbors or coinside
int IsNeighbor(int hitA, int hitB)
{
	if (user->zCoord(hitA) == user->zCoord(hitB) && abs(user->firstCoord(hitA) - user->firstCoord(hitB)) <= 1) return 1;
	if (abs(user->zCoord(hitA) - user->zCoord(hitB)) == 1) return 1;
	return 0;
}

// Do MC extra energy smearing 
double MCEnergySmear(double E)
{
	return Random->Gaus(E, sqrt(MCsmear.st * MCsmear.st * E + MCsmear.ct * MCsmear.ct * E * E));
}

// Longitudinal correction for PMT
double PMTYAverageLightColl(double x)
{
    //<func(x)=1>
	const double FuncAverage = 1.00147;
	double rez;
	rez = (0.987387*exp(-0.0016*(x-48)) + 0.023973*exp(-0.0877*(x-48)) - 0.0113581*exp(-0.1042*(x-48))
	        -2.30972E-6*exp(0.2214*(x-48))) / FuncAverage;
	return rez;
}

// Longitudinal correction for SiPM
double SiPMYAverageLightColl(double x)
{
    //<func(x)=1>
	const double FuncAverage = 1.02208;
	double rez;
	rez = (0.00577381*exp(-0.1823*(x-48)) + 0.999583*exp(-0.0024*(x-48)) - 8.095E-13*exp(0.5205*(x-48))
	        -0.00535714*exp(-0.1838*(x-48))) / FuncAverage;
	return rez;
}

// Longitudinal light correctrion
double YAverageLightColl(double x, int type)
{
	double rez;
	switch(type) {
	case bSiPm:
		rez = SiPMYAverageLightColl(x);
		break;
	case bPmt:
		rez = PMTYAverageLightColl(x);
		break;
	default:
		rez = 1.0;
	}
	return rez;
}

//	float energy - measured energy
//	float dist - distance from zero coordinate
//	return corrected energy
//	simulate "neutron" correction for MC events
float acorr(float energy, float dist, char side = 'Y', int type = bSiPm)
{
	float C, XY;

	if (dist >= 0) {
		C = 1.0 / YAverageLightColl(dist, type);
	} else if (IsMc && (iFlags & FLG_SIMLONGCORR)) {
		XY = (side == 'X') ? DanssMc.X[1] : DanssMc.X[0];
		XY -= 2;
		dist = Random->Gaus(XY, MCNEUTRONSIGGMA);
		if (dist < 0) dist = 0;
		if (dist > 96) dist = 96;
		C = 1.0 / YAverageLightColl(dist, type);
	} else {
		C = 1;
	}
	return C * energy;
}

/********************************************************************************************************************/
/************************		Main analysis					*****************************/
/********************************************************************************************************************/

// Calculate parameters assuming positron-like event
void CalculatePositron(void)
{
	int i, j, k, N;
	float A;
	float x, y, z;
	float nx, ny;
	int maxHit;
	int repeat;
	int clusterHits[10];		// Maximum possible cluster 5x2
	int xmin, xmax, ymin, ymax, zmin, zmax;
	int xy;
	int invalid;
	float E;

	DanssEvent.MinPositron2GammaZ = 1000;
	DanssEvent.PositronX[0] = -1;
	DanssEvent.PositronX[1] = -1;
	DanssEvent.PositronX[2] = -1;
	N = user->nhits();
//		Find the maximum hit
	A = 0;
	maxHit = -1;
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bSiPm && Energy(i) > A) {
		A = Energy(i);
		maxHit = i;
	}
	if (maxHit < 0) {	// nothing to do - no usable SiPM hits
//		DanssEvent.PositronFlags |= PFLAG_NOCLUSTER;
		return;
	}
	HitFlag[maxHit] = 10;
//		Find cluster
	for (k=0; k<MAXCLUSTITER; k++) {
		repeat = 0;
		for (i=0; i<N; i++) if (HitFlag[i] >= 10) for (j=0; j<N; j++) 
			if (HitFlag[j] >= 0 && HitFlag[j] < 10 && user->type(j) == bSiPm && IsNeighbor(i, j)) {
			HitFlag[j] = 20;
			repeat = 1;
		}
		if (!repeat) break;
	}
//		Find cluster position
	x = y = z = 0;
	nx = ny = 0;
	for (i=0; i<N; i++) if (HitFlag[i] >= 10) {
		DanssEvent.PositronHits++;
		if (user->side(i) == 'X') {
			x += user->firstCoord(i) * fStripWidth * Energy(i);
			z += user->zCoord(i) * fStripHeight * Energy(i);
			nx += Energy(i);
		} else {
			y += user->firstCoord(i) * fStripWidth * Energy(i);
			z += user->zCoord(i) * fStripHeight * Energy(i);
			ny += Energy(i);
		}
	}
	DanssEvent.PositronX[0] = (nx > 0) ? x / nx : -1;		// Coordinate is unknown
	DanssEvent.PositronX[1] = (ny > 0) ? y / ny : -1;		// Coordinate is unknown
	DanssEvent.PositronX[2] = (nx + ny > 0) ? z / (nx + ny) : -1;	// Coordinate is unknown
//		Find corrected energy
//	Step 1: Count SiPM
	for (i=0; i<N; i++) if (HitFlag[i] >= 10) {
		if (user->side(i) == 'X') {
			DanssEvent.PositronSiPmEnergy += acorr(Energy(i), DanssEvent.PositronX[1], 'X', bSiPm);
		} else {
			DanssEvent.PositronSiPmEnergy += acorr(Energy(i), DanssEvent.PositronX[0], 'Y', bSiPm);
		}
	}
//	Step 2: Count PMT
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bPmt) {
		for (j=0; j<N; j++) if (IsInModule(j, i) && HitFlag[j] >= 10) break;
		if (j >= N) continue;
		HitFlag[i] = 5;
		if (user->side(i) == 'X') {
			DanssEvent.PositronPmtEnergy += acorr(Energy(i), DanssEvent.PositronX[1], 'X', bPmt);
		} else {
			DanssEvent.PositronPmtEnergy += acorr(Energy(i), DanssEvent.PositronX[0], 'Y', bPmt);
		}
	}
//	Step 3: Subtract gammas in PMT
	if (!(iFlags & FLG_NOPMTCORR)) for (i=0; i<N; i++) if (HitFlag[i] >= 0 && HitFlag[i] < 10 && user->type(i) == bSiPm) {
		for (j=0; j<N; j++) if (IsInModule(i, j) && HitFlag[j] == 5) break;
		if (j >= N) continue;
		if (user->side(i) == 'X') {
			DanssEvent.PositronPmtEnergy -= acorr(Energy(i), DanssEvent.PositronX[1], 'X', bSiPm);
		} else {
			DanssEvent.PositronPmtEnergy -= acorr(Energy(i), DanssEvent.PositronX[0], 'Y', bSiPm);
		}
	}
	DanssEvent.PositronEnergy = DanssEvent.PositronSiPmEnergy + DanssEvent.PositronPmtEnergy;
//	Step 4: Divide by 2, because we count SiPM + PMT
	DanssEvent.PositronEnergy /= 2;
//	Calculate Total energy with longitudinal correction
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && (user->type(i) == bPmt || user->type(i) == bSiPm)) {
		if (user->side(i) == 'X') {
			DanssEvent.TotalEnergy += acorr(Energy(i), DanssEvent.PositronX[1], 'X', user->type(i));
		} else {
			DanssEvent.TotalEnergy += acorr(Energy(i), DanssEvent.PositronX[0], 'Y', user->type(i));
		}
	}
	DanssEvent.TotalEnergy /= 2;	// PMT + SiPM
//	if (FLG_POSECORRECTIONB & iFlags) DanssEvent.TotalEnergy = MCTotalCorrection(DanssEvent.TotalEnergy);
//
//		Count possible gammas
	A = 0;
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && HitFlag[i] < 5) switch(user->type(i)) {
	case bSiPm:
		DanssEvent.AnnihilationGammas++;	// Add SiPm hits
		if (user->side(i) == 'X') {
			E = acorr(Energy(i), DanssEvent.PositronX[1], 'X', bSiPm);
		} else {
			E = acorr(Energy(i), DanssEvent.PositronX[0], 'Y', bSiPm);
		}
		DanssEvent.AnnihilationEnergy += E;
		if (A < E) A = E;
		break;
	case bPmt:
		// We will add Pmt hit only if there is no SiPm hit
		for (j=0; j < N; j++) if (HitFlag[j] >= 0 && user->type(i) == bSiPm && IsInModule(j, i)) break;
		if (j >= N) DanssEvent.AnnihilationGammas++;
		// Add PMT energy
		for (j=0; j < N; j++) if (HitFlag[j] >= 0 && HitFlag[j] < 5 && user->type(i) == bSiPm && IsInModule(j, i)) break;
		if (user->side(i) == 'X') {
			E = acorr(Energy(i), DanssEvent.PositronX[1], 'X', bPmt);
		} else {
			E = acorr(Energy(i), DanssEvent.PositronX[0], 'Y', bPmt);
		}
		DanssEvent.AnnihilationEnergy += E;
		// Subtruct SiPMs in cluster if any
		for (j=0; j < N; j++) if (HitFlag[j] >= 10 && user->type(i) == bSiPm && IsInModule(j, i)) {
			if (user->side(j) == 'X') {
				E = acorr(Energy(j), DanssEvent.PositronX[1], 'X', bSiPm);
			} else {
				E = acorr(Energy(j), DanssEvent.PositronX[0], 'Y', bSiPm);
			}
			DanssEvent.AnnihilationEnergy -= E;
		}
	}
	DanssEvent.AnnihilationMax = A;
	DanssEvent.AnnihilationEnergy /= 2;	// (SiPm + Pmt) / 2
	
//		Do energy correction based on MC taking into account number of hits in the cluster - Deprecated
//	if (FLG_POSECORRECTIONA & iFlags) DanssEvent.PositronEnergy = HitNumberCorrection(DanssEvent.PositronEnergy, DanssEvent.PositronHits);
//	if (FLG_POSECORRECTIONB & iFlags) DanssEvent.PositronEnergy = MCAverageCorrection(DanssEvent.PositronEnergy);
//		Find Z-distance to the closest gamma
	A = 1000;
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && HitFlag[i] < 10 && user->type(i) == bSiPm && fabs(user->zCoord(i) * fStripHeight - DanssEvent.PositronX[2]) < A) 
		A = fabs(user->zCoord(i) * fStripHeight - DanssEvent.PositronX[2]);
	DanssEvent.MinPositron2GammaZ = A;
}

void CalculateNeutron(void)
{
	float x, y, z, r;
	int nx, ny;
	float exSiPm, eySiPm, exPmt, eyPmt;
	int i, j, N;

	N = user->nhits();
//	Find the center (1st approximation)
	x = y = z = 0;
	nx = ny = 0;
	exSiPm = eySiPm = exPmt = eyPmt = 0;
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bSiPm) {
		if (user->side(i) == 'X') {
			x += user->firstCoord(i) * fStripWidth;
			z += user->zCoord(i) * fStripHeight;
			exSiPm += Energy(i);
			nx++;
		} else {
			y += user->firstCoord(i) * fStripWidth;
			z += user->zCoord(i) * fStripHeight;
			eySiPm += Energy(i);
			ny++;
		}
	}
	DanssEvent.NeutronX[0] = (nx) ? x / nx : -1;			// 50 cm is DANSS center
	DanssEvent.NeutronX[1] = (ny) ? y / ny : -1;			// 50 cm is DANSS center
	DanssEvent.NeutronX[2] = (nx + ny) ? z / (nx + ny) : -1;	// 50 cm is DANSS center
//		Calculate lognitudinal correction
	if (nx && ny) {
		for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bPmt) {
			if (user->side(i) == 'X') {
				exPmt += Energy(i);
			} else {
				eyPmt += Energy(i);
			}
		}
		DanssEvent.NeutronEnergy = (
			acorr(exSiPm, DanssEvent.NeutronX[1], 'X', bSiPm) +
			acorr(eySiPm, DanssEvent.NeutronX[0], 'Y', bSiPm) +
			acorr(exPmt, DanssEvent.NeutronX[1], 'X', bPmt) +
			acorr(eyPmt, DanssEvent.NeutronX[0], 'Y', bPmt)
		) / 2;
	} else {
		DanssEvent.NeutronEnergy = (DanssEvent.SiPmCleanEnergy + DanssEvent.PmtCleanEnergy) / 2;
	}
	DanssEvent.NeutronHits = DanssEvent.SiPmCleanHits;
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bPmt) {
		for (j=0; j<N; j++) if (HitFlag[j] >= 0 && user->type(j) == bSiPm && IsInModule(j, i)) break;
		if (j >= N) DanssEvent.NeutronHits++;	// count Pmt hit if it was not counted by SiPm
	}
}

// Clean hits:
// - SiPM with zero or less number of pixels
// - bad (not a number) or not positive energy
// - bad time
// - from marked bad channels
// - from dead channel list
void CleanZeroes(void)
{
	int i, N;

	N = user->nhits();
	for (i=0; i<N; i++) if ((user->type(i) == bSiPm && user->npix(i) <= 0) || (!isfinite(user->e(i))) ||
		Energy(i) <= 0 || HitTime(i) < -1000 || user->isBadChannel(user->chanIndex(i)) || DeadList[user->adc(i)-1][user->adcChan(i)]) {
		HitFlag[i] = -10;
		DanssInfo.Cuts[0]++;
	}
}

/*	Clean only SiPM by PMT confirmation		*
*	Require PMT confirmation for ALL SiPM hits	*/
void CleanByConfirmation(void)
{
	int i, j, N;
	
	N = user->nhits();
	for (i=0; i<N; i++) if (user->type(i) == bSiPm) {
		for (j=0; j<N; j++) if (HitFlag[j] >= 0 && user->type(j) == bPmt && IsInModule(i, j)) break;
		if (j < N) continue;
		HitFlag[i] = -3;
		DanssInfo.Cuts[3]++;
	}
//		"early" hits
	for (i=0; i<N; i++) if (HitFlag[i] == -100)
	{
		for (j=0; j<N; j++) if (HitFlag[j] >= 0 && user->type(j) == bPmt && IsInModule(i, j)) break;
		if (j < N) continue;
		HitFlag[i] = -3;
	}
//		clean time
	for (i=0; i<N; i++) if (HitFlag[i] >= -1 && user->type(i) == bSiPm) {
		hSiPMtimeClean[0]->Fill(HitTime(i) - DanssEvent.fineTime);
		if (user->npix(i) < 1.5) {
			hSiPMtimeClean[1]->Fill(HitTime(i) - DanssEvent.fineTime);
		} else if (user->npix(i) < 2.5) {
			hSiPMtimeClean[2]->Fill(HitTime(i) - DanssEvent.fineTime);
		} else if (user->npix(i) < 5.5) {
			hSiPMtimeClean[3]->Fill(HitTime(i) - DanssEvent.fineTime);
		} else if (user->npix(i) < 10.5) {
			hSiPMtimeClean[4]->Fill(HitTime(i) - DanssEvent.fineTime);
		} else {
			hSiPMtimeClean[5]->Fill(HitTime(i) - DanssEvent.fineTime);
		}
	}
}

void Clean1Pixel(void)
{
	int i, N;
	N = user->nhits();
	for (i=0; i<N; i++) if (user->type(i) == bSiPm) if(user->npix(i) < 1.5) HitFlag[i] = -2;
}

void CleanByTime(void)
{
	int i, N;
	float tearly;
	
	N = user->nhits();
	if (DanssEvent.fineTime != NOFINETIME) {
		for (i=0; i<N; i++) if (HitFlag[i] >= 0) switch (user->type(i)) {
		case bSiPm:
			hSiPMtime[0]->Fill(HitTime(i) - DanssEvent.fineTime);
			if (user->npix(i) < 1.5) {
				hSiPMtime[1]->Fill(HitTime(i) - DanssEvent.fineTime);
			} else if (user->npix(i) < 2.5) {
				hSiPMtime[2]->Fill(HitTime(i) - DanssEvent.fineTime);
			} else if (user->npix(i) < 5.5) {
				hSiPMtime[3]->Fill(HitTime(i) - DanssEvent.fineTime);
			} else if (user->npix(i) < 10.5) {
				hSiPMtime[4]->Fill(HitTime(i) - DanssEvent.fineTime);
			} else {
				hSiPMtime[5]->Fill(HitTime(i) - DanssEvent.fineTime);
			}
			if (IsMc) {
				if ((HitTime(i) - DanssEvent.fineTime) < TMCCUTMIN ||
					(HitTime(i) - DanssEvent.fineTime) > TMCCUTMAX) {
					HitFlag[i] = -1;
					DanssInfo.Cuts[5]++;
				}
			} else {
				if (fabs(HitTime(i) - DanssEvent.fineTime) > TCUT) {
					HitFlag[i] = -1;
					DanssInfo.Cuts[5]++;
				}
			}
			break;
		case bPmt:
		case bVeto:
			if (fabs(HitTime(i) - DanssEvent.fineTime) > TCUTPMT && (iFlags & FLG_PMTTIMECUT)) {
				HitFlag[i] = -1;
				DanssInfo.Cuts[6]++;
			}
			break;
		}
		tearly = DanssEvent.fineTime - SIPMEARLYTIME;
	} else {
		tearly = SOMEEARLYTIME;
	}
	for (i=0; i<N; i++) if (user->type(i) == bSiPm && HitFlag[i] >= -5) {
		if (IsMc ) {
			if ((HitTime(i) - tearly) > TMCCUTMIN &&
				(HitTime(i) - tearly) < TMCCUTMAX) HitFlag[i] = -100;	// mark early hit candidates
		} else {
			if (fabs(HitTime(i) - tearly) <= TCUT) HitFlag[i] = -100;	// mark early hit candidates
		}
	}
}

void CorrectEnergy(double scale)
{
	int i;
	DanssEvent.VetoCleanEnergy *= scale;
	DanssEvent.BottomLayersEnergy *= scale;
	DanssEvent.PmtCleanEnergy *= scale;
	DanssEvent.SiPmEnergy *= scale;
	DanssEvent.SiPmCleanEnergy *= scale;
	DanssEvent.SiPmEarlyEnergy *= scale;
	DanssEvent.PositronEnergy *= scale;
	DanssEvent.TotalEnergy *= scale;
	DanssEvent.PositronSiPmEnergy *= scale;
	DanssEvent.PositronPmtEnergy *= scale;
	DanssEvent.AnnihilationEnergy *= scale;
	DanssEvent.AnnihilationMax *= scale;
	DanssEvent.NeutronEnergy *= scale;

	for (i=0; i<DanssEvent.NHits; i++) HitArray.E[i] *= scale;
}

void CreateDeadList(char *fname, int run)
{
	int i, j;
	char str[16*1024];
	char *ptr;
	FILE *f;
	int from, to;
	int chan;
	int cnt = 0;

	memset(DeadList, 0, sizeof(DeadList));
	if (!fname) return;
	f = fopen(fname, "rt");
	if (!f) {
		printf("Dead list file %s not found.\n", fname);
		return;
	}
	for (;;) {
		if (!fgets(str, sizeof(str), f)) break;
		ptr = strtok(str, " \t\n");
		if (!ptr) continue;
		from = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t\n");
		if (!ptr) continue;
		to = strtol(ptr, NULL, 10);
		if (run < from || run > to) continue;	// this is not the list for our run
		for(;;) {
			ptr = strtok(NULL, " \t\n");
			if (!ptr) break;	// end of dead channels list
			chan = 100 * (strtod(ptr, NULL) + 0.001);
			i = chan /100;
			j = chan % 100;
			if (i > 0 && i <= MAXADCBOARD && j >= 0 && j < iNChannels_AdcBoard) {
				DeadList[i-1][j] = 1;
				cnt++;
			}
		}
		break;
	}
	fclose(f);
	printf("Run %d: %d channels masked out.\n", run, cnt);
}

void DebugFullPrint(void)
{
	int i, N;
	time_t tm;

	N = user->nhits();
	tm = DanssEvent.unixTime;
	printf("******************************************************************************************************************\n");
	printf("Event: %Ld globalTime: %Ld fineTime: %6.1f ns   linux time: %s", 
		DanssEvent.number, DanssEvent.globalTime, DanssEvent.fineTime, ctime(&tm));
	printf("Total %d hits: %d SiPM %d PMT %d Veto; Clean: %d SiPM %d PMT %d Veto\n", 
		N, DanssExtra.SiPmHits, DanssExtra.PmtHits, DanssExtra.VetoHits, 
		DanssEvent.SiPmCleanHits, DanssEvent.PmtCleanHits, DanssEvent.VetoCleanHits);
	printf("Energy: %6.1f SiPM %6.1f PMT %6.1f Veto; Clean: %6.1f SiPM %6.1f PMT %6.1f Veto\n", 
		DanssExtra.SiPmEnergy, DanssExtra.PmtEnergy, DanssExtra.VetoEnergy, 
		DanssEvent.SiPmCleanEnergy, DanssEvent.PmtCleanEnergy, DanssEvent.VetoCleanEnergy);
	if (N) {
		printf("N    Type  N  S       E    time  ADC.Ch side XY Z  Flag\n");
//			1234512345123412345678123451234561231123123451231231234
		for(i=0; i<N; i++) switch(user->type(i)) {
		case bSiPm:
			printf("%4d SiPM %3.0f %7.1f %4.1f %5.1f %2d.%2.2d    %c  %2d %2d  %c\n", i+1, user->npix(i), user->signal(i),
				Energy(i), user->adc(i), HitTime(i), user->adcChan(i), user->side(i), user->firstCoord(i), user->zCoord(i),
				(HitFlag[i]<0) ? 'X' : ' ');
			break;
		case bPmt:
			printf("%4d PMT      %7.1f %4.1f %5.1f %2d.%2.2d    %c  %2d %2d  %c\n", i+1, user->signal(i),
				Energy(i), HitTime(i), user->adc(i), user->adcChan(i), user->side(i), user->firstCoord(i), user->zCoord(i),
				(HitFlag[i]<0) ? 'X' : ' ');
			break;
		case bVeto:
			printf("%4d VETO     %7.1f %4.1f %5.1f %2d.%2.2d    -  xx xx  %c\n", i+1, user->signal(i),
				Energy(i), HitTime(i), user->adc(i), user->adcChan(i),
				(HitFlag[i]<0) ? 'X' : ' ');
			break;
		}
	}
}

void DumpEvent(void)
{
	int i, N;
	char str[1024];
	
	DebugFullPrint();

	sprintf(str, "evt_%Ld.root", DanssEvent.globalTime);
	TFile *f = new TFile(str, "RECREATE");
	TH2D *SiPmX = new TH2D("hSiPmX", "SiPm X-side", 25, 0, 100, 50, 0, 100);
	TH2D *SiPmY = new TH2D("hSiPmY", "SiPm Y-side", 25, 0, 100, 50, 0, 100);
	TH2D *SiPmCleanX = new TH2D("hSiPmCleanX", "SiPm clean X-side", 25, 0, 100, 50, 0, 100);
	TH2D *SiPmCleanY = new TH2D("hSiPmCleanY", "SiPm clean Y-side", 25, 0, 100, 50, 0, 100);
	TH2D *PmtX  = new TH2D("hPmtX" , "Pmt  X-side", 5, 0, 100, 5, 0, 100);
	TH2D *PmtY  = new TH2D("hPmtY" , "Pmt  Y-side", 5, 0, 100, 5, 0, 100);
	TH1D *Veto  = new TH1D("hVeto", "Veto channels", 64, 0, 64);
	TH1D *Time  = new TH1D("hTime", "Raw time", 200, 100, 300);
	TH1D *TimeClean = new TH1D("hTimeClean", "Raw time clean", 200, 100, 300);
	TH1D *Par   = new TH1D("hPar", "Parameters", 20, 0, 20);
	
	Par->GetXaxis()->SetBinLabel(1, "fineTime");
	Par->GetXaxis()->SetBinLabel(2, "SiPmEnergy");
	Par->GetXaxis()->SetBinLabel(3, "PmtEnergy");
	Par->GetXaxis()->SetBinLabel(4, "VetoEnergy");
	Par->GetXaxis()->SetBinLabel(5, "PositronEnergy");
	Par->GetXaxis()->SetBinLabel(6, "SiPmHits");
	Par->GetXaxis()->SetBinLabel(7, "PmtHits");
	Par->GetXaxis()->SetBinLabel(8, "VetoHits");
	Par->GetXaxis()->SetBinLabel(9, "PositronHits");
	Par->GetXaxis()->SetBinLabel(10, "PositronX");
	Par->GetXaxis()->SetBinLabel(11, "PositronY");
	Par->GetXaxis()->SetBinLabel(12, "PositronZ");
	Par->GetXaxis()->SetBinLabel(13, "McX");
	Par->GetXaxis()->SetBinLabel(14, "McY");
	Par->GetXaxis()->SetBinLabel(15, "McZ");
	
	Par->Fill("fineTime", DanssEvent.fineTime);
	Par->Fill("SiPmEnergy", DanssEvent.SiPmCleanEnergy);
	Par->Fill("PmtEnergy", DanssEvent.PmtCleanEnergy);
	Par->Fill("VetoEnergy", DanssEvent.VetoCleanEnergy);
	Par->Fill("PositronEnergy", DanssEvent.PositronEnergy);
	Par->Fill("SiPmHits", DanssEvent.SiPmCleanHits);
	Par->Fill("PmtHits", DanssEvent.PmtCleanHits);
	Par->Fill("VetoHits", DanssEvent.VetoCleanHits);
	Par->Fill("PositronHits", DanssEvent.PositronHits);
	Par->Fill("PositronX", DanssEvent.PositronX[0]);
	Par->Fill("PositronY", DanssEvent.PositronX[1]);
	Par->Fill("PositronZ", DanssEvent.PositronX[2]);
	if (IsMc) {
		Par->Fill("McX", DanssMc.X[0]);
		Par->Fill("McY", DanssMc.X[1]);
		Par->Fill("McZ", DanssMc.X[2]);
	} else {
		Par->Fill("McX", -1.0);
		Par->Fill("McY", -1.0);
		Par->Fill("McZ", -1.0);
	}

	N = user->nhits();
	for(i=0; i<N; i++) {
		switch(user->type(i)) {
		case bSiPm:
			if (user->side(i) == 'X') {
				SiPmX->Fill(user->firstCoord(i)*4.0 + 2.0, user->zCoord(i)+0.5, Energy(i));
				if (HitFlag[i] >= 0) SiPmCleanX->Fill(user->firstCoord(i)*4.0 + 2.0, user->zCoord(i)+0.5, Energy(i));
			} else {
				SiPmY->Fill(user->firstCoord(i)*4.0 + 2.0, user->zCoord(i)+0.5, Energy(i));
				if (HitFlag[i] >= 0) SiPmCleanY->Fill(user->firstCoord(i)*4.0 + 2.0, user->zCoord(i)+0.5, Energy(i));
			}
			break;
		case bPmt:
			if (user->side(i) == 'X') {
				PmtX->Fill(user->firstCoord(i)*20.0 + 10.0, user->zCoord(i)*20.0+10.0, Energy(i));
			} else {
				PmtY->Fill(user->firstCoord(i)*20.0 + 10.0, user->zCoord(i)*20.0+10.0, Energy(i));
			}
			break;
		case bVeto:
			Veto->Fill(user->adcChan(i), Energy(i));
			break;
		}
		Time->Fill(HitTime(i), Energy(i));
		if (HitFlag[i] >= 0) TimeClean->Fill(HitTime(i), Energy(i));
	}
	SiPmX->Write();
	SiPmY->Write();
	SiPmCleanX->Write();
	SiPmCleanY->Write();
	PmtX->Write();
	PmtY->Write();
	Veto->Write();
	Time->Write();
	TimeClean->Write();
	Par->Write();
	
	delete SiPmX;
	delete SiPmY;
	delete SiPmCleanX;
	delete SiPmCleanY;
	delete PmtX;
	delete PmtY;
	delete Veto;
	delete Time;
	delete TimeClean;
	delete Par;
	
	f->Close();
}

void FillTimeHists(void) 
{
	int i, N;
	N = user->nhits();
	for (i=0; i<N; i++) if (Energy(i) > MINENERGY4TIME && HitTime(i) > 0) {
		hTimeDelta[user->adc(i)-1][user->adcChan(i)]->Fill(HitTime(i) - DanssEvent.fineTime);
//		time relative to PMT - we need some common calibration of delays
		if (PmtFineTime > 0) hPMTTimeDelta[user->adc(i)-1][user->adcChan(i)]->Fill(HitTime(i) - PmtFineTime);
	}
}

void FindFineTime(void)
{
	double tsum;
	double asum;
	double e;
	int i, k, n, N;
	double PMTsumT;
	double PMTsumA;
	
	tsum = asum = 0;
	PMTsumT = PMTsumA = 0;
	k = 0;
	n = 0;
	N = user->nhits();
	for (i=0; i<N; i++) if (HitFlag[i] >= 0) {
		switch(user->type(i)) {
		case bSiPm:
			e = Energy(i);
			if (user->npix(i) < MINSIPMPIXELS) e = 0;
			break;
		case bPmt:
			e = Energy(i);
			PMTsumT += HitTime(i) * e;
			PMTsumA += e;
			n++;
			break;
		case bVeto:
			e = 0;	// exclude veto from time averaging
			break;
		}
		if (e > MINENERGY4TIME && HitTime(i) > MINAVRTIME) {
			tsum += HitTime(i) * e;
			asum += e;
			k++;
		}
	}
	DanssEvent.fineTime = (asum > 0) ? tsum / asum : NOFINETIME;	// some large number if not usable hits found
	if (DanssEvent.trigType == masterTrgRandom) DanssEvent.fineTime = 200;	// some fixed good time
	PmtFineTime = -1;
	if (n >= 2 && PMTsumA > 2) PmtFineTime = PMTsumT / PMTsumA;
}


// Do additional smear of energy for MC events
void MCSmear(void)
{
	DanssEvent.PositronEnergy = MCEnergySmear(DanssEvent.PositronEnergy);
	DanssEvent.TotalEnergy = MCEnergySmear(DanssEvent.TotalEnergy);
	DanssEvent.AnnihilationEnergy = MCEnergySmear(DanssEvent.AnnihilationEnergy);
	DanssEvent.NeutronEnergy = MCEnergySmear(DanssEvent.NeutronEnergy);
}

void StoreHits(void)
{
	int i, j, N;

	j = 0;
	N = user->nhits();
	for (i=0; i<N; i++) if (HitFlag[i] >= 0) {
		HitArray.E[j] = Energy(i);
		HitArray.T[j] = HitTime(i);
		HitArray.type[j].type = user->type(i);
		HitArray.type[j].flag = HitFlag[i];
		switch (user->type(i)) {
		case bSiPm:
			HitArray.type[j].z = user->zCoord(i);
			HitArray.type[j].xy = user->firstCoord(i);
			break;
		case bPmt:
			HitArray.type[j].z = 2 * user->zCoord(i);
			if (user->side(i) == 'X') HitArray.type[j].z++;
			HitArray.type[j].xy = user->firstCoord(i);
			break;
		case bVeto:
			HitArray.type[j].z = user->adcChan(i);
			HitArray.type[j].xy = 0;
			break;
		}
		j++;
	}
	DanssEvent.NHits = j;
	
	DanssInfo.hits[0] += N;
	DanssInfo.hits[1] += j;
}

void SumClean(void)
{
	int i, N;
	
	N = user->nhits();
	for (i=0; i<N; i++) if (HitFlag[i] >= 0) switch (user->type(i)) {
	case bSiPm:
		DanssEvent.SiPmCleanHits++;
		DanssEvent.SiPmCleanEnergy += Energy(i);
		if (user->zCoord(i) < NBOTTOMLAYERS) DanssEvent.BottomLayersEnergy += Energy(i);
		if (user->npe(i) > 3) hCrossTalk->Fill(user->npix(i) / user->npe(i));
		if (IsMc && user->e_McTruth(i) > 0.3) {
			hEtoEMC->Fill(Energy(i) / user->e_McTruth(i));
			hNPEtoEMC->Fill(user->npe(i) / user->e_McTruth(i));
		}
//		DanssEvent.SiPmCleanEnergy += (iFlags & FLG_EAMPLITUDE) ? user->siPmAmp(user->side(i), user->firstCoord(i), user->zCoord(i)) : Energy(i);
//		Calculate asymmetry, clean hits 
		if (user->side(i) == 'X') {
			DanssEvent.NXYSiPmClean++;
		} else {
			DanssEvent.NXYSiPmClean--;
		}
		break;
	case bPmt:
		DanssEvent.PmtCleanHits++;
		DanssEvent.PmtCleanEnergy += Energy(i);
		if (Energy(i) > 0) hPMTAmpl[user->adcChan(i)]->Fill(user->signal(i) / Energy(i));
//		DanssEvent.PmtCleanEnergy += (iFlags & FLG_EAMPLITUDE) ? user->pmtAmp(user->side(i), user->firstCoord(i), user->zCoord(i)) : Energy(i);
//		Calculate asymmetry, clean hits 
		if (user->side(i) == 'X') {
			DanssEvent.NXYPmt++;
		} else {
			DanssEvent.NXYPmt--;
		}
		break;
	case bVeto:
		DanssEvent.VetoCleanHits++;
		DanssEvent.VetoCleanEnergy += Energy(i);
//		DanssEvent.VetoCleanEnergy += (iFlags & FLG_EAMPLITUDE) ? user->vetoAmp(user->indexByHit(i)) : Energy(i);
		break;
	}

	for (i=0; i<N; i++) if (HitFlag[i] == -100) {
		DanssEvent.SiPmEarlyHits++;
		DanssEvent.SiPmEarlyEnergy += Energy(i);
	}
}

void SumEverything(void)
{
	int i, N;
	
	N = user->nhits();
	for (i=0; i<N; i++) switch (user->type(i)) {
	case bSiPm:
		DanssExtra.SiPmHits++;
		DanssExtra.SiPmEnergy += Energy(i);
		if (user->side(i) == 'X') {
			DanssEvent.NXYSiPmRaw++;
		} else {
			DanssEvent.NXYSiPmRaw--;
		}
		break;
	case bPmt:
		DanssExtra.PmtHits++;
		DanssExtra.PmtEnergy += Energy(i);
		break;
	case bVeto:
		DanssExtra.VetoHits++;
		DanssExtra.VetoEnergy += Energy(i);
		break;
	}
	
	DanssEvent.SiPmHits = DanssExtra.SiPmHits;
	DanssEvent.SiPmEnergy = DanssExtra.SiPmEnergy;
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
 *  Read time calibration file of the format:
 *  BB.CC NNNNNN MMMMMM FFFF
 *  BB - board number
 *  CC - channel number
 *  NNNNNN - run range begin
 *  MMMMMM - run range end
 *  FFFF   - float number of the delay
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
//	for(i = 100; i < iNElements; i++) {
//		iAdcNum = i / 100;
//		iAdcChan = i % 100;
//		if(!isAdcChannelExist(iAdcNum, iAdcChan)) continue;
//		setTd(i, 0); // set all td = 0
//	}
//	Set MC time calibration
	if (IsMc) {
		printf("Setting delays for MC: SiPM = %6.2f,  PMT = %6.2f\n", MCSIPMDELAY, MCPMTDELAY);
		for(i = 100; i < iNElements; i++) {
			iAdcNum = i / 100;
			iAdcChan = i % 100;
			if(!isAdcChannelExist(iAdcNum, iAdcChan)) continue;
			val = (iAdcNum == 1 || iAdcNum == 3) ? MCPMTDELAY : MCSIPMDELAY;
//			if (i < 800) printf("%d %d %d => %f\n", i, iAdcNum, iAdcChan, val);
			setTd(i, val);
		}
//		getchar();
		return;
	}

	if (!chTimeCalibration) return;

//	Read and implement tcalib file
	f = fopen(chTimeCalibration, "rt");
	if (!f) {
		printf("Can not open file %s: %m\n", chTimeCalibration);
		return;
	}
	k = 0;
	for(;;) {
		ptr = fgets(str, sizeof(str), f);
		if (!ptr) break;		// EOF or error
		if (strlen(str) < 2) continue;	// empty line
		if (!isdigit(str[0])) {
			printf("Time calibration: %s", str);
			continue;	// Comment ?
		}
		i = 100 * (strtod(ptr, &ptr) + 0.002);
		iAdcNum = i / 100;
		iAdcChan = i % 100;
		iFirst = strtol(ptr, &ptr, 10);
		iLast = strtol(ptr, &ptr, 10);
		val = strtod(ptr, &ptr);
		if (iFirst > runnumber() || iLast < runnumber()) continue;	// not for our run
		if(!isAdcChannelExist(iAdcNum, iAdcChan)) {	// non-existing channel - strange
			printf("Time calibration wrong channel i=%d (%d.%d): %s", i, iAdcNum, iAdcChan, str);
			continue;
		}
		setTd(i, val);
		k++;
	}

	printf("Time calibration used: %s. %d channels found.\n", chTimeCalibration, k);

	fclose(f);
//	getchar();
}

//------------------------------->

void Help(void)
{
	printf("\tDANSS offline: digi event builder. Version %s\n", MYVERSION);
	printf("Process events and create root-tree with event parameters.\n");
	printf("\tOptions:\n");
	printf("-calib filename.txt     --- file with energy calibration. No default.\n");
	printf("-deadlist filename.txt  --- file with explicit list of dead channels.\n");
	printf("-dump gTime             --- dump an event with this gTime.\n");
	printf("-ecorr EnergyCorrection --- 0.96 by default.\n");
	printf("-ecorrsipm SiPMEnergyCorrection --- 1.08 by default - to correct for Ira's wrong dealing with SiPM waveforms.\n");
	printf("-events number          --- stop after processing this number of events. Default - do not stop.\n");
	printf("-file filename.txt      --- file with a list of files for processing. No default.\n");
	printf("-flag FLAGS             --- analysis flag mask. Default - 0. Recognized flags:\n");
	printf("\t       1 --- do debugging printout of events;\n");
	printf("\t       2 --- create delta time histograms;\n");
//	printf("\t   0x100 --- do energy correction based on MC taking into account number of hits in the cluster;\n");
//	printf("\t   0x200 --- do energy correction based on MC not taking into account number of hits in the cluster;\n");
	printf("\t  0x1000 --- simulate \"neutron\" correction for MC events;\n");
//	printf("\t 0x10000 --- do not clean small energies;\n");
	printf("\t 0x20000 --- do not do time cut;\n");
	printf("\t 0x40000 --- do not require confirmation for all hits;\n");
//	printf("\t 0x80000 --- do not require confirmation for SiPM single pixel hits;\n");
	printf("\t0x100000 --- do not correct PMT cluster energy for out of cluster SiPM hits.\n");
	printf("\t0x200000 --- Check PMT and VETO time.\n");
	printf("\t0x400000 --- do not confirm all SiPM hits.\n");
	printf("\t0x800000 --- Do aaditional energy smear for MC events (IBD simulation assumed)\n");
	printf("-help                   --- print this message and exit.\n");
	printf("-hitinfo hitinfo.txt[.bz2] --- add raw hits information tree.\n");
	printf("-mcdata                 --- this is Monte Carlo data - create McTruth branch.\n");
	printf("-mcfile mc.root         --- this is Monte Carlo data - create McTruth and McEvent branches.\n");
	printf("          Copy information from mc.root:DANSSEvent tree.\n");
	printf("-mcscale scale          --- scale energy factor for MC.\n");
	printf("-seed SEED              --- seed for random number generator.\n");
	printf("-smear stohastic_term impurity term --- additional smear for MC events.\n");
	printf("-output filename.root   --- output file name. Without this key output file is not written.\n");
	printf("-tcalib filename.txt    --- file with the time calibration.\n");
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
	char *DeadListName;
	char *RawHitsFileName;
	char *McRootFileName;
	int RandomSeed = 17321;
	
	user = this;
	MCsmear.st = 0.12;	// 12%/sqrt(E)
	MCsmear.ct = 0.04;	// 4%
	progStartTime = time(NULL);
	chOutputFile = NULL;
	OutputFile = NULL;
	OutputTree = NULL;
	InfoTree = NULL;
	dumpgTime = -1;
	chTimeCalibration = NULL;
	DeadListName = NULL;
	iFlags = 0;
	MaxEvents = -1;
	IsMc = 0;
	EnergyCorrection = 0.96;
	SiPMEnergyCorrection = 1.08;
	MCEnergyCorrection = 1.00;
	RawHitsFileName = NULL;
	RawHitsTree = NULL;
	RawHitsArray = NULL;
	McRootFileName = NULL;
	McFile = NULL;
	McEventTree = NULL;

	for (i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-output")) {
			i++;
			chOutputFile = (char *)argv[i];
		} else if (!strcmp(argv[i], "-tcalib")) {
			i++;
			chTimeCalibration = (char *)argv[i];
		} else if (!strcmp(argv[i], "-deadlist")) {
			i++;
			DeadListName = (char *)argv[i];
		} else if (!strcmp(argv[i], "-ecorr")) {
			i++;
			EnergyCorrection = strtod(argv[i], NULL);
		} else if (!strcmp(argv[i], "-ecorrsipm")) {
			i++;
			SiPMEnergyCorrection = strtod(argv[i], NULL);
		} else if (!strcmp(argv[i], "-events")) {
			i++;
			MaxEvents = strtol(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-flag")) {
			i++;
			iFlags = strtol(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-hitinfo")) {
			i++;
			RawHitsFileName = (char *)argv[i];
		} else if (!strcmp(argv[i], "-mcdata")) {
			IsMc = 1;
		} else if (!strcmp(argv[i], "-mcfile")) {
			IsMc = 1;
			i++;
			McRootFileName = (char *)argv[i];
		} else if (!strcmp(argv[i], "-mcscale")) {
			i++;
			MCEnergyCorrection = strtod(argv[i], NULL);
		} else if (!strcmp(argv[i], "-seed")) {
			i++;
			RandomSeed = strtol(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-smear")) {
			i++;
			MCsmear.st = strtod(argv[i], NULL);
			i++;
			MCsmear.ct = strtod(argv[i], NULL);
		} else if (!strcmp(argv[i], "-help")) {
			Help();
			exit(0);
		} else if (!strcmp(argv[i], "-dump")) {
			i++;
			dumpgTime = strtoll(argv[i], NULL, 0);
		}
	}

	Random = new TRandom2(RandomSeed);

	CreateDeadList(DeadListName, runnumber());
	if (chTimeCalibration) init_Tds();

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
		OutputTree = new TTree("DanssEvent", "Danss event tree");
		OutputTree->Branch("Data", &DanssEvent, 
//		Common parameters
			"globalTime/L:"		// time in terms of 125 MHz
			"number/L:"		// event number in the file
			"unixTime/I:"		// linux time, seconds
			"fineTime/F:"		// fine time of the event (for hit selection)
			"trigType/I:"		// trigger type.
//		Veto parameters
			"VetoCleanHits/I:"	// hits above threshold and in time window
			"VetoCleanEnergy/F:"	// Energy Sum of clean hits
			"BottomLayersEnergy/F:"	// Energy in 2 bottom layers - additional veto event signature
//		PMT parameters
			"PmtCleanHits/I:"
			"PmtCleanEnergy/F:"
//		SiPM parameters
			"SiPmHits/I:"
			"SiPmEnergy/F:"
			"SiPmCleanHits/I:"
			"SiPmCleanEnergy/F:"
			"SiPmEarlyHits/I:"
			"SiPmEarlyEnergy/F:"
//		"positron cluster" parameters
			"PositronHits/I:"	// hits in the cluster
			"PositronEnergy/F:"	// Energy sum of the cluster, corrected, (SiPM + PMT) / 2
			"TotalEnergy/F:"	// Total energy, longitudinally correctd (former Energy of the maximum hit)
			"PositronSiPmEnergy/F:"	// SiPM energy in the cluster, corrected
			"PositronPmtEnergy/F:"	// PMT energy in the cluster, corrected
			"PositronX[3]/F:"	// cluster position
			"AnnihilationGammas/I:"	// number of possible annihilation gammas
			"AnnihilationEnergy/F:"	// Energy in annihilation gammas
			"AnnihilationMax/F:"	// Energy in the maximum annihilation hit
			"MinPositron2GammaZ/F:"	// Z-distance to the closest gamma
//		"neutron" parameters
			"NeutronX[3]/F:"	// center of gammas position
			"NHits/I:"		// Number of hits
			"NeutronEnergy/F:"	// Neutron Energy 
			"NeutronHits/I:"	// Count Neutron hits by SiPm and add Pmt hits where no SiPm
			"NXYSiPmRaw/I:"		// SiPm X-Y asymmetry, raw hits
			"NXYSiPmClean/I:"	// SiPm X-Y asymmetry, clean hits
			"NXYPmt/I"		// Pmt X-Y asymmetry
		);
		OutputTree->Branch("HitE", HitArray.E, "HitE[NHits]/F");
		OutputTree->Branch("HitT", HitArray.T, "HitT[NHits]/F");
		OutputTree->Branch("HitType", HitArray.type, "HitType[NHits]/I");
		if (IsMc) OutputTree->Branch("MC", &DanssMc,
			"McEnergy/F:"		// MC true energy
			"McX[3]/F:"		// MC vertex position
			"DriftTime/F"		// MC time between positron and neutron, us
		);
		
		InfoTree = new TTree("DanssInfo", "Run info tree");	
		InfoTree->Branch("Info", &DanssInfo,  
			"gTime/L:"		// running time in terms of 125 MHz
			"runNumber/I:"		// the run number
			"startTime/I:"		// linux start time, seconds
			"stopTime/I:"		// linux stop time, seconds
			"events/I:"		// number of events
			"position/I:"		// Danss Position type
			"height/F:"		// Danss average height
			"hits[2]/L:"		// total hits [before/after]
			"Cuts[20]/L"		// Cuts statistics
		);
		if (RawHitsFileName && RawHitsArrayInit(RawHitsFileName)) {
			RawHitsTree = new TTree("RawHits", "RawHits");
			RawHitsTree->Branch("RawHits", &RawHits, "PmtCnt/s:VetoCnt/s:SiPmCnt/s");
		}

		if (McRootFileName) {
			McFile = new TFile(McRootFileName);
			if (!McFile->IsOpen()) {
				printf("Can not open raw MC file %s\n", McRootFileName);
				McFile = NULL;
			} else {
				McEventTree = (TTree *) McFile->Get("DANSSEvent");
				if (!McEventTree) {
					printf("Tree DANSSEvent not found in %s\n", McRootFileName);
					McFile->Close();
					delete McFile;
					McFile = NULL;
				} else {
					if (McEventTree->SetBranchAddress("EventData", &McEvent)) {
						printf("Branch DANSSEvent.EventData not found in %s\n", McRootFileName);
						McFile->Close();
						delete McFile;
						McFile = NULL;
						McEventTree = NULL;
					} else {
						McEntries = McEventTree->GetEntries();
						OutputTree->Branch("MCEvent", &McEvent, 
							"EventID/D:"
							"ParticleEnergy/D:"
							"EnergyLoss/D:"
							"DetectorEnergyLoss/D:"
							"CopperEnergyLoss/D:"
							"GdCoverEnergyLoss/D:"
							"X/D:Y/D:Z/D:"
							"DirX/D:DirY/D:DirZ/D:"
							"TimelineShift/D:"
							"FluxFlag/B");
					}
				}
			}
		}
	}
	iNevtTotal = 0;
	upTime = 0;
	fileFirstTime = -1;
	fileLastTime = -1;
	globalTimeWrapped = 0;
	memset(&DanssInfo, 0, sizeof(struct DanssInfoStruct4));
	memset(&RawHits, 0, sizeof(RawHits));
	
	if (iFlags & FLG_DTHIST) {
		for (i=0; i<MAXADCBOARD; i++) for (j=0; j<iNChannels_AdcBoard; j++) {
			sprintf(strs, "hDT%2.2dc%2.2d", i+1, j);
			sprintf(strl, "Time delta distribution for channel %2.2d.%2.2d;ns", i+1, j);
			hTimeDelta[i][j] = new TH1D(strs, strl, 250, -25, 25);
			sprintf(strs, "hDTP%2.2dc%2.2d", i+1, j);
			sprintf(strl, "Time delta distribution for channel %2.2d.%2.2d versus PMT;ns", i+1, j);
			hPMTTimeDelta[i][j] = new TH1D(strs, strl, 250, -25, 25);
		}
	}
	hCrossTalk = new TH1D("hCrossTalk", "Croos talk distribution;Pixels/Ph.e.", 280, 0.8, 2.2);
	hSiPMtime[0] = new TH1D("hSiPMtimeAll", "SiPM time - finetime, all hits;ns;Hits", 500, -40, 60);
	hSiPMtime[1] = new TH1D("hSiPMtime1Px", "SiPM time - finetime, 1 pixel hits;ns;Hits", 500, -40, 60);
	hSiPMtime[2] = new TH1D("hSiPMtime2Px", "SiPM time - finetime, 2 pixels hits;ns;Hits", 500, -40, 60);
	hSiPMtime[3] = new TH1D("hSiPMtime3Px", "SiPM time - finetime, 3-5 pixels hits;ns;Hits", 500, -40, 60);
	hSiPMtime[4] = new TH1D("hSiPMtime6Px", "SiPM time - finetime, 6-10 pixels hits;ns;Hits", 500, -40, 60);
	hSiPMtime[5] = new TH1D("hSiPMtime10Px", "SiPM time - finetime, > 10 pixels hits;ns;Hits", 500, -40, 60);
	hSiPMtimeClean[0] = new TH1D("hSiPMtimeCleanAll", "SiPM time - finetime, all hits;ns;Hits", 500, -40, 60);
	hSiPMtimeClean[1] = new TH1D("hSiPMtimeClean1Px", "SiPM time - finetime, 1 pixel hits;ns;Hits", 500, -40, 60);
	hSiPMtimeClean[2] = new TH1D("hSiPMtimeClean2Px", "SiPM time - finetime, 2 pixels hits;ns;Hits", 500, -40, 60);
	hSiPMtimeClean[3] = new TH1D("hSiPMtimeClean3Px", "SiPM time - finetime, 3-5 pixels hits;ns;Hits", 500, -40, 60);
	hSiPMtimeClean[4] = new TH1D("hSiPMtimeClean6Px", "SiPM time - finetime, 6-10 pixels hits;ns;Hits", 500, -40, 60);
	hSiPMtimeClean[5] = new TH1D("hSiPMtimeClean10Px", "SiPM time - finetime, > 10 pixels hits;ns;Hits", 500, -40, 60);
	for (j=0; j<iNChannels_AdcBoard; j++) {
		sprintf(strs, "hPMTAmpl%2.2d", j);
		sprintf(strl, "PMT amplification: ADC integral units per MeV %2.2d", j);
		hPMTAmpl[j] = new TH1D(strs, strl, 1000, 500, 1500);
	}
	if (IsMc) {
		hEtoEMC = new TH1D("hEtoEMC", "Measured to MC truth energy ratio", 100, 0, 3);
		hNPEtoEMC = new TH1D("hNPEtoEMC", "Photo electrons to MC truth energy ratio", 250, 0, 50);
	}
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
	int McNum;
	long long gt;
	int i;

	if( ttype() != 1 ) return 0;
	
	memset(HitFlag, 0, nhits() * sizeof(int));
	memset(&DanssEvent, 0, sizeof(struct DanssEventStruct7));
	memset(&DanssExtra, 0, sizeof(struct DanssExtraStruct));

	DanssInfo.stopTime = absTime();
	if (fileFirstTime < 0) {
		fileFirstTime = fileLastTime;
		DanssInfo.startTime = DanssInfo.stopTime;
		DanssInfo.events = 0;
	}

	if (fileLastTime > globalTime()) globalTimeWrapped++;
	DanssEvent.globalTime = globalTime();
	if (globalTimeWrapped) DanssEvent.globalTime += (1L<<45) * globalTimeWrapped;
	fileLastTime = globalTime();
	DanssEvent.number     = nevt();
	DanssEvent.unixTime   = absTime();
	DanssEvent.trigType = masterTriggerType();
	if (IsMc) {
		mcTruth(DanssMc.Energy, DanssMc.X[0], DanssMc.X[1], DanssMc.X[2], DanssMc.DriftTime);
		if (McEventTree) {
			gt = DanssEvent.globalTime + 10000000;
			McNum = gt / 125000000;
			if (McNum >= McEntries) {
				printf("Bad MC event number %d - more than in the file (%d).\n", McNum, McEntries);
				memset(&McEvent, 0, sizeof(McEvent));
			} else {
				McEventTree->GetEntry(McNum);
//				printf("Num=%d EventID=%f ParticleEnergy=%f EnergyLoss=%f", 
//					McNum, McEvent.EventID, McEvent.ParticleEnergy, McEvent.EnergyLoss);
			}
		}
	}
//	printf("MCTruth: %f %f %f %f %f\n", DanssMc.Energy, DanssMc.X[0], DanssMc.X[1], DanssMc.X[2], DanssMc.DriftTime);

	if (RawHitsArray) FindRawHits();
	CleanZeroes();
	SumEverything();
	FindFineTime();
	if (!(iFlags & FLG_NOTIMECUT)) CleanByTime();
//	Clean1Pixel();
	CleanByConfirmation();
//
	SumClean();
	CalculateNeutron();
	CalculatePositron();
	StoreHits();
//	CorrectEnergy((IsMc) ? MCEnergyCorrection : EnergyCorrection);
	if (IsMc && (iFlags & FLG_MCENERGYSMEAR)) MCSmear();
	if (iFlags & FLG_PRINTALL) DebugFullPrint();
	
//	if (iNevtTotal < 2) {
//		printf("\n\t==================\ncell t t_raw t0 t0raw t0Digi t0rawDigi t0rawRawDigi td\n");
//		for (i=0; i<nhits(); i++) printf("%d.%d %f %f %f %f %f %f %f %f\n", adc(i), adcChan(i), t(i), t_raw(i), t0(i), t0raw(i), 
//			t0Digi(i), t0rawDigi(i), t0rawRawDigi(i), td(100*adc(i) + adcChan(i)));
//	}
	
	if (DanssEvent.globalTime == dumpgTime) {
		DumpEvent();
		return -1;
	}

	if ((iFlags & FLG_DTHIST) && !IsPickUp() &&
		(DanssEvent.PmtCleanEnergy + DanssEvent.SiPmCleanEnergy + DanssEvent.VetoCleanEnergy > TIMEHISTMINENERGY) &&
		(DanssEvent.PmtCleanHits + DanssEvent.SiPmCleanHits + DanssEvent.VetoCleanHits > TIMEHISTMINHITS))
		FillTimeHists();

	if ((DanssEvent.SiPmCleanHits > 0 && DanssEvent.PmtCleanHits > 0) || DanssEvent.VetoCleanHits > 0
		|| DanssEvent.trigType == masterTrgRandom) {	// remove pure pickup noise
		iNevtTotal++;
		DanssInfo.events++;
		if (OutputTree) OutputTree->Fill();
		if (RawHitsTree) RawHitsTree->Fill();
	}

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

	if (fileFirstTime > 0) {
		DanssInfo.upTime = fileLastTime - fileFirstTime;
		if (globalTimeWrapped) DanssInfo.upTime += (1L<<45);
		if (InfoTree) InfoTree->Fill();
	}


	if (OutputFile) OutputFile->cd();
	if (OutputTree) OutputTree->Write();
	if (InfoTree) InfoTree->Write();
	if ((iFlags & FLG_DTHIST) && OutputFile) {
		for (i=0; i<MAXADCBOARD; i++) for (j=0; j<iNChannels_AdcBoard; j++) {
			if (hTimeDelta[i][j]->GetEntries() > 0) hTimeDelta[i][j]->Write();
			if (hPMTTimeDelta[i][j]->GetEntries() > 0) hPMTTimeDelta[i][j]->Write();
		}
	}
	hCrossTalk->Write();
	for (j=0; j<iNChannels_AdcBoard; j++) if (hPMTAmpl[j]->GetEntries() > 0) hPMTAmpl[j]->Write();
	for (i=0; i<6; i++) hSiPMtime[i]->Write();
	for (i=0; i<6; i++) hSiPMtimeClean[i]->Write();
	if (RawHitsTree) RawHitsTree->Write();
	if (RawHitsArray) free(RawHitsArray);
	if (IsMc) {
		hEtoEMC->Write();
		hNPEtoEMC->Write();
	}
	if (OutputFile) OutputFile->Close();
	if (McFile) McFile->Close();
  
	printf("Total up time        %Ld seconds\n", upTime / (long long) GLOBALFREQ);
	printf("Total physics events %Ld\n", iNevtTotal);
	printf("Run completed in     %d seconds\n", time(NULL) - progStartTime);
	return;
}

//--------------------->


int ReadDigiDataUser::userActionAtFileChange()
{
	DanssInfo.upTime = fileLastTime - fileFirstTime;
	if (globalTimeWrapped) DanssInfo.upTime += (1L<<45);
	DanssInfo.runNumber = runnumber();
	DanssInfo.position = danssPosition();
	DanssInfo.height = averagePosition();
	if (InfoTree) InfoTree->Fill();
	upTime += DanssInfo.upTime;
	fileFirstTime = -1;
	return 0;
}
