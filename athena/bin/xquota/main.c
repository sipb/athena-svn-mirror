/*
 * xquota -  X window system quota display program.
 *
 * $Athena: man.c,v 1.7 89/02/15 16:06:44 kit Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   March 5, 1989
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: man.c,v 4.7 89/02/15 20:09:34 kit Locked $";
#endif

#include <stdio.h>
#include <X11/Intrinsic.h>

#include <pwd.h>
#include <hesiod.h>
#include <ctype.h>

#include "xquota.h"

#define FILSYS ("filsys")
#define NFS    ("NFS")
#define AFS    ("AFS")

static void ParseHesiodInfo();
static void GetUserFilsys();

Info info;			/* external for action proc only. */

int homeIsAFS = 0;

/*	Function Name: main
 *	Description: main driver for the quota check program.
 *	Arguments: argc, argv - you all know about these right?
 *	Returns: none.
 */

void
main(argc, argv)
int argc;
char ** argv;
{
  Widget top;

  GetUserFilsys(&info);

  top = InitializeWorld(&info, &argc, argv);
  CreateWidgets(top, &info);

  XtRealizeWidget(top);
  XtAppMainLoop(info.appcon);
}

/*	Function Name: GetQuota
 *	Description: Gets the quota for user who invoke this program.
 *	Arguments: dqblk - the quota info ** RETURNED **
 *	Returns: none.
 */

static void
GetUserFilsys(info)
Info * info;
{
  struct passwd *user_info;
  char ** hesinfo;
  char *type;

/*
 * Proceedure:
 * 
 * o Get users filsys entries from hesiod.
 * o Use first entry only.
 * o If it is not an NFS or AFS filsys or hesiod failed the give up.
 */

  info->uid = getuid();
  user_info = getpwuid(info->uid);
  hesinfo = hes_resolve(user_info->pw_name, FILSYS);
  if ( hesinfo != NULL ) {
    ParseHesiodInfo(*hesinfo, &type, &(info->host), &(info->path) );
    if ( !streq(type, NFS) && !streq(type, AFS) ) {
      printf("Your home filesystem is not NFS or AFS, giving up.\n");
      exit(0);
    }
    if (streq(type, AFS)) {
	homeIsAFS = 1;
	info->host = "AFS cell";
    }
  }
  else {
    printf("hesiod failure, giving up.\n");
    hes_error();
    exit(0);
  }

  hesinfo++;			/* We are still using this space. */

  if (*hesinfo != NULL) {
    printf("You have more than one filesystem we will only provide a quota\n");
    printf("graph for the filsys: %s on machine %s.\n",
	   info->path, info->host);
    while (*hesinfo != NULL)	/* clean up. */
      free(*hesinfo++);
  }
}

/*	Function Name: ParseHesiodInfo
 *	Description: Parses a hesiod filsys entry.
 *	Arguments: string - string from hesiod.
 *                 type - type of filsys ** RETURNED **
 *                 host - host of filsys ** RETURNED **
 *                 path - path to filsys ** RETURNED **
 *	Returns: none
 */

static void
ParseHesiodInfo(string, type, host, path)
char * string;
char **type, **host, **path;
{
  char * c = string;

  *type = string;
  while( !isspace(*c) ) c++;
  *c = '\0';
  *path = ++c;
  while( !isspace(*c) ) c++;
  *c = '\0';
  *host = ++c;
  while( !isspace(*c) ) c++;
  *c = '\0';
}
