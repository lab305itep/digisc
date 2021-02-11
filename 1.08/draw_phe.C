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
#define MAXCHI2		500

class RandomSmear:public TRandom2 {
private:
	double eC;
	double cC;
public:
	inline RandomSmear(int seed, double eCoef, double cCoef) : TRandom2(seed) {
	        eC = eCoef * eCoef;
		cC = cCoef * cCoef;
	};
	inline double Smear(double val) {
		return Gaus(val, sqrt(val * eC + val * val * cC));
	};
};

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

double Median(TH1 *h, int firstbin, int lastbin)
{
	double half;
	double sum;
	double val;
	double x;
	int i;
	
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
	return x;
}

Double_t langaufun(Double_t *x, Double_t *par) {

//Fit parameters:
//par[0]=Width (scale) parameter of Landau density
//par[1]=Most Probable (MP, location) parameter of Landau density
//par[2]=Total area (integral -inf to inf, normalization constant)
//par[3]=Width (sigma) of convoluted Gaussian function
//
//In the Landau distribution (represented by the CERNLIB approximation), 
//the maximum is located at x=-0.22278298 with the location parameter=0.
//This shift is corrected within this function, so that the actual
//maximum is identical to the MP parameter.

// Numeric constants
	Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
	Double_t mpshift  = -0.22278298;       // Landau maximum location
// Control constants
	Double_t np = 100.0;      // number of convolution steps
	Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas
// Variables
	Double_t xx;
	Double_t mpc;
	Double_t fland;
	Double_t sum = 0.0;
	Double_t xlow,xupp;
	Double_t step;
	Double_t i;
// MP shift correction
	mpc = par[1] - mpshift * par[0]; 
// Range of convolution integral
	xlow = x[0] - sc * par[3];
	xupp = x[0] + sc * par[3];
	step = (xupp-xlow) / np;
// Convolution integral of Landau and Gaussian by sum
	for(i=1.0; i<=np/2; i++) {
		xx = xlow + (i-.5) * step;
		fland = TMath::Landau(xx,mpc,par[0]) / par[0];
		sum += fland * TMath::Gaus(x[0],xx,par[3]);
		xx = xupp - (i-.5) * step;
		fland = TMath::Landau(xx,mpc,par[0]) / par[0];
		sum += fland * TMath::Gaus(x[0],xx,par[3]);
	}
        return (par[2] * step * sum * invsq2pi / par[3]);
}

