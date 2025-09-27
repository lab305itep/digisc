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

//	Calculate chi^2 difference of two histograms
//  hcm, hMc - 1-dim histograms
//  kEmin, kEmax - minimum and maximum energy bins for the calculation
double chi2Diff(TH1 *hcm, TH1 *hMc, int kEmin, int kEmax)
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

//	Make experimental hists from file(s) with a tree of 248Cm fission events
//  fnameIn - input file name(s)
//  fnameOut - outptu file name
void src_248Cm(const char *fnameIn, const char *fnameOut, int nMin = 2, int nMax = 7)
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
		printf("Something is wrong with the experimental tree file %s\n", fnameIn);
		return;
	}

	TH1D *hCm1 = (TH1D*) hCm->Clone("hCm1");
	TH1D *hCmSiPM1 = (TH1D*) hCmSiPM->Clone("hCmSiPM1");
	TH1D *hCmPMT1 = (TH1D*) hCmPMT->Clone("hCmPMT1");
	TH2D *hCmXY1 = (TH2D*) hCmXY->Clone("hCmXY1");

	fOut->cd();
	for (i=nMin; i<=nMax; i++) {
//		sprintf(strA, "(SiPmCleanEnergy[%d]+PmtCleanEnergy[%d])/2", i, i);
		sprintf(strA, "NeutronEnergy[%d]", i);
		sprintf(strB, "N>%d && gtDiff[%d]/125<50 && gtDiff[%d]/125>2", i, i, i);
		t->Project(hCm1->GetName(), strA, strB);
		hCm->Add(hCm1);
		sprintf(strA, "SiPmCleanEnergy[%d]", i);
		t->Project(hCmSiPM1->GetName(), strA, strB);
		hCmSiPM->Add(hCmSiPM1);
		sprintf(strA, "PmtCleanEnergy[%d]", i);
		t->Project(hCmPMT1->GetName(), strA, strB);
		hCmPMT->Add(hCmPMT1);
		sprintf(strA, "NeutronX[%d][1]+2:NeutronX[%d][0]+2", i, i);
		sprintf(strB, "N>%d && gtDiff[%d]/125<50 && gtDiff[%d]/125>2 && NeutronX[%d][1]>=0 && NeutronX[%d][0]>=0", i, i, i, i, i);
		t->Project(hCmXY1->GetName(), strA, strB);
		hCmXY->Add(hCmXY1);
	}
	hCmXY->Fill(18., 2., -1);
	hCm->Sumw2();
	hCmSiPM->Sumw2();
	hCmPMT->Sumw2();
	
	hCm->Write();
	hCmSiPM->Write();
	hCmPMT->Write();
	hCmXY->Write();
	fOut->Close();
}

//	Make MC hists from file(s) with a tree of 248Cm neutrons
//  fnameIn - input file name(s)
//  fnameOut - outptu file name
void src_248CmMCn(const char *fnameIn, const char *fnameOut)
{
	char strA[1024];
	char strB[1024];
	int i;
	double scale;
	TH1D *hMC;
	TH1D *hMCSiPM;
	TH1D *hMCPMT;
	TH2D *hMCXY;

	TFile *fOut = new TFile(fnameOut, "RECREATE");
	if (!fOut->IsOpen()) return;
	
	TChain *t = create_chain(fnameIn, "DanssEvent");
	if (!t) {
		printf("Something is wrong with the MC tree file %s\n", fnameIn);
		return;
	}

	fOut->cd();
	for (i=0; i<41; i++) {
		scale = 0.9 + 0.005 * i;
		sprintf(strA, "hMC_%2.2d", i);
		sprintf(strB, "^{248}Cm MC neutron data. Scale=%5.3f;Energy of delayed event, MeV;Events", scale);
		hMC = new TH1D(strA, strB, 120, 0, 12);
		sprintf(strA, "hMCSiPM_%2.2d", i);
		sprintf(strB, "^{248}Cm MC neutron data. Scale=%5.3f, SiPM;Energy of delayed event, MeV;Events", scale);
		hMCSiPM = new TH1D(strA, strB, 120, 0, 12);
		sprintf(strA, "hMCPMT_%2.2d", i);
		sprintf(strB, "^{248}Cm MC neutron data. Scale=%5.3f, PMT;Energy of delayed event, MeV;Events", scale);
		hMCPMT = new TH1D(strA, strB, 120, 0, 12);
		sprintf(strA, "NeutronEnergy*%5.3f", scale);
		t->Project(hMC->GetName(), strA, "(globalTime % 125000000) > 250 && (globalTime % 125000000) < 6250");
		sprintf(strA, "SiPmCleanEnergy*%5.3f", scale);
		t->Project(hMCSiPM->GetName(), strA, "(globalTime % 125000000) > 250 && (globalTime % 125000000) < 6250");
		sprintf(strA, "PmtCleanEnergy*%5.3f", scale);
		t->Project(hMCPMT->GetName(), strA, "(globalTime % 125000000) > 250 && (globalTime % 125000000) < 6250");

		hMC->Sumw2();
		hMCSiPM->Sumw2();
		hMCPMT->Sumw2();

		hMC->Write();
		hMCSiPM->Write();
		hMCPMT->Write();
	}
	hMCXY = new TH2D("hMCXY", "^{248}Cm MC neutron data;X, cm;Y, cm", 25, 0, 100, 25, 0, 100);
	t->Project(hMCXY->GetName(), "NeutronX[1]+2:NeutronX[0]+2", "NeutronX[1]>=0 && NeutronX[0]>=0 && (globalTime % 125000000) > 125 && (globalTime % 125000000) < 6250");
	hMCXY->Fill(18., 2., -1);
	hMCXY->Write();
	fOut->Close();
}

