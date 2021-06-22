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
const double PMTpheADC[64] = {
	77.6, 74.0, 73.6, 70.7, 65.7, 81.8, 71.2, 69.6, 74.6, 78.2, 
	83.2, 74.8, 63.1, 73.3, 74.1, 71.5, 62.8, 72.9, 64.4, 64.6,
	83.1, 66.4, 80.3, 68.3, 74.8, 76.4, 68.4, 58.9, 69.1, 72.3,
	68.8, 75.1, 64.7, 71.1, 72.2, 85.2, 63.2, 62.8, 60.1, 68.7,
	75.9, 65.5, 65.3, 66.5, 78.3, 73.5, 64.8, 78.9, 75.2, 81.2,
	77.6, 78.2, 71.5, 0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0};
const double PMTintcorr = 0.905; // integral to positive part integral ratio

/*
 *  Do random smearing as adding Gaus with width^2 = eCoef^2 * E + cCoef^2 * E^2
 */
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


/*
 * Fill experiment and MC histograms of muon energy deposit in ph.e.
 * For PMT ADC units are translated to ph.e. using 1 ph.e. LED data
 * and integral correction
 * If file calib.bin exists it provides channel by channel gains
 * SiPM channels with median < MINMDN are not used for sum histogramm (experiment)
 */
