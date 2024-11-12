//	Make positions.h and positions.txt file out of stat_all and fission fractions
#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char * strptime (const char *S, const char *FMT, struct tm *TP);

#define MAXRUN		300000
#define MAXDAYS		10000
#define MAXTOKEN	30
#define POWER_ZERO	10		// MW
#define POWER_FULL	2850		// MW
#define MINTIME		(5*3600)	// 5 hours
#define MAXTIME		(240*3600)	// 10 days
#define MAXBREAK	(24*3600)	// 1 day

#define TOK_RUN	0
#define TOK_TYP	1
#define TOK_DAY	2
#define TOK_TIM	3
#define TOK_NVT	11
#define TOK_20M	14
#define TOK_TOT	17
#define TOK_DED	19
#define TOK_EFF	21
#define TOK_PWR	22

#define FTOK_DAY	0
#define FTOK_DATE	1
#define FTOK_TIM	2
#define FTOK_U235	3
#define FTOK_U238	5
#define FTOK_PU239	6
#define FTOK_PU241	8

struct StatAllStruct {
	int 	type;
	time_t	start;
	double	no_veto;
	double	time_total;
	double	time_dead;
	double	eff;
	double	power;
	double U235;
	double U238;
	double Pu239;
	double Pu241;
};

struct FissFractStruct {
	time_t	date;
	double U235;
	double U238;
	double Pu239;
	double Pu241;
};

enum PowerType {zero_power = 0, some_power = 1, full_power = 2};

int GlobalPeriod(int run, enum PowerType ptype)
{
	if (ptype != full_power) return 0;	// reactor OFF
	if (run < 5000) return 1;		// April-June 2016
	if (run < 24000) return 2;		// October 2016 - July 2017: till the end of Campaign 4
	if (run < 37000) return 3;		// August 2017 - March 2018: Campaign 5 till detector repair
	if (run < 56000) return 4;		// May 2018 - January 2019: after detector repair till the end of  Campaign 5
	if (run < 87500) return 5;		// Campaign 6
	if (run < 117700) return 6;		// Campaign 7
	if (run < 147000) return 7;		// Campaign 8
	return 8;				// Campaign 9
}

void StrSplit(char *str, char **token)
{
	int i;
	const char DELIM[] = " \t\r\n";
	
	memset(token, 0, MAXTOKEN * sizeof(char *));
	for (i=0; i<MAXTOKEN; i++) {
		token[i] = strtok((i) ? NULL : str, DELIM);
		if (!token[i]) break;
	}
}

time_t ConvertTime(const char *date, const char *time)
{
	struct tm tm_time;
	
	memset(&tm_time, 0, sizeof(tm_time));
	if (strstr(date, "-")) {
		strptime(date, "%F", &tm_time);
	} else {
		strptime(date, "%d.%m.%C%y", &tm_time);
	}
	strptime(time, "%H:%M", &tm_time);
	return mktime(&tm_time);
}

void PeriodName(char *name, int type, time_t time)
{
	const char *posName[] = {
		"xfile", "xpos", "down", "mid",  "up",   "stuck", "xold", "xold", "xold", "xold", 
		"xold",  "xsrc", "xsrc", "xsrc", "xsrc", "xled", "raised", "xsrc"};
	char *ptr;
	struct tm tm_time;
	
	localtime_r(&time, &tm_time);
	sprintf(name, "%s%2.2d_%2.2d.%2.2d.%2.2d", posName[type], tm_time.tm_hour, tm_time.tm_mday, tm_time.tm_mon + 1, tm_time.tm_year % 100);
}

int ReadFiss(const char *name, struct FissFractStruct *Fiss)
{
	FILE *fFiss;
	char str[1024];
	char *ptr;
	char *token[MAXTOKEN];
	int irc;
	int cnt;
	
	fFiss = fopen(name, "rt");
	if (!fFiss) {
		printf("Can not open Fissions-file %s: %m\n", name);
		return 10;
	}
	memset(Fiss, 0, MAXDAYS * sizeof(struct FissFractStruct));
	cnt = 0;
	for (;;) {
		ptr = fgets(str, sizeof(str), fFiss);
		if (!ptr) break;	// EOF
		StrSplit(str, token);
		//	Getting run number
		if (!token[FTOK_PU241]) continue;
		if (!isdigit(token[FTOK_DAY][0])) continue;	// comment
		Fiss[cnt].date = ConvertTime(token[FTOK_DATE], token[FTOK_TIM]);
		Fiss[cnt].U235 = strtod(token[FTOK_U235], NULL);
		Fiss[cnt].U238 = strtod(token[FTOK_U238], NULL);
		Fiss[cnt].Pu239 = strtod(token[FTOK_PU239], NULL);
		Fiss[cnt].Pu241 = strtod(token[FTOK_PU241], NULL);
		cnt++;
		if (cnt >= MAXDAYS) {
			printf("No space for fissions data - increase MAXDAYS.\n");
			return 20;
		}
	}
	
	fclose(fFiss);
	return 0;
}

