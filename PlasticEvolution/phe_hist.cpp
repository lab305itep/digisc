/****************************************************************
 * Draw phe histograms and get median versus run number plot	*
 ****************************************************************/
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <TTree.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TH1D.h>

/****************************************************************/

#define VDIR "/home/clusters/rrcmpi/alekseev/igor/dvert"
#define AVR1PXFILE "avr1px.bin"
#define MAXRUN 200000
#define MAXADC 60
#define MAXCHAN 64

struct HitOutStruct {
	float E;
	float phe;
	float pix;
	float signal;
	float dist;
	int adc;
	int chan;
	int xy;
	int z;
	int ovf;
};

/****************************************************************/

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
	const int DeadADC[] = {4, 13, 20, 24, 25, 43, 47, 51, 52};	// too complex history to analyze here

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

/****************************************************************/

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

	for (i=0; i < MAXRUN; i++) RunList[i] = -1;
	
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

/****************************************************************/

double f1pxAvr[MAXADC][MAXCHAN];
double f1px[MAXADC][MAXCHAN];
double f1pxMid[MAXADC][MAXCHAN];
int f1pxCnt[MAXADC][MAXCHAN];


void InitCoef(const char *fname)
{
	FILE *fIn;
	int irc;

	fIn = fopen(fname, "rb");
	if (!fIn) {
		printf("Can not open file %s - %m\n", fname);
		throw;
	}
	irc = fread(f1pxAvr, sizeof(f1pxAvr), 1, fIn);
	if (irc != 1) {
		printf("Can not read file %s - %m\n", fname);
		throw;
	}
	fclose(fIn);
}

int ReadCoef(TFile *fIn)
{
	TTree *tCalib;
	struct chanCalibStruct {
		int adc;
		int chan;
		float fConv;
		float f1px;
		float fXtalk;
	} chanCalib;
	int j, N;

	memset(f1px, 0, sizeof(f1px));
	tCalib = (TTree *) fIn->Get("Calib");
	if (!tCalib) {
		printf("Tree Calib not found in %s\n", fIn->GetName());
		return -10;
	}
	tCalib->SetBranchAddress("SiPM", &chanCalib);
	N = tCalib->GetEntries();
	for (j=0; j<N; j++) {
		tCalib->GetEntry(j);
		if (chanCalib.f1px <= 0) continue;
		f1px[chanCalib.adc][chanCalib.chan] = chanCalib.f1px;
		if (f1pxAvr[chanCalib.adc][chanCalib.chan]) {
			f1pxMid[chanCalib.adc][chanCalib.chan] += (chanCalib.f1px - f1pxAvr[chanCalib.adc][chanCalib.chan]) / f1pxAvr[chanCalib.adc][chanCalib.chan];
			f1pxCnt[chanCalib.adc][chanCalib.chan]++;
		}
	}
	return 0;
}

/****************************************************************/

struct MyRange {
	int min;
	int max;
};

class MyCuts {
	private:
		struct MyRange adc;
		struct MyRange chan;
		struct MyRange xy;
		struct MyRange z;
		struct MyRange dist;
		struct MyRange side;
		int GetSide(int adc);
	public:
		MyCuts(char *strcut);
		int Check(struct HitOutStruct *Hit);
};

MyCuts::MyCuts(char *strcut)
{
	const char *param[6] = {"adc=[", "chan=[", "xy=[", "z=[", "dist=[", "side=["};
	char *ptr;
	int i, min, max;
	
	adc.min = 0;
	adc.max = 100;
	chan.min = 0;
	chan.max = 63;
	xy.min = 0;
	xy.max = 24;
	z.min = 0;
	z.max = 49;
	dist.min = 0;
	dist.max = 100;
	side.min = 0;
	side.max = 1;
	
	if (strcut) for (i=0; i<6; i++) {
		ptr = strstr(strcut, param[i]);
		if (!ptr) continue;
		ptr += strlen(param[i]);
		min = strtol(ptr, &ptr, 10);
		ptr++;
		if (ptr >= strcut + strlen(strcut)) continue;
		max = strtol(ptr, NULL, 10);
		switch(i) {
		case 0:
			adc.min = min;
			adc.max = max;
			break;
		case 1:
			chan.min = min;
			chan.max = max;
			break;
		case 2:
			xy.min = min;
			xy.max = max;
			break;
		case 3:
			z.min = min;
			z.max = max;
			break;
		case 4:
			dist.min = min;
			dist.max = max;
			break;
		case 5:
			side.min = min;
			side.max = max;
			break;
		}
	}
	printf("Cuts: adc=[%d:%d] chan=[%d:%d] xy=[%d:%d] z=[%d:%d] dist=[%d:%d] side=[%d:%d]\n",
		adc.min, adc.max, chan.min, chan.max, xy.min, xy.max, z.min, z.max, 
		dist.min, dist.max, side.min, side.max);
}

