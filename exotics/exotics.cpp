/****************************************************************************************
 *	DANSS data analysis - find possible exotics					*
 *	nu-e scattering: isolated vertical track of 3 strips. No other activity.	*
 *		top and bottom strips exactly above each other. Energy ranges:		*
 *		0.2-3.5 MeV, 1-3.5 MeV, 0.2-3.5 MeV. total 3-strip energy > 3 MeV.	*
 *		Other activity < 0.1 MeV and < 2 strips. All 3 strips must not be in	*
 *		4 bottom or top layers and no strip on side edges.			*
 *	axion: Cluster 5-10 MeV. Gamma > 0.2 MeV.					*
 *	dark neutron: Gadolinium capture 7-10 MeV. No other limits.			*
 *	For all events nothing in vicinity +- 100 us					*
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
#define VETO		(200 * GFREQ2US)	// 100 us veto 
#define ISOLATION	(200 * GFREQ2US)	// 100 us isolation
#define MAXZ	100
#define MAXXY	25
#define iMaxDataElements 3000
#define DEBUG 0

#define TYPE_NUE	0x10	// nu-e elastic 
#define TYPE_AXION	0x20	// axion
#define TYPE_DARKN	0x40	// dark neutron

#pragma pack(push,1)
struct ExoticsStruct {
	int index;		// Index in the DanssEvent tree
	int type;		// Event type
	long long globalTime;	// global time 125 MHz ticks
	double E;		// characteristic enery
	double Est[3];		// energy for each of the 3 strips (nu-e) [Index counts along the track]
	double Egamma;		// ouside clustwer energy for axion-like search
	double X[3];		// event position
};
#pragma pack(pop)


int IsItNuE(double ZX[MAXZ/2][MAXXY], double ZY[MAXZ/2][MAXXY], int &iZ, int &iXY, int &iXY1)
{
	int i, j, k, n;
	double E, Etot;
	
	n = 0;
	Etot = 0;
	for (i = 0; i < MAXZ/2; i++) for (j = 0; j < MAXXY; j++) {
		if (ZX[i][j] > 0) {
			n++;
			Etot += ZX[i][j];
		}
		if (ZY[i][j] > 0) {
			n++;
			Etot += ZY[i][j];
		}
	}
	if (n > 4 || Etot > 11) return 0;
	//	middle strip in X
	for (i = 2; i < MAXZ/2 - 3; i++) {
		for (j = 1; j < MAXXY - 1; j++) if (ZY[i][j] > 0.2 && ZY[i+1][j] > 0.2 
			&& ZY[i][j] < 3.5 && ZY[i+1][j] < 3.5) break;
		if (j < MAXXY - 1) break;
	}
	if (i < MAXZ/2 - 3) {
		for (k = 1; k < MAXXY - 1; k++) if (ZX[i][k] > 1 && ZX[i][k] < 3.5) break;
		if (k < MAXXY - 1) {
			E = ZY[i][j] + ZY[i+1][j] + ZX[i][k];
			if (Etot - E < 0.1) {
				iZ = 2 * i + 1;
				iXY = k;
				iXY1 = j;
				return 1;
			}
		}
	}
	//	middle strip in Y
	for (i = 2; i < MAXZ/2 - 3; i++) {
		for (j = 1; j < MAXXY - 1; j++) if (ZX[i][j] > 0.2 && ZX[i+1][j] > 0.2 
			&& ZX[i][j] < 3.5 && ZX[i+1][j] < 3.5) break;
		if (j < MAXXY - 1) break;
	}
	if (i < MAXZ/2 - 3) {
		for (k = 1; k < MAXXY - 1; k++) if (ZY[i+1][k] > 1 && ZY[i+1][k] < 3.5) break;
		if (k < MAXXY - 1) {
			E = ZX[i][j] + ZX[i+1][j] + ZY[i+1][k];
			if (Etot - E < 0.1) {
				iZ = 2 * (i + 1);
				iXY = k;
				iXY1 = j;
				return 1;
			}
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	const char LeafList[] = 
		"index/I:"		// Index in the DanssEvent tree
		"type/I:"		// Event type
		"globalTime/L:"		// global time 125 MHz ticks
		"E/D:"			// characteristic energy
		"Est[3]/D:"		// energy for each of the 3 strips
		"Egamma/D:"		// ouside clustwer energy for axion-like search
		"X[3]";			// event position
	struct HitStruct {
		float			E[iMaxDataElements];
		float			T[iMaxDataElements];
		struct HitTypeStruct 	type[iMaxDataElements];
	} HitData;
	struct DanssEventStruct7 DanssEvent;
	struct ExoticsStruct Exotics;
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
	int i, j, N, irc;
	const char *rootdir = "/home/clusters/rrcmpi/alekseev/igor/root8n2";
	const char *outdir = "/home/clusters/rrcmpi/alekseev/igor/exotics8n2";
	double ZX[MAXZ/2][MAXXY];
	double ZY[MAXZ/2][MAXXY];
	int Stat[10];
	const char *StatName[10] = {
		"Total triggers       ", 
		"Nothing before       ",
		"> 3 MeV              ",
		"Some exotics         ",
		"Nothing after        ",
		"nu-e                 ", 
		"dark neutron         ", 
		"axion                ", 
		"", ""};
	long long gtOld;
	double E;
	int iZ, iXY, iXY1;
	
//			Check number of arguments
	if (argc < 2) {
		printf("Usage: %s runnumber [rootdir [outdir]]\n", argv[0]);
		printf("Will process run and create root-file with exotic candidates\n");
		return 10;
	}
//			Process arguments & open files
	if (argc > 2) rootdir = argv[2];
	if (argc > 3) outdir = argv[3];
	if (isdigit(argv[1][0])) {
		RunNumber = strtol(argv[1], NULL, 10);
		sprintf(filein, "%s/%3.3dxxx/danss_%6.6d.root", rootdir, RunNumber/1000, RunNumber);
		sprintf(fileout, "%s/%3.3dxxx/exotics_%6.6d.root", outdir, RunNumber/1000, RunNumber);
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
	tOut = new TTree("Exotics", "Exotics");
	tOut->Branch("Exotics", &Exotics, LeafList);
	if (EventTree->GetBranch("MCEvent")) tOut->Branch("MCEventID", &MCEventID, "MCEventID/I");

//			Main cycle
	N = EventTree->GetEntries();
	memset(Stat, 0, sizeof(Stat));
	gtOld = -VETO;
	for (i=0; i<N; i++) {
		EventTree->GetEntry(i);
		memset(&Exotics, 0, sizeof(Exotics));
		Stat[0]++;
//			Check VETO
		if (DanssEvent.globalTime - gtOld < VETO) {
			gtOld = DanssEvent.globalTime;
			continue;
		}
		gtOld = DanssEvent.globalTime;
		Stat[1]++;
		if (DanssEvent.TotalEnergy < 3) continue;	// 3 Mev minimum for anything
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
		Stat[2]++;
//			Look for the exotics
		if (IsItNuE(ZX, ZY, iZ, iXY, iXY1)) {			// nu-e
			Exotics.type = TYPE_NUE;
			if (iZ & 1) {	// central strip X
				Exotics.Est[0] = ZY[(iZ + 1) / 2][iXY1];
				Exotics.Est[1] = ZX[iZ / 2][iXY];
				Exotics.Est[2] = ZY[(iZ - 1) / 2][iXY1];
				Exotics.X[0] = 4.0 * iXY;
				Exotics.X[1] = 4.0 * iXY1;
			} else {	// central strip Y
				Exotics.Est[0] = ZX[(iZ + 1) / 2][iXY1];
				Exotics.Est[1] = ZY[iZ / 2][iXY];
				Exotics.Est[2] = ZX[(iZ - 1) / 2][iXY1];
				Exotics.X[0] = 4.0 * iXY1;
				Exotics.X[1] = 4.0 * iXY;
			}
			Exotics.E = Exotics.Est[0] + Exotics.Est[1] + Exotics.Est[2];
			Exotics.X[2] = iZ;
		} else if (DanssEvent.PositronEnergy > 5 && DanssEvent.PositronEnergy < 10 && DanssEvent.AnnihilationGammas > 0.2 
			&& DanssEvent.PositronX[0] > 2 && DanssEvent.PositronX[0] < 94
			&& DanssEvent.PositronX[1] > 2 && DanssEvent.PositronX[1] < 94
			&& DanssEvent.PositronX[2] > 3.5 && DanssEvent.PositronX[2] < 95.5) {	// axion
			Exotics.type = TYPE_AXION;
			Exotics.E = DanssEvent.PositronEnergy;
			Exotics.X[0] = DanssEvent.PositronX[0];
			Exotics.X[1] = DanssEvent.PositronX[1];
			Exotics.X[2] = DanssEvent.PositronX[2];
		} else if (DanssEvent.NeutronEnergy > 7 && DanssEvent.NeutronEnergy < 10
			&& DanssEvent.NeutronX[0] > 2 && DanssEvent.NeutronX[0] < 94
			&& DanssEvent.NeutronX[1] > 2 && DanssEvent.NeutronX[1] < 94
			&& DanssEvent.NeutronX[2] > 3.5 && DanssEvent.NeutronX[2] < 95.5) {	// Dark neutron
			Exotics.type = TYPE_DARKN;
			Exotics.E = DanssEvent.NeutronEnergy;
			Exotics.X[0] = DanssEvent.NeutronX[0];
			Exotics.X[1] = DanssEvent.NeutronX[1];
			Exotics.X[2] = DanssEvent.NeutronX[2];
		} else continue;
//			Fill the tree
		Exotics.index = i;
		Exotics.globalTime = DanssEvent.globalTime;
		Exotics.Egamma = DanssEvent.AnnihilationGammas;
		lfEventID = EventTree->GetLeaf("EventID");
		if (lfEventID) MCEventID = lfEventID->GetValueLong64();
		Stat[3]++;
//			Look for isolation cut. We look for any event in the window
		if (i+1 < N) {
			EventTree->GetEntry(i+1);
			if (DanssEvent.globalTime > Exotics.globalTime &&
				DanssEvent.globalTime < Exotics.globalTime + ISOLATION) continue;
		}
		Stat[4]++;
		switch(Exotics.type) {
		case TYPE_NUE:
			Stat[5]++;
			break;
		case TYPE_AXION:
			Stat[6]++;
			break;
		case TYPE_DARKN:
			Stat[7]++;
			break;
		
		}
		tOut->Fill();
	}

	tOut->Write();
	fOut->Close();
	fIn->Close();
	printf("\t\tStatics:\n");
	for (i=0; i<sizeof(Stat) / sizeof(Stat[0]); i++) if (strlen(StatName[i]) > 1) 
		printf("\t%s :\t%d\n", StatName[i], Stat[i]);
	return 0;
}
