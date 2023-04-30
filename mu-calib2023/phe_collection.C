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
	Compare muon energy deposit for MC and experiment
*/

#define MAXWFD		60
#define MAXCHAN		64
#define MINEVENTS	500
#define MAXCHI2		5000.0
#define MINMEDIAN	13.0
#define MINMDN		28.0
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
	Fill per channel experimental histogramms from Ira's files
	fname - filename.root
*/
void make_ind_hists(const char *fname)
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
	for (i=53500; i<53650; i++) {
		sprintf(strl, "/home/clusters/rrcmpi/alekseev/igor/ROOT/Rdata_ovfl/danss_data_%6.6d_Ttree.root", i);
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
	
	gStyle->SetOptStat(1110);
	TH1D *hSiPM = new TH1D("hSiPMmedian", "SiPM vertical muons median distribution;ph.c.;Channels", 50, 0, 50);
	TH1D *hPMT = new TH1D("hPMTmedian", "PMT vertical muons median distribution;ph.c.;Channels", 50, 150, 450);
	hSiPM->SetLineWidth(2);
	hPMT->SetLineWidth(2);
	
	FILE *fIn = fopen(fname, "rb");
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
	cv->cd(2);
	hPMT->Draw();
	
	TString pngname(fname);
	pngname.ReplaceAll(".bin", "_median_distr.png");
	cv->SaveAs(pngname.Data());
}
