/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: run-data.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
#endif

/*
 * Data for "run" commands.
 */

#include "defs.h"

/*
 * Specific command to determine our model name.
 */
#if	defined(MODEL_COMMAND)
char *ModelCommand[] = { MODEL_COMMAND, NULL };
#endif	/* MODEL_COMMAND */

/*
 * Application architecture commands.  
 * These commands should print the system's application architecture.
 */
char *AppArchCmds[] = { 
    "/bin/arch", 
    "/bin/mach", 
    "/bin/machine", 
    NULL };

/*
 * Kernel architecture commands.  
 * These commands should print the system's kernel architecture.
 */
char *KernArchCmds[] = { 
    "/bin/arch -k", 
    "/bin/mach",
    "/bin/machine", 
    NULL };

/*
 * Architecture test files.
 * Each test file is run and if the exit status is 0, 
 * the basename of the command is the name of the system architecture. 
 */
char *AppArchFiles[] = { 
    "/bin/alliant", 
    "/bin/vax", 
    "/bin/sun", 
    NULL };

/*
 * Kernel Architecture test files.
 * Each test file is run and if the exit status is 0, 
 * the basename of the command is the name of the kernel architecture. 
 */
char *KernArchFiles[] = { 
    "/bin/hp9000s200", 
    "/bin/hp9000s300", 
    "/bin/hp9000s400", 
    "/bin/hp9000s500", 
    "/bin/hp9000s700", 
    "/bin/hp9000s800", 
    NULL };

/*
 * CPU type test files.
 * Each test file is run and if the exit status is 0, 
 * the basename of the command is the name of the system CPU type. 
 */
char *CPUFiles[] = { 
	"/bin/sparc",
	"/bin/mc68010",
	"/bin/mc68020",
	"/bin/mc68030",
	"/bin/mc68040",
	"/bin/m68k",
	"/bin/vax",
	"/bin/alliant",
	"/bin/i386", 
	"/bin/i860", 
	"/bin/iAPX286",
	"/bin/pdp11",
	"/bin/u370",
	"/bin/u3b15",
	"/bin/u3b2",
	"/bin/u3b5",
	"/bin/u3b",
	NULL };
