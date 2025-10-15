{
/*	const char *mcname[] = {
		"cm_MC_8.7_Center_Chikuma_Birks_el_0_0108",
		"cm_MC_8.7_Center_Chikuma_Birks_el_0_0308",
		"cm_MC_8.7_Center_Chikuma_Cher_coeff_0_033",
		"cm_MC_8.7_Center_Chikuma_Cher_coeff_0_233",
		"cm_MC_8.7_Center_Chikuma",
		"cm_MC_8.7_Center_Chikuma_main_Birks_0_0108",
		"cm_MC_8.7_Center_Chikuma_main_Birks_0_0308",
		"cm_MC_8.7_Center_Chikuma_paint_0_15",
		"cm_MC_8.7_Center_Chikuma_paint_0_45"};
*/

	const char *mcname[] = {
		"cm_MC_8.7_Center_Chikuma_xzmap",
		"cm_MC_8.7_Center_Chikuma_xzmap_Shielding"};

	int i;
	char str[1024];
	
	gROOT->ProcessLine(".L cm_capture_energy5.C");
	for (i=0;i<sizeof(mcname)/sizeof(mcname[0]);i++) {
		sprintf(str, "src_248CmMC(\"%s.root\", \"%s.hist.root\")", mcname[i], mcname[i]);
		gROOT->ProcessLine(str);
	}
}
