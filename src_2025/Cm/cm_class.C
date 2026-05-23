#include "../../evtbuilder.h"
/**********************************************************
	Process data and MC for 248Cm measurements
248Cm runs:
Position  Date   Runs
center    mar17  14428-14485
center    nov18  50578-50647
center    jun22  127720-127772
center    may25  175471-175528
edge      mar17  14487-14512
edge      nov18  50875-50947
***********************************************************/
struct EnergyStruct {
	double SiPM;
	double PMT;
};

class CmClass {
private:
	int kEmin;	// Energy range
	int kEmax;
	double Emin;
	double Emax;
	int NMin;	// Neutron range
	int NMax;
	TRandom *rndm;
	TChain *tMC;
	TH1D *hExp;
	TH1D *hExpSiPM;
	TH1D *hExpPMT;
	TFile *fExp;
	struct EnergyStruct *MCEnergy;
	int NMC;
	struct DanssCmStruct MCdata;
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
	CmClass(const char *expname, const char *mcname, const char *name);
	~CmClass(void);
	void SetErange(int nMin, int nMax);
	void SetErange(double rMin, double rMax);
	inline void SetScale(double kSiPM, double kPMT) { kScaleSiPM = kSiPM; kScalePMT = kPMT; };
	inline void SetRndm(double kSiPM, double kPMT) { kRndmSiPM = kSiPM; kRndmPMT = kPMT; };
	inline void SetSqrt(double kSiPM, double kPMT) { kSqrtSiPM = kSiPM; kSqrtPMT = kPMT; };
	void ScanScale(int nDiv, double sMin, double sMax, int iSet);
	void ScanRndm(int nDiv, double sMin, double sMax, int iSet);
	void ScanSqrt(int nDiv, double sMin, double sMax, int iSet);
};

CmClass::CmClass(const char *expname, const char *mcname, const char *name)
{
	printf("Making class for Run=%s and MC=%s\n", expname, mcname);
	hExp = hExpSiPM = hExpPMT = NULL;
	fExp = NULL;
	tMC = NULL;
	Name = strdup(name);
	MCEnergy = NULL;
	NMC = 0;
	NMin = 1;
	NMax = 6;
	kScaleSiPM = kScalePMT = 1.0;
	kRndmSiPM = kRndmPMT = 0.08;
	kSqrtSiPM = kSqrtPMT = 0.04;
	rndm = new TRandom2(time(NULL));
	
	fExp = new TFile(expname);
	if (!fExp->IsOpen()) return;
	hExp = (TH1D *) fExp->Get("hCm");
	hExpSiPM = (TH1D *) fExp->Get("hCmSiPM");
	hExpPMT = (TH1D *) fExp->Get("hCmPMT");
	if (!hExp || !hExpSiPM || !hExpPMT) {
		printf("Not all histograms found in %s.\n", expname);
		return;
	}
	hExp->SetLineColor(kRed);
	hExp->SetMarkerColor(kRed);
	hExp->SetMarkerStyle(kFullCircle);
	hExp->SetMarkerSize(0.03);
	hExpSiPM->SetLineColor(kRed);
	hExpSiPM->SetMarkerColor(kRed);
	hExpSiPM->SetMarkerStyle(kFullCircle);
	hExpSiPM->SetMarkerSize(0.03);
	hExpPMT->SetLineColor(kRed);
	hExpPMT->SetMarkerColor(kRed);
	hExpPMT->SetMarkerStyle(kFullCircle);
	hExpPMT->SetMarkerSize(0.03);

	tMC = create_chain(mcname, "DanssCm");
	if (!tMC) {
		printf("Can not open MC: %s\n", mcname);
		return;
	}
	tMC->SetBranchAddress("Cm", &MCdata);
	MakeMCArray();
	SetErange(16, 95);
}

