/*
 * $Id: oclient.h,v 1.6 1999-06-28 22:52:27 ghudson Exp $
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <hesiod.h>
#include <string.h>
#include <ctype.h>

#include "system.h"

#include "requests.h"

#ifdef HAVE_KRB4
#define K_SERVICE "olc"
#endif /* HAVE_KRB4 */

/* oreplay.c */
void usage (void);
void punt (int fd, char *filename);

/* io.c */
int sread (int fd , void *buf , int nbytes );
int swrite (int fd , void *buf , int nbytes );
void expand_hostname (char *hostname , char *instance , char *realm );
