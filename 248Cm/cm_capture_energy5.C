/**********************************************************
	Process data and MC for 248Cm measurements
248Cm runs:
Position  Date   Runs
center    mar17  14428-14485
center    nov18  50578-50647
center    jun22  127720-127772
edge      mar17  14487-14512
edge      nov18  50875-50947
***********************************************************/

//	Calculate chi^2 difference of two histograms
//  hcm, hMc - 1-dim histograms
//  kEmin, kEmax - minimum and maximum energy bins for the calculation
double CountHistChi2(TH1 *hcm, TH1 *hMc, int kEmin, int kEmax)
{
	int i;
	char str[256];
	double chi2 = 0;
	
	for (i=kEmin; i<=kEmax; i++) 
		chi2 += (hcm->GetBinContent(i) - hMc->GetBinContent(i)) * (hcm->GetBinContent(i) - hMc->GetBinContent(i)) /
		(hcm->GetBinError(i) * hcm->GetBinError(i) + hMc->GetBinError(i) * hMc->GetBinError(i));

	return chi2;
}

//	Create a chain and adds files to it
//  fname   - file name(s)
//  chname  - the chainname
TChain *create_chain(const char *fname, const char *chname)
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
	
	return ch;
}

//	Make experimental hists from file(s) with a tree of 248 fission events
//  fnameIn - input file name(s)
//  fnameOut - outptu file name
void src_248Cm(const char *fnameIn, const char *fnameOut, int nMax = 7)
{
	char strA[1024];
	char strB[1024];
	int i;

	TFile *fOut = new TFile(fnameOut, "RECREATE");
	if (!fOut->IsOpen()) return;
	
	TH1D *hCm = new TH1D("hCm", "^{248}Cm source data;Energy of delayed event, MeV;Events", 120, 0, 12);
	TH1D *hCmSiPM = new TH1D("hCmSiPM", "^{248}Cm source data: SiPM;Energy of delayed event, MeV;Events", 120, 0, 12);
	TH1D *hCmPMT = new TH1D("hCmPMT", "^{248}Cm source data: PMT;Energy of delayed event, MeV;Events", 120, 0, 12);
	TH2D *hCmXY = new TH2D("hCmXY", "^{248}Cm source data;X, cm;Y, cm", 25, 0, 100, 25, 0, 100);

	TChain *t = create_chain(fnameIn, "DanssCm");
	if (!t) {
		printf("Something is wrong with the experimental tree file %s\n", fname);
		return;
	}

	TH1D *hCm1 = (TH1D) hCm->Clone("hCm1");
	TH1D *hCmSiPM1 = (TH1D) hCmSiPM->Clone("hCmSiPM1");
	TH1D *hCmPMT1 = (TH1D) hCmPMT->Clone("hCmPMT1");
	TH1D *hCmXY1 = (TH2D) hCmXY->Clone("hCmXY1");

	fOut->cd();
	for (i=1; i<=nMax; i++) {
		sprintf(strA, "(SiPmCleanEnergy[%d]+PmtCleanEnergy[%d])/2", i, i);
		sprintf(strB, "N>%d && gtDiff[%d]/125<50 && gtDiff[%d]/125>1", i, i, i);
		t->Project(hCm1->GetName(), strA, strB);
		hCm->Add(hCm1);
		sprintf(strA, "SiPmCleanEnergy[%d]", i);
		t->Project(hCmSiPM1->GetName(), strA, strB);
		hCmSiPM->Add(hCmSiPM1);
		sprintf(strA, "PmtCleanEnergy[%d]", i);
		t->Project(hCmPMT1->GetName(), strA, strB);
		hCmPMT->Add(hCmPMT1);
		sprintf(atrA, "NeutronX[%d][1]+2:NeutronX[%d][0]+2", i, i);
		sprintf(strB, "N>%d && gtDiff[%d]/125<50 && gtDiff[%d]/125>1 && NeutronX[%d][1]>=0 && NeutronX[%d][0]>=0", i, i, i, i, i);
		t->Project(hCmXY1->GetName(), strA, strB);
		hCmXY->Add(hCmXY1);
	}
	hCmXY->Fill(18., 2., -1);
	
	fOut->cd();
	hCm->Write();
	hCmSiPM->Write();
	hCmPMT->Write();
	hCmXY->Write();
	fOut->Close();
}


