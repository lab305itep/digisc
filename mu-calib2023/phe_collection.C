#include <TCanvas.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TMath.h>
#include <TRandom2.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TTree.h>
/*
	Make muon calibration
*/

#define MAXWFD		60
#define MAXCHAN		64
#define MINEVENTS	500
#define MAXCHI2		5000.0
#define MINMEDIAN	13.0		// consider channel as dead and produce no MC calibration for it
#define MINMDN		28.0		// don't use for average median and cumulutive hit distribution
#define UGLY_MC_SIPM_CORR	1.08

/* MC nominal values */
const double pheSiPM = 20.4 / UGLY_MC_SIPM_CORR;
const double phePMT = 15.2;
// 0 = 50, 9 = 51, 15 = 52 = <Avr> = 71.1, 27 = 53
const double PMTpheADC[64] = {
	77.2, 73.6, 73.5, 70.3, 65.1, 81.5, 70.8, 69.2, 74.3, 77.8, 
	82.8, 74.4, 62.7, 72.9, 73.7, 71.1, 62.3, 72.6, 64.0, 67.2,
	82.9, 66.1, 79.9, 67.9, 74.5, 76.0, 68.0, 58.4, 68.7, 71.5,
	68.4, 74.7, 64.3, 70.7, 71.8, 84.9, 62.8, 62.3, 59.7, 68.3,
	74.4, 65.1, 64.9, 66.1, 78.1, 73.1, 64.5, 78.6, 73.4, 80.0,
	77.2, 77.8, 71.1, 58.4,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0};
const double PMTintcorr = 0.905; // integral to positive part integral ratio

/************************************************************************************************************************
 *			Useful functions										*
 ************************************************************************************************************************/

/*
 *  Calculate average and RMS
 */
class AvrSum {
private:
	double sum;
	double sum2;
	int cnt;
public:
	inline AvrSum(void) {
		sum = sum2 = cnt = 0;
	};
	inline void Add(double val) {
		sum += val;
		sum2 += val*val;
		cnt++;
	};
	inline double Avr(void) {
		return (cnt > 0) ? sum / cnt : 0;
	};
	inline double RMS(void) {
		return (cnt > 0) ? sqrt(sum2 / cnt - Avr() * Avr()): 0;
	}
};

/*
 * Chi^2 difference of two histograms hA and hB with errors between bins first and last
 */
double hDiff(TH1D *hA, TH1D *hB, int first, int last)
{
	int i;
	double sum;
	double d, sA, sB;
	
	sum = 0;
	for(i = first; i <= last; i++) {
		d = hA->GetBinContent(i) - hB->GetBinContent(i);
		sA = hA->GetBinError(i);
		sB = hB->GetBinError(i);
		if (sA > 0 || sB > 0) sum += d * d / (sA * sA + sB * sB);
	}
	return sum;
}

/*
 *  Calculate histogram average rejecting portions below leftedge and above rightedge
 */
double Average(TH1 *h, double leftedge = 0.0, double rightedge = 1.0)
{
	int leftbin, rightbin;
	int i;
	double sum, sumx;
	double Evt;
	
	sum = 0;
	leftbin = rightbin = -1;
	Evt = h->Integral(0, h->GetNbinsX()+2);
	for (i=0; i<h->GetNbinsX()+2; i++) {
		if (sum >= leftedge * Evt && leftbin < 0) leftbin = i;
		sum += h->GetBinContent(i);
		if (sum >= rightedge * Evt && rightbin < 0) rightbin = i;
	}
	if (rightedge < leftedge) rightedge = leftedge;
	sum = sumx = 0;
	for (i=leftbin; i<rightbin; i++) {
		sum += h->GetBinContent(i);
		sumx += h->GetBinContent(i) * h->GetBinCenter(i);
//		printf("%d: %f %f\n", i, sum, sumx);
	}
	return sumx / sum;
}

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

