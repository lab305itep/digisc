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

void map4MC(const char *nMap)
{
	int i, j, k, l, m;
	TH2D *hMap;
	char str[1024];
	double sum;
	int cnt;
	struct {			// for Tree
		double cellindex;
		double row;
		double column;
		double energy;
	} SiPM;

	TFile *fMap = new TFile(nMap);
	if (!fMap->IsOpen()) return;
	
	TFile *fOut = new TFile("PMT_strip_map.root", "RECREATE");
	TTree *tOut = new TTree("StripTree", "StripTree");
	tOut->Branch("StripBranch", &SiPM, "cellindex/D:row:column:lightcorrection");
	
	for (i=0; i<2; i++) for (j=0; j<5; j++) for (k=0; k<5; k++) {
		sprintf(str, "hEAvr_%c_z%dxy%d", (i) ? 'X' : 'Y', j, k);
		hMap = (TH2D*) fMap->Get(str);
		if (!hMap) {
			printf("%s not found\n", str);
			return;
		}
		sum = 0;
		cnt = 0;
		for (l=0; l<10; l++) for (m=0; m<5; m++) {
			sum += hMap->GetBinContent(m+1, l+1);
			if (hMap->GetBinContent(m+1, l+1) > 0) cnt++;
		}
		sum = (cnt) ? sum / cnt : 1.0;
		
		for (l=0; l<10; l++) for (m=0; m<5; m++) {
			SiPM.energy = hMap->GetBinContent(m+1, l+1) / sum;
			if (SiPM.energy == 0) SiPM.energy = 1.0;
			SiPM.row = 20*j + 2*l + i;
			SiPM.column = 5*k + m;
			SiPM.cellindex = 1250*i + 250*j + 25*l + 24 - (5*k + m);
			tOut->Fill();
		}
	}
	tOut->Write();
	fOut->Close();
	fMap->Close();
}

