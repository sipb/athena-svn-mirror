/* 
 * $Id: aklog_param.c,v 1.1 1990-06-22 18:03:11 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/aklog/aklog_param.c,v $
 * $Author: qjb $
 *
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Id: aklog_param.c,v 1.1 1990-06-22 18:03:11 qjb Exp $";
#endif /* lint || SABER */

#include <aklog.h>


extern int readlink ARGS((char *, char *, int));
extern int lstat ARGS((char *, struct stat *));

#ifdef __STDC__
static void pstderr(char *string)
#else
static void pstderr(string)
  char *string;
#endif /* __STDC__ */
{
    write(2, string, strlen(string));
    write(2, "\n", 1);
}


#ifdef __STDC__
static void pstdout(char *string)
#else
static void pstdout(string)
  char *string;
#endif /* __STDC__ */
{
    write(1, string, strlen(string));
    write(1, "\n", 1);
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
    params->lstat = lstat;
    params->pstderr = pstderr;
    params->pstdout = pstdout;
    params->exitprog = exitprog;
}
