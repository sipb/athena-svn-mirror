/* driver.c */

/*
 *	The following code is to install and remove RAM drivers in the system
 *	heap. Written by Pete Resnick with the help of Joe Holt, Phil Shapiro,
 *	Tom Johnson, Michael A. Libes, Matthias Urlichs, John Norstad, and
 *	Charles Martin.
 *
 *	Change Log
 *	----------
 *	Date:		Change:										Who:
 *	-----		-------										----
 *	22 Jan 91	Changed "if (UTableSize < UnitNtryCnt) {"	pr
 *				to "if (UTableSize > UnitNtryCnt) {" in
 *				DriverAvail.
 *
 *	28 Jan 91	Make sure resource map is updated to		pr
 *				change back the resource ID's in
 *				InstallRAMDriver by adding code to
 *				SetResFileAttrs and UpdateResFile.
 */

#include <Files.h>
#include <Types.h>
#include <Events.h>
#include <Memory.h>
#include <Resources.h>
#include <Errors.h>
#include <Devices.h>
#include <SysEqu.h>

#include <StdArg.h>
#include "driverInstall.h"


#define LOW_UNIT		48		/* First Unit Table Entry to use	*/
#define NEW_UNIT		64		/* Size of a "normal" Unit Table	*/
#define MAX_UNIT		128		/* Maximum size of a Unit Table		*/
#define UP_UNIT			4		/* Size to bounce up Unit Table		*/
#define SCCLockOut		0x2600	/* Disable interrupts flag			*/
#define OKICONID		128
#define NOGOICONID		130

#define debugPrint(a,b,c) /* nothing */
#define debugPrintH(a,b) /* nothing */
#define debugPrintS(a,b) /* nothing */
#define debugPrintL(a,b) /* nothing */


pascal void main()
{
	short		refNum;
	EventRecord evtRec;
	OSErr err;
	// char theChar;
	THz saveZone,sysHeap;
	// Ptr aPtr;
	unsigned short ref;
	err = 0;
	
	saveZone = GetZone();
	sysHeap = SystemZone();
	SetZone(sysHeap);

	if( GetNextEvent( 0, &evtRec ) );	/* get a null event to check the modifiers */
	
	if( ( ( evtRec.modifiers  ^ btnState) & ( optionKey ) ) == 0 
		&&  (err = InstallRAMDriver( "\p.kerberos", &refNum, open | thinkDATA | thinkMultSeg ))
		 == noErr ) {
		ShowINIT( OKICONID, -1 );			/* display OK icon */
	}
	else {
		DebugStr("\pFailed InstallRAMDriver");
		ShowINIT( NOGOICONID, -1 );			/* display disabled icon */
//		debugPrintS("Kerberos Install Error: ",&err);

	}
	
	SetZone(saveZone);
// 	ref = (unsigned short) GetDrvrRefNum("\p.kerberos");
// 	debugPrintS("Driver Refnum:",&ref);
}

#if 0
debugPrint(char *str,char *s,long i)
{
	char *s2;
	long k;
	long len;
	len = strlen(str);
	if (len) {
		s2=&(str[len-1]);
		for (k=0;k<len;k++,i++) *--s = *s2--;
	}
	
	*--s = (char) i;
//	DebugStr(s);
}
 

debugPrintS(char *str, void *vn)
{
	char buf[512];
	char *s;
	unsigned long n = 0;
	long i = 0;
	s = &buf[512];
	
	if (vn) { 
		n = *((unsigned short *) vn);
		for (; n; n /= 10, i++) *--s = n % 10 + '0';
	}

	debugPrint(str,s,i);
}

debugPrintL(char *str, void *vn)
{
	char buf[512];
	char *s;
	unsigned long n = 0;
	long i = 0;
	s = &buf[512];
	
	if (vn) { 
		n = *((unsigned long *) vn);
		for (; n; n /= 10, i++) *--s = n % 10 + '0';
	}
	
	debugPrint(str,s,i);
}