int MyCuts::GetSide(int adc)
{
	const int Side[MAXADC] = {
//		 0   1   2   3   4   5   6   7   8   9
		-1, -1,  1, -1,  0,  1,  1,  1,  1,  1,		// 0X
		 1,  1,  1,  1,  0,  0,  1,  1,  1,  1,		// 1X
		 1,  1,  1,  1,  1,  1,  0,  0,  0,  0,		// 2X
		 0,  0,  0,  0,  0,  1,  1,  0,  0,  0,		// 3X
		 0,  0,  0,  0,  0,  0,  0,  0, -1, -1,		// 4X
		-1,  1,  0, -1, -1, -1, -1, -1, -1, -1		// 5X
	};
	if (adc < 0 || adc >= MAXADC) return -1;
	return Side[adc];
}

int MyCuts::Check(struct HitOutStruct *Hit)
{
	if (Hit->adc  < adc.min  || Hit->adc  > adc.max)  return 0;
	if (Hit->chan < chan.min || Hit->chan > chan.max) return 0;
	if (Hit->xy   < xy.min   || Hit->xy   > xy.max)   return 0;
	if (Hit->z    < z.min    || Hit->z    > z.max)    return 0;
	if (Hit->dist < dist.min || Hit->dist > dist.max) return 0;
	int s = GetSide(Hit->adc);
	if (s < side.min || s > side.max) return 0;
//	printf("adc=%d chan=%d xy=%d z=%d dist=%f\n", 
//		Hit->adc, Hit->chan, Hit->xy, Hit->z, Hit->dist);
	return 1;
}

/****************************************************************/
/*
 * Caculate histogram median in the range [firstbin, lastbin]
 * The default includes underflow and overflow
 * Error = binwidth * sqrt(N) / 2 / binheight
 * Histogram could be scaled
 * Unit weights (just events) assumed during the fill
 */