//	Make MC hists from file(s) with a tree of 248Cm fission events
//  fnameIn - input file name(s)
//  fnameOut - output file name
void src_248CmMC(const char *fnameIn, const char *fnameOut, int nMin = 2, int nMax = 7)
{
	char strA[1024];
	char strB[1024];
	int i, j;
	double scale;
	TH1D *hMC;
	TH1D *hMCSiPM;
	TH1D *hMCPMT;
	TH2D *hMCXY;
	TH1D *hMC1;
	TH1D *hMCSiPM1;
	TH1D *hMCPMT1;
	
	TFile *fOut = new TFile(fnameOut, "RECREATE");
	if (!fOut->IsOpen()) return;
	
	TChain *t = create_chain(fnameIn, "DanssCm");
	if (!t) {
		printf("Something is wrong with the MC tree file %s\n", fnameIn);
		return;
	}

	fOut->cd();
	for (i=0; i<41; i++) {
		scale = 0.9 + 0.005 * i;
		sprintf(strA, "hMC_%2.2d", i);
		sprintf(strB, "^{248}Cm MC neutron data. Scale=%5.3f;Energy of delayed event, MeV;Events", scale);
		hMC = new TH1D(strA, strB, 120, 0, 12);
		sprintf(strA, "hMCSiPM_%2.2d", i);
		sprintf(strB, "^{248}Cm MC neutron data. Scale=%5.3f, SiPM;Energy of delayed event, MeV;Events", scale);
		hMCSiPM = new TH1D(strA, strB, 120, 0, 12);
		sprintf(strA, "hMCPMT_%2.2d", i);
		sprintf(strB, "^{248}Cm MC neutron data. Scale=%5.3f, PMT;Energy of delayed event, MeV;Events", scale);
		hMCPMT = new TH1D(strA, strB, 120, 0, 12);
		
		hMC1 = (TH1D*) hMC->Clone("hMC1");
		hMCSiPM1 = (TH1D*) hMCSiPM->Clone("hMCSiPM1");
		hMCPMT1 = (TH1D*) hMCPMT->Clone("hMCPMT1");
		for (j=nMin; j<=nMax; j++) {
			sprintf(strA, "NeutronEnergy[%d]*%5.3f", j, scale);
			sprintf(strB, "N>%d && gtDiff[%d]/125<50 && gtDiff[%d]/125>2", j, j, j);
			t->Project(hMC1->GetName(), strA, strB);
			hMC->Add(hMC1);
			sprintf(strA, "SiPmCleanEnergy[%d]*%5.3f", j, scale);
			t->Project(hMCSiPM1->GetName(), strA, strB);
			hMCSiPM->Add(hMCSiPM1);
			sprintf(strA, "PmtCleanEnergy[%d]*%5.3f", j, scale);
			t->Project(hMCPMT1->GetName(), strA, strB);
			hMCPMT->Add(hMCPMT1);
		}

		hMC->Sumw2();
		hMCSiPM->Sumw2();
		hMCPMT->Sumw2();

		hMC->Write();
		hMCSiPM->Write();
		hMCPMT->Write();
		
		delete hMC1;
		delete hMCSiPM1;
		delete hMCPMT1;
	}
	hMCXY = new TH2D("hMCXY", "^{248}Cm MC neutron data;X, cm;Y, cm", 25, 0, 100, 25, 0, 100);
	TH2D *hMCXY1 = (TH2D*) hMCXY->Clone("hCmXY1");
	for (i=nMin; i<=nMax; i++) {
		sprintf(strA, "NeutronX[%d][1]+2:NeutronX[%d][0]+2", i, i);
		sprintf(strB, "N>%d && gtDiff[%d]/125<50 && gtDiff[%d]/125>2 && NeutronX[%d][1]>=0 && NeutronX[%d][0]>=0", i, i, i, i, i);
		t->Project(hMCXY1->GetName(), strA, strB);
		hMCXY->Add(hMCXY1);
	}
	hMCXY->Fill(18., 2., -1);
	hMCXY->Write();
	fOut->Close();
}

