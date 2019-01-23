void drawevent(long long gTime)
{
	TLine ln;
	TText txt;
	char str[1024];
	double fineTime, SiPmEnergy, PmtEnergy, VetoEnergy, PositronEnergy;
	int SiPmHits, PmtHits, VetoHits, PositronHits;
	double X[3], McX[3];
	int i;
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	sprintf(str, "evt_%Ld.root", gTime);
	TFile *f = new TFile(str);
	if (!f->IsOpen()) return;
	
	TH2D *SiPmX = (TH2D*) f->Get("hSiPmX");
	TH2D *SiPmY = (TH2D*) f->Get("hSiPmY");
	TH2D *SiPmCleanX = (TH2D*) f->Get("hSiPmCleanX");
	TH2D *SiPmCleanY = (TH2D*) f->Get("hSiPmCleanY");
	TH2D *PmtX  = (TH2D*) f->Get("hPmtX");
	TH2D *PmtY  = (TH2D*) f->Get("hPmtY");
	TH1D *Veto  = (TH1D*) f->Get("hVeto");
	TH1D *Time  = (TH1D*) f->Get("hTime");
	TH1D *TimeClean  = (TH1D*) f->Get("hTimeClean");
	TH1D *Par   = (TH1D*) f->Get("hPar");

	if (!(SiPmX && SiPmY && SiPmCleanX && SiPmCleanY && PmtX && PmtY && Veto && Time && TimeClean && Par)) {
		printf("Hists not found.\n");
		f->Close();
		return;
	}

	fineTime = Par->GetBinContent(1);
	SiPmEnergy = Par->GetBinContent(2);
	PmtEnergy = Par->GetBinContent(3);
	VetoEnergy = Par->GetBinContent(4);
	PositronEnergy = Par->GetBinContent(5);
	SiPmHits = Par->GetBinContent(6);
	PmtHits = Par->GetBinContent(7);
	VetoHits = Par->GetBinContent(8);
	PositronHits = Par->GetBinContent(9);
	for (i=0; i<3; i++) X[i] = Par->GetBinContent(i+10);
	if (X[0] >= 0) X[0] += 2.0;
	if (X[1] >= 0) X[1] += 2.0;
	X[2] += 0.5;
	for (i=0; i<3; i++) McX[i] = Par->GetBinContent(i+13);

	TCanvas *cv = new TCanvas("CV", "Event draw", 1600, 1800);
	
	cv->Divide(2, 3);

	cv->cd(1)->SetGrid();
	PmtX->SetTitle("All hits;X, cm;Z, cm");
	PmtX->SetMinimum(0);
	PmtX->SetMaximum(8.0);
	PmtX->SetLineWidth(1);
	PmtX->SetLineColor(kRed);
	SiPmX->SetMinimum(0);
	SiPmX->SetMaximum(5.0);
	PmtX->DrawCopy("box");
	SiPmX->Draw("colorz,same");
	
	cv->cd(2)->SetGrid();
	PmtY->SetTitle("All hits;Y, cm;Z, cm");
	PmtY->SetMinimum(0);
	PmtY->SetMaximum(8.0);
	PmtY->SetFillStyle(3013);
	PmtY->SetLineWidth(1);
	PmtY->SetLineColor(kRed);
	SiPmY->SetMinimum(0);
	SiPmY->SetMaximum(5.0);
	PmtY->DrawCopy("box");
	SiPmY->Draw("colorz,same");
	
	cv->cd(3)->SetGrid();
	PmtX->SetTitle(str);
	PmtX->SetTitle("Clean hits;X, cm;Z, cm");
	PmtX->SetLineWidth(1);
	PmtX->SetLineColor(kRed);
	SiPmCleanX->SetMinimum(0);
	SiPmCleanX->SetMaximum(5.0);
	PmtX->DrawCopy("box");
	SiPmCleanX->Draw("colorz,same");
	
	cv->cd(4)->SetGrid();
	PmtY->SetTitle("Clean hits;Y, cm;Z, cm");
	PmtY->SetLineWidth(1);
	PmtY->SetLineColor(kRed);
	SiPmCleanY->SetMinimum(0);
	SiPmCleanY->SetMaximum(5.0);
	PmtY->DrawCopy("box");
	SiPmCleanY->Draw("colorz,same");

	TVirtualPad *pd = cv->cd(5);
	pd->SetBottomMargin(0.5);
	pd->SetGrid();
	Veto->SetTitle("VETO;Chan;MeV");
	Veto->SetLineColor(kOrange);
	Veto->SetFillColor(kOrange);
	Veto->SetFillStyle(1001);
	Veto->Draw("hist");

	sprintf(str, "SiPM \t%6.2f MeV / %d hits", SiPmEnergy, SiPmHits);
	txt.DrawTextNDC(0.05, 0.40, str);
	sprintf(str, "PMT \t%6.2f MeV / %d hits", PmtEnergy, PmtHits);
	txt.DrawTextNDC(0.05, 0.33, str);
	sprintf(str, "VETO \t%6.2f MeV / %d hits", VetoEnergy, VetoHits);
	txt.DrawTextNDC(0.05, 0.26, str);
	sprintf(str, "Cluster \t%6.2f MeV / %d hits", PositronEnergy, PositronHits);
	txt.DrawTextNDC(0.05, 0.19, str);
	sprintf(str, "XYZ \t%5.1f %5.1f %5.1f cm", X[0], X[1], X[2]);
	txt.DrawTextNDC(0.05, 0.12, str);
	if (McX[0] >= 0) {
		sprintf(str, "MC XYZ \t%5.1f %5.1f %5.1f cm", McX[0], McX[1], McX[2]);
		txt.DrawTextNDC(0.05, 0.05, str);
	}

	cv->cd(6)->SetGrid();
	Time->SetLineColor(kBlue);
	Time->SetFillColor(kBlue);
	Time->SetFillStyle(1001);
	TimeClean->SetLineColor(kRed);
	TimeClean->SetFillColor(kRed);
	TimeClean->SetFillStyle(1001);
	Time->Draw("hist");
	TimeClean->Draw("hist,same");
	ln.SetLineWidth(2);
	ln.SetLineColor(kBlack);
	ln.DrawLine(fineTime, 0, fineTime, Time->GetMaximum());
	
	sprintf(str, "evt_%Ld.pdf", gTime);
	cv->SaveAs(str);

}