debugPrintH(char *str, void *vn)
{
	char buf[512];
	char *s;
	unsigned long n = 0;
	long i;
	s = &buf[512];
	
		n = *((unsigned long *) vn);
		if (n) for (i=0; n; n /= 10, i++) *--s = n % 10 + '0';
		else { *--s = '0'; i++; }
		debugPrint(str,s,i);
		
		s = &buf[512];
		n = GetHandleSize(*((Handle *) vn));
		if (n) for (i=0; n; n /= 10, i++) *--s = n % 10 + '0';
		else { *--s = '0'; i++; }
		debugPrint("   handle size:",s,i);

		s = &buf[512];
		n = **((unsigned long **) vn);
		n = (long) StripAddress((void *) n);
		if (n) for (i=0; n; n /= 10, i++) *--s = n % 10 + '0';
		else { *--s = '0'; i++; }
		debugPrint("   ptr:",s,i);
}
#endif

testControl()
{
	short dummyCode = 1;
	ParamBlockRec aPBR;
	short ref;
	OSErr err;
	
	ref = GetDrvrRefNum("\p.kerberos");
	if (!ref) {
		DebugStr("\pCouldn't find driver in table");
		return;
	}
	aPBR.cntrlParam.ioCompletion = nil;
	aPBR.cntrlParam.ioVRefNum = 0;
	aPBR.cntrlParam.ioCRefNum = ref;
	aPBR.cntrlParam.csCode = dummyCode;

	err = PBControl( &aPBR, false );
	debugPrintS("Controltest rc: ",&err);		
}


/*
 *	InstallRAMDriver will install the named driver into the system heap
 *	locked and return the driver reference number in refNum. Make sure
 *	that the DRVR resources are numbered between 0 and 47 and that the
 *	resources owned by the driver are also numbered appropriately. Using
 *	resource ID's of 48 and higher may cause conflicts if you are using
 *	the THINK C global data DATA resource or the THINK C multi-segment
 *	DCOD resource, which get temporarily renumbered by this routine.
 *	One major kludge: the ioMisc field of the ioParam block is not used
 *	for non-file operations, so it is used to store a pointer to a block
 *	containing the addresses of the DCOD resources (if they are being
 *	used) so that they can be removed by RemoveRAMDriver. If Apple ever
 *	uses that field, that mechanism will be gone. See driver.h for the
 *	drvrInstFlags.
 */
 
