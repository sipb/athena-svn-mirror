/* driver.h - Useful declarations for driver.c. By Pete Resnick */

#define UTBase		(* (DCtlHandle **) UTableBase)
#define UECnt		(* (short *) UnitNtryCnt)

typedef enum {
	open = 0x01,		/* Open the driver after installed */
	thinkDATA = 0x02,	/* Driver uses THINK C global data DATA rsrc */
	thinkMultSeg = 0x04	/* Driver uses THINK C multi-segment DCOD rsrc's */
} drvrInstFlagBits;

//typedef enum {
//	drvrOpened	= 0x20,
//	drvrRAMBased= 0x40,
//	dActive	= 0x80
//} dCtlFlagsBits;

typedef struct {
	short drvrFlags;
	short drvrDelay;
	short drvrEMask;
	short drvrMenu;
	short drvrOpen;
	short drvrPrime;
	short drvrCtl;
	short drvrStatus;
	short drvrClose;
	Str255 drvrName;
	unsigned char drvrRoutines[];
} DriverStruct, *DriverPtr, **DriverHandle;

/* These two documented in Technical Note 184 */
OSErr DrvrInstall(Handle drvrHandle, long refNum);
OSErr DrvrRemove(long refNum);

/* These three routines are the ones you want to call */
OSErr InstallRAMDriver(Str255 drvrName, short *refNum, Byte drvrInstFlags);
OSErr RemoveRAMDriver(short refNum, Boolean dcodRemove);
short GetDrvrRefNum(Str255 drvrName);
pascal void ShowINIT( short iconID, short moveX );

/* These are used internally and might be useful in unusual circumstances */
OSErr GrowUTable(short newEntries);
OSErr DriverAvail(short *unitNum);
void ReleaseDrvrSegments(Handle *dcodHList, short rsrcID, Boolean detach);

/* 
	new definitions for the standalone assembler changes 
	these routines are defined in the new file driverInstall.a 
*/
	void enableInterrupts(short savedStatus);
	short disableInterrupts(void);
	
/* imported from Think.h, for driverInstall.c */

#define dRAMBased		0x0040 
#define dOpened			0x0020

#define UTableBase (*(DCtlHandle**) 0x11C)
#define UnitNtryCnt (*(short*) 0x1D2)

#define ResErr (*(short *) 0xA60)