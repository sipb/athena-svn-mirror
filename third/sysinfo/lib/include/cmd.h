/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * Header file for Cmd interface
 */
#ifndef __cmd_h__
#define __cmd_h__ 

#include <sys/types.h>

#define CMF_READ		0x001		/* Read from FD */
#define CMF_WRITE		0x002		/* Write to FD */
#define CMF_STDERR		0x100		/* Enable stderr */
#define CMF_WITHPRIVS		0x200		/* Run with current privs */

/*
 * Cmd type
 */
typedef struct {
    /* Input from caller */
    char		      **CmdPath;	/* Possible cmd paths */
    char		      **Argv;		/* Args to command */
    int				Argc;		/* Arg count of Argv */
    char		      **Env;		/* Environment */
    int				Flags;		/* CMF_* */
    /* Caller public */
    int				FD;		/* File Descriptor */
    /* Private */
    char		       *Program;	/* Argv[0] */
    pid_t			ProcID;		/* PID of process we run */
} Cmd_t;

/*
 * Declarations
 */
extern int			CmdOpen();
extern int			CmdClose();

#endif	/* __cmd_h__ */