/*
	Scan the difference between the experimental and MC histgrams.
	expname - file with experimental histograms
	mcname - file with MC histograms
	resname - file for output scans (root). The same name is used for png picture.
*/
void scan_248Cm(const char *expname, const char *mcname, const char *resname)
{
	const int kEmin = 16;
	const int kEmax = 95;
	char str[256];
	TH1D *hMC;
	TH1D *hMCSiPM;
	TH1D *hMCPMT;
	int i;
	double xmin;
	TLatex txt;
	
	TH1D *hScan = new TH1D("hScan248Cm", "#chi^{2} difference between MC and data, ^{248}Cm, SiPM+PMT;Scale;Shift;#chi^{2}", 41, 0.9, 1.1);
	TH1D *hScanSiPM = new TH1D("hScan248CmSiPM", "#chi^{2} difference between MC and data, ^{248}Cm, SiPM;Scale;Shift;#chi^{2}", 41, 0.9, 1.1);
	TH1D *hScanPMT = new TH1D("hScan248CmPMT", "#chi^{2} difference between MC and data, ^{248}Cm, PMT;Scale;Shift;#chi^{2}", 41, 0.9, 1.1);
	
	TFile *fExp = new TFile(expname);
	TFile *fMC  = new TFile(mcname);
	if (!fExp->IsOpen() || !fMC->IsOpen()) return;
	
	TH1D *hExp = (TH1D *) fExp->Get("hCm");
	TH1D *hExpSiPM = (TH1D *) fExp->Get("hCmSiPM");
	TH1D *hExpPMT = (TH1D *) fExp->Get("hCmPMT");
	if (!hExp || !hExpSiPM || !hExpPMT) {
		printf("Not all histograms found in %s.\n", expname);
		return;
	}
	for (i=0; i<41; i++) {
		sprintf(str, "hMC_%2.2d", i);
		hMC = (TH1D *) fMC->Get(str);
		sprintf(str, "hMCSiPM_%2.2d", i);
		hMCSiPM = (TH1D *) fMC->Get(str);
		sprintf(str, "hMCPMT_%2.2d", i);
		hMCPMT = (TH1D *) fMC->Get(str);
		if (!hMC || !hMCSiPM || !hMCPMT) {
			printf("Not all histograms found in %s.\n", mcname);
			return;
		}
		hMC->Scale(hExp->Integral(kEmin, kEmax) / hMC->Integral(kEmin, kEmax));
		hMCSiPM->Scale(hExpSiPM->Integral(kEmin, kEmax) / hMCSiPM->Integral(kEmin, kEmax));
		hMCPMT->Scale(hExpPMT->Integral(kEmin, kEmax) / hMCPMT->Integral(kEmin, kEmax));
		hScan->SetBinContent(i+1, chi2Diff(hExp, hMC, kEmin, kEmax));
		hScanSiPM->SetBinContent(i+1, chi2Diff(hExpSiPM, hMCSiPM, kEmin, kEmax));
		hScanPMT->SetBinContent(i+1, chi2Diff(hExpPMT, hMCPMT, kEmin, kEmax));
	}
	
	TF1 *fpol2 = new TF1("fpol2", "pol2", -10, 10);
	
	gStyle->SetOptStat(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	cv->Clear();
	cv->Divide(3, 2);

	cv->cd(4);
	hScan->Fit(fpol2, "q");
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	if (xmin < 0.95) xmin = 0.95;
	if (xmin > 1.05) xmin = 1.05;
	hScan->Fit(fpol2, "", "", xmin - 0.05, xmin + 0.05);
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);
	// Get MC hists for plots
	i = (xmin - 0.8975) / 0.005;
	if (i < 0) i = 0;
	if (i > 40) i = 40;
	sprintf(str, "hMC_%2.2d", i);
	hMC = (TH1D *) fMC->Get(str);
	sprintf(str, "hMCSiPM_%2.2d", i);
	hMCSiPM = (TH1D *) fMC->Get(str);
	sprintf(str, "hMCPMT_%2.2d", i);
	hMCPMT = (TH1D *) fMC->Get(str);

	cv->cd(5);
	hScanSiPM->Fit(fpol2, "q");
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	if (xmin < 0.95) xmin = 0.95;
	if (xmin > 1.05) xmin = 1.05;
	hScanSiPM->Fit(fpol2, "", "", xmin - 0.05, xmin + 0.05);
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);

	cv->cd(6);
	hScanPMT->Fit(fpol2, "q");
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	if (xmin < 0.95) xmin = 0.95;
	if (xmin > 1.05) xmin = 1.05;
	hScanPMT->Fit(fpol2, "", "", xmin - 0.05, xmin + 0.05);
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);

	cv->cd(1);
	hExp->SetMarkerColor(kRed);
	hExp->SetMarkerStyle(kFullCircle);
	hExp->SetLineColor(kRed);
	hExp->DrawCopy();
	hMC->SetLineWidth(2);
	hMC->SetLineColor(kBlue);
	hMC->DrawCopy("same,hist");
	TLegend *lg = new TLegend(0.65, 0.55, 0.89, 0.7);
	lg->AddEntry(hExp, "Data", "pe");
	lg->AddEntry(hMC, "MC", "l");
	lg->Draw();

	cv->cd(2);
	hExpSiPM->SetMarkerColor(kRed);
	hExpSiPM->SetMarkerStyle(kFullCircle);
	hExpSiPM->SetLineColor(kRed);
	hExpSiPM->DrawCopy();
	hMCSiPM->SetLineWidth(2);
	hMCSiPM->SetLineColor(kBlue);
	hMCSiPM->DrawCopy("same,hist");
	lg->Draw();

	cv->cd(3);
	hExpPMT->SetMarkerColor(kRed);
	hExpPMT->SetMarkerStyle(kFullCircle);
	hExpPMT->SetLineColor(kRed);
	hExpPMT->DrawCopy();
	hMCPMT->SetLineWidth(2);
	hMCPMT->SetLineColor(kBlue);
	hMCPMT->DrawCopy("same,hist");
	lg->Draw();

	TString fres(resname);
	cv->SaveAs(fres.ReplaceAll(".root", ".png").Data());
	
	TFile *fOut = new TFile(resname, "RECREATE");
	if (!fOut->IsOpen()) return;
	hScan->Write();
	hScanSiPM->Write();
	hScanPMT->Write();
	fOut->Close();
	fExp->Close();
	fMC->Close();
}

