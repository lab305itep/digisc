/****************************************************************
 * Draw phe histograms and get median versus run number plot	*
 ****************************************************************/
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <TChain.h>
#include <TCut.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH1D.h>

#define VDIR "/home/clusters/rrcmpi/alekseev/igor/dvert"
#define MAXRUN 150000
#define MAXADC 60
#define MAXCHAN 64

double Median(TH1 *h, double *err = NULL);

class MyDead {
	private:
		int DeadList[MAXADC][MAXCHAN];
	public:
		MyDead(const char *file_list);
		inline ~MyDead(void) {;};
		inline int IsDead(int adc, int chan) {
			if (adc < 0 || adc >= MAXADC) {
				printf("Bad adc=%d\n", adc);
				return 1;
			}
			if (chan < 0 || chan >= MAXCHAN) {
				printf("Bad chan=%d\n", chan);
				return 1;
			}
			return DeadList[adc][chan];
		};
};

MyDead::MyDead(const char *file_list)
{
	int chan, adc, irc;
	const int DeadADC[] = {4, 13, 20, 24, 25, 43, 47, 51, 52};	// too complex history to analyze hete
	
	memset(DeadList, 0, sizeof(DeadList));

	FILE *f = fopen(file_list, "rt");
	if (!f) printf("File %s is not opened. Empty dead list is used.\n");
	for(;!feof(f);) {
		irc = fscanf(f, "%d.%d", &adc, &chan);
		if (irc == 2) {
			DeadList[adc][chan] = 1;
		}
	}
	fclose(f);
	
	for(adc=0; adc < sizeof(DeadADC)/sizeof(int); adc++) for (chan=0; chan<MAXCHAN; chan++) 
		DeadList[DeadADC[adc]][chan] = 1;
}

class MyRunList {
	private:
		double *RunList;
		double date2year(int year, int month, int day);
	public:
		MyRunList(const char *stat_all);
		~MyRunList(void);
		inline double GetRunYear(int run) {		// time in years since 01.01.2016
			if (run < 0 || run >= MAXRUN) {
				printf("Out of range run number %d.\n", run);
				return -1000;
			}
			return RunList[run];
		}
};

MyRunList::MyRunList(const char *stat_all)
{
	FILE *f;
	int i, type, year, month, day, cnt;
	char str[1024];
	char *ptr;
	
	RunList = (double *) malloc(MAXRUN * sizeof(double));
	if (!RunList) {
		printf("No Memory for RunList!\n");
		throw;
	}
	
	f = fopen(stat_all, "rt");
	if (!f) {
		printf("File %s not opened. Dummy run list created.\n");
		for (i=0; i<MAXRUN; i++) RunList[i] = i/20000.0;
	} else {
		cnt = 0;
		for (i=0; i<MAXRUN; i++) RunList[i] = -1000;	// unknown files are bad
		for (;!feof(f);) {
			ptr = fgets(str, sizeof(str), f);
			if (!ptr) break;
			ptr = strtok(str, " \t");
			if (!ptr) continue;
			i = strtol(ptr, NULL, 10);
			if (i <= 0 || i >= MAXRUN) continue;
			ptr = strtok(NULL, " \t");
			if (!ptr) continue;
			type = strtol(ptr, NULL, 10);
			if (type <= 0) continue;
			ptr = strtok(NULL, " -\t");
			if (!ptr) continue;
			year = strtol(ptr, NULL, 10);
			if (year < 2016 || year > 2030) continue;
			ptr = strtok(NULL, " -\t");
			if (!ptr) continue;
			month = strtol(ptr, NULL, 10);
			if (month < 1 || month > 12) continue;
			ptr = strtok(NULL, " -\t");
			if (!ptr) continue;
			day = strtol(ptr, NULL, 10);
			if (day < 1 || day > 31) continue;
			RunList[i] = date2year(year, month, day);
			cnt++;
		}
		fclose(f);
		printf("%d good runs found\n", cnt);
	}
}

MyRunList::~MyRunList(void)
{
	if (RunList) free(RunList);
}

