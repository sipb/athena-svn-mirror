/* VLDB structures for VLDB version 1. */
struct vital_vlheader_1 {
    int32    vldbversion;	
    int32    headersize;		
    int32    freePtr;		
    int32    eofPtr;		
    int32    allocs;		
    int32    frees;		
    int32    MaxVolumeId;	
    int32    totalEntries[3];
};

typedef  struct vital_vlheader_1 vital_vlheader1;

struct vlheader_1 {
    vital_vlheader1  vital_header; 
    int32    IpMappedAddr[31];  
    int32    VolnameHash[8191];	  
    int32    VolidHash[3][8191];
};
struct vlentry_1 {
    int32    volumeId[3]; 
    int32    flags;		
    int32    LockAfsId;		
    int32    LockTimestamp;	
    int32    cloneId;		
    int32    spares0;
    int32    nextIdHash[3];
    int32    nextNameHash;		
    int32    spares1[2];			
    char    name[65];
    char    spares3;			
    unsigned char  serverNumber[8];	
    unsigned char  serverPartition[8];
    unsigned char  serverFlags[8];	
    char    spares4;			
    char    spares2[1];			
};

/* VLDB structures for VLDB version 2. */
typedef  struct vital_vlheader_1 vital_vlheader2;

struct vlheader_2 {
    vital_vlheader2  vital_header; 
    int32  IpMappedAddr[255];   /* == 0..254 */
    int32    VolnameHash[8191];	  
    int32    VolidHash[3][8191];
    int32    SIT;
};

struct vlentry_2 {
    int32  volumeId[3]; 
    int32    flags;		
    int32    LockAfsId;		
    int32    LockTimestamp;	
    int32    cloneId;		
    int32    spares0;
    int32    nextIdHash[3];
    int32    nextNameHash;		
    int32    spares1[2];			
    char    name[65];
    char    spares3;			
    unsigned char  serverNumber[8];	
    unsigned char  serverPartition[8];
    unsigned char  serverFlags[8];	
    char    spares4;			
    char    spares2[1];			
};

typedef  struct vital_vlheader_1 vital_vlheader3;

struct vlheader_3 {
    vital_vlheader3  vital_header; 
    int32  IpMappedAddr[255];   /* == 0..254 */
    int32    VolnameHash[8191];	  
    int32    VolidHash[3][8191];
    int32    SIT;
};


struct vlentry_3 {
    int32    volumeId[3]; 
    int32    flags;		
    int32    LockAfsId;		
    int32    LockTimestamp;	
    int32    cloneId;		
    int32    nextIdHash[3];
    int32    nextNameHash;		
    char    name[65];
#define MAXSERVERS	13
    unsigned char  serverNumber[MAXSERVERS];	
    unsigned char  serverPartition[MAXSERVERS];
    unsigned char  serverFlags[MAXSERVERS];	

#ifdef	obsolete_vldb_fields
    int32    spares0;			/* AssociatedChain */
    int32    spares1[0];			
    int32    spares1[1];			
    char    spares3;			/* volumeType */
    char    spares4;			/* RefCount */
    char    spares2[1];			
#endif
};



















