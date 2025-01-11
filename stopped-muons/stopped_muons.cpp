/****************************************************************************************
 *	DANSS data analysis - find stopped muons					*
 ****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
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
#include "TRandom2.h"
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

#include "../evtbuilder.h"

#define GFREQ2US	(GLOBALFREQ / 1000000.0)
#define MAXZ	100
#define MAXXY	25
#define iMaxDataElements 3000
#define masterTrgRandom 2

#pragma pack(push,1)
struct StoppedMuonStruct {
	int index;		// Index in the DanssEvent tree
	long long globalTime;	// global time 125 MHz ticks
	int Z;			// z of the stopping strip
	int XY;			// xy of the stopping strip
	float thetaX;		// angle of the track in ZX
	float thetaY;		// angle of the track in ZY
	int NHits;		// number of hits
	float Ehit[MAXZ];	// hit energy
};
#pragma pack(pop)

struct HitStruct {
	float			E[iMaxDataElements];
	float			T[iMaxDataElements];
	struct HitTypeStruct 	type[iMaxDataElements];
} HitData;

struct PlaneHitStruct {
	float xy;
	float E;
};

struct TrackProjStruct {
	int Zmin;	// the lowest plane of the track
	int Zmax;	// the highest plane of the track
	float A;	// XY = A*Z + B
	float B;
};

struct DanssEventStruct7 DanssEvent;

int DeadChannels[MAXZ][MAXXY];

double SiPMYAverageLightColl(double x)
{
    //<func(x)=1>
	const double FuncAverage = 1.02208;
	double rez;
	rez = (0.00577381*exp(-0.1823*(x-48)) + 0.999583*exp(-0.0024*(x-48)) - 8.095E-13*exp(0.5205*(x-48))
	        -0.00535714*exp(-0.1838*(x-48))) / FuncAverage;
	return rez;
}

/*	Scan events for all active channels		*/
/*	Channel is considered dead if its occurence is	*/
/*	Less than 1/3000 of the number of events	*/
void CreateDeadMap(TTree *evt)
{
	int i, N, j;
	memset(DeadChannels, 0, sizeof(DeadChannels));
	
	N = evt->GetEntries();
	for (i=0; i<N; i++) {
		evt->GetEntry(i);
		for (j=0; j<DanssEvent.NHits; j++) if (HitData.type[j].type == 0 && HitData.E[j] > 0.15) 
			DeadChannels[HitData.type[j].z][HitData.type[j].xy]++;
	}
	for (i=0; i<MAXZ; i++) for (j=0; j<MAXXY; j++) 
		DeadChannels[i][j] = (DeadChannels[i][j] > (N / 3000)) ? 0 : 1;
//	int cnt = 0;
//	for (i=0; i<MAXZ; i++) for (j=0; j<MAXXY; j++) {
//		printf("%d %d %d\n", i, j, DeadChannels[i][j]);
//		cnt += DeadChannels[i][j];
//	}
//	printf("%d dead channels found\n", cnt);
}

/*	Scan strip plane for adjasent hits and merge them	*/
/*	Return 0 if one or no merged hits, else 1		*/
/*	We select gold tracks only and ignore events with 	*/
/*	more than one hit in any plane. hit lengths 1 and 2	*/
/*	only accepted						*/
int MergeHits(double *pl, struct PlaneHitStruct *hit)
{
	int i, j, cnt;
	double x;
	double E;
	cnt = 0;
	x = 0;
	E = 0;
	for (i=0; i<MAXXY; i++) if (pl[i] > 0) {
		if (cnt > 0) return 1;
		x = i * pl[i];
		E = pl[i];
		if (i < MAXXY - 1) {
			x += (i+1) * pl[i+1];
			E += pl[i+1];
			i++;
		}
		x /= E;
		cnt++;
	}
	hit->xy = x;
	hit->E = E;
	return 0;
}

/*	Look for a track as a continuous set of hit planes	*/
/*	Require minimum of 5 planes and maximum total breaks	*/
/*	of 5 planes. Hits must be less than 1 strip aside	*/
/*	from the track. Return 0 on success.			*/