void draw_phe(
	double rSiPM = 1.0,	// MC SiPM coef
	double rPMT = 1.0, 	// MC PMT coef
	double kSiPM = 0.12,	// MC SiPM additional smearing kSiPM/sqrt(E)
	double kPMT = 0.12,	// MC PMT additional smearing kPMT/sqrt(E)
	double cSiPM = 0.125,	// MC SiPM additional smearing cSiPM
	double cPMT = 0.06,	// MC PMT additional smearing cPMT
	int NRNDM = 1,		// Number of random iterations
	int MCIndLY = 1)	// MC has individual light yields
{
	char strs[128], strl[1024];
	int i, j, N, adc, chan, index, irc;
	struct {
		double index;
		double value;
	} Signal;
	struct {
		float SiPM;
		float PMT;
	} dDiff;
	TH1D *hExpC[MAXWFD][MAXCHAN];
	TH1D *hMCC[MAXWFD][MAXCHAN];
	double gain[MAXWFD][MAXCHAN];
	double mdn[MAXWFD][MAXCHAN];
	double merr;

	// Get gains
	irc = 0;
	FILE *fGain = fopen("calib.bin", "rb");
	if (fGain) {
//		fseek(fGain, sizeof(mpv), SEEK_CUR);
		fread(mdn, sizeof(mdn), 1, fGain);
		fseek(fGain, sizeof(mdn), SEEK_CUR);
		irc = fread(gain, sizeof(gain), 1, fGain);
	}
	gain[1][0] = gain[1][51];	// fix moved channel
	gain[1][9] = gain[1][52];	// fix moved channel
	gain[1][15] = gain[1][50];	// fix moved channel
	if (irc != 1) for (i=0; i<MAXWFD; i++) for (j=0; j<MAXCHAN; j++) gain[i][j] = 1.0;
//	printf("gains: %f %f %f\n", gain[1][5], gain[1][12], gain[2][2]);
//	return;

	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	
	// Create chains
	TChain *tExp = new TChain("DANSSSignal", "ExpSignal");
	for (i=53500; i<53650; i++) {
		sprintf(strl, "/home/clusters/rrcmpi/alekseev/igor/ROOT/Rdata_ovfl/danss_data_%6.6d_Ttree.root", i);
//		sprintf(strl, "/home/clusters/rrcmpi/alekseev/igor/hits8n1/053xxx/muhits_%6.6d.root", i);
		tExp->AddFile(strl);
	}
	tExp->SetBranchAddress("DANSSSignalNpe", &Signal);
	
	TChain *tMC = new TChain("DANSSSignal", "MCSignal");
	if (MCIndLY) {
		for (i=0; i<32; i++) {
			sprintf(strl, "/home/clusters/rrcmpi/danss/DANSS/ROOT/MC_WF_ovfl_period2/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d.root", 
				i/16, (i%16) + 1);
			tMC->AddFile(strl);
		}
	} else {
		tMC->AddFile("/home/clusters/rrcmpi/danss/DANSS/ROOT/MC_WF_ovfl_period2/mc_Muons_glbLY_transcode_rawProc_pedSim.root");
		for (i=0; i<12; i++) {
			sprintf(strl, "/home/clusters/rrcmpi/danss/DANSS/ROOT/MC_WF_ovfl_period2/mc_Muons_glbLY_transcode_rawProc_pedSim_%2.2d.root", i);
			tMC->AddFile(strl);
		}
	}
	tMC->SetBranchAddress("DANSSSignalNpe", &Signal);
	
	system("mkdir -p mu-calib");
	sprintf(strl, "mu-calib/%s%5.3f_%5.3f_%5.3f_%5.3f_%5.3f_%5.3f.root",
		(MCIndLY) ? "Ind_" : "", rSiPM, rPMT, kSiPM, kPMT, cSiPM, cPMT); 
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
	memset(hMCC, 0, sizeof(hMCC));
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
			hExpC[adc][chan]->Fill(Signal.value * PMTintcorr / PMTpheADC[chan]);
			hExpPMT->Fill(Signal.value * PMTintcorr / PMTpheADC[chan] / gain[adc][chan]); // no small gain channels
		} else if (adc != 3) {	// SiPM - we do nothing with veto and don't expect it here
			if (!hExpC[adc][chan]) {
				sprintf(strs, "hExpC%2.2d%2.2d", adc, chan);
				sprintf(strl, "Muon energy deposit, SiPM %d.%2.2d;ph.e.", adc, chan);
				hExpC[adc][chan] = new TH1D(strs, strl, 150, 0, 150);
			}
			hExpC[adc][chan]->Fill(Signal.value);
			if (mdn[adc][chan] > MINMDN) hExpSiPM->Fill(Signal.value / gain[adc][chan]);
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
	AvrSum *MdnSiPM = new AvrSum();
	AvrSum *MdnPMT = new AvrSum();
	
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
			if (MCIndLY) {
				if (adc == 1) {	// PMT
					if (!hMCC[adc][chan]) {
						sprintf(strs, "hMCC%2.2d%2.2d", adc, chan);
						sprintf(strl, "MC: Muon energy deposit, PMT %d;ph.e.", chan);
						hMCC[adc][chan] = new TH1D(strs, strl, 150, 0, 600);
					}
					hMCC[adc][chan]->Fill(Signal.value * rPMT);
					hMCPMT->Fill(SmearPMT->Smear(Signal.value / gain[adc][chan] * rPMT));
				} else {	// SiPM
					if (!hMCC[adc][chan]) {
						sprintf(strs, "hMCC%2.2d%2.2d", adc, chan);
						sprintf(strl, "MC: Muon energy deposit, SiPM %d;ph.e.", chan);
						hMCC[adc][chan] = new TH1D(strs, strl, 150, 0, 150);
					}
					hMCC[adc][chan]->Fill(Signal.value * rSiPM);
					if (mdn[adc][chan] > MINMDN) hMCSiPM->Fill(SmearSiPM->Smear(UGLY_MC_SIPM_CORR * Signal.value / gain[adc][chan] * rSiPM));
				}
			} else {
				if (adc == 1) {	// PMT
					hMCPMT->Fill(SmearPMT->Smear(Signal.value * rPMT));
				} else {	// SiPM
					hMCSiPM->Fill(SmearSiPM->Smear(Signal.value * rSiPM));
				}
			}
		}
		hMCSiPM->Scale(hExpSiPM->Integral(20,150) / hMCSiPM->Integral(20,150));
		hMCPMT->Scale(hExpPMT->Integral(50,100) / hMCPMT->Integral(50,100));
		dDiff.SiPM = hDiff(hExpSiPM, hMCSiPM, 20, 150);
		dDiff.PMT = hDiff(hExpPMT, hMCPMT, 50, 100);
		AvrSiPM->Add(dDiff.SiPM);
		AvrPMT->Add(dDiff.PMT);
		tDiff->Fill();
		MdnSiPM->Add(Median(hMCSiPM));
		MdnPMT->Add(Median(hMCPMT));
	}

	TCanvas *cv = (TCanvas *)gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1600, 1024);
	cv->Divide(2,1);
	TLatex *lt = new TLatex();
	lt->SetTextSize(0.04);
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
	sprintf(strl, "Mdn: Exp: %5.2f+-%4.2f", Median(hExpSiPM, &merr), merr);
	lt->DrawLatexNDC(0.45, 0.53, strl);
	Median(hMCSiPM, &merr);
	sprintf(strl, "MC: %5.2f+-%4.2f(%4.2f)", MdnSiPM->Avr(), merr, MdnSiPM->RMS()/sqrt(NRNDM));
	lt->DrawLatexNDC(0.45, 0.47, strl);
	lg->DrawClone();
	cv->cd(2);
	hMCPMT->DrawCopy("hist");
	hExpPMT->DrawCopy("sames,hist");
	sprintf(strl, "#chi^{2}/n.d.f. = %6.1f+-%4.1f/%d", AvrPMT->Avr(), AvrPMT->RMS()/sqrt(NRNDM), 100 - 50);
	lt->DrawLatexNDC(0.3, 0.87, strl);
	sprintf(strl, "k = %5.3f", rPMT);
	lt->DrawLatexNDC(0.55, 0.65, strl);
	sprintf(strl, "%5.3f/#sqrt{E} #oplus %5.3f", kPMT, cPMT);
	lt->DrawLatexNDC(0.55, 0.59, strl);
	sprintf(strl, "Mdn: Exp: %5.2f+-%4.2f", Median(hExpPMT, &merr), merr);
	lt->DrawLatexNDC(0.55, 0.53, strl);
	Median(hMCPMT, &merr);
	sprintf(strl, "MC: %5.2f+-%4.2f(%4.2f)", MdnPMT->Avr(), merr, MdnPMT->RMS()/sqrt(NRNDM));
	lt->DrawLatexNDC(0.55, 0.47, strl);
	lg->DrawClone();
	
	fOut->cd();
	for(adc = 0; adc < MAXWFD; adc++) for(chan = 0; chan < MAXCHAN; chan++) {
		if (hExpC[adc][chan]) hExpC[adc][chan]->Write();
		if (hMCC[adc][chan]) hMCC[adc][chan]->Write();
	}
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

