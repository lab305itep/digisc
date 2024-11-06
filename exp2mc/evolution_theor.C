//	Compare MC to IBD spectrum
#define BASEDIR "/home/clusters/rrcmpi/alekseev/igor/pair8n2/MC/IBD"
#define EFF0 0.63		// nominal efficiency to events in pair files - to estimate number of events to process
#define EFISS 200.0		// nominal fission energy
#define CROSSAVR	6.5	// average IBD yield
#define CMPDIV 5		// number of campaign divisions
//#define EVTTOT 400000		// 100 days * 4000 events/day
#define EVTTOT 1000000

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
		add_IBD2chain(ch, "235U", 3, 4);
		break;
	case 238:
		add_IBD2chain(ch, "238U", 1, 4);
		break;
	case 239:
		add_IBD2chain(ch, "239Pu", 3, 4);
		break;
	case 241:
		add_IBD2chain(ch, "241Pu", 1, 4);
		break;
	default:
		delete ch;
		return NULL;
	}
	return ch;
}

TCut *IBDcuts(void)
{
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

	return new TCut(cX && cY && cZ && c20 && cPh && cR && cN && cSingle);
}

//	make theoretical spectrum for a given fuel mixture and total number of events N
TH1D *theorSpectrum(const char *name, const double fuel_fract[4], int N, int part)
{
	const int a_fuel[4] = {235, 238, 239, 241};
	const char *name_fuel[4] = {"235U", "238U", "239Pu", "241Pu"};
	const double cross_fuel[4] = {6.69, 10.10, 4.36, 6.04};
	const double EfisDB[4] = {202.36, 211.12, 205.99, 214.26};
	TH1D *hFuel[4];
	TChain *tFuel[4];
	int i;
	char str[1024];
	int Ncorr, Nmax, Nget, Ntot, Nbegin;

	TCut *cut = IBDcuts();
	Ncorr = N * EFISS * 100.0 / 
		(fuel_fract[0] * EfisDB[0] + fuel_fract[1] * EfisDB[1] + fuel_fract[2] * EfisDB[2] + fuel_fract[3] * EfisDB[3]); 
	Ntot = 0;
	printf("Ncorr = %d\n", Ncorr);
	for (i=0; i<4; i++) {
		tFuel[i] = make_MCchain(a_fuel[i]);
		if (!tFuel[i]) return NULL;
		sprintf(str, "hMC%s", name_fuel[i]);
		hFuel[i] = new TH1D(str, ";MeV", 32, 0, 8);
		Nmax = Ncorr * fuel_fract[i] * cross_fuel[i] / (100 * EFF0 * CROSSAVR);
		Nbegin = part * tFuel[i]->GetEntries() / CMPDIV;
		if (Nmax + Nbegin > tFuel[i]->GetEntries()) {
			printf("Not enough events for %s: %d+%d required\n", name_fuel[i], Nbegin, Nmax);
			return NULL;
		}
		Nget = tFuel[i]->Project(hFuel[i]->GetName(), "PositronEnergy", *cut, "", Nmax, Nbegin);
		printf("%s: %d events processed; %d got\n", name_fuel[i], Nmax, Nget);
		hFuel[i]->Sumw2();
		Ntot += Nget;
	}
	printf("Ntot = %d\n", Ntot);

	TH1D *h = new TH1D(name, ";MeV", 32, 0, 8);
	for (i=0; i<4; i++) h->Add(hFuel[i]);
	delete cut;
	for (i=0; i<4; i++) delete hFuel[i];
	for (i=0; i<4; i++) delete tFuel[i];
	return h;
}


