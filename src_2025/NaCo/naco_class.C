#include "../../evtbuilder.h"
/***************************************************************************************
*			Runs with sources
* Runs		Source	Date		Position	Nruns
* 12198-12303	none	2017-02-03	down
* 12306-12346	60Co	2017-02-04	center		41
* 12348-12361	60Co	2017-02-05	edge		14
* 12364-12373	22Na	2017-02-05	edge		10
* 12376-12407	22Na	2017-02-05	center		32
* 12411-12421	none	2017-02-06	down		later position was changed to up
* 50750-50873	none	2018-11-18	down
* 50949-50997	60Co	2018-11-20	center		49
* 50999-51034	60Co	2018-11-21	edge		36
* 51036-51095	22Na	2018-11-21	edge		60
* 51099-51161	22Na	2018-11-22	center		63
* 51167-51267	none	2018-11-23	down
* 127280-127379	none	2022-06-19	down
* 127389-127456	22Na	2022-06-21	center		68
* 127462-127512	none	2022-06-22	down
* 127513-127580	24Na	2022-06-23	center, not seen, strong pickup (?)
* 127581-127657	none	2022-06-24	down
* 127658-127719	24Na	2022-06-25	center, not seen
* 127720-127772	248Cm	2022-06-26	center		53
* 127774-127837	60Co	2022-06-27	center		64
* 127838-127900	none	2022-06-28	down
* 127901-127957	none	2022-06-29	up
* 127958-128016	22Na	2022-06-30	center, UP	59
* 128020-128119	none	2022-07-01	up
* 174324-174343	none	2025-04-22	down		20
* 174345-174400	60Co	2025-04-22	center		58
* 174403-174457	Th	2025-04-23	center		55
* 174459-174496	none	2025-04-24	down		38
* 174720-174828	none	2025-04-30	down		109
* 174830-175058	22Na	2025-05-01	center		229
* 175060-175114	none	2025-05-05	down		55
*
***************************************************************************************
* Center position (50, 50, 50)
* Edge position   (50, 90, 50)
***************************************************************************************
* The channel for teflon tube to introduce sources is machined in left low part of 
* strip X=12 Z=49. So strips X=11,12 Z=49 and Y=12 Z=48 could be afected by beta
* reaching it through the capsule. We remove events with these strips hit
***************************************************************************************/

const double centerX = 50;
const double centerY = 50;
const double centerZ = 50;

struct EnergyStruct {
	double SiPM;
	double PMT;
};

class NaCoClass {
private:
	int kEmin;	// Energy range
	int kEmax;
	double Emin;
	double Emax;
	double Rcut;
	TRandom *rndm;
	TChain *tMC;
	TH1D *hExp;
	TH1D *hExpSiPM;
	TH1D *hExpPMT;
	TFile *fExp;
	struct EnergyStruct *MCEnergy;
	int NMC;
	struct DanssEventStruct7 MCdata;
	double kScaleSiPM, kScalePMT;	// energy scale
	double kRndmSiPM, kRndmPMT;	// proportional smearing
	double kSqrtSiPM, kSqrtPMT;	// sqrt smearing
	char *Name;
	
	double chi2Diff(TH1 *hcm, TH1 *hMc);
	TChain *create_chain(const char *fname, const char *chname);
	void MakeMCArray(void);
	double calcE(double E, double kScl, double kRnd, double kSqr);
	void Erase(const char *name);
public:
	NaCoClass(const char *expname, const char *mcname, const char *name);
	~NaCoClass(void);
	void SetErange(int nMin, int nMax);
	void SetErange(double rMin, double rMax);
	inline void SetScale(double kSiPM, double kPMT) { kScaleSiPM = kSiPM; kScalePMT = kPMT; };
	inline void SetRndm(double kSiPM, double kPMT) { kRndmSiPM = kSiPM; kRndmPMT = kPMT; };
	inline void SetSqrt(double kSiPM, double kPMT) { kSqrtSiPM = kSiPM; kSqrtPMT = kPMT; };
	inline void SetRcut(double val) { Rcut = val; };
	void ScanScale(int nDiv, double sMin, double sMax, int iSet);
	void ScanRndm(int nDiv, double sMin, double sMax, int iSet);
	void ScanSqrt(int nDiv, double sMin, double sMax, int iSet);
};

