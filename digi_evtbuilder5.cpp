/***
 *
 * Version:       4.0
 *
 * Package:       DANSS SiPm Signal Processing and Calibration
 *
 * Description:   Calculate different event parameters and put to root file
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
#include "TRandom2.h"
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
#include "evtbuilder.h"

/***********************	Definitions	****************************/
#define MYVERSION	"4.01"
//	Initial clean parameters
#define MINSIPMPIXELS	3			// Minimum number of pixels to consider SiPM hit
#define MINSIPMPIXELS2	2			// Minimum number of pixels to consider SiPM hit without confirmation (method 2)
#define MINPMTENERGY	0.1			// Minimum PMT energy for a hit
#define MINVETOENERGY	0.1			// Minimum VETO energy for a hit
#define SIPMEARLYTIME	45			// ns - shift from fine time
#define SOMEEARLYTIME	130			// ns - absolute if fineTime is not defined
#define MAXPOSITRONENERGY	20		// Maximum Total clean energy to calculate positron parameters
#define MAXCLUSTITER		10		// Maximum number of iterations in cluster search
#define EDGEPMARK		1.0		// Minimum energy to flag event with edge hits
#define MCNEUTRONSIGGMA		20.0		// Sigma for neutron based longitudinal correction for MC
//	fine time
#define MINENERGY4TIME	0.25			// Minimum energy to use for fine time averaging
#define TCUT		15			// fine time cut, ns
#define NOFINETIME	10000			// something out of range
//	Flags
#define FLG_PRINTALL		       1	// do large debuggging printout
#define FLG_DTHIST		       2	// create time delta histogramms
#define FLG_EAMPLITUDE		       4	// put amplitude instead of energy to XXCleanEnergy cells
#define FLG_POSECORRECTIONA	   0x100	// do positron energy correction based on MC and NHITS
#define FLG_POSECORRECTIONB	   0x200	// do positron energy correction based on average MC
#define FLG_SIMLONGCORR		  0x1000	// simulate "neutron" correction for MC events
#define FLG_NOCLEANNOISE 	 0x10000	// do not clean low energy signals
#define FLG_NOTIMECUT		 0x20000	// do not clean signals by time
#define FLG_NOCONFIRM		 0x40000	// do not search PMT confirmation for SiPM and vice versa
#define FLG_NOCONFIRM2		 0x80000	// do not search PMT confirmation for 1 pixel SiPM signals
#define FLG_NOPMTCORR		0x100000	// do not correct PMT energy of cluster for out of cluster SiPM hits
#define FLG_PMTTIMECUT		0x200000	// Cut PMT and Veto by time

using namespace std;

// Globals:

long long                               iNevtTotal;
long long				upTime;
long long				fileFirstTime;
long long				fileLastTime;
long long				dumpgTime;
int                                     progStartTime;
char *					chTimeCalibration;
char *					chOutputFile;
int					iFlags;
int					MaxEvents;
int					IsMc;				// MC run flag
double					AttenuationLength;

TRandom2 *				Random;
TFile *					OutputFile;
TTree *					OutputTree;
TTree *					InfoTree;
struct DanssEventStruct5		DanssEvent;
struct DanssInfoStruct4			DanssInfo;
struct DanssMcStruct			DanssMc;
int 					HitFlag[iMaxDataElements];	// array to flag out SiPM hits
TH1D *					hTimeDelta[iMaxAddress_AdcBoard][iNChannels_AdcBoard];
int					DeadList[iMaxAddress_AdcBoard][iNChannels_AdcBoard];
struct HitStruct {
	float			E[iMaxDataElements];
	float			T[iMaxDataElements];
	struct HitTypeStruct 	type[iMaxDataElements];
}					HitArray;

/********************************************************************************************************************/
/************************	Analysis functions					*****************************/
/********************************************************************************************************************/

//	int SiPm - hit number in SiPM
//	int Pmt  - hit number in PMT
//	ReadDigiDataUser *user - event reader
//	return true if SiPM is read by this PMT
int IsInModule(int SiPm, int Pmt, ReadDigiDataUser *user)
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
//	ReadDigiDataUser *user - event reader
//	return true if the SiPMs are neighbors or coinside
int IsNeighbor(int hitA, int hitB, ReadDigiDataUser *user)
{
	if (user->zCoord(hitA) == user->zCoord(hitB) && abs(user->firstCoord(hitA) - user->firstCoord(hitB)) <= 1) return 1;
	if (abs(user->zCoord(hitA) - user->zCoord(hitB)) == 1) return 1;
	return 0;
}

//	float energy - measured energy
//	float dist - distance from zero coordinate
//	return corrected energy
//	simulate "neutron" correction for MC events
float acorr(float energy, float dist, char side = 'Y')
{
	float C, XY;

	if (dist >= 0) {
		C = exp((dist - 48.0) / AttenuationLength);	// 48 cm is the effective middle
	} else if (IsMc && (iFlags & FLG_SIMLONGCORR)) {
		XY = (side == 'X') ? DanssMc.X[1] : DanssMc.X[0];
		dist = Random->Gaus(XY, MCNEUTRONSIGGMA);
		if (dist < 0) dist = 0;
		if (dist > 100) dist = 100;
		C = exp((dist - 50.0) / AttenuationLength);	// 50 cm is the middle for McTruth
	} else {
		C = 1;
	}
	return C * energy;
}

