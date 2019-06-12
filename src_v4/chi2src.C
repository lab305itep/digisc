double chi2calc(const char *exp, const char *mc, const char *texp, const char *tmc, double lowEdge, double highEdge, int *NDF)
{
	int nExp, nMc;
	int iLow, iHigh;
	TFile *fExp = new TFile(exp);
	TFile *fMc = new TFile(mc);
	if (!fExp->IsOpen() || !fMc->IsOpen()) return -1;
	
	TH1 *hExp = (TH1*) fExp->Get(texp);
	TH1 *hMc =  (TH1*) fMc->Get(tmc);;
	if (!hExp || !hMc) return -1;
	
	nExp = hExp->GetXaxis()->GetNbins();
	nMc = hMc->GetXaxis()->GetNbins();
	
	if (nMc == 2*nExp) {
		hMc->Rebin(2);
	} else if (2*nMc == nExp) {
		hExp->Rebin(2);
	} else if(nMc != nExp) {
		printf("Can not equalize number of bins: %d(Exp) <-> %d(MC)\n", nExp, nMc);
		return -1;
	}
	iLow = hExp->FindBin(lowEdge+0.0001);
	iHigh = hExp->FindBin(highEdge-0.0001);
	hExp->Add(hMc, -hExp->Integral(iLow, iHigh) / hMc->Integral(iLow, iHigh));
	TF1 fLevel("fPol0", "pol0", 0, 100);
	hExp->Fit("fPol0", "Q0", "", lowEdge, highEdge);
	double res = fLevel.GetChisquare();
	*NDF = fLevel.GetNDF();
	fExp->Close();
	fMc->Close();
	return res;
}

void draw_chi2src(const char *what, const char *src, double lowEdge = 1.0, double highEdge = 3.5)
{
	const char *Name[] = {"All", "SiPm", "Pmt"};
	const char *expHist[] = {"hExpC", "hExpSiPMC", "hExpPMTC"};
	const char *mcHist[] =  {"hMc",   "hMcSiPM",   "hMcPMT"};
	char strExp[1024];
	char strMc[1024];
	char strs[128];
	char strl[1024];
	int iWhat;
	int i, j;
	double val;
	int NDF;
	
	sprintf(strs, "hScan%s%s", src, what);
	TH2D *hScan = new TH2D(strs, "Scan", 26, 0.8775, 1.0075, 17, -0.01, 0.33);
	for (iWhat = 0; iWhat < sizeof(Name) / sizeof(Name[0]); iWhat++) if (!strcasecmp(what, Name[iWhat])) break;
	if (iWhat == sizeof(Name) / sizeof(Name[0])) {
		printf("%s not found.\n", what);
		return;
	}
	for (i=0; i<26; i++) for (j=0; j<17; j++) {
		sprintf(strExp, "%s_nov18_center_%5.3f.root", src, 0.88 + 0.005*i);
		sprintf(strMc, "%s_MC_center_rndm_%4.2f.root", src, 0.02*j);
		val = chi2calc(strExp, strMc, expHist[iWhat], mcHist[iWhat], lowEdge, highEdge, &NDF);
		hScan->SetBinContent(i + 1, j + 1, val);
	}
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	sprintf(strl, "%s %s: #chi^{2} of Exp to MC ratio in %3.1f-%3.1f MeV, NDF=%d;Scale;Resolution", src, what, lowEdge, highEdge, NDF);
	hScan->SetTitle(strl);
	TCanvas *cv = new TCanvas("CV", "CV", 800, 600);
	cv->SetLogz(1);
	hScan->Draw("colorz");
	sprintf(strl, "scale-width-scan-%s-%s.png", what, src);
	cv->SaveAs(strl);
}

void draw_src2mc(const char *src, double scale, double rndm)
{
	int i;
	const char *expHists[4] = {"hExpC", "hExpSiPMC", "hExpPMTC", "hHitsC"};
	const char *mcHists[4] = {"hMc", "hMcSiPM", "hMcPMT", "hMcHits"};
	char exp[1024];
	char mc[1024];
	char fname[1024];
	TText txt;
	char str[128];
	
	sprintf(exp, "%s_nov18_center_%5.3f.root", src, scale);
	sprintf(mc, "%s_MC_center_rndm_%4.2f.root", src, rndm);
	sprintf(fname, "%s_center_scale_%5.3f_rndm_%4.2f.png", src, scale, rndm);

	TFile *fExp = new TFile(exp);
	TFile *fMc = new TFile(mc);
	if (!fExp->IsOpen() || !fMc->IsOpen()) return;
	
	gStyle->SetOptStat(0);
	
	TH1 *hExp[4];
	TH1 *hMc[4];
	
	for (i=0; i<4; i++) {
		hExp[i] = (TH1 *) fExp->Get(expHists[i]);
		hMc[i] = (TH1 *) fMc->Get(mcHists[i]);
		if (!hExp[i] || !hMc[i]) {
			printf("i=%d\n", i);
			return;
		}
		hExp[i]->SetMarkerColor(kBlue);
		hExp[i]->SetLineColor(kBlue);
		hMc[i]->SetLineColor(kBlack);
		hMc[i]->Scale(hExp[i]->Integral() * hMc[i]->GetNbinsX() / (hMc[i]->Integral() * hExp[i]->GetNbinsX()));
	}
	
	TLegend *lg = new TLegend(0.65, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp[3], "Experiment", "l");
	lg->AddEntry(hMc[3], "Monte Carlo", "l");
	
	TCanvas *cv = new TCanvas("CV", "Exp versus MC", 1200, 900);
	cv->Divide(2, 2);
	for (i=0; i<4; i++) {
		cv->cd(i+1);
		hExp[i]->Draw("e");
		hMc[i]->Draw("hist,same");
		if (i == 0) {
			sprintf(str, "Scale = %5.3f", scale);
			txt.DrawTextNDC(0.65, 0.55, str);
			sprintf(str, "Rndm = %4.2f", rndm);
			txt.DrawTextNDC(0.65, 0.48, str);
		}
		if (i == 3) lg->Draw();
	}
	cv->SaveAs(fname);
}

void chi2src(void)
{
	draw_chi2src("ALL",  "22Na", 1.3, 2.5);
	draw_chi2src("SiPM", "22Na", 1,   2.8);
	draw_chi2src("PMT",  "22Na", 1.1, 2.6);
	draw_chi2src("ALL",  "60Co", 1.4, 3);
	draw_chi2src("SiPM", "60Co", 1.2, 3.2);
	draw_chi2src("PMT",  "60Co", 1.4, 3);
	
	draw_src2mc("22Na", 0.965, 0.16);
	draw_src2mc("60Co", 0.950, 0.16);
}