// approximate calculation
double MyRunList::date2year(int year, int month, int day)
{
	return ((year - 2016.0) + (month - 1) / 12.0 + (day - 1) / 365.0);
}

class MyStrip {
	private:
		int adc;
		int chan;
		int run;
		int gcnt;
		TH1D *hMedian;
		TGraphErrors *gMedian;
		TGraph *gCoef;
		TH1D *hPhe;
		TH1D *hCoef;
	public:
		MyStrip(int a, int c, int run_begin, int run_end, int run_step);
		~MyStrip(void);
		void StepBegin(int r);
		void StepEnd(double year);
		void AddHit(double phe);
		void AddHist(TH1D *h);
};

MyStrip::MyStrip(int a, int c, int run_begin, int run_end, int run_step)
{
	char strs[128];
	char strl[1024];
	
	adc = a;
	chan = c;
	sprintf(strs, "hMedian_%d_%2.2d", adc, chan);
	sprintf(strl, "Median p.h.e. plot for %d.%2.2d;run;p.h.e.", adc, chan);
	hMedian = new TH1D(strs, strl, (run_end - run_begin + 1) / run_step, run_begin, run_end);
	gMedian = new TGraphErrors();
	sprintf(strs, "gMedian_%d_%2.2d", adc, chan);
	gMedian->SetName(strs);
	sprintf(strl, "Median p.h.e. graph for %d.%2.2d;year;p.h.e.", adc, chan);
	gMedian->SetTitle(strl);
	gcnt = 0;
	gCoef = new TGraph();
	sprintf(strs, "gCoef_%d_%2.2d", adc, chan);
	gCoef->SetName(strs);
	sprintf(strl, "Coefficient: integral/p.h.e. graph for %d.%2.2d;year;S/p.h.e.", adc, chan);
	gCoef->SetTitle(strl);
	hPhe = NULL;
	hCoef = NULL;
}

MyStrip::~MyStrip(void)
{
	hMedian->Write();
	gMedian->Write();
	gCoef->Write();
	delete hMedian;
	delete gMedian;
	delete gCoef;
}

void MyStrip::StepBegin(int r)
{
	char strs[128];
	char strl[1024];
	
	run = r;
	sprintf(strs, "hPhe_%6.6d_%d_%2.2d", run, adc, chan);
	sprintf(strl, "Runs atarting at %d, strip=%d.%2.2d;p.h.e.", run, adc, chan);
	hPhe = new TH1D(strs, strl, 200, 0, 200);
	sprintf(strs, "hCoef_%6.6d_%d_%2.2d", run, adc, chan);
	sprintf(strl, "Coefficient: integral/p.h.e. hist for runs at %d for %d.%2.2d;year;S/p.h.e.", run, adc, chan);
	hCoef = new TH1D(strs, strl, 500, 40, 140);
}

void MyStrip::StepEnd(double year)
{
	double median, error;
	int j;
	
	if (hPhe->GetEntries() > 10) {
		median = Median(hPhe, &error);
		j = hMedian->FindBin(run);
		hMedian->SetBinContent(j, median);
		hMedian->SetBinError(j, error);
		gMedian->AddPoint(year, median);
		gMedian->SetPointError(gcnt, 0, error);
		gCoef->AddPoint(year, hCoef->GetMean());
		gcnt++;
	}
	hPhe->Write();
	hCoef->Write();
	delete hCoef;
	delete hPhe;
}

void MyStrip::AddHit(double phe)
{
	hPhe->Fill(phe);
}

void MyStrip::AddHist(TH1D *h)
{
	hCoef->Add(h);
}


/*
 * Caculate histogram median in the range [firstbin, lastbin]
 * The default includes underflow and overflow
 * Error = binwidth * sqrt(N) / 2 / binheight
 * Histogram could be scaled
 * Unit weights (just events) assumed during the fill
 */
