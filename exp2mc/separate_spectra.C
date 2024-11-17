//	Compare MC to IBD spectrum
#define BASEDIR "/home/clusters/rrcmpi/alekseev/igor/pair8n2/MC/IBD_large_fuel"

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
