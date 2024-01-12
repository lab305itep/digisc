/* Draw upto 8 plots in a frame histogram	*/
void drawGraphs(TH1D *frame, const char *f0, const char *f1 = NULL, const char *f2 = NULL, const char *f3 = NULL, 
	const char *f4 = NULL, const char *f5 = NULL, const char *f6 = NULL, const char *f7 = NULL)
{
	const int Colors[] = {kRed, kGreen, kBlue, kMagenta, kOrange, kBlack, kYellow, kGray};
	TFile *f;
	TGraphErrors *g;
	
	TF1 *fPol = new TF1("fPol", "[0]*(1-[1]*x/100.0)", 0, 10);
	fPol->SetParNames("Const.", "Decay %/year");
	fPol->SetLineColor(kBlack);
	fPol->SetLineWidth(2);
	
	frame->Draw();
	
	f = new TFile(f0);
	if (!f->IsOpen()) return;
	g = (TGraphErrors*) f->Get("gMedian");
	if (!g) {
		printf("Graph gMedian is not found in %s.\n", f0);
		delete f;
		return;
	}
	g->SetMarkerColor(Colors[0]);
	g->SetMarkerSize(0.7);
	g->SetMarkerStyle(kFullCircle);
	g->Draw("p");
	g->Fit(fPol, "", "", frame->GetXaxis()->GetXmin(), frame->GetXaxis()->GetXmax());
	TLegend *lg = new TLegend(0.11, 0.11, 0.35, 0.35);
	lg->AddEntry(g, f0, "p");
	delete f;
	
	if (f1) {
		f = new TFile(f1);
		if (!f->IsOpen()) return;
		g = (TGraphErrors*) f->Get("gMedian");
		if (!g) {
			printf("Graph gMedian is not found in %s.\n", f1);
			delete f;
			return;
		}
		g->SetMarkerColor(Colors[1]);
		g->SetMarkerSize(0.5);
		g->SetMarkerStyle(kFullCircle);
		g->Draw("p");
		lg->AddEntry(g, f1, "p");
		delete f;
	}
	if (f2) {
		f = new TFile(f2);
		if (!f->IsOpen()) return;
		g = (TGraphErrors*) f->Get("gMedian");
		if (!g) {
			printf("Graph gMedian is not found in %s.\n", f2);
			delete f;
			return;
		}
		g->SetMarkerColor(Colors[2]);
		g->SetMarkerSize(0.5);
		g->SetMarkerStyle(kFullCircle);
		g->Draw("p");
		lg->AddEntry(g, f2, "p");
		delete f;
	}
	if (f3) {
		f = new TFile(f3);
		if (!f->IsOpen()) return;
		g = (TGraphErrors*) f->Get("gMedian");
		if (!g) {
			printf("Graph gMedian is not found in %s.\n", f3);
			delete f;
			return;
		}
		g->SetMarkerColor(Colors[3]);
		g->SetMarkerSize(0.5);
		g->SetMarkerStyle(kFullCircle);
		g->Draw("p");
		lg->AddEntry(g, f3, "p");
		delete f;
	}
	if (f4) {
		f = new TFile(f4);
		if (!f->IsOpen()) return;
		g = (TGraphErrors*) f->Get("gMedian");
		if (!g) {
			printf("Graph gMedian is not found in %s.\n", f4);
			delete f;
			return;
		}
		g->SetMarkerColor(Colors[4]);
		g->SetMarkerSize(0.5);
		g->SetMarkerStyle(kFullCircle);
		g->Draw("p");
		lg->AddEntry(g, f4, "p");
		delete f;
	}
	if (f5) {
		f = new TFile(f5);
		if (!f->IsOpen()) return;
		g = (TGraphErrors*) f->Get("gMedian");
		if (!g) {
			printf("Graph gMedian is not found in %s.\n", f5);
			delete f;
			return;
		}
		g->SetMarkerColor(Colors[5]);
		g->SetMarkerSize(0.5);
		g->SetMarkerStyle(kFullCircle);
		g->Draw("p");
		lg->AddEntry(g, f5, "p");
		delete f;
	}
	if (f6) {
		f = new TFile(f6);
		if (!f->IsOpen()) return;
		g = (TGraphErrors*) f->Get("gMedian");
		if (!g) {
			printf("Graph gMedian is not found in %s.\n", f6);
			delete f;
			return;
		}
		g->SetMarkerColor(Colors[6]);
		g->SetMarkerSize(0.5);
		g->SetMarkerStyle(kFullCircle);
		g->Draw("p");
		lg->AddEntry(g, f6, "p");
		delete f;
	}
	if (f7) {
		f = new TFile(f7);
		if (!f->IsOpen()) return;
		g = (TGraphErrors*) f->Get("gMedian");
		if (!g) {
			printf("Graph gMedian is not found in %s.\n", f7);
			delete f;
			return;
		}
		g->SetMarkerColor(Colors[7]);
		g->SetMarkerSize(0.5);
		g->SetMarkerStyle(kFullCircle);
		g->Draw("p");
		lg->AddEntry(g, f7, "p");
		delete f;
	}
	if (f1) lg->Draw();		// no legend for a single plot
}