double Median(TH1 *h, double *err)
{
	int firstbin, lastbin;
	double half;
	double sum;
	double val;
	double x;
	int i;
	
	firstbin = 0;			// underflow
	lastbin = h->GetNbinsX() + 1;	// overflow
	half = 0.5 * h->Integral(firstbin, lastbin);
	sum = 0;
	for (i = firstbin; i <= lastbin; i++) {
		val = h->GetBinContent(i);
		if (val <= 0) continue;
		if (sum + val < half) {
			sum += val;
		} else {
			x = h->GetXaxis()->GetBinLowEdge(i) + h->GetXaxis()->GetBinWidth(i) * (half - sum) / val;
			break;
		}
	}
	if (err) *err = h->GetXaxis()->GetBinWidth(i) * half / val / sqrt(h->GetEntries());
	return x;
}

int main(int argc, char **argv)
{
	int run_begin, run_end, run_step;
	int i, j, k, m, irc;
	long cnt, Num;
	char strs[128], strl[1024];
	TH1D *h;
	TFile *f;
	TCut ct;
	double median, error, year;
	struct HitOutStruct {
		float E;
		float phe;
		float pix;
		float signal;
		float dist;
		int adc;
		int chan;
		int ovf;
	} MyHit;
	MyStrip *Strip[MAXADC][MAXCHAN];
	
	memset(Strip, 0, sizeof(Strip));
	if (argc < 5) {
		printf("Usage: %s fname run_begin run_end run_step\n", argv[0]);
		return 10;
	}
	
	MyDead Dead("all_dead_list.txt");
	MyRunList Runs("../stat_all.txt");
	run_begin = strtol(argv[2], NULL, 10);
	run_end = strtol(argv[3], NULL, 10);
	run_step = strtol(argv[4], NULL, 10);
	
	TFile *fOut = new TFile(argv[1], "RECREATE");
	if (!fOut->IsOpen()) return 20;

	TChain *ch = new TChain("Hit", "Hit");
	for (i = run_begin; i <= run_end; i += run_step) {
		for (j=0; j<MAXADC; j++) for (k=0; k<MAXCHAN; k++) if (Strip[j][k]) Strip[j][k]->StepBegin(i);
		ch->Reset();
		ch->SetBranchAddress("Data", &MyHit);
		for (j=0; j<run_step; j++) {
			if (Runs.GetRunYear(i+j) < -100) continue;
			sprintf(strl, "%s/%3.3dxxx/vert_%6.6d.root", VDIR, (i+j) / 1000, i+j);
			irc = access(strl, R_OK);
			if (!irc) {
				f = new TFile(strl);
				for (m=0; m<MAXADC; m++) for (k=0; k<MAXCHAN; k++) {
					if (Dead.IsDead(m, k)) continue;
					sprintf(strs, "hCoef_%d_%2.2d", m, k);
					h = (TH1D *) f->Get(strs);
					if (!h) continue;
					if (!Strip[m][k]) {
						fOut->cd();
						Strip[m][k] = new MyStrip(m, k, run_begin, run_end, run_step);
						Strip[m][k]->StepBegin(i);
					}
					Strip[m][k]->AddHist(h);
				}
				delete f;
				ch->AddFile(strl);
				year = Runs.GetRunYear(i+j);
			}
		}
		Num = ch->GetEntries();
		for (cnt = 0; cnt < Num; cnt++) {
			ch->GetEntry(cnt);
			if (Dead.IsDead(MyHit.adc, MyHit.chan)) continue;
			if (!Strip[MyHit.adc][MyHit.chan]) {
				Strip[MyHit.adc][MyHit.chan] = new MyStrip(MyHit.adc, MyHit.chan, run_begin, run_end, run_step);
				Strip[MyHit.adc][MyHit.chan]->StepBegin(i);
			}
			Strip[MyHit.adc][MyHit.chan]->AddHit(MyHit.phe);
		}
		fOut->cd();
		for (j=0; j<MAXADC; j++) for (k=0; k<MAXCHAN; k++) if (Strip[j][k]) Strip[j][k]->StepEnd(year);
	}

	for (j=0; j<MAXADC; j++) for (k=0; k<MAXCHAN; k++) if (Strip[j][k]) delete Strip[j][k];
	fOut->Close();
	
	return 0;
}
