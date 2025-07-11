//	Compare MC to IBD spectrum
const char *BASEDIR = "/home/clusters/rrcmpi/alekseev/igor/pair8n2/MC/IBD_large_fuel";

#define MAXPOSITIONS 1000
struct PositionsStruct {
	char name[32];
	int first;
	int last;
	int period;
	char chpower;
	double power;
	double bgnd;
	double total;
	double dead;
	double eff;
	double f235;
	double f238;
	double f239;
	double f241;
	double f239S;	// renormalized to 100% 235U + 239Pu fraction of 239Pu
	TH1D *hSp;	// experimental sectrum
} positions[MAXPOSITIONS];
int PosEntries;

const char *NeutronCorrN = "0.5109-0.01932*x";		// neutron correction for signal per day
const char *NeutronCorrC = "2.143-0.09811*x";		// neutron correction for cosmic per day
const char *LowMuonCorr = "10.7*exp(-1.00*x)";		// low energy reactor off correction
const double rOffCosmCorr = 0.98 * (1-0.092) / (1-0.137);	// 8 m of additional water above reactor and different dead times
const double OtherBlockFraction = 0.0060;	// Neutrino fraction of other blocks. Distances to other reactors: 160, 336 and 478 m
const double bgCorrUp = 0.958;			// Up background correction from the reator off
const double bgCorrMid = 0.988;			// Mid background correction from the reator off
const double bgCorrDown = 1.058;		// Down background correction from the reator off

void add_IBD2chain(TChain *ch, const char *what, int nser, int ninser)
{
	char str[1024];
	int i, j, irc;
	for (i=0; i<nser; i++) for (j=1; j<=ninser; j++) {
		sprintf(str, "%s/%s/mc_IBD_indLY_transcode_rawProc_pedSim_%s_%2.2d_%2.2d.root", BASEDIR, what, what, i, j);
		irc = access(str, R_OK);
		if (!irc) ch->AddFile(str);
	}
}

TChain *make_MCchain(int isotope)
{
	TChain *ch = new TChain("DanssPair");
	
	switch (isotope) {
	case 235:
		add_IBD2chain(ch, "235U", 12, 8);
		break;
	case 238:
		add_IBD2chain(ch, "238U", 2, 8);
		break;
	case 239:
		add_IBD2chain(ch, "239Pu", 8, 8);
		break;
	case 241:
		add_IBD2chain(ch, "241Pu", 2, 8);
		break;
	default:
		delete ch;
		return NULL;
	}
	return ch;
}

void mkMC(const char *mc_file = "MC_large_fuel.root")
{
	char strs[128], strl[1024];
	const int a_fuel[4] = {235, 238, 239, 241};
	const char *name_fuel[4] = {"235U", "238U", "239Pu", "241Pu"};
	const int nfiles[4]={94, 16, 64, 16};
	const double cross_fuel[4] = {6.69, 10.10, 4.36, 6.04};
	const int EVT_PER_FILE = 500000;
	TH1D *hFuel[4];
	TChain *tFuel[4];
	int i;
	
	TCut cVeto("gtFromVeto > 90");
	TCut cIso("EventsBetween == 0");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut c20("gtDiff > 1");
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
	TCut cPh("PositronHits < 8");
	TCut cR2("Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR3("Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR = cR3 && (cRXY || cR2);
	TCut cNH("NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
	TCut cNE("NeutronEnergy > 1.5 + 2.6 * exp(-0.15 * PositronEnergy*PositronEnergy)");
	TCut cN = cNH && cNE;
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))");

	TCut cSel = cX && cY && cZ && c20 && cPh && cR && cN && cSingle;
	
	for (i=0; i<4; i++) {
		tFuel[i] = make_MCchain(a_fuel[i]);
		if (!tFuel[i]) return;
		sprintf(strs, "hMC%s", name_fuel[i]);
		sprintf(strl,"MC IBD for %s;MeV;Events/bin/fiss.", name_fuel[i]);
		hFuel[i] = new TH1D(strs, strl, 128, 0, 16);
		tFuel[i]->Project(hFuel[i]->GetName(), "PositronEnergy", cSel);
		hFuel[i]->Scale(cross_fuel[i]/(nfiles[i] * EVT_PER_FILE));
	}

	TFile *fOut = new TFile(mc_file, "RECREATE");
	for (i=0; i<4; i++) hFuel[i]->Write();

	fOut->Close();
}