//	Calculate corrected energy from MC taking number of cluster hits into account
double HitNumberCorrection(double E, int N)
{
	const double coef[6][2] = {{0.0197, -0.0444}, {0.1976, -0.1081}, {0.3504, -0.1272}, {0.4837, -0.1372}, {0.5737, -0.1388}, {0.6682, -0.1346}};
	int i;
	double EC;
	
	if (N < 1) return -1;	// internal error.
	i = (N < 6) ? N : 6;
	EC = (E - coef[i-1][0])/ (1 + coef[i-1][1]);
	return EC;
}

//	Calculate corrected energy from MC not taking number of cluster hits into account
double MCAverageCorrection(double E)
{
	const double coef[2] = {0.1702, -0.0868};
	double EC;
	
	EC = (E - coef[0])/ (1 + coef[1]);
	return EC;
}

//	Calculate corrected positron energy from total energy
double MCTotalCorrection(double E)
{
	const double coef[2] = {0.6812, -0.0872};
	double EC;
	
	EC = (E - coef[0])/ (1 + coef[1]);
	return EC;
}

int GetEdgeFlag(int hit, ReadDigiDataUser *user)
{
	int z;
	int xy;
	char side;
	int flag;

	flag = 0;
	z = user->zCoord(hit);
	xy = user->firstCoord(hit);
	side = user->side(hit);

	switch(z) {
		case 0  : flag |= PFLAG_HIT_D1; break;
		case 1  : flag |= PFLAG_HIT_D2; break;
		case 2  :
		case 3  : flag |= PFLAG_HIT_D34; break;
		case 99 : flag |= PFLAG_HIT_U1; break;
		case 98 : flag |= PFLAG_HIT_U2; break;
		case 97 :
		case 96 : flag |= PFLAG_HIT_U34; break;
	}

	switch(xy) {
		case 0  : flag |= (side == 'X') ? PFLAG_HIT_E : PFLAG_HIT_N;
		case 24 : flag |= (side == 'X') ? PFLAG_HIT_W : PFLAG_HIT_S;
	}

	return flag;
}

/********************************************************************************************************************/
/************************		Main analysis					*****************************/
/********************************************************************************************************************/

// Calculate parameters assuming positron-like event
void CalculatePositron(ReadDigiDataUser *user)
{
#include "clust_table.h"
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

	if (DanssEvent.SiPmCleanEnergy + DanssEvent.PmtCleanEnergy > 2 * MAXPOSITRONENERGY) {
		DanssEvent.PositronFlags |= PFLAG_MAXENERGY;
		return;
	}

	N = user->nhits();
//		Find the maximum hit
	A = 0;
	maxHit = -1;
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bSiPm && user->e(i) > A) {
		A = user->e(i);
		maxHit = i;
	}
	if (maxHit < 0) {	// nothing to do - no usable SiPM hits
		DanssEvent.PositronFlags |= PFLAG_NOCLUSTER;
		return;
	}
	HitFlag[maxHit] = 10;
//		Find cluster
	for (k=0; k<MAXCLUSTITER; k++) {
		repeat = 0;
		for (i=0; i<N; i++) if (HitFlag[i] >= 10) for (j=0; j<N; j++) if (HitFlag[j] >= 0 && HitFlag[j] < 10 && user->type(j) == bSiPm && IsNeighbor(i, j, user)) {
			HitFlag[j] = 20;
			repeat = 1;
		}
		if (!repeat) break;
	}
//		Check cluster
//	Step 1: find cluster range
	xmin = ymin = zmin = 200;
	xmax = ymax = zmax = -1;
	for (i=0; i<N; i++) if (HitFlag[i] >= 10 && user->type(i) == bSiPm) {
		if (user->zCoord(i) > zmax) zmax = user->zCoord(i);
		if (user->zCoord(i) < zmin) zmin = user->zCoord(i);
		if (user->side(i) == 'X') {
			if (user->firstCoord(i) > xmax) xmax = user->firstCoord(i);
			if (user->firstCoord(i) < xmin) xmin = user->firstCoord(i);
		} else {
			if (user->firstCoord(i) > ymax) ymax = user->firstCoord(i);
			if (user->firstCoord(i) < ymin) ymin = user->firstCoord(i);
		}
	}
	A = 0;
	if (xmax - xmin > 1) A += fStripWidth  * fStripWidth  * (xmax - xmin - 1) * (xmax - xmin - 1);
	if (ymax - ymin > 1) A += fStripWidth  * fStripWidth  * (ymax - ymin - 1) * (ymax - ymin - 1);
	if (zmax - zmin > 1) A += fStripHeight * fStripHeight * (zmax - zmin - 1) * (zmax - zmin - 1);
	DanssEvent.PositronMinLen = sqrt(A);
	invalid = 0;
	if (xmax - xmin > 1 || ymax - ymin > 1 || zmax - zmin > 4) {	// Maximum cluster is 5x2 
		invalid = -10000;	// too large
	} else {
//	Step 2: fill clust array
		memset(clusterHits, 0, sizeof(clusterHits));
		for (i=0; i<N; i++) if (HitFlag[i] >= 10 && user->type(i) == bSiPm) {
			xy = user->firstCoord(i) - ((user->side(i) == 'X') ? xmin : ymin);
			clusterHits[2*(user->zCoord(i)-zmin) + xy]++;
		}
//	Step 3: look for forbidden combinations
		j = 0;
		for (i=0; i<10; i++) if (clusterHits[i]) j |= 1 << i;
		if (!cTable[j]) j = -j;		// zero is also bad value
		invalid = j;
	}
	if (invalid <= 0) DanssEvent.PositronFlags |= PFLAG_INVCLUSTER;