OSErr InstallRAMDriver(Str255 drvrName, short *refNum, Byte drvrInstFlags)
{
	OSErr errCode;
	/* FIXED jcm - Think sign extends the right shift unless unsigned */
	unsigned short rsrcID, dcodRsrcID;
	short index, unitNum, dcodSegments, dcodIndex;
	ResType rsrcType;
	Str255 rsrcName;
	Handle dataHandle, drvrHandle, *dcodHList = nil, tempHandle;
	IOParam openBlock;
	register DriverPtr drvrPtr;
	register DCtlPtr ctlEntryPtr;
/*
	DBGlines, 3:
 	long debugData;		//debug
 	Handle saveData;	//debug
 	short tempAttr;		//debug
*/

	/* Get the unit number for the driver */
	errCode = DriverAvail(&unitNum);
	/* 
	DebugStr("\pInstallRAMDriver - errCode = DriverAvail(&unitNum)"); 
	*/
	if(errCode != noErr)
		return(errCode);

	/* The driver must load into the system heap locked. */
	SetResLoad(false);
	drvrHandle = Get1NamedResource('DRVR', drvrName);
	/*
	DebugStr("\pdrvrHandle = Get1NamedResource('DRVR'...)");
	*/
	SetResLoad(true);
	if(drvrHandle == nil)
		return(dInstErr);
	SetResAttrs( drvrHandle, GetResAttrs( drvrHandle ) | resSysHeap | resLocked );
	if(ResErr != noErr) {
		errCode = ResErr;
		ReleaseResource(drvrHandle);
		return(errCode);
	}
	
	/* Save the resource info for later use */
	/* FIXED jcm - Think needs a str255 for the last argument, not a ptr to one */
	GetResInfo(drvrHandle, (short*) &rsrcID, &rsrcType, rsrcName);
	if(ResErr != noErr) {
		errCode = ResErr;
		ReleaseResource(drvrHandle);
		return(errCode);
	}
	
	/* Now load it and detach it */
	LoadResource(drvrHandle);
	if(ResErr != noErr) {
		errCode = ResErr;
		ReleaseResource(drvrHandle);
		return(errCode);
	}
	DetachResource(drvrHandle);

	if(drvrInstFlags & thinkDATA) {
		/*
		 *	Make sure the DATA resource will load into the system heap,
		 *	locked, and is owned by our driver, where the ID is the
		 *	unitNum in bits 5 through 10 (or 11 if needed), zeros in
		 *	bits 11 (if there is room) 12 and 13 (which means DRVR), zeros
		 *	in bits 0 through 4 (since there is only 1 DATA resource), and
		 *	ones in bits 14 and 15. If any errors occur after the resource
		 *	ID is changed, it must be changed back!
		 */
		SetResLoad(false);
		dataHandle = Get1Resource('DATA', (rsrcID << 5) | 0xC000);
		/*
		DebugStr("\pdataHandle = Get1Resource('DATA'...");
		*/
		SetResLoad(true);
		if(dataHandle == nil) {
			errCode = ResErr;
			DisposHandle(drvrHandle);
			DebugStr("\pFailed loading DATA, about to return");
			return(errCode != noErr ? errCode : resNotFound);
		}
		SetResAttrs(dataHandle, GetResAttrs (dataHandle) | resSysHeap | resLocked );
		if(ResErr != noErr) {
			DebugStr("\pError SetResAttrs on DATA");
			errCode = ResErr;
			DisposHandle(drvrHandle);
			return(errCode);
		}
		SetResInfo(dataHandle, (unitNum << 5) | 0xC000, 0);
		if(ResErr != noErr) {
			DebugStr("\pError SetResInfo dataHandle");
			errCode = ResErr;
			DisposHandle(drvrHandle);
			return(errCode);
		}
	/* 
 	DBGLines, 1:
		saveData = dataHandle;	//debug
	*/
	
	}

	if(drvrInstFlags & thinkMultSeg) {
		/*
		 *	Make sure the DCOD resources will load into the system heap,
		 *	locked, and are owned by our driver, where the ID is the
		 *	unitNum in bits 5 through 10 (or 11 if needed), zeros in
		 *	bits 11 (if there is room) 12 and 13 (which means DRVR),
		 *	the same ID in bits 0 through 4, and ones in bits 14 and 15.
		 *	Keep a block containing the handles to those segments so that
		 *	they can be thrown away later if needed. If any errors
		 *	occur after the resource ID's are changed, they are changed
		 *	back in ReleaseDrvrSegments. 
		 */
		dcodSegments = 0;
		errCode = noErr;
		SetResLoad(false);
		
		/* Count how many segments there are */
		for(index = 1;
		    (index <= Count1Resources('DCOD')) && (errCode == noErr);
		    ++index) {
			tempHandle = Get1IndResource('DCOD', index);
			/*
			DebugStr("\ptempHandle = Get1IndResource('DCOD'...");
			*/
			if(tempHandle == nil)
				errCode = (ResErr != noErr ? ResErr : resNotFound);
			/* Think wants a str255 not a pointer to one for the last argument */
			GetResInfo(tempHandle, (short*) &dcodRsrcID, &rsrcType, rsrcName);
			ReleaseResource(tempHandle);
			if((dcodRsrcID & ~0xF01F) >> 5 == rsrcID) {
				++dcodSegments;
				/*
				DebugStr("\pDCOD belongs to theis rsrcID");
				*/
			}
		}
		SetResLoad(true);

		if(errCode != noErr) {
			DebugStr("\pError locating DCOD");
			if(drvrInstFlags & thinkDATA) {
				SetResInfo(dataHandle, (rsrcID << 5) | 0xC000, 0);
				SetResFileAttrs(CurResFile(),
				                GetResFileAttrs(index) | mapChanged);
				UpdateResFile(CurResFile());
				ReleaseResource(dataHandle);
			}
			DisposHandle(drvrHandle);
			return(errCode);
		}

		/* Get a block of memory to hold the handles */
		dcodHList = (Handle *)NewPtrSysClear(sizeof(Handle) * dcodSegments);
		/*
		DebugStr("\pdcodHList = (Handle*)NewPtrSysClear...");
		*/
		if(dcodHList == nil) {
			if(drvrInstFlags & thinkDATA) {
				SetResInfo(dataHandle, (rsrcID << 5) | 0xC000, 0);
				SetResFileAttrs(CurResFile(),
				                GetResFileAttrs(index) | mapChanged);
				UpdateResFile(CurResFile());
				ReleaseResource(dataHandle);
			}
			DisposHandle(drvrHandle);
			return(memFullErr);
		}

		/* Get the resources and change the attributes and ID's */
		dcodIndex = 0;
		SetResLoad(false);
		for(index = 1;
		    (index <= Count1Resources('DCOD')) && (errCode == noErr);
		    ++index) {
			tempHandle = Get1IndResource('DCOD', index);
			if(tempHandle == nil) {
				errCode = (ResErr != noErr ? ResErr : resNotFound);
			} else {
			/* Think wants a Str255 as the last argument not a pointer to one */
				GetResInfo(tempHandle, (short*) &dcodRsrcID, &rsrcType, rsrcName);

			/*
 				debugPrintH("DCOD Handle: ",&tempHandle);
			*/
				
				if((dcodRsrcID & ~0xF01F) >> 5 == rsrcID) {
					dcodHList[dcodIndex] = tempHandle;
					/*
					DebugStr("\pdcodHList[dcodIndex]= tempHandle");
					*/
					SetResAttrs( dcodHList[ dcodIndex ],
								 GetResAttrs( dcodHList[ dcodIndex ] ) | resSysHeap | resLocked );
					if(ResErr != noErr) {
						errCode = ResErr;
						DebugStr("\pFailed SetResAttrs dcodH");
					} else {
						SetResInfo(dcodHList[dcodIndex],
									(dcodRsrcID & 0xF01F) + (unitNum << 5),
									0);
						errCode = ResErr;
					}

				/*
 					debugData = GetResAttrs(dcodHList[dcodIndex]);
 					debugPrintL("Set rez info: ",&debugData);
 					debugData = (long) tempHandle;
				*/
					++dcodIndex;
				}
				
/*
 				debugPrintH("DCOD Handle after: ",&tempHandle);
*/
			}
		}
		SetResLoad(true);
		
		if(errCode != noErr) {
			DebugStr("\pError changing DCOD resources");
			if(drvrInstFlags & thinkDATA) {
				SetResInfo(dataHandle, (rsrcID << 5) | 0xC000, 0);
				ReleaseResource(dataHandle);
			}
			ReleaseDrvrSegments(dcodHList, rsrcID, false);
			DisposHandle(drvrHandle);
			return(dInstErr);
		}
	}
		    
	/* Install with the refNum. The refNum is the -(unitNum + 1) */
	errCode = DrvrInstall(drvrHandle, ~unitNum);
	if(errCode != noErr) {
		DebugStr("\pError returned from DrvrInstall. About to free DATA and DCOD");
		if(drvrInstFlags & thinkDATA) {
			SetResInfo(dataHandle, (rsrcID << 5) | 0xC000, 0);
			SetResFileAttrs(CurResFile(),
			                GetResFileAttrs(index) | mapChanged);
			UpdateResFile(CurResFile());
			ReleaseResource(dataHandle);
		}
		if(drvrInstFlags & thinkMultSeg)
			ReleaseDrvrSegments(dcodHList, rsrcID, false);
		DisposHandle(drvrHandle);
		return(dInstErr);
	}
	
	/* Move the important information to the driver entry */
	ctlEntryPtr = *(UTableBase[unitNum]);
	drvrPtr = *(DriverHandle)drvrHandle;
	ctlEntryPtr->dCtlDriver = (Ptr)drvrHandle;
	ctlEntryPtr->dCtlFlags = drvrPtr->drvrFlags;
	ctlEntryPtr->dCtlDelay = drvrPtr->drvrDelay;
	ctlEntryPtr->dCtlEMask = drvrPtr->drvrEMask;
	ctlEntryPtr->dCtlMenu = drvrPtr->drvrMenu;
	ctlEntryPtr->dCtlFlags |= dRAMBased;
	/*
	DebugStr("\pdctlEntry updated with driver header info");
	*/
	
	/* Hang onto the refNum just in case open changes it. */
	index = CurResFile();
	
	/*
	 *	The open routine better load all the DATA and DCOD resources. If
	 *	the driver is going to want to be closed, it should store what
	 *	is passed to it in ioMisc and pass it back when close is called.
	 */

/*
 	debugPrintS("Flags before open: ",&(ctlEntryPtr->dCtlFlags));
 	debugPrintH("Driver Handle: ",&drvrHandle);
*/
	if(drvrInstFlags & open) {
		openBlock.ioCompletion = nil;
		openBlock.ioNamePtr = drvrName;
		openBlock.ioPermssn = fsCurPerm;
		openBlock.ioMisc = (Ptr)dcodHList;
		/*
		DebugStr("\pCalling PBOpen");
		*/
		errCode = PBOpen( (ParmBlkPtr) &openBlock, false);
	}

/*
 	debugPrintH("DCOD Handle after open: ",&debugData);
 	tempAttr = GetResAttrs((Handle) debugData);	// debug
 	debugPrintS("DCOD Attrs: ",&tempAttr);
 	debugPrintS("Flags after open: ",&(ctlEntryPtr->dCtlFlags));
*/
 	testControl();	//debug
	
	/* Change CurResFile back to our original one. */
	UseResFile(index);
	
	if(drvrInstFlags & thinkDATA) {
		/*
		 *	If the open was successful, the dataHandle will be detached.
		 *	If the open failed, dataHandle may or may not be an attached
		 *	resource, but probably isn't attached. Therefore, the resource
		 *	must be retrieved to change the resource ID back, just in case
		 *	the open routine changed the file's resource map. After that,
		 *	release it. Errors here will be horrific because the file will
		 *	basically be corrupted if the ID can't be changed back, so
		 *	don't bother checking. At this point in the game, errors
		 *	shouldn't occur anyway.
		 */
		SetResLoad(false);
		dataHandle = Get1Resource('DATA', (unitNum << 5) | 0xC000);
		SetResLoad(true);
		if(dataHandle != nil) {
			/* DebugStr("\pAfter open DATA handle not nil"); */
			SetResInfo(dataHandle, (rsrcID << 5) | 0xC000, 0);
			SetResFileAttrs(CurResFile(),
			                GetResFileAttrs(index) | mapChanged);
			UpdateResFile(CurResFile());
			ReleaseResource(dataHandle);
		}
	}
	
	/* If an error occurred during the open */
	if(errCode != noErr) {
		DebugStr("\pError during open - removing RAM driver");
		RemoveRAMDriver(~unitNum, false);
		if(drvrInstFlags & thinkMultSeg)
			ReleaseDrvrSegments(dcodHList, rsrcID, false);
	} 
	else {
	if(drvrInstFlags & thinkMultSeg) 
		ReleaseDrvrSegments(dcodHList, rsrcID, true);
	*refNum = ~unitNum;
	}


/*
 	DebugStr("\pEnd of rtn");
 	debugPrintH("DCOD Handle: ",&debugData);
 	debugPrintS("Driver Unit Number: ",&unitNum);
 	debugPrintH("Driver Handle: ",&drvrHandle);
 	debugPrintH("Data Handle: ",&saveData);
 	debugPrintH("Code (temp) Handle: ",&tempHandle);
*/
	/*
 	DebugStr("\pDone installing driver!");
 	*/
	return(errCode);
}