void evol_draw(int what)
{
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	gStyle->SetPadLeftMargin(0.05);
	gStyle->SetPadRightMargin(0.01);
	gStyle->SetTitleYOffset(0.6);

	TH1D *hFrame = new TH1D("hFrame", ";Year-2016;LY, ph.c.", 100, 0.75, 7.2);
	hFrame->SetMinimum(32.5);
	hFrame->SetMaximum(37);
	
	switch (what) {
	case 0:
		drawGraphs(hFrame, "root/phe_none_0.0.root", "root/phe_none_0.3.root", "root/phe_none_0.4.root", 
			"root/phe_none_0.5.root", "root/phe_none_0.6.root", "root/phe_none_0.7.root", "root/phe_none_0.8.root");
		break;
	case 1:
		drawGraphs(hFrame, "root/phe_none_0.5.root", "root/phe_none_0.0.root");
		break;
	case 2:
		drawGraphs(hFrame, "root/phe_none_0.6.root", "root/phe_none_0.0.root");
		break;
//
	case 65:
		drawGraphs(hFrame, "phe_2200_142799a-0.65.root");
		break;
	case 70:
		drawGraphs(hFrame, "phe_2200_142799a-0.70.root");
		break;
	case 75:
		drawGraphs(hFrame, "phe_2200_142799a-0.75.root");
		break;
	case 80:
		drawGraphs(hFrame, "phe_2200_142799a-0.80.root");
		break;
	case 85:
		drawGraphs(hFrame, "phe_2200_142799a-0.85.root");
		break;
	case 90:
		drawGraphs(hFrame, "phe_2200_142799a-0.90.root");
		break;
	case 95:
		drawGraphs(hFrame, "phe_2200_142799a-0.95.root");
		break;
	case 100:
		drawGraphs(hFrame, "phe_2200_142799a-1.00.root");
		break;
	case 105:
		drawGraphs(hFrame, "phe_2200_142799a-1.05.root");
		break;
	case 110:
		drawGraphs(hFrame, "phe_2200_142799a-1.10.root");
		break;
	case 200:
		drawGraphs(hFrame, "phe_2200_142799a0.root");
		break;
	case 265:
		drawGraphs(hFrame, "phe_2200_142799a-0.65.root", "phe_2200_142799a0.root");
		break;
	case 270:
		drawGraphs(hFrame, "phe_2200_142799a-0.70.root", "phe_2200_142799a0.root");
		break;
	case 275:
		drawGraphs(hFrame, "phe_2200_142799a-0.75.root", "phe_2200_142799a0.root");
		break;
	case 280:
		drawGraphs(hFrame, "phe_2200_142799a-0.80.root", "phe_2200_142799a0.root");
		break;
	case 285:
		drawGraphs(hFrame, "phe_2200_142799a-0.85.root", "phe_2200_142799a0.root");
		break;
	case 290:
		drawGraphs(hFrame, "phe_2200_142799a-0.90.root", "phe_2200_142799a0.root");
		break;
	case 295:
		drawGraphs(hFrame, "phe_2200_142799a-0.95.root", "phe_2200_142799a0.root");
		break;
	case 300:
		drawGraphs(hFrame, "phe_2200_142799a-1.00.root", "phe_2200_142799a0.root");
		break;
	case 305:
		drawGraphs(hFrame, "phe_2200_142799a-1.05.root", "phe_2200_142799a0.root");
		break;
	case 310:
		drawGraphs(hFrame, "phe_2200_142799a-1.10.root", "phe_2200_142799a0.root");
		break;
	default:
		printf("Undefined value %d\n", what);
	}
}