void draw_phe_digi(int runFirst, int runLast)
{
	const double rPMT = 1.0;	// MC PMT coef
	const double kPMT = 0.12;	// MC PMT additional smearing kPMT/sqrt(E)
	const double cPMT = 0.06;	// MC PMT additional smearing cPMT
	const double rSiPM = 1.0;	// MC SiPM coef
	const double kSiPM = 0.12;	// MC SiPM additional smearing kPMT/sqrt(E)
	const double cSiPM = 0.125;	// MC SiPM additional smearing cPMT
	const int NRNDM = 1;
	char strs[128], strl[1024];
	int i, j, N, adc, chan, index, irc;
	struct {
		double index;
		double value;
	} Signal;
	TH1D *hExpC[MAXWFD][MAXCHAN];
	double merr;

	// Get gains
//	printf("gains: %f %f %f\n", gain[1][5], gain[1][12], gain[2][2]);
//	return;

	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	
	// Create chains
	TChain *tExp = new TChain("Pmt", "ExpSignal");
	for (i=runFirst; i<=runLast; i++) {
		sprintf(strl, "/home/clusters/rrcmpi/alekseev/igor/hits8n1/%3.3dxxx/muhits_%6.6d.root", i/1000, i);
		tExp->AddFile(strl);
	}
	tExp->SetBranchAddress("Hits", &Signal);
	
	TChain *tMC = new TChain("Pmt", "MCSignal");
	tMC->AddFile("mc_muon_hits.root");
	tMC->SetBranchAddress("Hits", &Signal);
	
	TH1D *hExpSiPM = new TH1D("hExpSiPM", "Muon energy deposit, SiPM;MeV", 50, 0, 5);
	hExpSiPM->SetLineWidth(2);
	hExpSiPM->SetLineColor(kRed);
	TH1D *hMCSiPM = new TH1D("hMCSiPM", "Muon energy deposit, SiPM;MeV", 50, 0, 5);
	hMCSiPM->SetLineWidth(2);
	hMCSiPM->SetLineColor(kBlue);
	TH1D *hExpPMT = new TH1D("hExpPMT", "Muon energy deposit, PMT;MeV", 140, 0, 35);
	hExpPMT->SetLineWidth(2);
	hExpPMT->SetLineColor(kRed);
	TH1D *hMCPMT = new TH1D("hMCPMT", "Muon energy deposit, PMT;MeV", 140, 0, 35);
	hMCPMT->SetLineWidth(2);
	hMCPMT->SetLineColor(kBlue);

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
				sprintf(strl, "Muon energy deposit, PMT %d;MeV", chan);
				hExpC[adc][chan] = new TH1D(strs, strl, 140, 0, 35);
			}
			hExpC[adc][chan]->Fill(Signal.value);
			hExpPMT->Fill(Signal.value); // no small gain channels
		} 
		else if (adc != 3) {	// SiPM - we do nothing with veto and don't expect it here
			if (!hExpC[adc][chan]) {
				sprintf(strs, "hExpC%2.2d%2.2d", adc, chan);
				sprintf(strl, "Muon energy deposit, SiPM %d.%2.2d;ph.e.", adc, chan);
				hExpC[adc][chan] = new TH1D(strs, strl, 50, 0, 5);
			}
			hExpC[adc][chan]->Fill(Signal.value);
			hExpSiPM->Fill(Signal.value);
		}

	}
	hExpSiPM->Sumw2();
	hExpPMT->Sumw2();

	// Random numbers
	RandomSmear *SmearSiPM = new RandomSmear(time(NULL), kSiPM * sqrt(rSiPM), cSiPM);
	RandomSmear *SmearPMT = new RandomSmear(time(NULL) - 10, kPMT * sqrt(rPMT), cPMT);
	// Average & RMS
