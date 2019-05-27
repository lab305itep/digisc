#ifndef EVTBUILDER_H
#define EVTBUILDER_H

//	Glaobal
#define GLOBALFREQ	125000000.0		// 125 Mhz

/***********************	Types		****************************/

struct DanssEventStruct {
//		Common parameters
	long long	globalTime;		// time in terms of 125 MHz
	long long	number;			// event number in the file
	int		unixTime;		// linux time, seconds
	float		fineTime;		// fine time of the event (for hit selection)
//		Veto parameters
	int		VetoHits;		// hits in the event record
	float		VetoEnergy;		// Energy Sum of all hits
	int		VetoCleanHits;		// hits above threshold and in time window
	float		VetoCleanEnergy;	// Energy Sum of clean hits
//		PMT parameters
	int		PmtHits;		// the same as above for PMT
	float		PmtEnergy;
	int		PmtCleanHits;
	float		PmtCleanEnergy;
//		SiPM parameters
	int		SiPmHits;		// the same as above for PMT
	float		SiPmEnergy;
	int		SiPmCleanHits;
	float		SiPmCleanEnergy;
	int		SiPmEarlyHits;		// to understand random background
	float		SiPmEarlyEnergy;
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronSiPmEnergy;	// Energy sum of the cluster (SiPM)
	float		MaxHitEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
//		"neutron" parameters
	int		NeutronHits;		// number of hits considered as neutron capture gammas
	float		NeutronSiPmEnergy;	// Energy sum of above (SiPM)
	float		NeutronX[3];		// center of gammas position
	float		NeutronGammaEnergy[5];	// sorted list of the 5 most energetic gammas
	float		NeutronGammaDistance[5];	// distances for the gammas above to the "neutron" center
	float		NeutronRadius;		// average distance between hits and the center
};

struct DanssEventStruct2 {
//		Common parameters
	long long	globalTime;		// time in terms of 125 MHz
	long long	number;			// event number in the file
	int		unixTime;		// linux time, seconds
	float		fineTime;		// fine time of the event (for hit selection)
//		Veto parameters
	int		VetoHits;		// hits in the event record
	float		VetoEnergy;		// Energy Sum of all hits
	int		VetoCleanHits;		// hits above threshold and in time window
	float		VetoCleanEnergy;	// Energy Sum of clean hits
//		PMT parameters
	int		PmtHits;		// the same as above for PMT
	float		PmtEnergy;
	int		PmtCleanHits;
	float		PmtCleanEnergy;
//		SiPM parameters
	int		SiPmHits;		// the same as above for PMT
	float		SiPmEnergy;
	int		SiPmCleanHits;
	float		SiPmCleanEnergy;
	int		SiPmEarlyHits;		// to understand random background
	float		SiPmEarlyEnergy;
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronSiPmEnergy;	// Energy sum of the cluster, corrected (SiPM)
	float		PositronPmtEnergy;	// Energy sum of the cluster, corrected (PMT)
	float		MaxHitEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
//		"neutron" parameters
	float		NeutronX[3];		// center of gammas position
	float		NeutronRadius;		// average distance between hits and the center
};

struct DanssEventStruct3 {
//		Common parameters
	long long	globalTime;		// time in terms of 125 MHz
	long long	number;			// event number in the file
	int		runNumber;		// the run number
	int		unixTime;		// linux time, seconds
	float		fineTime;		// fine time of the event (for hit selection)
//		Veto parameters
	int		VetoHits;		// hits in the event record
	float		VetoEnergy;		// Energy Sum of all hits
	int		VetoCleanHits;		// hits above threshold and in time window
	float		VetoCleanEnergy;	// Energy Sum of clean hits
//		PMT parameters
	int		PmtHits;		// the same as above for PMT
	float		PmtEnergy;
	int		PmtCleanHits;
	float		PmtCleanEnergy;
//		SiPM parameters
	int		SiPmHits;		// the same as above for PMT
	float		SiPmEnergy;
	int		SiPmCleanHits;
	float		SiPmCleanEnergy;
	int		SiPmEarlyHits;		// to understand random background
	float		SiPmEarlyEnergy;
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	int		PositronFlags;		// Positron flags
	float		PositronMinLen;		// Minimum track length to create the cluster
	float		PositronEnergy;		// Energy sum of the cluster, corrected (SiPM+PMT)
	float		TotalEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
//		"neutron" parameters
	float		NeutronX[3];		// center of gammas position
	float		NeutronRadius;		// average distance between hits and the center
};