/*
 *  DANSS geometry translation from Ira's cell index to WFD.chan and Sasha's row.column
 */ 
class DANSSGeom {
private:
	int wfd[2500];		// SiPM WFDs
	int chan[2500];		// SiPM WFD channels
	int pmtchan[50];	// PMT WFD channels
	int GetKey(const char *str, const char *key);
public:
	DANSSGeom(void);
	inline int SiPMWFD(int index)    { return wfd[index]; };
	inline int SiPMchan(int index)   { return chan[index]; };
	inline int PMTchan(int index)    { return pmtchan[index]; };
	inline int SiPMrow(int index)    { return (index < 1250) ? (index / 25) * 2 : ((index-1250) / 25) * 2 + 1; };
	inline int SiPMcolumn(int index) { return 24 - (index % 25); };
	inline int PMTrow(int index)     { return (index % 5); };
	inline int PMTcolumn(int index)  { return (index < 25) ? 4 - (index / 5) : 9 - (index / 5); };
};

/*
 * Look for the integer value of a key in the string
 */
int DANSSGeom::GetKey(const char *str, const char *key)
{
	char *ptr = (char *)strstr(str, key);
	if (!ptr) return -1;	// not found
	return strtol(ptr + strlen(key), NULL, 10);
}

/*
 * Initialize translation tables from Ira's config files.
 */
DANSSGeom::DANSSGeom(void)
{
	FILE *f;
	int index, mod, chn;
	int wfdtab[MAXWFD];
	char str[1024];
//	Clear all tables
	memset(wfdtab, 0, sizeof(wfdtab));
	memset(wfd, 0, sizeof(wfd));
	memset(chan, 0, sizeof(chan));
	memset(pmtchan, 0, sizeof(pmtchan));
//	Read WFD translation table
	f = fopen("geom/danssAdcBoardHwPars.txt", "rt");
	if (!f) {
		printf("!!! No WFD translation file geom/danssAdcBoardHwPars.txt\n");
		return;
	}
	for(;;) {
		if (!fgets(str, sizeof(str), f)) break;
		index = GetKey(str, "Index=");
		mod = GetKey(str, "Address=");
		if (index < 0 || mod < 0) continue;
		wfdtab[index] = mod;
	}
	fclose(f);
//	Read PMT channels translation
	f = fopen("geom/danssModuleHwPars.txt", "rt");
	if (!f) {
		printf("!!! No PMT translation file geom/danssModuleHwPars.txt\n");
		return;
	}
	for(;;) {
		if (!fgets(str, sizeof(str), f)) break;
		index = GetKey(str, "Index=");
		chn = GetKey(str, "AdcBoardCh=");
		if (index < 0 || chn < 0) continue;
		pmtchan[index] = chn;
	}
	fclose(f);
//	Read SiPM channels translation
	f = fopen("geom/danssCellHwPars.txt", "rt");
	if (!f) {
		printf("!!! No SiPM translation file geom/danssCellHwPars.txt\n");
		return;
	}
	for(;;) {
		if (!fgets(str, sizeof(str), f)) break;
		index = GetKey(str, "Index=");
		mod = GetKey(str, "AdcBoardNum=");
		chn = GetKey(str, "AdcBoardCh=");
		if (index < 0 || chn < 0 || mod < 0) continue;
		chan[index] = chn;
		wfd[index] = wfdtab[mod];
	}
	fclose(f);
}


/************************************************************************************************************************
 *			Analysis											*
 ************************************************************************************************************************/
