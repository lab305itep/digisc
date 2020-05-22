#include <stdio.h>
#include <TChain.h>
#include <TCut.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TLeaf.h>
#include <TROOT.h>
#include "HPainter2.h"

#define NRANDOM	16

//	We need that even zero bins have reasonable errors. 
//	We assume that one sigma range fits in 0 and use Poisson distribution: sigma=0.3817
void MakeNonZeroErrors(TH1 *h)
{
	int i, N;
	N = h->GetNbinsX();
	for (i=0; i<N; i++) if (!h->GetBinError(i+1)) h->SetBinError(i+1, 0.3817);
}

void MakeNonZeroErrors(TH2 *h)
{
	int i, N;
	int j, K;
	N = h->GetNbinsX();
	K = h->GetNbinsY();
	for (i=0; i<N; i++) for (j=0; j<K; j++) if (!h->GetBinError(i+1, j+1)) h->SetBinError(i+1, j+1, 0.3817);
}

//	type = 0 - neutrino
//	type = 1 - muon induced reactions
HPainter2::HPainter2(int mask, int run_from, int run_to, const char *root2dir, int type)
{
	const char *prefix;
	TChain *info;

	fRes = NULL;
	ClosefRes = 0;
	tSig = tRand = tMuRand = NULL;
	upTime = 0;
	tBegin = 0;
	tEnd = 0;
	int *list;
	int lsize;
	int i, num;
	char str[1024];
	
	lsize = run_to - run_from + 1;
	if (lsize <= 0) {
		printf("Nothing to do!\n");
		return;
	}
	list = (int *) malloc(lsize * sizeof(int));
	num = Make_file_list(list, lsize, mask, run_from, run_to);
	if (num <= 0) {
		printf("No runs found!\n");
		return;
	}

	switch (type) {
	case 0:
		tSig = new TChain("DanssPair", "SignalPair");
		tRand = new TChain("DanssRandom", "RandomPair");
		tMuRand = new TChain("DanssMuRandom", "MuRandomPair");
		prefix = "pair";
		break;
	case 1:
		tSig = new TChain("MuonPair", "SignalPair");
		tRand = new TChain("MuonRandom", "RandomPair");
		tMuRand = new TChain("MuonMuRandom", "MuRandomPair");
		prefix = "muon";
		break;
	default:
		return;
	}
	info = new TChain("SumInfo", "Info");

	for (i=0; i<num; i++) {
		sprintf(str, "%s/%3.3dxxx/%s_%6.6d.root", root2dir, list[i]/1000, prefix, list[i]);
		tSig->Add(str);
		info->Add(str);
		tRand->Add(str);
		tMuRand->Add(str);
	}
	
	for (i=0; i<num; i++) {
		info->GetEntry(i);
		upTime += info->GetLeaf("gTime")->GetValue() / 125E6;
	}
	delete info;
	gROOT->cd();
}

HPainter2::~HPainter2(void)
{
	if (ClosefRes && fRes) fRes->Close();
	delete tSig;
	delete tRand;
}

void HPainter2::OpenFile(const char *name)
{
	if (fRes) return;
	fRes = new TFile(name, "UPDATE");
	if (!fRes->IsOpen()) {
		fRes = NULL;
	} else {
		ClosefRes = 1;
	}
	
}

/*
	Make list of runs for analysis
	Return number of runs found, negative on error
	int *list - allocated array to accept the run list
	int size  - size of allocated array
	int mask  - mask of possible conditions
	int run_from, int run_to - runs range to consider
*/
int HPainter2::Make_file_list(int *list, int size, int mask, int run_from, int run_to)
{
	int N;
	FILE *f;
	char str[1024];
	const char *ptr;
	int i, num, cond;

	ptr = getenv("STAT_ALL");
	if (!ptr) ptr = "stat_all.txt";

	f = fopen(ptr, "rt");
	if (!f) {
		printf("Stat file %s not found: %m\n", ptr);
		return -1;
	}

	N = 0;
	for(i=0;;i++) {
		ptr = fgets(str, sizeof(str), f);
		if (!ptr || feof(f)) break;
		ptr = strtok(str, " \t");
		if (!isdigit(ptr[0])) continue;
		num = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Strange record at line %d\n", i);
			continue;
		}
		cond = strtol(ptr, NULL, 10);
		if (num < run_from) continue;
		if (num > run_to) break;
		if (cond <= 0) continue;
		if (mask & (1 << (cond - 1))) {
			list[N] = num;
			N++;
			if (N >= size) break;
		}
	}

	fclose(f);

	return N;
}