//	AvrSum *AvrSiPM = new AvrSum();
//	AvrSum *AvrPMT = new AvrSum();
//	AvrSum *MdnSiPM = new AvrSum();
//	AvrSum *MdnPMT = new AvrSum();
	
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
			} else if (adc != 3) {	// SiPM
				hMCSiPM->Fill(SmearSiPM->Smear(Signal.value * rSiPM));
			}
		}
		hMCSiPM->Scale(hExpSiPM->Integral(5, 45) / hMCSiPM->Integral(5, 45));
		hMCPMT->Scale(hExpPMT->Integral(50, 100) / hMCPMT->Integral(50, 100));
//		dDiff.SiPM = hDiff(hExpSiPM, hMCSiPM, 20, 150);
//		dDiff.PMT = hDiff(hExpPMT, hMCPMT, 50, 100);
//		AvrSiPM->Add(dDiff.SiPM);
//		AvrPMT->Add(dDiff.PMT);
//		tDiff->Fill();
//		MdnSiPM->Add(Median(hMCSiPM));
//		MdnPMT->Add(Median(hMCPMT));
	}

	TCanvas *cv = (TCanvas *)gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1600, 1024);
	cv->Divide(2, 1);
	TLatex *lt = new TLatex();
	lt->SetTextSize(0.04);
	TLegend *lg = new TLegend(0.65, 0.8, 0.98, 0.9);
	lg->AddEntry(hExpPMT, "Experiment", "l");
	lg->AddEntry(hMCPMT, "Monte-Carlo", "l");
	cv->cd(1);
	hMCSiPM->DrawCopy("hist");
	hExpSiPM->DrawCopy("hist,same");
	sprintf(strl, "Exp: %5.3f+-%5.3f", Median(hExpSiPM, &merr), merr);
	lt->DrawLatexNDC(0.6, 0.75, strl);
	sprintf(strl, "MC: %5.3f+-%5.3f", Median(hMCSiPM, &merr), merr);
	lt->DrawLatexNDC(0.6, 0.70, strl);
	lg->DrawClone();
	cv->cd(2);
	hMCPMT->DrawCopy("hist");
	hExpPMT->DrawCopy("hist,same");
	sprintf(strl, "Exp: %5.2f+-%4.2f", Median(hExpPMT, &merr), merr);
	lt->DrawLatexNDC(0.6, 0.75, strl);
	sprintf(strl, "MC: %5.2f+-%4.2f", Median(hMCPMT, &merr), merr);
	lt->DrawLatexNDC(0.6, 0.70, strl);
	lg->DrawClone();
}