/*
	Fill per channel experimental histogramms from Ira's files
	fname - filename.root
*/
void make_ind_hists(const char *fname, int run_first = 53500, int run_last = 53649)
{
	char strs[256], strl[1024];
	int i, j;
	int N, index, adc, chan;
	struct {
		double index;
		double value;
	} Signal;
	TH1D *hE[MAXWFD][MAXCHAN];
	
	//	Create file
	TFile *fOut = new TFile(fname, "RECREATE");
	if (!fOut->IsOpen()) return;
	
	//	Create histogramms
	for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) {
		sprintf(strs, "hE_%2.2dc%2.2d", i, j);
		sprintf(strl, "Hit energy %d.%2.2d;ph.c.;Events", i, j);
		hE[i][j] = new TH1D(strs, strl, 150, 0, (i==1) ? 600 : 150);	// different scale for PMT and SiPM
	}
	//	Make chain
	TChain *tExp = new TChain("DANSSSignal", "ExpSignal");
	for (i = run_first; i <= run_last; i++) {
		if ((i/1000) == 53) {
			sprintf(strl, "/home/clusters/rrcmpi/alekseev/igor/ROOT/Rdata_ovfl/danss_data_%6.6d_Ttree.root", i);
		} else {
			sprintf(strl, "/home/clusters/rrcmpi/danss/DANSS/ROOT/Rdata_ovfl/%3.3dxxx/danss_data_%6.6d_Ttree.root",
				i/1000, i);
		}
		tExp->AddFile(strl);
	}
	tExp->SetBranchAddress("DANSSSignalNpe", &Signal);
	//	Fill experimental hists
	N = tExp->GetEntries();
	for (i=0; i < N; i++) {
		tExp->GetEntry(i);
		index = Signal.index;
		adc = index / 100;
		chan = index % 100;
		if (adc >= MAXWFD || chan >= MAXCHAN) {
			printf("Strange index = %d\n", index);
			continue;
		}
		if (adc == 1) {	// PMT
			if (PMTpheADC[chan] == 0) {
				printf("Strange index = %d\n", index);
				continue;
			}
			if (Signal.value > 49999) continue;		// ignore overflow
			hE[adc][chan]->Fill(Signal.value * PMTintcorr / PMTpheADC[chan]);
		} else if (adc != 3) {	// SiPM - we do nothing with veto and don't expect it here
			hE[adc][chan]->Fill(Signal.value);
		}
	}
	//	Save histogramms
	fOut->cd();
	for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) if (hE[i][j]->GetEntries() > MINEVENTS) hE[i][j]->Write();
	fOut->Close();
}

/*
	Calculate median values from histogramm file
	fname - filename.root
	filename.bin will be created
*/
void make_median_file(const char *fname)
{
	int i, j;
	char strs[256];
	float median;
	TH1D *h;
	
	TFile *fIn = new TFile(fname);
	if (!fIn->IsOpen()) return;
	TString fstr(fname);
	fstr.ReplaceAll(".root", ".bin");
	FILE *fOut = fopen(fstr.Data(), "wb");

	for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) {
		median = 0;
		sprintf(strs, "hE_%2.2dc%2.2d", i, j);
		h = (TH1D *) fIn->Get(strs);
		if (h) median = Median(h);
		fwrite(&median, sizeof(float), 1, fOut);
	}

	fclose(fOut);
	fIn->Close();
}

/*
	Draw distribution of median values from the given file
	fname - filename.bin
*/
void draw_median_distrib(const char *fname)
{
	int i, j;
	float median;
	TLine ln;
	
	gStyle->SetOptStat(1110);
	TH1D *hSiPM = new TH1D("hSiPMmedian", "SiPM vertical muons median distribution;ph.c.;Channels", 50, 0, 50);
	TH1D *hPMT = new TH1D("hPMTmedian", "PMT vertical muons median distribution;ph.c.;Channels", 50, 150, 450);
	hSiPM->SetLineWidth(2);
	hPMT->SetLineWidth(2);
	
	FILE *fIn = fopen(fname, "rb");
	if (!fIn) {
		printf("Can not open file %s\n", fname);
		return;
	}
	for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) {
		fread(&median, sizeof(float), 1, fIn);
		if (median > 0) {
			if (i == 1) {
				hPMT->Fill(median);
			} else {
				hSiPM->Fill(median);
			}
		}
	}
	fclose(fIn);
	
	TCanvas *cv = new TCanvas("CV", "CV", 1200, 800);
	cv->Divide(2, 1);
	cv->cd(1);
	hSiPM->Draw();
	ln.SetLineWidth(2);
	ln.SetLineColor(kGreen);
	ln.DrawLine(MINMDN, 0, MINMDN, hSiPM->GetMaximum()/2);
	ln.SetLineColor(kRed);
	ln.DrawLine(MINMEDIAN, 0, MINMEDIAN, hSiPM->GetMaximum()/4);
	cv->cd(2);
	hPMT->Draw();
	
	TString pngname(fname);
	pngname.ReplaceAll(".bin", "_median_distr.png");
	cv->SaveAs(pngname.Data());
}