void cm_capture_energy5(const char *fname, const char *mcname, const char *pos, double scale = 1.0)
{
	int i, j;
	char str[2048];
	double chi2;
	TLatex txt;
	long irc;
	
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

	sprintf(str, "248Cm_%s_v8.2_S%5.3f_R%5.3f_C%5.3f.pdf", pos, scale, RndmSqe, RndmC);
	TString pdfName(str);
	TString rootName = pdfName;
	rootName.ReplaceAll(".pdf", ".root");
	TFile *fOut = new TFile(rootName.Data(), "RECREATE");

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

	TH1D *hMcSi = new TH1D("HMCSi", "Neutron Monte Carlo: SiPM;Energy of delayed event, MeV", 120, 0, 12);
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

	TChain *t   = create_chain(fname, "DanssCm");
	TChain *tMc = create_chain(mcname, "DanssEvent");
	if (!tMc || !t) {
		printf("Trees Exp %p  MC %p\n", t, tMc);
		return;
	}

	fOut->cd();
	sprintf(str, "(SiPmCleanEnergy[1]+PmtCleanEnergy[1])/2");
	irc = t->Project("HDC", str, "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2");
	sprintf(str, "MyRandom::GausAdd(%5.3f*(SiPmCleanEnergy+PmtCleanEnergy)/2, %6.4f, %6.4f)", scale, RndmSqe, RndmC);
	irc = tMc->Project("HMC", str, "(globalTime % 125000000) > 250");
	sprintf(str, "SiPmCleanEnergy[1]");
	irc = t->Project("HDCSi", str, "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2");
	sprintf(str, "MyRandom::GausAdd(%5.3f*SiPmCleanEnergy, %6.4f, %6.4f)", scale, RndmSqe, RndmC);
	irc = tMc->Project("HMCSi", str, "(globalTime % 125000000) > 250");
	sprintf(str, "PmtCleanEnergy[1]");
	irc = t->Project("HDCPMT", str, "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2");
	sprintf(str, "MyRandom::GausAdd(%5.3f*PmtCleanEnergy, %6.4f, %6.4f)", scale, RndmSqe, RndmC);
	irc = tMc->Project("HMCPMT", str, "(globalTime % 125000000) > 250");
	irc = t->Project("HXY", "NeutronX[1][1]+2:NeutronX[1][0]+2", "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2 && NeutronX[1][1]>=0 && NeutronX[1][0]>=0");
	irc = tMc->Project("HMCXY", "NeutronX[1]+2:NeutronX[0]+2", "NeutronX[1]>=0 && NeutronX[0]>=0 && (globalTime % 125000000) > 250");
	hcmxy->Fill(18., 2., -1);
	hMcxy->Fill(18., 2., -1);
//	*hDiff = ((*hcm) - (*hMc)) / (*hcm);

//	TF1 *fGP2 = new TF1("fGP2", "gaus(0) + pol2(3)", 0, 10);
//	fGP2->SetParNames("Const", "Mean", "#sigma", "P0", "P1", "P2");
//	fGP2->SetLineColor(kRed);
//	fGP2->SetLineWidth(2);

//	sprintf(str, "248Cm_R%4.2f_N%5.3f_FIFRELIN.pdf", kRndm, scale);
//	TString pdfName((outname) ? outname : str);
	
	TCanvas *cv = new TCanvas("CV", "Neutron Capture", 1200, 900);
	cv->Divide(2, 2);
	cv->cd(1);
//	fGP2->SetParameters(hcm->GetMaximum()/5, 2, 0.5, 0, 0, 0);
//	hcm->Fit(fGP2, "", "0", 0.9, 4);
	hcm->DrawCopy();
	TVirtualPad *pd2 = cv->cd(2);
	pd2->SetRightMargin(0.16);
	hcmxy->Draw("COLZ");
	cv->cd(3);
//	fGP2->SetParameters(hMc->GetMaximum()/5, 2, 0.5, 0, 0, 0);
//	hMc->Fit(fGP2, "", "", 0.9, 4);
	hMc->DrawCopy();
	pd2 = cv->cd(4);
	pd2->SetRightMargin(0.16);
	hMcxy->Draw("COLZ");
	cv->SaveAs((pdfName+"(").Data());
	
	TCanvas *cvA = new TCanvas("CVA", "Neutron Capture", 1200, 900);
	cvA->Divide(2, 2);
	cvA->cd(1);
//	fGP2->SetParameters(hcmSi->GetMaximum()/5, 2, 0.5, 0, 0, 0);
//	hcmSi->Fit(fGP2, "", "", 0.9, 4);
	hcmSi->DrawCopy();
	cvA->cd(2);
//	fGP2->SetParameters(hcmPMT->GetMaximum()/5, 2, 0.5, 0, 0, 0);
//	hcmPMT->Fit(fGP2, "", "", 0.9, 4);
	hcmPMT->DrawCopy();
	cvA->cd(3);
//	fGP2->SetParameters(hMcSi->GetMaximum()/5, 2, 0.5, 0, 0, 0);
//	hMcSi->Fit(fGP2, "", "", 0.9, 4);
	hMcSi->DrawCopy();
	cvA->cd(4);
//	fGP2->SetParameters(hMcPMT->GetMaximum()/5, 2, 0.5, 0, 0, 0);
//	hMcPMT->Fit(fGP2, "", "", 0.9, 4);
	hMcPMT->DrawCopy();
	cvA->SaveAs(pdfName.Data());

	TCanvas *cvB = new TCanvas("CVB", "Neutron Capture", 1200, 900);
	cvB->Divide(2, 1);
	cvB->cd(1);
	hcmSi->SetFillStyle(kNone);
	hcmSi->SetLineColor(kBlack);
	hcmSi->SetMarkerStyle(kNone);
	hcmSi->SetTitle(";SiPM delayed energy, MeV;Events/100 keV");
	hcmSi->Draw("e");
	hMcSi->SetFillStyle(kNone);
	hMcSi->Scale(hcmSi->Integral(kEmin, kEmax) / hMcSi->Integral(kEmin, kEmax));
	hMcSi->Draw("same,hist,][");
	CountHistChi2(hcmSi, hMcSi, 16, 95, 0.16 * hcm->GetMaximum());
	cvB->cd(2);
	hcmPMT->SetFillStyle(kNone);
	hcmPMT->SetLineColor(kBlack);
	hcmPMT->SetMarkerStyle(kNone);
	hcmPMT->SetTitle(";PMT delayed energy, MeV;Events/100 keV");
	hcmPMT->Draw("e");
	hMcPMT->SetFillStyle(kNone);
	hMcPMT->Scale(hcmPMT->Integral(kEmin, kEmax) / hMcPMT->Integral(kEmin, kEmax));
	hMcPMT->Draw("same,hist,][");
	CountHistChi2(hcmPMT, hMcPMT, 16, 95, 0.16 * hcm->GetMaximum());
	cvB->SaveAs(pdfName.Data());
	
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
//	TPaveStats *st = (TPaveStats *)hcm->FindObject("stats");
//	st->SetX1NDC(0.23);
//	st->SetX2NDC(0.50);
//	st->SetY1NDC(0.65);
//	st->SetY2NDC(0.95);
	hMc->SetFillStyle(kNone);
	hMc->Scale(hcm->Integral(kEmin, kEmax) / hMc->Integral(kEmin, kEmax));
//	hMc->GetXaxis()->SetRange(kEmin, kEmax);
	hMc->Draw("same,hist,][");
	TLegend *lg = new TLegend(0.73, 0.83, 0.99, 0.95);
	sprintf(str, "Experiment");
	lg->AddEntry(hcm, str, "LE");
	sprintf(str, "MC(smeared) x %5.3f", scale);
	lg->AddEntry(hMc, str, "L");
	lg->Draw();

	CountHistChi2(hcm, hMc, 16, 95, 0.16 * hcm->GetMaximum());
	CountHistChi2(hcm, hMc, 31, 95, 0.10 * hcm->GetMaximum());
	CountHistChi2(hcm, hMc, 71, 95, 0.04 * hcm->GetMaximum());

	prl->Update();
	prl->SaveAs(pdfName.Data());
	
	hcm->SetLineColor(kRed);
	hcm->SetMarkerColor(kRed);
	hcm->SetMarkerStyle(kFullStar);
	hcm->SetMarkerSize(2);
	hcm->Draw("e");
	prl->Update();
	hMc->Draw("same,hist,][");
	delete lg;
	lg = new TLegend(0.73, 0.83, 0.99, 0.95);
	lg->AddEntry(hcm, "Exp.", "P");
	lg->AddEntry(hMc, "MC", "L");
	lg->Draw();
	prl->SaveAs(pdfName.Data());
	
	TCanvas *prl1 = new TCanvas("PRL1", "Neutron capture", 900, 500);
	prl1->cd();
	prl1->SetLeftMargin(0.20);
	prl1->SetRightMargin(0.03);
	prl1->SetTopMargin(0.03);
	prl1->SetBottomMargin(0.12);
	hDiff->Add(hcm, hMc, 1, -1);
	hDiff->Divide(hcm);
//	*hDiff = ((*hcm) - (*hMc)) / (*hcm);
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
	prl1->SaveAs(pdfName.Data());
	
	TCanvas *spt = new TCanvas("SiPM2PMT", "SiPM versus PMT", 1200, 900);
	spt->Divide(2, 1);
	spt->cd(1);
	hcmSi->SetLineColor(kRed);
	hcmPMT->SetLineColor(kBlue);
	lg = new TLegend(0.73, 0.83, 0.99, 0.95);
	lg->AddEntry(hcmSi, "SiPM", "l");
	lg->AddEntry(hcmPMT, "PMT", "l");
	hcmPMT->SetTitle("Experiment");
	hcmPMT->DrawCopy("hist");
	hcmSi->DrawCopy("hist,same");
	lg->Draw();
	spt->cd(2);
	hMcSi->SetLineColor(kRed);
	hMcPMT->SetLineColor(kBlue);
	hMcPMT->SetTitle("Monte-Carlo");
	hMcPMT->Draw("hist");
	hMcSi->Draw("hist,same");
	lg->Draw();
	spt->Update();
	spt->SaveAs((pdfName+")").Data());

	fOut->cd();
	hcm->Write();
	hMc->Write();
	hcmSi->Write();
	hMcSi->Write();
	hcmPMT->Write();
	hMcPMT->Write();
	hcmxy->Write();
	hMcxy->Write();
	hDiff->Write();
	fOut->Close();
}

