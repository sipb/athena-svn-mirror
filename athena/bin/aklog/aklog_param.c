/* 
 * $Id: aklog_param.c,v 1.10 1997-11-17 16:23:49 ghudson Exp $
 * 
 * Copyright 1990,1991 by the Massachusetts Institute of Technology
 * For distribution and copying rights, see the file "mit-copyright.h"
 */

static const char rcsid[] = "$Id: aklog_param.c,v 1.10 1997-11-17 16:23:49 ghudson Exp $";

#include "aklog.h"
#include <sys/stat.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


static int isdir(char *path, unsigned char *val)
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


static int get_cred(char *name, char *inst, char *realm, CREDENTIALS *c)
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


static int get_user_realm(char *realm)
{
    return (krb_get_tf_realm(TKT_FILE, realm));
}


static void pstderr(char *string)
{
    write(2, string, strlen(string));
}


static void pstdout(char *string)
{
    write(1, string, strlen(string));
}


static void exitprog(char status)
{
    exit(status);
}


void aklog_init_params(aklog_params *params)
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