//		Find cluster position
	x = y = z = 0;
	nx = ny = 0;
	for (i=0; i<N; i++) if (HitFlag[i] >= 10) {
		DanssEvent.PositronHits++;
		if (user->side(i) == 'X') {
			x += user->firstCoord(i) * fStripWidth * user->e(i);
			z += user->zCoord(i) * fStripHeight * user->e(i);
			nx += user->e(i);
		} else {
			y += user->firstCoord(i) * fStripWidth * user->e(i);
			z += user->zCoord(i) * fStripHeight * user->e(i);
			ny += user->e(i);
		}
	}
	DanssEvent.PositronX[0] = (nx > 0) ? x / nx : -1;		// 50 cm is DANSS center
	DanssEvent.PositronX[1] = (ny > 0) ? y / ny : -1;		// 50 cm is DANSS center
	DanssEvent.PositronX[2] = (nx + ny > 0) ? z / (nx + ny) : -1;	// 50 cm is DANSS center
//		Find corrected energy
//	Step 1: Count SiPM
	for (i=0; i<N; i++) if (HitFlag[i] >= 10) {
		if (user->side(i) == 'X') {
			DanssEvent.PositronSiPmEnergy += acorr(user->e(i), DanssEvent.PositronX[1], 'X');
		} else {
			DanssEvent.PositronSiPmEnergy += acorr(user->e(i), DanssEvent.PositronX[0], 'Y');
		}
	}
//	Step 2: Count PMT
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bPmt) {
		for (j=0; j<N; j++) if (IsInModule(j, i, user) && HitFlag[j] >= 10) break;
		if (j >= N) continue;
		HitFlag[i] = 5;
		if (user->side(i) == 'X') {
			DanssEvent.PositronPmtEnergy += acorr(user->e(i), DanssEvent.PositronX[1], 'X');
		} else {
			DanssEvent.PositronPmtEnergy += acorr(user->e(i), DanssEvent.PositronX[0], 'Y');
		}
	}
//	Step 3: Subtract gammas in PMT
	if (!(iFlags & FLG_NOPMTCORR)) for (i=0; i<N; i++) if (HitFlag[i] >= 0 && HitFlag[i] < 10 && user->type(i) == bSiPm) {
		for (j=0; j<N; j++) if (IsInModule(i, j, user) && HitFlag[j] == 5) break;
		if (j >= N) continue;
		if (user->side(i) == 'X') {
			DanssEvent.PositronPmtEnergy -= acorr(user->e(i), DanssEvent.PositronX[1], 'X');
		} else {
			DanssEvent.PositronPmtEnergy -= acorr(user->e(i), DanssEvent.PositronX[0], 'Y');
		}
	}
	DanssEvent.PositronEnergy = DanssEvent.PositronSiPmEnergy + DanssEvent.PositronPmtEnergy;
//	Step 4: Divide by 2, because we count SiPM + PMT
	DanssEvent.PositronEnergy /= 2;
//	Calculate Total energy with longitudinal correction
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && (user->type(i) == bPmt || user->type(i) == bSiPm)) {
		if (user->side(i) == 'X') {
			DanssEvent.TotalEnergy += acorr(user->e(i), DanssEvent.PositronX[1], 'X');
		} else {
			DanssEvent.TotalEnergy += acorr(user->e(i), DanssEvent.PositronX[0], 'Y');
		}
	}
	DanssEvent.TotalEnergy /= 2;	// PMT + SiPM
	if (FLG_POSECORRECTIONB & iFlags) DanssEvent.TotalEnergy = MCTotalCorrection(DanssEvent.TotalEnergy);
//
//		Count possible gammas
	A = 0;
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && HitFlag[i] < 10 && user->type(i) == bSiPm) {
		DanssEvent.AnnihilationGammas++;
		DanssEvent.AnnihilationEnergy += user->e(i);
		if (A < user->e(i)) A = user->e(i);
	}
	DanssEvent.AnnihilationMax = A;
//		Count hits on the edge with E > 1 MeV
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bSiPm && user->e(i) > EDGEPMARK) {
		j = GetEdgeFlag(i, user);
		if (!j) continue;
		DanssEvent.PositronFlags |= j;
		if (HitFlag[i] >= 10) 	DanssEvent.PositronFlags |= j >> 12;
	}