double chi2Diff(const TH1D *hA, const TH1D *hB, int binMin, int binMax)
{
	double sum;
	int i;
	for (i = binMin; i <= binMax; i++) sum += 
		(hA->GetBinContent(i) - hB->GetBinContent(i)) * (hA->GetBinContent(i) - hB->GetBinContent(i)) /
		(hA->GetBinError(i) * hA->GetBinError(i) + hB->GetBinError(i) * hB->GetBinError(i));
	return sum;
}

TH2D *cm_scan(const char *fname, const char *mcname, const double cRndm = 0)
{
	const double Rrndm[2] = {0.02, 0.22};
	const int Nrndm = 5;
	double Drndm = (Rrndm[1] - Rrndm[0]) / (Nrndm - 1);
	const double Rscale[2] = {0.97, 1.03};
	const int Nscale = 13;
	double Dscale = (Rscale[1] - Rscale[0]) / (Nscale - 1);
	double kRndm, kScale;
	int i, j;
	TH2D *h2d;
	TFile *f;
	char strs[128], strl[1024];
	TH1D *hExp[Nscale];
	TH1D *hMC[Nrndm];
	int binMin, binMax;
	double chi2;
	const double Emin = 1.5;
	const double Emax = 9.5;
	
	gStyle->SetOptStat(0);

	TChain *tExp = create_chain(fname, "DanssCm");
	TChain *tMC  = create_chain(mcname, "DanssEvent");
	if (!tMC || !tExp) return NULL;

	sprintf(strl, "248Cm-scan_cnst_%5.3f.root", cRndm);
	TFile *fOut = new TFile(strl, "RECREATE");
	if (!fOut->IsOpen()) return NULL;
	sprintf(strl, "248Cm-scan_cnst_%5.3f.pdf", cRndm);
	TString pdfName(strl);
	TCanvas cv("CV", "CV", 1280, 900);
	cv.SaveAs((pdfName + "[").Data());
	TLegend lg(0.6, 0.75, 0.98, 0.88);
	TLatex txt;
	TLine ln;
	ln.SetLineColor(kCyan);
	ln.SetLineStyle(kDashed);
	ln.SetLineWidth(2);
	
	sprintf(strl, "Scan rndm and scale for 248Cm with const=%4.1f%%;Scale;Rndm", cRndm * 100);
	h2d = new TH2D("h248Cmscan", "Scan rndm and scale for 248Cm;Scale;Rndm", 
		Nscale, Rscale[0] - Dscale/2, Rscale[1] + Dscale/2, Nrndm, Rrndm[0] - Drndm/2, Rrndm[1] + Drndm/2);
	for (i=0; i<Nscale; i++) {
		kScale = Rscale[0] + Dscale * i;
		sprintf(strs, "hExp248Cm_%d", i);
		sprintf(strl, "^{248}Cm delayed energy * %5.1f%%;MeV", kScale * 100);
		hExp[i] = new TH1D(strs, strl, 120, 0, 12);
		sprintf(strl, "%f*(SiPmCleanEnergy[1]+PmtCleanEnergy[1])/2", kScale);
		tExp->Project(strs, strl, "N>1 && gtDiff[1]/125<50 && gtDiff[1]/125>2");
		hExp[i]->SetLineColor(kRed);
		hExp[i]->SetMarkerColor(kRed);
		hExp[i]->SetMarkerStyle(kFullStar);
	}
	for (j=0; j<Nrndm; j++) {
		kScale = Rscale[0] + Dscale * i;
		kRndm = Rrndm[0] + Drndm * j;
		sprintf(strs, "hMC248Cm_%d", j);
		sprintf(strl, "MC: neutron capture energy + %2.0f%%/#sqrt{E} #oplus %4.1f%%;MeV", kRndm * 100, cRndm * 100);
		hMC[j] = new TH1D(strs, strl, 120, 0, 12);
		sprintf(strl, "MyRandom::GausAdd((SiPmCleanEnergy+PmtCleanEnergy)/2, %6.4f, %6.4f)", kRndm, cRndm);
		tMC->Project(strs, strl, "(globalTime % 125000000) > 250");
		hMC[j]->SetLineColor(kBlue);
		hMC[j]->SetLineWidth(2);
	}

	binMin = hExp[0]->FindBin(Emin + 0.001);
	binMax = hExp[0]->FindBin(Emax - 0.001);
	for (i=0; i<Nscale; i++) for (j=0; j<Nrndm; j++) {
		hMC[j]->Scale(hExp[i]->Integral(binMin, binMax) / hMC[j]->Integral(binMin, binMax));
		chi2 = chi2Diff(hExp[i], hMC[j], binMin, binMax);
		h2d->SetBinContent(i+1, j+1, chi2);
		hExp[i]->Draw("e");
		hMC[j]->Draw("hist,same");
		lg.Clear();
		lg.AddEntry(hExp[i], hExp[i]->GetTitle(), "lpe");
		lg.AddEntry(hMC[j], hMC[j]->GetTitle(), "l");
		lg.Draw();
		sprintf(strl, "#chi^{2}/N.d.f. = %6.2g / %d", chi2, binMax - binMin);
		txt.DrawLatex(2, hExp[i]->GetMaximum() / 10.0, strl);
		ln.DrawLine(Emin, 0, Emin, hExp[i]->GetMaximum() * 0.6);
		ln.DrawLine(Emax, 0, Emax, hExp[i]->GetMaximum() * 0.6);
		cv.Update();
		cv.SaveAs(pdfName.Data());
	}

	h2d->Draw("box");
	cv.SaveAs(pdfName.Data());
	h2d->Draw("colorz");
	cv.SaveAs(pdfName.Data());
	h2d->Draw("lego2");
	cv.SaveAs(pdfName.Data());
	TH1D * hPrjX = h2d->ProjectionX("__X", 2, 2);
	hPrjX->SetTitle("^{248}Cm scan profile at Rndm = 7%/#sqrt{E}");
	hPrjX->Draw("e");
	cv.SaveAs(pdfName.Data());
	TH1D * hPrjY = h2d->ProjectionY("__Y", 7, 7);
	hPrjY->SetTitle("^{248}Cm scan profile at scale = 1");
	hPrjY->Draw("e");
	cv.SaveAs(pdfName.Data());

	cv.SaveAs((pdfName + "]").Data());
	
	fOut->cd();
	h2d->Write();
	for (i=0; i<Nscale; i++) hExp[i]->Write();
	for (j=0; j<Nrndm; j++) hMC[j]->Write();
	fOut->Close();
	
	return h2d;
}

