#define MAXZ 100
#define DEPTHBINS	11	// 0.0-1.1 mm
#define SCALEBINS	30	// 1.0-4.0
#define MAXLAYERS	25

#pragma pack(push,1)
struct StoppedMuonStruct {
        int index;              // Index in the DanssEvent tree
        long long globalTime;   // global time 125 MHz ticks
        int Z;                  // z of the stopping strip
        int XY;                 // xy of the stopping strip
        float thetaX;           // angle of the track in ZX, 0 - vertical
        float thetaY;           // angle of the track in ZY, 0 - vertical
        int NHits;              // number of hits including empty hits
        float Ehit[MAXZ];       // hit energy
};
#pragma pack(pop)

//	Draw a single track number num from a tree tIn
void draw_one(int num, TTree *tIn)
{
	struct StoppedMuonStruct Muon;
	double L[MAXZ], dedx[MAXZ];
	double scale;
	int i, k;
	char str[256];
	
	tIn->SetBranchAddress("Stopped", &Muon);
	if (!tIn->GetEntry(num)) return;
	
	scale = sqrt(1 + tan(Muon.thetaX)*tan(Muon.thetaX) + tan(Muon.thetaY)*tan(Muon.thetaY));
	k = 0;
//	printf("Scale = %f\n", scale);
	for (i=0; i<Muon.NHits; i++) if (Muon.Ehit[i] > 0) {
		if (i) {
			L[k] = i * scale;
			dedx[k] = Muon.Ehit[i] / scale;
		} else {
			L[k] = 0.25 * scale;
			dedx[k] = 2 * Muon.Ehit[i] / scale;
		}
//		printf("%2d  %f MeV => %f cm %f MeV/cm\n",
//			i, Muon.Ehit[i], L[k], dedx[k]);
		k++;
	}
	TGraph *gr = new TGraph(k, L, dedx);
	gr->SetMarkerStyle(kFullSquare);
	gr->SetMarkerColor(kBlue);
	gr->SetMarkerSize(0.9);
	gr->SetMinimum(0);
	sprintf(str, "Evt No %d, scale = %5.3f;L, cm;dE/dx, MeV/cm", num, scale);
	gr->SetTitle(str);
	gr->Draw("AP");
}

struct fit_one_struct {
	TH1D *hMCdE;
	struct StoppedMuonStruct Muon;
} FitOneData;

void fit_one_fun(int &Npar, double *gin, double &f, double *x, int iflag)
{
	double d, sum;
	int i;
	
	if (iflag == 1) return;
	
	sum = 0;
	for (i=0; i<FitOneData.Muon.NHits && i<MAXLAYERS; i++) if (FitOneData.Muon.Ehit[i] > 0) {
		d = FitOneData.Muon.Ehit[i] - x[0] * FitOneData.hMCdE->GetBinContent(i+1);
		sum += d * d;
	}
	f = sum;
}

double FitOneFit(TH1D *hMC, double &escale)
{
	double kE, ekE;
	int n;
	double f;
	
	FitOneData.hMCdE = hMC;
	TMinuit *mn = new TMinuit(2);
	mn->mninit(5, 6, 7);
	mn->Command("set print -1");
	mn->SetFCN(fit_one_fun);
	mn->DefineParameter(0, "Escale", 1.0, 0.05, 0.1, 10.);	// Escale
	mn->Migrad();
	mn->GetParameter(0, kE, ekE);
	n = 1;
	fit_one_fun(n, NULL, f, &kE, 0);
	
	delete mn;
	escale = kE;
	return f;
}