void sum_maps(const char *iNames, const char *oName)
{
	char str[1024];
	char strl[4096];
	const char *ptr;
	char *pend;
	int i, j, k, l, m, n;
	TH1D *h[2][5][5][10][5];	// side, z_PMT, xy_PMT, z_SiPM in PMT, xy_SiPM in PMT
	TH1D *hAvr[10][5];		// z_SiPM in PMT, xy_SiPM in PMT
	TH2D *hMapPMTX;
	TH2D *hMapPMTY;
	TH2D *hMapSiPM;
	TH2D *hMapSiPMX;
	TH2D *hMapSiPMY;
	TH2D *hEFit[2][5][5];	// Gaus fit values per PMT
	TH2D *hAvrEFit;		// Average Gaus fit values
	TH2D *hEAvr[2][5][5];	// Average ratio values per PMT
	TH2D *hAvrEAvr;		// Average map of average values
	double mean, sigma, low, high;
	TFile *fIn;
	TH1D *h1;
	TH2D *h2;

	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<10; m++) for (n=0; n<5; n++) {
		sprintf(str, "hRatio_%c_z%dxy%d_z%dxy%d", (j) ? 'X' : 'Y', k, l, m , n);
		sprintf(strl, "PMT/SiPM for side %c PMT z%dxy%d SiPM z%dxy%d", (j) ? 'X' : 'Y', k, l, m , n);
		h[j][k][l][m][n] = new TH1D(str, strl, 116, 0.1, 3);
	}
	for (m=0; m<10; m++) for (n=0; n<5; n++) {
		sprintf(str, "hRatio_Avr_z%dxy%d", m , n);
		sprintf(strl, "Average PMT/SiPM SiPM z%dxy%d", m , n);
		hAvr[m][n] = new TH1D(str, strl, 116, 0.1, 3);
	}

	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) {
		sprintf(str, "hEAvr_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
		sprintf(strl, "Average Ratio map for side %c PMT z%dxy%d", (j) ? 'X' : 'Y', k, l);
		hEAvr[j][k][l] = new TH2D(str, strl, 5, 0, 5, 10, 0, 10);
		sprintf(str, "hEFit_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
		sprintf(strl, "Fit Ratio map for side %c PMT z%dxy%d", (j) ? 'X' : 'Y', k, l);
		hEFit[j][k][l] = new TH2D(str, strl, 5, 0, 5, 10, 0, 10);
	}
	
	hMapPMTX = new TH2D("hMapPMTX", "PMT fill map XZ", 5, 0, 5, 5, 0, 5);
	hMapPMTY = new TH2D("hMapPMTY", "PMT fill map YZ", 5, 0, 5, 5, 0, 5);
	hMapSiPM = new TH2D("hMapSiPM", "SiPM fill map XYZ", 5, 0, 5, 10, 0, 10);
	hMapSiPMX = new TH2D("hMapSiPMX", "SiPM fill map XZ", 25, 0, 25, 50, 0, 50);
	hMapSiPMY = new TH2D("hMapSiPMY", "SiPM fill map YZ", 25, 0, 25, 50, 0, 50);
	hAvrEFit = new TH2D("hAvrEFit", "Fit ratio map over all PMTs", 5, 0, 5, 10, 0, 10);
	hAvrEAvr = new TH2D("hAvrEAvr", "Averge ratio map over all PMTs", 5, 0, 5, 10, 0, 10);
	
	ptr = iNames;
	for(;;) {
		ptr = strchr(ptr, ':');
		if(!ptr) break;
		ptr++;
		strcpy(strl, ptr);
		pend = strchr(strl, ':');
		if (pend) *pend = '\0';
		fIn = new TFile(strl);
		if (!fIn->IsOpen()) break;
		printf("%s\n", fIn->GetName());
		for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<10; m++) for (n=0; n<5; n++) {
			sprintf(str, "hRatio_%c_z%dxy%d_z%dxy%d", (j) ? 'X' : 'Y', k, l, m , n);
			h1 = (TH1D*) fIn->Get(str);
			if (h1) h[j][k][l][m][n]->Add(h1);
		}
		for (m=0; m<10; m++) for (n=0; n<5; n++) {
			sprintf(str, "hRatio_Avr_z%dxy%d", m , n);
			h1 = (TH1D*) fIn->Get(str);
			if (h1) hAvr[m][n]->Add(h1);
		}

		for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) {
			sprintf(str, "hEAvr_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
			h2 = (TH2D*) fIn->Get(str);
			if (h2) hEAvr[j][k][l]->Add(h2);
			sprintf(str, "hEFit_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
			h2 = (TH2D*) fIn->Get(str);
			if (h2) hEFit[j][k][l]->Add(h2);
		}
		h2 = (TH2D*) fIn->Get("hMapPMTX");
		if (h2) hMapPMTX->Add(h2);
		h2 = (TH2D*) fIn->Get("hMapPMTY");
		if (h2) hMapPMTY->Add(h2);
		h2 = (TH2D*) fIn->Get("hMapSiPM");
		if (h2) hMapSiPM->Add(h2);
		h2 = (TH2D*) fIn->Get("hMapSiPMX");
		if (h2) hMapSiPMX->Add(h2);
		h2 = (TH2D*) fIn->Get("hMapSiPMY");
		if (h2) hMapSiPMY->Add(h2);
		h2 = (TH2D*) fIn->Get("hAvrEFit");
		if (h2) hAvrEFit->Add(h2);
		h2 = (TH2D*) fIn->Get("hAvrEAvr");
		if (h2) hAvrEAvr->Add(h2);

		fIn->Close();
	}
	
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<10; m++) for (n=0; n<5; n++) 
		if (h[j][k][l][m][n]->GetEntries() > 1000) {
		mean = h[j][k][l][m][n]->GetMean();
		sigma = h[j][k][l][m][n]->GetMeanError();
		hEAvr[j][k][l]->SetBinContent(n+1, m+1, mean);
		hEAvr[j][k][l]->SetBinError(n+1, m+1, sigma);
		h[j][k][l][m][n]->Fit("gaus", "Q0", "", 0.2, 1.6);
		mean = h[j][k][l][m][n]->GetFunction("gaus")->GetParameter(1);
		sigma = h[j][k][l][m][n]->GetFunction("gaus")->GetParameter(2);
		low  = mean - sigma;
		high = mean + sigma;
		if (low < 0.1) low = 0.1;
		if (high > 2) high = 2;
		h[j][k][l][m][n]->Fit("gaus", "Q0", "", low, high);
		mean = h[j][k][l][m][n]->GetFunction("gaus")->GetParameter(1);
		sigma = h[j][k][l][m][n]->GetFunction("gaus")->GetParError(1);
		hEFit[j][k][l]->SetBinContent(n+1, m+1, mean);
		hEFit[j][k][l]->SetBinError(n+1, m+1, sigma);
	}

	for (m=0; m<10; m++) for (n=0; n<5; n++) if (hAvr[m][n]->GetEntries() > 1000) {
		mean = hAvr[m][n]->GetMean();
		sigma = hAvr[m][n]->GetMeanError();
		hAvrEAvr->SetBinContent(n+1, m+1, mean);
		hAvrEAvr->SetBinError(n+1, m+1, sigma);
		hAvr[m][n]->Fit("gaus", "Q0", "", 0.2, 1.6);
		mean = hAvr[m][n]->GetFunction("gaus")->GetParameter(1);
		sigma = hAvr[m][n]->GetFunction("gaus")->GetParameter(2);
		low  = mean - sigma;
		high = mean + sigma;
		if (low < 0.1) low = 0.1;
		if (high > 2) high = 2;
		hAvr[m][n]->Fit("gaus", "Q0", "", low, high);
		mean = hAvr[m][n]->GetFunction("gaus")->GetParameter(1);
		sigma = hAvr[m][n]->GetFunction("gaus")->GetParError(1);
		hAvrEFit->SetBinContent(n+1, m+1, mean);
		hAvrEFit->SetBinError(n+1, m+1, sigma);
	}
	
	TFile *fOut = new TFile(oName, "RECREATE");
	if (!fOut->IsOpen()) return;

	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) for (m=0; m<10; m++) for (n=0; n<5; n++) 
		if (h[j][k][l][m][n]->GetEntries()) h[j][k][l][m][n]->Write();
	for (m=0; m<10; m++) for (n=0; n<5; n++) 
		if (hAvr[m][n]->GetEntries()) hAvr[m][n]->Write();
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) hEFit[j][k][l]->Write();
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) hEAvr[j][k][l]->Write();
	hMapPMTX->Write();
	hMapPMTY->Write();
	hMapSiPM->Write();
	hMapSiPMX->Write();
	hMapSiPMY->Write();
	hAvrEAvr->Write();
	hAvrEFit->Write();
	
	fOut->Close();

}