NaCoClass::NaCoClass(const char *expname, const char *mcname, const char *name)
{
	printf("Making class for Run=%s and MC=%s\n", expname, mcname);
	hExp = hExpSiPM = hExpPMT = NULL;
	fExp = NULL;
	tMC = NULL;
	Name = strdup(name);
	MCEnergy = NULL;
	NMC = 0;
	kScaleSiPM = kScalePMT = 1.0;
	kRndmSiPM = kRndmPMT = 0.08;
	kSqrtSiPM = kSqrtPMT = 0.04;
	Rcut = 30;	// cm
	rndm = new TRandom2(time(NULL));
	
	fExp = new TFile(expname);
	if (!fExp->IsOpen()) return;
	hExp = (TH1D *) fExp->Get("hExpC");
	hExpSiPM = (TH1D *) fExp->Get("hExpSiPMC");
	hExpPMT = (TH1D *) fExp->Get("hExpPMTC");
	if (!hExp || !hExpSiPM || !hExpPMT) {
		printf("Not all histograms found in %s.\n", expname);
		return;
	}
	hExp->SetLineColor(kRed);
	hExp->SetMarkerColor(kRed);
	hExp->SetMarkerStyle(kFullCircle);
	hExp->SetMarkerSize(0.03);
	hExp->SetStats(0);
	hExpSiPM->SetLineColor(kRed);
	hExpSiPM->SetMarkerColor(kRed);
	hExpSiPM->SetMarkerStyle(kFullCircle);
	hExpSiPM->SetMarkerSize(0.03);
	hExpSiPM->SetStats(0);
	hExpPMT->SetLineColor(kRed);
	hExpPMT->SetMarkerColor(kRed);
	hExpPMT->SetMarkerStyle(kFullCircle);
	hExpPMT->SetMarkerSize(0.03);
	hExpPMT->SetStats(0);

	tMC = create_chain(mcname, "DanssEvent");
	if (!tMC) {
		printf("Can not open MC: %s\n", mcname);
		return;
	}
	tMC->SetBranchAddress("Data", &MCdata);
	MakeMCArray();
	SetErange(0.8, 2.9);
}

NaCoClass::~NaCoClass(void)
{
	if (MCEnergy) free(MCEnergy);
	if (fExp && fExp->IsOpen()) delete fExp;
	if (tMC) delete tMC;
	if (Name) free(Name);
	delete rndm;
}

void NaCoClass::Erase(const char *name)
{
	TObject *obj = gROOT->FindObject(name);
	if (obj) delete obj;
}

void NaCoClass::MakeMCArray(void)
{
	int N;
	int i;
	double r2;
	
	N = tMC->GetEntries();
	if (MCEnergy) free(MCEnergy);
	MCEnergy = (struct EnergyStruct *) malloc(N * sizeof(struct EnergyStruct));	// Maximum possible
	if (!MCEnergy) {
		printf("Can not allocate memory for %d events\n", N);
		return;
	}
	NMC = 0;
	for (i=0; i<N; i++) {
		tMC->GetEntry(i);
//	TCut cxyz("NeutronX[0] >= 0 && NeutronX[1] >= 0 && NeutronX[2] >= 0");
		if (MCdata.NeutronX[0] < 0) continue;
		if (MCdata.NeutronX[1] < 0) continue;
		if (MCdata.NeutronX[2] < 0) continue;
//	TCut cVeto("VetoCleanHits < 2 && VetoCleanEnergy < 4");
		if (MCdata.VetoCleanHits >= 2 || MCdata.VetoCleanEnergy >= 4) continue;
//	TCut cn("SiPmCleanHits > 2");
		if (MCdata.SiPmCleanHits <= 2) continue;
		if (MCdata.PmtCleanHits < 1) continue;
//	Cut over r2
		r2  = (MCdata.NeutronX[0] + 2 - centerX) * (MCdata.NeutronX[0] + 2 - centerX);
		r2 += (MCdata.NeutronX[1] + 2 - centerY) * (MCdata.NeutronX[1] + 2 - centerY);
		r2 += (MCdata.NeutronX[2] + 0.5 - centerZ) * (MCdata.NeutronX[2] + 0.5 - centerZ);
		if (r2 > Rcut*Rcut) continue;		// 30 cm
		MCEnergy[NMC].SiPM = MCdata.SiPmCleanEnergy;
		MCEnergy[NMC].PMT = MCdata.PmtCleanEnergy;
		NMC++;
	}
	printf("MC: %d events from %d triggers\n", NMC, N);
}

//	Calculate chi^2 difference of two histograms
//  hcm, hMc - 1-dim histograms
double NaCoClass::chi2Diff(TH1 *hcm, TH1 *hMc)
{
	int i;
	double chi2 = 0;
	
	for (i=kEmin; i<=kEmax; i++) 
		chi2 += (hcm->GetBinContent(i) - hMc->GetBinContent(i)) * (hcm->GetBinContent(i) - hMc->GetBinContent(i)) /
		(hcm->GetBinError(i) * hcm->GetBinError(i) + hMc->GetBinError(i) * hMc->GetBinError(i));

	return chi2;
}

//	Create a chain and adds files to it
//  fname   - file name(s)
//  chname  - the chainname
TChain *NaCoClass::create_chain(const char *fname, const char *chname)
{
	char *tok;
	const char *DELIM = " \t,;";
	char *str;
	
	TChain *ch = new TChain(chname);
	
	str = strdup(fname);
	if (!str) return NULL;
	tok = strtok(str, DELIM);
	for(;;) {
		if (!tok) break;
		ch->AddFile(tok);
		tok = strtok(NULL, DELIM);
	}
	if (!ch->GetEntries()) return NULL;
	
	free(str);
	return ch;
}

