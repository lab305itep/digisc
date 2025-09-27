#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <TChain.h>
#include <TFile.h>
#include <TH1D.h>

TChain *create_chain(const char *name, int from, int to)
{
	TChain *ch;
	char str[1024];
	int i;
	FILE *f_stat;
	int *rc_stat;
	char *ptr;
	int num;
	
	f_stat = fopen("stat_all.txt", "rt");
	if (!f_stat) {
		printf("Can not open stat file!\n");
		return NULL;
	}
	
	rc_stat = (int *) malloc((to - from + 1) * sizeof(int));
	if (!rc_stat) {
		printf("No memory !\n");
		return NULL;
	}
	memset(rc_stat, 0, (to - from + 1) * sizeof(int));
	for (;;) {
		ptr = fgets(str, sizeof(str), f_stat);
		if (!ptr) break;
		ptr = strtok(str, " \t");
		if (!ptr) continue;
		if (!isdigit(ptr[0])) continue;
		num = strtol(ptr, NULL, 10);
		if (num < from || num > to) continue;
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		if (!isdigit(ptr[0])) continue;
		rc_stat[num - from] = strtol(ptr, NULL, 10);
	}
	fclose(f_stat);
	
	ch = new TChain(name, name);
	for (i=from; i<=to; i++) {
		if (rc_stat[i - from] != 2 && rc_stat[i - from] != 3 && rc_stat[i - from] != 4 && rc_stat[i - from] != 5 && rc_stat[i - from] != 16) continue;
		sprintf(str, "muon_dir/%3.3dxxx/muon_%6.6d.root", i/1000, i);
		num = access(str, R_OK);	// R_OK = 4 - test read access
		if (num) continue;
		ch->AddFile(str, 0);
	}
	printf("%Ld entries found.\n", ch->GetEntries());
	
	free(rc_stat);
	
	return ch;
}

void makeExp_12B(int from, int to)
{
	char str[1024];

	TChain *chA = create_chain("MuonPair", from, to);
	TChain *chR = create_chain("MuonRandom", from, to);
	if (!chA || !chR) return;
	
	TFile *fOut = new TFile("Danss_12B_v8.7.root", "RECREATE");

	TH1D *hExp = new TH1D("hExp12B", "Experiment with ^{12}B cuts, SiPM+PMT;MeV", 80, 0, 20);
	TH1D *hExpSiPM = new TH1D("hExp12BSiPM", "Experiment with ^{12}B cuts, SiPM;MeV", 80, 0, 20);
	TH1D *hExpPMT = new TH1D("hExp12BPMT", "Experiment with ^{12}B cuts, PMT;MeV", 80, 0, 20);
	TH1D *hRndm = new TH1D("hRndm12B", "Random with ^{12}B cuts, SiPM+PMT;MeV", 80, 0, 20);
	TH1D *hRndmSiPM = new TH1D("hRndm12BSiPM", "Random with ^{12}B cuts, SiPM;MeV", 80, 0, 20);
	TH1D *hRndmPMT = new TH1D("hRndm12BPMT", "Random with ^{12}B cuts, PMT;MeV", 80, 0, 20);
	TH1D *hExpT = new TH1D("hExp12BT", "Time from muon, experiment;ms", 99, 1, 100);
	TH1D *hRndmT = new TH1D("hRndm12BT", "Time from muon, MC;ms", 99, 1, 100);

	chA->Project(hExp->GetName(), "ClusterEnergy", "gtDiff > 500");
	chR->Project(hRndm->GetName(), "ClusterEnergy", "gtDiff > 500");
	chA->Project(hExpSiPM->GetName(), "ClusterSiPmEnergy", "gtDiff > 500");
	chR->Project(hRndmSiPM->GetName(), "ClusterSiPmEnergy", "gtDiff > 500");
	chA->Project(hExpPMT->GetName(), "ClusterPmtEnergy", "gtDiff > 500");
	chR->Project(hRndmPMT->GetName(), "ClusterPmtEnergy", "gtDiff > 500");
	chA->Project(hExpT->GetName(), "gtDiff / 1000.0");
	chR->Project(hRndmT->GetName(), "gtDiff / 1000.0");
	
	hExp->Sumw2();
	hRndm->Sumw2();
	hExp->Add(hRndm, -1.0/16);
	hExpSiPM->Sumw2();
	hRndmSiPM->Sumw2();
	hExpSiPM->Add(hRndmSiPM, -1.0/16);
	hExpPMT->Sumw2();
	hRndmPMT->Sumw2();
	hExpPMT->Add(hRndmPMT, -1.0/16);
	hExpT->Sumw2();
	hRndmT->Sumw2();
	hExpT->Add(hRndmT, -1.0/16);
	fOut->cd();
	hExp->Write();
	hRndm->Write();
	hExpSiPM->Write();
	hRndmSiPM->Write();
	hExpPMT->Write();
	hRndmPMT->Write();
	hExpT->Write();
	hRndmT->Write();
	fOut->Close();
}
