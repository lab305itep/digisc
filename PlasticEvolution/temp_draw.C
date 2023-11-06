void temp_draw(const char *fname, int chA, int chB = 0, int chC = 0, int chD = 0)
{
	char str[1024];
	
	gStyle->SetOptStat(0);
	gStyle->SetPadTopMargin(0.02);
	gStyle->SetPadRightMargin(0.01);
	gStyle->SetPadLeftMargin(0.04);
	
	TCanvas *cv = new TCanvas("CV", "CV", 2000, 700);
	TH1D *hFrame = new TH1D("hFrame", ";Year-2016;t, ^{o}C", 100, 0.75, 7.2);
	hFrame->SetMinimum(14);
	hFrame->SetMaximum(26);
	hFrame->GetYaxis()->SetTitleOffset(0.5);
	hFrame->Draw();
	
	TFile *fIn = new TFile(fname);
	if (!fIn->IsOpen()) return;
	
	sprintf(str, "gTemp%3.3d", chA);
	TGraph *grA = (TGraph *) fIn->Get(str);
	if (!grA) return;
	grA->SetMarkerColor(kRed);
	grA->SetMarkerStyle(kFullCircle);
	grA->SetMarkerSize(0.3);
	grA->Draw("p");
	if (chB) {
		sprintf(str, "gTemp%3.3d", chB);
		TGraph *grB = (TGraph *) fIn->Get(str);
		if (!grB) return;
		grB->SetMarkerColor(kBlue);
		grB->SetMarkerStyle(kFullCircle);
		grB->SetMarkerSize(0.3);
		grB->Draw("p");
	}
	if (chC) {
		sprintf(str, "gTemp%3.3d", chC);
		TGraph *grC = (TGraph *) fIn->Get(str);
		if (!grC) return;
		grC->SetMarkerColor(kGreen);
		grC->SetMarkerStyle(kFullCircle);
		grC->SetMarkerSize(0.3);
		grC->Draw("p");
	}
	if (chD) {
		sprintf(str, "gTemp%3.3d", chD);
		TGraph *grD = (TGraph *) fIn->Get(str);
		if (!grD) return;
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
	
	TCanvas *cv = new TCanvas("CV", "CV", 2000, 700);
	TH1D *hFrame = new TH1D("hFrame", ";Year-2016;1px, ADC integral", 100, 0.75, 7.2);
	hFrame->SetMinimum(54);
	hFrame->SetMaximum(72);
	hFrame->GetYaxis()->SetTitleOffset(0.5);
	hFrame->Draw();
	
	
	if (chA) draw_graph(chA, kRed, 0, fIn);
	if (chB) draw_graph(chB, kBlue, 5, fIn);
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
