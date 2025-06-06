void draw_XYZ(const char *prefix)
{
	int i;
	char str[1024];
	TFile *fIn;
	double x[16], ex[16], y[16], ey[16], z[16], ez[16];
	double xx[16], exx[16];
	TH1D *h;
	
	gStyle->SetOptFit(1);
	gStyle->SetOptStat(1001100);
	TF1 *fGauss = new TF1("fGauss", "gaus", -100.0, 100.0);
	TCanvas *cv = new TCanvas("CV", "CV", 1000, 1000);
	TString pdf(prefix);
	pdf += ".pdf";
	cv->SaveAs((pdf + "[").Data());
	for (i=0; i<16; i++) {
		sprintf(str, "%s_%6.6d_%6.6d.root", prefix, 2210 + 10000*i, 12209 + 10000*i);
		fIn = new TFile(str);
		if (!fIn->IsOpen()) continue;
		cv->Clear();
		cv->Divide(2, 2);
		cv->cd(1);
		h = (TH1D *) fIn->Get("hResX");
		h->Fit(fGauss);
		x[i] = fGauss->GetParameter(1);
		ex[i] = fGauss->GetParError(1);
		cv->cd(2);
		h = (TH1D *) fIn->Get("hResY");
		h->Fit(fGauss);
		y[i] = fGauss->GetParameter(1);
		ey[i] = fGauss->GetParError(1);
		cv->cd(3);
		h = (TH1D *) fIn->Get("hResZ");
		h->Fit(fGauss);
		z[i] = fGauss->GetParameter(1);
		ez[i] = fGauss->GetParError(1);
		cv->SaveAs(pdf);
		xx[i] = i + 0.5;
		exx[i] = 0;
	}
	TGraphErrors *gX = new TGraphErrors(16, xx, x, exx, ex);
	gX->SetLineColor(kRed);
	gX->SetMarkerColor(kRed);
	gX->SetMarkerStyle(kFullTriangleDown);
	gX->SetMarkerSize(1.5);
	TGraphErrors *gY = new TGraphErrors(16, xx, y, exx, ey);
	gY->SetLineColor(kGreen);
	gY->SetMarkerColor(kGreen);
	gY->SetMarkerStyle(kFullTriangleUp);
	gY->SetMarkerSize(1.5);
	TGraphErrors *gZ = new TGraphErrors(16, xx, z, exx, ez);
	gZ->SetLineColor(kBlue);
	gZ->SetMarkerColor(kBlue);
	gZ->SetMarkerStyle(kFullSquare);
	gZ->SetMarkerSize(1.5);
	gStyle->SetOptFit(0);
	cv->Clear();
	gX->SetMaximum(3);
	gX->SetMinimum(-2);
	gX->Draw("ape");
	TF1 *fLinX = new TF1("fLinX", "pol0", 0, 16);
	fLinX->SetLineColor(kRed);
	gX->Fit(fLinX);
	gY->Draw("pe");
	TF1 *fLinY = new TF1("fLinY", "pol0", 0, 16);
	fLinY->SetLineColor(kGreen);
	gY->Fit(fLinY);
	gZ->Draw("pe");
	TF1 *fLinZ = new TF1("fLinZ", "pol0", 0, 16);
	fLinZ->SetLineColor(kBlue);
	gZ->Fit(fLinZ);
	TLegend *lg = new TLegend(0.6, 0.75, 0.89, 0.89);
	sprintf(str, "X: %f #pm %f", fLinX->GetParameter(0), fLinX->GetParError(0));
	lg->AddEntry(gX, str, "lep");
	sprintf(str, "Y: %f #pm %f", fLinY->GetParameter(0), fLinY->GetParError(0));
	lg->AddEntry(gY, str, "lep");
	sprintf(str, "Z: %f #pm %f", fLinZ->GetParameter(0), fLinZ->GetParError(0));
	lg->AddEntry(gZ, str, "lep");
	lg->Draw();
	cv->Update();
	cv->SaveAs(pdf);
	cv->SaveAs((pdf + "]").Data());
}

#define MAXMCPERIODS	50

