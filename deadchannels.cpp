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

#define MAXMODULE	56
#define MAXCHANNEL	64
#define MINEVENTS	1000000
#define MINHITS		300


//	Translate module and channel numbers to Period 0 settings
void ModTranslate(int Period, int hMod, int hChan, int &mod, int &chan)
{
	chan = hChan;	// no channel translation so far
	
	switch (Period) {
	case 1:
		mod = (hMod == 4) ? 47 : hMod;
		break;
	case 2:
		switch (hMod) {
		case 4:
			mod = 47;	// module 4 is the replacement of 47
			break;
		case 20:
			mod = 25;	// later module 20 replaced 25
			break;
		case 24:
			mod = (hChan < 16) ? 25 : 24;	// connector 0 of 25 was moved to 24
			break;
		case 25:
			mod = 20;	// module 25 is the replacement of 20
			break;
		case 51:
			mod = 13;	// module 51 is the replacement of 13
			break;
		case 52:
			mod = 43;	// module 52 is the replacement of 43
			break;
		default:
			mod = hMod;
			break;
		}
		break;
	default:
		mod = hMod;
		break;
	}
}

void process(int run, const char *fmt, FILE *fOut)
{
/*
    We have these variations of the board assigments so far:
    0 - initial variation, used in MC : runs 2210  - 59260
    1 - board 47 replaced by board 4  : runs 61541 - 69739
    2 - board 13 replaced by board 51 : runs 69765 - XXXXXX
        board 20 replaced by board 25
        and connector 25.0 moved to 24.0 
        board 43 replaced by board 52 and then 43 was put back
        later board 20 was installed and former connector 24.0 was put to it
    Boards appearence/disappearence:
    47 - 59260 - last run
    4  - 61541 - first run
    13 - 69739 - last run
    20 - 69739 - last run
    20 - 110848 - the first run with it back on the new place
    51 - 69765 - first run
    25.1 --> 24.1 - 69765 - first run
    20 --> 25 - 69765 - first run
    43 - 89039 - last run
    43 - 110848 - first run back
    52 - 89360 - first run
    52 - 110801 - last run
    
*/
	const unsigned short ChanMask[MAXMODULE][4] = {
//			Runs 2210 - 61540 - we translate everything to this module/channel numeration
		{0, 0, 0, 0},				// 1 - ignore PMT
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 2
		{0, 0, 0, 0},				// 3 - ignore VETO
		{0, 0, 0, 0},				// 4 - not now
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 5
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 6
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 7
		{0x36DB, 0x7FFF, 0x7FFF, 0x7FFF},	// 8 - 2/3 on connector 1
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 9
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 10
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 11
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 12
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 13
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 14
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 15
		{0x7FFF, 0x7FFF, 0x36DB, 0x7FFF},	// 16 - 2/3 on connector 3
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 17
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 18
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 19
		{0x7FFF, 0x7FFF, 0x7FFF, 0x36DB},	// 20 - 2/3 on connector 4
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 21
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 22
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 23
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 24
		{0x36DB, 0, 0, 0},			// 25 - only one connector 2/3 filled
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 26
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 27
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 28
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 29
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 30
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 31
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 32
		{0x36DB, 0x7FFF, 0x7FFF, 0x7FFF},	// 33 - 2/3 on connector 1
		{0x7FFF, 0x36DB, 0x7FFF, 0x7FFF},	// 34 - 2/3 on connector 2
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 35
		{0x7FFF, 0x7FFF, 0x7FFF, 0x36DB},	// 36 - 2/3 on connector 4
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 37
		{0x7FFF, 0x36DB, 0x7FFF, 0x7FFF},	// 38 - 2/3 on connector 2
		{0x7FFF, 0x7FFF, 0x36DB, 0x7FFF},	// 39 - 2/3 on connector 3
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 40
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 41
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 42
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 43
		{0x7FFF, 0x36DB, 0x7FFF, 0x7FFF},	// 44 - 2/3 on connector 2
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 45
		{0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF},	// 46
		{0x7FFF, 0, 0, 0},			// 47 - only one connector
		{0, 0, 0, 0},				// 48 - never used
		{0, 0, 0, 0},				// 49 - never used
		{0, 0, 0, 0},				// 50 - never used
		{0, 0, 0, 0},				// 51 - not now
		{0, 0, 0, 0},				// 52 - not now
		{0, 0, 0, 0},				// 53 - never used
		{0, 0, 0, 0},				// 54 - never used
		{0, 0, 0, 0},				// 55 - never used
		{0, 0, 0, 0}				// 56 - never used
	};
	unsigned short ChanMet[MAXMODULE][4];
	TTree *DanssEvent;
	TH1 *h;
	TFile *f;
	char str[1024];
	char lstr[32000];	// very long string to be on the safe side
	int i, j;
	int ptr;
	int Cnt[2];
	int Period;
	int mod, chan;

	memset(ChanMet, 0, sizeof(ChanMet));
	if (run < 61541) {
		Period = 0;
	} else if (run < 69765) {
		Period = 1;
	} else {
		Period = 2;
	}

	sprintf(str, fmt, run/1000, run);
	if (access(str, R_OK)) return;
	f = new TFile(str);
	if (!f->IsOpen()) return;
	
	DanssEvent = (TTree *) f->Get("DanssEvent");
	if (!DanssEvent) return;
	if (DanssEvent->GetEntries() < MINEVENTS) {
		f->Close();
		return;
	}
	
	// Fill channel present map
	for (i=0; i<MAXMODULE; i++) for (j=0; j<MAXCHANNEL; j++) {
		sprintf(str, "hDT%2.2dc%2.2d", i+1, j);
		h = (TH1 *) f->Get(str);
		if (h && h->Integral(101, 150) >= MINHITS) {
			ModTranslate(Period, i + 1, j, mod, chan);
			ChanMet[mod-1][chan/16] |= 1 << (chan % 16);
		}
	}
	// Write the result
	lstr[0] = '\0';
	ptr = 0;
	Cnt[0] = Cnt[1] = 0;
	for (i=0; i<MAXMODULE; i++) for (j=0; j<MAXCHANNEL; j++) if (ChanMask[i][j/16] & (1 << (j%16))) {
		if (!(ChanMet[i][j/16] & (1 << (j%16)))) {
			ptr += sprintf(&lstr[ptr], " %2.2d.%2.2d", i + 1, j);
			Cnt[1]++;
		} else {
			Cnt[0]++;
		}
	}

	fprintf(fOut, "%6d %5d %5d %s\n", run, Cnt[0], Cnt[1], lstr);

	f->Close();
}

int main(int argc, char **argv)
{
	int run, first, last;
	char fmt[1024];
	FILE *fOut;
	
	if (argc < 4) {
		printf("Usage: %s run_first run_last base_dir\n", argv[0]);
		printf("Count and list dead channels using time hists. Short runs are ignored.\n");
		return 10;
	}
	
	first = strtol(argv[1], NULL, 10);
	last = strtol(argv[2], NULL, 10);
	sprintf(fmt, "%s/deadchan/deadchan_%6.6d_%6.6d.txt", argv[3], first, last);
	fOut = fopen(fmt, "wt");
	if (!fOut) return 20;
	sprintf(fmt, "%s/%%3.3dxxx/danss_%%6.6d.root", argv[3]);
	
	fprintf(fOut, "Run    Alive  Dead  List\n");
	for (run=first; run <= last; run++) process(run, fmt, fOut);
	
	fclose(fOut);
	return 0;
}
