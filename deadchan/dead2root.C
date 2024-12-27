void dead2root(const char *fname)
{
	struct {
		int Run;
		unsigned short Live;
		unsigned short N;
		unsigned short D[2500];
	} Dead;
	char str[20000];
	char *tok;
	int i;
	
	FILE *fIn = fopen(fname, "rt");
	if (!fIn) return;
	TString fn(fname);
	TFile *fOut = new TFile(fn.ReplaceAll(".txt", ".root").Data(), "RECREATE");
	TTree *tOut = new TTree("Dead", "Dead");
	tOut->Branch("Dead", &Dead, "Run/I:Live/s:N/s:D[N]/s");
	
	for(;!feof(fIn);) {
		if (!fgets(str, sizeof(str), fIn)) break;
		tok = strtok(str, " \t\n");
		if (!tok) continue;
		Dead.Run = strtol(tok, NULL, 10);
		if (Dead.Run <= 0) continue;
		tok = strtok(NULL, " \t\n");
		if (!tok) continue;
		Dead.Live = strtol(tok, NULL, 10);
		tok = strtok(NULL, " \t\n");
		if (!tok) continue;
		Dead.N = strtol(tok, NULL, 10);
		for (i=0; i<Dead.N; i++) {
			tok = strtok(NULL, " \t\n");
			if (!tok) break;
			Dead.D[i] = 100 * (strtod(tok, NULL) + 0.001);
		}
		tOut->Fill();
	}
	
	tOut->Write();
	fOut->Close();
}

void do_map(const char *fname, int from, int to)
{
	char strs[256], strl[1024];
	int i, j;
	double r;
	
	TFile *fOut = new TFile(fname, "UPDATE");
	TTree *tDead = (TTree *) fOut->Get("Dead");
	if (!tDead) {
		fOut->Close();
		return;
	}
	TString oname(fname);
	FILE *fList = fopen(oname.ReplaceAll(".root", ".list").Data(), "at");
	if (!fList) return;
	fprintf(fList, "%6d %6d ", from, to);
	sprintf(strs, "hMap_%d_%d", from, to);
	sprintf(strl, "Dead channels map runs %d - %d;Channel;Module", from, to);
	TH2D *hMap = new TH2D(strs, strl, 64, 0, 64, 51, 1, 52);
	sprintf(strl, "Run >= %d && Run <= %d", from, to);
	tDead->Project(hMap->GetName(), "D/100:D%100", strl);
	hMap->Scale(1.0/tDead->GetEntries(strl));
	
	sprintf(strs, "hOcc_%d_%d", from, to);
	sprintf(strl, "Dead channels occurence runs %d - %d;Occurence;N", from, to);
	TH1D *hOcc = new TH1D(strs, strl, 201, 0, 1.005);
	for (j=0; j<51; j++) for (i=0; i<64; i++) {
		r = hMap->GetBinContent(i+1, j+1);
		if (r) hOcc->Fill(r);
		if (r >= 0.1) fprintf(fList, " %2.2d.%2.2d", j+1, i);
	}
	fprintf(fList, "\n");
	fclose(fList);
	
	hMap->Write();
	hOcc->Write();
	fOut->Close();
}

void do_maps(const char *fname)
{
	const int range[][2] = {
		{2210, 3862},     {3863,  5000},    {5001,  8372},    {8373, 20144},    {20145, 29925}, 
		{29926, 37000},   {37001, 43368},   {43369, 45444},   {45445, 47419},   {47420, 47622},
		{47623, 49659},   {49660, 50398},   {50399, 59309},   {59310, 61539},   {61540, 62089}, 
		{62090, 64300},   {64301, 69760},   {69761, 70925},   {70926, 89039},   {89040, 93328},
		{93329, 93951},   {93952, 97020},   {97021, 99000},   {99001, 104425},  {104426, 105654},
		{105655, 106100}, {106101, 106726}, {106727, 106804}, {106805, 108214}, {108215, 108472},
		{108473, 109070}, {109071, 110801}, {110802, 137000}, {137001, 151800}, {151801, 156330},
		{156331, 159300}, {159301, 162600}, {162601, 200000}
	};
	int i;
	char strs[256];
	
	for (i=0; i< sizeof(range) / sizeof(range[0]); i++) do_map(fname, range[i][0], range[i][1]);
	
	gStyle->SetOptStat("e");
	TFile f(fname);
	if (!f.IsOpen()) return;
	TString fn(fname);
	TString pdf = fn.ReplaceAll(".root", ".pdf");
	
	TCanvas *cv = new TCanvas("CV", "CV", 800, 1200);
	cv->SaveAs((pdf + "[").Data());
	for (i=0; i< sizeof(range) / sizeof(range[0]); i++) {
		cv->Clear();
		cv->Divide(1, 2);
		cv->cd(1);
		sprintf(strs, "hMap_%d_%d", range[i][0], range[i][1]);
		TH2 *hMap = (TH2 *) f.Get(strs);
		if (hMap) {
			hMap->SetStats(0);
			hMap->Draw("colorz");
		}
		cv->cd(2);
		sprintf(strs, "hOcc_%d_%d", range[i][0], range[i][1]);
		TH1 *hOcc = (TH1 *) f.Get(strs);
		if (hOcc) hOcc->Draw();
		cv->SaveAs(pdf.Data());
	}
	cv->SaveAs((pdf + "]").Data());
}