//	Calculate scaled and smeared energy
//	E - original energy
//	kScl - scale
//	kRnd - proportional smearing
//	kSqr - stokhastic smearing
double NaCoClass::calcE(double E, double kScl, double kRnd, double kSqr)
{
	double X = kScl * E;
	double S = sqrt(kRnd * kRnd * X * X + kSqr * kSqr * X);
	return rndm->Gaus(X, S);
}

//	Set range for histogram compare
//	nMin, nMax - bin numbers
void NaCoClass::SetErange(int nMin, int nMax)
{
	kEmin = nMin;
	kEmax = nMax;
	if(!hExp) {
		printf("ERROR: No histogramm!\n");
		return;
	}
	Emin = hExp->GetXaxis()->GetBinLowEdge(kEmin);
	Emax = hExp->GetXaxis()->GetBinUpEdge(kEmax);
}

//	Set range for histogram compare
//	rMin, rMax - minimum and maximum energy
void NaCoClass::SetErange(double rMin, double rMax)
{
	if(!hExp) {
		printf("ERROR: No histogramm!\n");
		return;
	}
	kEmin = hExp->GetXaxis()->FindBin(rMin);
	kEmax = hExp->GetXaxis()->FindBin(rMax);
	Emin = hExp->GetXaxis()->GetBinLowEdge(kEmin);
	Emax = hExp->GetXaxis()->GetBinUpEdge(kEmax);
}

void NaCoClass::ScanScale(int nDiv, double sMin, double sMax, int iSet = 0)
{
	double xmin;
	double xminSiPM, xminPMT;
	TLatex txt;
	TLine ln;
	double ds;
	int nBins;
	double rLow, rUp;
	int i, j;
	double Scoef;
	double ESiPM, EPMT;
	double xLow, xUp;
	int imin;
	char str[256];
	
	// Erase old
	Erase("hScan");
	Erase("hScanSiPM");
	Erase("hScanPMT");
	Erase("hTmp");
	Erase("hTmpSiPM");
	Erase("hTmpPMT");
	Erase("fpol2");
	// Book histogramms
	ds = 0.5 * (sMax - sMin) / (nDiv - 1);
	TH1D *hScan = new TH1D("hScan", "#chi^{2} difference between MC and data, SiPM+PMT;Scale;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanSiPM = new TH1D("hScanSiPM", "#chi^{2} difference between MC and data, SiPM;Scale;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanPMT = new TH1D("hScanPMT", "#chi^{2} difference between MC and data, PMT;Scale;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	nBins = hExp->GetXaxis()->GetNbins();
	rLow = hExp->GetXaxis()->GetBinLowEdge(1);
	rUp = hExp->GetXaxis()->GetBinUpEdge(nBins);
	sprintf(str, "SiPM+PMT %s;MeV", Name);
	TH1D *hTmp = new TH1D("hTmp", str, nBins, rLow, rUp);
	sprintf(str, "SiPM %s;MeV", Name);
	TH1D *hTmpSiPM = new TH1D("hTmpSiPM", str, nBins, rLow, rUp);
	sprintf(str, "PMT %s;MeV", Name);
	TH1D *hTmpPMT = new TH1D("hTmpPMT", str, nBins, rLow, rUp);
	TF1 *fpol2 = new TF1("fpol2", "pol2", sMin, sMax);
	// Fill with smeared MC
	for (i=0; i<nDiv; i++) {
		Scoef = sMin + 2 * ds * i;
		hTmp->Reset();
		hTmpSiPM->Reset();
		hTmpPMT->Reset();
		for (j=0; j<NMC; j++) {
			ESiPM = calcE(MCEnergy[j].SiPM, Scoef, kRndmSiPM, kSqrtSiPM);
			EPMT = calcE(MCEnergy[j].PMT, Scoef, kRndmPMT, kSqrtPMT);
			hTmp->Fill((ESiPM + EPMT) / 2);
			hTmpSiPM->Fill(ESiPM);
			hTmpPMT->Fill(EPMT);
		}
		hTmp->Scale(hExp->Integral(kEmin, kEmax) / hTmp->Integral(kEmin, kEmax));
		hTmpSiPM->Scale(hExpSiPM->Integral(kEmin, kEmax) / hTmpSiPM->Integral(kEmin, kEmax));
		hTmpPMT->Scale(hExpPMT->Integral(kEmin, kEmax) / hTmpPMT->Integral(kEmin, kEmax));
		hScan->SetBinContent(i+1, chi2Diff(hExp, hTmp));
		hScanSiPM->SetBinContent(i+1, chi2Diff(hExpSiPM, hTmpSiPM));
		hScanPMT->SetBinContent(i+1, chi2Diff(hExpPMT, hTmpPMT));
		printf(">");
		fflush(stdout);
	}
	printf("\n");

	gStyle->SetOptStat(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	cv->Clear();
	cv->Divide(3, 2);
//		find minimums
	cv->cd(4);
	imin = hScan->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScan->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScan->GetXaxis()->GetBinCenter(imin + 5);
	hScan->Fit(fpol2, "", "", xLow, xUp);
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.77, str);

	cv->cd(5);
	imin = hScanSiPM->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScanSiPM->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScanSiPM->GetXaxis()->GetBinCenter(imin + 5);
	hScanSiPM->Fit(fpol2, "", "", xLow, xUp);
	xminSiPM = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xminSiPM);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xminSiPM));
	txt.DrawLatexNDC(0.4, 0.77, str);
	sprintf(str, "Smear=%5.3f", kRndmSiPM);
	txt.DrawLatexNDC(0.4, 0.69, str);
	sprintf(str, "Stoch=%5.3f", kSqrtSiPM);
	txt.DrawLatexNDC(0.4, 0.61, str);

	cv->cd(6);
	imin = hScanPMT->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScanPMT->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScanPMT->GetXaxis()->GetBinCenter(imin + 5);
	hScanPMT->Fit(fpol2, "", "", xLow, xUp);
	xminPMT = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xminPMT);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xminPMT));
	txt.DrawLatexNDC(0.4, 0.77, str);
	sprintf(str, "Smear=%5.3f", kRndmPMT);
	txt.DrawLatexNDC(0.4, 0.69, str);
	sprintf(str, "Stoch=%5.3f", kSqrtPMT);
	txt.DrawLatexNDC(0.4, 0.61, str);