/*
	Draw cumulutive distribution of experimental hit energy deposit values with calibration
	fname - filename.bin
*/
void draw_exp_hist(const char *fname, int run_first = 53500, int run_last = 53649)
{
	int i, j, N;
	int adc, chan, index;
	char strl[1024];
	float median[MAXWFD][MAXCHAN];
	double avr_median_SiPM, avr_median_PMT;
	double median_SiPM, median_PMT;
	double emedian_SiPM, emedian_PMT;
	int nSiPM, nPMT;
	struct {
		double index;
		double value;
	} Signal;
	TLatex txt;
	TLine ln;

	// Read median file
	FILE *fIn = fopen(fname, "rb");
	if (!fIn) {
		printf("Can not open median file %s\n", fname);
		return;
	}
	i = fread(median, sizeof(median), 1, fIn);
	if (i != 1) {
		printf("Can not read median file %s\n", fname);
		return;
	}
	fclose(fIn);
	// Calculate average median - just average of all values above 
	avr_median_SiPM = avr_median_PMT = 0;
	nSiPM = nPMT = 0;
	for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) if (median[i][j] > 0) {
		if (i==1) {		// PMT
			avr_median_PMT += median[i][j];
			nPMT++;
		} else if (median[i][j] > MINMDN) {	// SiPM
			avr_median_SiPM += median[i][j];
			nSiPM++;
		}
	}
	avr_median_SiPM /= nSiPM;
	avr_median_PMT /= nPMT;
	printf("Get average medians: SiPM = %f    PMT = %f\n", avr_median_SiPM, avr_median_PMT);
	// Create histogramms
	TH1D *hESiPM = new TH1D("hESiPM", "SiPM hit energy distribution;ph.c.;Events", 150, 0, 150);	// different scale for PMT and SiPM
	TH1D *hEPMT = new TH1D("hEPMT", "PMT hit energy distribution;ph.c.;Events", 150, 0, 600);	// different scale for PMT and SiPM
	hESiPM->SetLineWidth(2);
	hEPMT->SetLineWidth(2);
	//	Make chain
	TChain *tExp = new TChain("DANSSSignal", "ExpSignal");
	for (i = run_first; i <= run_last; i++) {
		if ((i/1000) == 53) {
			sprintf(strl, "/home/clusters/rrcmpi/alekseev/igor/ROOT/Rdata_ovfl/danss_data_%6.6d_Ttree.root", i);
		} else {
			sprintf(strl, "/home/clusters/rrcmpi/danss/DANSS/ROOT/Rdata_ovfl/%3.3dxxx/danss_data_%6.6d_Ttree.root",
				i/1000, i);
		}
		tExp->AddFile(strl);
	}
	tExp->SetBranchAddress("DANSSSignalNpe", &Signal);
	//	Fill experimental hists
	N = tExp->GetEntries();
	for (i=0; i < N; i++) {
		tExp->GetEntry(i);
		index = Signal.index;
		adc = index / 100;
		chan = index % 100;
		if (adc >= MAXWFD || chan >= MAXCHAN) {
			printf("Strange index = %d\n", index);
			continue;
		}
		if (adc == 1) {	// PMT
			if (PMTpheADC[chan] == 0 || median[adc][chan] == 0) {
				printf("Strange index = %d\n", index);
				continue;
			}
			if (Signal.value > 49999) continue;		// ignore overflow
			hEPMT->Fill((Signal.value * PMTintcorr / PMTpheADC[chan]) * (avr_median_PMT / median[adc][chan]));
		} else if (adc != 3 && median[adc][chan] > MINMDN) {	// SiPM - we do nothing with veto and don't expect it here
			hESiPM->Fill(Signal.value * avr_median_SiPM / median[adc][chan]);
		}
	}
	median_SiPM = Median(hESiPM, &emedian_SiPM);
	median_PMT = Median(hEPMT, &emedian_PMT);
	//	Draw
	ln.SetLineWidth(2);
	ln.SetLineColor(kBlack);
	txt.SetTextSize(0.04);
	TCanvas *cv = new TCanvas("CV", "CV", 1200, 800);
	cv->Divide(2, 1);
	cv->cd(1);
	hESiPM->Draw();
	ln.DrawLine(median_SiPM, 0, median_SiPM, hESiPM->GetMaximum() / 2);
	sprintf(strl, "Median = %5.2f#pm%4.2f ph.c.", median_SiPM, emedian_SiPM);
	txt.DrawLatexNDC(0.1, 0.03, strl);
	cv->cd(2);
	hEPMT->Draw();
	ln.DrawLine(median_PMT, 0, median_PMT, hEPMT->GetMaximum() / 2);
	sprintf(strl, "Median = %6.2f#pm%4.2f ph.c.", median_PMT, emedian_PMT);
	txt.DrawLatexNDC(0.1, 0.03, strl);
	
	TString pngname(fname);
	pngname.ReplaceAll(".bin", "_hit_distr.png");
	cv->SaveAs(pngname.Data());
	
	TString rootname(fname);
	rootname.ReplaceAll(".bin", "_hist.root");
	TFile *fOut = new TFile(rootname, "RECREATE");
	hESiPM->Write();
	hEPMT->Write();
	fOut->Close();
}