/*
 *	Removes the driver installed in the system heap by InstallRAMDriver.
 *	See the warning on InstallRAMDriver about the ioMisc field.
 */

OSErr RemoveRAMDriver(short refNum, Boolean dcodRemove)
{
	OSErr errCode = noErr, dcodHandles, index;
	Handle dataHandle = nil, *dcodHList = nil, drvrHandle;
	IOParam closeBlock;
	
	/* If the driver is open, close it */
	if((**UTableBase[~refNum]).dCtlFlags & dOpened) {
		closeBlock.ioCompletion = nil;
		closeBlock.ioRefNum = refNum;
		errCode = PBClose((ParmBlkPtr) &closeBlock, false);
		if(errCode != noErr)
			return(errCode);
		dcodHList = (Handle*) closeBlock.ioMisc;
		dataHandle = (Handle)(**UTableBase[~refNum]).dCtlStorage;
	}
	
	/*
	 *	Since the driver has been detached, it will have to be disposed of
	 *	later since DrvrRemove just does a ReleaseResource on the handle.
	 */
	drvrHandle = (Handle)(**UTableBase[~refNum]).dCtlDriver;
	errCode = DrvrRemove(refNum);
	if(errCode != noErr)
		return(errCode);
	if(drvrHandle != nil) {
		DisposHandle(drvrHandle);
		if(dcodRemove && (dcodHList != nil)) {

			/*	The driver has passed back the handles to its segments so
			 *	they can be disposed of.
			 */
			dcodHandles = GetPtrSize((Ptr) &dcodHList) / sizeof(Handle);
			for(index = 0; index < dcodHandles; ++index)
				DisposHandle(dcodHList[index]);
			DisposPtr((Ptr) dcodHList);
		}
	}
	return(noErr);
}