int FindTrack(struct PlaneHitStruct *pl, struct TrackProjStruct *tr)
{
	int i, cnt, N;
	double sZ, sX, sZ2, sZX;
	float x;
//		Check planes
	for (i=0; i<MAXZ/2; i++) if (pl[i].E > 0) break;
	tr->Zmin = i;
	for (i = MAXZ/2 - 1; i >= 0; i--) if (pl[i].E > 0) break;
	tr->Zmax = i;
	cnt = 0;
	for (i = tr->Zmin; i <= tr->Zmax; i++) if (pl[i].E <= 0) cnt++;
	if (cnt > 5) return 1;
	N = tr->Zmax - tr->Zmin - cnt + 1;
	if (N < 5) return 1;
//		Find track parameters
	sZ = sX = sZ2 = sZX = 0;
	for (i = tr->Zmin; i <= tr->Zmax; i++) {
		sZ += i;
		sX += pl[i].xy;
		sZ2 += i * i;
		sZX += i * pl[i].xy;
	}
	sZ /= N;
	sX /= N;
	sZ2 /= N;
	sZX /= N;
	tr->A = (sZX - sZ * sX) / (sZ2 - sZ * sZ);
	tr->B = sX - tr->A * sZ;
//		Check xits on the track
	cnt = 0;
	for (i = tr->Zmin; i <= tr->Zmax; i++) if (pl[i].E > 0) {
		x = i * tr->A + tr->B;
		if (fabs(pl[i].xy - x) > 1) cnt++;
	}
	if (cnt) return 1;
	return 0;
}

/*	Consider that all muons come from the top hemisphere	*/
/*	and look for the next strip for 3D track given. Check	*/
/*	that possible strip is availble and not dead. Cut 4	*/
/*	bottom layers as well as all edge strips. Return 0 if	*/
/*	OK, 1 on failure.					*/
int FindEndPoint(struct TrackProjStruct *TX, struct TrackProjStruct *TY, int &iZ, int &iXY)
{
	int ix, iy;
	
	ix = 2 * TX->Zmin + 1;
	iy = 2 * TY->Zmin;
	iZ = std::min(ix, iy) - 1;			// next Z
	if (iZ < 4) return 1;
	ix = 0.5 + TX->A * (iZ - 1) / 2.0 + TX->B;	// next X
	iy = 0.5 + TY->A * iZ / 2.0 + TY->B;		// next Y
	if (ix < 1 || ix > MAXXY - 2) return 1;
	if (iy < 1 || iy > MAXXY - 2) return 1;
	iXY = (iZ & 1) ? ix : iy;
	if (DeadChannels[iZ][iXY]) return 1;
	return 0;
}