//		Draw best fits
	hTmp->Reset();
	hTmpSiPM->Reset();
	hTmpPMT->Reset();
	for (j=0; j<NMC; j++) {
		ESiPM = calcE(MCEnergy[j].SiPM, xminSiPM, kRndmSiPM, kSqrtSiPM);
		EPMT = calcE(MCEnergy[j].PMT, xminPMT, kRndmPMT, kSqrtPMT);
		hTmp->Fill((ESiPM + EPMT) / 2);
		hTmpSiPM->Fill(ESiPM);
		hTmpPMT->Fill(EPMT);
	}
	hTmp->Scale(hExp->Integral(kEmin, kEmax) / hTmp->Integral(kEmin, kEmax));
	hTmpSiPM->Scale(hExpSiPM->Integral(kEmin, kEmax) / hTmpSiPM->Integral(kEmin, kEmax));
	hTmpPMT->Scale(hExpPMT->Integral(kEmin, kEmax) / hTmpPMT->Integral(kEmin, kEmax));
	hTmp->SetLineColor(kBlue);
	hTmpSiPM->SetLineColor(kBlue);
	hTmpPMT->SetLineColor(kBlue);
	ln.SetLineColor(kGreen);

	cv->cd(1);
	hTmp->Draw("hist");
	hExp->Draw("same");
	TLegend *lg = new TLegend(0.65, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp, "Exp", "lp");
	lg->AddEntry(hTmp, "MC", "l");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExp->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExp->GetMaximum());

	cv->cd(2);
	hTmpSiPM->Draw("hist");
	hExpSiPM->Draw("same");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExpSiPM->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExpSiPM->GetMaximum());

	cv->cd(3);
	hTmpPMT->Draw("hist");
	hExpPMT->Draw("same");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExpPMT->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExpPMT->GetMaximum());

	if (iSet) SetScale(xminSiPM, xminPMT);
}