//		Do energy correction based on MC taking into account number of hits in the cluster
	if (FLG_POSECORRECTIONA & iFlags) DanssEvent.PositronEnergy = HitNumberCorrection(DanssEvent.PositronEnergy, DanssEvent.PositronHits);
	if (FLG_POSECORRECTIONB & iFlags) DanssEvent.PositronEnergy = MCAverageCorrection(DanssEvent.PositronEnergy);
}

void CalculateNeutron(ReadDigiDataUser *user)
{
	float x, y, z, r;
	int nx, ny;
	int i, N;

	N = user->nhits();
//	Find the center (1st approximation)
	x = y = z = 0;
	nx = ny = 0;
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bSiPm) {
		if (user->side(i) == 'X') {
			x += user->firstCoord(i) * fStripWidth;
			z += user->zCoord(i) * fStripHeight;
			nx++;
		} else {
			y += user->firstCoord(i) * fStripWidth;
			z += user->zCoord(i) * fStripHeight;
			ny++;
		}
	}
	DanssEvent.NeutronX[0] = (nx) ? x / nx : -1;			// 50 cm is DANSS center
	DanssEvent.NeutronX[1] = (ny) ? y / ny : -1;			// 50 cm is DANSS center
	DanssEvent.NeutronX[2] = (nx + ny) ? z / (nx + ny) : -1;	// 50 cm is DANSS center
}

// Clean hits:
// - SiPM with zero or less number of pixels
// - bad (not a number) or not positive energy
// - bad time
// - from marked bad channels
// - from dead channel list
void CleanZeroes(ReadDigiDataUser *user)
{
	int i, N;

	N = user->nhits();
	for (i=0; i<N; i++) if ((user->type(i) == bSiPm && user->npix(i) <= 0) || (!isfinite(user->e(i))) ||
		user->e(i) <= 0 || user->t_raw(i) < -1000 || user->isBadChannel(user->chanIndex(i)) || DeadList[user->adc(i)][user->adcChan(i)]) HitFlag[i] = -1;
}

void CleanNoise(ReadDigiDataUser *user)
{
	int i, N;
	
	N = user->nhits();
	for (i=0; i<N; i++) switch (user->type(i)) {
	case bSiPm:
		if (user->npix(i) < MINSIPMPIXELS) HitFlag[i] = -1;
		break;
	case bPmt:
		if (user->e(i) < MINPMTENERGY) HitFlag[i] = -1;
		break;
	case bVeto:
		if (user->e(i) < MINVETOENERGY) HitFlag[i] = -1;
		break;
	}
}

void CleanByConfirmation(ReadDigiDataUser *user)
{
	int i, j, N;
	
	N = user->nhits();
	for (i=0; i<N; i++) if (HitFlag[i] >= 0) switch (user->type(i)) {
	case bSiPm:
		for (j=0; j<N; j++) if (HitFlag[j] >= 0 && user->type(j) == bPmt && IsInModule(i, j, user)) break;
		if (j == N) HitFlag[i] = -1;
		break;
	case bPmt:
		for (j=0; j<N; j++) if (HitFlag[j] >= 0 && user->type(j) == bSiPm && IsInModule(j, i, user)) break;
		if (j == N) HitFlag[i] = -1;
		break;
	}
}

/*	Clean SiPM only if npix == 1 and no PMT confirmation	*/
void CleanByConfirmation2(ReadDigiDataUser *user)
{
	int i, j, N;
	
	N = user->nhits();
	for (i=0; i<N; i++) if (HitFlag[i] >= 0 && user->type(i) == bSiPm) {
		if (user->npix(i) >= MINSIPMPIXELS2) continue;		// that's enough
		for (j=0; j<N; j++) if (HitFlag[j] >= 0 && user->type(j) == bPmt && IsInModule(i, j, user)) break;
		if (j < N) continue;
		HitFlag[i] = -1;
	}
//		"early" hits
	for (i=0; i<N; i++) if (HitFlag[i] == -100)
	{
		if (user->npix(i) >= MINSIPMPIXELS2) continue;		// that's enough
		for (j=0; j<N; j++) if (HitFlag[j] >= 0 && user->type(j) == bPmt && IsInModule(i, j, user)) break;
		if (j < N) continue;
		HitFlag[i] = -1;
	}
}

void CleanByTime(ReadDigiDataUser *user)
{
	int i, N;
	float tearly;
	
	N = user->nhits();
	if (DanssEvent.fineTime != NOFINETIME) {
		for (i=0; i<N; i++) 
			if (fabs(user->t_raw(i) - DanssEvent.fineTime) > TCUT && ((iFlags & FLG_PMTTIMECUT) || user->type(i) == bSiPm)) HitFlag[i] = -1;
		tearly = DanssEvent.fineTime - SIPMEARLYTIME;
	} else {
		tearly = SOMEEARLYTIME;
	}
	for (i=0; i<N; i++) if (user->type(i) == bSiPm && fabs(user->t_raw(i) - tearly) <= TCUT) HitFlag[i] = -100;	// mark early hit candidates
}