void draw_phe(
    double rSiPM = 1.0,		// MC SiPM coef
    double rPMT = 1.0, 		// MC PMT coef
    double kSiPM = 0.12,	// MC SiPM additional smearing kSiPM/sqrt(E)
    double kPMT = 0.12,		// MC PMT additional smearing kPMT/sqrt(E)
    double cSiPM = 0.04,	// MC SiPM additional smearing cSiPM
    double cPMT = 0.04,		// MC PMT additional smearing cPMT
    int NRNDM = 1)		// Number of random iterations
{
	const double PMTpheADC[64] = {
		77.6, 74.0, 73.6, 70.7, 65.7, 81.8, 71.2, 69.6, 74.6, 78.2, 
		83.2, 74.8, 63.1, 73.3, 74.1, 71.5, 62.8, 72.9, 64.4, 64.6,
		83.1, 66.4, 80.3, 68.3, 74.8, 76.4, 68.4, 58.9, 69.1, 72.3,
		68.8, 75.1, 64.7, 71.1, 72.2, 85.2, 63.2, 62.8, 60.1, 68.7,
		75.9, 65.5, 65.3, 66.5, 78.3, 73.5, 64.8, 78.9, 75.2, 81.2,
		77.6, 78.2, 71.5, 0,    0,    0,    0,    0,    0,    0,
		0,    0,    0,    0};
	const double PMTintcorr = 0.905; // integral to positive part integral ratio
	const double pheSiPM = 19.9;
	const double phePMT = 18.5;
	char strs[128], strl[1024];
	int i, j, N, adc, chan, index;
	struct {
		double index;
		double value;
	} Signal;
	struct {
		float SiPM;
		float PMT;
	} dDiff;
	TH1D *hExpC[MAXWFD][MAXCHAN];
	double gain[MAXWFD][MAXCHAN];

	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	
	// Create chains
	TChain *tExp = new TChain("DANSSSignal", "ExpSignal");
	for (i=53500; i<53650; i++) {
		sprintf(strl, "/home/clusters/rrcmpi/danss/DANSS/ROOT/Rdata/danss_data_%6.6d_Ttree.root", i);
		tExp->AddFile(strl);
	}
	tExp->SetBranchAddress("DANSSSignalNpe", &Signal);
	
	TChain *tMC = new TChain("DANSSSignal", "MCSignal");
	tMC->AddFile("/home/clusters/rrcmpi/danss/DANSS/ROOT/MC_WF/mc_Muons_glbLY_transcode_rawProc_pedSim.root");
	for (i=0; i<12; i++) {
		sprintf(strl, "/home/clusters/rrcmpi/danss/DANSS/ROOT/MC_WF/mc_Muons_glbLY_transcode_rawProc_pedSim_%2.2d.root", i);
		tMC->AddFile(strl);
	}
	tMC->SetBranchAddress("DANSSSignalNpe", &Signal);
	
	system("mkdir -p mu-calib");
	sprintf(strl, "mu-calib/%5.3f_%5.3f_%5.3f_%5.3f_%5.3f_%5.3f.root",
		rSiPM, rPMT, kSiPM, kPMT, cSiPM, cPMT); 
	TFile *fOut = new TFile(strl, "RECREATE");
	// Create hists
	TH1D *hExpSiPM = new TH1D("hExpSiPM", "Muon energy deposit, SiPM;ph.e.", 150, 0, 150);
	hExpSiPM->SetLineWidth(2);
	hExpSiPM->SetLineColor(kRed);
	TH1D *hMCSiPM = new TH1D("hMCSiPM", "Muon energy deposit, SiPM;ph.e.", 150, 0, 150);
	hMCSiPM->SetLineWidth(2);
	hMCSiPM->SetLineColor(kBlue);
	TH1D *hExpPMT = new TH1D("hExpPMT", "Muon energy deposit, PMT;ph.e.", 150, 0, 600);
	hExpPMT->SetLineWidth(2);
	hExpPMT->SetLineColor(kRed);
	TH1D *hMCPMT = new TH1D("hMCPMT", "Muon energy deposit, PMT;ph.e.", 150, 0, 600);
	hMCPMT->SetLineWidth(2);
	hMCPMT->SetLineColor(kBlue);
	TTree *tDiff = new TTree("Diff", "Diff");
	tDiff->Branch("D", &dDiff, "SiPM/F:PMT/F");

	memset(hExpC, 0, sizeof(hExpC));
	// Fill experimental hists
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
			if (!hExpC[adc][chan]) {
				sprintf(strs, "hExpC%2.2d%2.2d", adc, chan);
				sprintf(strl, "Muon energy deposit, PMT %d;ph.e.", chan);
				hExpC[adc][chan] = new TH1D(strs, strl, 150, 0, 600);
			}
			hExpPMT->Fill(Signal.value * PMTintcorr / PMTpheADC[chan]);
			hExpC[adc][chan]->Fill(Signal.value * PMTintcorr / PMTpheADC[chan]);
		} else if (adc != 3) {	// SiPM - we do nothing with veto and don't expect it here
			hExpSiPM->Fill(Signal.value);
			if (!hExpC[adc][chan]) {
				sprintf(strs, "hExpC%2.2d%2.2d", adc, chan);
				sprintf(strl, "Muon energy deposit, SiPM %d.%2.2d;ph.e.", adc, chan);
				hExpC[adc][chan] = new TH1D(strs, strl, 150, 0, 150);
			}
			hExpC[adc][chan]->Fill(Signal.value);
		}
	}
	hExpSiPM->Sumw2();
	hExpPMT->Sumw2();

	// Random numbers
	RandomSmear *SmearSiPM = new RandomSmear(time(NULL), kSiPM * sqrt(pheSiPM * rSiPM), cSiPM);
	RandomSmear *SmearPMT = new RandomSmear(time(NULL) - 10, kPMT * sqrt(phePMT * rPMT), cPMT);
	// Average & RMS
	AvrSum *AvrSiPM = new AvrSum();
	AvrSum *AvrPMT = new AvrSum();
	
	hMCSiPM->Sumw2();
	hMCPMT->Sumw2();
	
	// Fill MC hists
	N = tMC->GetEntries();
	for (j = 0; j < NRNDM; j++) {
		hMCSiPM->Reset();
		hMCPMT->Reset();
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
				hMCPMT->Fill(SmearPMT->Smear(Signal.value * rPMT));
			} else {	// SiPM
				hMCSiPM->Fill(SmearSiPM->Smear(Signal.value * rSiPM));
			}
		}
		hMCSiPM->Scale(hExpSiPM->Integral(20,150) / hMCSiPM->Integral(20,150));
		hMCPMT->Scale(hExpPMT->Integral(30,150) / hMCPMT->Integral(30,150));
		dDiff.SiPM = hDiff(hExpSiPM, hMCSiPM, 20, 150);
		dDiff.PMT = hDiff(hExpPMT, hMCPMT, 30, 150);
		AvrSiPM->Add(dDiff.SiPM);
		AvrPMT->Add(dDiff.PMT);
		tDiff->Fill();
	}

	TCanvas *cv = (TCanvas *)gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1600, 1024);
	cv->Divide(2,1);
	TLatex *lt = new TLatex();
	lt->SetTextSize(0.05);
	TLegend *lg = new TLegend(0.6, 0.7, 0.95, 0.82);
	lg->AddEntry(hExpSiPM, "Experiment", "l");
	lg->AddEntry(hMCSiPM, "Monte-Carlo", "l");
	cv->cd(1);
	hMCSiPM->DrawCopy("hist");
	hExpSiPM->DrawCopy("sames,hist");
	sprintf(strl, "#chi^{2}/n.d.f. = %6.1f+-%4.1f/%d", AvrSiPM->Avr(), AvrSiPM->RMS()/sqrt(NRNDM), 150 - 20);
	lt->DrawLatexNDC(0.3, 0.87, strl);
	sprintf(strl, "k = %5.3f", rSiPM);
	lt->DrawLatexNDC(0.45, 0.65, strl);
	sprintf(strl, "%5.3f/#sqrt{E} #oplus %5.3f", kSiPM, cSiPM);
	lt->DrawLatexNDC(0.45, 0.59, strl);
	lg->DrawClone();
	cv->cd(2);
	hMCPMT->DrawCopy("hist");
	hExpPMT->DrawCopy("sames,hist");
	sprintf(strl, "#chi^{2}/n.d.f. = %6.1f+-%4.1f/%d", AvrPMT->Avr(), AvrPMT->RMS()/sqrt(NRNDM), 150 - 30);
	lt->DrawLatexNDC(0.3, 0.87, strl);
	sprintf(strl, "k = %5.3f", rPMT);
	lt->DrawLatexNDC(0.6, 0.65, strl);
	sprintf(strl, "%5.3f/#sqrt{E} #oplus %5.3f", kPMT, cPMT);
	lt->DrawLatexNDC(0.6, 0.59, strl);
	lg->DrawClone();
	
	fOut->cd();
	for(adc = 0; adc < MAXWFD; adc++) for(chan = 0; chan < MAXCHAN; chan++) if (hExpC[adc][chan]) hExpC[adc][chan]->Write();
	cv->Write();
	hMCSiPM->Write();
	hMCPMT->Write();
	hExpSiPM->Write();
	hExpPMT->Write();
	tDiff->Write();
	fOut->Close();
	printf("The result for %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f : %6.1f %6.1f\n",
		rSiPM, rPMT, kSiPM, kPMT, cSiPM, cPMT, AvrSiPM->Avr(), AvrPMT->Avr()); 

}