void NaCoClass::ScanRndm(int nDiv, double sMin, double sMax, int iSet = 0)
{
	double xmin;
	double xminSiPM, xminPMT;
	TLatex txt;
	TLine ln;
	double ds;
	int nBins;
	double rLow, rUp;
	int i, j;
	double Scoef;
	double ESiPM, EPMT;
	double xLow, xUp;
	int imin;
	char str[256];
	
	// Erase old
	Erase("hScan");
	Erase("hScanSiPM");
	Erase("hScanPMT");
	Erase("hTmp");
	Erase("hTmpSiPM");
	Erase("hTmpPMT");
	Erase("fpol2");
	// Book histogramms
	ds = 0.5 * (sMax - sMin) / (nDiv - 1);
	TH1D *hScan = new TH1D("hScan", "#chi^{2} difference between MC and data, SiPM+PMT;Smearing;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanSiPM = new TH1D("hScanSiPM", "#chi^{2} difference between MC and data, SiPM;Smearing;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanPMT = new TH1D("hScanPMT", "#chi^{2} difference between MC and data, PMT;Smearing;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	nBins = hExp->GetXaxis()->GetNbins();
	rLow = hExp->GetXaxis()->GetBinLowEdge(1);
	rUp = hExp->GetXaxis()->GetBinUpEdge(nBins);
	sprintf(str, "SiPM+PMT %s;MeV", Name);
	TH1D *hTmp = new TH1D("hTmp", str, nBins, rLow, rUp);
	sprintf(str, "SiPM %s;MeV", Name);
	TH1D *hTmpSiPM = new TH1D("hTmpSiPM", str, nBins, rLow, rUp);
	sprintf(str, "PMT %s;MeV", Name);
	TH1D *hTmpPMT = new TH1D("hTmpPMT", str, nBins, rLow, rUp);
	TF1 *fpol2 = new TF1("fpol2", "pol2", sMin, sMax);
	// Fill with smeared MC
	for (i=0; i<nDiv; i++) {
		Scoef = sMin + 2 * ds * i;
		hTmp->Reset();
		hTmpSiPM->Reset();
		hTmpPMT->Reset();
		for (j=0; j<NMC; j++) {
			ESiPM = calcE(MCEnergy[j].SiPM, kScaleSiPM, Scoef, kSqrtSiPM);
			EPMT = calcE(MCEnergy[j].PMT, kScalePMT, Scoef, kSqrtPMT);
			hTmp->Fill((ESiPM + EPMT) / 2);
			hTmpSiPM->Fill(ESiPM);
			hTmpPMT->Fill(EPMT);
		}
		hTmp->Scale(hExp->Integral(kEmin, kEmax) / hTmp->Integral(kEmin, kEmax));
		hTmpSiPM->Scale(hExpSiPM->Integral(kEmin, kEmax) / hTmpSiPM->Integral(kEmin, kEmax));
		hTmpPMT->Scale(hExpPMT->Integral(kEmin, kEmax) / hTmpPMT->Integral(kEmin, kEmax));
		hScan->SetBinContent(i+1, chi2Diff(hExp, hTmp));
		hScanSiPM->SetBinContent(i+1, chi2Diff(hExpSiPM, hTmpSiPM));
		hScanPMT->SetBinContent(i+1, chi2Diff(hExpPMT, hTmpPMT));
		printf(">");
		fflush(stdout);
	}
	printf("\n");

	gStyle->SetOptStat(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	cv->Clear();
	cv->Divide(3, 2);
//		find minimums
	cv->cd(4);
	imin = hScan->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScan->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScan->GetXaxis()->GetBinCenter(imin + 5);
	hScan->Fit(fpol2, "", "", xLow, xUp);
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Smear=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.77, str);

	cv->cd(5);
	imin = hScanSiPM->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScanSiPM->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScanSiPM->GetXaxis()->GetBinCenter(imin + 5);
	hScanSiPM->Fit(fpol2, "", "", xLow, xUp);
	xminSiPM = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Smear=%5.3f", xminSiPM);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xminSiPM));
	txt.DrawLatexNDC(0.4, 0.77, str);
	sprintf(str, "Scale=%5.3f", kScaleSiPM);
	txt.DrawLatexNDC(0.4, 0.69, str);
	sprintf(str, "Stoch=%5.3f", kSqrtSiPM);
	txt.DrawLatexNDC(0.4, 0.61, str);

	cv->cd(6);
	imin = hScanPMT->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScanPMT->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScanPMT->GetXaxis()->GetBinCenter(imin + 5);
	hScanPMT->Fit(fpol2, "", "", xLow, xUp);
	xminPMT = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Smear=%5.3f", xminPMT);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xminPMT));
	txt.DrawLatexNDC(0.4, 0.77, str);
	sprintf(str, "Scale=%5.3f", kScalePMT);
	txt.DrawLatexNDC(0.4, 0.69, str);
	sprintf(str, "Stoch=%5.3f", kSqrtPMT);
	txt.DrawLatexNDC(0.4, 0.61, str);

//		Draw best fits
	hTmp->Reset();
	hTmpSiPM->Reset();
	hTmpPMT->Reset();
	for (j=0; j<NMC; j++) {
		ESiPM = calcE(MCEnergy[j].SiPM, kScaleSiPM, xminSiPM, kSqrtSiPM);
		EPMT = calcE(MCEnergy[j].PMT, kScalePMT, xminPMT, kSqrtPMT);
		hTmp->Fill((ESiPM + EPMT) / 2);
		hTmpSiPM->Fill(ESiPM);
		hTmpPMT->Fill(EPMT);
	}
	hTmp->Scale(hExp->Integral(kEmin, kEmax) / hTmp->Integral(kEmin, kEmax));
	hTmpSiPM->Scale(hExpSiPM->Integral(kEmin, kEmax) / hTmpSiPM->Integral(kEmin, kEmax));
	hTmpPMT->Scale(hExpPMT->Integral(kEmin, kEmax) / hTmpPMT->Integral(kEmin, kEmax));
	hTmp->SetLineColor(kBlue);
	hTmpSiPM->SetLineColor(kBlue);
	hTmpPMT->SetLineColor(kBlue);
	ln.SetLineColor(kGreen);

	cv->cd(1);
	hTmp->Draw("hist");
	hExp->Draw("same");
	TLegend *lg = new TLegend(0.65, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp, "Exp", "lp");
	lg->AddEntry(hTmp, "MC", "l");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExp->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExp->GetMaximum());

	cv->cd(2);
	hTmpSiPM->Draw("hist");
	hExpSiPM->Draw("same");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExpSiPM->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExpSiPM->GetMaximum());

	cv->cd(3);
	hTmpPMT->Draw("hist");
	hExpPMT->Draw("same");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExpPMT->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExpPMT->GetMaximum());

	if (iSet) SetRndm(xminSiPM, xminPMT);
}

