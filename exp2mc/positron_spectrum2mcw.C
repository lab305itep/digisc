#include <TRandom2.h>
TRandom2 rnd;

class MyRandom {
    public:
	inline MyRandom(void) {;};
	inline ~MyRandom(void) {;};
	static inline double Gaus(double mean = 0, double sigma = 1) 
	{
		return rnd.Gaus(mean, sigma);
	};
	static inline double GausAdd(double val, double sigma)
	{
		return rnd.Gaus(val, sqrt(val)*sigma);
	};
};

double CalculateScale(TH1 *hA, TH1 *hB, int iMin, int iMax)
{
	return  hA->Integral(iMin, iMax) / hB->Integral(iMin, iMax);
}

void positron_spectrum2mcw(int RangeMin, int RangeMax, int NormMin, int NormMax, float norm = 1.0, double sigma = 0.17)
{
	const double reno[20] = {
		1.00, 1.02, 0.995, 1.00, 1.00, 1.015, 1.005, 0.98,  1.02, 1.055,
		1.09, 1.12, 1.13,  1.12, 1.06, 1.05,  1.04,  1.005, 0.95, 0.98
	};
	const char fuel[4][6] = {"235U", "238U", "239Pu", "241Pu"};
	const Color_t fColor[4] = {kRed, kBlue, kGreen, kOrange};
//	const float crossection[4] = {6.39, 8.90, 4.18, 5.76};	// from Sinev
	const float crossection[4] = {6.69, 10.10, 4.36, 6.04};	// from H-M as quoted by DB 1707.07728
//	const char mcfile[4][128] = {
//		"/space/danss_root3/withdead-uncorr/mc_positron_235U_simple_newScale.root",
//		"/space/danss_root3/withdead-uncorr/mc_positron_238U_simple_newScale.root",
//		"/space/danss_root3/withdead-uncorr/mc_positron_239Pu_simple_newScale.root",
//		"/space/danss_root3/withdead-uncorr/mc_positron_241Pu_simple_newScale.root"
//	};
	const char mcfile[4][128] = {
		"mc_ibdNewGd_235U_transcode_pair.root",
		"mc_ibdNewGd_238U_transcode_pair.root",
		"mc_ibdNewGd_239Pu_transcode_pair.root",
		"mc_ibdNewGd_241Pu_transcode_pair.root"
	};
//	From Sinev
//	const double fuelmix[3][4]  = {{0.69, 0.07, 0.21, 0.03}, {0.58, 0.07, 0.30, 0.05}, {0.47, 0.07, 0.39, 0.07}};
//	From Khvatov
//	const double fuelmix[3][4]  = {{0.78, 0.07, 0.12, 0.03}, {0.69, 0.07, 0.19, 0.05}, {0.59, 0.07, 0.26, 0.07}};
//		Replace end with 2 month before the end and begin with 5th campaign begin - our calculations
//	const double fuelmix[3][4]  = {{0.795, 0.07, 0.111, 0.024}, {0.69, 0.07, 0.19, 0.05}, {0.613, 0.079, 0.244, 0.065}};
//		KNPP calculations       begin 5                      Sinev (old) middle        End 4 - 2 month
//	const double fuelmix[3][4]  = {{0.661, 0.067, 0.249, 0.023}, {0.58, 0.07, 0.30, 0.05}, {0.474, 0.074, 0.371, 0.077}};
//	KNPP fuel contribution calculations Begin 4			End 4				Begin 5
	const double fuelmix0[3][4]  = {{0.637, 0.068, 0.266, 0.028}, {0.447, 0.075, 0.389, 0.085}, {0.661, 0.067, 0.249, 0.023}};
//		Calculated   begin 5 + 2.5 months           middle (not used)    End 4 - 1.5 month Coeffs
	double fuelmix[3][4];
	const char cmppart[3][20] = {"Begin", "Middle", "End"};
	TString expname("danss_report_v4_mar18-calc");
	TString prc_v1("danss_report_v4_sep17-calc");
	TFile *fMc[4];
	TTree *tMc[4];
	TH1D *hMc[4];
	TH1D *hMcMixt[3];
	TH1D *hMcRatio;
	TFile *fExp;
	TFile *fExp_v1;
	TH1D *hExpw;
	TH1D *hExp;
	TH1D *hExpw_v1;
	TH1D *hExp_v1;
	TH1D *hExpRatio;
	TH1D *hRatio;
	TH1D *hRatio_v1;
	TH1D *hDiff;
	TH1D *hReno;
	TPaveStats *pv;
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
        TCut cGamma("AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10");
	TCut cXYZ = cX && cY && cZ && cGamma;
	int i, j;
	char strs[128];
	char strl[1024];
	double s, term, A, B;

//		Calculate Reno with our resolution
	hReno = new TH1D("hReno", "Reno with data smoothed with DANSS energy resolution", 20, 1, 6);
	for (i=0; i<20; i++) hReno->SetBinContent(i+1, 1);
	TF1 *fGaus = new TF1("fGaus", "gaus", -30.0, 30.0);
	for (i=0; i<20; i++) {
		fGaus->SetParameter(0, 1);	// amplitude
		fGaus->SetParameter(1, 1.125 + 0.25*i);	// position
		fGaus->SetParameter(2, sqrt((1.125 + 0.25*i) * (0.3*0.3 + sigma*sigma) ));	// DANSS resolution: 30%/sqrt(E)
		hReno->Add(fGaus, (reno[i]-1)*0.25/fGaus->Integral(-30.0, 30.0));
	}
	hReno->SetLineColor(kBlue);
	hReno->SetLineWidth(2);

//		Calculated   begin 5 + 2.5 months     middle (not used)    End 4 - 1.5 month
	for (i=0; i<4; i++) {
//		begin 5 + 2.5 months => <begin 5> * (1- 2.5*30/471) + <end 4> * (2.5*30/471)
		term = 2.5 * 30 / 471;
		fuelmix[0][i] = fuelmix0[2][i] * (1 - term) + fuelmix0[1][i] * term;
//		middle (not used)
		fuelmix[1][i] = (fuelmix0[0][i] + fuelmix0[1][i]) / 2.0;
//		End 4 - 1.5 month
		term = 1.5 * 30 / 471;
		fuelmix[2][i] = fuelmix0[1][i] * (1 - term) + fuelmix0[0][i] * term;
	}
	
	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	gStyle->SetTitleXSize(0.05);
	gStyle->SetTitleYSize(0.05);
	gStyle->SetLabelSize(0.05);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadBottomMargin(0.16);

	fExp = new TFile(expname + ".root");
	fExp_v1 = new TFile(prc_v1 + ".root");
	if (!(fExp->IsOpen() && fExp_v1->IsOpen())) return;
	hExpw = (TH1D*) fExp->Get("hSumAfter");
	hExpw_v1 = (TH1D*) fExp_v1->Get("hSum3");
	if (!(hExpw && hExp_v1)) return;
	hExp = new TH1D("hExp", "Experimental positron spectrum;MeV;Events/(day*0.25 MeV)", 
		RangeMax - RangeMin + 1, hExpw->GetBinLowEdge(RangeMin), hExpw->GetBinLowEdge(RangeMax) + hExpw->GetBinWidth(RangeMax));
	hExp_v1 = new TH1D("hExp_v1", "Experimental positron spectrum (V1);MeV;Events/(day*0.25 MeV)", 
		RangeMax - RangeMin + 1, hExpw->GetBinLowEdge(RangeMin), hExpw->GetBinLowEdge(RangeMax) + hExpw->GetBinWidth(RangeMax));
	for (i=0; i<RangeMax - RangeMin + 1; i++) {
		hExp->SetBinContent(i+1, hExpw->GetBinContent(i + RangeMin));
		hExp->SetBinError(i+1, hExpw->GetBinError(i + RangeMin));
	}
	for (i=0; i<RangeMax - RangeMin + 1; i++) {
		hExp_v1->SetBinContent(i+1, hExpw_v1->GetBinContent(i + RangeMin));
		hExp_v1->SetBinError(i+1, hExpw_v1->GetBinError(i + RangeMin));
	}
	hExp->SetLineColor(kBlack);
	hExp->SetMarkerColor(kBlack);
	hExp->SetMarkerStyle(20);
	hExp->SetLineWidth(4);
	hExp_v1->SetLineColor(kBlack);
	hExp_v1->SetMarkerColor(kBlack);
	hExp_v1->SetMarkerStyle(20);
	hExp_v1->SetLineWidth(4);

	hExpRatio = (TH1D*) fExp->Get("hNRatioAfter2Before");
	if (!hExpRatio) return;
	hExpRatio->Scale(1.004);	// Dead time & Power correction
	hExpRatio->SetLineWidth(3);
	hExpRatio->SetLineColor(kGreen);
	hExpRatio->GetXaxis()->SetRange(RangeMin, RangeMax);
	hExpRatio->SetMinimum(1.0);
	hExpRatio->SetMaximum(1.2);
	hExpRatio->GetYaxis()->SetLabelSize(0.05);

	gROOT->cd();
	for (i=0; i<4; i++) {
		sprintf(strs, "h%s", fuel[i]);
		sprintf(strl, "Positron spectrum of %s;MeV", fuel[i]);
		hMc[i] = new TH1D(strs, strl, RangeMax - RangeMin + 1, hExpw->GetBinLowEdge(RangeMin), hExpw->GetBinLowEdge(RangeMax) + hExpw->GetBinWidth(RangeMax));
		hMc[i]->SetLineColor(fColor[i]);
	}
	for (i=0; i<4; i++) {
		fMc[i] = new TFile(mcfile[i]);
		if (!fMc[i]->IsOpen()) break;
		tMc[i] = (TTree *) fMc[i]->Get("DanssPair");
//		tMc[i] = (TTree *) fMc[i]->Get("DanssEvent");
	}
	if (i != 4) {
		printf("Mc file %s error\n", mcfile[i]);
		return;
	}
	gROOT->cd();
	sprintf(strs, "%6.3f*MyRandom::GausAdd(PositronEnergy, %8.5f)", norm, sigma);
//	sprintf(strs, "%6.3f*MyRandom::GausAdd((PositronEnergy-0.179)/0.929, %8.5f)", norm, sigma);
	for (i=0; i<4; i++) tMc[i]->Project(hMc[i]->GetName(), strs, cXYZ);
	for (i=0; i<4; i++) hMc[i]->Sumw2();
	for (i=0; i<4; i++) hMc[i]->Scale(crossection[i]);
	for (j=0; j<3; j++) {
		sprintf(strs, "h%sMixt", cmppart[j]);
		hMcMixt[j] = (TH1D*) hMc[0]->Clone(strs);
		hMcMixt[j]->Reset();
		for (i=0; i<4; i++) hMcMixt[j]->Add(hMc[i], fuelmix[j][i]);
		hMcMixt[j]->SetLineColor(fColor[j]);
		hMcMixt[j]->SetLineWidth(2);
		hMcMixt[j]->SetMarkerSize(3);
		hMcMixt[j]->SetMarkerStyle(kFullStar);
		hMcMixt[j]->SetMarkerColor(fColor[j]);
		hMcMixt[j]->GetYaxis()->SetLabelSize(0.05);
		sprintf(strs, "MC spectrum %s;Positron energy, MeV;Events/(day*0.25 MeV)", cmppart[j]);
		hMcMixt[j]->SetTitle(strs);
	}
	hMcRatio = (TH1D*) hMc[0]->Clone("hMcBegin2End");
	hMcRatio->Reset();
	hMcRatio->Divide(hMcMixt[0], hMcMixt[2]);
	for (i=0; i<hMcRatio->GetNbinsX(); i++) {
		s = 0;
		A = hMcMixt[0]->GetBinContent(i + 1);
		B = hMcMixt[2]->GetBinContent(i + 1);
		for (j=0; j<4; j++) {
			term = fuelmix[0][j] * B - fuelmix[2][j] * A;
			term *= hMc[j]->GetBinError(i+1);
			s += term*term;
		}
		s = sqrt(s) / (B * B);
		hMcRatio->SetBinError(i+1, s);
	}
	hMcRatio->SetLineColor(kBlue);
	hMcRatio->SetLineWidth(2);
	hMcRatio->SetMarkerStyle(kFullStar);
	hMcRatio->SetMarkerColor(kBlue);
	hMcRatio->SetMarkerSize(3);

	hMcRatio->GetYaxis()->SetLabelSize(0.05);
	hMcRatio->SetTitle("Begin to end spectrum ratio;Positron energy, MeV;Events/(day*0.25 MeV)");
	
	
//	for (j=0; j<3; j++) hMcMixt[j]->Scale(CalculateScale(hExp, hMcMixt[j], NormMin, NormMax));
	hMcMixt[0]->Scale(CalculateScale(hExp, hMcMixt[0], NormMin, NormMax));	// compare to V2 after shutdown
	hMcMixt[2]->Scale(CalculateScale(hExp_v1, hMcMixt[2], NormMin, NormMax));	// compare to V1 before shutdown
	hRatio = (TH1D *) hMcMixt[0]->Clone("hRatioExpMc");
	hRatio->Divide(hExp, hMcMixt[0]);
	hRatio->SetTitle(";Positron energy, MeV;#frac{N_{EXP}}{N_{MC}}");
	hRatio_v1 = (TH1D *) hMcMixt[0]->Clone("hRatioExpMc_v1");
	hRatio_v1->Divide(hExp_v1, hMcMixt[2]);
	hRatio_v1->SetTitle(";Positron energy, MeV;#frac{N_{EXP}}{N_{MC}}");

	hDiff = (TH1D *) hMcMixt[0]->Clone("hDiffExpMc");
	hDiff->Add(hExp, hMcMixt[0], 1, -1);
	hDiff->SetTitle("Experiment - Monte Carlo;Positron energy, MeV;#Delta Events/(day*0.25 MeV)");

	sprintf(strs, "_S%5.3f_R%4.2f", norm, sigma);
	expname += strs;
	TFile *fSave = new TFile(expname + "-pos2mc.root", "RECREATE");
	fSave->cd();
	for (j=0; j<4; j++) hMc[j]->Write();
	for (j=0; j<3; j++) hMcMixt[j]->Write();
	hMcRatio->Write();
	hExp->Write();
	hExp_v1->Write();
	hExpRatio->Write();
	hRatio->Write();
	hRatio_v1->Write();
	hDiff->Write();
	fSave->Close();

//		MC 4-fuel plot
	TCanvas *cm = new TCanvas("CM", "Fuel", 1200, 900);
	hMc[1]->SetTitle("DANSS simulated spectrum per isotope;MeV;");
	hMc[1]->Draw("hist");
	hMc[0]->Draw("hist,same");
	hMc[2]->Draw("hist,same");
	hMc[3]->Draw("hist,same");
	TLegend *lm = new TLegend(0.5, 0.7, 0.9, 0.9);
	for (i=0; i<4; i++) {
		sprintf(strs, "%s - %4.1f%% (before) / %4.1f%% (after)", fuel[i], 100*fuelmix[2][i], 100*fuelmix[0][i]);
		lm->AddEntry(hMc[i], strs, "L");
	}
	lm->Draw();
	TLatex *txt = new TLatex();
	txt->SetTextSize(0.035);
	sprintf(strs, "MC Scale=%5.3f Random+=%2.0f%%", norm, sigma*100);
	txt->DrawLatex(4.3, 95000, strs);
	cm->SaveAs(expname + "-pos2mc.pdf[");
	cm->SaveAs(expname + "-pos2mc.pdf");
	
//		EXP.V2 vs MC after shutdown
	TCanvas *cv = new TCanvas("CV", "Exp & MC", 1200, 900);
//	for (j=0; j<3; j++) hMcMixt[j]->Draw((j) ? "same,hist" : "hist");
	hMcMixt[0]->SetTitle("3 months period one month after campaign begin, V2");
	hMcMixt[0]->Draw("E");
	hExp->Draw("same");
	TLegend *lg = new TLegend(0.6, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp, "DANSS data", "LP");
	lg->AddEntry(hMcMixt[0], "Monte Carlo", "LP");
//	for (j=0; j<3; j++) {
//		sprintf(strs, "MC - %s", cmppart[j]);
//		lg->AddEntry(hMcMixt[j], strs, "L");
//	}
	lg->Draw();
	cv->SaveAs(expname + "-pos2mc.pdf");

//		EXP.V1 vs MC before shutdown
	cv->Clear();
	hMcMixt[2]->SetTitle("3 months just before shutdown, V1");
	hMcMixt[2]->Draw("E");
	hExp_v1->Draw("same");
	lg = new TLegend(0.6, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp_v1, "DANSS data", "LP");
	lg->AddEntry(hMcMixt[2], "Monte Carlo", "LP");
	lg->Draw();
	cv->SaveAs(expname + "-pos2mc.pdf");

//		EXP.V2 vs MC after shutdown - ratio
	TCanvas *cr = new TCanvas("CR", "Ratio", 1200, 500);
	hRatio->SetTitle("EXP.V2 / MC after shutdown");
	hRatio->GetYaxis()->SetLabelSize(0.08);
	hRatio->GetXaxis()->SetLabelSize(0.08);
	hRatio->GetYaxis()->SetTitleSize(0.08);
	hRatio->GetXaxis()->SetTitleSize(0.08);
	hRatio->SetLineWidth(4);
	hRatio->SetMinimum(0.9);
	hRatio->SetMaximum(1.2);
	hRatio->Draw();
	hReno->Draw("hist,same,][");
	lg = new TLegend(0.6, 0.7, 0.9, 0.9);
	lg->AddEntry(hRatio, "DANSS data", "LPE");
	lg->AddEntry(hReno, "RENO smoothed by DANSS resolution", "L");
	lg->Draw();
	cr->SaveAs(expname + "-pos2mc.pdf");

//		EXP.V1 vs MC before shutdown - ratio
	cr->Clear();
	hRatio_v1->SetTitle("EXP.V1 / MC before shutdown");
	hRatio_v1->GetYaxis()->SetLabelSize(0.08);
	hRatio_v1->GetXaxis()->SetLabelSize(0.08);
	hRatio_v1->GetYaxis()->SetTitleSize(0.08);
	hRatio_v1->GetXaxis()->SetTitleSize(0.08);
	hRatio_v1->SetLineWidth(4);
	hRatio_v1->SetMinimum(0.9);
	hRatio_v1->SetMaximum(1.2);
	hRatio_v1->Draw();
	hReno->Draw("hist,same,][");
	lg->Draw();
	cr->SaveAs(expname + "-pos2mc.pdf");

//		EXP.V2 vs after shutdown - difference
	TCanvas *cd = new TCanvas("CD", "Difference", 1200, 500);
	hDiff->GetYaxis()->SetLabelSize(0.07);
	hDiff->GetXaxis()->SetLabelSize(0.07);
	hDiff->GetYaxis()->SetTitleSize(0.07);
	hDiff->GetXaxis()->SetTitleSize(0.07);
	hDiff->SetLineWidth(4);
	hDiff->SetMinimum(-10);
	hDiff->SetMaximum(10);
	hDiff->Draw();
	cd->SaveAs(expname + "-pos2mc.pdf");

//		Spectrum ratio before / after
	TCanvas *cnr = new TCanvas("CNR", "Spectrum ratio: Exp vs MC", 1200, 900);
	TH1D *hExpRatioR = (TH1D *)hExpRatio->Clone("hExpRatioR");
	TH1D *hOneE = (TH1D *)hExpRatio->Clone("hOne");
	TH1D *hMcRatioR = (TH1D *)hMcRatio->Clone("hMcRatioR");
	TH1D *hOneM = (TH1D *)hMcRatio->Clone("hOne");
	hExpRatioR->SetTitle("Normalized ratio before shutdown / after shutdown");
	for (i=hOneE->GetXaxis()->GetFirst(); i<=hOneE->GetXaxis()->GetLast(); i++) {
		hOneE->SetBinContent(i, 1);
		hOneE->SetBinError(i, 1.0E-10);
	}
	for (i=hOneM->GetXaxis()->GetFirst(); i<=hOneM->GetXaxis()->GetLast(); i++) {
		hOneM->SetBinContent(i, 1);
		hOneM->SetBinError(i, 1.0E-10);
	}
	hExpRatioR->Divide(hOneE, hExpRatio);
	hMcRatioR->Divide(hOneM, hMcRatio);
	hExpRatioR->SetMinimum(0.75);
	hExpRatioR->SetMaximum(1);
	hExpRatioR->Draw();
	hMcRatioR->Draw("same");
	lg = new TLegend(0.2, 0.25, 0.5, 0.4);
	lg->AddEntry(hExpRatioR, "DANSS data", "LE");
	lg->AddEntry(hMcRatioR, "Monte Carlo", "LPE");
	lg->Draw();
	cnr->SaveAs(expname + "-pos2mc.pdf");

//		Spectrum ratio after / before
	cnr->Clear();
	hExpRatio->Draw();
	hMcRatio->Draw("same");
	lg = new TLegend(0.2, 0.7, 0.5, 0.85);
	lg->AddEntry(hExpRatio, "DANSS data", "LE");
	lg->AddEntry(hMcRatio, "Monte Carlo", "LPE");
	lg->Draw();
	cnr->SaveAs(expname + "-pos2mc.pdf");

//		Spectrum ratio after / before - numbers
	cnr->Clear();
	gStyle->SetOptFit();
	hExpRatio->SetStats();
	hMcRatio->SetStats();
	hExpRatio->Fit("pol1", "", "", 1, 6);
	hExpRatio->GetFunction("pol1")->SetLineColor(kGreen);
	hMcRatio->Fit("pol1", "", "sames", 1, 6);
	hMcRatio->GetFunction("pol1")->SetLineColor(kBlue);
	lg->Draw();
	cnr->Update();
	pv = (TPaveStats *) hExpRatio->FindObject("stats");
	pv->SetLineColor(kGreen);
	pv->SetX1NDC(0.46);
	pv->SetX2NDC(0.66);
	pv->SetY1NDC(0.18);
	pv->SetY2NDC(0.32);
	
	pv = (TPaveStats *) hMcRatio->FindObject("stats");
	pv->SetLineColor(kBlue);
	pv->SetX1NDC(0.68);
	pv->SetX2NDC(0.88);
	pv->SetY1NDC(0.18);
	pv->SetY2NDC(0.32);

//		Calculate various chi2
	TF1 *fPol1 = new TF1("fPol1", "pol1", 1, 6);
	txt->SetTextSize(0.02);
	fPol1->FixParameter(0, 1);
	fPol1->FixParameter(1, 0);
	sprintf(strl, "Const=1.000  #chi^{2}/ndf = %7.2f/20", hExpRatio->Chisquare(fPol1, "r"));
	txt->DrawLatex(1.5, 1.01, strl);
	fPol1->FixParameter(0, 1.015);
	sprintf(strl, "Const=1.015  #chi^{2}/ndf = %7.2f/20", hExpRatio->Chisquare(fPol1, "r"));
	txt->DrawLatex(1.5, 1.02, strl);
	fPol1->FixParameter(0, 1.02);
	sprintf(strl, "Const=1.02  #chi^{2}/ndf = %7.2f/20", hExpRatio->Chisquare(fPol1, "r"));
	txt->DrawLatex(1.5, 1.03, strl);

	hExpRatio->Fit("pol0", "Q+", "", 1, 6);
	sprintf(strl, "Const=%5.3f  #chi^{2}/ndf = %7.2f/19", 
		((TF1*)hExpRatio->FindObject("pol0"))->GetParameter(0),
		((TF1*)hExpRatio->FindObject("pol0"))->GetChisquare());
	txt->DrawLatex(1.5, 1.12, strl);

	fPol1->FixParameter(0, ((TF1*)hMcRatio->FindObject("pol1"))->GetParameter(0));
	fPol1->FixParameter(1, ((TF1*)hMcRatio->FindObject("pol1"))->GetParameter(1));
	sprintf(strl, "MC   #chi^{2}/ndf = %7.2f/20", hExpRatio->Chisquare(fPol1, "r"));
	txt->DrawLatex(1.5, 1.13, strl);

	cnr->Update();
	cnr->SaveAs(expname + "-pos2mc.pdf");
	cnr->SaveAs(expname + "-pos2mc.pdf]");
}

