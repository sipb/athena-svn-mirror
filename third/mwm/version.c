/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: version.c,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:28 $"
#endif
#endif
/*
*  (c) Copyright 1988, 1989, 1990, HEWLETT-PACKARD COMPANY */

#ifndef        lint
#define        osfversion() \
   static char _motif_version[] = "@(#)OSF/Motif mwm 1.2.3 Release";
#else  /* lint */
#define        osfversion()
#endif /* lint */

osfversion()