/*	Project histogram doing random background subtraction	*/
void HPainter2::Project(TH1 *hist, const char *what, TCut cut)
{
	TH1 *hSig;	// Signal hist
	TH1 *hRand;	// Accidental background (e+ - n)
	TH1 *hMuRand;	// Accidental background (muon - pair)
	TH1 *hDiff;
	TCut ct;
	char str[256];
	char csig[1024];
	char crand[1024];
	char cmurand[1024];
	char cdiff[1024];
	
	if (!IsOpen()) return;
	
	snprintf(csig, sizeof(csig), "%s-sig", hist->GetName());
	snprintf(crand, sizeof(crand), "%s-rand", hist->GetName());
	snprintf(cmurand, sizeof(cmurand), "%s-murand", hist->GetName());
	snprintf(cdiff, sizeof(cdiff), "%s-diff", hist->GetName());
	hSig = (TH1 *) hist->Clone(csig);
	hRand = (TH1 *) hist->Clone(crand);
	hMuRand = (TH1 *) hist->Clone(cmurand);
	hDiff = (TH1 *) hist->Clone(cdiff);
	hSig->SetYTitle("Events");
	hRand->SetYTitle("Events");
	hMuRand->SetYTitle("Events");
	hDiff->SetYTitle("Events");
	if (tBegin < tEnd) {
		sprintf(str, "unixTime > %d && unixTime < %d", tBegin, tEnd);
		ct = cut && str;
		tSig->Project(csig, what, ct);
		tRand->Project(crand, what, ct);
		tMuRand->Project(cmurand, what, ct);
	} else {
		tSig->Project(csig, what, cut.GetTitle());
		tRand->Project(crand, what, cut.GetTitle());
		tMuRand->Project(cmurand, what, cut.GetTitle());
	}
	
	hSig->Sumw2();
	hRand->Sumw2();
	hMuRand->Sumw2();
	MakeNonZeroErrors(hSig);
	MakeNonZeroErrors(hRand);
	MakeNonZeroErrors(hMuRand);
	hRand->Scale(1.0/NRANDOM);
	hMuRand->Scale(1.0/NRANDOM);
	
	hDiff->Add(hSig, hRand, 1.0, -1.0);

	hist->Reset();
	hist->Add(hDiff, 1000.0 / upTime);	// mHz

	if (fRes) {
		fRes->cd();
		hSig->Write();
		hRand->Write();
		hMuRand->Write();	// hMuRand is only calculated and saved. No subtraction here.
		hDiff->Write();
	}
	
//	delete hSig;
//	delete hRand;
//	delete hDiff;
}

/*	Project histogram doing random background subtraction
void HPainter2::Project(TH2 *hist, const char *what, TCut cut)
{
	TH2 *hSig;
	TH2 *hRand;
	TH2 *hDiff;
	TCut ct;
	char str[256];
	char csig[1024];
	char crand[1024];
	char cdiff[1024];
	
	if (!IsOpen()) return;
	
	snprintf(csig, sizeof(csig), "%s-sig", hist->GetName());
	snprintf(crand, sizeof(crand), "%s-rand", hist->GetName());
	snprintf(cdiff, sizeof(cdiff), "%s-diff", hist->GetName());
	hSig = (TH2 *) hist->Clone(csig);
	hRand = (TH2 *) hist->Clone(crand);
	hDiff = (TH2 *) hist->Clone(cdiff);
	hSig->SetZTitle("Events");
	hRand->SetZTitle("Events");
	hDiff->SetZTitle("Events");
	if (tBegin < tEnd) {
		sprintf(str, "unixTime > %d && unixTime < %d", tBegin, tEnd);
		ct = cut && str;
		tSig->Project(csig, what, ct);
		tRand->Project(crand, what, ct);
	} else {
		tSig->Project(csig, what, cut.GetTitle());
		tRand->Project(crand, what, cut.GetTitle());
	}
	
	hSig->Sumw2();
	hRand->Sumw2();
	MakeNonZeroErrors(hSig);
	MakeNonZeroErrors(hRand);
	hRand->Scale(1.0/NRANDOM);
	
	hDiff->Add(hSig, hRand, 1.0, -1.0);

	hist->Reset();
	hist->Add(hDiff, 1000.0 / upTime);	// mHz

	if (fRes) {
		fRes->cd();
		hSig->Write();
		hRand->Write();
		hDiff->Write();
	}
	
	delete hSig;
	delete hRand;
	delete hDiff;
} */

void HPainter2::SetUpTime(unsigned int t0, unsigned int t1)
{
	int i, N, S;
	TH1S *h;
	tBegin = t0;
	tEnd = t1;
	N = (t1 - t0) / 100;
	h = (TH1S*) gROOT->FindObject("_tmp_utime");
	if (h) delete h;
	h = new TH1S("_tmp_utime", "unix time", N, tBegin, tBegin + 100*N);
	tSig->Project("_tmp_utime", "unixTime");
	S = 0;
	for (i=0; i<N; i++) if (h->GetBinContent(i+1)) S++;
	upTime = S*100;
}
