/*
 * cmd.h:
 * This file is automatically generated; please do not edit it.
 */
/* Including cmd.p.h at beginning of cmd.h file. */

/* $Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_63/include/afs/cmd.h,v 1.1.1.1 1997-10-16 14:40:28 ghudson Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_63/include/afs/cmd.h,v $ */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988, 1989
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef __CMD_INCL__
#define	__CMD_INCL__	    1
#include <afs/param.h>

/* parmdesc types */
#define	CMD_FLAG	1	/* no parms */
#define	CMD_SINGLE	2	/* one parm */
#define	CMD_LIST	3	/* two parms */

/* syndesc flags */
#define	CMD_ALIAS	1	/* this is an alias */

#define CMD_HELPPARM	31	/* last one is used by -help switch */
#define	CMD_MAXPARMS	32	/* max number of parm types to a cmd line */

/* parse items are here */
struct cmd_item {
    struct cmd_item *next;
    char *data;
};

struct cmd_parmdesc {
    char *name;			/* switch name */
    int	type;			/* flag, single or list */
    struct cmd_item *items;	/* list of cmd items */
    int32 flags;			/* flags */
    char *help;			/* optional help descr */
};

/* cmd_parmdesc flags */
#define	CMD_REQUIRED	    0
#define	CMD_OPTIONAL	    1
#define	CMD_EXPANDS	    2	/* if list, try to eat tokens through eoline, instead of just 1 */
#define	CMD_PROCESSED	    8

struct cmd_syndesc {
    struct cmd_syndesc *next;	/* next one in system list */
    struct cmd_syndesc *nextAlias;  /* next in alias chain */
    struct cmd_syndesc *aliasOf;    /* back ptr for aliases */
    char *name;		    /* subcommand name */
    char *a0name;	    /* command name from argv[0] */
    char *help;		    /* help description */
    int (*proc)();
    char *rock;
    int	nParms;		    /* number of parms */
    int32 flags;		    /* random flags */
    struct cmd_parmdesc parms[CMD_MAXPARMS];	/* parms themselves */
};

#if 0
#define	CMD_TOOMANY	    1900
#define	CMD_SYNTAX	    1901
#define	CMD_AMBIG	    1902
#define	CMD_TOOFEW	    1903
#endif

extern struct cmd_syndesc *cmd_CreateSyntax();

#endif /* __CMD_INCL__ */

/* End of prolog file cmd.p.h. */

#define CMD_EXCESSPARMS                          (3359744L)
#define CMD_INTERNALERROR                        (3359745L)
#define CMD_NOTLIST                              (3359746L)
#define CMD_TOOMANY                              (3359747L)
#define CMD_USAGE                                (3359748L)
#define CMD_UNKNOWNCMD                           (3359749L)
#define CMD_UNKNOWNSWITCH                        (3359750L)
#define CMD_AMBIG                                (3359751L)
#define CMD_TOOFEW                               (3359752L)
#define CMD_TOOBIG                               (3359753L)
extern void initialize_cmd_error_table ();
#define ERROR_TABLE_BASE_cmd (3359744L)

/* for compatibility with older versions... */
#define init_cmd_err_tbl initialize_cmd_error_table
#define cmd_err_base ERROR_TABLE_BASE_cmd