/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: getopt.h,v 1.1.1.3 2000-03-31 15:48:09 mwhitson Exp $
 ***************************************************************************/



#ifndef _GETOPT_H_
#define _GETOPT_H_ 1


extern int Optind, Opterr;
extern char *Optarg;
extern char *Name;			/* program name */

/* PROTOTYPES */
int Getopt( int argc, char *argv[], char *optstring );

#endif
