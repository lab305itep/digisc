double chi2calc(const char *exp, const char *mc, double lowEdge, double highEdge, int *NDF)
{
	int iLow, iHigh;
	
	TFile *fExp = new TFile(exp);
	TFile *fMc = new TFile(mc);
	if (!fExp->IsOpen() || !fMc->IsOpen()) return -1;
	
	TH1 *hExp = (TH1*) fExp->Get("hExp12B");
	TH1 *hMc =  (TH1*) fMc->Get("hMC12B");
	if (!hExp || !hMc) return -1;
	
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

void draw_chi2src(double lowEdge = 4.0, double highEdge = 10.0)
{
	char strExp[1024];
	char strMc[1024];
	char strs[128];
	char strl[1024];
	int iWhat;
	int i, j;
	double val;
	int NDF;
	
	TH2D *hScan = new TH2D("hScan12B", "Scan", 26, 0.8775, 1.0075, 17, -0.01, 0.33);
	for (i=0; i<26; i++) for (j=0; j<17; j++) {
		sprintf(strExp, "Danss_12B_scale_%5.3f.root", 0.88 + 0.005*i);
		sprintf(strMc, "MC_12B_rndm_%4.2f.root", 0.02*j);
		val = chi2calc(strExp, strMc, lowEdge, highEdge, &NDF);
		hScan->SetBinContent(i + 1, j + 1, val);
	}
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	sprintf(strl, "12B: #chi^{2} of Exp to MC ratio in %3.1f-%3.1f MeV, NDF=%d;Scale;Resolution", lowEdge, highEdge, NDF);
	hScan->SetTitle(strl);
	TCanvas *cv = new TCanvas("CV", "CV", 800, 600);
	cv->SetLogz(1);
	hScan->Draw("colorz");
	cv->SaveAs("scale-width-scan-12B.png");
}

void draw_src2mc(double scale, double rndm, double lowEdge = 4.0, double highEdge = 10.0)
{
	int i;
	char strExp[1024];
	char strMc[1024];
	char fname[1024];
	TText txt;
	char str[128];
	int iLow, iHigh;

	sprintf(strExp, "Danss_12B_scale_%5.3f.root", scale);
	sprintf(strMc, "MC_12B_rndm_%4.2f.root", rndm);
	sprintf(fname, "12B_scale_%5.3f_rndm_%4.2f.png", scale, rndm);

	gStyle->SetOptStat(0);

	TFile *fExp = new TFile(strExp);
	TFile *fMc = new TFile(strMc);
	if (!fExp->IsOpen() || !fMc->IsOpen()) return;
	
	TH1 *hExp = (TH1*) fExp->Get("hExp12B");
	TH1 *hMc =  (TH1*) fMc->Get("hMC12B");
	if (!hExp || !hMc) return -1;

	hExp->SetLineColor(kRed);
	hExp->SetLineWidth(2);
	hExp->SetMarkerStyle(kFullCircle);
	hExp->SetMarkerColor(kRed);
	hMc->SetLineWidth(2);

	iLow = hExp->FindBin(lowEdge+0.0001);
	iHigh = hExp->FindBin(highEdge-0.0001);
	hMc->Scale(hExp->Integral(iLow, iHigh) / hMc->Integral(iLow, iHigh));
	
	TLegend *lg = new TLegend(0.65, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp, "Experiment", "pe");
	lg->AddEntry(hMc, "Monte Carlo", "l");
	
	TCanvas *cv = new TCanvas("CV", "Exp versus MC", 1200, 900);
	hExp->Draw("e");
	hMc->Draw("hist,same");
	sprintf(str, "Scale = %5.3f", scale);
	txt.DrawTextNDC(0.65, 0.55, str);
	sprintf(str, "Rndm = %4.2f", rndm);
	txt.DrawTextNDC(0.65, 0.48, str);
	lg->Draw();
	
	cv->SaveAs(fname);
}

void chi2src_12B(void)
{
	draw_chi2src(4.5, 11);
	draw_src2mc(0.945, 0.18, 4.5, 11);
}