struct DanssEventStruct4 {
//		Common parameters
	long long	globalTime;		// time in terms of 125 MHz
	long long	number;			// event number in the file
	int		runNumber;		// the run number
	int		unixTime;		// linux time, seconds
	float		fineTime;		// fine time of the event (for hit selection)
//		Veto parameters
	int		VetoHits;		// hits in the event record
	float		VetoEnergy;		// Energy Sum of all hits
	int		VetoCleanHits;		// hits above threshold and in time window
	float		VetoCleanEnergy;	// Energy Sum of clean hits
//		PMT parameters
	int		PmtHits;		// the same as above for PMT
	float		PmtEnergy;
	int		PmtCleanHits;
	float		PmtCleanEnergy;
//		SiPM parameters
	int		SiPmHits;		// the same as above for PMT
	float		SiPmEnergy;
	int		SiPmCleanHits;
	float		SiPmCleanEnergy;
	int		SiPmEarlyHits;		// to understand random background
	float		SiPmEarlyEnergy;
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	int		PositronFlags;		// Positron flags
	float		PositronMinLen;		// Minimum track length to create the cluster
	float		PositronEnergy;		// Energy sum of the cluster, corrected (SiPM+PMT)
	float		TotalEnergy;		// Event full energy correctd (SiPM+PMT)
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
//		"neutron" parameters
	float		NeutronX[3];		// center of gammas position
//	float		NeutronRadius;		// average distance between hits and the center
	int		NHits;			// Number of hits
};

struct DanssEventStruct5 {
//		Common parameters
	long long	globalTime;		// time in terms of 125 MHz
	long long	number;			// event number in the file
	int		runNumber;		// the run number
	int		unixTime;		// linux time, seconds
	float		fineTime;		// fine time of the event (for hit selection)
//		Veto parameters
	int		VetoHits;		// hits in the event record
	float		VetoEnergy;		// Energy Sum of all hits
	int		VetoCleanHits;		// hits above threshold and in time window
	float		VetoCleanEnergy;	// Energy Sum of clean hits
//		PMT parameters
	int		PmtHits;		// the same as above for PMT
	float		PmtEnergy;
	int		PmtCleanHits;
	float		PmtCleanEnergy;
//		SiPM parameters
	int		SiPmHits;		// the same as above for PMT
	float		SiPmEnergy;
	int		SiPmCleanHits;
	float		SiPmCleanEnergy;
	int		SiPmEarlyHits;		// to understand random background
	float		SiPmEarlyEnergy;
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	int		PositronFlags;		// Positron flags
	float		PositronMinLen;		// Minimum track length to create the cluster
	float		PositronEnergy;		// Energy sum of the cluster, corrected (SiPM+PMT)
	float		TotalEnergy;		// Event full energy correctd (SiPM+PMT)
	float		PositronSiPmEnergy;	// SiPM energy in the cluster, corrected
	float		PositronPmtEnergy;	// PMT energy in the cluster, corrected
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
//		"neutron" parameters
	float		NeutronX[3];		// center of gammas position
//	float		NeutronRadius;		// average distance between hits and the center
	int		NHits;			// Number of hits
};

//	Positron flag masks
#define PFLAG_MAXENERGY		1		// Too much energy in DANSS
#define PFLAG_NOCLUSTER		2		// No cluster
#define PFLAG_INVCLUSTER	4		// Invalid cluster
#define PFLAG_CHIT		0x3FF0		// Cluster hit in an edge
#define PFLAG_CHIT_U1		0x10		// Cluster hit in Upper edge 1
#define PFLAG_CHIT_U2		0x20		// Cluster hit in Upper edge 2
#define PFLAG_CHIT_U34		0x40		// Cluster hit in Upper edge 34
#define PFLAG_CHIT_D1		0x80		// Cluster hit in Down edge 1
#define PFLAG_CHIT_D2		0x100		// Cluster hit in Down edge 2
#define PFLAG_CHIT_D34		0x200		// Cluster hit in Down edge 34
#define PFLAG_CHIT_E		0x400		// Cluster hit in East edge
#define PFLAG_CHIT_W		0x800		// Cluster hit in West edge
#define PFLAG_CHIT_N		0x1000		// Cluster hit in North edge
#define PFLAG_CHIT_S		0x2000		// Cluster hit in South edge
#define PFLAG_HIT		0x3FF0000	// hit in an edge
#define PFLAG_HIT_U1		0x10000		// hit in Upper edge 1
#define PFLAG_HIT_U2		0x20000		// hit in Upper edge 2
#define PFLAG_HIT_U34		0x40000		// hit in Upper edge 34
#define PFLAG_HIT_D1		0x80000		// hit in Down edge 1
#define PFLAG_HIT_D2		0x100000	// hit in Down edge 2
#define PFLAG_HIT_D34		0x200000	// hit in Down edge 34
#define PFLAG_HIT_E		0x400000	// hit in East edge
#define PFLAG_HIT_W		0x800000	// hit in West edge
#define PFLAG_HIT_N		0x1000000	// hit in North edge
#define PFLAG_HIT_S		0x2000000	// hit in South edge