//	Try to fit depth
void fit_one(int num, TTree *tIn, TFile *fMC)
{
	TH1D *hMCdE[DEPTHBINS];
	double L[MAXLAYERS], dedx[MAXLAYERS];
	double scale;
	int i, j, k, m;
	char str[256];
	double chi2[DEPTHBINS];
	double escale[DEPTHBINS];
	double d;
	TCanvas *cv;
	
	tIn->SetBranchAddress("Stopped", &FitOneData.Muon);
	if (!tIn->GetEntry(num)) return;
	
	scale = sqrt(1 + tan(FitOneData.Muon.thetaX)*tan(FitOneData.Muon.thetaX) + 
		tan(FitOneData.Muon.thetaY)*tan(FitOneData.Muon.thetaY));
	m = (scale - 1.0) / 0.1;
	if (m < 0) m = 0;
	if (m >= SCALEBINS) m = SCALEBINS - 1;
	for (i=0; i<DEPTHBINS; i++) {
		sprintf(str, "hdEprof_s%2.2dd%d", m, i);
		hMCdE[i] = (TH1D*) fMC->Get(str);
		if (!hMCdE[i]) {
			printf("No hist %s\n", str);
			return;
		}
	}
	k = 0;
	for (i=0; i<FitOneData.Muon.NHits && k < MAXLAYERS; i++) if (FitOneData.Muon.Ehit[i] > 0) {
		L[k] = i + 0.5;
		dedx[k] = FitOneData.Muon.Ehit[i];
		k++;
	}
	
	for (i=0; i<DEPTHBINS; i++) chi2[i] = FitOneFit(hMCdE[i], escale[i]);
	
	cv = new TCanvas("CV", "CV", 800, 900);
	cv->Divide(1, 2);
	
	TH1D h("_h", "S^{2};Depth, mm", 11, 0, 11.0);
	for (i=0; i<DEPTHBINS; i++) h.SetBinContent(i+1, chi2[i]);
	TF1 fpar("fPar", "pol2");
	cv->cd(1);
	h.SetMarkerStyle(kFullCircle);
	h.SetMarkerSize(2.0);
	h.SetMarkerColor(kMagenta);
	h.Fit(fpar.GetName(), "QW", "");
	h.DrawCopy("P");
	d = - 0.5 * fpar.GetParameter(1) / fpar.GetParameter(2);
	if (d < 0) d = 0;
	if (d > 11) d = 11;
	j = d + 0.5;
	if (j >= DEPTHBINS) j = DEPTHBINS-1;
	
	TGraph *gr = new TGraph(k, L, dedx);
	gr->SetMarkerStyle(kFullSquare);
	gr->SetMarkerColor(kRed);
	gr->SetMarkerSize(1.1);
	gr->SetMinimum(0);
	TH1D *hMC = (TH1D *)hMCdE[j]->Clone("hMC");
	sprintf(str, "Evt No %d, Length = %5.3f mm, depth = %5.3f mm, kE = %5.3f;Layer;dE, MeV", 
		num, 10.0*scale, d, escale[j]);
	hMC->SetTitle(str);
	hMC->Scale(escale[j]);
	hMC->SetMinimum(0);
	hMC->SetMaximum(25);
	cv->cd(2);
	hMC->Draw();
	gr->Draw("P");
	cv->Update();
}

//	sum hEL histogramms
void sum_hEL(void)
{
	int i;
	char str[1024];
	TH2D *h;
	TH2D *hSum;
	TFile *fIn;
	
	hSum = NULL;
	for (i=0; i<167309; i+=20000) {
		sprintf(str, "bragg_%6.6d_%6.6d.root", i, i + 19999);
		fIn = new TFile(str);
		if (!fIn->IsOpen()) continue;
		h = (TH2D*) fIn->Get("hEL");
		if (!h) {
			printf("hEL not found in %s\n", str);
			continue;
		}
		if (hSum) {
			hSum->Add(h);
		} else {
			gROOT->cd();
			hSum = (TH2D*) h->Clone("hSum");
		}
		fIn->Close();
	}
	hSum->SetName("hEL");
	
	auto hELProfX = hSum->ProfileX("hEL_profX");
	
	TFile *fOut = new TFile("bragg_002210_167308.root", "RECREATE");
	hSum->Write();
	hELProfX->Write();
	fOut->Close();
}