double Median(TH1 *h, double *err = NULL)
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
	double alpha;
	double r;
	int i, j, irc, mcnt;
	long cnt, lcnt, Num;
	char strs[128], strl[1024];
	TH1D *h;
	TFile *fIn;
	TTree *tHit;
	double median, error, year;
	struct HitOutStruct MyHit;
	int adc, chan;
	double r_sum, r_sum2, r_cnt;
	
	if (argc < 6) {
		printf("Usage: %s fname run_begin run_end run_step alpha [\"cuts\"]\n", argv[0]);
		printf("Cuts are given in the form parameter=[min:max] separated with comma.\n");
		printf("The default is no cut on the parameter.\n");
		printf("Possible parameters: adc, chan, xy, z, dist, side\n");
		printf("side == 0: X, 1: Y\n");
		return 10;
	}
	
	MyDead Dead("all_dead_list.txt");
	MyRunList Runs("../stat_all.txt");
	InitCoef(AVR1PXFILE);
	run_begin = strtol(argv[2], NULL, 10);
	run_end = strtol(argv[3], NULL, 10);
	run_step = strtol(argv[4], NULL, 10);
	alpha = strtod(argv[5], NULL);
	MyCuts Cuts((argc > 6) ? argv[6] : NULL);

	TFile *fOut = new TFile(argv[1], "RECREATE");
	if (!fOut->IsOpen()) return 20;

	TH1D *hMedian = new TH1D("hMedian", "Median p.h.e. plot;run;p.h.e.", (run_end - run_begin + 1) / run_step, run_begin, run_end);
	TH1D *hR = new TH1D("hR", "Average relative coef;run;r", (run_end - run_begin + 1) / run_step, run_begin, run_end);
	TGraphErrors *gMedian = new TGraphErrors();
	gMedian->SetName("gMedian");
	gMedian->SetTitle("Median p.h.e. graph;year;p.h.e.");
	TGraphErrors *gR = new TGraphErrors();
	gR->SetName("gR");
	gR->SetTitle("Average relative coef;year;r");
	mcnt = 0;
	for (i = run_begin; i <= run_end; i += run_step) {
		sprintf(strs, "hPhe_%6.6d", i);
		sprintf(strl, "Runs %d - %d;p.h.e.", i, i+run_step-1);
		h = new TH1D(strs, strl, 200, 0, 200);
		lcnt = 0;
		memset(f1pxMid, 0, sizeof(f1pxMid));
		memset(f1pxCnt, 0, sizeof(f1pxCnt));
		for (j=0; j<run_step; j++) {
			if (Runs.GetRunYear(i+j) < -100) continue;
			sprintf(strl, "%s/%3.3dxxx/vert_%6.6d.root", VDIR, (i+j) / 1000, i+j);
			irc = access(strl, R_OK);
			if (irc) continue;
			fIn = new TFile(strl);
			if (!fIn->IsOpen()) {
				delete fIn;
				continue;
			}
			tHit = (TTree *) fIn->Get("Hit");
			if (!tHit) {
				delete fIn;
				continue;
			}
			if (ReadCoef(fIn)) {
				delete fIn;
				continue;
			}
			tHit->SetBranchAddress("Data", &MyHit);
			year = Runs.GetRunYear(i+j);
			Num = tHit->GetEntries();
//			printf("file %s: %d entries\n", strl, Num);
			for (cnt = 0; cnt < Num; cnt++) {
				tHit->GetEntry(cnt);
				if (Dead.IsDead(MyHit.adc, MyHit.chan)) continue;
				if (!Cuts.Check(&MyHit)) continue;
				if (f1px[MyHit.adc][MyHit.chan] <= 0) continue;
				if (f1pxAvr[MyHit.adc][MyHit.chan] <= 0) continue;
				r = (f1px[MyHit.adc][MyHit.chan] - f1pxAvr[MyHit.adc][MyHit.chan]) / f1pxAvr[MyHit.adc][MyHit.chan];
				h->Fill(MyHit.phe*(1+alpha*r));
				lcnt++;
			}
			delete fIn;
		}
		if (lcnt > 100) {
			median = Median(h, &error);
			j = hMedian->FindBin(i);
			hMedian->SetBinContent(j, median);
			hMedian->SetBinError(j, error);
			gMedian->AddPoint(year, median);
			gMedian->SetPointError(mcnt, 0, error);
			r_sum = r_sum2 = r_cnt = 0;
			for (adc = 0; adc < MAXADC; adc++) for(chan = 0; chan < MAXCHAN; chan++) if (!Dead.IsDead(adc, chan) && f1pxCnt[adc][chan]) {
				r_sum += f1pxMid[adc][chan];
				r_sum2 += f1pxMid[adc][chan] * f1pxMid[adc][chan] / f1pxCnt[adc][chan];
				r_cnt += f1pxCnt[adc][chan];
			}
			r_sum /= r_cnt;
			r_sum2 /= r_cnt;
			r_sum2 = sqrt(r_sum2 - r_sum*r_sum);
			hR->SetBinContent(j, r_sum);
			hR->SetBinError(j, r_sum2);
			gR->AddPoint(year, r_sum);
			gR->SetPointError(mcnt, 0, r_sum2);
			mcnt++;
		}
		fOut->cd();
		h->Write();
		delete h;
	}

	gMedian->Write();
	hMedian->Write();
	gR->Write();
	hR->Write();
	fOut->Close();
	
	return 0;
}