struct DanssEventStruct6 {
//		Common parameters
	long long	globalTime;		// time in terms of 125 MHz
	long long	number;			// event number in the file
	int		runNumber;		// the run number
	int		unixTime;		// linux time, seconds
	float		fineTime;		// fine time of the event (for hit selection)
//		Veto parameters
	int		VetoCleanHits;		// hits above threshold and in time window
	float		VetoCleanEnergy;	// Energy Sum of clean hits
//		PMT parameters
	int		PmtCleanHits;
	float		PmtCleanEnergy;
//		SiPM parameters
	int		SiPmCleanHits;
	float		SiPmCleanEnergy;
	int		SiPmEarlyHits;		// to understand random background
	float		SiPmEarlyEnergy;
//		"positron cluster" parameters
	int		PositronHits;		// SiPm hits in the cluster
	float		PositronEnergy;		// Energy sum of the cluster, corrected (SiPM+PMT)
	float		TotalEnergy;		// Event full energy correctd (SiPM+PMT)
	float		PositronSiPmEnergy;	// SiPM energy in the cluster, corrected
	float		PositronPmtEnergy;	// PMT energy in the cluster, corrected
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
	float		MinPositron2GammaZ;	// Z-distance to the closest gamma
//		"neutron" parameters
	float		NeutronX[3];		// center of gammas position
//	float		NeutronRadius;		// average distance between hits and the center
	int		NHits;			// Number of hits
};

struct DanssEventStruct7 {
//		Common parameters
	long long	globalTime;		// time in terms of 125 MHz
	long long	number;			// event number in the file
	int		unixTime;		// linux time, seconds
	float		fineTime;		// fine time of the event (for hit selection)
	int		trigType;		// type of the trigger
//		Veto parameters
	int		VetoCleanHits;		// hits above threshold and in time window
	float		VetoCleanEnergy;	// Energy Sum of clean hits
	float		BottomLayersEnergy;	// Energy in the two bottom SiPM layers to be used in Veto
//		PMT parameters
	int		PmtCleanHits;
	float		PmtCleanEnergy;
//		SiPM parameters
	int		SiPmHits;
	float		SiPmEnergy;
	int		SiPmCleanHits;
	float		SiPmCleanEnergy;
	int		SiPmEarlyHits;		// to understand random background
	float		SiPmEarlyEnergy;
//		"positron cluster" parameters
	int		PositronHits;		// SiPm hits in the cluster
	float		PositronEnergy;		// Energy sum of the cluster, longitudinally corrected (SiPM+PMT)
	float		TotalEnergy;		// Event full energy  longitudinally correctd (SiPM+PMT)
	float		PositronSiPmEnergy;	// SiPM energy in the  longitudinally cluster, corrected
	float		PositronPmtEnergy;	// PMT energy in the  longitudinally cluster, corrected
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
	float		MinPositron2GammaZ;	// Z-distance to the closest gamma
//		"neutron" parameters
	float		NeutronX[3];		// center of gammas position
	int		NHits;			// Number of hits
};

struct HitTypeStruct {
	char 	type;
	char	z;
	char	xy;
	char	flag;
};

struct DanssInfoStruct {
	long long	upTime;			// running time in terms of 125 MHz
	int		startTime;		// linux start time, seconds
	int		stopTime;		// linux stop time, seconds
	long long	events;			// number of events
};

