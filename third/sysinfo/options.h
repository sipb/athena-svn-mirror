/*
 * Copyright (c) 1990 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Revision: 1.1.1.2 $
 */
#ifndef __options_h__
#define __options_h__

#include <stdio.h>
#include <sys/types.h>
#include "mconfig.h"

#define Num_Opts(o)	(sizeof(o)/sizeof(OptionDescRec))
#define HELPSTR		"-help"

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/*
 * Values for OptionDescRec.flags.
 */
#define NoArg		0x001	/* No argument for this option.  Use
				   OptionDescRec.value. */
#define IsArg		0x002	/* Value is the option string itself */
#define SepArg		0x004	/* Value is in next argument in argv */
#define StickyArg	0x008	/* Value is characters immediately following 
				   option */
#define SkipArg		0x010	/* Ignore this option and the next argument in 
				   argv */
#define SkipLine	0x020	/* Ignore this option and the rest of argv */
#define SkipNArgs	0x040	/* Ignore this option and the next 
				   OptionDescRes.value arguments in argv */
#define ArgHidden	0x080	/* Don't show in usage or help messages */

/*
 * Option description record.
 */
typedef void *		OptPtr_t;
typedef struct {
    char	*option;		/* Option string in argv	    */
    int		 flags;			/* Flag bits			    */
    int		(*cvtarg)();		/* Function to convert argument     */
    OptPtr_t	 valp;			/* Variable to set		    */
    OptPtr_t	 value;			/* Default value to provide	    */
    char	*usage;			/* Usage message		    */
    char	*desc;			/* Description message		    */
} OptionDescRec, *OptionDescList;

OptionDescRec  *FindOption();
int 		OptBool();
int 		OptInt();
int 		OptLong();
int 		OptShort();
int 		OptStr();
int 		ParseOptions();
void 		HelpOptions();
void 		UsageOptions();
#if	defined(ARG_TYPE) && ARG_TYPE == ARG_STDARG
void 		UserError(char *fmt, ...);
#else	/* !ARG_STDARG */
void 		UserError();
#endif	/* ARG_STDARG */

extern char *OptionChars;
#ifndef _AIX
extern int errno;
extern long strtol();
extern char *strcpy();
#endif

#endif /*  __options_h__ */