struct FissFractStruct *findFissions(time_t date, struct FissFractStruct *Fiss)
{
	int i;
	
	for (i=0; i<MAXDAYS && Fiss[i].date>0; i++) if (Fiss[i].date > date) break;
	if (i > 0) i--;
	return (&Fiss[i]);
}

int ReadStat(const char *name, struct StatAllStruct *Stat, struct FissFractStruct *Fiss)
{
	FILE *fStat;
	char str[1024];
	char *ptr;
	char *token[MAXTOKEN];
	int irc;
	int type;
	double d;
	struct FissFractStruct *FissPtr;
	
	fStat = fopen(name, "rt");
	if (!fStat) {
		printf("Can not open stat-file %s: %m\n", name);
		return 10;
	}
	memset(Stat, 0, MAXRUN * sizeof(struct StatAllStruct));
	
	for (;;) {
		ptr = fgets(str, sizeof(str), fStat);
		if (!ptr) break;	// EOF
		StrSplit(str, token);
		//	Getting run number
		if (!token[TOK_PWR]) continue;
		irc = strtol(token[TOK_RUN], NULL, 10);
		type = strtol(token[TOK_TYP], NULL, 10);
		if (irc <= 0 || type <= 0) continue;
		if (irc >= MAXRUN) {
			printf("Run %d > MAXRUN=%d met = skipping\n", irc, MAXRUN);
			continue;
		}
		Stat[irc].type = type;
		Stat[irc].start = ConvertTime(token[TOK_DAY], token[TOK_TIM]);
		d = strtod(token[TOK_20M], NULL);
		Stat[irc].no_veto = (d > 0) ? strtod(token[TOK_NVT], NULL) / d : 0;
		Stat[irc].time_total = strtod(token[TOK_TOT], NULL);
		Stat[irc].time_dead = strtod(token[TOK_DED], NULL);
		Stat[irc].eff = strtod(token[TOK_EFF], NULL);
		Stat[irc].power = strtod(token[TOK_PWR], NULL);
		if (Stat[irc].power >= POWER_ZERO) {
			FissPtr = findFissions(Stat[irc].start, Fiss);
			if (FissPtr) {
				Stat[irc].U235 = FissPtr->U235;
				Stat[irc].U238 = FissPtr->U238;
				Stat[irc].Pu239 = FissPtr->Pu239;
				Stat[irc].Pu241 = FissPtr->Pu241;
			}
		}
	}
	
	fclose(fStat);
	return 0;
}

enum PowerType GetPower(double power)
{
	if (power < POWER_ZERO) {
		return zero_power;
	} else if (power > POWER_FULL) {
		return full_power;
	} else {
		return some_power;
	}
}

