/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions for operating-system routines that don't
 * appear to be defined elsewhere, at least in BSD.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/system.h,v $
 *	$Id: system.h,v 1.3 1991-04-08 21:03:01 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

#ifndef HAS_ANSI_INCLUDES
#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

#include <sys/types.h>
#ifdef KERBEROS
#include <krb.h>

char *krb_realmofhost P((char *host));
int krb_mk_req P((KTEXT authent, char *service, char *instance, char *realm,
		  u_long checksum)); 

#endif /* KERBEROS */

char **hes_resolve P((char *name, char *HesiodNameType));
void *calloc P((unsigned nelem, unsigned elsize));
char *index P((char *s, int c));
void *malloc P((unsigned size));
char *strcpy P((char *s1, char *s2));
char *strncpy P((char *s1, char *s2, int n));
int atoi P((char *nptr));
int bcopy P((void *src, void *dst, int length));
int bzero P((void *b, int length));
int close P((int d));
int exit P((int status));
int free P((void *ptr));
int getdtablesize P(());
int getopt P((int argc, char **argv, char *optstring));
int getsockopt P((int s, int level, int optname, caddr_t optval,int *optlen));
struct hostent *gethostbyname P((char *name));
void ioctl P((int d, unsigned int request, char *argp));
int listen P((int s, int backlog));
off_t lseek P((int d, off_t offset, int whence));
int open P((char *path, int flags, int mode));
int open P((char *path, int flags, int mode));
void openlog P((char *ident, int logopt, int facility));
int perror P((char *s));
int psignal P((unsigned sig, char *s));
int read P((int d, void *buf, int nbytes));
struct servent *getservbyname P((char *name, char *proto));
int setsockopt P((int s, int level, int optname, void *optval, int optlen));
int shutdown P((int s, int how));
int socket P((int domain, int type, int protocol));
int strcmp P((char *s1, char *s2));
int strlen P((char *s));
void syslog P((int priority, char *message, ...));
int write P((int d, void *buf, int nbytes));

#undef P
#endif