// To be called from batch - generate exp and MC files
void cm_capture_energy5(void)
{
//	src_248Cm("cm_14428_14485_8.2.root", "248Cm_mar17_center_8.2.root");
//	src_248Cm("cm_50578_50647_8.2.root", "248Cm_nov18_center_8.2.root");
//	src_248Cm("cm_127720_127772_8.2.root", "248Cm_jun22_center_8.2.root");
//	src_248Cm("cm_14487_14512_8.2.root", "248Cm_mar17_edge_8.2.root");
//	src_248Cm("cm_50875_50947_8.2.root", "248Cm_nov18_edge_8.2.root");
//	src_248Cm("cm_14428_14485_8.2.root cm_50578_50647_8.2.root cm_127720_127772_8.2.root", "248Cm_ALL_center_8.2.root");
//	src_248Cm("cm_14487_14512_8.2.root cm_50875_50947_8.2.root", "248Cm_ALL_edge_8.2.root");
//	src_248CmMCn("/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v6/mc_248Cm_indLY_transcode_rawProc_pedSim_Center1.root "
//		"/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v6/mc_248Cm_indLY_transcode_rawProc_pedSim_Center2.root", "248Cm_MC_center_8.2.root");
//	src_248CmMCn("/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v6/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm1.root "
//		"/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v6/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm2.root", "248Cm_MC_edge_8.2.root");
	src_248CmMCn("/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_Center1.root "
		"/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_Center2.root",
		"248Cm_MC_center_8.2_v10.root");
	src_248CmMCn("/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm1.root "
		"/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/248Cm_neutron/mc_248Cm_indLY_transcode_rawProc_pedSim_92_5_cm2.root",
		"248Cm_MC_edge_8.2_v10.root");
	src_248CmMC("cm_MC_center_8.2_v10.root", "248Cm_MCF_center_8.2_v10.root");
	src_248CmMC("cm_MC_edge_8.2_v10.root", "248Cm_MCF_edge_8.2_v10.root");
}