void SetHistProperties(TH1 *h, EColor color, EMarkerStyle marker)
{
	h->SetLineColor(color);
	h->SetMarkerColor(color);
	h->SetLineWidth(2);
	h->SetMarkerSize(1.5);
	h->SetMarkerStyle(marker);	
}

//	Get spectum with all corrections and subtractions
TH1D *GetSpectrum(TFile *fData, struct PositionsStruct *pos, double bgScale = 1.8)
{
	int N;
	TH1D *hSig;
	TH1D *hConst;
	TH1D *hBgnd;
	TH1D *hRBgnd;
	TH1D *hSum;
	char str[1024];
	char strs[128];
	double dt;
	TF1 fBgndN("fBgndN", NeutronCorrN, 0.5, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0.5, 100);
	TF1 fLowMuon("fLowMuon", LowMuonCorr, 0.5, 100);

	sprintf(str, "%s_hSig-diff", pos->name);
	hSig = (TH1D*) fData->Get(str);
	sprintf(str, "%s_hConst", pos->name);
	hConst = (TH1D*) fData->Get(str);
	sprintf(str, "%s_hCosm-diff", pos->name);
	hBgnd = (TH1D*) fData->Get(str);
	sprintf(str, "%s_hCosm-murand", pos->name);
	hRBgnd = (TH1D*) fData->Get(str);
	if (!(hSig && hConst && hBgnd && hRBgnd)) return NULL;
	dt = hConst->GetBinContent(1) / 1000.0;	// seconds * 10^3

	sprintf(strs, "hSp_%s", pos->name);
	sprintf(str, "Spectrum at %s;MeV;Events/bin", pos->name);
	hSum = new TH1D(strs, str, 128, 0, 16);
	hSum->Add(hSig);
	hSum->Add(hBgnd, -pos->bgnd * bgScale);
	hSum->Add(hRBgnd, pos->bgnd * bgScale);	// subtract accidental muon to IBD coincidence
	hSum->Add(&fBgndN, -dt / 86.4);
	hSum->Add(&fLowMuon, - dt / 86.4);
	hSum->Add(&fBgndC, dt * pos->bgnd * bgScale / 86.4);
	return hSum;
}

int GlobalPeriod(int run)
{
	if (run < 5000) return 1;		// April-June 2016
	if (run < 24000) return 2;		// October 2016 - July 2017: till the end of Campaign 4
	if (run < 37000) return 3;		// August 2017 - March 2018: Campaign 5 till detector repair
	if (run < 56000) return 4;		// May 2018 - January 2019: after detector repair till the end of  Campaign 5
	if (run < 87500) return 5;		// Campaign 6
	if (run < 117700) return 6;		// Campaign 7
	if (run < 147000) return 7;		// Campaign 8
	return 8;				// Campaign 9
}

