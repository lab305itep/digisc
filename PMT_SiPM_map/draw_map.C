void draw_map(TFile *f, char side, int z, int xy)
{
	char str[256];
	TH2D *hSiPM;
	TH2D *hEFit;
	TH2D *hEAvr;
	TCanvas *cv;
	
	if (!f || !f->IsOpen()) {
		printf("File is not opened.\n");
		return;
	}
	if (z < 0 || z > 4 || xy < 0 || xy > 4 
		|| (toupper(side) != 'X' && toupper(side) != 'Y')) {
		printf("Bad PMT position: %c%d.%d\n", side, z, xy);
		return;
	}
	if (toupper(side) == 'X') {
		hSiPM = (TH2D*) f->Get("hMapSiPMX");
	} else {
		hSiPM = (TH2D*) f->Get("hMapSiPMY");
	}
	sprintf(str, "hEFit_%c_z%dxy%d", side, z, xy);
	hEFit = (TH2D*) f->Get(str);
	hEFit->GetXaxis()->SetTitle("XY");
	hEFit->GetYaxis()->SetTitle("Z");
	hEFit->GetZaxis()->SetTitle("PMT/SiPM");
	hEFit->GetZaxis()->SetRangeUser(0.3, 1.3);
	sprintf(str, "hEAvr_%c_z%dxy%d", side, z, xy);
	hEAvr = (TH2D*) f->Get(str);
	hEAvr->GetXaxis()->SetTitle("XY");
	hEAvr->GetYaxis()->SetTitle("Z");
	hEAvr->GetZaxis()->SetTitle("PMT/SiPM");
	hEAvr->GetZaxis()->SetRangeUser(0.3, 1.3);
	if (!hEFit || !hEAvr || !hSiPM) {
		printf("Histogramms are not found\n");
		return;
	}
	if (gPad) {
		cv = gPad->GetCanvas();
	} else {
		cv = new TCanvas();
	}
	cv->Clear();
	gStyle->SetPadRightMargin(0.15);
	cv->Divide(3, 1);
	gStyle->SetOptStat(0);
	cv->cd(1);
	hEFit->Draw("colorz,text");
	cv->cd(2);
	hEAvr->Draw("colorz,text");
	cv->cd(3);
	hSiPM->GetXaxis()->SetTitle("XY");
	hSiPM->GetYaxis()->SetTitle("Z");
	hSiPM->GetZaxis()->SetTitle("Events");
	hSiPM->GetXaxis()->SetRange(5*xy+1, 5*xy+5);
	hSiPM->GetYaxis()->SetRange(10*z+1, 10*z+10);
	hSiPM->Draw("colorz,text");
}

void draw_average(TFile *f)
{
	TH1D *h[10][5];
	TH2D *hEAvr[2][5][5];
	TH1D *hEE[2][5][5];
	char strs[256];
	char strl[1024];
	int i, j, k, l, m;
	TH2D *hOut;
	TH2D *hPMTX;
	TH2D *hPMTY;
	TCanvas *cv;
	
	for (i=0; i<2; i++) for (j=0; j<5; j++) for (k=0; k<5; k++) {
		sprintf(strs, "hEAvr_%c_z%dxy%d", (i) ? 'X' : 'Y', j, k);
		hEAvr[i][j][k] = (TH2D*) f->Get(strs);
		if (!hEAvr[i][j][k]) {
			printf("Hist %s not found in %s\n", strs, f->GetName());
			return;
		}
	}

	for (i=0; i<2; i++) for (j=0; j<5; j++) for (k=0; k<5; k++) {
		sprintf(strs, "hEE_%c_z%dxy%d", (i) ? 'X' : 'Y', j, k);
		sprintf(strl, "Ratio in PMT %c Z=%d XY=%d", (i) ? 'X' : 'Y', j, k);
		hEE[i][j][k] = new TH1D(strs, strl, 60, 0.3, 1.8);
	}
	for (i=0; i<10; i++) for (j=0; j<5; j++) {
		sprintf(strs, "hR_z%dxy%d", i, j);
		sprintf(strl, "Ratio for SiPM in PMT position Z=%d XY=%d", i, j);
		h[i][j] = new TH1D(strs, strl, 30, 0.3, 1.8);
	}
	
	for (i=0; i<2; i++) for (j=0; j<5; j++) for (k=0; k<5; k++) for (l=0; l<10; l++) for (m=0; m<5; m++) {
		h[l][m]->Fill(hEAvr[i][j][k]->GetBinContent(m+1, l+1));
		hEE[i][j][k]->Fill(hEAvr[i][j][k]->GetBinContent(m+1, l+1));
	}
	hOut = new TH2D("hAverageMap", "Average PMT/SiPM map;XY;Z;PMT/SiPM", 5, 0, 5, 10, 0, 10);
	hPMTX = new TH2D("hPMTX", "Per PMT average PMT/SiPM ratio XZ;X;Z;PMT/SiPM", 5, 0, 5, 5, 0, 5);
	hPMTY = new TH2D("hPMTY", "Per PMT average PMT/SiPM ratio YZ;Y;Z;PMT/SiPM", 5, 0, 5, 5, 0, 5);
	for (i=0; i<10; i++) for (j=0; j<5; j++) {
		hOut->SetBinContent(j+1, i+1, h[i][j]->GetMean());
		hOut->SetBinError(j+1, i+1, h[i][j]->GetMeanError());
	}
	
	for (i=0; i<2; i++) for (j=0; j<5; j++) for (k=0; k<5; k++) {
		if (i) {
			hPMTX->SetBinContent(j+1, k+1, hEE[i][j][k]->GetMean());
			hPMTX->SetBinError(j+1, k+1, hEE[i][j][k]->GetMeanError());
		} else {
			hPMTY->SetBinContent(j+1, k+1, hEE[i][j][k]->GetMean());
			hPMTY->SetBinError(j+1, k+1, hEE[i][j][k]->GetMeanError());
		}
	}
	
	TString pdfname(f->GetName());
	pdfname.ReplaceAll(".root", "");
	pdfname += ".pdf";

	if (gPad) {
		cv = gPad->GetCanvas();
	} else {
		cv = new TCanvas();
	}
	cv->Clear();
	hOut->Draw("colorz,text");
	cv->SaveAs(pdfname.Data());
	
	cv->Clear();
	cv->Divide(2, 1);
	cv->cd(1);
	hPMTX->Draw("colorz,text");
	cv->cd(2);
	hPMTY->Draw("colorz,text");
	cv->SaveAs(pdfname.Data());
	
	TString fname(f->GetName());
	fname.ReplaceAll(".root", "");
	TFile *fOut = new TFile((fname + "-hists.root").Data(), "RECREATE");
	if (!fOut->IsOpen()) return;
	for (i=0; i<10; i++) for (j=0; j<5; j++) h[i][j]->Write();
	for (i=0; i<2; i++) for (j=0; j<5; j++) for (k=0; k<5; k++) hEE[i][j][k]->Write();
	hPMTX->Write();
	hPMTY->Write();
	hOut->Write();
	fOut->Close();
}