void CalcMinMax(TH1D **hArray, int size, TH1D **hMin, TH1D **hMax)
{
	char str[128];
	int i, j;
	double aMin, aMax;
	TH1D *hA;
	TH1D *hB;
	
	sprintf(str, "%sMin", hArray[0]->GetName());
	hA = (TH1D *) hArray[0]->Clone(str);
	hA->Reset();
	sprintf(str, "%sMax", hArray[0]->GetName());
	hB = (TH1D *) hArray[0]->Clone(str);
	hB->Reset();
	
	for (i=0; i<hArray[0]->GetNbinsX(); i++) {
		aMin =  1.0E20;
		aMax = -1.0E20;
		for (j=0; j<size; j++) {
			if (hArray[j]->GetBinContent(i+1) > aMax) aMax = hArray[j]->GetBinContent(i+1);
			if (hArray[j]->GetBinContent(i+1) < aMin) aMin = hArray[j]->GetBinContent(i+1);
		}
		hA->SetBinContent(i+1, aMin);
		hB->SetBinContent(i+1, aMax);
	}
	*hMin = hA;
	*hMax = hB;
}

void positron_spectrum_with_systematics(void)
{
	const double reno[20] = {
		1.00, 1.02, 0.995, 1.00, 1.00, 1.015, 1.005, 0.98,  1.02, 1.055,
		1.09, 1.12, 1.13,  1.12, 1.06, 1.05,  1.04,  1.005, 0.95, 0.98
	};
	const char fname[9][128] = {
		"exp2mc/danss_report_v4_mar18-calc_S0.980_R0.12-pos2mc.root",
		"exp2mc/danss_report_v4_mar18-calc_S1.000_R0.12-pos2mc.root",
		"exp2mc/danss_report_v4_mar18-calc_S1.020_R0.12-pos2mc.root",
		"exp2mc/danss_report_v4_mar18-calc_S0.980_R0.17-pos2mc.root",
		"exp2mc/danss_report_v4_mar18-calc_S1.000_R0.17-pos2mc.root",
		"exp2mc/danss_report_v4_mar18-calc_S1.020_R0.17-pos2mc.root",
		"exp2mc/danss_report_v4_mar18-calc_S0.980_R0.22-pos2mc.root",
		"exp2mc/danss_report_v4_mar18-calc_S1.000_R0.22-pos2mc.root",
		"exp2mc/danss_report_v4_mar18-calc_S1.020_R0.22-pos2mc.root"};
	TFile *f[9];
	TH1D *hExp;
	TH1D *hMC[9];
	TH1D *hMCMin;
	TH1D *hMCMax;
	TH1D *hRatio[9];
	TH1D *hRatioMin;
	TH1D *hRatioMax;
	TH1D *hRatioAvr;
	TH1D *hReno;
	int i;
//		Get histogramms
	for (i=0; i<9; i++) {
		f[i] = new TFile(fname[i]);
		hMC[i] = (TH1D *) f[i]->Get("hBeginMixt");
		hRatio[i] = (TH1D *) f[i]->Get("hRatioExpMc");
		if (!(hMC[i] && hRatio[i])) break;
	}
	hExp = (TH1D *) f[4]->Get("hExp");
	if (!((i == 9) && hExp)) {
		printf("Something is wrong.\n");
		return;
	}
	printf("Files read\n");
//		Calculate systematic
	CalcMinMax(hMC, 9, &hMCMin, &hMCMax);
	printf("Calc done\n");
	for (i=0; i<hExp->GetNbinsX(); i++) hExp->SetBinError(i+1, (hMCMax->GetBinContent(i+1) - hMCMin->GetBinContent(i+1)) / 2);
	printf("Exp errors set\n");
	hExp->SetFillColor(kBlue - 10);
	hExp->SetLineColor(kBlue);
	hExp->SetStats(0);
	hExp->SetLineWidth(2);
	hExp->GetXaxis()->SetTitle("Positron energy, MeV");
	hMC[4]->SetMarkerSize(2);
	printf("hExp done\n");
	CalcMinMax(hRatio, 9, &hRatioMin, &hRatioMax);
	hRatioAvr = (TH1D *) hRatioMin->Clone("hRatioAvr");
	hRatioAvr->Reset();
	for (i=0; i<hRatioAvr->GetNbinsX(); i++) {
		hRatioAvr->SetBinContent(i+1, (hRatioMax->GetBinContent(i+1) + hRatioMin->GetBinContent(i+1)) / 2);
		hRatioAvr->SetBinError(i+1, (hRatioMax->GetBinContent(i+1) - hRatioMin->GetBinContent(i+1)) / 2);
	}
	hRatioAvr->SetFillColor(kBlue-10);
	hRatioAvr->SetMaximum(1.25);
	hRatioAvr->SetMinimum(0.95);
	hRatioAvr->SetStats(0);
	hRatioAvr->SetLineColor(kBlue);
	hRatioAvr->SetLineWidth(1);
	hRatioAvr->SetMarkerStyle(kNone);
//	hRatioAvr->GetYaxis()->SetTitleOffset(2);
	hRatioAvr->GetYaxis()->SetTitle("N_{EXP}/N_{MC}");
	hRatio[4]->SetLineColor(kBlack);
	hRatio[4]->SetLineWidth(2);
	hRatio[4]->SetMarkerStyle(kNone);
	printf("hRatio done\n");
	
//		Calculate Reno with our resolution
	hReno = new TH1D("hReno", "Reno with data smoothed with DANSS energy resolution 30%/sqrt(E)", 20, 1, 6);
	for (i=0; i<20; i++) hReno->SetBinContent(i+1, 1);
	TF1 *fGaus = new TF1("fGaus", "gaus", -30.0, 30.0);
	for (i=0; i<20; i++) {
		fGaus->SetParameter(0, 1);	// amplitude
		fGaus->SetParameter(1, 1.125 + 0.25*i);	// position
		fGaus->SetParameter(2, 0.30*sqrt(1.125 + 0.25*i));	// DANSS resolution: 30%/sqrt(E)
		hReno->Add(fGaus, (reno[i]-1)*0.25/fGaus->Integral(-30.0, 30.0));
	}
	hReno->SetMarkerStyle(kFullStar);
	hReno->SetMarkerSize(2);
	hReno->SetMarkerColor(kRed);
	printf("hReno done\n");

	TCanvas *cv = new TCanvas("CV", "Experiment versus MC", 900, 1200);
	cv->Divide(1, 2);
	cv->cd(1);
	hExp->Draw("E3");
	hExp->Draw("E,same");
	hMC[4]->Draw("P,same");
	TLegend *lg = new TLegend(0.45, 0.7, 0.95, 0.9);
	lg->AddEntry(hExp, "Data with systematic error band", "LFEP");
	lg->AddEntry(hMC[4], "Monte Carlo", "P");
	lg->Draw();
	
	cv->cd(2);//->SetLeftMargin(0.15);
	hRatioAvr->Draw("E3");
	hRatio[4]->Draw("E,same");
	hReno->Draw("P,same");
	lg = new TLegend(0.1, 0.75, 0.55, 1.0);
	lg->AddEntry(hRatio[4], "DANSS Data", "LE");
	lg->AddEntry(hRatioAvr, "Systematic error band", "F");
	lg->AddEntry(hReno, "Reno data with DANSS resolution", "P");
	lg->Draw();
	
	cv->Update();
	cv->SaveAs("exp2mc_withsystematic.pdf");
//		Dump
	printf("Experimental spectra\nEnergy        Events/day/0.25 Mev     Systematic error\n");
	for (i=0; i<hExp->GetNbinsX(); i++) printf("%5.3f-%5.3f    %f               %f\n",
		hExp->GetBinLowEdge(i+1), hExp->GetBinLowEdge(i+1) + hExp->GetBinWidth(i+1),
		hExp->GetBinContent(i+1), hExp->GetBinError(i+1));
	printf("Monte Carlo spectra\nEnergy        Events/day/0.25 Mev\n");
	for (i=0; i<hMC[4]->GetNbinsX(); i++) printf("%5.3f-%5.3f    %f +- %f\n",
		hMC[4]->GetBinLowEdge(i+1), hMC[4]->GetBinLowEdge(i+1) + hMC[4]->GetBinWidth(i+1),
		hMC[4]->GetBinContent(i+1), hMC[4]->GetBinError(i+1));
	printf("Experiment to MC ratio\nEnergy          Ratio               Systematic band\n");
	for (i=0; i<hRatioAvr->GetNbinsX(); i++) printf("%5.3f-%5.3f    %f +- %f   %f - %f\n",
		hRatioAvr->GetBinLowEdge(i+1), hRatioAvr->GetBinLowEdge(i+1) + hRatioAvr->GetBinWidth(i+1),
		hRatio[4]->GetBinContent(i+1), hRatio[4]->GetBinError(i+1),
		hRatioAvr->GetBinContent(i+1) - hRatioAvr->GetBinError(i+1), 
		hRatioAvr->GetBinContent(i+1) + hRatioAvr->GetBinError(i+1));
	printf("Ratio from Reno with DANSS resolution\nEnergy          Ratio\n");
	for (i=0; i<hExp->GetNbinsX(); i++) printf("%5.3f-%5.3f     %f\n",
		hReno->GetBinLowEdge(i+1), hReno->GetBinLowEdge(i+1) + hReno->GetBinWidth(i+1),
		hReno->GetBinContent(i+1));
}