short GetDrvrRefNum(Str255 drvrName)
{
	short unitNum;
	DCtlHandle curDCtlEntry;
	DriverPtr curDrvrPtr;
 	long temp;		// debug
	Handle thandle;
	
	/* Walk through the Unit Table */
	for(unitNum = 0; unitNum < UnitNtryCnt; ++unitNum) {
		curDCtlEntry = GetDCtlEntry(~unitNum);
		if(curDCtlEntry != nil) {
		/*
		DebugStr("\pcurDCtlEntry");
		*/
		
			/* If this is a RAM driver, it's a handle. ROM is a pointer */
/*
 			temp = (long) ((**curDCtlEntry).dCtlFlags);	//debug
 			debugPrintL("Flags: ",&temp);
 			temp = (long) ((**curDCtlEntry).dCtlDriver);	//debug
*/
			if((**curDCtlEntry).dCtlFlags & dRAMBased) {
				curDrvrPtr = *(DriverHandle)(**curDCtlEntry).dCtlDriver;
/*
 				debugPrintH("Ram driver handle: ",&temp);
*/
			}
			else {
				curDrvrPtr = (DriverPtr)(**curDCtlEntry).dCtlDriver;
/*
 				debugPrintL("Rom driver Ptr: ",&temp);
*/
			}
			
			/* Does the driver name match? */
			if(curDrvrPtr != nil)
			
			/*
			DebugStr("\pcurDrvrPtr");
			*/
			
				if(EqualString(drvrName, curDrvrPtr->drvrName,
				               false, false)) {
/*
 					DebugStr(curDrvrPtr->drvrName);
 					debugPrintS("Driver Unit Number: ",&unitNum);
 					thandle = (**curDCtlEntry).dCtlStorage;
 					debugPrintH("Driver Storage handle: ",&thandle);
*/
					return(~unitNum);
				}
		}
	}
 	DebugStr("\pCouldn't find driver");
	return(0);
}