void NaCoClass::ScanSqrt(int nDiv, double sMin, double sMax, int iSet = 0)
{
	double xmin;
	double xminSiPM, xminPMT;
	TLatex txt;
	TLine ln;
	double ds;
	int nBins;
	double rLow, rUp;
	int i, j;
	double Scoef;
	double ESiPM, EPMT;
	double xLow, xUp;
	int imin;
	char str[256];
	
	// Erase old
	Erase("hScan");
	Erase("hScanSiPM");
	Erase("hScanPMT");
	Erase("hTmp");
	Erase("hTmpSiPM");
	Erase("hTmpPMT");
	Erase("fpol2");
	// Book histogramms
	ds = 0.5 * (sMax - sMin) / (nDiv - 1);
	TH1D *hScan = new TH1D("hScan", "#chi^{2} difference between MC and data, SiPM+PMT;Stochastic;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanSiPM = new TH1D("hScanSiPM", "#chi^{2} difference between MC and data, SiPM;Stochastic;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanPMT = new TH1D("hScanPMT", "#chi^{2} difference between MC and data, PMT;Stochastic;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	nBins = hExp->GetXaxis()->GetNbins();
	rLow = hExp->GetXaxis()->GetBinLowEdge(1);
	rUp = hExp->GetXaxis()->GetBinUpEdge(nBins);
	sprintf(str, "SiPM+PMT %s;MeV", Name);
	TH1D *hTmp = new TH1D("hTmp", str, nBins, rLow, rUp);
	sprintf(str, "SiPM %s;MeV", Name);
	TH1D *hTmpSiPM = new TH1D("hTmpSiPM", str, nBins, rLow, rUp);
	sprintf(str, "PMT %s;MeV", Name);
	TH1D *hTmpPMT = new TH1D("hTmpPMT", str, nBins, rLow, rUp);
	TF1 *fpol2 = new TF1("fpol2", "pol2", sMin, sMax);
	// Fill with smeared MC
	for (i=0; i<nDiv; i++) {
		Scoef = sMin + 2 * ds * i;
		hTmp->Reset();
		hTmpSiPM->Reset();
		hTmpPMT->Reset();
		for (j=0; j<NMC; j++) {
			ESiPM = calcE(MCEnergy[j].SiPM, kScaleSiPM, kRndmSiPM, Scoef);
			EPMT = calcE(MCEnergy[j].PMT, kScalePMT, kRndmPMT, Scoef);
			hTmp->Fill((ESiPM + EPMT) / 2);
			hTmpSiPM->Fill(ESiPM);
			hTmpPMT->Fill(EPMT);
		}
		hTmp->Scale(hExp->Integral(kEmin, kEmax) / hTmp->Integral(kEmin, kEmax));
		hTmpSiPM->Scale(hExpSiPM->Integral(kEmin, kEmax) / hTmpSiPM->Integral(kEmin, kEmax));
		hTmpPMT->Scale(hExpPMT->Integral(kEmin, kEmax) / hTmpPMT->Integral(kEmin, kEmax));
		hScan->SetBinContent(i+1, chi2Diff(hExp, hTmp));
		hScanSiPM->SetBinContent(i+1, chi2Diff(hExpSiPM, hTmpSiPM));
		hScanPMT->SetBinContent(i+1, chi2Diff(hExpPMT, hTmpPMT));
		printf(">");
		fflush(stdout);
	}
	printf("\n");

	gStyle->SetOptStat(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	cv->Clear();
	cv->Divide(3, 2);
//		find minimums
	cv->cd(4);
	imin = hScan->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScan->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScan->GetXaxis()->GetBinCenter(imin + 5);
	hScan->Fit(fpol2, "", "", xLow, xUp);
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Stoch=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.77, str);

	cv->cd(5);
	imin = hScanSiPM->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScanSiPM->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScanSiPM->GetXaxis()->GetBinCenter(imin + 5);
	hScanSiPM->Fit(fpol2, "", "", xLow, xUp);
	xminSiPM = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Stoch=%5.3f", xminSiPM);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xminSiPM));
	txt.DrawLatexNDC(0.4, 0.77, str);
	sprintf(str, "Scale=%5.3f", kScaleSiPM);
	txt.DrawLatexNDC(0.4, 0.69, str);
	sprintf(str, "Smear=%5.3f", kRndmSiPM);
	txt.DrawLatexNDC(0.4, 0.61, str);

	cv->cd(6);
	imin = hScanPMT->GetMinimumBin();
	if (imin < 6) imin = 6;
	if (imin > nDiv - 5) imin = nDiv - 5;
	xLow = hScanPMT->GetXaxis()->GetBinCenter(imin - 5);
	xUp = hScanPMT->GetXaxis()->GetBinCenter(imin + 5);
	hScanPMT->Fit(fpol2, "", "", xLow, xUp);
	xminPMT = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Stoch=%5.3f", xminPMT);
	txt.DrawLatexNDC(0.4, 0.85, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xminPMT));
	txt.DrawLatexNDC(0.4, 0.77, str);
	sprintf(str, "Scale=%5.3f", kScalePMT);
	txt.DrawLatexNDC(0.4, 0.69, str);
	sprintf(str, "Smear=%5.3f", kRndmPMT);
	txt.DrawLatexNDC(0.4, 0.61, str);

//		Draw best fits
	hTmp->Reset();
	hTmpSiPM->Reset();
	hTmpPMT->Reset();
	for (j=0; j<NMC; j++) {
		ESiPM = calcE(MCEnergy[j].SiPM, kScaleSiPM, kRndmSiPM, xminSiPM);
		EPMT = calcE(MCEnergy[j].PMT, kScalePMT, kRndmPMT, xminPMT);
		hTmp->Fill((ESiPM + EPMT) / 2);
		hTmpSiPM->Fill(ESiPM);
		hTmpPMT->Fill(EPMT);
	}
	hTmp->Scale(hExp->Integral(kEmin, kEmax) / hTmp->Integral(kEmin, kEmax));
	hTmpSiPM->Scale(hExpSiPM->Integral(kEmin, kEmax) / hTmpSiPM->Integral(kEmin, kEmax));
	hTmpPMT->Scale(hExpPMT->Integral(kEmin, kEmax) / hTmpPMT->Integral(kEmin, kEmax));
	hTmp->SetLineColor(kBlue);
	hTmpSiPM->SetLineColor(kBlue);
	hTmpPMT->SetLineColor(kBlue);
	ln.SetLineColor(kGreen);

	cv->cd(1);
	hTmp->Draw("hist");
	hExp->Draw("same");
	TLegend *lg = new TLegend(0.65, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp, "Exp", "lp");
	lg->AddEntry(hTmp, "MC", "l");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExp->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExp->GetMaximum());

	cv->cd(2);
	hTmpSiPM->Draw("hist");
	hExpSiPM->Draw("same");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExpSiPM->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExpSiPM->GetMaximum());

	cv->cd(3);
	hTmpPMT->Draw("hist");
	hExpPMT->Draw("same");
	lg->Draw();
	ln.DrawLine(Emin, 0, Emin, 0.7 * hExpPMT->GetMaximum());
	ln.DrawLine(Emax, 0, Emax, 0.7 * hExpPMT->GetMaximum());

	if (iSet) SetSqrt(xminSiPM, xminPMT);
}