void dumpfuelratios(const char *fname)
{
	int i;
	TFile f(fname);
	TH1 *hExp = (TH1 *) f.Get("hNRatioAfter2Before");
	TH1 *hMC = (TH1 *) f.Get("hMcBegin2End");
	if (!(hExp && hMC)) {
		printf("Something is wrong.\n");
		return;
	}
	
	printf("Ratio end of campaign to begin\nExperiment\nEnergy          Ratio\n");
	for (i=0; i<hExp->GetNbinsX(); i++) printf("%5.3f-%5.3f    %f +- %f\n",
		hExp->GetBinLowEdge(i+1), hExp->GetBinLowEdge(i+1) + hExp->GetBinWidth(i+1),
		1.0/hExp->GetBinContent(i+1), hExp->GetBinError(i+1)/(hExp->GetBinContent(i+1)*hExp->GetBinContent(i+1)));
	printf("Monte Carlo\nEnergy          Ratio\n");
	for (i=0; i<hMC->GetNbinsX(); i++) printf("%5.3f-%5.3f    %f +- %f\n",
		hMC->GetBinLowEdge(i+1), hMC->GetBinLowEdge(i+1) + hMC->GetBinWidth(i+1),
		1.0/hMC->GetBinContent(i+1), hMC->GetBinError(i+1)/(hMC->GetBinContent(i+1)*hMC->GetBinContent(i+1)));
}