//	Make MC delta files
void make_deltas(void)
{
	char str[1024];
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/stopped8n2/MC/Chikuma/Muons";
	const char *fnames[] = {
		"Hit_checker_cutted_Chikuma-bragg.root",
		"Hit_checker_cutted_Chikuma_Birks_el_0_0108-bragg.root",
		"Hit_checker_cutted_Chikuma_Birks_el_0_0308-bragg.root",
		"Hit_checker_cutted_Chikuma_Cher_coeff_0_033-bragg.root",
		"Hit_checker_cutted_Chikuma_Cher_coeff_0_233-bragg.root",
		"Hit_checker_cutted_Chikuma_main_Birks_0_0108-bragg.root",
		"Hit_checker_cutted_Chikuma_main_Birks_0_0308-bragg.root",
		"Hit_checker_cutted_Chikuma_paint_0_15-bragg.root",
		"Hit_checker_cutted_Chikuma_paint_0_45-bragg.root"
	};
	int i;
	TFile *fIn[9];
	TH1D *hIn[9];
	
	for (i=0; i<9; i++) {
		sprintf(str, "%s/%s", mcdir, fnames[i]);
		fIn[i] = new TFile(str);
		if (!fIn[i]->IsOpen()) return;
		hIn[i] = (TH1D *) fIn[i]->Get("hEL_profX");
		if (!hIn[i]) {
			printf("Hist hEL_profX not found in %s\n", str);
			return;
		}
	}
	TFile *fOut = new TFile("MC-bragg.root", "RECREATE");
	
	TH1D *hCenter = (TH1D *) hIn[0]->Clone("hCenter");
	TH1D *hDBirks = (TH1D *) hIn[0]->Clone("hDBirks");
	TH1D *hDCher = (TH1D *) hIn[0]->Clone("hDCher");
	TH1D *hDMBirks = (TH1D *) hIn[0]->Clone("hDMBirks");
	TH1D *hDPaint = (TH1D *) hIn[0]->Clone("hDPaint");
	
	hDBirks->Add(hIn[1], hIn[2], 0.5, -0.5);
	hDCher->Add(hIn[3], hIn[4], 0.5, -0.5);
	hDMBirks->Add(hIn[5], hIn[6], 0.5, -0.5);
	hDPaint->Add(hIn[7], hIn[8], 0.5, -0.5);
	
	hDBirks->Write();
	hDCher->Write();
	hDMBirks->Write();
	hDPaint->Write();
	hCenter->Write();
	fOut->Close();
	for (i=0; i<9; i++) fIn[i]->Close();
}