//============================================================//
NaCoClass *makeNa(int iRun, int iMC)
{
	char str[256];
	char mclong[2048];
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/root8n7/MC/Fuso/22Na";
	const char *mcfname = "mc_22Na_indLY_transcode_rawProc_pedSim_Center1.root";
	const char *MC[] = {
		"Full_decay_center_Fuso",
		"Full_decay_center_Fuso_Birks_0_005",
		"Full_decay_center_Fuso_Cher_coeff_0_05",
		"Full_decay_center_Fuso_paint_0_2",
		"Full_decay_center_Fuso_paint_0_3",
		"Full_decay_center_Fuso_PMT_strip_map"
	};
	const char *MCshort[] = {
		"Fuso",
		"Fuso_Birks_0.005",
		"Fuso_Cher_0.05",
		"Fuso_paint_0_2",
		"Fuso_paint_0.3",
		"Fuso_PMT_strip_map"
	};
	const char *Run[] = {
		"22Na_feb17_center_root8n7_R30.0.root",
		"22Na_nov18_center_root8n7_R30.0.root",
		"22Na_jun22_center_root8n7_R30.0.root"
	};
	const char *RunShort[] = {
		"feb17", "nov18", "jun22"
	};
	sprintf(mclong, "%s/%s/%s", mcdir, MC[iMC], mcfname);
	sprintf(str, "^{22}Na %s %s", RunShort[iRun], MCshort[iMC]);
	NaCoClass *src = new NaCoClass(Run[iRun], mclong, str);
	return src;
}

//============================================================//
NaCoClass *makeCo(int iRun, int iMC)
{
	char str[256];
	char mclong[2048];
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/root8n7/MC/Fuso/60Co";
	const char *mcfname = "mc_60Co_indLY_transcode_rawProc_pedSim_Center1.root";
	const char *MC[] = {
		"Center_Fuso",
		"Center_Fuso_Birks_0_005",
		"Center_Fuso_Cher_coeff_0_05",
		"Center_Fuso_paint_0_2",
		"Center_Fuso_paint_0_3",
		"Center_Fuso_PMT_strip_map"
	};
	const char *MCshort[] = {
		"Fuso",
		"Fuso_Birks_0.005",
		"Fuso_Cher_0.05",
		"Fuso_paint_0_2",
		"Fuso_paint_0.3",
		"Fuso_PMT_strip_map"
	};
	const char *Run[] = {
		"60Co_feb17_center_root8n7_R30.0.root",
		"60Co_nov18_center_root8n7_R30.0.root",
		"60Co_jun22_center_root8n7_R30.0.root",
		"60Co_apr25_center_root8n7_R30.0.root"
	};
	const char *RunShort[] = {
		"feb17Co", "nov18Co", "jun22Co", "apr25Co"
	};
	sprintf(mclong, "%s/%s/%s", mcdir, MC[iMC], mcfname);
	sprintf(str, "^{60}Co %s %s", RunShort[iRun], MCshort[iMC]);
	NaCoClass *src = new NaCoClass(Run[iRun], mclong, str);
	return src;
}

