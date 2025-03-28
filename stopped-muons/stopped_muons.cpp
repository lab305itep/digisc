/****************************************************************************************
 *	DANSS data analysis - find stopped muons					*
 ****************************************************************************************/
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TBranch.h"
#include "TChain.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"

#include "../evtbuilder.h"

#define GFREQ2US	(GLOBALFREQ / 1000000.0)
#define VETO	(20 * GFREQ2US)	// 20 us veto 
#define DECAY	(10 * GFREQ2US)	// 10 us to decay
#define MAXZ	100
#define MAXXY	25
#define iMaxDataElements 3000
#define masterTrgRandom 2
#define DEBUG 0

#pragma pack(push,1)
struct StoppedMuonStruct {
	int index;		// Index in the DanssEvent tree
	int flags;		// some flags (or'ed). 1 - Decay was found later.
	long long globalTime;	// global time 125 MHz ticks
	int Z;			// z of the stopping strip
	int XY;			// xy of the stopping strip
	float thetaX;		// angle of the track in ZX, 0 - vertical
	float thetaY;		// angle of the track in ZY, 0 - vertical
	int NHits;		// number of hits including empty hits
	float Ehit[MAXZ];	// hit energy
};
#pragma pack(pop)

struct HitStruct {
	float			E[iMaxDataElements];
	float			T[iMaxDataElements];
	struct HitTypeStruct 	type[iMaxDataElements];
} HitData;

struct PlaneHitStruct {
	float xy;		// resulting coordinate
	float E;		// resulting E
	int n;			// number of strips (1 or 2)
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
	int i, j, cnt, n;
	double x;
	double E;
	cnt = 0;
	x = 0;
	E = 0;
	n = 0;
	for (i=0; i<MAXXY; i++) if (pl[i] > 0) {
		if (cnt > 0) return 1;
		x = i * pl[i];
		E = pl[i];
		n = 1;
		if (i < MAXXY - 1 && pl[i+1] > 0) {
			x += (i+1) * pl[i+1];
			E += pl[i+1];
			n++;
			i++;
		}
		x /= E;
		cnt++;
	}
	hit->xy = x;
	hit->E = E;
	hit->n = n;
	return 0;
}

/*	Look for a track as a continuous set of hit planes	*/
/*	Require minimum of 5 planes and maximum total breaks	*/
/*	of 5+ planes. Hits must be less than 1 strip aside	*/
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
	if (cnt > 4 + (tr->Zmax - tr->Zmin + 1) / 5) return 1;
	N = tr->Zmax - tr->Zmin - cnt + 1;
	if (N < 5) return 1;
