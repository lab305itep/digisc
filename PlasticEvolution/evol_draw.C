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

void draw_chi2(double from, double to)
{
	double alpha, chi2, alpha0;
	TFile *f;
	TGraph *g;
	char str[1024];
	int i;
	
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
	hChi2->GetYaxis()->SetTitleOffset(0.5);
	hChi2->Fit("fPbl", "", "P");
	alpha0 = - fPbl->GetParameter(1) / (2*fPbl->GetParameter(2));
	TLine ln;
	ln.SetLineColor(kRed);
	ln.SetLineWidth(3);
	ln.DrawLine(alpha0, hChi2->GetMinimum(), alpha0, hChi2->GetMaximum());
	TLatex txt;
	sprintf(str, "#alpha = %4.2f", alpha0);
	txt.DrawLatexNDC(0.6, 0.8, str);
	delete fPol;
}