void evol_draw(const char *what)
{
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	gStyle->SetPadLeftMargin(0.05);
	gStyle->SetPadRightMargin(0.01);
	gStyle->SetTitleYOffset(0.6);

	TH1D *hFrame = new TH1D("hFrame", ";Year-2016;LY, ph.c.", 100, 0.75, 7.2);
	hFrame->SetMinimum(25);
	hFrame->SetMaximum(38);
	drawGraphs(hFrame, what);
}

void draw_chi2(double from, double to)
{
	double alpha, chi2, alpha0, sigma;
	TFile *f;
	TGraph *g;
	char str[1024];
	int i, NDF;
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	TF1 *fPol = new TF1("fPol", "[0]*(1-[1]*x/100.0)", 0, 10);
	TF1 *fPbl = new TF1("fPbl", "pol2", 0, 2);
	fPbl->SetLineColor(kBlack);
	fPbl->SetLineWidth(3);
	
	TH1D *hChi2 = new TH1D("hChi2", ";#alpha;#chi^{2}", 10, 0.625, 1.125);
	for (i=0; i < 10; i++) {
		alpha = 0.65 + 0.05*i;
		sprintf(str, "phe_2200_142799a-%4.2f.root", alpha);
		f = new TFile(str);
		if (!f->IsOpen()) return;
		g = (TGraph *) f->Get("gMedian");
		if (!g) return;
		g->Fit("fPol", "", "", from, to);
		chi2 = fPol->GetChisquare();
		NDF = fPol->GetNDF();
		printf("alpha = %4.2f    chi2 = %f\n", alpha, chi2);
		delete f;
		hChi2->SetBinContent(i+1, chi2);
	}
	hChi2->SetMarkerStyle(kFullCircle);
	hChi2->SetMarkerColor(kBlue);
	hChi2->SetMarkerSize(5.0);
	hChi2->GetXaxis()->SetLabelSize(0.05);
	hChi2->GetYaxis()->SetLabelSize(0.05);
	hChi2->GetXaxis()->SetTitleSize(0.05);
	hChi2->GetYaxis()->SetTitleSize(0.05);
	hChi2->GetYaxis()->SetTitleOffset(0.9);
	hChi2->Fit("fPbl", "", "P");
	alpha0 = - fPbl->GetParameter(1) / (2 * fPbl->GetParameter(2));
	sigma = sqrt(fPbl->Eval(alpha0) / (2 * fPbl->GetParameter(2) * NDF));
	TLine ln;
	ln.SetLineColor(kRed);
	ln.SetLineWidth(3);
	ln.DrawLine(alpha0, hChi2->GetMinimum(), alpha0, hChi2->GetMaximum());
	TLatex txt;
	sprintf(str, "#alpha = %5.3f #pm %5.3f", alpha0, sigma);
	txt.DrawLatexNDC(0.55, 0.8, str);
	delete fPol;
}

