/*
 * $Id: oclient.h,v 1.5 1999-03-06 16:48:25 ghudson Exp $
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

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

#ifdef HAVE_KRB4
#define K_SERVICE "olc"
#endif /* HAVE_KRB4 */

/* oreplay.c */
void usage P((void));
void punt P((int fd, char *filename));

/* io.c */
int sread P((int fd , void *buf , int nbytes ));
int swrite P((int fd , void *buf , int nbytes ));
void expand_hostname P((char *hostname , char *instance , char *realm ));

#undef P
