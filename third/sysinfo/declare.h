/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __sysinfo_declare_h__
#define __sysinfo_declare_h__ 

/*
 * Declarations
 */

/*
 * Variables
 */
extern int 			DoPrintUnknown;
extern int 			MsgLevel;
extern int 			MsgClassFlags;
extern int 			Terse;
extern int 			UseProm;
extern char 		       *UnSupported;
extern char		       *ProgramName;
extern DefineList_t	       *Definetions;
extern DevDefine_t	       *DevDefinetions;

/*
 * Functions
 */

#if	ARG_TYPE == ARG_STDARG
extern void			SImsg(int MsgType, char *fmt, ...);
#else	/* !ARG_STDARG */
extern void			SImsg();
#endif	/* ARG_STDARG */

#if	!defined(HAVE_STR_DECLARE)
char 			       *strchr();
char		 	       *strrchr();
char 			       *strdup();
char 			       *strcat();
char 			       *strtok();
#endif	/* HAVE_STR_DECLARE */

char			       *itoa();
char			       *itoax();
void	 		       *xmalloc();
void	 		       *xrealloc();
void	 		       *xcalloc();
int			        StrToArgv();
void				DestroyArgv();
extern char		       *strlower();
extern char		       *strupper();
extern void			strtolower();

extern DevDefine_t	       *DevDefGet();
extern void			DevDefAdd();

extern Define_t		       *DefGet();
extern Define_t		       *DefGetList();
extern void			DefAdd();
extern int			DefValid();

extern void			Report();

extern void			PCIsetDeviceInfo();

extern DevDesc_t	       *PrimeDescPtr();
extern DevInfo_t 	       *DeviceCreate();
extern DevInfo_t	       *DDBgetDevInfo();
extern DevInfo_t	       *DevFind();
extern DevInfo_t	       *MkMasterFromDevData();
extern DevInfo_t	       *ProbeDevice();
extern DevInfo_t 	       *ProbeDiskDrive();
extern DevInfo_t 	       *ProbeFloppy();
extern DevInfo_t 	       *ProbeFrameBuffer();
extern DevInfo_t	       *ProbeUnknown();
extern DevInfo_t 	       *NewDevInfo();
extern DevInfo_t 	       *ProbeCDROMDrive();
extern DevInfo_t 	       *ProbeKbd();
extern DevInfo_t 	       *ProbeNetif();
extern DevInfo_t 	       *ProbeTapeDrive();
extern DiskDrive_t	       *NewDiskDrive();
extern DiskDriveData_t	       *NewDiskDriveData();
extern DiskPart_t  	       *NewDiskPart();
extern FrameBuffer_t 	       *NewFrameBuffer();
extern Monitor_t	       *NewMonitor();
extern NetIF_t	 	       *NewNetif();
extern char		       *CleanString();
extern char		       *FreqStr();
extern char		       *GetCpuName();
extern char		       *GetCpuType();
extern char		       *GetCpuTypeCmds();
extern char		       *GetCpuTypeDef();
extern char		       *GetCpuTypeHostInfo();
extern char		       *GetCpuTypeIsalist();
extern char		       *GetCpuTypeSysinfo();
extern char		       *GetCpuTypeTest();
extern char		       *GetHostName();
extern char		       *GetKernArch();
extern char		       *GetKernArchCmds();
extern char		       *GetKernArchDef();
extern char		       *GetKernArchSysinfo();
extern char		       *GetKernArchUname();
extern char		       *GetKernArchTest();
extern char		       *GetNameTabName();
extern char		       *GetOSNameSysinfo();
extern char		       *GetOSNameUname();
extern char		       *GetOSNameDef();
extern char		       *GetOSVerDef();
extern char		       *GetOSVerKernVer();
extern char		       *GetOSVerSysinfo();
extern char		       *GetOSVerUname();
extern char		       *GetSizeStr();
extern char		       *GetTapeModel();
extern char		       *MkDevName();
extern char		       *RunCmds();
extern char		       *RunTestFiles();
extern char	 	       *GetMemoryPhysmemSym();
extern char	 	       *MkDevName();
extern char	 	       *MkMasterName();
extern char 		       *GetAppArch();
extern char 		       *GetAppArchCmds();
extern char 		       *GetAppArchDef();
extern char 		       *GetAppArchSysinfo();
extern char 		       *GetAppArchTest();
extern char 		       *GetBootTime();
extern char 		       *GetBootTimeUtmp();
extern char 		       *GetBootTimeGetutid();
extern char 		       *GetBootTimeSym();
extern char 		       *GetCPU();
extern char 		       *GetCharFile();
extern char 		       *GetDiskCapacity();
extern char 		       *GetHostAddrs();
extern char 		       *GetHostAliases();
extern char 		       *GetHostID();
extern char 		       *GetKernVer();
extern char 		       *GetKernVerSym();
extern char 		       *GetMan();
extern char 		       *GetManLong();
extern char 		       *GetManLongDef();
extern char 		       *GetManLongSysinfo();
extern char 		       *GetManShort();
extern char 		       *GetManShortDef();
extern char 		       *GetManShortSysinfo();
extern char 		       *GetMemory();
extern char 		       *GetMemoryStr();
extern char 		       *GetModel();
extern char 		       *GetModelCmd();
extern char 		       *GetModelDef();
extern char 		       *GetModelFile();
extern char 		       *GetNumCpu();
extern char 		       *GetNumCpuHostInfo();
extern char 		       *GetNumCpuSysconf();
extern char 		       *GetOSName();
extern char 		       *GetOSDist();
extern char 		       *GetOSVer();
extern char 		       *GetRawFile();
extern char 		       *GetRomVer();
extern char 		       *GetSerial();
extern char 		       *GetSerialSysinfo();
extern char 		       *GetVendorName();
extern char 		       *GetVirtMem();
extern char 		       *GetVirtMemAnoninfo();
extern char 		       *GetVirtMemNswap();
extern char 		       *GetVirtMemSwapctl();
extern char 		       *GetVirtMemStr();
extern char 		       *PSIquery();
extern char 		       *PrimeDesc();
extern int		        AddDevice();
extern void		        DeviceList();
extern void		        DeviceShow();
extern void		        DevAddFile();
extern void		        KernelList();
extern void		        KernelShow();
extern void		        SetMacInfo();
extern void 		        DetectDevices();
extern void 		        GeneralList();
extern void 		        GeneralShow();
extern void 		        SysConfList();
extern void 		        SysConfShow();

extern int			ScsiCmd();

extern void		        ClassShowBanner();
extern void		        ClassShowValue();
extern void			ClassList();
extern void			ClassSetInfo();
extern int			ClassCall();
extern void			ClassCallList();

extern void			TypeList();
extern void			TypeSetInfo();
extern DevType_t	       *TypeGetByType();
extern DevType_t	       *TypeGetByName();
extern ClassType_t	       *ClassTypeGetByType();
extern ClassType_t	       *ClassTypeGetByName();

extern namelist_t	       *NameListFind();
extern void 		        NameListAdd();
extern void 		        NameListFree();

extern void			DevTypesInit();
extern DevType_t		DevTypes[];
extern FormatType_t		FormatType;

extern char		       *VarSub();
extern char		       *VarGetVal();

#if	defined(HAVE_KVM)
extern kvm_t 		       *KVMopen();
extern void			KVMclose();
extern int			KVMget();
#if	defined(HAVE_NLIST)
extern nlist_t		       *KVMnlist();
#endif	/* HAVE_NLIST */
#endif	/* HAVE_KVM */

#endif /* __sysinfo_declare_h__ */
