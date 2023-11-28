void Add2X(TGraph *g, double deltaX)
{
	int i, N;
	double x, y;
	N = g->GetN();
	for (i=0; i<N; i++) {
		g->GetPoint(i, x, y);
		g->SetPoint(i, x + deltaX, y);
	}
}

void temp_draw(const char *fname, int chA, int chB = 0, int chC = 0, int chD = 0)
{
	char str[1024];
	
	gStyle->SetOptStat(0);
	gStyle->SetPadTopMargin(0.02);
	gStyle->SetPadRightMargin(0.01);
	gStyle->SetPadLeftMargin(0.04);
	
	TCanvas *cv = new TCanvas("CV", "CV", 1600, 600);
	TH1D *hFrame = new TH1D("hFrame", ";Year;t, ^{o}C", 100, 2016.75, 2023.2);
	hFrame->GetXaxis()->SetNdivisions(7, 4, 3);
	hFrame->SetMinimum(14);
	hFrame->SetMaximum(26);
	hFrame->GetYaxis()->SetTitleOffset(0.5);
	hFrame->Draw();
	
	TFile *fIn = new TFile(fname);
	if (!fIn->IsOpen()) return;
	
	sprintf(str, "gTemp%3.3d", chA);
	TGraph *grA = (TGraph *) fIn->Get(str);
	if (!grA) return;
	Add2X(grA, 2016);
	grA->SetMarkerColor(kRed);
	grA->SetMarkerStyle(kFullCircle);
	grA->SetMarkerSize(0.4);
	grA->Draw("p");
	if (chB) {
		sprintf(str, "gTemp%3.3d", chB);
		TGraph *grB = (TGraph *) fIn->Get(str);
		if (!grB) return;
		Add2X(grB, 2016);
		grB->SetMarkerColor(kBlue);
		grB->SetMarkerStyle(kFullCircle);
		grB->SetMarkerSize(0.3);
		grB->Draw("p");
	}
	if (chC) {
		sprintf(str, "gTemp%3.3d", chC);
		TGraph *grC = (TGraph *) fIn->Get(str);
		if (!grC) return;
		Add2X(grC, 2016);
		grC->SetMarkerColor(kGreen);
		grC->SetMarkerStyle(kFullCircle);
		grC->SetMarkerSize(0.3);
		grC->Draw("p");
	}
	if (chD) {
		sprintf(str, "gTemp%3.3d", chD);
		TGraph *grD = (TGraph *) fIn->Get(str);
		if (!grD) return;
		Add2X(grD, 2016);
		grD->SetMarkerColor(kOrange);
		grD->SetMarkerStyle(kFullCircle);
		grD->SetMarkerSize(0.3);
		grD->Draw("p");
	}
	TString fpng("temp_");
	sprintf(str, "%d", chA);
	fpng += TString(str);
	if (chB) {
		sprintf(str, "_%d", chB);
		fpng += TString(str);
	}
	if (chC) {
		sprintf(str, "_%d", chC);
		fpng += TString(str);
	}
	if (chD) {
		sprintf(str, "_%d", chD);
		fpng += TString(str);
	}
	fpng += ".png";
	cv->SaveAs(fpng.Data());
	
	delete fIn;
}

void draw_graph(int num, int color, double shift, TFile *fIn[5])
{
	int i;
	TGraph *gr;
	char str[1024];
	
	sprintf(str, "y+%f", shift);
	TF2 *fAdd = new TF2("fAdd", str, -1e10, 1e10);
	
	for (i = 0; i<5; i++) {
		sprintf(str, "g1px%2.2d_%2.2d", num/100, num%100);
		gr = (TGraph *) fIn[i]->Get(str);
		if (!gr) return;
		gr->Apply(fAdd);
		Add2X(gr, 2016);
		gr->SetMarkerColor(color);
		gr->SetMarkerStyle(kFullCircle);
		gr->SetMarkerSize(0.3);
		gr->Draw("p");
	}
	delete fAdd;
}

void px1_draw(int chA, int chB = 0, int chC = 0, int chD = 0)
{
	const char *files[] = {"gr1px_2210_29999.root", "gr1px_30000_59999.root", 
		"gr1px_60000_89999.root", "gr1px_90000_119999.root", "gr1px_120000_142815.root"};
	char str[1024];
	TFile *fIn[5];
	int i;
	
	gStyle->SetOptStat(0);
	gStyle->SetPadTopMargin(0.02);
	gStyle->SetPadRightMargin(0.01);
	gStyle->SetPadLeftMargin(0.04);
	
	for (i=0; i<5; i++) {
		fIn[i] = new TFile(files[i]);
		if (!fIn[i]->IsOpen()) return;
	}
	
	TCanvas *cv = new TCanvas("CV", "CV", 1600, 600);
	TH1D *hFrame = new TH1D("hFrame", ";Year;1px, ADC integral", 100, 2016.75, 2023.2);
	hFrame->GetXaxis()->SetNdivisions(7, 4, 3);
	hFrame->SetMinimum(54);
	hFrame->SetMaximum(72);
	hFrame->GetYaxis()->SetTitleOffset(0.5);
	hFrame->Draw();
	
	
	if (chA) draw_graph(chA, kBlue, 0, fIn);
	if (chB) draw_graph(chB, kRed, 5, fIn);
	if (chC) draw_graph(chC, kGreen, 10, fIn);
	if (chD) draw_graph(chD, kOrange, 15, fIn);
	TString fpng("g1px_");
	if (chA) {
		sprintf(str, "%d", chA);
		fpng += TString(str);
	}
	if (chB) {
		sprintf(str, "_%d", chB);
		fpng += TString(str);
	}
	if (chC) {
		sprintf(str, "_%d", chC);
		fpng += TString(str);
	}
	if (chD) {
		sprintf(str, "_%d", chD);
		fpng += TString(str);
	}
	fpng += ".png";
	cv->SaveAs(fpng.Data());
}

void drawGraph(TH1D *frame, const char *f0)
{
	TFile *f;
	TGraphErrors *g;
	char str[128];
	
	TF1 *fPol = new TF1("fPol", "[0]*(1-[1]*(x-2016)/100.0)", 2016, 2030);
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
	Add2X(g, 2016);
	g->SetMarkerColor(kRed);
	g->SetMarkerSize(0.7);
	g->SetMarkerStyle(kFullCircle);
	g->Draw("p");
	g->Fit(fPol, "", "", 2016.75, 2023.2);
	TLatex lt;
	sprintf(str, "Slope = -%4.2f %%/year", fPol->GetParameter(1));
	lt.DrawLatex(2021, 36.5, str);
	
	delete f;
}

void LY_draw(const char *what)
{
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	gStyle->SetPadLeftMargin(0.05);
	gStyle->SetPadRightMargin(0.01);
	gStyle->SetTitleYOffset(0.6);

	TCanvas *cv = new TCanvas("CV", "CV", 1600, 600);
	TH1D *hFrame = new TH1D("hFrame", ";Year;LY, ph.c.", 100, 2016.75, 2023.2);
	hFrame->GetXaxis()->SetNdivisions(7, 4, 3);
	hFrame->SetMinimum(33);
	hFrame->SetMaximum(37);
	drawGraph(hFrame, what);
}
