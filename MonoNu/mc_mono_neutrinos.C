#define NBINS 64

double asym_gaus(double *x, double *par)
{
	double E = x[0];
	double C = par[0];
	double M = par[1];
	double S1 = par[2];
	double S2 = par[3];
	
	double S = (E > M) ? S2 : S1;
	
	return C * exp(-0.5 * (E - M)*(E - M) / (S*S));
}

void mc_mono_neutrinos(const char *name)
{
	double Enu;
	char strF[1024];
	char strH[1024];
	char strT[1024];
	double e[NBINS];	// energy
	double er[NBINS];	// mean from fit
	double Mean[NBINS];	// mean from the distribution
	double RMS[NBINS];	// RMS of the distribution
	double eeMean[NBINS];	// mean from the distribution, error
	double dr[NBINS];	// difference
	double es[NBINS];	// sigma
	double se[NBINS];	// sqrt(energy)
	double ee[NBINS];	// energy error
	double ere[NBINS];	// mean error
	double ese[NBINS];	// sigma error
	int i;
	TH1D *h[NBINS];
	double elow, ehigh;
	TF1 *fg;

	gStyle->SetOptStat("emr");
	gStyle->SetOptFit(1);
	TString sname(name);
	TFile *fIn = new TFile((sname + ".root").Data());
	TH2D *h2PE = (TH2D*) fIn->Get("h2PE");
	if(!h2PE) return;
	
	TF1 *aGaus = new TF1("aGaus", asym_gaus, 0, 20, 4);
	aGaus->SetParNames("Const.", "Mean", "#sigma_{L}", "#sigma_{H}");
	
	TCanvas *cv = new TCanvas("CV", "CV", 800, 1000);
	cv->SaveAs((sname+".pdf[").Data());
	
	for (i = 0; i < NBINS; i++) {
		cv->Clear();
		e[i] = h2PE->GetXaxis()->GetBinCenter(i+1);
		se[i] = sqrt(e[i]);
		ee[i] = 0;
		sprintf(strH, "hPE_%d", i);
		sprintf(strT, "Reconstructed positron energy for MC neutrinos at %7.4f MeV;E, MeV", Enu);
		h[i] = h2PE->ProjectionY(strH, i+1, i+1);
		aGaus->SetParameters(h[i]->GetMaximum(), h[i]->GetMean(), h[i]->GetRMS(), h[i]->GetRMS());
		h[i]->Fit("aGaus");
		er[i] = aGaus->GetParameter(1);
		es[i] = (aGaus->GetParameter(2) + aGaus->GetParameter(3))/2; 
		ere[i] = aGaus->GetParError(1);
		ese[i] = aGaus->GetParError(2);
		Mean[i] = h[i]->GetMean();
		RMS[i] = h[i]->GetRMS();
		eeMean[i] = RMS[i] / sqrt(h[i]->GetEntries());
		cv->SaveAs((sname+".pdf").Data());
	}

	h2PE->SetStats(0);
	h2PE->Draw("colorz");
	cv->SaveAs((sname+".pdf").Data());

	cv->Clear();
	cv->Divide(1, 2);
	cv->cd(1);
	TH1D *hG = new TH1D("hG", "Cluster energy;E_{#nu}, MeV;E_{cluster}, MeV", NBINS, 1.81, 9.81);
	hG->SetMinimum(-0.5);
	hG->SetMaximum(12.5);
	hG->SetStats(0);
	hG->Draw();
	TLegend *lg = new TLegend(0.5, 0.2, 0.8, 0.3);
	TGraphErrors *gE = new TGraphErrors(NBINS, e, er, ee, ere);
	gE->SetMarkerStyle(kFullCircle);
	gE->SetMarkerColor(kBlue);
	gE->Draw("PE");
	gE->Fit("pol1", "", "", 2.8, 9.8);
	lg->AddEntry(gE, "Data", "PE");
	fg = gE->GetFunction("pol1");
	for (i=0; i<NBINS; i++) dr[i] = 10*(er[i] - fg->Eval(e[i]));
	TGraph *gER = new TGraph(NBINS, e, dr);
	gER->SetMarkerStyle(kFullCircle);
	gER->SetMarkerColor(kRed);
	gER->Draw("P");
	lg->AddEntry(gER, "Difference x10", "P");
	lg->Draw();
	gE->Write();

	cv->cd(2);
	TH1D *hGG = new TH1D("hS", "#sigma;#sqrt{E}, MeV;#sigma,MeV", 12, 0, 3.5);
	hGG->SetMinimum(0);
	hGG->SetMaximum(1.5);
	hGG->SetStats(0);
	TGraphErrors *gEE = new TGraphErrors(NBINS, se, es, ee, ese);
	gEE->SetMarkerStyle(kCircle);
	gEE->SetMarkerColor(kGreen);
	gEE->SetTitle("#sigma;#sqrt{E}, MeV;#sigma,MeV");
	hGG->Draw();
	gEE->Draw("PE");
	gEE->Fit("pol1", "", "", 1.7, 3.0);
	cv->SaveAs((sname+".pdf").Data());
	gEE->Write();

	cv->Clear();
	cv->Divide(1, 2);
	cv->cd(1);
	hG->Draw();
	TGraphErrors *gEM = new TGraphErrors(NBINS, e, Mean, ee, eeMean);
	gEM->SetMarkerStyle(kFullCircle);
	gEM->SetMarkerColor(kBlue);
	gEM->Draw("PE");
	gEM->Fit("pol1", "", "", 2.8, 9.8);
	fg = gEM->GetFunction("pol1");
	for (i=0; i<NBINS; i++) dr[i] = 10*(Mean[i] - fg->Eval(e[i]));
	TGraph *gEMR = new TGraph(NBINS, e, dr);
	gEMR->SetMarkerStyle(kFullCircle);
	gEMR->SetMarkerColor(kRed);
	gEMR->Draw("P");
	lg->Draw();
	gEMR->Write();

	cv->cd(2);
	TGraphErrors *gEEM = new TGraphErrors(NBINS, se, RMS, ee, eeMean);
	gEEM->SetMarkerStyle(kCircle);
	gEEM->SetMarkerColor(kGreen);
	gEEM->SetTitle("#sigma;#sqrt{E}, MeV;#sigma,MeV");
	hGG->Draw();
	gEEM->Draw("PE");
	gEEM->Fit("pol1", "", "", 1.7, 3.0);
	cv->SaveAs((sname+".pdf").Data());
	gEEM->Write();

	cv->SaveAs((sname+".pdf]").Data());
	
	delete cv;
}