OSErr GrowUTable(short newEntries)
{
	DCtlHandle *newUTableBase;
	short savedStatus;
	
	/*
	DebugStr("\pGrowUTable");
	*/
	
	/* Make room for the new Unit Table */
	newUTableBase = (DCtlHandle *)NewPtrSysClear((UnitNtryCnt + newEntries)
													* sizeof(DCtlHandle));
	if(MemError() != noErr)
		return(MemError());
		
	savedStatus = DisableInterrupts();
	/* 
	FIXME jcm - Turning Think inline assembler into standalone .a procs for MPW 
	asm {
		MOVE	SR,-(SP)		; Save status register
		MOVE	#SCCLockOut,SR	; Disable interrupts
	}
	*/
	
	/* Move the old Unit Table to the new Unit Table */
	BlockMove((void *)UTableBase,(void *)newUTableBase, UnitNtryCnt * sizeof(DCtlHandle));
	DisposPtr((Ptr) UTableBase);
	UTableBase = newUTableBase;
	UnitNtryCnt += newEntries;
	
	EnableInterrupts(savedStatus);
	/*
	FIXME jcm - Turning Think inline assembler into stadalone .a procs for MPW 
	asm {
		MOVE	(SP)+,SR		; Restore status register
	}
	*/ 
	
	return(noErr);
}