//============================================================//
NaCoClass *makeNaNoise(int iRun)
{
	char str[256];
	char mclong[2048];
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/root8n7/MC/FusoWithNoise/22Na";
	const char *mcfname = "mc_22Na_indLY_transcode_rawProc_pedSim_Center1.root";
	const char *MC = "Full_decay_center_Fuso";
	const char *MCshort = "FusoWithNoise";
	const char *Run[] = {
		"22Na_feb17_center_root8n7_R30.0.root",
		"22Na_nov18_center_root8n7_R30.0.root",
		"22Na_jun22_center_root8n7_R30.0.root"
	};
	const char *RunShort[] = {
		"feb17", "nov18", "jun22"
	};
	sprintf(mclong, "%s/%s/%s", mcdir, MC, mcfname);
	sprintf(str, "^{22}Na %s %s", RunShort[iRun], MCshort);
	NaCoClass *src = new NaCoClass(Run[iRun], mclong, str);
	return src;
}

//============================================================//
NaCoClass *makeCoNoise(int iRun)
{
	char str[256];
	char mclong[2048];
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/root8n7/MC/FusoWithNoise/60Co";
	const char *mcfname = "mc_60Co_indLY_transcode_rawProc_pedSim_Center1.root";
	const char *MC = "Center_Fuso";
	const char *MCshort = "FusoWithNoise";
	const char *Run[] = {
		"60Co_feb17_center_root8n7_R30.0.root",
		"60Co_nov18_center_root8n7_R30.0.root",
		"60Co_jun22_center_root8n7_R30.0.root",
		"60Co_apr25_center_root8n7_R30.0.root"
	};
	const char *RunShort[] = {
		"feb17Co", "nov18Co", "jun22Co", "apr25Co"
	};
	sprintf(mclong, "%s/%s/%s", mcdir, MC, mcfname);
	sprintf(str, "^{60}Co %s %s", RunShort[iRun], MCshort);
	NaCoClass *src = new NaCoClass(Run[iRun], mclong, str);
	return src;
}

void naco_class_v0(const char *pdfname)
{
	int i, j;
	NaCoClass *src;
	
	gStyle->SetOptStat(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	TString pdf(pdfname);
	cv->SaveAs((pdf+"[").Data());
	// Na
	for (i=0; i<6; i++) for (j=0; j<3; j++) {
		src = makeNa(j, i);
		if (!src) {
			printf("Fatal error !\n");
			goto fin;
		}
		src->SetErange(0.8, 2.9);
		src->SetSqrt(0.07, 0.05);
		if (i<5) {
			src->SetRndm(0.1, 0.05);
		} else {
			src->SetRndm(0.1, 0.02);
		}
		src->ScanScale(20, 0.85, 1.05, 1);
		cv->SaveAs(pdf.Data());
		src->ScanRndm(20, 0.02, 0.12);
		cv->SaveAs(pdf.Data());
		delete src;
	}
	// Co
	for (i=0; i<6; i++) for (j=0; j<4; j++) {
		src = makeCo(j, i);
		if (!src) {
			printf("Fatal error !\n");
			goto fin;
		}
		src->SetErange(0.8, 3.1);
		src->SetSqrt(0.05, 0.05);
		if (i<5) {
			src->SetRndm(0.115, 0.069);
		} else {
			src->SetRndm(0.115, 0.01);
		}
		src->ScanScale(20, 0.85, 1.05, 1);
		cv->SaveAs(pdf.Data());
		src->ScanRndm(30, 0.0, 0.15);
		cv->SaveAs(pdf.Data());
		delete src;
	}
fin:
	cv->SaveAs((pdf+"]").Data());
}

void naco_class(const char *pdfname)
{
	int i, j;
	NaCoClass *src;
	
	gStyle->SetOptStat(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	TString pdf(pdfname);
	cv->SaveAs((pdf+"[").Data());
	// Na
	for (j=0; j<3; j++) {
		src = makeNaNoise(j);
		if (!src) {
			printf("Fatal error !\n");
			goto fin;
		}
		src->SetErange(0.8, 2.9);
		src->SetSqrt(0.07, 0.05);
		if (i<5) {
			src->SetRndm(0.1, 0.05);
		} else {
			src->SetRndm(0.1, 0.02);
		}
		src->ScanScale(20, 0.85, 1.05, 1);
		cv->SaveAs(pdf.Data());
		src->ScanRndm(20, 0.02, 0.12);
		cv->SaveAs(pdf.Data());
		delete src;
	}
	// Co
	for (j=0; j<4; j++) {
		src = makeCoNoise(j);
		if (!src) {
			printf("Fatal error !\n");
			goto fin;
		}
		src->SetErange(0.8, 3.1);
		src->SetSqrt(0.05, 0.05);
		if (i<5) {
			src->SetRndm(0.115, 0.069);
		} else {
			src->SetRndm(0.115, 0.01);
		}
		src->ScanScale(20, 0.85, 1.05, 1);
		cv->SaveAs(pdf.Data());
		src->ScanRndm(30, 0.0, 0.15);
		cv->SaveAs(pdf.Data());
		delete src;
	}
fin:
	cv->SaveAs((pdf+"]").Data());
}
