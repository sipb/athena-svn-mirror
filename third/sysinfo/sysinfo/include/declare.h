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

#if	!defined(HAVE_STR_DECLARE)
char 			       *strchr();
char		 	       *strrchr();
char 			       *strdup();
char 			       *strcat();
char 			       *strtok();
#endif	/* HAVE_STR_DECLARE */

extern int 			MsgLevel;
extern FormatType_t		FormatType;
extern char		       *ProgramName;
extern char 		       *UnSupported;

extern void			ClassList();
extern void			ClassCallList();
extern void			TypeList();
extern void 		        GeneralList();
extern void 		        GeneralShow();
extern void		        KernelList();
extern void		        KernelShow();
extern void 		        SysConfList();
extern void 		        SysConfShow();
extern void		        DeviceList();
extern void		        DeviceShow();
extern void		        ShowSoftInfo();
extern void		        ListSoftInfo();
extern void		        ShowPartition();
extern void		        ListPartition();

extern char		       *CFchooseConfDir();

extern Define_t                *DefGetList();

#endif /* __sysinfo_declare_h__ */