void sum_ibd(void) 
{
	sum_maps("ibd_2200_32199.root:ibd_32200_62199.root:ibd_62200_92199.root:ibd_92200_122199.root:"
		"ibd_122200_152199.root:ibd_152200_182199.root", "ibd_2200_182199.root");
}

void sum_hits(void)
{
	sum_maps("map_2200_12199.root:" "map_12200_22199.root:" "map_22200_32199.root:" "map_32200_42199.root:"
		"map_42200_52199.root:" "map_52200_62199.root:" "map_62200_72199.root:" "map_72200_82199.root:"
		"map_82200_92199.root:" "map_92200_102199.root", "map_2200_102199.root");
}

void sum_muon(void)
{
	sum_maps("muon_2200_12199.root:" "muon_12200_22199.root:" "muon_22200_32199.root:" "muon_32200_42199.root:"
		"muon_42200_52199.root:" "muon_52200_62199.root:" "muon_62200_72199.root:" "muon_72200_82199.root:"
		"muon_82200_92199.root:" "muon_92200_102199.root", "muon_2200_102199.root");
}


// Makes and draws a histogram of average map differences
void cmp_maps(const char *mapA, const char *mapB, double scale = 1)
{
	TH2D *hA;
	TH2D *hB;
	char str[1024];
	int j, k, l, m, n;
	
	TFile *fA = new TFile(mapA);
	TFile *fB = new TFile(mapB);
	if (!fA->IsOpen() || !fB->IsOpen()) {
		printf("Can not open files\n");
		return;
	}
	
	TH1D *hDiff = new TH1D("hDiff", "Map difference", 100, -0.1, 0.1);
	
	for (j=0; j<2; j++) for (k=0; k<5; k++) for(l=0; l<5; l++) {
		sprintf(str, "hEAvr_%c_z%dxy%d", (j) ? 'X' : 'Y', k, l);
		hA = (TH2D*) fA->Get(str);
		hB = (TH2D*) fB->Get(str);
		if (!hA || !hB) {
			printf("%s not found\n", str);
			continue;
		}
		for (m=0; m<10; m++) for (n=0; n<5; n++) hDiff->Fill(hA->GetBinContent(n+1, m+1) - scale*hB->GetBinContent(n+1, m+1));
	}
	
	hDiff->Draw();
}