struct DanssInfoStruct3 {
	long long	upTime;			// running time in terms of 125 MHz
	int		runNumber;		// the run number
	int		startTime;		// linux start time, seconds
	int		stopTime;		// linux stop time, seconds
	int		events;			// number of events
};

struct DanssInfoStruct4 {
	long long	upTime;			// running time in terms of 125 MHz
	int		runNumber;		// the run number
	int		startTime;		// linux start time, seconds
	int		stopTime;		// linux stop time, seconds
	int		events;			// number of events
	int		position;		// Danss Position type
	float		height;			// Danss average height
};

struct DanssPairStruct {
//		Common parameters
	long long	number[2];		// event numbers in the file
	int		unixTime;		// linux time, seconds
	float		SiPmCleanEnergy[2];	// Full Clean energy SiPm
	float		PmtCleanEnergy[2];	// Full Clean energy Pmt
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronMinLen;		// Minimum track length to create the cluster
	float		PositronEnergy;		// Energy sum of the cluster (SiPM)
	float		TotalEnergy;		// Event full energy correctd (SiPM+PMT)
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas

//		"neutron" parameters
	int		NeutronHits;		// number of hits considered as neutron capture gammas
	float		NeutronSiPmEnergy;	// Energy sum of above (SiPM)
	float		NeutronX[3];		// center of gammas position
        float           NeutronGammaEnergy[5];  // sorted list of the 5 most energetic gammas
        float           NeutronGammaDistance[5];        // distances for the gammas above to the "neutron" center
	float		NeutronRadius;		// average distance between hits and the center
//		Pair parameters
	float		gtDiff;			// time difference in us (from 125 MHz clock)
	float		Distance;		// distance between neutron and positron, cm
	float		DistanceZ;		// in Z, cm
};

struct DanssPairStruct2 {
//		Common parameters
	long long	number[2];		// event numbers in the file
	int		unixTime;		// linux time, seconds
	float		SiPmCleanEnergy[2];	// Full Clean energy SiPm
	float		PmtCleanEnergy[2];	// Full Clean energy Pmt
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronEnergy;		// Energy sum of the cluster (SiPM)
	float		MaxHitEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
//		"neutron" parameters
	int		NeutronHits;		// number of hits considered as neutron capture gammas
	float		NeutronEnergy;		// Energy sum of above (SiPM)
	float		NeutronX[3];		// center of gammas position
	float		NeutronRadius;		// average distance between hits and the center
//		Pair parameters
	float		gtDiff;			// time difference in us between positron and neutron
	float		Distance;		// distance between neutron and positron, cm
	float		DistanceZ;		// in Z, cm
//		Environment
	float		gtFromPrevious;		// time from the previous hit before positron, us
	float		gtToNext;		// time to the next hit after neutron, counted from positron, us
	int		EventsBetween;		// Events between positron and neutron
//		Veto
	float		gtFromVeto;		// time from the last Veto event
	int		VetoHits;		// hits in Veto counters
	float		VetoEnergy;		// Energy in Veto counters
	float		DanssEnergy;		// Veto Energy in Danss (Pmt + SiPm)/2
};

struct DanssPairStruct3 {
//		Common parameters
	long long	number[2];		// event numbers in the file
	int		unixTime;		// linux time, seconds
	float		SiPmCleanEnergy[2];	// Full Clean energy SiPm
	float		PmtCleanEnergy[2];	// Full Clean energy Pmt
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronEnergy;		// Energy sum of the cluster (SiPM)
	float		MaxHitEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
//		"neutron" parameters
	int		NeutronHits;		// number of hits considered as neutron capture gammas
	float		NeutronEnergy;		// Energy sum of above (SiPM)
	float		NeutronX[3];		// center of gammas position
	float		NeutronRadius;		// average distance between hits and the center
//		Pair parameters
	float		gtDiff;			// time difference in us between positron and neutron
	float		Distance;		// distance between neutron and positron, cm
	float		DistanceZ;		// in Z, cm
//		Environment
	float		gtFromPrevious;		// time from the previous hit before positron, us
	float		PreviousEnergy;		// energy of the previous event
	float		gtToNext;		// time to the next hit after neutron, counted from positron, us
	float		NextEnergy;		// energy of the next event
	int		EventsBetween;		// Events between positron and neutron
//		Veto
	float		gtFromVeto;		// time from the last Veto event
	int		VetoHits;		// hits in Veto counters
	float		VetoEnergy;		// Energy in Veto counters
	float		DanssEnergy;		// Veto Energy in Danss (Pmt + SiPm)/2
};