CmClass::~CmClass(void)
{
	if (MCEnergy) free(MCEnergy);
	if (fExp && fExp->IsOpen()) delete fExp;
	if (tMC) delete tMC;
	if (Name) free(Name);
	delete rndm;
}

void CmClass::Erase(const char *name)
{
	TObject *obj = gROOT->FindObject(name);
	if (obj) delete obj;
}

void CmClass::MakeMCArray(void)
{
	int N;
	int i, j;
	
	N = tMC->GetEntries();
	if (MCEnergy) free(MCEnergy);
	MCEnergy = (struct EnergyStruct *) malloc(N * (NMax - NMin + 1) * sizeof(struct EnergyStruct));	// Maximum possible
	if (!MCEnergy) {
		printf("Can not allocate memory for %d events\n", N);
		return;
	}
	NMC = 0;
	for (i=0; i<N; i++) {
		tMC->GetEntry(i);
		for (j = NMin; j <= NMax && j < MCdata.N; j++) { 
			// Cut: "N>j && gtDiff[j]/125<50 && gtDiff[j]/125>2 && PmtCleanEnergy[j]*scale > 0.7"
			if (MCdata.gtDiff[j]/125 <= 2 || MCdata.gtDiff[j]/125 >= 50) continue;
			if (MCdata.PmtCleanEnergy[j] <= 0.7) continue;
			MCEnergy[NMC].SiPM = MCdata.SiPmCleanEnergy[j];
			MCEnergy[NMC].PMT = MCdata.PmtCleanEnergy[j];
			NMC++;
		}
	}
	printf("MC: %d neutrons found in %d fissions\n", NMC, N);
}

//	Calculate chi^2 difference of two histograms
//  hcm, hMc - 1-dim histograms
double CmClass::chi2Diff(TH1 *hcm, TH1 *hMc)
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
TChain *CmClass::create_chain(const char *fname, const char *chname)
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
double CmClass::calcE(double E, double kScl, double kRnd, double kSqr)
{
	double X = kScl * E;
	double S = sqrt(kRnd * kRnd * X * X + kSqr * kSqr * X);
	return rndm->Gaus(X, S);
}

//	Set range for histogram compare
//	nMin, nMax - bin numbers
void CmClass::SetErange(int nMin, int nMax)
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
void CmClass::SetErange(double rMin, double rMax)
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