/*
	Draw cumulutive distribution of MC hit energy deposit values with calibration
	fname - filename.bin
*/
void draw_MC_hist(const char *fname, int ver = 0)
{
	int i, j, N;
	int adc, chan, index;
	char strs[256], strl[1024];
	float median[MAXWFD][MAXCHAN];
	double avr_median_SiPM, avr_median_PMT;
	double median_SiPM, median_PMT;
	double emedian_SiPM, emedian_PMT;
	int nSiPM, nPMT;
	struct {
		double index;
		double value;
	} Signal;
	TLatex txt;
	TLine ln;

	// Read median file
	FILE *fIn = fopen(fname, "rb");
	if (!fIn) {
		printf("Can not open median file %s\n", fname);
		return;
	}
	i = fread(median, sizeof(median), 1, fIn);
	if (i != 1) {
		printf("Can not read median file %s\n", fname);
		return;
	}
	fclose(fIn);
	// Calculate average median - just average of all values above 
	avr_median_SiPM = avr_median_PMT = 0;
	nSiPM = nPMT = 0;
	for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) if (median[i][j] > 0) {
		if (i==1) {		// PMT
			avr_median_PMT += median[i][j];
			nPMT++;
		} else if (median[i][j] > MINMDN) {	// SiPM
			avr_median_SiPM += median[i][j];
			nSiPM++;
		}
	}
	avr_median_SiPM /= nSiPM;
	avr_median_PMT /= nPMT;
	//	correct dead channels which present in MC
	median[1][0] = median[1][50];
	median[1][9] = median[1][51];
	printf("Get average medians: SiPM = %f    PMT = %f\n", avr_median_SiPM, avr_median_PMT);
	// Create histogramms
	sprintf(strs, "hMCSiPMv%d", ver);
	sprintf(strl, "MC v%d SiPM hit energy distribution;ph.c.;Events", ver);
	TH1D *hMCSiPM = new TH1D(strs, strl, 150, 0, 150);	// different scale for PMT and SiPM
	sprintf(strs, "hMCPMTv%d", ver);
	sprintf(strl, "MC v%d PMT hit energy distribution;ph.c.;Events", ver);
	TH1D *hMCPMT = new TH1D(strs, strl, 150, 0, 600);	// different scale for PMT and SiPM
	hMCSiPM->SetLineWidth(2);
	hMCPMT->SetLineWidth(2);
	//	Make chain
	TChain *tMC = new TChain("DANSSSignal", "MCSignal");
	switch (ver) {
	case 5:
		for (i=0; i<32; i++) {
			sprintf(strl, 
"/home/clusters/rrcmpi/danss/DANSS/ROOT/MCMuons_testProfiles_v5/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d.root", 
				i/16, (i%16) + 1);
			tMC->AddFile(strl);
		}
		break;
	default:
		for (i=0; i<32; i++) {
			sprintf(strl, 
"/home/clusters/rrcmpi/danss/DANSS/ROOT/MC_WF_ovfl_period2_v2/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d.root", 
				i/16, (i%16) + 1);
			tMC->AddFile(strl);
		}
	}
	tMC->SetBranchAddress("DANSSSignalNpe", &Signal);
	//	Fill MC hists
	N = tMC->GetEntries();
	for (i=0; i < N; i++) {
		tMC->GetEntry(i);
		index = Signal.index;
		adc = index / 100;
		chan = index % 100;
		if (adc >= MAXWFD || chan >= MAXCHAN) {
			printf("Strange index = %d\n", index);
			continue;
		}
		if (adc == 1) {	// PMT
			if (PMTpheADC[chan] == 0 || median[adc][chan] == 0) {
				printf("Strange index = %d\n", index);
				continue;
			}
			if (Signal.value > 49999) continue;		// ignore overflow
			hMCPMT->Fill(Signal.value * (avr_median_PMT / median[adc][chan]));
		} else if (adc != 3 && median[adc][chan] > MINMDN) {	// SiPM - we do nothing with veto and don't expect it here
			hMCSiPM->Fill(Signal.value * UGLY_MC_SIPM_CORR * avr_median_SiPM / median[adc][chan]);
		}
	}
	median_SiPM = Median(hMCSiPM, &emedian_SiPM);
	median_PMT = Median(hMCPMT, &emedian_PMT);
	//	Draw
	ln.SetLineWidth(2);
	ln.SetLineColor(kBlack);
	txt.SetTextSize(0.04);
	TCanvas *cv = new TCanvas("CV", "CV", 1200, 800);
	cv->Divide(2, 1);
	cv->cd(1);
	hMCSiPM->Draw();
	ln.DrawLine(median_SiPM, 0, median_SiPM, hMCSiPM->GetMaximum() / 2);
	sprintf(strl, "Median = %5.2f#pm%4.2f ph.c.", median_SiPM, emedian_SiPM);
	txt.DrawLatexNDC(0.1, 0.03, strl);
	cv->cd(2);
	hMCPMT->Draw();
	ln.DrawLine(median_PMT, 0, median_PMT, hMCPMT->GetMaximum() / 2);
	sprintf(strl, "Median = %6.2f#pm%4.2f ph.c.", median_PMT, emedian_PMT);
	txt.DrawLatexNDC(0.1, 0.03, strl);
	
	TString pngname(fname);
	sprintf(strl, "_MC_v%d_distr.png", ver);
	pngname.ReplaceAll(".bin", strl);
	cv->SaveAs(pngname.Data());

	TString rootname(fname);
	rootname.ReplaceAll(".bin", "_hist.root");
	TFile *fOut = new TFile(rootname, "UPDATE");
	hMCSiPM->Write();
	hMCPMT->Write();
	fOut->Close();
}