void WritePositionsFile(const char *name, struct StatAllStruct *Stat)
{
	FILE *fOut;
	FILE *fH;
	int run, run_begin, run_end;
	int type, type_begin;
	enum PowerType ptype, ptype_begin;
	int N;
	double acq_time;
	int period_begin, period_end, period_runs;
	double period_total, period_dead, period_power, period_no_veto, period_eff;
	double f235, f238, f239, f241;
	time_t period_start;
	time_t run_end_time;
	char period_name[128];
	int i, j;
	char str[1024];
	const char *cpower_type[] = {"ZERO", "SOME", "FULL"};
	double Sum[3][2];
	
	sprintf(str, "%s.txt", name);
	fOut = fopen(str, "wt");
	if (!fOut) {
		printf("Can not open output file %s: %m\n", str);
		return;
	}
	sprintf(str, "%s.h", name);
	fH = fopen(str, "wt");
	if (!fH) {
		printf("Can not open output file %s: %m\n", str);
		return;
	}
	
	fprintf(fOut, "  Name              \t  Begin   End   P      Power  NoVeto  Total     Dead    %%       Eff     U235  U238  Pu239  Pu241\n");
	
	run = 0;
	memset(Sum, 0, sizeof(Sum));
	for(;;) {
//		Search for uniform period
//			search for the first valid run
		for (; run<MAXRUN; run++) {
			type = Stat[run].type;
			if (type == 2 || type == 3 || type == 4 || type == 5 || type == 16) break;// down, up, mid, stuck, raised
		}
		if (run >= MAXRUN) break;		// end of runs
		run_begin = run;
		type_begin = type;
		ptype_begin = GetPower(Stat[run].power);
		acq_time = Stat[run].time_total;
		run_end_time = Stat[run].start + Stat[run].time_total;
//			search for a run of different kind (type, ptype)
		for (; run<MAXRUN; run++) {
			type = Stat[run].type;
			if (type <= 0) continue;	// we ignore breaks
			ptype = GetPower(Stat[run].power);
			if (type != type_begin || ptype != ptype_begin) break;
			if (Stat[run].start - run_end_time > MAXBREAK) break;
			run_end_time = Stat[run].start + Stat[run].time_total;
			acq_time += Stat[run].time_total;
			run_end = run;
		}
		if (acq_time < MINTIME) continue;	// period too short
		N = acq_time / MAXTIME + 1;
		period_begin = run_begin;
		for (i=0; i<N; i++) {
			for(;period_begin < run_end; period_begin++) if (Stat[period_begin].type > 0) break;
			period_total = 0;
			period_dead = 0;
			period_power = 0;
			period_runs = 0;
			period_eff = 0;
			f235 = f238 = f239 = f241 = 0;
			period_start = Stat[period_begin].start;
			for(j=period_begin; j<run_end; j++) {
				if (Stat[j].type <= 0) continue;	// we ignore breaks
				period_total += Stat[j].time_total;
				period_dead += Stat[j].time_dead;
				period_power += Stat[j].power;
				period_no_veto += Stat[j].no_veto * Stat[j].time_total;
				period_eff += Stat[j].eff;
				f235 += Stat[j].U235;
				f238 += Stat[j].U238;
				f239 += Stat[j].Pu239;
				f241 += Stat[j].Pu241;
				period_runs++;
				if (period_total >= acq_time / N) break;
			}
			period_end = j;
			period_power /= period_runs;
			period_eff /= period_runs;
			period_no_veto /= period_total;
			f235 /= period_runs;
			f238 /= period_runs;
			f239 /= period_runs;
			f241 /= period_runs;
			PeriodName(period_name, type_begin, period_start);
			fprintf(fOut, "%20s\t%6d  %6d  %4s  %5.0f   %5.3f  %8.1f  %7.1f  %6.4f  %7.5f %5.2f %5.2f %5.2f %5.2f\n",
				period_name, period_begin, period_end, cpower_type[ptype_begin], period_power,
				period_no_veto, period_total, period_dead, period_dead / period_total, period_eff, f235, f238, f239, f241);
			fprintf(fH, "\t\t{ \"%s\",   %5d, %5d, %d, %5.3f},\n",
				period_name, period_begin, period_end, GlobalPeriod(period_begin, ptype_begin), 
				1.55 * period_no_veto);
				
			period_begin = period_end + 1;
			if (ptype_begin == 2) switch(period_name[0]) {
			case 'u' :
				Sum[0][0] += period_total;
				Sum[0][1] += period_dead;
				break;
			case 'm' :
				Sum[1][0] += period_total;
				Sum[1][1] += period_dead;
				break;
			case 'd' :
				Sum[2][0] += period_total;
				Sum[2][1] += period_dead;
				break;
			}
		}
	}
	fclose(fOut);
	printf("Dead time fraction   UP = %f (%f / %f)\n", (Sum[0][0] != 0) ? Sum[0][1] / Sum[0][0] : 0, Sum[0][1], Sum[0][0]);
	printf("Dead time fraction  MID = %f (%f / %f)\n", (Sum[1][0] != 0) ? Sum[1][1] / Sum[1][0] : 0, Sum[1][1], Sum[1][0]);
	printf("Dead time fraction DOWN = %f (%f / %f)\n", (Sum[2][0] != 0) ? Sum[2][1] / Sum[2][0] : 0, Sum[2][1], Sum[2][0]);
}

int main(int argc, char **argv)
{
	struct StatAllStruct *Stat;
	struct FissFractStruct *Fiss;
	int irc;
	
	if (argc < 4) {
		printf("Usage: %s stat_all_file fiss_fract_file positions_file\n", argv[0]);
		return 10;
	}
	
	Fiss = (struct FissFractStruct *) malloc(MAXDAYS * sizeof(struct FissFractStruct));
	Stat = (struct StatAllStruct *) malloc(MAXRUN * sizeof(struct StatAllStruct));
	if (!Stat || !Fiss) {
		printf("No memory: %m\n");
		return 20;
	}
	irc = ReadFiss(argv[2], Fiss);
	if (irc) {
		printf("Can not read fissions file\n");
		return 27;
	}
	irc = ReadStat(argv[1], Stat, Fiss);
	if (irc) {
		printf("Can not read stat file\n");
		return 30;
	}
	
	WritePositionsFile(argv[3], Stat);
}
