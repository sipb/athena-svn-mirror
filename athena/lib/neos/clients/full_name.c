/**********************************************************************
 * full_name module
 *
 * $Author
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/full_name.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/full_name.c,v 1.3 1996-09-20 04:34:34 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#include <stdio.h>
#include <string.h>
#include <hesiod.h>

/**** borrowed from eos sources ****/
/*
  Takes a username and tries to find a password entry for it via Hesiod;
  If the resolve fails or the password entry cannot be parsed, then the
  original name is returned, else the name given in passwd is returned,
  with the parameter name following in parentheses;
  e.g. RealName("jsmith") == "jsmith" || "John Smith (jsmith)";
*/

char *full_name(name)
     char *name;
{
  char **namelist, *realname, *tmp;
  static char finalname[256];
  int i;

  if ((namelist = hes_resolve(name, "passwd")) == NULL) {
    strcpy(finalname, name);
    strcat(finalname, " (no hesiod info)");
  } else {
    /* Extract name from password entry */
    realname = *namelist;
    for (i=0; i<4; i++)
      if ((realname = strchr(++realname, ':')) == NULL) {
	/* Password entry is screwy - so give up and return original */
	strcpy(finalname, name);
	return finalname;
      }
    /* Remove rest of password entry */
    if ((tmp = strchr(++realname,':')) != NULL)
      *tmp = '\0';
    /* Make sure this is just the name, no unneccassry junk */
    if ((tmp = strchr(realname, ',')) != NULL)
      *tmp = '\0';
    /* Just to be nice, add on the original name */
    strcpy(finalname, realname);
    strcat(finalname, " (");
    strcat(finalname, name);
    strcat(finalname, ")");
  }
  return finalname;
}
