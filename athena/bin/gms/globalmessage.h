/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/globalmessage.h,v $
 * $Author: miki $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#ifndef __GLOBALMESSAGE_H__
#define __GLOBALMESSAGE_H__

#include <errno.h>
#ifdef SOLARIS
#include <string.h>
#include <fcntl.h>
#else
#include <strings.h>
#endif
extern int errno;
#include "globalmessage_err.h"
	/* Function return code */
typedef int Code_t;

#define GMS_NAME_CLASS "globalmessage"
#define GMS_NAME_TYPE "sloc"

#define GMS_SERV_NAME "globalmessage"
#define GMS_SERV_PROTO "udp"

#define GMS_MESSAGE_NAME "/usr/tmp/.messages"

#define GMS_USERFILE_NAME "/.message_times"
#define GMS_USERFILE_NAME_LEN (sizeof(GMS_USERFILE_NAME)-1)

#define GMS_VERSION_STRING "GMS:0"
#define GMS_VERSION_STRING_LEN (sizeof(GMS_VERSION_STRING)-1)

#define GMS_MAX_MESSAGE_LEN 2048

#define GMS_TIMEOUT_SEC 5
#define GMS_TIMEOUT_USEC 0

/* log_10(2^32) >= 10, plus a NL, plus a NUL */
#define GMS_TIMESTAMP_LEN 12

#define BFSZ 1024

char *malloc(), *realloc();

#endif /* __GLOBALMESSAGE_H__ */