void CmClass::ScanScale(int nDiv, double sMin, double sMax, int iSet = 0)
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
	Erase("hScan248Cm");
	Erase("hScan248CmSiPM");
	Erase("hScan248CmPMT");
	Erase("hTmp");
	Erase("hTmpSiPM");
	Erase("hTmpPMT");
	Erase("fpol2");
	// Book histogramms
	ds = 0.5 * (sMax - sMin) / (nDiv - 1);
	TH1D *hScan = new TH1D("hScan248Cm", "#chi^{2} difference between MC and data, ^{248}Cm, SiPM+PMT;Scale;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanSiPM = new TH1D("hScan248CmSiPM", "#chi^{2} difference between MC and data, ^{248}Cm, SiPM;Scale;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanPMT = new TH1D("hScan248CmPMT", "#chi^{2} difference between MC and data, ^{248}Cm, PMT;Scale;#chi^{2}", nDiv, sMin - ds, sMax + ds);
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

void CmClass::ScanRndm(int nDiv, double sMin, double sMax, int iSet = 0)
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
	Erase("hScan248Cm");
	Erase("hScan248CmSiPM");
	Erase("hScan248CmPMT");
	Erase("hTmp");
	Erase("hTmpSiPM");
	Erase("hTmpPMT");
	Erase("fpol2");
	// Book histogramms
	ds = 0.5 * (sMax - sMin) / (nDiv - 1);
	TH1D *hScan = new TH1D("hScan248Cm", "#chi^{2} difference between MC and data, ^{248}Cm, SiPM+PMT;Smearing;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanSiPM = new TH1D("hScan248CmSiPM", "#chi^{2} difference between MC and data, ^{248}Cm, SiPM;Smearing;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanPMT = new TH1D("hScan248CmPMT", "#chi^{2} difference between MC and data, ^{248}Cm, PMT;Smearing;#chi^{2}", nDiv, sMin - ds, sMax + ds);
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

void CmClass::ScanSqrt(int nDiv, double sMin, double sMax, int iSet = 0)
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
	Erase("hScan248Cm");
	Erase("hScan248CmSiPM");
	Erase("hScan248CmPMT");
	Erase("hTmp");
	Erase("hTmpSiPM");
	Erase("hTmpPMT");
	Erase("fpol2");
	// Book histogramms
	ds = 0.5 * (sMax - sMin) / (nDiv - 1);
	TH1D *hScan = new TH1D("hScan248Cm", "#chi^{2} difference between MC and data, ^{248}Cm, SiPM+PMT;Stochastic;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanSiPM = new TH1D("hScan248CmSiPM", "#chi^{2} difference between MC and data, ^{248}Cm, SiPM;Stochastic;#chi^{2}", nDiv, sMin - ds, sMax + ds);
	TH1D *hScanPMT = new TH1D("hScan248CmPMT", "#chi^{2} difference between MC and data, ^{248}Cm, PMT;Stochastic;#chi^{2}", nDiv, sMin - ds, sMax + ds);
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
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xminSiPM));
	txt.DrawLatexNDC(0.4, 0.72, str);
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
	sprintf(str, "Scale=%5.3f", kScaleSiPM);
	txt.DrawLatexNDC(0.4, 0.69, str);
	sprintf(str, "Smear=%5.3f", kRndmSiPM);
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
CmClass *makeCm(int iRun, int iMC)
{
	char str[256];
	const char *MC[] = {
		"cm_MC_8.7_Center_Fuso.root",
		"cm_MC_8.7_Center_Fuso_Birks_0_005.root",
		"cm_MC_8.7_Center_Fuso_Cher_coeff_0_05.root",
		"cm_MC_8.7_Center_Fuso_paint_0_2.root",
		"cm_MC_8.7_Center_Fuso_paint_0_3.root",
		"cm_MC_8.7_Center_Fuso_PMT_strip_map.root"
	};
	const char *MCshort[] = {
		"Fuso",
		"Fuso_Birks=0.005",
		"Fuso_Cher_0.05",
		"Fuso_paint=0_2",
		"Fuso_paint=0.3",
		"Fuso_PMT_strip_map"
	};
	const char *Run[] = {
		"cm_14428_14485_8.7.hist.root",
		"cm_50578_50647_8.7.hist.root",
		"cm_127720_127772_8.7.hist.root",
		"cm_175471_175528_8.7.hist.root"
	};
	const char *RunShort[] = {
		"mar17", "nov18", "jun22", "may25"
	};
	sprintf(str, "%s %s", RunShort[iRun], MCshort[iMC]);
	CmClass *cm = new CmClass(Run[iRun], MC[iMC], str);
	return cm;
}


void cm_class(const char *pdfname)
{
	int i, j;
	CmClass *cm;
	
	gStyle->SetOptStat(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	TString pdf(pdfname);
	cv->SaveAs((pdf+"[").Data());
	for (i=0; i<6; i++) for (j=0; j<4; j++) {
		cm = makeCm(j, i);
		if (!cm) {
			printf("Fatal error !\n");
			goto fin;
		}
		cm->SetErange(5.5, 9.0);
		cm->SetSqrt(0.04, 0.04);
		if (i<5) {
			cm->SetRndm(0.08, 0.07);
		} else {
			cm->SetRndm(0.08, 0.05);
		}
		cm->ScanScale(20, 0.87, 1.07, 1);
		cm->ScanRndm(15, 0.04, 0.115);
		cv->SaveAs(pdf.Data());
		delete cm;
	}
fin:
	cv->SaveAs((pdf+"]").Data());
}