int main(int argc, char **argv)
{
	const char LeafList[] = 
		"index/I:"		// Index in the DanssEvent tree
		"globalTime/L:"		// global time 125 MHz ticks
		"Z/I:"			// z of the stopping strip
		"XY/I:"			// xy of the stopping strip
		"thetaX/F:"		// angle of the track in ZX
		"thetaY/F:"		// angle of the track in ZX
		"NHits/I:"		// number of hits
		"Ehit[NHits]/F";	// hit energy
	
	struct StoppedMuonStruct StoppedMuon;
	TTree *EventTree = NULL;
	TTree *tOut = NULL;
	TFile *fOut = NULL;
	TFile *fIn = NULL;
	char str[1024];
	int RunNumber;
	int i, j, k, N, irc;
	const char *rootdir = "/home/clusters/rrcmpi/alekseev/igor/root8n2";
	const char *outdir = "/home/clusters/rrcmpi/alekseev/igor/stopped8n2";
	double ZX[MAXZ/2][MAXXY];
	double ZY[MAXZ/2][MAXXY];
	struct PlaneHitStruct XX[MAXZ/2];
	struct PlaneHitStruct YY[MAXZ/2];
	struct TrackProjStruct TX;
	struct TrackProjStruct TY;
	int iZ, iXY;
//			Check number of arguments
	if (argc < 2) {
		printf("Usage: %s runnumber [rootdir [outdir]]\n", argv[0]);
		printf("Will process run and create root-file with stopped muons\n");
		return 10;
	}
//			Process arguments
	RunNumber = strtol(argv[1], NULL, 10);
	if (argc > 2) rootdir = argv[2];
	if (argc > 3) outdir = argv[3];
//			Open files
	sprintf(str, "%s/%3.3dxxx/danss_%6.6d.root", rootdir, RunNumber/1000, RunNumber);
	fIn = new TFile(str);
	if (!fIn->IsOpen()) return 12;
	EventTree = (TTree *) fIn->Get("DanssEvent");
	if (!EventTree) {
		printf("No EventChain in %s\n", str);
		return 13;
	}
	EventTree->SetBranchAddress("Data", &DanssEvent);
	EventTree->SetBranchAddress("HitE", &HitData.E);
	EventTree->SetBranchAddress("HitT", &HitData.T);
	EventTree->SetBranchAddress("HitType", &HitData.type);

	sprintf(str, "mkdir -p %s/%3.3dxxx", outdir, RunNumber/1000);
	i = system(str);
	if (i) {
		printf("Can not make the target directory: %s %m\n", str);
		return 15;
	}
	sprintf(str, "%s/%3.3dxxx/stopped_%6.6d.root", outdir, RunNumber/1000, RunNumber);
	fOut = new TFile(str, "RECREATE");
	if (!fOut->IsOpen()) return 16;
//			Create output Chain
	tOut = new TTree("StoppedMuons", "Stopped muons");
	tOut->Branch("Stopped", &StoppedMuon, LeafList);

	CreateDeadMap(EventTree);
//			Main cycle
	N = EventTree->GetEntries();
	for (i=0; i<N; i++) {
		EventTree->GetEntry(i);
		if (DanssEvent.TotalEnergy < 20) continue;
//			Create hit table
		memset(ZX, 0, sizeof(ZX));
		memset(ZY, 0, sizeof(ZY));
		for (j=0; j<DanssEvent.NHits; j++) if (HitData.type[j].type == 0) {
			if (HitData.type[j].z & 1) {
				ZX[HitData.type[j].z / 2][HitData.type[j].xy] = HitData.E[j];
			} else {
				ZY[HitData.type[j].z / 2][HitData.type[j].xy] = HitData.E[j];
			}
		}
//			Create hit planes
		memset(XX, 0, sizeof(XX));
		memset(YY, 0, sizeof(YY));
		irc = 0;
		for (j=0; j<MAXZ/2; j++) {
			irc += MergeHits(ZX[j], &XX[j]);
			irc += MergeHits(ZY[j], &YY[j]);
		}
		if (irc) continue;		// Use gold tracks only
//		printf("**************************** ZX *******************************\n");
//		for (j=0; j<MAXZ/2;j++) {
//			printf("\t|");
//			for (k=0; k<MAXXY; k++) printf("%c ", (ZX[j][k] > 0) ? '*' : ' ');
//			printf("|\t%5.1f  %5.1f MeV\n", XX[j].xy, XX[j].E);
//		}
//		printf("**************************** ZY *******************************\n");
//		for (j=0; j<MAXZ/2;j++) {
//			printf("\t|");
//			for (k=0; k<MAXXY; k++) printf("%c ", (ZY[j][k] > 0) ? '*' : ' ');
//			printf("|\t%5.1f  %5.1f MeV\n", YY[j].xy, YY[j].E);
//		}
//		printf("----------------------------------------------------------------\n");
//			Find track
		irc += FindTrack(XX, &TX);
		irc += FindTrack(YY, &TY);
		if (irc) continue;		// no good track
//			Find the end point strip
		irc = FindEndPoint(&TX, &TY, iZ, iXY);
		if (irc) continue;		// bad endpoint
		
		printf("**************************** ZX *******************************\n");
		for (j=MAXZ/2-1; j>=0; j--) {
			printf("%d\t|", 2*j+1);
			for (k=0; k<MAXXY; k++) printf("%c ", (ZX[j][k] > 0) ? '*' : ' ');
			printf("|\t%5.1f  %5.1f MeV\n", XX[j].xy, XX[j].E);
		}
		printf("**************************** ZY *******************************\n");
		for (j=MAXZ/2-1; j>=0; j--) {
			printf("%d\t|", 2*j);
			for (k=0; k<MAXXY; k++) printf("%c ", (ZY[j][k] > 0) ? '*' : ' ');
			printf("|\t%5.1f  %5.1f MeV\n", YY[j].xy, YY[j].E);
		}
		printf("**************************** PARS *****************************\n");
		printf("\tZ = %2d   XY=%2d\n", iZ, iXY);
		
		printf("----------------------------------------------------------------\n");
		
	}

	tOut->Write();
	fOut->Close();
	fIn->Close();
	return 0;
}