struct DanssPairStruct4 {
//              Common parameters
        long long       number[2];              // event numbers in the file
        int             unixTime;               // linux time, seconds
        float           SiPmCleanEnergy[2];     // Full Clean energy SiPm
        float           PmtCleanEnergy[2];      // Full Clean energy Pmt
        int             PositronFlags[2];       // positron flags for both triggers
//              "positron cluster" parameters
        int             PositronHits;           // hits in the cluster
        float           PositronEnergy;         // Energy sum of the cluster (SiPM)
        float           TotalEnergy;            // Energy of the maximum hit (SiPM)
        float           PositronX[3];           // cluster position
        int             AnnihilationGammas;     // number of possible annihilation gammas
        float           AnnihilationEnergy;     // Energy in annihilation gammas
//              "neutron" parameters
        int             NeutronHits;            // number of hits considered as neutron capture gammas
        float           NeutronEnergy;          // Energy sum of above (SiPM)
        float           NeutronX[3];            // center of gammas position
        float           NeutronRadius;          // average distance between hits and the center
//              Pair parameters
        float           gtDiff;                 // time difference in us between positron and neutron
        float           Distance;               // distance between neutron and positron, cm
        float           DistanceZ;              // in Z, cm
//              Environment
        float           gtFromPrevious;         // time from the previous hit before positron, us
        float           PreviousEnergy;         // energy of the previous event
        float           gtToNext;               // time to the next hit after neutron, counted from positron, us
        float           NextEnergy;             // energy of the next event
        int             EventsBetween;          // Events between positron and neutron
//              Veto
        float           gtFromVeto;             // time from the last Veto event
        int             VetoHits;               // hits in Veto counters
        float           VetoEnergy;             // Energy in Veto counters
        float           DanssEnergy;            // Veto Energy in Danss (Pmt + SiPm)/2
        float           gtFromShower;           // time from large energy shower in DANSS
        float           ShowerEnergy;           // shower event energy in DANSS (Pmt + SiPm)/2
};

struct DanssPairStruct5 {
//		Common parameters
	long long	number[2];		// event numbers in the file
	int		unixTime;		// linux time, seconds
	float		SiPmCleanEnergy[2];	// Full Clean energy SiPm
	float		PmtCleanEnergy[2];	// Full Clean energy Pmt
	int		PositronFlags[2];	// positron flags for both triggers
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronMinLen;		// mimimum track length for the cluster
	float		PositronEnergy;		// Energy sum of the cluster (SiPM)
	float		TotalEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
//		"neutron" parameters
	int		NeutronHits;		// number of hits considered as neutron capture gammas
	float		NeutronEnergy;		// Energy sum of above (SiPM)
	float		NeutronX[3];		// center of gammas position
//		Pair parameters
	float		gtDiff;			// time difference in us between positron and neutron
	float		Distance;		// distance between neutron and positron, cm
	float		DistanceZ;		// in Z, cm
//		Environment
	float		gtFromPrevious;		// time from the previous hit before positron, us
	float		PreviousEnergy;		// energy of the previous event
	float		gtToNext;		// time to the next hit after neutron, counted from positron, us
	float		NextEnergy;		// energy of the next event
	int		EventsBetween;		// Events between positron and neutron
//		Veto
	float		gtFromVeto;		// time from the last Veto event
	int		VetoHits;		// hits in Veto counters
	float		VetoEnergy;		// Energy in Veto counters
	float		DanssEnergy;		// Veto Energy in Danss (Pmt + SiPm)/2
	float		gtFromShower;		// time from large energy shower in DANSS
	float		ShowerEnergy;		// shower event energy in DANSS (Pmt + SiPm)/2
//		Hits
	int		NPHits;			// Number of hits in "positron event"
	int		NNHits;			// Number of hits in "neutron event"
};

