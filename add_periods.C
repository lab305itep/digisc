#include <string.h>
#define MAXHIST		10000

void add_periods(const char *newname, const char *perioddir)
{
	int i, j, K, N, L;
	char str[1024];
	TList *keys;
	TFile *fIn;
	TFile *f0;
	TSystemFile *fSys;
	char prefix[128];
	char *ptr;
	TNamed *obj;
	TH1D *h;
	TH1D *hist[MAXHIST];
	
	TSystemDirectory *dir = new TSystemDirectory("MyDir", perioddir);
	TList *files = dir->GetListOfFiles();
	if (!files) {
		printf("%s - nothing to do(files)\n", perioddir);
		delete dir;
		return;
	}
	N = files->GetEntries() - 2;
	if (N <= 1) {
		printf("%s - nothing to do\n", perioddir);
		delete dir;
		return;
	}
	
	TFile *fNew = new TFile(newname, "RECREATE");
	if (!fNew->IsOpen()) {
	    delete dir;
	    delete files;
	    return;
	}
	
	fSys = (TSystemFile *) files->At(2);
	if (!fSys) {
		printf("Can not open the first file\n");
		delete dir;
		delete files;
		return;
	}
	sprintf(str, "%s/%s", perioddir, fSys->GetName());
	f0 = new TFile(str);
	if (!f0->IsOpen()) {
		printf("Can not open the first file\n");
		delete dir;
		delete files;
		return;
	}
	keys = f0->GetListOfKeys();
	K = keys->GetEntries();
	if (K <= 0) {
		printf("Nothing to do: K=0\n");
		delete dir;
		delete files;
		return;
	}
	L = 0;
	for (j=0; j<K; j++) {
		obj = (TNamed *) keys->At(j);
		if (!obj) continue;
		obj = (TNamed *) f0->Get(obj->GetName());
		if (!obj) continue;
		if (strcmp(obj->ClassName(), "TH1D")) continue;
		hist[L] = (TH1D *)obj;
		L++;
	}
	if (!L) {
		printf("Nothing to do: L=0\n");
		f0->Close();
		delete dir;
		delete files;
		return;
	}
	
	for (i=1; i<N; i++) {
		fSys = (TSystemFile *) files->At(i+2);	// skip . and ..
		if (!fSys) continue;
		sprintf(str, "%s/%s", perioddir, fSys->GetName());
		fIn = new TFile(str);
		if (!fIn->IsOpen()) continue;
		for (j=0; j<L; j++) {
			h = (TH1D *) fIn->Get(hist[j]->GetName());
			if (!h) continue;
			hist[j]->Add(h);
		}
//		printf("%s\n", fSys->GetName());
		fIn->Close();
	}

	fNew->cd();
	for (j=0; j<L; j++) hist[j]->Write();
	fNew->Close();
	
	f0->Close();
	delete files;
	delete dir;
}
