/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
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
extern int 			MsgClassFlags;
extern int 			Terse;
extern int 			UseProm;
extern char		       *ProgramName;
extern DefineList_t	       *Definetions;
extern DevDefine_t	       *DevDefinetions;

/*
 * Functions
 */

#if	defined(NEED_STR_DECLARE)
char 			       *strchr();
char		 	       *strrchr();
char 			       *strdup();
char 			       *strcat();
char 			       *strtok();
#endif	/* HAVE_NEED_DECLARE */

extern int			GetHostName();
extern int			GetHostAliases();
extern int			GetHostAddrs();

char			       *itoax();
Large_t				atoL();
void	 		       *xmalloc();
void	 		       *xrealloc();
void	 		       *xcalloc();
int			        StrToArgv();
void				DestroyArgv();
extern char		       *strlower();
extern char		       *strupper();
extern char		       *SkipWhiteSpace();
extern void			strtolower();

extern char		       *TimeToStr();

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
extern DiskDrive_t	       *DiskSetup();
extern DiskDrive_t	       *NewDiskDrive();
extern DiskDriveData_t	       *NewDiskDriveData();
extern DiskPart_t  	       *NewDiskPart();
extern FrameBuffer_t 	       *NewFrameBuffer();
extern Monitor_t	       *NewMonitor();
extern NetIF_t	 	       *NewNetif();
extern char		       *CleanString();
extern char		       *FreqStr();
extern char		       *GetCpuName();
extern int		       GetCpuType();
extern char		       *GetCpuTypeCmds();
extern char		       *GetCpuTypeDef();
extern char		       *GetCpuTypeHostInfo();
extern char		       *GetCpuTypeIsalist();
extern char		       *GetCpuTypeSysinfo();
extern char		       *GetCpuTypeTest();
extern int 		       DeviceInfoMCSI();
extern int 		       KernelVarsMCSI();
extern int		       GetKernArch();
extern int 		       GetSysConf();
extern char		       *GetKernArchStr();
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
extern int 		       GetAppArch();
extern char 		       *GetAppArchStr();
extern char 		       *GetAppArchCmds();
extern char 		       *GetAppArchDef();
extern char 		       *GetAppArchSysinfo();
extern char 		       *GetAppArchTest();
extern int 		       GetBootTime();
extern char 		       *GetBootTimeUtmp();
extern char 		       *GetBootTimeGetutid();
extern char 		       *GetBootTimeSym();
extern int			GetCurrentTime();
extern char 		       *GetCurrentTimeStrftime();
extern char 		       *GetCurrentTimeCtime();
extern int 		       GetCPU();
extern char 		       *GetCharFile();
extern char 		       *GetDiskCapacity();
extern int 		       GetHostID();
extern int 		       GetKernVer();
extern char 		       *GetKernVerSym();
extern int 		       GetMan();
extern int 		       GetManLong();
extern char 		       *GetManLongStr();
extern char 		       *GetManLongDef();
extern char 		       *GetManLongSysinfo();
extern int 		       GetManShort();
extern char 		       *GetManShortStr();
extern char 		       *GetManShortDef();
extern char 		       *GetManShortSysinfo();
extern int 		       GetMemory();
extern char 		       *GetMemoryStr();
extern int 		       GetModel();
extern char 		       *GetModelCmd();
extern char 		       *GetModelDef();
extern char 		       *GetModelFile();
extern int 		       GetNumCpu();
extern char 		       *GetNumCpuHostInfo();
extern char 		       *GetNumCpuSysconf();
extern int 		       GetOSName();
extern int 		       GetOSDist();
extern int 		       GetOSVer();
extern char 		       *GetRawFile();
extern int 		       GetRomVer();
extern int 		       GetSerial();
extern char 		       *GetSerialSysinfo();
extern char 		       *GetVendorName();
extern int 		       GetVirtMem();
extern char 		       *GetVirtMemAnoninfo();
extern char 		       *GetVirtMemNswap();
extern char 		       *GetVirtMemSwapctl();
extern char 		       *GetVirtMemStr();
extern char 		       *PSIquery();
extern char 		       *PrimeDesc();
extern int		        AddDevice();
extern void		        DevAddFile();
extern void		        SetMacInfo();
extern void 		        DetectDevices();

extern int			ScsiCmd();

extern int			IdentMatch();
extern char		       *IdentString();
extern Ident_t		       *IdentCreate();

extern void		        ClassShowBanner();
extern void		        ClassShowValue();
extern void			ClassList();
extern void			ClassSetInfo();
extern int			ClassCall();
extern void			ClassCallList();

extern void			TypeList();
extern void			TypeSetInfo();
extern DevType_t	       *TypeGetByName();
extern ClassType_t	       *ClassTypeGetByName();

extern namelist_t	       *NameListFind();
extern void 		        NameListAdd();
extern void 		        NameListFree();

extern SoftInfo_t	       *SoftInfoCreate();
extern SoftInfo_t	       *SoftInfoFind();
extern int			SoftInfoAdd();
extern int			SoftInfoMCSI();
extern int			SoftInfoDestroy();

extern char		       *SignalName();

extern PartInfo_t	       *PartInfoCreate();
extern int			PartInfoDestroy();
extern int			PartInfoAdd();
extern int			PartInfoMCSI();

extern void			DevTypesInit();
extern DevType_t		DevTypes[];

extern char		       *VarSub();
extern char		       *VarGetVal();

extern Offset_t			myllseek();

extern kvm_t 		       *KVMopen();
extern void			KVMclose();
extern int			KVMget();
#if	defined(HAVE_NLIST)
extern nlist_t		       *KVMnlist();
#endif	/* HAVE_NLIST */

extern char		       *DirName();
extern char		       *BaseName();

extern char		       *CFchooseConfDir();

#endif /* __sysinfo_declare_h__ */
