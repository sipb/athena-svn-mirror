/*
 * Copyright (c) 1990 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Revision: 1.1.1.1 $
 */
#ifndef __mc_options_h__
#define __mc_options_h__

#if	defined(HAVE_AUTOCONFIG_H)
#include "autoconfig.h"
#endif	/* HAVE_AUTOCONFIG_H */

#include <stdio.h>
#include <sys/types.h>

/* Define ARG_* if needed */
#ifndef ARG_NONE
#define ARG_NONE	1
#endif
#ifndef ARG_STDARG
#define ARG_STDARG	2
#endif
#ifndef ARG_VARARG
#define ARG_VARARG	3
#endif
/*
 * Set ARG_TYPE if not already set
 */
#if 	!defined(ARG_TYPE) && defined(HAVE_STDARG_H) && defined(__STDC__)
	/* ARG_TYPE = ARG_STDARG */
#define ARG_TYPE	ARG_STDARG
#include <stdarg.h>
#endif
#if	!defined(ARG_TYPE) && (defined(HAVE_VARARGS) || defined(HAVE_VARARGS_H))
	/* ARG_TYPE = ARG_VARARG */
#define ARG_TYPE	ARG_VARARG
#include <varargs.h>
#endif
/* Set to something */
#if	!defined(ARG_TYPE)
	/* ARG_TYPE = ARG_NONE */
#define ARG_TYPE	ARG_NONE
#endif

#define Num_Opts(o)	(int) (sizeof(o)/sizeof(OptionRec_t))
#define HELPSTR		"-help"

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/*
 * Values for OptionRec_t.flags.
 */
#define NoArg		0x001	/* No argument for this option.  Use
				   OptionRec_t.value. */
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
} OptionRec_t;
#define OptionDescRec	OptionRec_t

OptionRec_t    *FindOption();
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
#endif

#endif /*  __mc_options_h__ */