//	Fit energy scale for each MC
//	Use integral ratio, not relly a fit.
void fit_Escale(void)
{
	char str[1024];
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/stopped8n2/MC/Chikuma/Muons";
	const char *fnames[] = {
		"Hit_checker_cutted_Chikuma-bragg.root",
		"Hit_checker_cutted_Chikuma_Birks_el_0_0108-bragg.root",
		"Hit_checker_cutted_Chikuma_Birks_el_0_0308-bragg.root",
		"Hit_checker_cutted_Chikuma_Cher_coeff_0_033-bragg.root",
		"Hit_checker_cutted_Chikuma_Cher_coeff_0_233-bragg.root",
		"Hit_checker_cutted_Chikuma_main_Birks_0_0108-bragg.root",
		"Hit_checker_cutted_Chikuma_main_Birks_0_0308-bragg.root",
		"Hit_checker_cutted_Chikuma_paint_0_15-bragg.root",
		"Hit_checker_cutted_Chikuma_paint_0_45-bragg.root"
	};
	int i;
	TFile *fIn[9];
	TH1D *hIn[9];
	TH1D *hDiff[9];
	double scale;
	
	TFile *fExp = new TFile("bragg_002210_167308.root");
	if (!fExp->IsOpen()) return;
	TH1D *hExp = (TH1D *) fExp->Get("hEL_profX");
	if (!hExp) {
		printf("Hist hEL_profX not found in bragg_002210_167308.root\n");
		return;
	}
	hExp->SetLineColor(kRed);
	hExp->SetMarkerColor(kRed);
	hExp->SetLineWidth(2);
	hExp->SetMarkerSize(0.7);
	hExp->SetMarkerStyle(kFullCircle);
	hExp->GetYaxis()->SetTitle("MeV/cm");
	
	for (i=0; i<9; i++) {
		sprintf(str, "%s/%s", mcdir, fnames[i]);
		fIn[i] = new TFile(str);
		if (!fIn[i]->IsOpen()) return;
		hIn[i] = (TH1D *) fIn[i]->Get("hEL_profX");
		if (!hIn[i]) {
			printf("Hist hEL_profX not found in %s\n", str);
			return;
		}
		hIn[i]->SetLineColor(kBlue);
		hIn[i]->SetMarkerColor(kBlue);
		hIn[i]->SetLineWidth(2);
		hIn[i]->SetMarkerSize(0.7);
		hIn[i]->SetMarkerStyle(kOpenDiamond);
		gROOT->cd();
		sprintf(str, "hDiff_%d", i);
		hDiff[i] = new TH1D(str, "dE/dx difference;cm;MeV/cm", 600, 0, 60);
		hDiff[i]->SetLineColor(kBlack);
		hDiff[i]->SetMarkerColor(kBlack);
		hDiff[i]->SetLineWidth(2);
		hDiff[i]->SetMarkerSize(0.7);
		hDiff[i]->SetMarkerStyle(kFullSquare);
	}
	
	gStyle->SetOptStat(0);
	TCanvas *cv = new TCanvas("CV", "CV", 1000, 800);
	cv->SaveAs("bragg_002210_167308-fit.pdf[");
	
	TLatex lt;
	lt.SetTextSize(0.027);
	TLegend lg(0.7, 0.7, 0.95, 0.82);
	lg.AddEntry(hExp, "Experiment", "lpe");
	lg.AddEntry(hIn[0], "MC", "lpe");
	lg.AddEntry(hDiff[0], "Diff. x5", "lpe");
	hExp->SetMinimum(-2);
	TLine ln;
	for (i=0; i<9; i++) {
		scale = hExp->Integral() / hIn[i]->Integral();
		hIn[i]->Scale(scale);
		hExp->Draw();
		hIn[i]->Draw("same");
		hDiff[i]->Add(hExp, hIn[i], 1.0, -1.0);
//		printf("%f %f %f\n", hExp->GetBinContent(30), hIn[i]->GetBinContent(30), hDiff[i]->GetBinContent(30)); 
		hDiff[i]->Scale(5.0);
		hDiff[i]->Draw("same");
		lg.Draw();
		lt.DrawLatexNDC(0.15, 0.83, fnames[i]);
		sprintf(str, "scale = %f", scale);
		lt.DrawLatexNDC(0.15, 0.75, str);
		ln.DrawLine(0, 0, 60, 0);
		cv->Update();
		cv->SaveAs("bragg_002210_167308-fit.pdf");
	}
	
	cv->SaveAs("bragg_002210_167308-fit.pdf]");
	for (i=0; i<9; i++) fIn[i]->Close();
	fExp->Close();
}

struct HistArrayStruct {
	TH1D *hExp;
	TH1D *hCenter;
	TH1D *hDBirks;
	TH1D *hDCher;
	TH1D *hDMBirks; 
	TH1D *hDPaint;
	TH1D *hSum;
	int MinBin;
	int MaxBin;
} HistArray;

void BraggFitFun(int &Npar, double *gin, double &f, double *x, int iflag)
{
	int i;
	double d, sum;
	
	if (iflag == 1) return;
	HistArray.hSum->Reset();
	
	HistArray.hSum->Add(HistArray.hCenter, 1.0);
	HistArray.hSum->Add(HistArray.hDBirks, x[1]);
	HistArray.hSum->Add(HistArray.hDCher, x[2]);
	HistArray.hSum->Add(HistArray.hDMBirks, x[3]);
	HistArray.hSum->Add(HistArray.hDPaint, x[4]);
	HistArray.hSum->Scale(x[0]);
	HistArray.hSum->Add(HistArray.hExp, -1.0);
	
	sum = 0;
	for (i=HistArray.MinBin; i<=HistArray.MaxBin; i++) if (HistArray.hSum->GetBinError(i) > 0) {
		d = HistArray.hSum->GetBinContent(i) / HistArray.hSum->GetBinError(i);
		sum += d*d;
	}
	f = sum;
}