OSErr DriverAvail(short *unitNum)
{
	short unitIndex;
	Size UTableSize;
	OSErr errCode = noErr;
	
	*unitNum = 0;
	
	/* Increase Unit Table size for Mac Plus */
	if(UnitNtryCnt <= LOW_UNIT)
		errCode = GrowUTable(NEW_UNIT - UnitNtryCnt);
	if(errCode != noErr)
		return(errCode);
	
	/* Look for an empty slot in what's already there */
	for(unitIndex = LOW_UNIT;
			(unitIndex < UnitNtryCnt) && (*unitNum == 0);
			++unitIndex)
			
		if(UTableBase[unitIndex] == nil) {
			*unitNum = unitIndex;
			/*
			DebugStr("\p*unitNum = unitIndex, under UnitNtryCnt");
			*/
		}
	
	/* Unit Table full up to UnitNtryCnt, so increase it */
	if(*unitNum == 0) {
		UTableSize = GetPtrSize((Ptr)UTableBase) / sizeof(DCtlHandle);
		
		/* If there is space in the Unit Table, just up the count */
		if(UTableSize > UnitNtryCnt) {
			*unitNum = UnitNtryCnt;
			/*
			DebugStr("\p*unitNum = UnitNtryCnt. UTableSize > UnitNtryCnt");
			*/
			UnitNtryCnt += (UTableSize - UnitNtryCnt < UP_UNIT
								? UTableSize - UnitNtryCnt
								: UP_UNIT);
		
		
		/* If there isn't enough space, try to increase it */
		} else {
			if(MAX_UNIT - UnitNtryCnt != 0) {
				unitIndex = UnitNtryCnt;
				errCode = GrowUTable(MAX_UNIT - UnitNtryCnt < UP_UNIT
										? MAX_UNIT - UnitNtryCnt
										: UP_UNIT);
				if(errCode != noErr)
					return(errCode);
				*unitNum = unitIndex;
				/*
				DebugStr("\p*unitNum = unitIndex. grow utable to max.");
				*/
			}
		}
	}
	if(*unitNum == 0)
		return(unitTblFullErr);
	else
		return(noErr);
}

void ReleaseDrvrSegments(Handle *dcodHList, short rsrcID, Boolean detach)
{
	short index, dcodHandles, dcodRsrcID;
 	Handle tempHandle;	//debug
	ResType rsrcType;
	Str255 rsrcName;
	
	dcodHandles = GetPtrSize((Ptr) /*&*/dcodHList) / sizeof(Handle);
 	debugPrintS("ReleaseDrvrSegments: ",&dcodHandles);
	if(rsrcID != 0) {
		for(index = 0; index < dcodHandles; ++index)
			if(dcodHList[index]) {
				/* FIXED jcm - Think wants a Str255 not a pointer to one as the last arg */
				GetResInfo(dcodHList[index], &dcodRsrcID,
				           &rsrcType, rsrcName);
				SetResInfo(dcodHList[index],
							(dcodRsrcID & 0xF01F) + (rsrcID << 5),
							0);
			}
		SetResFileAttrs(CurResFile(), GetResFileAttrs(index) | mapChanged);
		UpdateResFile(CurResFile());
	}
	for(index = 0; index < dcodHandles; ++index)
		if(dcodHList[index]) {
			if(detach) {
 				tempHandle = dcodHList[index];	// debug
				DetachResource(dcodHList[index]);
			}
			else
				ReleaseResource(dcodHList[index]);
		}
	if(!detach)
		DisposPtr((Ptr)dcodHList);
}