#include <string.h>

void join_periods(const char *newname, const char *perioddir)
{
	int i, j, K, N;
	char str[1024];
	TList *keys;
	TFile *fIn;
	TSystemFile *fSys;
	char prefix[128];
	char *ptr;
	TNamed *obj;
	
	TSystemDirectory *dir = new TSystemDirectory("MyDir", perioddir);
	TList *files = dir->GetListOfFiles();
	if (!files) return;
	N = files->GetEntries();
	if (N <= 0) {
		printf("%s - nothing to do\n", perioddir);
	}
	
	TFile *fNew = new TFile(newname, "RECREATE");
	if (!fNew->IsOpen()) return;

	for (i=0; i<N; i++) {
		fSys = (TSystemFile *) files->At(i);
		if (!fSys) continue;
//		printf("%d: %s\n", i, fSys->GetName());
		ptr = (char *) fSys->GetName();
		if (ptr[0] == '.') continue;	// skip . and ..
		if (strcmp(&ptr[strlen(ptr) - strlen(".root")], ".root")) continue;	// not a root file
		sprintf(str, "%s/%s", perioddir, ptr);
		fIn = new TFile(str);
		if (!fIn->IsOpen()) continue;
		keys = fIn->GetListOfKeys();
		K = keys->GetEntries();
		if (K <= 0) continue;
		strcpy(prefix, fSys->GetName());
		ptr = strstr(prefix, ".root");
		if (!ptr) continue;
		*ptr = '\0';
		for (j=0; j<K; j++) {
			obj = (TNamed *) keys->At(j);
			if (!obj) continue;
			obj = (TNamed *) fIn->Get(obj->GetName());
			if (!obj) continue;
			sprintf(str, "%s_%s", prefix, obj->GetName());
			obj->SetName(str);
			fNew->cd();
			obj->Write();
		}
//		printf("%s\n", fSys->GetName());
		fIn->Close();
	}
	
	fNew->Close();
	delete files;
	delete dir;
}