/*
	Compare MC and experimental cumulutive distributions
	fname - filename_hist.root
*/
void draw_mc2exp(const char *fname)
{
	double median_MCSiPM, median_MCPMT;
	double emedian_MCSiPM, emedian_MCPMT;
	double median_ESiPM, median_EPMT;
	double emedian_ESiPM, emedian_EPMT;
	char str[256];

	gStyle->SetOptFit(0);
	gStyle->SetOptStat(0);
	
	TFile *fIn = new TFile(fname);
	if (!fIn->IsOpen()) return;
	TH1D *hESiPM = (TH1D *) fIn->Get("hESiPM");
	TH1D *hEPMT = (TH1D *) fIn->Get("hEPMT");
	TH1D *hMCSiPM = (TH1D *) fIn->Get("hMCSiPM");
	TH1D *hMCPMT = (TH1D *) fIn->Get("hMCPMT");
	if (!hESiPM || !hEPMT || !hMCSiPM || !hMCPMT) {
		printf("File %s missing histograms\n", fname);
		return;
	}
	
	hESiPM->SetStats(0);
	hEPMT->SetStats(0);
	hMCSiPM->SetStats(0);
	hMCPMT->SetStats(0);
	hESiPM->SetMarkerStyle(kFullCircle);
	hEPMT->SetMarkerStyle(kFullCircle);
	hESiPM->SetMarkerSize(0.75);
	hEPMT->SetMarkerSize(0.75);
	hESiPM->SetMarkerColor(kRed);
	hEPMT->SetMarkerColor(kRed);
	hESiPM->SetLineColor(kRed);
	hEPMT->SetLineColor(kRed);
	hMCSiPM->SetLineWidth(2);
	hMCPMT->SetLineWidth(2);
	hMCSiPM->SetLineColor(kBlack);
	hMCPMT->SetLineColor(kBlack);
	hMCSiPM->Scale(hESiPM->GetBinContent(hESiPM->GetMaximumBin()) / hMCSiPM->GetBinContent(hMCSiPM->GetMaximumBin()));
	hMCPMT->Scale(hEPMT->GetBinContent(hEPMT->GetMaximumBin()) / hMCPMT->GetBinContent(hMCPMT->GetMaximumBin()));

	median_ESiPM = Median(hESiPM, &emedian_ESiPM);
	median_EPMT = Median(hEPMT, &emedian_EPMT);
	median_MCSiPM = Median(hMCSiPM, &emedian_MCSiPM);
	median_MCPMT = Median(hMCPMT, &emedian_MCPMT);
	
	TCanvas *cv = new TCanvas("CV", "CV", 1200, 800);
	cv->Divide(2, 1);
	cv->cd(1);
	hESiPM->Draw("ep");
	hMCSiPM->Draw("hist,same");
	TLegend *lgSiPM = new TLegend(0.5, 0.75, 0.89, 0.89);
	sprintf(str, "Exp: %5.2f#pm%4.2f", median_ESiPM, emedian_ESiPM);
	lgSiPM->AddEntry(hESiPM, str, "pe");
	sprintf(str, "MC:  %5.2f#pm%4.2f", median_MCSiPM, emedian_MCSiPM);
	lgSiPM->AddEntry(hMCSiPM, str, "l");
	lgSiPM->Draw();
	cv->cd(2);
	hEPMT->Draw("e");
	hMCPMT->Draw("hist,same");
	TLegend *lgPMT = new TLegend(0.6, 0.75, 0.99, 0.89);
	sprintf(str, "Exp: %5.2f#pm%4.2f", median_EPMT, emedian_EPMT);
	lgPMT->AddEntry(hEPMT, str, "pe");
	sprintf(str, "MC:  %5.2f#pm%4.2f", median_MCPMT, emedian_MCPMT);
	lgPMT->AddEntry(hMCPMT, str, "l");
	lgPMT->Draw();
}