void draw_LY(double from, double to)
{
	TFile *f;
	TGraph *g;
	char str[1024];
	int i, j, k;
	
	gStyle->SetOptStat(1100);
	gStyle->SetOptFit(0);
	gPad->SetRightMargin(0.02);
	gPad->SetLeftMargin(0.05);
	gPad->SetTopMargin(0.07);
	gPad->SetBottomMargin(0.12);
	TF1 *fPol = new TF1("fPol", "[0]*(1-[1]*x/100.0)", 0, 10);
	
	TH1D *hLY = new TH1D("hLY", ";#frac{-dLY}{dt}, %/year", 15, 0.4, 0.7);
	hLY->GetXaxis()->SetTitleOffset(1.5);
	hLY->SetLineWidth(2);
	for (i=0; i<2; i++) for (j=0; j<5; j++) for (k=0; k<5; k++) {
		sprintf(str, "newroot/phe_side_%d_%dxy_%d_%dz_%d_%d_0.85.root", i, i, 5*j, 5*j+4, 10*k, 10*k+9);
		f = new TFile(str);
		if (!f->IsOpen()) return;
		g = (TGraph *) f->Get("gMedian");
		if (!g) return;
		g->Fit("fPol", "", "", from, to);
		hLY->Fill(fPol->GetParameter(1));
		delete f;
	}
	hLY->Draw();
}

void draw_dist(double from, double to)
{
	TFile *f;
	TGraph *g;
	char str[1024];
	int i;
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	TCanvas *cv = new TCanvas("CV", "CV", 1000, 1000);
	gPad->SetRightMargin(0.03);
	gPad->SetLeftMargin(0.15);
	TF1 *fPol = new TF1("fPol", "[0]*(1-[1]*x/100.0)", 0, 10);
	
	TH1D *hL = new TH1D("hL", ";l, cm;#frac{-dLY}{LY dt}, %/year", 100, 0, 100);
	hL->SetMinimum(0.52);
	hL->SetMaximum(0.62);
	hL->GetYaxis()->SetTitleOffset(1.7);
	TGraphErrors *gL = new TGraphErrors();
	for (i=0; i<5; i++) {
		sprintf(str, "newroot/phe_dist_%d_%d_0.85.root", 20*i, 20*(i+1));
		f = new TFile(str);
		if (!f->IsOpen()) return;
		g = (TGraph *) f->Get("gMedian");
		if (!g) return;
		g->Fit("fPol", "", "", from, to);
		gL->AddPoint(20*i+10, fPol->GetParameter(1));
		gL->SetPointError(i, 0, 7*fPol->GetParError(1));
		delete f;
	}
	TGraph *gLX = new TGraph();
	for (i=0; i<5; i++) {
		sprintf(str, "newroot/phe_side_0_0dist_%d_%d_0.85.root", 20*i, 20*(i+1));
		f = new TFile(str);
		if (!f->IsOpen()) return;
		g = (TGraph *) f->Get("gMedian");
		if (!g) return;
		g->Fit("fPol", "", "", from, to);
		gLX->AddPoint(20*i+10, fPol->GetParameter(1));
		delete f;
	}
	TGraph *gLY = new TGraph();
	for (i=0; i<5; i++) {
		sprintf(str, "newroot/phe_side_1_1dist_%d_%d_0.85.root", 20*i, 20*(i+1));
		f = new TFile(str);
		if (!f->IsOpen()) return;
		g = (TGraph *) f->Get("gMedian");
		if (!g) return;
		g->Fit("fPol", "", "", from, to);
		gLY->AddPoint(20*i+10, fPol->GetParameter(1));
		delete f;
	}
	hL->Draw();
	gL->SetMarkerColor(kRed);
	gL->SetMarkerStyle(kFullCircle);
	gL->SetMarkerSize(2.5);
	gLX->SetMarkerColor(kGreen);
	gLX->SetMarkerStyle(kFullTriangleUp);
	gLX->SetMarkerSize(2.5);
	gLY->SetMarkerColor(kBlue);
	gLY->SetMarkerStyle(kFullTriangleDown);
	gLY->SetMarkerSize(2.5);
	gL->Draw("pe");
	TF1 *fp1 = new TF1("fp1", "pol1", 0, 100);
	gL->Fit(fp1, "", "", 20, 100);
	sprintf(str, "slope = (%7.5f #pm %7.5f) %%/year/cm", fp1->GetParameter(1), fp1->GetParError(1));
	TLatex lt;
	lt.SetTextSize(0.04);
	lt.DrawLatex(10, 0.6, str);
//	gLX->Draw("p");
//	gLY->Draw("p");
}