void makeMCBragg(TH1D *h, double scale, double kB, double kC, double kH, double kP)
{
	h->Reset();
	h->Add(HistArray.hCenter, 1.0);
	h->Add(HistArray.hDBirks, kB);
	h->Add(HistArray.hDCher, kC);
	h->Add(HistArray.hDMBirks, kH);
	h->Add(HistArray.hDPaint, kP);
	h->Scale(scale);
}

void draw_BraggShape(int from, int to, double kE, double kB, double kC, double kH, double kP)
{
	double x[5];
	double f;
	int n;
	char str[256];
	
	TH1D *hRes = new TH1D("hBraggMC", "MC dE/dx;cm;MeV/cm", 600, 0, 60);
	makeMCBragg(hRes, kE, kB, kC, kH, kP);
	HistArray.hExp->SetLineColor(kRed);
	HistArray.hExp->SetMarkerColor(kRed);
	HistArray.hExp->SetLineWidth(2);
	HistArray.hExp->SetMarkerSize(0.7);
	HistArray.hExp->SetMarkerStyle(kFullCircle);
	HistArray.hExp->GetYaxis()->SetTitle("MeV/cm");
	HistArray.hExp->GetXaxis()->SetRange(from, to);

	hRes->SetLineColor(kBlue);
	hRes->SetMarkerColor(kBlue);
	hRes->SetLineWidth(2);
	hRes->SetMarkerSize(0.7);
	hRes->SetMarkerStyle(kOpenDiamond);

	gStyle->SetOptStat(0);

	x[0] = kE;
	x[1] = kB;
	x[2] = kC;
	x[3] = kH;
	x[4] = kP;
	n = 5;
	BraggFitFun(n, NULL, f, x, 0);
	sprintf(str, "#chi^{2} / NDF = %6.1f / %d", f, to - from - 1);

	HistArray.hExp->DrawCopy();
	hRes->DrawCopy("same");
	TLegend *lg = new TLegend(0.65, 0.75, 0.9, 0.9);
	lg->AddEntry(HistArray.hExp, "Experiment", "lpe");
	lg->AddEntry(hRes, "MC", "lpe");
	lg->Draw();
	TLatex lt;
	lt.SetTextSize(0.045);
	lt.DrawLatexNDC(0.45, 0.67, str);
	gPad->Update();
}

void fit_BraggShape(int from, int to)
{
	double kE, ekE, kB, ekB, kC, ekC, kH, ekH, kP, ekP;
	
	gSystem->LoadAllLibraries();

	TFile *fExp = new TFile("bragg_002210_167308.root");
	if (!fExp->IsOpen()) return;
	HistArray.hExp = (TH1D *) fExp->Get("hEL_profX");
	TFile *fIn = new TFile("MC-bragg.root");
	if (!fIn->IsOpen()) return;
	HistArray.hCenter = (TH1D *) fIn->Get("hCenter");
	HistArray.hDBirks = (TH1D *) fIn->Get("hDBirks");
	HistArray.hDCher = (TH1D *) fIn->Get("hDCher");
	HistArray.hDMBirks = (TH1D *) fIn->Get("hDMBirks");
	HistArray.hDPaint = (TH1D *) fIn->Get("hDPaint");
	if (!HistArray.hCenter || !HistArray.hDBirks || !HistArray.hDCher || 
		!HistArray.hDMBirks || !HistArray.hDPaint || !HistArray.hExp) {
		printf("Can not find some histogramms\n");
		return;
	}
	HistArray.hSum = new TH1D("_hSum", "dE/dx difference;cm;MeV/cm", 600, 0, 60);
	HistArray.MinBin = from;
	HistArray.MaxBin = to;
	
	TMinuit *mn = new TMinuit();
	mn->mninit(5, 6, 7);
	mn->SetFCN(BraggFitFun);
	mn->DefineParameter(0, "Escale", 1.0, 0.05, 0.1, 10.);	// Escale
	mn->DefineParameter(1, "kBirks", 0, 0.05, -5., 2.);	// kBirks (electrons)
	mn->FixParameter(1);
	mn->DefineParameter(2, "kCher", 0, 0.05, -5., 2.);	// kCherenkov
	mn->FixParameter(2);
	mn->DefineParameter(3, "kMBirks", 0, 0.05, -5., 2.);	// kBirks (heavy)
	mn->FixParameter(3);
	mn->DefineParameter(4, "kPaint", 0, 0.05, -5., 2.);	// kPaint
	
	mn->Migrad();
	
	mn->GetParameter(0, kE, ekE);
	mn->GetParameter(1, kB, ekB);
	mn->GetParameter(2, kC, ekC);
	mn->GetParameter(3, kH, ekH);
	mn->GetParameter(4, kP, ekP);
	
	printf("Fit: kE=%f+-%f  kB=%f+-%f  kC=%f+-%f  kH=%f+-%f  kP=%f+-%f\n",
		kE, ekE, kB, ekB, kC, ekC, kH, ekH, kP, ekP);
	
	draw_BraggShape(from, to, kE, kB, kC, kH, kP);
	delete mn;
//	fExp->Close();
//	fIn->Close();
}