int readPositions(const char *fname)
{
	FILE *f;
	char str[1024];
	char *ptr;
	
	PosEntries = 0;
	memset(positions, 0, sizeof(positions));
	f = fopen(fname, "rt");
	if (!f) return 0;
	for(;;) {
		ptr = fgets(str, sizeof(str), f);
		if (!ptr) break;
		ptr = strtok(str, " \t");
		if (!ptr || ptr[0] == '*') continue;
		if (!strchr("udmrs", ptr[0])) continue;
		strncpy(positions[PosEntries].name, ptr, sizeof(positions[PosEntries].name));
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].first = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].last = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].chpower = ptr[0];
		if (positions[PosEntries].chpower != 'F') continue;	// full power only
		positions[PosEntries].period = (ptr[0] == 'F') ? GlobalPeriod(positions[PosEntries].first) : 0;
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].power = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].bgnd = 1.55 * strtod(ptr, NULL);	// we use this coef to keep to the old definition
		switch (positions[PosEntries].name[0]) {		// do background position correction
		case 'u' : positions[PosEntries].bgnd *= bgCorrUp; break;
		case 'm' : positions[PosEntries].bgnd *= bgCorrMid; break;
		case 'd' : positions[PosEntries].bgnd *= bgCorrDown; break;
		}
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].total = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].dead = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].eff = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].f235 = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].f238 = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].f239 = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].f241 = strtod(ptr, NULL);		
		PosEntries++;
		if (PosEntries == MAXPOSITIONS) {
			printf("Warning ! MAXPOSITIONS = %d reached\n", MAXPOSITIONS);
			break;
		}
	}
	fclose(f);
	return PosEntries;
}

//	Create graph for bin k
TGraphErrors *MakeGraph(int k)
{
	int i;
	char str[256];
	
	TVectorD x(PosEntries);
	TVectorD ex(PosEntries);
	TVectorD y(PosEntries);
	TVectorD ey(PosEntries);
	for (i=0; i<PosEntries; i++) {
		x[i] = positions[i].f239S;
		ex[i] = 0;
		y[i] = positions[i].hSp->GetBinContent(k);
		ey[i] = positions[i].hSp->GetBinError(k);
	}
	TGraphErrors *g = new TGraphErrors(x, y, ex, ey);
	sprintf(str, "Bin %d", k);
	g->SetTitle(str);
	g->SetMarkerStyle(kFullCircle);
	g->SetMarkerSize(1.5);
	g->SetMarkerColor(kBlue);
	g->SetLineColor(kBlue);
	g->SetLineWidth(2);
	return g;
}