/*
 * Calculate per channel calibration from histogramms.
 * Bin file with medians, MPVs and gains calculated from them is created.
 * SiPM channels with Median < MINMDN are not used in the averaging.
 */
void get_calibration(const char *fname, int toWFD = 100)
{
	int i, j, Cnt;
	double mdn[MAXWFD][MAXCHAN];
	double mpv[MAXWFD][MAXCHAN];
	double MeanMdn, MeanMpv;
	char str[1024];
	char pdfname[1024];
	TH1 *h;
	TF1 *fL;
	char *ptr;

	TH1D *hMedianSiPM = new TH1D("hMedianSiPM", "Median for SiPM;ph.e.", 100, 0, 50);
	TH1D *hMPVSiPM = new TH1D("hMPVSiPM", "MPV for SiPM;ph.e.", 100, 0, 50);
	TH1D *hMedianPMT = new TH1D("hMedianPMT", "Median for PMT;ph.e.", 40, 200, 400);
	TH1D *hMPVPMT = new TH1D("hMPVPMT", "MPV for PMT;ph.e.", 40, 200, 400);
	TH1D *hMedian2MPVSiPM = new TH1D("hMedian2MPVSiPM", "Median to MPV ratio, SiPM;Median/MPV", 60, 0.9, 1.5);
	TH1D *hMedian2MPVPMT = new TH1D("hMedian2MPVPMT", "Median to MPV ratio, PMT;Median/MPV", 30, 0.9, 1.2);
	TH1D *hLwidthSiPM = new TH1D("hLwidthSiPM", "Landau width, SiPM;ph.e.", 100, 0, 10);
	TH1D *hLwidthPMT = new TH1D("hLwidthPMT", "Landau width, PMT;ph.e.", 100, 0, 50);
	TH1D *hGwidthSiPM = new TH1D("hGwidthSiPM", "Gauss width, SiPM;ph.e.", 100, 0, 30);
	TH1D *hGwidthPMT = new TH1D("hGwidthPMT", "Gauss width, PMT;ph.e.", 100, 0, 100);
	TH1D *hChi2SiPM = new TH1D("hChi2SiPM", "#chi^{2}, SiPM", 100, 0, 1000);
	TH1D *hChi2PMT = new TH1D("hChi2PMT", "#chi^{2}, PMT", 100, 0, 1000);
	TLatex *ltx = new TLatex();
	ltx->SetTextSize(0.025);
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	memset(mdn, 0, sizeof(mdn));
	memset(mpv, 0, sizeof(mpv));
	fL = (TF1*) gROOT->FindObject("fL");
	if (fL) delete fL;
	fL = new TF1("fL", langaufun, 0, 1000, 4);
	strcpy(pdfname, fname);
	for(ptr = pdfname + strlen(pdfname); ptr >= str; ptr--) if (*ptr == '.') break;
	if (*ptr == '.') *ptr = '\0';
	strcat(pdfname, ".pdf[");
	TCanvas ccv("CCV", "CCV", 800, 800);
	ccv.SaveAs(pdfname);
	ptr[4] = '\0';

	for (i=0; i<MAXWFD && i < toWFD; i++) for (j=0; j<MAXCHAN; j++) {
		sprintf(str, "hExpC%2.2d%2.2d", i, j);
		h = (TH1 *) f->Get(str);
		if (!h) continue;
		if (h->GetEntries() < MINEVENTS) {
			printf("%d.%2.2d: low statistics = %d\n", i, j, (int)h->GetEntries());
			mdn[i][j] = -1;
			mpv[i][j] = -1;
			continue;
		}
		mdn[i][j] = Median(h);
		if (mdn[i][j] < MINMEDIAN) continue;
		fL->SetParameters(mdn[i][j] / ((i==1) ? 40 : 9), mdn[i][j], h->Integral(5, 120), 10);
		fL->FixParameter(0, mdn[i][j] / ((i==1) ? 40 : 9));
		h->Fit(fL, "Q", "", (i==1) ? 180 : 5, (i==1) ? 480 : 120);
		if (fL->GetChisquare() > MAXCHI2) {
			printf("%d.%2.2d: bad fit chi2 = %f\n", i, j, fL->GetChisquare());
			mpv[i][j] = -2;
		} else {
			mpv[i][j] = fL->GetParameter(1);
		}
		sprintf(str, "%2.2d.%2.2d: Evts = %d, Median = %6.2f, MPV = %6.2f, #chi^{2} = %5.0f", 
			i, j, (int)h->GetEntries(), mdn[i][j], mpv[i][j], fL->GetChisquare());
		ltx->DrawLatexNDC(0.12, 0.89, str);
		sprintf(str, "Lwidth = %6.2f, Gwidth = %6.2f", 
			fL->GetParameter(0), fL->GetParameter(3));
		ltx->DrawLatexNDC(0.12, 0.85, str);
		ccv.SaveAs(pdfname);
		if (i == 1) {
			hMedianPMT->Fill(mdn[i][j]);
			hMPVPMT->Fill(mpv[i][j]);
			if (mpv[i][j] > 0) {
				hMedian2MPVPMT->Fill(mdn[i][j]/mpv[i][j]);
				hLwidthPMT->Fill(fL->GetParameter(0));
				hGwidthPMT->Fill(fL->GetParameter(3));
				hChi2PMT->Fill(fL->GetChisquare());
			}
		} else {
			hMedianSiPM->Fill(mdn[i][j]);
			hMPVSiPM->Fill(mpv[i][j]);
			if (mpv[i][j] > 0) {
				hMedian2MPVSiPM->Fill(mdn[i][j]/mpv[i][j]);
				hLwidthSiPM->Fill(fL->GetParameter(0));
				hGwidthSiPM->Fill(fL->GetParameter(3));
				hChi2SiPM->Fill(fL->GetChisquare());
			}
		}
	}

	hMedianSiPM->SetStats(1);
	hMedianSiPM->Draw();
	ccv.SaveAs(pdfname);
	hMPVSiPM->SetStats(1);
	hMPVSiPM->Draw();
	ccv.SaveAs(pdfname);
	hMedian2MPVSiPM->SetStats(1);
	hMedian2MPVSiPM->Draw();
	ccv.SaveAs(pdfname);
	hLwidthSiPM->SetStats(1);
	hLwidthSiPM->Draw();
	ccv.SaveAs(pdfname);
	hGwidthSiPM->SetStats(1);
	hGwidthSiPM->Draw();
	ccv.SaveAs(pdfname);
	hChi2SiPM->SetStats(1);
	hChi2SiPM->Draw();
	ccv.SaveAs(pdfname);
	hMedianPMT->SetStats(1);
	hMedianPMT->Draw();
	ccv.SaveAs(pdfname);
	hMPVPMT->SetStats(1);
	hMPVPMT->Draw();
	ccv.SaveAs(pdfname);
	hMedian2MPVPMT->SetStats(1);
	hMedian2MPVPMT->Draw();
	ccv.SaveAs(pdfname);
	hLwidthPMT->SetStats(1);
	hLwidthPMT->Draw();
	ccv.SaveAs(pdfname);
	hGwidthPMT->SetStats(1);
	hGwidthPMT->Draw();
	ccv.SaveAs(pdfname);
	hChi2PMT->SetStats(1);
	hChi2PMT->Draw();
	ccv.SaveAs(pdfname);

	strcat(pdfname, "]");
	ccv.SaveAs(pdfname);
	f->Close();

	strcpy(str, fname);
	for(ptr = str + strlen(str); ptr >= str; ptr--) if (*ptr == '.') break;
	if (*ptr == '.') *ptr = '\0';
	strcat(str, ".bin");
	FILE *fOut = fopen(str, "wb");
	fwrite(mdn, sizeof(mdn), 1, fOut);
	fwrite(mpv, sizeof(mpv), 1, fOut);
//		PMT
	MeanMdn = MeanMpv = 0;
	Cnt = 0;
	for(j=0; j<MAXCHAN; j++) if (mdn[1][j] > 0) {
		MeanMdn += mdn[1][j];
		Cnt++;
	}
	MeanMdn /= Cnt;
	Cnt = 0;
	for(j=0; j<MAXCHAN; j++) if (mpv[1][j] > 0) {
		MeanMpv += mpv[1][j];
		Cnt++;
	}
	MeanMpv /= Cnt;
	for(j=0; j<MAXCHAN; j++) {
		mdn[1][j] /= MeanMdn;
		mpv[1][j] /= MeanMpv;
	}
//		SiPM
	MeanMdn = MeanMpv = 0;
	Cnt = 0;
	for (i=2; i<MAXWFD && i<toWFD ; i++) for(j=0; j<MAXCHAN; j++) if (mdn[i][j] > MINMDN) {
		MeanMdn += mdn[i][j];
		MeanMpv += mpv[i][j];
		Cnt++;
	}
	MeanMdn /= Cnt;
	MeanMpv /= Cnt;
	for (i=2; i<MAXWFD && i<toWFD; i++) for(j=0; j<MAXCHAN; j++) {
		mdn[i][j] /= MeanMdn;
		mpv[i][j] /= MeanMpv;
	}

	fwrite(mdn, sizeof(mdn), 1, fOut);
	fwrite(mpv, sizeof(mpv), 1, fOut);
	fclose(fOut);
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
	char *ptr = strstr(str, key);
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

/*
 * Calculate per channel calibration in ph.e/MeV in format sutable for MC
 * MC median values for K=1 should be provided
 * MedMCSiPM = 35.63  MedMCPMT = 282.0
 * The result is written in two files (for SiPM and PMT)
 */
void calib4sasha(const char *fname, double MedMCSiPM, double MedMCPMT, double corrSiPM = 1)
{
	int i;
	double mdn[MAXWFD][MAXCHAN];
	struct {
		double cellindex;
		double row;
		double column;
		double energy;
	} data;
	int mod, chan, column, row;
	TFile *fOut;
	
	DANSSGeom geom;
	
	FILE *fIn = fopen(fname, "rb");
	if (!fIn) {
		printf("Can not open file %s: %m\n", fname);
		return;
	}
	i = fread(mdn, sizeof(mdn), 1, fIn);
	if (i != 1) {
		printf("file %s read error: %m\n", fname);
		fclose(fIn);
		return;
	}
	fclose(fIn);
//		Cycle over SiPM
	fOut = new TFile("SiPM_response_data.root", "RECREATE");
	TTree *tSiPM = new TTree("EnergyTree", "Tree with coefficients");
	tSiPM->Branch("EnergyBranch", &data, "cellindex/D:row:column:energy");
	
	for (i=0; i<2500; i++) {
		column = geom.SiPMcolumn(i);
		row    = geom.SiPMrow(i);
		mod    = geom.SiPMWFD(i);
		chan   = geom.SiPMchan(i);
		data.energy = (mdn[mod][chan] > 0) ? mdn[mod][chan] * pheSiPM / MedMCSiPM : pheSiPM;	// nominal for dead channels
		data.energy *= corrSiPM;		// correction to overcome Ira's bug
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
		data.energy = mdn[mod][chan] * phePMT / MedMCPMT;
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

void dump_calib(const char *fnameIn, const char *fnameOut)
{
	double mdn[MAXWFD][MAXCHAN];
	int j, k;
	
	FILE *fIn = fopen(fnameIn, "rb");
	if (!fIn) {
		printf("Can not open file %s: %m\n", fnameIn);
		return;
	}
	j = fread(mdn, sizeof(mdn), 1, fIn);
	if (j != 1) {
		printf("file %s read error: %m\n", fnameIn);
		fclose(fIn);
		return;
	}
	fclose(fIn);
	FILE *fOut = fopen(fnameOut, "wt");
	if (!fOut) {
		printf("Can not open file %s: %m\n", fnameOut);
		return ;
	}
	for(k=0; k<MAXCHAN; k++) if (mdn[1][k] > 0) 
		fprintf(fOut, "ADC 01.%2.2d = %8.3f\n", k, (mdn[1][k] / 18.539) * (PMTpheADC[k] / PMTintcorr));
	for(j=2; j<MAXWFD; j++) for(k=0; k<MAXCHAN; k++) if (mdn[j][k] > 0) 
		fprintf(fOut, "ADC %2.2d.%2.2d = %8.3f\n", j, k, mdn[j][k] / 1.745);
	fclose(fOut);
}

int loadCalib(const char *fname, double clb[MAXWFD][MAXCHAN])
{
	char str[1024];
	int i, j;
	FILE *f = fopen(fname, "rt");
	if (!f) {
		printf("Can not open file %s: %m\n", fname);
		return -1;
	}
	memset(clb, 0, sizeof(double) * MAXWFD * MAXCHAN);
	for (;;) {
		if (!fgets(str, sizeof(str), f)) break;
		if (strlen(str) < 5) continue;
		if (str[0] != 'A') continue;
		i = strtol(&str[4], NULL, 10);
		j = strtol(&str[7], NULL, 10);
		if (i < 0 || i >= MAXWFD) continue;
		if (j < 0 || j >= MAXCHAN) continue;
		clb[i][j] = strtod(&str[12], NULL);
	}
	fclose(f);
	return 0;
}


void cmp2calib(const char *fA, const char *fB, int what = 3)
{
	double clbA[MAXWFD][MAXCHAN];
	double clbB[MAXWFD][MAXCHAN];
	int i, j;
	
	if (loadCalib(fA, clbA) || loadCalib(fB, clbB)) return;
	TH1D *h = new TH1D("hcDiff", "Calibration difference", 100, -0.1, 0.1);
	if (what & 1) for(i=1; i<2; i++) for(j=0; j<MAXCHAN; j++) if (clbA[i][j] > 0 && clbB[i][j] > 0) 
		h->Fill((clbA[i][j] - clbB[i][j]) / (clbA[i][j] + clbB[i][j]));
	if (what & 2) for(i=2; i<MAXWFD; i++) for(j=0; j<MAXCHAN; j++) if (clbA[i][j] > 0 && clbB[i][j] > 0 && i != 3) 
		h->Fill((clbA[i][j] - clbB[i][j]) / (clbA[i][j] + clbB[i][j]));
	h->Draw();
}

void cmpExp2MCcalib(const char *fname, const char *txtname)
{
	char strE[256], strM[256];
	int i, j;
	TH1D *hExp;
	TH1D *hMC;
	double vExp, eExp;
	double vMC, eMC;
	int nExp, nMC;
	double diff2;
	double Sum2;
	int Cnt;
	
	TFile *fIn = new TFile(fname);
	if (!fIn->IsOpen()) return;
	FILE *fOut = fopen(txtname, "wt");
	if (!fOut) return;
	
	Sum2 = 0;
	Cnt = 0;
	for(i=1; i<MAXWFD; i++) for(j=0; j<MAXCHAN; j++) {
		sprintf(strE, "hExpC%2.2d%2.2d", i, j);
		hExp = (TH1D*) fIn->Get(strE);
		if (!hExp) continue;
		sprintf(strM, "hMCC%2.2d%2.2d", i, j);
		hMC = (TH1D*) fIn->Get(strM);
		if (!hMC) continue;
		nExp = hExp->GetEntries();
		nMC = hMC->GetEntries();
		vExp = Median(hExp, &eExp);
		vMC  = Median(hMC, &eMC);
		diff2 = (vExp - vMC) * (vExp - vMC) / (eExp*eExp + eMC*eMC);
		Sum2 += diff2;
		Cnt++;
		fprintf(fOut, "%2.2d.%2.2d : Exp %8.3f +- %6.3f [%8d hits]  <--> MC %8.3f +- %6.3f [%8d hits] ==>> %8.3f\n",
			i, j, vExp, eExp, nExp, vMC, eMC, nMC, diff2);
	}
	fprintf(fOut, "Sum2/n.d.f. = %f / %d \n", Sum2, Cnt);
	fclose(fOut);
	fIn->Close();
}
