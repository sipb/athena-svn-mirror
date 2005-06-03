/*$Header: /afs/dev.mit.edu/source/repository/third/tcsh/win32/version.h,v 1.1.1.1 2005-06-03 14:35:56 ghudson Exp $*/
#ifndef VERSION_H
#define VERSION_H

/* remember to change both instance of the version -amol */

#ifdef NTDBG
#define LOCALSTR ",nt-rev-7.15-debug"
#else
#define LOCALSTR ",nt-rev-7.15" 
								//patches
#endif NTDBG

#endif VERSION_H
