/* 
 * $Id: aklog_param.c,v 1.8 1994-04-14 18:46:34 probe Exp $
 * 
 * Copyright 1990,1991 by the Massachusetts Institute of Technology
 * For distribution and copying rights, see the file "mit-copyright.h"
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Id: aklog_param.c,v 1.8 1994-04-14 18:46:34 probe Exp $";
#endif /* lint || SABER */

#include "aklog.h"
#include <sys/stat.h>

extern int readlink ARGS((const char *, void *, int));

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#ifdef __STDC__
static int isdir(char *path, unsigned char *val)
#else
static int isdir(path, val)
  char *path;
  unsigned char *val;
#endif /* __STDC__ */
{
    struct stat statbuf;

    if (lstat(path, &statbuf) < 0)
	return (-1);
    else {
	if ((statbuf.st_mode & S_IFMT) == S_IFDIR) 
	    *val = TRUE;
	else
	    *val = FALSE;
	return (0);
    }  
}


#ifdef __STDC__
static int get_cred(char *name, char *inst, char *realm, CREDENTIALS *c)
#else
static int get_cred(name, inst, realm, c)
  char *name;
  char *inst;
  char *realm;
  CREDENTIALS *c;
#endif /* __STDC__ */
{
    int status; 

    status = krb_get_cred(name, inst, realm, c);
    if (status != KSUCCESS) {
	status = get_ad_tkt(name, inst, realm, 255);
	if (status == KSUCCESS)
	    status = krb_get_cred(name, inst, realm, c);
    }

    return (status);
}


#ifdef __STDC__
static int get_user_realm(char *realm)
#else
static int get_user_realm(realm)
  char *realm;
#endif /* __STDC__ */
{
    return (krb_get_tf_realm(TKT_FILE, realm));
}


#ifdef __STDC__
static void pstderr(char *string)
#else
static void pstderr(string)
  char *string;
#endif /* __STDC__ */
{
    write(2, string, strlen(string));
}


#ifdef __STDC__
static void pstdout(char *string)
#else
static void pstdout(string)
  char *string;
#endif /* __STDC__ */
{
    write(1, string, strlen(string));
}


#ifdef __STDC__
static void exitprog(char status)
#else
static void exitprog(status)
  char status;
#endif /* __STDC__ */
{
    exit(status);
}


#ifdef __STDC__
void aklog_init_params(aklog_params *params)
#else
void aklog_init_params(params)
  aklog_params *params;
#endif /* __STDC__ */
{
    params->readlink = readlink;
    params->isdir = isdir;
    params->getcwd = getcwd;
    params->get_cred = get_cred;
    params->get_user_realm = get_user_realm;
    params->pstderr = pstderr;
    params->pstdout = pstdout;
    params->exitprog = exitprog;
}


#if defined(vax)
static char *getcwd(buf, size)
    char *buf;
    size_t size;
{
    return(getwd(buf));
}
#endif