void CreateDeadList(char *fname)
{
	int i, j;
	char str[1024];
	char *ptr;
	FILE *f;

	memset(DeadList, 0, sizeof(DeadList));
	if (!fname) return;
	f = fopen(fname, "rt");
	if (!f) {
		printf("Dead list file %s not found.\n", fname);
		return;
	}
	for (;;) {
		if (!fgets(str, sizeof(str), f)) break;
		i = strtol(str, &ptr, 10) - 1;
		ptr++;
		j = strtol(ptr, NULL, 10);
		if (i >= 0 && i < iMaxAddress_AdcBoard && j >= 0 && j < iNChannels_AdcBoard) DeadList[i][j] = 1;
	}
	fclose(f);
}


void DebugFullPrint(ReadDigiDataUser *user)
{
	int i, N;
	time_t tm;

	N = user->nhits();
	tm = DanssEvent.unixTime;
	printf("******************************************************************************************************************\n");
	printf("Event: %Ld globalTime: %Ld fineTime: %6.1f ns   linux time: %s", 
		DanssEvent.number, DanssEvent.globalTime, DanssEvent.fineTime, ctime(&tm));
	printf("Total %d hits: %d SiPM %d PMT %d Veto; Clean: %d SiPM %d PMT %d Veto\n", 
		N, DanssEvent.SiPmHits, DanssEvent.PmtHits, DanssEvent.VetoHits, 
		DanssEvent.SiPmCleanHits, DanssEvent.PmtCleanHits, DanssEvent.VetoCleanHits);
	printf("Energy: %6.1f SiPM %6.1f PMT %6.1f Veto; Clean: %6.1f SiPM %6.1f PMT %6.1f Veto\n", 
		DanssEvent.SiPmEnergy, DanssEvent.PmtEnergy, DanssEvent.VetoEnergy, 
		DanssEvent.SiPmCleanEnergy, DanssEvent.PmtCleanEnergy, DanssEvent.VetoCleanEnergy);
	if (N) {
		printf("N    Type  N  S       E    time  ADC.Ch side XY Z  Flag\n");
//			1234512345123412345678123451234561231123123451231231234
		for(i=0; i<N; i++) switch(user->type(i)) {
		case bSiPm:
			printf("%4d SiPM %3.0f %7.1f %4.1f %5.1f %2d.%2.2d    %c  %2d %2d  %c\n", i+1, user->npix(i), user->signal(i),
				user->e(i), user->adc(i), user->t_raw(i), user->adcChan(i), user->side(i), user->firstCoord(i), user->zCoord(i),
				(HitFlag[i]<0) ? 'X' : ' ');
			break;
		case bPmt:
			printf("%4d PMT      %7.1f %4.1f %5.1f %2d.%2.2d    %c  %2d %2d  %c\n", i+1, user->signal(i),
				user->e(i), user->t_raw(i), user->adc(i), user->adcChan(i), user->side(i), user->firstCoord(i), user->zCoord(i),
				(HitFlag[i]<0) ? 'X' : ' ');
			break;
		case bVeto:
			printf("%4d VETO     %7.1f %4.1f %5.1f %2d.%2.2d    -  xx xx  %c\n", i+1, user->signal(i),
				user->e(i), user->t_raw(i), user->adc(i), user->adcChan(i),
				(HitFlag[i]<0) ? 'X' : ' ');
			break;
		}
	}
}