void draw_all(const char *fname)
{
	int j, z, xy;
	
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	TString pdfname(fname);
	pdfname.ReplaceAll(".root", "");
	pdfname += ".pdf";
	TCanvas cv("CV", "CV", 1800, 600);
	cv.SaveAs((pdfname+"[").Data());
	for (j=0; j<2; j++) for (z=0; z<5; z++) for (xy=0; xy<5; xy++) {
		draw_map(f, (j) ? 'X' : 'Y', z, xy);
		cv.SaveAs(pdfname.Data());
	}
	draw_average(f);
	cv.SaveAs((pdfname+"]").Data());
	f->Close();
}

void draw_cmp(const char *title, TH1D *hMuon, TH2D *hMap)
{
	TCanvas *cv;
	if (gPad) {
		cv = gPad->GetCanvas();
	} else {
		cv = new TCanvas("CV", "CV", 1000, 1200);
	}
	cv->Clear();
	cv->cd();
	TPad *pd1 = new TPad("pd1", "pd1", 0, 0.35, 1.0, 1.0, kWhite);
	pd1->Draw();
	pd1->cd();
	hMap->SetTitle(title);
	hMap->Draw("colorz,text");
	cv->cd();
	TPad *pd2 = new TPad("pd2", "pd2", 0, 0, 1.0, 0.35, kWhite);
	pd2->Draw();
	pd2->cd();
	TH1D *hPrj = (TH1D *) hMap->ProjectionX();
	hPrj->Scale(0.1);
	hPrj->SetLineColor(kRed);
	hPrj->SetLineWidth(2);
	hPrj->SetMarkerColor(kRed);
	hPrj->SetMarkerSize(0.1);
	hPrj->SetMarkerStyle(kFullSquare);
	hPrj->SetStats(0);
	hMuon->SetLineColor(kBlue);
	hMuon->SetLineWidth(2);
	hMuon->SetMarkerColor(kBlue);
	hMuon->SetMarkerSize(0.1);
	hMuon->SetMarkerStyle(kOpenCircle);
	hMuon->SetStats(0);
	hMuon->SetMinimum(0.6);
	hMuon->SetMaximum(1.2);
	hMuon->SetTitle("");
	hMuon->Draw();
	hPrj->Draw("same");
	TLegend *lg = new TLegend(0.75, 0.8, 0.95, 0.95);
	lg->AddEntry(hPrj, "Single hits", "pl");
	lg->AddEntry(hMuon, "Vertical muons", "pl");
	lg->Draw();
}

void cmp_all(const char *nMuon, const char *nMap)
{
	int i, j, k;
	TH1D *hMuon;
	TH2D *hMap;
	char str[1024];

	gStyle->SetOptStat(0);
	TFile *fMuon = new TFile(nMuon);
	TFile *fMap = new TFile(nMap);
	TString nMapHist(nMap);
	nMapHist.ReplaceAll(".root", "");
	nMapHist += "-hist.root";
	TFile *fMapHist = new TFile(nMapHist.Data());
	if (!fMuon->IsOpen() || !fMap->IsOpen()) return;
	
	TCanvas *cv = new TCanvas("CV", "CV", 1000, 1200);
	TString pdf(nMuon);
	pdf.ReplaceAll(".root", "");
	pdf += "-cmp.pdf";
	cv->SaveAs((pdf + "[").Data());
	for (i=0; i<2; i++) for (j=0; j<5; j++) for (k=0; k<5; k++) {
		sprintf(str, "hEAvr_%c_z%dxy%d", (i) ? 'X' : 'Y', j, k);
		hMap = (TH2D*) fMap->Get(str);
		if (!hMap) {
			printf("%s not found\n", str);
			return;
		}
		sprintf(str, "hmuEAvr_%c_z%dxy%d", (i) ? 'X' : 'Y', j, k);
		hMuon = (TH1D*) fMuon->Get(str);
		if (!hMuon) {
			printf("%s not found\n", str);
			return;
		}
		sprintf(str, "PMT profile %c z=%d xy=%d", (i) ? 'X' : 'Y', j, k);
		draw_cmp(str, hMuon, hMap);
		cv->SaveAs(pdf.Data());
	}
	cv->SaveAs((pdf + "]").Data());
}