/*
 * Calculate per channel calibration in ph.c./MeV in format sutable for MC
 * fname - Calibration filename.bin
 * pheSiPM0, phePMT - new values for MC ph.c./MeV
 * 01.05.2023: 20.94 and 15.61
 * Median valuse are calculated from the calibration file.
 * The result is written in two files (for SiPM and PMT)
 * Old constants: MC nominal values 
 *	const double pheSiPM = 20.4 / UGLY_MC_SIPM_CORR;
 *	const double phePMT = 15.2;
 *	MedMCSiPM = 35.63  MedMCPMT = 282.0
 *	MedSiPM = 35.58 and MedPMT = 281.73 - from the experiment analysis - see draw_exp_hist()
 */
void calib4sasha(const char *fname, double pheSiPM0, double phePMT)
{
	int i, j;
	float median[MAXWFD][MAXCHAN];
	double avr_median_SiPM, avr_median_PMT;
	int nSiPM, nPMT;
	struct {
		double cellindex;
		double row;
		double column;
		double energy;
	} data;
	int mod, chan, column, row;
	TFile *fOut;
	DANSSGeom geom;
	double pheSiPM = pheSiPM0 / UGLY_MC_SIPM_CORR;

	// Read median file
	FILE *fIn = fopen(fname, "rb");
	if (!fIn) {
		printf("Can not open median file %s\n", fname);
		return;
	}
	i = fread(median, sizeof(median), 1, fIn);
	if (i != 1) {
		printf("Can not read median file %s\n", fname);
		return;
	}
	fclose(fIn);
	// Calculate average median - just average of all values above 
	avr_median_SiPM = avr_median_PMT = 0;
	nSiPM = nPMT = 0;
	for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) if (median[i][j] > 0) {
		if (i==1) {		// PMT
			avr_median_PMT += median[i][j];
			nPMT++;
		} else if (median[i][j] > MINMDN) {	// SiPM
			avr_median_SiPM += median[i][j];
			nSiPM++;
		}
	}
	avr_median_SiPM /= nSiPM;
	avr_median_PMT /= nPMT;
	printf("Get average medians: SiPM = %f    PMT = %f\n", avr_median_SiPM, avr_median_PMT);