struct DanssPairStruct6 {
//		Common parameters
	long long	number[2];		// event numbers in the file
	int		unixTime;		// linux time, seconds
	float		SiPmCleanEnergy[2];	// Full Clean energy SiPm
	float		PmtCleanEnergy[2];	// Full Clean energy Pmt
	int		PositronFlags[2];	// positron flags for both triggers
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronMinLen;		// mimimum track length for the cluster
	float		PositronEnergy;		// Energy sum of the cluster (SiPM)
	float		TotalEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronSiPmEnergy;	// SiPM energy in the cluster, corrected
	float		PositronPmtEnergy;	// PMT energy in the cluster, corrected
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
//		"neutron" parameters
	int		NeutronHits;		// number of hits considered as neutron capture gammas
	float		NeutronEnergy;		// Energy sum of above (SiPM)
	float		NeutronX[3];		// center of gammas position
//		Pair parameters
	float		gtDiff;			// time difference in us between positron and neutron
	float		Distance;		// distance between neutron and positron, cm
	float		DistanceZ;		// in Z, cm
//		Environment
	float		gtFromPrevious;		// time from the previous hit before positron, us
	float		PreviousEnergy;		// energy of the previous event
	float		gtToNext;		// time to the next hit after neutron, counted from positron, us
	float		NextEnergy;		// energy of the next event
	int		EventsBetween;		// Events between positron and neutron
//		Veto
	float		gtFromVeto;		// time from the last Veto event
	int		VetoHits;		// hits in Veto counters
	float		VetoEnergy;		// Energy in Veto counters
	float		DanssEnergy;		// Veto Energy in Danss (Pmt + SiPm)/2
	float		gtFromShower;		// time from large energy shower in DANSS
	float		ShowerEnergy;		// shower event energy in DANSS (Pmt + SiPm)/2
//		Hits
	int		NPHits;			// Number of hits in "positron event"
	int		NNHits;			// Number of hits in "neutron event"
};

struct DanssPairStruct7 {
//		Common parameters
	long long	number[2];		// event numbers in the file
	long long	globalTime[2];		// event times
	int		unixTime;		// linux time, seconds
//	int		runNumber;		// the run number
	float		SiPmCleanEnergy[2];	// Full Clean energy SiPm
	float		PmtCleanEnergy[2];	// Full Clean energy Pmt
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronEnergy;		// Energy sum of the cluster (SiPM)
	float		TotalEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronSiPmEnergy;	// SiPM energy in the cluster, corrected
	float		PositronPmtEnergy;	// PMT energy in the cluster, corrected
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
	float		MinPositron2GammaZ;	// Z-distance to the closest gamma
//		"neutron" parameters
	int		NeutronHits;		// number of hits considered as neutron capture gammas
	float		NeutronEnergy;		// Energy sum of above (SiPM)
	float		NeutronX[3];		// center of gammas position
//		Pair parameters
	float		gtDiff;			// time difference in us between positron and neutron
	float		Distance;		// distance between neutron and positron, cm
	float		DistanceZ;		// in Z, cm
//		Environment
	float		gtFromPrevious;		// time from the previous hit before positron, us
	float		PreviousEnergy;		// energy of the previous event
	float		gtToNext;		// time to the next hit after neutron, counted from positron, us
	float		NextEnergy;		// energy of the next event
	int		EventsBetween;		// Events between positron and neutron
//		Veto
	float		gtFromVeto;		// time from the last Veto event
	int		VetoHits;		// hits in Veto counters
	float		VetoEnergy;		// Energy in Veto counters
	float		DanssEnergy;		// Veto Energy in Danss (Pmt + SiPm)/2
	float		gtFromShower;		// time from large energy shower in DANSS
	float		ShowerEnergy;		// shower event energy in DANSS (Pmt + SiPm)/2
//		Hits
	int		NPHits;			// Number of hits in "positron event"
	int		NNHits;			// Number of hits in "neutron event"
};