// Draw SiPM to PMT comparison
void sipm2pmt(const char *fname)
{
	gStyle->SetOptStat(0);
	TFile *f = new TFile(fname);
	TH1D *hExpSiPM = (TH1D *) f->Get("HDCSi");
	TH1D *hExpPMT  = (TH1D *) f->Get("HDCPMT");
	TH1D *hMCSiPM  = (TH1D *) f->Get("HMCSi");
	TH1D *hMCPMT   = (TH1D *) f->Get("HMCPMT");
	hMCPMT->Scale(hMCSiPM->Integral() / hMCPMT->Integral());
	hExpSiPM->SetLineColor(kRed);
	hExpPMT->SetLineColor(kBlue);
	hMCSiPM->SetLineColor(kRed);
	hMCPMT->SetLineColor(kBlue);
	hExpSiPM->SetLineWidth(2);
	hExpPMT->SetLineWidth(2);
	hMCSiPM->SetLineWidth(2);
	hMCPMT->SetLineWidth(2);
	hExpSiPM->SetTitle("^{248}Cm - Experiment, SiPM and PMT;MeV;N");
	hMCSiPM->SetTitle("^{248}Cm - MC, SiPM and PMT;MeV;N");
	
	TCanvas *cv = new TCanvas("CV", "CV", 1600, 1200);
	cv->Divide(2, 1);
	cv->cd(1);
	TLegend *lg = new TLegend(0.7, 0.85, 0.9, 0.93);
	lg->AddEntry(hExpSiPM, "SiPM", "l");
	lg->AddEntry(hExpPMT, "PMT", "l");
	hExpSiPM->Draw("hist");
	hExpPMT->Draw("hist,same");
	lg->Draw();
	cv->cd(2);
	hMCSiPM->Draw("hist");
	hMCPMT->Draw("hist,same");
	lg->Draw();
}