//	make MC chain
TChain *MCchain(const char *mcname = "")
{
	char str[1024];
	int i, j, N, K, M;
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/stopped8n2/MC/Chikuma/Muons/Hit_checker_cutted_Chikuma";

	TChain *chIn = new TChain("StoppedMuons", "StoppedMuons");
	TChain *chDepth = new TChain("Depth", "Depth");
	TChain *chFDepth = new TChain("FDepth", "FDepth");
	for (i=0; i<5; i++) for (j=1; j<=16; j++) {
		sprintf(str, "%s%s/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d.root", mcdir, mcname, i, j);
		chIn->AddFile(str);
		sprintf(str, "%s%s/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d-depth.root", mcdir, mcname, i, j);
		chDepth->AddFile(str);
		sprintf(str, "%s%s/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d-fdepth.root?#Depth", mcdir, mcname, i, j);
		chFDepth->AddFile(str);
	}
	N = chIn->GetEntries();
	K = chDepth->GetEntries();
	M = chFDepth->GetEntries();
	if (N != K || N != M) {
		printf("Chain lengths %d %d %d mismatch\n", N, K, M);
		return NULL;
	}
	chIn->AddFriend(chDepth);
	chIn->AddFriend(chFDepth);
	printf("Total %d events\n", N);
	return chIn;
}

void draw_MCdE(int iscale, const char *mcname = "")
{
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/stopped8n2/MC/Chikuma/Muons/Hit_checker_cutted_Chikuma";
	const int color[] = {kRed, kGreen, kBlue, kMagenta, kBlack};
	const char *legend[] = {"0 - 2 mm", "2 - 4 mm", "4 - 6 mm", "6 - 8 mm", "8 - 11 mm"};
	TH2D *hEL[DEPTHBINS];
	TH2D *hSum;
	TProfile *hProf[5];
	int i;
	char str[1024];
	
	gStyle->SetOptStat(0);
	sprintf(str, "%s%s-hist.root", mcdir, mcname);
	TFile *fIn = new TFile(str);
	if (!fIn->IsOpen()) return;
	
	for (i=0; i<DEPTHBINS; i++) {
		sprintf(str, "hdE_s%2.2dd%d", iscale, i);
		hEL[i]=(TH2D *) fIn->Get(str);
		if (!hEL[i]) {
			printf("%s not found in %s\n", str, mcname);
			return;
		}
	}
	hSum = (TH2D *) hEL[0]->Clone("__hSum");
	for (i=0; i<5; i++) {
		hSum->Add(hEL[2*i], hEL[2*i+1]);
		if (i == 4) hSum->Add(hEL[10]);
		sprintf(str, "hdEprof_%d", i);
		hProf[i] = hSum->ProfileX(str);
		hProf[i]->SetLineWidth(2);
		hProf[i]->SetLineColor(color[i]);
		hProf[i]->SetMinimum(0);
		sprintf(str, "dE in the layer for length in strip %4.1f mm;Layer;dE, MeV", 10.0 + iscale);
		hProf[i]->SetTitle(str);
	}
	TLegend *lg = new TLegend(0.6, 0.7, 0.9, 0.9);
	for (i=4; i>=0; i--) {
		hProf[i]->Draw((i != 4) ? "same" : "");
		lg->AddEntry(hProf[i], legend[i], "l");
	}
	lg->Draw();
}