struct DanssPairStruct8 {
//		Common parameters
	long long	number[2];		// event numbers in the file
	long long	globalTime[2];		// event times
	int		unixTime;		// linux time, seconds
	float		SiPmCleanEnergy[2];	// Full Clean energy SiPm
	float		PmtCleanEnergy[2];	// Full Clean energy Pmt
//		"positron cluster" parameters
	int		PositronHits;		// hits in the cluster
	float		PositronEnergy;		// Energy sum of the cluster (SiPM)
	float		TotalEnergy;		// Energy of the maximum hit (SiPM)
	float		PositronSiPmEnergy;	// SiPM energy in the cluster, corrected
	float		PositronPmtEnergy;	// PMT energy in the cluster, corrected
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
	float		MinPositron2GammaZ;	// Z-distance to the closest gamma
//		"neutron" parameters
	int		NeutronHits;		// number of hits considered as neutron capture gammas
	float		NeutronEnergy;		// Energy sum of above (SiPM)
	float		NeutronX[3];		// center of gammas position
//		Pair parameters
	float		gtDiff;			// time difference in us between positron and neutron
	float		Distance;		// distance between neutron and positron, cm
	float		DistanceZ;		// in Z, cm
//		Environment
	float		gtFromPreviousH;		// time from the previous Hard event before positron, us
	float		PreviousEnergyH;		// energy of the previous Hard event
	float		gtFromPreviousS;		// time from the previous hit before positron, us
	float		PreviousEnergyS;		// energy of the previous event
	float		gtToNext;		// time to the next hit after neutron, counted from positron, us
	float		NextEnergy;		// energy of the next event
	int		EventsBetween;		// Events between positron and neutron
//		Veto
	float		gtFromVeto;		// time from the last Veto event
	int		VetoHits;		// hits in Veto counters
	float		VetoEnergy;		// Energy in Veto counters
	float		DanssEnergy;		// Veto Energy in Danss (Pmt + SiPm)/2
	float		gtFromShower;		// time from large energy shower in DANSS
	float		ShowerEnergy;		// shower event energy in DANSS (Pmt + SiPm)/2
//		Hits
	int		NPHits;			// Number of hits in "positron event"
	int		NNHits;			// Number of hits in "neutron event"
};

struct DanssMuonStruct {
//		Common parameters
	long long	number[2];		// event numbers in the file
	long long	globalTime[2];		// event times
	int		unixTime;		// linux time, seconds
//	int		runNumber;		// the run number
//		Hit summs
	int		SiPmHits[2];		// SiPm clean hits
	float		SiPmEnergy[2];		// Full Clean energy SiPm
	int		PmtHits[2];		// Pmt clean hits
	float		PmtEnergy[2];		// Full Clean energy Pmt
	int		VetoHits[2];		// hits in Veto counters
	float		VetoEnergy[2];		// Energy in Veto counters
//		"positron cluster" parameters for delayed event
	float		TotalEnergy;		// Total energy, corrected for custer position
	int		ClusterHits;		// hits in the cluster
	float		ClusterEnergy;		// Energy sum of the cluster (SiPM)
	float		ClusterSiPmEnergy;	// SiPM energy in the cluster, corrected
	float		ClusterPmtEnergy;	// PMT energy in the cluster, corrected
	float		ClusterX[3];		// cluster position
	int		OffClusterHits;		// number of off cluster hits
	float		OffClusterEnergy;	// off cluster energy
//		"muon" parameters - prompt event
	float		MuonEnergy;		// Energy sum of prompt event
//		Pair parameters
	float		gtDiff;			// time difference in us between positron and neutron
//		Hits
	int		NPHits;			// Number of hits in "muon event"
	int		NDHits;			// Number of hits in "decay event"
};

//		248Cm analysis
struct DanssCmStruct {
	long long	number[10];		// event numbers in the file
	int		unixTime;		// linux time, seconds
	int		N;			// number of neutrons + 1
	float		SiPmCleanEnergy[10];	// Full Clean energy SiPm
	float		PmtCleanEnergy[10];	// Full Clean energy Pmt
//		"neutron" parameters
	int		Hits[10];
//	int		NeutronHits[10];	// number of hits considered as neutron capture gammas
	float		NeutronEnergy[10];	// Energy sum of above (SiPM + PMT)
	float		NeutronX[10][3];	// center of gammas position
	float		PositronX[10][3];	// center of maximum hit cluster
//	float		NeutronGammaEnergy[10][5];	// sorted list of the 5 most energetic gammas
//	float		NeutronGammaDistance[10][5];	// distances for the gammas above to the "neutron" center
	float		PositronEnergy[10];	// maximum hit cluster energy
	float		NeutronRadius[10];	// average distance between hits and the center
//		Pair parameters
	float		gtDiff[10];		// time difference in us (from 125 MHz clock)
	float		Distance[10];		// distance between neutron and positron, cm
	float		DistanceZ[10];		// in Z, cm
};

//		MC truth
struct DanssMcStruct {
	float		Energy;
	float		X[3];
	float		DriftTime;
};

//		Raw hits information
struct RawHitInfoStruct {
	unsigned short PmtCnt;
	unsigned short VetoCnt;
	unsigned short SiPmCnt;
};


#endif /* EVTBUILDER_H */