//		Find track parameters
	sZ = sX = sZ2 = sZX = 0;
	for (i = tr->Zmin; i <= tr->Zmax; i++) if (pl[i].E > 0) {
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
/*	bottom layers as well as all edge strips.		*/
/*	Return 0 if OK, 1 on failure.				*/
/*	iZ - z of the end point.				*/
/*	iXY - xy of the end point.				*/
/*	NHits - number of points on the track including empty	*/
int FindEndPoint(struct TrackProjStruct *TX, struct TrackProjStruct *TY, int &iZ, int &iXY, int &NHits)
{
	int ix, iy, znext, xynext;
	
	ix = 2 * TX->Zmin + 1;
	iy = 2 * TY->Zmin;
	znext = std::min(ix, iy) - 1;			// next Z
	if (znext < 3) return 1;
	ix = 0.5 + TX->A * (znext - 1) / 2.0 + TX->B;	// next X
	iy = 0.5 + TY->A * znext / 2.0 + TY->B;		// next Y
	if (ix < 1 || ix > MAXXY - 2) return 1;
	if (iy < 1 || iy > MAXXY - 2) return 1;
	xynext = (znext & 1) ? ix : iy;
	if (DeadChannels[znext][xynext]) return 1;
	iZ = znext + 1;
	iXY = (iZ & 1) ? 0.5 + TX->A * (iZ - 1) / 2.0 + TX->B : 0.5 + TY->A * iZ / 2.0 + TY->B;
	ix = 2 * TX->Zmax + 1;
	iy = 2 * TY->Zmax;
	NHits = std::max(ix, iy) - iZ + 1;
	return 0;
}

int main(int argc, char **argv)
{
	const char LeafList[] = 
		"index/I:"		// Index in the DanssEvent tree
		"flags/I:"		// Event flags. So far bit 0: decay found
		"globalTime/L:"		// global time 125 MHz ticks
		"Z/I:"			// z of the stopping strip
		"XY/I:"			// xy of the stopping strip
		"thetaX/F:"		// angle of the track in ZX
		"thetaY/F:"		// angle of the track in ZX
		"NHits/I:"		// number of hits
		"Ehit[NHits]/F";	// hit energy
	
	struct StoppedMuonStruct StoppedMuon;
	int MCEventID;
	TLeaf *lfEventID;
	TTree *EventTree = NULL;
	TTree *tOut = NULL;
	TFile *fOut = NULL;
	TFile *fIn = NULL;
	char filein[1024];
	char fileout[1024];
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
	int Stat[10];
	const char *StatName[10] = {
		"Total triggers       ", 
		"Total energy > 20 MeV", 
		"No muon before       ",
		"Gold planes          ", 
		"Good track           ", 
		"Stopped muon         ", 
		"Good last hits       ",
		"Decay found          ", 
		"", ""};
	int iZ, iXY, iXYn, NHits;
	int iFound;
	long long gtOld;
	double E;
	
//			Check number of arguments
	if (argc < 2) {
		printf("Usage: %s runnumber [rootdir [outdir]]\n", argv[0]);
		printf("Will process run and create root-file with stopped muons\n");
		return 10;
	}
//			Process arguments & open files
	if (argc > 2) rootdir = argv[2];
	if (argc > 3) outdir = argv[3];
	if (isdigit(argv[1][0])) {
		RunNumber = strtol(argv[1], NULL, 10);
		sprintf(filein, "%s/%3.3dxxx/danss_%6.6d.root", rootdir, RunNumber/1000, RunNumber);
		sprintf(fileout, "%s/%3.3dxxx/stopped_%6.6d.root", outdir, RunNumber/1000, RunNumber);
	} else {
		sprintf(filein, "%s/%s", rootdir, argv[1]);
		sprintf(fileout, "%s/%s", outdir, argv[1]);
	}
	fIn = new TFile(filein);
	if (!fIn->IsOpen()) return 12;
	EventTree = (TTree *) fIn->Get("DanssEvent");
	if (!EventTree) {
		printf("No EventChain in %s\n", filein);
		return 13;
	}
	EventTree->SetBranchAddress("Data", &DanssEvent);
	EventTree->SetBranchAddress("HitE", &HitData.E);
	EventTree->SetBranchAddress("HitT", &HitData.T);
	EventTree->SetBranchAddress("HitType", &HitData.type);
	sprintf(str, "mkdir -p `dirname %s`", fileout);
	i = system(str);
	if (i) {
		printf("Can not make the target directory for %s %m\n", fileout);
		return 15;
	}
	fOut = new TFile(fileout, "RECREATE");
	if (!fOut->IsOpen()) return 16;
//			Create output Chain
	tOut = new TTree("StoppedMuons", "Stopped muons");
	tOut->Branch("Stopped", &StoppedMuon, LeafList);
	if (EventTree->GetBranch("MCEvent")) tOut->Branch("MCEventID", &MCEventID, "MCEventID/I");

	CreateDeadMap(EventTree);
//			Main cycle
	N = EventTree->GetEntries();
	memset(Stat, 0, sizeof(Stat));
	gtOld = -VETO;
	for (i=0; i<N; i++) {
		EventTree->GetEntry(i);
		Stat[0]++;
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
		Stat[1]++;
//			Check VETO
		if (DanssEvent.globalTime - gtOld < VETO) {
			gtOld = DanssEvent.globalTime;
			continue;
		}
		gtOld = DanssEvent.globalTime;
		Stat[2]++;
//			Create hit planes
		memset(XX, 0, sizeof(XX));
		memset(YY, 0, sizeof(YY));
		irc = 0;
		for (j=0; j<MAXZ/2; j++) {
			irc += MergeHits(ZX[j], &XX[j]);
			irc += MergeHits(ZY[j], &YY[j]);
		}
		if (irc) continue;		// Use gold tracks only
		Stat[3]++;
		if (DEBUG & 1) {
			printf("**************************** ZX %8d *******************************\n", i);
			for (j=MAXZ/2-1; j>=0; j--) {
				printf("%d\t|", 2*j+1);
				for (k=0; k<MAXXY; k++) printf("%c ", (ZX[j][k] > 0) ? '*' : ' ');
				printf("|\t%5.1f  %5.1f MeV\n", XX[j].xy, XX[j].E);
			}
			printf("**************************** ZY %8d *******************************\n", i);
			for (j=MAXZ/2-1; j>=0; j--) {
				printf("%d\t|", 2*j);
				for (k=0; k<MAXXY; k++) printf("%c ", (ZY[j][k] > 0) ? '*' : ' ');
				printf("|\t%5.1f  %5.1f MeV\n", YY[j].xy, YY[j].E);
			}
		}
//			Find track
		irc += FindTrack(XX, &TX);
		irc += FindTrack(YY, &TY);
		if (irc) continue;		// no good track
		Stat[4]++;
//			Find the end point strip
		irc = FindEndPoint(&TX, &TY, iZ, iXY, NHits);
		if (irc) continue;		// bad endpoint
		Stat[5]++;
		if (DEBUG & 2) {
			printf("**************************** ZX %8d *******************************\n", i);
			for (j=MAXZ/2-1; j>=0; j--) {
				printf("%d\t|", 2*j+1);
				for (k=0; k<MAXXY; k++) printf("%c ", (ZX[j][k] > 0) ? '*' : ' ');
				printf("|\t%5.1f  %5.1f MeV\n", XX[j].xy, XX[j].E);
			}
			printf("**************************** ZY %8d *******************************\n", i);
			for (j=MAXZ/2-1; j>=0; j--) {
				printf("%d\t|", 2*j);
				for (k=0; k<MAXXY; k++) printf("%c ", (ZY[j][k] > 0) ? '*' : ' ');
				printf("|\t%5.1f  %5.1f MeV\n", YY[j].xy, YY[j].E);
			}
		}
//			Check the end point and the previous point
//			in both of these points exactly one strip should be hit and
//			it should not be near the edge
		if (iZ & 1) {
			if (XX[iZ / 2].n != 1 || YY[(iZ+1) / 2].n != 1) continue;
			if (YY[(iZ+1) / 2].xy < 0.5 || YY[(iZ+1) / 2].xy > MAXXY - 1.5) continue;
		} else {
			if (YY[iZ / 2].n != 1 || XX[(iZ+1) / 2].n != 1) continue;
			if (XX[(iZ+1) / 2].xy < 0.5 || XX[(iZ+1) / 2].xy > MAXXY - 1.5) continue;
		}
		Stat[6]++;
		if (DEBUG & 4) {
			printf("**************************** Index %8d *****************************\n", i);
			printf("\ttrack X: [%d-%d] %f*z + %f\n", 2*TX.Zmin+1, 2*TX.Zmax+1, TX.A, TX.B);
			printf("\ttrack Y: [%d-%d] %f*z + %f\n", 2*TY.Zmin, 2*TY.Zmax, TY.A, TY.B);
			printf("\tStopping point: Z = %2d   XY=%2d   N=%2d\n", iZ, iXY, NHits);
		}
//			Fill the tree
		StoppedMuon.index = i;
		StoppedMuon.flags = 0;
		StoppedMuon.globalTime = DanssEvent.globalTime;
		StoppedMuon.Z = iZ;
		StoppedMuon.XY = iXY;
		StoppedMuon.thetaX = atan(2 * TX.A);
		StoppedMuon.thetaY = atan(2 * TY.A);
		StoppedMuon.NHits = NHits;
		for (j=0; j <NHits; j++) {
			if ((iZ + j) & 1) {
				E = (XX[(iZ + j) / 2].n == 1) ?		// don't use energy from split hits
					XX[(iZ + j) / 2].E / SiPMYAverageLightColl(4.0*(0.5*(iZ+j)*TY.A + TY.B)) : 0;
			} else {
				E = (YY[(iZ + j) / 2].n == 1) ?
					YY[(iZ + j) / 2].E / SiPMYAverageLightColl(4.0*(0.5*(iZ+j-1)*TX.A + TX.B)) : 0;
			}
			StoppedMuon.Ehit[j] = E;
		}
		lfEventID = EventTree->GetLeaf("EventID");
		if (lfEventID) MCEventID = lfEventID->GetValueLong64();
//			Look for decay
//			We consider decay if there is at least 3 MeV energy deposit and the end strip, 
//			or one of its neighbors is hit
		iFound = 0;
		iXYn = ((iZ & 1) ? (iZ * TY.A + TY.B) : (iZ * TX.A + TX.B)) + 0.5;	// iXY in the neighbor layer
		for (j=i+1; j<N; j++) {
			EventTree->GetEntry(j);
			if (DanssEvent.globalTime - StoppedMuon.globalTime > DECAY) break;
			if (DanssEvent.globalTime < StoppedMuon.globalTime) break;	// a rare case of globaltime counter wrap
			if (DanssEvent.TotalEnergy < 3) continue;
			for (k=0; k<DanssEvent.NHits; k++) if (HitData.type[k].type == 0) {
				if (HitData.type[k].z == iZ && HitData.type[k].xy >= iXY - 1 && HitData.type[k].xy <= iXY + 1) {
					iFound = 1;
					break;
				}
				if ((HitData.type[k].z == iZ-1 || HitData.type[k].z == iZ+1) && 
					HitData.type[k].xy >= iXYn - 1 && HitData.type[k].xy <= iXYn + 1) {
					iFound = 1;
					break;
				}
			}
			if (iFound) break;
		}
		if (iFound) {
			StoppedMuon.flags |= 1;
			Stat[7]++;
		}
		tOut->Fill();
		if (DEBUG) printf("----------------------------------------------------------------\n");
	}

	tOut->Write();
	fOut->Close();
	fIn->Close();
	printf("\t\tStatics:\n");
	for (i=0; i<sizeof(Stat) / sizeof(Stat[0]); i++) if (strlen(StatName[i]) > 1) 
		printf("\t%s :\t%d\n", StatName[i], Stat[i]);
	return 0;
}
