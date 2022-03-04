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
	static inline double GausAdd2(double val, double sigma)
	{
		return rnd.Gaus(val, val*sigma);
	};
};

void CountHistChi2(TH1 *hcm, TH1 *hMc, int kEmin, int kEmax, double y)
{
	int i;
	char str[256];
	double chi2 = 0;
	TLatex txt;
	
	for (i=kEmin; i<=kEmax; i++) 
		chi2 += (hcm->GetBinContent(i) - hMc->GetBinContent(i)) * (hcm->GetBinContent(i) - hMc->GetBinContent(i)) /
		(hcm->GetBinError(i) * hcm->GetBinError(i) + hMc->GetBinError(i) * hMc->GetBinError(i));
	txt.SetTextSize(0.03);
	sprintf(str, "#chi^{2}/n.d.f. = %6.1f / %d  [%4.1f-%4.1f] MeV", chi2, kEmax-kEmin+1, (kEmin-1)*0.1, kEmax*0.1);
	txt.DrawLatex(1.5, y, str);
}

void cm_capture_energy2(const char *fname, const char *mcname, double kRndm = 0.17, double scale = 1.0)
{
	int i, j;
	char str[2048];
	double chi2;
	TLatex txt;
	
	const int kEmin = 16;
	const int kEmax = 95;
	
	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	gStyle->SetTitleXSize(0.055);
	gStyle->SetTitleYSize(0.055);
	gStyle->SetLabelSize(0.055);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadBottomMargin(0.16);
	gStyle->SetLineWidth(2);
//	gStyle->SetPalette(kRainBow, 0);

	TH1D *hcm = new TH1D("HDC", "^{248}Cm source data;Energy of delayed event, MeV", 120, 0, 12);
	hcm->SetLineWidth(4);
	hcm->SetMarkerStyle(21);
	hcm->SetLineColor(kBlue);
	hcm->SetFillColor(kBlue-10);
	hcm->SetMarkerColor(kBlue);
	hcm->GetYaxis()->SetLabelSize(0.055);

	TH1D *hcmSi = new TH1D("HDCSi", "^{248}Cm source data: SiPM;Energy of delayed event, MeV", 120, 0, 12);
	hcmSi->SetLineWidth(4);
	hcmSi->SetMarkerStyle(21);
	hcmSi->SetLineColor(kBlue);
	hcmSi->SetFillColor(kBlue-10);
	hcmSi->SetMarkerColor(kBlue);
	hcmSi->GetYaxis()->SetLabelSize(0.055);

	TH1D *hcmPMT = new TH1D("HDCPMT", "^{248}Cm source data: PMT;Energy of delayed event, MeV", 120, 0, 12);
	hcmPMT->SetLineWidth(4);
	hcmPMT->SetMarkerStyle(21);
	hcmPMT->SetLineColor(kBlue);
	hcmPMT->SetFillColor(kBlue-10);
	hcmPMT->SetMarkerColor(kBlue);
	hcmPMT->GetYaxis()->SetLabelSize(0.055);

	TH1D *hMc = new TH1D("HMC", "Neutron Monte Carlo;Energy of delayed event, MeV", 120, 0, 12);
	hMc->SetLineWidth(4);
	hMc->SetMarkerStyle(21);
	hMc->SetLineColor(kBlue);
	hMc->SetFillColor(kBlue-10);
	hMc->SetMarkerColor(kBlue);
	hMc->GetYaxis()->SetLabelSize(0.06);

	TH1D *hMcSi = new TH1D("HMCSi", "Neutron Monte Carlo: SiPM;Energy of delayed event, MeV", 60, 0, 12);
	hMcSi->SetLineWidth(4);
	hMcSi->SetMarkerStyle(21);
	hMcSi->SetLineColor(kBlue);
	hMcSi->SetFillColor(kBlue-10);
	hMcSi->SetMarkerColor(kBlue);
	hMcSi->GetYaxis()->SetLabelSize(0.06);

	TH1D *hMcPMT = new TH1D("HMCPMT", "Neutron Monte Carlo: PMT;Energy of delayed event, MeV", 120, 0, 12);
	hMcPMT->SetLineWidth(4);
	hMcPMT->SetMarkerStyle(21);
	hMcPMT->SetLineColor(kBlue);
	hMcPMT->SetFillColor(kBlue-10);
	hMcPMT->SetMarkerColor(kBlue);
	hMcPMT->GetYaxis()->SetLabelSize(0.06);

	TH2D *hcmxy = new TH2D("HXY", "^{248}Cm source data;X, cm;Y, cm", 25, 0, 100, 25, 0, 100);
	hcmxy->GetXaxis()->SetLabelSize(0.05);
	hcmxy->GetYaxis()->SetLabelSize(0.05);
	hcmxy->GetZaxis()->SetLabelSize(0.05);

	TH2D *hMcxy = new TH2D("HMCXY", "Neutron Monte Carlo;X, cm;Y, cm", 25, 0, 100, 25, 0, 100);
	hMcxy->GetXaxis()->SetLabelSize(0.05);
	hMcxy->GetYaxis()->SetLabelSize(0.05);
	hMcxy->GetZaxis()->SetLabelSize(0.05);
	
	TH1D *hDiff = new TH1D("HDIFF", "Data - MC;Energy of delayed event, MeV;(Data-MC)/Data", 120, 0, 12);
	hDiff->SetLineWidth(4);
	hDiff->SetLineColor(kBlue);
	hDiff->GetYaxis()->SetLabelSize(0.06);

	TFile f(fname);
	if (!f.IsOpen()) return;
	TTree *t = (TTree *) f.Get("DanssCm");
	if (!t) return;
	TChain *tMc = new TChain("DanssEvent", "DanssEvent");
	if (!strcmp(mcname, "all")) {
		tMc->AddFile("/mnt/root0/danss_root4/mc_248Cm_neutron_Gd_corr_transcode.root");
		tMc->AddFile("/mnt/root0/danss_root4/mc_248Cm_neutron_Gd_corr_0_transcode.root");
		tMc->AddFile("/mnt/root0/danss_root4/mc_248Cm_neutron_Gd_corr_1_transcode.root");
		tMc->AddFile("/mnt/root0/danss_root4/mc_248Cm_neutron_Gd_corr_2_transcode.root");
	} else {
		tMc->AddFile(mcname);
	}
	if (!tMc->GetEntries()) return;
	
	gROOT->cd();
	sprintf(str, "%f*(SiPmCleanEnergy[1]+PmtCleanEnergy[1])/2", scale);
	t->Project("HDC", str, "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2");
	sprintf(str, "MyRandom::GausAdd((SiPmCleanEnergy+PmtCleanEnergy)/2, %6.4f)", kRndm);
	tMc->Project("HMC", str, "(globalTime % 125000000) > 250");
	sprintf(str, "%f*SiPmCleanEnergy[1]", scale);
	t->Project("HDCSi", str, "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2");
	sprintf(str, "MyRandom::GausAdd(SiPmCleanEnergy, %6.4f)", kRndm);
	tMc->Project("HMCSi", str, "(globalTime % 125000000) > 250");
	sprintf(str, "%f*PmtCleanEnergy[1]", scale);
	t->Project("HDCPMT", str, "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2");
	sprintf(str, "MyRandom::GausAdd(PmtCleanEnergy, %6.4f)", kRndm);
	tMc->Project("HMCPMT", str, "(globalTime % 125000000) > 250");
	t->Project("HXY", "NeutronX[1][1]+2:NeutronX[1][0]+2", "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2 && NeutronX[1][1]>=0 && NeutronX[1][0]>=0");
	tMc->Project("HMCXY", "NeutronX[1]+2:NeutronX[0]+2", "NeutronX[1]>=0 && NeutronX[0]>=0 && (globalTime % 125000000) > 250");
	hcmxy->Fill(18., 2., -1);
	hMcxy->Fill(18., 2., -1);
	*hDiff = ((*hcm) - (*hMc)) / (*hcm);

	TF1 *fGP2 = new TF1("fGP2", "gaus(0) + pol2(3)", 0, 10);
	fGP2->SetParNames("Const", "Mean", "#sigma", "P0", "P1", "P2");
	fGP2->SetLineColor(kRed);
	fGP2->SetLineWidth(2);

	sprintf(str, "248Cm_R%4.2f_N%5.3f_FIFRELIN.pdf", kRndm, scale);
	TString pdfName(str);
	
	TCanvas *cv = new TCanvas("CV", "Neutron Capture", 1200, 900);
	cv->Divide(2, 2);
	cv->cd(1);
	fGP2->SetParameters(hcm->GetMaximum()/5, 2, 0.5, 0, 0, 0);
	hcm->Fit(fGP2, "", "0", 0.9, 4);
	hcm->DrawCopy();
	TVirtualPad *pd2 = cv->cd(2);
	pd2->SetRightMargin(0.16);
	hcmxy->Draw("COLZ");
	cv->cd(3);
	fGP2->SetParameters(hMc->GetMaximum()/5, 2, 0.5, 0, 0, 0);
	hMc->Fit(fGP2, "", "", 0.9, 4);
	pd2 = cv->cd(4);
	pd2->SetRightMargin(0.16);
	hMcxy->Draw("COLZ");
	cv->SaveAs((pdfName+"(").Data());
	
	TCanvas *cvA = new TCanvas("CVA", "Neutron Capture", 1200, 900);
	cvA->Divide(2, 2);
	cvA->cd(1);
	fGP2->SetParameters(hcmSi->GetMaximum()/5, 2, 0.5, 0, 0, 0);
	hcmSi->Fit(fGP2, "", "", 0.9, 4);
	cvA->cd(2);
	fGP2->SetParameters(hcmPMT->GetMaximum()/5, 2, 0.5, 0, 0, 0);
	hcmPMT->Fit(fGP2, "", "", 0.9, 4);
	cvA->cd(3);
	fGP2->SetParameters(hMcSi->GetMaximum()/5, 2, 0.5, 0, 0, 0);
	hMcSi->Fit(fGP2, "", "", 0.9, 4);
	cvA->cd(4);
	fGP2->SetParameters(hMcPMT->GetMaximum()/5, 2, 0.5, 0, 0, 0);
	hMcPMT->Fit(fGP2, "", "", 0.9, 4);
	cvA->SaveAs(pdfName.Data());
	
	TCanvas *prl = new TCanvas("PRL", "Neutron capture", 900, 900);
	prl->cd();
	prl->SetLeftMargin(0.20);
	prl->SetRightMargin(0.03);
	prl->SetTopMargin(0.03);
	prl->SetBottomMargin(0.12);
	hcm->GetYaxis()->SetTitleOffset(1.9);
	hcm->SetStats(1);
	hcm->SetFillStyle(kNone);
	hcm->SetLineColor(kBlack);
	hcm->SetMarkerStyle(kNone);
	hcm->SetTitle(";Delayed energy, MeV;Events/100 keV");
	hcm->Draw("e");
	prl->Update();
	TPaveStats *st = (TPaveStats *)hcm->FindObject("stats");
	st->SetX1NDC(0.23);
	st->SetX2NDC(0.50);
	st->SetY1NDC(0.65);
	st->SetY2NDC(0.95);
	hMc->SetFillStyle(kNone);
	hMc->Scale(hcm->Integral(kEmin, kEmax) / hMc->Integral(kEmin, kEmax));
//	hMc->GetXaxis()->SetRange(kEmin, kEmax);
	hMc->Draw("same,hist,][");
	TLegend *lg = new TLegend(0.73, 0.83, 0.99, 0.95);
	sprintf(str, "Experiment * %5.1f%%", 100*scale);
	lg->AddEntry(hcm, str, "LE");
	sprintf(str, "MC+%2.0f%%/#sqrt{E}", 100*kRndm);
	lg->AddEntry(hMc, str, "L");
	lg->Draw();

	CountHistChi2(hcm, hMc, 16, 95, 0.16 * hcm->GetMaximum());
	CountHistChi2(hcm, hMc, 31, 95, 0.10 * hcm->GetMaximum());
	CountHistChi2(hcm, hMc, 71, 95, 0.04 * hcm->GetMaximum());

	prl->Update();
	prl->SaveAs(pdfName.Data());
	
	TCanvas *prl1 = new TCanvas("PRL1", "Neutron capture", 900, 500);
	prl1->cd();
	prl1->SetLeftMargin(0.20);
	prl1->SetRightMargin(0.03);
	prl1->SetTopMargin(0.03);
	prl1->SetBottomMargin(0.12);
	*hDiff = ((*hcm) - (*hMc)) / (*hcm);
	hDiff->SetMinimum(-0.15);
	hDiff->SetMaximum(0.15);
	hDiff->GetXaxis()->SetRange(kEmin, kEmax);
	hDiff->GetYaxis()->SetTitleOffset(1.9);
	hDiff->SetStats(0);
	hDiff->SetFillStyle(kNone);
	hDiff->SetLineColor(kBlack);
	hDiff->SetMarkerStyle(kNone);
	hDiff->SetTitle(";Delayed energy, MeV;(Data-MC)/Data");
	hDiff->Draw("e");
	
	prl1->Update();
	prl1->SaveAs((pdfName+")").Data());
	
	f.Close();
}