//	Draw theoretical spectrum eveolution along a campaign
void evolution_theor(void)
{
//	Campaign 5, day 50, 150, 250, 350, 450
	const double fract[CMPDIV][4] = {
		/* 10/12/2017 */	{68.96, 6.86, 20.93, 3.25},
		/* 01/22/2018 */	{62.33, 7.03, 26.48, 4.16},
		/* 05/02/2018 */	{56.89, 7.16, 30.67, 5.27},
		/* 08/10/2018 */	{52.23, 7.30, 33.97, 6.50},
		/* 11/20/2018 */	{47.87, 7.47, 36.83, 7.83}};
		
	char str[1024];
	TLatex *txt;
	TLegend *lg;
	int j;
	TH1D *h[CMPDIV];
	TH1D *hd[CMPDIV];
	TH1D *hr[CMPDIV];
	TF1  *f[CMPDIV-1];
	const EColor color[CMPDIV] = {kBlack, kRed, kGreen, kBlue, kOrange};
	const EMarkerStyle marker[CMPDIV] = {kOpenCircle, kFullCircle, kFullSquare, kFullDiamond, kFullTriangleUp};
	double A, B, eA, eB;

	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	gStyle->SetLineWidth(2);
	gStyle->SetPadLeftMargin(0.15);
	
	TFile *fOut = new TFile("theor_evol.root", "RECREATE");
	TCanvas *cv = new TCanvas("CV", "CV", 1200, 1200);
	cv->Print("theor_evol.pdf[");
	txt = new TLatex();
	txt->SetTextSize(0.04);
	lg = new TLegend(0.6, 0.65, 0.89, 0.89);
	
	for (j=0; j<CMPDIV; j++) {
		sprintf(str, "hMC_%d", j+1);
		h[j] = theorSpectrum(str, fract[j], EVTTOT, j);
		if (!h[j]) return;
		h[j]->SetLineColor(color[j]);
		h[j]->SetMarkerColor(color[j]);
		h[j]->SetMarkerStyle(marker[j]);
		h[j]->SetMarkerSize(2.5);
		h[j]->SetTitle("MC spectra per period;Positron energy, MeV;Events/bin");
		h[j]->GetXaxis()->SetRangeUser(0, 6);
		sprintf(str, "hDMC_%d", j+1);
		hd[j] = (TH1D *) h[j]->Clone(str);
		hd[j]->SetTitle("MC, spectra rate and shape difference from the first period;Positron energy, MeV;Events/bin");
		hd[j]->Add(h[j], h[0], 1, -1);
		sprintf(str, "hRMC_%d", j+1);
		hr[j] = (TH1D *) h[j]->Clone(str);
		hr[j]->SetTitle("MC, spectra ratio to the first period;Positron energy, MeV;");
		hr[j]->Divide(h[j], h[0]);
		hr[j]->SetMinimum(0.83);
		hr[j]->SetMaximum(1.12);
		h[j]->Draw((j) ? "same" : "");
		sprintf(str, "Period %d", j+1);
		lg->AddEntry(h[j], str, "pl");
		fOut->cd();
		h[j]->Write();
		hr[j]->Write();
		hd[j]->Write();
	}
	lg->Draw();
	cv->Print("theor_evol.pdf");
	for (j=CMPDIV-1; j>0; j--) hd[j]->Draw((j != CMPDIV - 1) ? "same" : "");
	cv->Print("theor_evol.pdf");
	for (j=0; j<CMPDIV-1; j++) {
		sprintf(str, "fLin%d", j);
		f[j] = new TF1(str, "pol1", 1, 6);
		f[j]->SetLineColor(color[j+1]);
	}
	for (j=CMPDIV-1; j>0; j--) {
		hr[j]->Fit(f[j-1], "Q", (j != CMPDIV - 1) ? "same" : "", 0.75, 6);
		B = f[j-1]->GetParameter(0);
		eB = f[j-1]->GetParError(0);
		A = f[j-1]->GetParameter(1);
		eA = f[j-1]->GetParError(1);
		txt->SetTextColor(color[j]);
		sprintf(str, "(%5.3f#pm%5.3f) + (%5.3f#pm%5.3f)#bullet#it{X}", B, eB, A, eA);
		txt->DrawLatexNDC(0.18, 0.9 - 0.055*j, str);
	}
	cv->Print("theor_evol.pdf");

	cv->Print("theor_evol.pdf]");
	fOut->Close();
}
