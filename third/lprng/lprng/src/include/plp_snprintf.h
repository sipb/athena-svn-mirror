/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: plp_snprintf.h,v 1.1.1.1 1999-05-04 18:07:15 danw Exp $
 ***************************************************************************/



#ifndef _PLP_SNPRINTF_
#define _PLP_SNPRINTF_
/* VARARGS3 */
#ifdef HAVE_STDARGS
int	plp_snprintf (char *str, size_t count, const char *fmt, ...);
int	plp_vsnprintf (char *str, size_t count, const char *fmt, va_list arg);
#else
int plp_snprintf ();
int plp_vsnprintf ();
#endif
#endif