// Subtract 238U and 241Pu using HM and MC and then distillate pure 235U and 239Pu spectra
// fPos - file with positions
// fBase - file with spectrum per position
// fMCname - file with MC for the 4 isotopes
// bgScale - scale cosmogenic background to 2.5%
int MakePure59(const char *fPos = "pos_10dF_2210_154797.txt", const char *fBase = "base_10d.root", 
	const char *fMCname = "MC_large_fuel.root", double bgScale = 1.8)
{
	int i;
	int irc;
	TH1D *hMC[4];	// 235, 238, 239, 241
	TH1D *h238;
	TH1D *h241;
	double N0[4];	// Sum of MC spectra
	double a, a5, a9, Ni, NMC;
	double ea, ea5, ea9;
	double sum235, esum235, sum239, esum239;
	double sumMC235, esumMC235, sumMC239, esumMC239;
	const char *name_fuel[4] = {"235U", "238U", "239Pu", "241Pu"};
	char str[1024];
	double s235, es235, s239, es239;
	TGraphErrors *gr;
//		Read positins file	
	irc = readPositions(fPos);
	if (!irc) return -10;
//		Open Data and MC files
	TFile *fData = new TFile(fBase);
	if (!fData->IsOpen()) return -20;
	TFile *fMC = new TFile(fMCname);
	if (!fMC->IsOpen()) return -21;
	TString oname(fBase);
	oname.ReplaceAll(".root", "");
	oname += "-sp";
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	TCanvas *cv = new TCanvas("CV", "CV", 1000, 800);
	TLatex *lt = new TLatex();
	lt->SetTextSize(0.025);
	
	cv->SaveAs((oname + ".pdf[").Data());
//		Read data file
	for (i=0; i<PosEntries; i++) {
		positions[i].hSp = GetSpectrum(fData, &positions[i], bgScale);
		if (!positions[i].hSp) {
			printf("Can not find histogramms for position %s in %s\n", positions[i].name, fBase);
			return -30;
		}
	}
//		Read MC file
	for (i=0; i<4; i++) {
		sprintf(str, "hMC%s", name_fuel[i]);
		hMC[i] = (TH1D*) fMC->Get(str);
		if (!hMC[i]) {
			printf("Can not find fuel histogramm %s in %s\n", str, fBase);
			return -40;
		}
		N0[i] = hMC[i]->Integral();	// Full range
	}
//		Subtruct 238 and 241
	TLegend *lgA = new TLegend(0.7, 0.4, 0.89, 0.6);
	for (i=0; i<PosEntries; i++) {
		Ni = positions[i].hSp->Integral();	// Full range
		NMC = positions[i].f235 * N0[0] + positions[i].f238 * N0[1] + positions[i].f239 * N0[2] + positions[i].f241 * N0[3];
		a = Ni / NMC;				// experimental spectrum normalization
		positions[i].hSp->Scale(1.0/a);		// Scale all experimental points
		SetHistProperties(positions[i].hSp, kRed, kFullCircle);
		positions[i].hSp->SetMinimum(0);
		positions[i].hSp->SetMaximum(12);
		positions[i].hSp->GetYaxis()->SetTitle("a.u.");
		positions[i].hSp->DrawCopy();
		if (!i) lgA->AddEntry(positions[i].hSp->Clone("_Total"), "Total", "lp");
		sprintf(str, "h238_%d", i);
		h238 = (TH1D*) hMC[1]->Clone(str);
		h238->Scale(positions[i].f238);
		SetHistProperties(h238, kBlue, kFullSquare);
		h238->DrawCopy("same");
		sprintf(str, "h241_%d", i);
		h241 = (TH1D*) hMC[3]->Clone(str);
		SetHistProperties(h241, kGreen, kOpenSquare);
		h241->Scale(positions[i].f241);
		h241->DrawCopy("same");
		positions[i].hSp->Add(hMC[1], -positions[i].f238);		// subtract 235U
		positions[i].hSp->Add(hMC[3], -positions[i].f241);		// subtract 241Pu
		SetHistProperties(positions[i].hSp, kBlack, kFullDiamond);
		positions[i].hSp->DrawCopy("same");
		positions[i].f239S = 100.0 * positions[i].f239 / (positions[i].f235 + positions[i].f239);
		positions[i].hSp->Scale(100.0 / (positions[i].f235 + positions[i].f239));
		sprintf(str, "Fractions: ^{235}U: %5.2f  ^{238}U: %5.2f  ^{239}Pu: %5.2f  ^{241}Pu: %5.2f", 
			positions[i].f235, positions[i].f238, positions[i].f239, positions[i].f241);
		lt->DrawLatexNDC(0.4, 0.82, str);
		sprintf(str, "Scaled fraction ^{239}Pu: %5.2f", positions[i].f239S);
		lt->DrawLatexNDC(0.6, 0.75, str);
		if (!i) {
			lgA->AddEntry(positions[i].hSp, "^{235}U+^{239}Pu", "lp");
			lgA->AddEntry(h238, "^{238}U (HM)", "lp");
			lgA->AddEntry(h241, "^{241}Pu (HM)", "lp");
		}
		lgA->Draw();
		cv->Update();
		cv->SaveAs((oname + ".pdf").Data());		
	}
//		Create and fit Graphs 
	TF1 *lFit = new TF1("lFit", "[0]+([1]-[0])*x/100.0", 0, 100);
	lFit->SetParNames("S_{5}", "S_{9}");
	TH1D *h235 = new TH1D("h235", "^{235}U spectrum;MeV;a.u.", 128, 0, 16);
	TH1D *h239 = new TH1D("h239", "^{239}Pu spectrum;MeV;a.u.", 128, 0, 16);
	double _ndf = PosEntries - 2;
	double chimin = _ndf - 5*sqrt(_ndf);
	if (chimin < 0) chimin = 0;
	double chimax = _ndf + 9*sqrt(_ndf);
	TH1D *hChi2 = new TH1D("hChi2", "#chi^{2} distribution;#chi^{2}", 15, chimin, chimax);
	SetHistProperties(h235, kRed, kFullSquare);
	SetHistProperties(h239, kBlue, kFullSquare);
	for (i=5; i<80; i++) {	// 0.5 - 10 MeV
		gr = MakeGraph(i);
		cv->Clear();
		gr->Fit(lFit);
		s235 = lFit->GetParameter(0);
		es235 = lFit->GetParError(0);
		s239 = lFit->GetParameter(1);
		es239 = lFit->GetParError(1);
		if (i < 57) hChi2->Fill(lFit->GetChisquare());	// till 7 MeV
		sprintf(str, "^{235}U: %6.3f#pm%6.3f", s235, es235);
		lt->DrawLatexNDC(0.6, 0.82, str);
		sprintf(str, "^{239}Pu: %6.3f#pm%6.3f", s239, es239);
		lt->DrawLatexNDC(0.6, 0.75, str);
		h235->SetBinContent(i, s235);
		h235->SetBinError(i, es235);
		h239->SetBinContent(i, s239);
		h239->SetBinError(i, es239);
		gr->Draw("ap");
		cv->Update();
		cv->SaveAs((oname + ".pdf").Data());
	}
	
	cv->Clear();
	TH1D *hMC235S = (TH1D*) hMC[0]->Clone("hMC235S");
	SetHistProperties(hMC235S, kRed, kFullSquare);
	TH1D *hMC239S = (TH1D*) hMC[2]->Clone("hMC239S");
	SetHistProperties(hMC239S, kBlue, kFullSquare);
	sum235 = h235->IntegralAndError(1, 128, esum235);
	sum239 = h239->IntegralAndError(1, 128, esum239);
	sumMC235 = hMC235S->IntegralAndError(1, 128, esumMC235);
	sumMC239 = hMC239S->IntegralAndError(1, 128, esumMC239);
	a = (sum235 + sum239) / (sumMC235 + sumMC239);
	a5 = sum235 / sumMC235;
	a9 = sum239 / sumMC239;
	ea5 = a5 * sqrt((esum235*esum235) / (sum235*sum235) + (esumMC235*esumMC235) / (sumMC235*sumMC235));
	ea9 = a9 * sqrt((esum239*esum239) / (sum239*sum239) + (esumMC239*esumMC239) / (sumMC239*sumMC239));
	ea = a * sqrt((esum235*esum235 + esum239*esum239) / ((sum235 + sum239)*(sum235 + sum239)) +
		(esumMC235*esumMC235 + esumMC239*esumMC239) / ((sumMC235 + sumMC239)*(sumMC235 + sumMC239)));
	hMC235S->Scale(a);
	hMC239S->Scale(a);
	h235->SetTitle("Extracted spectra");
	h235->SetMinimum(0);
	h235->GetXaxis()->SetRangeUser(0, 10);
	h235->Draw();
	h239->Draw("same");
	hMC235S->Draw("same,hist");
	hMC239S->Draw("same,hist");
	TLegend *lgB = new TLegend(0.65, 0.55, 0.89, 0.82);
	lgB->AddEntry(h235, "^{235}U", "p");
	lgB->AddEntry(h239, "^{239}Pu", "p");
	lgB->AddEntry(hMC235S, "HM ^{235}U", "l");
	lgB->AddEntry(hMC239S, "HM ^{239}Pu", "l");
	lgB->Draw();
	sprintf(str, "exp/MC. ^{235}U: %7.3f#pm%6.3f ^{239}Pu: %7.3f#pm%6.3f Sum: %7.3f#pm%6.3f", 
		a5, ea5, a9, ea9, a, ea);
	lt->DrawLatexNDC(0.3, 0.85, str);
	cv->SaveAs((oname + ".pdf").Data());
	cv->Clear();
	gStyle->SetOptStat(1001110);
	hChi2->Draw();
	cv->SaveAs((oname + ".pdf").Data());
	cv->SaveAs((oname + ".pdf]").Data());
	TFile *fOut = new TFile((oname + ".root").Data(), "RECREATE");
	h235->Write();
	h239->Write();
	hChi2->Write();
	fOut->Close();
	return 0;
}