void DumpEvent(ReadDigiDataUser *user)
{
	int i, N;
	char str[1024];
	
	DebugFullPrint(user);

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
	TH1D *Par   = new TH1D("hPar", "Parameters", 10, 0, 10);
	
	Par->GetXaxis()->SetBinLabel(1, "fineTime");
	Par->GetXaxis()->SetBinLabel(2, "SiPmEnergy");
	Par->GetXaxis()->SetBinLabel(3, "PmtEnergy");
	Par->GetXaxis()->SetBinLabel(4, "VetoEnergy");
	Par->GetXaxis()->SetBinLabel(5, "PositronEnergy");
	Par->GetXaxis()->SetBinLabel(6, "SiPmHits");
	Par->GetXaxis()->SetBinLabel(7, "PmtHits");
	Par->GetXaxis()->SetBinLabel(8, "VetoHits");
	Par->GetXaxis()->SetBinLabel(9, "PositronHits");
	
	Par->Fill("fineTime", DanssEvent.fineTime);
	Par->Fill("SiPmEnergy", DanssEvent.SiPmCleanEnergy);
	Par->Fill("PmtEnergy", DanssEvent.PmtEnergy);
	Par->Fill("VetoEnergy", DanssEvent.VetoEnergy);
	Par->Fill("PositronEnergy", DanssEvent.PositronEnergy);
	Par->Fill("SiPmHits", DanssEvent.SiPmCleanHits);
	Par->Fill("PmtHits", DanssEvent.PmtHits);
	Par->Fill("VetoHits", DanssEvent.VetoHits);
	Par->Fill("PositronHits", DanssEvent.PositronHits);

	N = user->nhits();
	for(i=0; i<N; i++) {
		switch(user->type(i)) {
		case bSiPm:
			if (user->side(i) == 'X') {
				SiPmX->Fill(user->firstCoord(i)*4.0 + 2.0, user->zCoord(i)+0.5, user->e(i));
				if (HitFlag[i] >= 0) SiPmCleanX->Fill(user->firstCoord(i)*4.0 + 2.0, user->zCoord(i)+0.5, user->e(i));
			} else {
				SiPmY->Fill(user->firstCoord(i)*4.0 + 2.0, user->zCoord(i)+0.5, user->e(i));
				if (HitFlag[i] >= 0) SiPmCleanY->Fill(user->firstCoord(i)*4.0 + 2.0, user->zCoord(i)+0.5, user->e(i));
			}
			break;
		case bPmt:
			if (user->side(i) == 'X') {
				PmtX->Fill(user->firstCoord(i)*20.0 + 10.0, user->zCoord(i)*20.0+10.0, user->e(i));
			} else {
				PmtY->Fill(user->firstCoord(i)*20.0 + 10.0, user->zCoord(i)*20.0+10.0, user->e(i));
			}
			break;
		case bVeto:
			Veto->Fill(user->adcChan(i), user->e(i));
			break;
		}
		Time->Fill(user->t_raw(i), user->e(i));
		if (HitFlag[i] >= 0) TimeClean->Fill(user->t_raw(i), user->e(i));
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


void FindFineTime(ReadDigiDataUser *user)
{
	float tsum;
	float asum;
	float e;
	int i, k, N;
	
	tsum = asum = 0;
	k = 0;
	N = user->nhits();
	for (i=0; i<N; i++) if (HitFlag[i] >= 0) {
		switch(user->type(i)) {
		case bSiPm:
			e = user->e(i);
			if (user->npix(i) < MINSIPMPIXELS) e = 0;
			break;
		case bPmt:
		case bVeto:
			e = user->e(i);
			break;
		}
		if (e > MINENERGY4TIME && user->t_raw(i) > 0) {
			tsum += user->t_raw(i) * e;
			asum += e;
			k++;
		}
	}
	DanssEvent.fineTime = (asum > 0) ? tsum / asum : NOFINETIME;	// some large number if not usable hits found
	if (k > 1 && (iFlags & FLG_DTHIST)) for (i=0; i<N; i++) 
		if (HitFlag[i] >= 0 && !(user->type(i) == bSiPm && user->npix(i) < MINSIPMPIXELS2) && user->e(i) > MINENERGY4TIME && user->t_raw(i) > 0) 
		hTimeDelta[user->adc(i)-1][user->adcChan(i)]->Fill(user->t_raw(i) - DanssEvent.fineTime);
}

void StoreHits(ReadDigiDataUser *user)
{
	int i, j, N;

	j = 0;
	N = user->nhits();
	for (i=0; i<N; i++) if (HitFlag[i] >= 0) {
		HitArray.E[j] = user->e(i);
		HitArray.T[j] = user->t_raw(i);
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
}

void SumClean(ReadDigiDataUser *user)
{
	int i, N;
	
	N = user->nhits();
	for (i=0; i<N; i++) if (HitFlag[i] >= 0) switch (user->type(i)) {
	case bSiPm:
		DanssEvent.SiPmCleanHits++;
		DanssEvent.SiPmCleanEnergy += (iFlags & FLG_EAMPLITUDE) ? user->siPmAmp(user->side(i), user->firstCoord(i), user->zCoord(i)) : user->e(i);
		break;
	case bPmt:
		DanssEvent.PmtCleanHits++;
		DanssEvent.PmtCleanEnergy += (iFlags & FLG_EAMPLITUDE) ? user->pmtAmp(user->side(i), user->firstCoord(i), user->zCoord(i)) : user->e(i);
		break;
	case bVeto:
		DanssEvent.VetoCleanHits++;
		DanssEvent.VetoCleanEnergy += (iFlags & FLG_EAMPLITUDE) ? user->vetoAmp(user->indexByHit(i)) : user->e(i);
		break;
	}

	for (i=0; i<N; i++) if (HitFlag[i] == -100) {
		DanssEvent.SiPmEarlyHits++;
		DanssEvent.SiPmEarlyEnergy += user->e(i);		
	}
}

void SumEverything(ReadDigiDataUser *user)
{
	int i, N;
	
	N = user->nhits();
	for (i=0; i<N; i++) switch (user->type(i)) {
	case bSiPm:
		DanssEvent.SiPmHits++;
		DanssEvent.SiPmEnergy += user->e(i);
		break;
	case bPmt:
		DanssEvent.PmtHits++;
		DanssEvent.PmtEnergy += user->e(i);
		break;
	case bVeto:
		DanssEvent.VetoHits++;
		DanssEvent.VetoEnergy += user->e(i);
		break;
	}
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
	FILE *f;
	char str[1024];
	char *ptr;
	int i, k;
	int iAdcNum, iAdcChan;
	

//	Set all zeroes
	for(i = 0; i < iNElements; i++) {
    		iAdcNum = i / 100;
    		iAdcChan = i % 100;
    		if(!isAdcChannelExist(iAdcNum, iAdcChan)) continue;
    		setTd(i, 0); // set all td = 0
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
		if (str[0] != 'C') {
			printf("Time calibration: %s", str);
			continue;	// Comment ?
		}
		ptr = strstr(str, "Channel=");
		if (!ptr) {
			printf("Time calibration no Channel=: %s", str);
			continue;		// strange string
		}
		ptr += strlen("Channel=");
		i = 100 * (strtod(ptr, NULL) + 0.002);
    		iAdcNum = i / 100;
    		iAdcChan = i % 100;
    		if(!isAdcChannelExist(iAdcNum, iAdcChan)) {	// non-existing channel - strange
			printf("Time calibration wrong channel i=%d (%d.%d): %s", i, iAdcNum, iAdcChan, str);
			continue;
		}
		ptr = strstr(str, "DT=");
		if (!ptr) {		// strange string	
			printf("Time calibration no value DT=: %s", str);
			continue;
		}
		ptr += strlen("DT=");
		setTd(i, strtod(ptr, NULL));
		k++;
	}

	printf("Time calibration used: %s. %d channels found.\n", chTimeCalibration, k);

	fclose(f);
}

//------------------------------->

void Help(void)
{
	printf("\tDANSS offline: digi event builder. Version %s\n", MYVERSION);
	printf("Process events and create root-tree with event parameters.\n");
	printf("\tOptions:\n");
	printf("-alen AttenuationLength --- signal attenuation length in cm. Default - 300 cm.\n");
	printf("-calib filename.txt --- file with energy calibration. No default.\n");
	printf("-deadlist filename.txt --- file with explicit list of dead channels.\n");
	printf("-dump gTime --- dump an event with this gTime.\n");
	printf("-events number --- stop after processing this number of events. Default - do not stop.\n");
	printf("-file filename.txt --- file with a list of files for processing. No default.\n");
	printf("-flag FLAGS --- analysis flag mask. Default - 0. Recognized flags:\n");
	printf("\t       1 --- do debugging printout of events;\n");
	printf("\t       2 --- create delta time histograms;\n");
	printf("\t   0x100 --- do energy correction based on MC taking into account number of hits in the cluster;\n");
	printf("\t   0x200 --- do energy correction based on MC not taking into account number of hits in the cluster;\n");
	printf("\t  0x1000 --- simulate \"neutron\" correction for MC events;\n");
	printf("\t 0x10000 --- do not clean small energies;\n");
	printf("\t 0x20000 --- do not do time cut;\n");
	printf("\t 0x40000 --- do not require confirmation for all hits;\n");
	printf("\t 0x80000 --- do not require confirmation for SiPM single pixel hits;\n");
	printf("\t0x100000 --- do not correct PMT cluster energy for out of cluster SiPM hits.\n");
	printf("-help --- print this message and exit.\n");
	printf("-mcdata --- this is Monte Carlo data - create McTruth branch.\n");
	printf("-seed SEED --- seed for random number generator.\n");
	printf("-output filename.root --- output file name. Without this key output file is not written.\n");
	printf("-tcalib filename.txt --- file with the time calibration.\n");
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
	char strs[128];
	char strl[1024];
	char *DeadListName;
	int RandomSeed = 17321;

	AttenuationLength = 300;
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
		} else if (!strcmp(argv[i], "-flag")) {
			i++;
			iFlags = strtol(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-events")) {
			i++;
			MaxEvents = strtol(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-mcdata")) {
			IsMc = 1;
		} else if (!strcmp(argv[i], "-alen")) {
			i++;
			AttenuationLength = strtod(argv[i], NULL);
		} else if (!strcmp(argv[i], "-seed")) {
			i++;
			RandomSeed = strtol(argv[i], NULL, 0);
		} else if (!strcmp(argv[i], "-help")) {
			Help();
			exit(0);
		} else if (!strcmp(argv[i], "-dump")) {
			i++;
			dumpgTime = strtoll(argv[i], NULL, 0);
		}
	}

	Random = new TRandom2(RandomSeed);

	CreateDeadList(DeadListName);
	if (chTimeCalibration) init_Tds();

	if (chOutputFile) {
		OutputFile = new TFile(chOutputFile, "RECREATE");
		if (!OutputFile->IsOpen()) throw "Panic - can not open output file!";
		OutputTree = new TTree("DanssEvent", "Danss event tree");
		OutputTree->Branch("Data", &DanssEvent, 
//		Common parameters
			"globalTime/L:"		// time in terms of 125 MHz
			"number/L:"		// event number in the file
			"runNumber/I:"		// the run number
			"unixTime/I:"		// linux time, seconds
			"fineTime/F:"		// fine time of the event (for hit selection)
//		Veto parameters
			"VetoHits/I:"		// hits in the event record
			"VetoEnergy/F:"		// Energy Sum of all hits
			"VetoCleanHits/I:"	// hits above threshold and in time window
			"VetoCleanEnergy/F:"	// Energy Sum of clean hits
//		PMT parameters
			"PmtHits/I:"		// the same as above for PMT
			"PmtEnergy/F:"
			"PmtCleanHits/I:"
			"PmtCleanEnergy/F:"
//		SiPM parameters
			"SiPmHits/I:"		// the same as above for SiPM
			"SiPmEnergy/F:"
			"SiPmCleanHits/I:"
			"SiPmCleanEnergy/F:"
			"SiPmEarlyHits/I:"
			"SiPmEarlyEnergy/F:"
//		"positron cluster" parameters
			"PositronHits/I:"	// hits in the cluster
			"PositronFlags/I:"	// flags
			"PositronMinLen/F:"	// Minimum track length to create the cluster
			"PositronEnergy/F:"	// Energy sum of the cluster, corrected, (SiPM + PMT) / 2
			"TotalEnergy/F:"	// Total energy, longitudinally correctd (former Energy of the maximum hit)
			"PositronSiPmEnergy/F:"	// SiPM energy in the cluster, corrected
			"PositronPmtEnergy/F:"	// PMT energy in the cluster, corrected
			"PositronX[3]/F:"	// cluster position
			"AnnihilationGammas/I:"	// number of possible annihilation gammas
			"AnnihilationEnergy/F:"	// Energy in annihilation gammas
			"AnnihilationMax/F:"	// Energy in the maximum annihilation hit
//		"neutron" parameters
			"NeutronX[3]/F:"	// center of gammas position
//		"NeutronRadius/F"	// average distance between hits and the center
			"NHits/I"		// Number of hits
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
			"height/F"		// Danss average height
		);
	}
	iNevtTotal = 0;
	upTime = 0;
	fileFirstTime = -1;
	fileLastTime = -1;
	memset(&DanssInfo, 0, sizeof(struct DanssInfoStruct4));
	
	if (iFlags & FLG_DTHIST) for (i=0; i<iMaxAddress_AdcBoard; i++) for (j=0; j<iNChannels_AdcBoard; j++) {
		sprintf(strs, "hDT%2.2dc%2.2d", i+1, j);
		sprintf(strl, "Time delta distribution for channel %2.2d.%2.2d;ns", i+1, j);
		hTimeDelta[i][j] = new TH1D(strs, strl, 250, -25, 25);
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
	float fineTime;

  	if( ttype() != 1 ) return 0;
	
	memset(HitFlag, 0, nhits() * sizeof(int));
	memset(&DanssEvent, 0, sizeof(struct DanssEventStruct5));

	fileLastTime = globalTime();
	DanssInfo.stopTime = absTime();
	if (fileFirstTime < 0) {
		fileFirstTime = fileLastTime;
		DanssInfo.startTime = DanssInfo.stopTime;
		DanssInfo.events = 0;
	}

	DanssEvent.globalTime = globalTime();
	DanssEvent.number     = nevt();
	DanssEvent.runNumber  = runnumber();
	DanssEvent.unixTime   = absTime();

	if (IsMc) mcTruth(DanssMc.Energy, DanssMc.X[0], DanssMc.X[1], DanssMc.X[2], DanssMc.DriftTime);

	CleanZeroes(this);
	SumEverything(this);
	if (!(iFlags & FLG_NOCLEANNOISE)) CleanNoise(this);
	FindFineTime(this);
	if (!(iFlags & FLG_NOTIMECUT)) CleanByTime(this);
	if (!(iFlags & FLG_NOCONFIRM)) CleanByConfirmation(this);
	if (!(iFlags & FLG_NOCONFIRM2)) CleanByConfirmation2(this);
	SumClean(this);
	CalculateNeutron(this);
	CalculatePositron(this);
	StoreHits(this);
	if (iFlags & FLG_PRINTALL) DebugFullPrint(this);
	if (DanssEvent.globalTime == dumpgTime) {
		DumpEvent(this);
		return -1;
	}

  	if ((DanssEvent.SiPmCleanHits > 0 && DanssEvent.PmtCleanHits > 0) || DanssEvent.VetoCleanHits > 0) {	// remove pure pickup noise
		iNevtTotal++;
		DanssInfo.events++;
		if (OutputTree) OutputTree->Fill();
	}

	if (MaxEvents > 0 && iNevtTotal >= MaxEvents) return -1;
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

	if (fileFirstTime > 0) {
		DanssInfo.upTime = fileLastTime - fileFirstTime;
		if (InfoTree) InfoTree->Fill();
	}

	if (OutputFile) OutputFile->cd();

	if (OutputTree) OutputTree->Write();
	if (InfoTree) InfoTree->Write();
	if ((iFlags & FLG_DTHIST) && OutputFile) for (i=0; i<iMaxAddress_AdcBoard; i++) for (j=0; j<iNChannels_AdcBoard; j++) hTimeDelta[i][j]->Write();

	if (OutputFile) OutputFile->Close();
  
	printf("Total up time        %Ld seconds\n", upTime / (long long) GLOBALFREQ);
	printf("Total physics events %Ld\n", iNevtTotal);
	printf("Run completed in     %d seconds\n", time(NULL) - progStartTime);
  	return;
}

//--------------------->


int ReadDigiDataUser::userActionAtFileChange()
{
	DanssInfo.upTime = fileLastTime - fileFirstTime;
	DanssInfo.runNumber = runnumber();
	DanssInfo.position = danssPosition();
	DanssInfo.height = averagePosition();
	if (InfoTree) InfoTree->Fill();
	upTime += DanssInfo.upTime;
	fileFirstTime = -1;
	return 0;
}

