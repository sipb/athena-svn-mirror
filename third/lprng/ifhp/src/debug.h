/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****
$Id: debug.h,v 1.1.1.3 1999-05-04 18:50:40 mwhitson Exp $
 **** ENDHEADER ****/

/****************************************
 * DEBUG Flags
 ****************************************/

#ifndef _DEBUG_H_
#define _DEBUG_H_ 1

/* to remove all debugging, redefine this as follows
 * note that a good optimizing compiler should not produce code
 *	for the logmsg call.  It may produce lots of warnings, but no code...
 */

#ifdef NODEBUG

#define DEBUG0      if(0) logmsg
#define DEBUGL0     (0)
#define DEBUG1      if(0) logmsg
#define DEBUGL1     (0)
#define DEBUG2      if(0) logmsg
#define DEBUGL2     (0)
#define DEBUG3      if(0) logmsg
#define DEBUGL3     (0)
#define DEBUG4      if(0) logmsg
#define DEBUGL4     (0)
#define DEBUG5      if(0) logmsg
#define DEBUGL5     (0)

#else

EXTERN int Debug;	/* debug flags */

/* Debug variable level */
#define DEBUG0      if(Debug>0)logmsg
#define DEBUGL0     (Debug>0)
#define DEBUG1      if(Debug>=1)logmsg
#define DEBUGL1     (Debug>=1)
#define DEBUG2      if(Debug>=2)logmsg
#define DEBUGL2     (Debug>=2)
#define DEBUG3      if(Debug>=3)logmsg
#define DEBUGL3     (Debug>=3)
#define DEBUG4      if(Debug>=4)logmsg
#define DEBUGL4     (Debug>=4)
#define DEBUG5      if(Debug>=5)logmsg
#define DEBUGL5     (Debug>=5)

#endif
#endif
