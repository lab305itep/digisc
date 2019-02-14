/****************************************************************************************
 *	DANSS data analysis - calculate dead time					*
 ****************************************************************************************/

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
#define AFTERVETO	(long long) (60 * GFREQ2US)
#define AFTERTRIG	(long long) (45 * GFREQ2US)
#define BEFORETRIG	(long long) (80 * GFREQ2US)

int IsVeto(struct DanssEventStruct7 *Event)
{
	if (Event->VetoCleanEnergy > 4 || Event->VetoCleanHits >= 2 || 
		Event->PmtCleanEnergy + Event->SiPmCleanEnergy > 40 || 
		Event->BottomLayersEnergy > 3) return 1;
	return 0;
}

void process(int run, const char *fmt, FILE *fOut)
{
	struct DanssEventStruct7 DanssEvent;
	TChain *EventChain;
	long long GlobalFirst;
	long long LiveCnt;
	long long PrevEnd;
	long long Delta;
	int iEvt, nEvt;
	double RunTime;
	double LiveTime;
	char *ptr;
	char str[1024];
	int irc;

	EventChain = new TChain("DanssEvent");
	EventChain->SetBranchAddress("Data", &DanssEvent);
	sprintf(str, fmt, run/1000, run);
	irc = EventChain->Add(str, 0);
	if (!irc) return;	// no run

	LiveCnt = 0;
	nEvt = EventChain->GetEntries();
	if (!nEvt) return;
	EventChain->GetEntry(0);
	GlobalFirst = DanssEvent.globalTime;
	PrevEnd = DanssEvent.globalTime + ((IsVeto(&DanssEvent)) ? AFTERVETO : AFTERTRIG);
	
	for (iEvt = 1; iEvt < nEvt; iEvt++) {
		EventChain->GetEntry(iEvt);
		Delta = DanssEvent.globalTime - PrevEnd;
		Delta -= BEFORETRIG;
		if (Delta > 0) LiveCnt += Delta;
		PrevEnd = DanssEvent.globalTime + ((IsVeto(&DanssEvent))? AFTERVETO : AFTERTRIG);
	}
	
	RunTime = (DanssEvent.globalTime - GlobalFirst) / GLOBALFREQ;
	LiveTime = LiveCnt / GLOBALFREQ;
	fprintf(fOut, "%6d  %6.1f  %6.1f  %6.1f  %7.5f\n", 
		run, RunTime, LiveTime, RunTime - LiveTime, (RunTime - LiveTime) / RunTime);
	delete EventChain;
}

int main(int argc, char **argv)
{
	int run, first, last;
	char fmt[1024];
	FILE *fOut;
	
	if (argc < 4) {
		printf("Usage: %s run_first run_last base_dir\n", argv[0]);
		printf("Will process files and count time alive\n");
		return 10;
	}
	
	
	first = strtol(argv[1], NULL, 10);
	last = strtol(argv[2], NULL, 10);
	sprintf(fmt, "dead_%6.6d_%6.6d.txt", first, last);
	fOut = fopen(fmt, "wt");
	if (!fOut) return 20;
	sprintf(fmt, "%s/%%3.3dxxx/danss_%%6.6d.root", argv[3]);
	
	fprintf(fOut, "Run     Total   Alive   Dead    Fraction\n");
	for (run=first; run <= last; run++) process(run, fmt, fOut);
	
	fclose(fOut);
	return 0;
}