void get_calibration(const char *fname)
{
	int i, j, Cnt;
	double mdn[MAXWFD][MAXCHAN];
	double mpv[MAXWFD][MAXCHAN];
	double MeanMdn, MeanMpv;
	char str[1024];
	TH1 *h;
	TF1 *fL;
	char *ptr;
	
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	memset(mdn, 0, sizeof(mdn));
	memset(mpv, 0, sizeof(mpv));
	fL = (TF1*) gROOT->FindObject("fL");
	if (fL) delete fL;
	fL = new TF1("fL", langaufun, 0, 1000, 4);
	
	for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) {
		sprintf(str, "hExpC%2.2d%2.2d", i, j);
		h = (TH1 *) f->Get(str);
		if (!h) continue;
		if (h->GetEntries() < MINEVENTS) {
			printf("%d.%2.2d: low statistics = %d\n", i, j, (int)h->GetEntries());
			mdn[i][j] = -1;
			mpv[i][j] = -1;
			continue;
		}
		mdn[i][j] = Median(h, 5, 120);
		fL->SetParameters(5, (i==1) ? 300 : 30, h->Integral(5, 120), 10);
		h->Fit(fL, "Q", "0", (i==1) ? 20 : 5, (i==1) ? 480 : 120);
		if (fL->GetChisquare() > MAXCHI2) {
			printf("%d.%2.2d: bad fit chi2 = %f\n", i, j, fL->GetChisquare());
			mpv[i][j] = -2;
			continue;
		}
		mpv[i][j] = fL->GetParameter(1);
	}
	
	f->Close();
	
	strcpy(str, fname);
	ptr = strrchr(str, '.');
	if (ptr) *ptr = '\0';
	strcat(str, ".bin");
	FILE *fOut = fopen(str, "wb");
	fwrite(mdn, sizeof(mdn), 1, fOut);
	fwrite(mpv, sizeof(mdn), 1, fOut);
	MeanMdn = MeanMpv = 0;
	Cnt = 0;
	for(i=0; i<MAXWFD; i++) for(j=0; j<MAXCHAN; j++) if (mdn[i][j] > 0) {
		MeanMdn += mdn[i][j];
		Cnt++;
	}
	MeanMdn /= Cnt;
	Cnt = 0;
	for(i=0; i<MAXWFD; i++) for(j=0; j<MAXCHAN; j++) if (mpv[i][j] > 0) {
		MeanMpv += mpv[i][j];
		Cnt++;
	}
	MeanMpv /= Cnt;
	for(i=0; i<MAXWFD; i++) for(j=0; j<MAXCHAN; j++) {
		mdn[i][j] /= MeanMdn;
		mpv[i][j] /= MeanMpv;
	}
	fwrite(mdn, sizeof(mdn), 1, fOut);
	fwrite(mpv, sizeof(mdn), 1, fOut);
	fclose(fOut);
}