void draw_XYZMC(const char *prefix, int num)
{
	int i;
	char str[1024];
	TFile *fIn;
	double x[MAXMCPERIODS], ex[MAXMCPERIODS], y[MAXMCPERIODS], ey[MAXMCPERIODS], z[MAXMCPERIODS], ez[MAXMCPERIODS];
	double xx[MAXMCPERIODS], exx[MAXMCPERIODS];
	TH1D *h;
	
	if (num > MAXMCPERIODS) {
		printf("Increase MAXMCPERIODS, please!\n");
		return;
	}
	
	gStyle->SetOptFit(1);
	gStyle->SetOptStat(1001100);
	TF1 *fGauss = new TF1("fGauss", "gaus", -100.0, 100.0);
	TCanvas *cv = new TCanvas("CV", "CV", 1000, 1000);
	TString pdf(prefix);
	pdf += ".pdf";
	cv->SaveAs((pdf + "[").Data());
	for (i=0; i<num; i++) {
		sprintf(str, "%s_%d.root", prefix, i);
		fIn = new TFile(str);
		if (!fIn->IsOpen()) continue;
		cv->Clear();
		cv->Divide(2, 2);
		cv->cd(1);
		h = (TH1D *) fIn->Get("hResX");
		h->Fit(fGauss);
		x[i] = fGauss->GetParameter(1);
		ex[i] = fGauss->GetParError(1);
		cv->cd(2);
		h = (TH1D *) fIn->Get("hResY");
		h->Fit(fGauss);
		y[i] = fGauss->GetParameter(1);
		ey[i] = fGauss->GetParError(1);
		cv->cd(3);
		h = (TH1D *) fIn->Get("hResZ");
		h->Fit(fGauss);
		z[i] = fGauss->GetParameter(1);
		ez[i] = fGauss->GetParError(1);
		cv->SaveAs(pdf);
		xx[i] = i + 0.5;
		exx[i] = 0;
	}
	TGraphErrors *gX = new TGraphErrors(num, xx, x, exx, ex);
	gX->SetLineColor(kRed);
	gX->SetMarkerColor(kRed);
	gX->SetMarkerStyle(kFullTriangleDown);
	gX->SetMarkerSize(1.5);
	TGraphErrors *gY = new TGraphErrors(num, xx, y, exx, ey);
	gY->SetLineColor(kGreen);
	gY->SetMarkerColor(kGreen);
	gY->SetMarkerStyle(kFullTriangleUp);
	gY->SetMarkerSize(1.5);
	TGraphErrors *gZ = new TGraphErrors(num, xx, z, exx, ez);
	gZ->SetLineColor(kBlue);
	gZ->SetMarkerColor(kBlue);
	gZ->SetMarkerStyle(kFullSquare);
	gZ->SetMarkerSize(1.5);
	gStyle->SetOptFit(0);
	cv->Clear();
	gX->SetMaximum(3);
	gX->SetMinimum(-2);
	gX->Draw("ape");
	TF1 *fLinX = new TF1("fLinX", "pol0", 0, num);
	fLinX->SetLineColor(kRed);
	gX->Fit(fLinX);
	gY->Draw("pe");
	TF1 *fLinY = new TF1("fLinY", "pol0", 0, num);
	fLinY->SetLineColor(kGreen);
	gY->Fit(fLinY);
	gZ->Draw("pe");
	TF1 *fLinZ = new TF1("fLinZ", "pol0", 0, num);
	fLinZ->SetLineColor(kBlue);
	gZ->Fit(fLinZ);
	TLegend *lg = new TLegend(0.6, 0.75, 0.89, 0.89);
	sprintf(str, "X: %f #pm %f", fLinX->GetParameter(0), fLinX->GetParError(0));
	lg->AddEntry(gX, str, "lep");
	sprintf(str, "Y: %f #pm %f", fLinY->GetParameter(0), fLinY->GetParError(0));
	lg->AddEntry(gY, str, "lep");
	sprintf(str, "Z: %f #pm %f", fLinZ->GetParameter(0), fLinZ->GetParError(0));
	lg->AddEntry(gZ, str, "lep");
	lg->Draw();
	cv->Update();
	cv->SaveAs(pdf);
	cv->SaveAs((pdf + "]").Data());
}
