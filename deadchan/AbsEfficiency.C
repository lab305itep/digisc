#define NHISTS	7

TChain *makeChain(const char *list, const char *name)
{
	char str[2048];
	FILE *f = fopen(list, "rt");
	if (!f) {
		printf("File %s not opened: %m\n", list);
		return NULL;
	}
    
	TChain *ch = new TChain(name);
	for (;;) {
		if (!fgets(str, sizeof(str), f)) break;
		if (strlen(str) < 2 || str[0] == ' ' || str[0] == '#') continue;
		str[strlen(str) - 1] = '\0';	// remove <LF>
		if (access(str, R_OK)) {
			printf("A problem accessing file %s: %m\n", str);
			continue;
		}
		ch->AddFile(str);
	}
	if (ch->GetEntries() == 0) {
		printf("No records found in chain=%s    list=%s\n", name, list);
		delete ch;
		return NULL;
	}

	return ch;
}

//	calculate hists for efficiency as function of positron energy
//	mcorig - list of original fuel MC root files
//	mcprc - list of processed fuel MC root files
//	infoprc - list of original MC information files 
void AbsEfficiency(const char *mcorig, const char *mcprc, const char *infoprc, const char *resname)
{
	int i, j;
	TH1D *hPrc[2][NHISTS];
	char strs[128], strl[1024];
	
	TCut cVeto("gtFromVeto > 60 && EventsBetween == 0");
	TCut cN("NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20 && NeutronEnergy > 2");
	TCut cP("PositronEnergy > 0.5");
	TCut cT("gtDiff > 1");
	TCut cBase = cVeto && cN && cP && cT;
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cXYZ = cX && cY && cZ;
	TCut cPh("PositronHits < 6");
	TCut cNE("NeutronEnergy > 4.7 - 0.77 * PositronEnergy");
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 7 && AnnihilationMax < 0.8");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut cR2("Distance < 48 && Distance < 19 + 6.4 * PositronEnergy");
	TCut cR3("Distance < 52 && Distance < 28 + 5.5 * PositronEnergy && Distance < 81 - 5.3 * PositronEnergy");
	TCut cR = cR3 && (cRXY || cR2);
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))");

	const struct {
		const char *name;
		const char *title;
		const TCut cut;
	} Hists[NHISTS] = {
		{"Basic", "Basic cuts only", cBase},
		{"XYZ", "Fiducal volume", cBase && cXYZ},
		{"PH", "Positron hits", cBase && cXYZ && cPh},
		{"NE", "Capture energy", cBase && cXYZ && cPh && cNE},
		{"AH", "Annihilation gammas", cBase && cXYZ && cPh && cNE && cGamma},
		{"R", "Positron to neutron distance", cBase && cXYZ && cPh && cNE && cGamma && cR},
		{"1", "Single hit clusters", cBase && cXYZ && cPh && cNE && cGamma && cR && cSingle}
	};

	TChain *Orig = makeChain(mcorig, "DANSSParticle");
	TChain *Prc  = makeChain(mcprc, "DanssPair");
	TChain *Info  = makeChain(infoprc, "FromMC");
	if (!Orig || !Prc) return;	// nothing to do
	if (Info->GetEntries() != Prc->GetEntries()) {
		printf("Info and Prc chains mismatch!\n");
		return;
	}
	Prc->AddFriend(Info);
	
	TH1D *hOrig = new TH1D("hOrig", "Positron energy spectrum generated;E_{e^{+}}, MeV;N", 80, 0, 10.0);
	Orig->Project(hOrig->GetName(), "ParticleEnergy", "ID==1");
	hOrig->Sumw2();
	
	for (j=0; j<2; j++) for (i=0; i<NHISTS; i++) {
		sprintf(strs, "h%s_%c", Hists[i].name, (j) ? 'R' : 'M');
		sprintf(strl, "Positron energy distribution, cut( %s ), %s;E_{e^{+}}, MeV;N", Hists[i].title, (j) ? "reconstructed" : "MC-truth");
		hPrc[j][i] = new TH1D(strs, strl, 80, 0, 10.0);
		Prc->Project(hPrc[j][i]->GetName(), (j) ? "PositronEnergy" : "MCPositronEnergy", Hists[i].cut);
		hPrc[j][i]->Sumw2();
	}
	
	TFile fOut(resname, "RECREATE");
	hOrig->Write();
	for (j=0; j<2; j++) for (i=0; i<NHISTS; i++) hPrc[j][i]->Write();
	fOut.Close();
	delete Orig;
	delete Prc;
	delete Info;
}