//		Cycle over SiPM
	fOut = new TFile("SiPM_response_data.root", "RECREATE");
	TTree *tSiPM = new TTree("EnergyTree", "Tree with coefficients");
	tSiPM->Branch("EnergyBranch", &data, "cellindex/D:row:column:energy");
	
	for (i=0; i<2500; i++) {
		column = geom.SiPMcolumn(i);
		row    = geom.SiPMrow(i);
		mod    = geom.SiPMWFD(i);
		chan   = geom.SiPMchan(i);
		data.energy = (median[mod][chan] > MINMEDIAN) ? median[mod][chan] * pheSiPM / avr_median_SiPM : pheSiPM;	// nominal for dead channels
		data.cellindex = i;
		data.row = row;
		data.column = column;
		tSiPM->Fill();
	}
	tSiPM->Write();
	fOut->Close();

//		Cycle over PMT
	fOut = new TFile("PMT_response_data.root", "RECREATE");
	TTree *tPMTeven = new TTree("EnergyTreeEven", "Even tree with coefficients");
	tPMTeven->Branch("EnergyBranchEven", &data, "modindex/D:row:column:energy");
	TTree *tPMTodd = new TTree("EnergyTreeOdd", "Odd tree with coefficients");
	tPMTodd->Branch("EnergyBranchOdd", &data, "modindex/D:row:column:energy");
	
	for (i=0; i<50; i++) {
		column = geom.PMTcolumn(i);
		row    = geom.PMTrow(i);
		mod    = 1;
		chan   = geom.PMTchan(i);
		data.energy = median[mod][chan] * phePMT / avr_median_PMT;
		data.cellindex = i;
		data.row = row;
		data.column = column;
		if (i < 25) {
			tPMTodd->Fill();
		} else {
			tPMTeven->Fill();
		}
	}
	tPMTeven->Write();
	tPMTodd->Write();
	fOut->Close();
}
