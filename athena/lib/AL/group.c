/**********************************************************************
 *  group.c -- add user to group file
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <AL/AL.h>
#include <stdio.h>
#include <hesiod.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

/* ALcopySubstring("foo:bar", buf, (int)':') puts "foo" in buf
 * and returns a pointer to "bar".
 * ALcopySubstring("foo", buf, (int)':') puts "foo" in buf and
 * returns a pointer to the terminating '\0' of "foo".
 */

char *
ALcopySubstring(char *string, char buffer[], int delimiter)
{
  char *ptr1, *ptr2;

  for (ptr1= string, ptr2= buffer;
       *ptr1 != (char)delimiter && *ptr1 != '\0';
       ptr1++, ptr2++)
    *ptr2 = *ptr1;

  *ptr2='\0';
  if (*ptr1) ptr1++;		/* skip delimiter; don't skip '\0' */
  return (ptr1);
}

/* get a user's groups */

long
ALgetGroups(ALsession session)
{
  char **hesinfo, *ptr;
  int ncolons, ngroups, cnt;

  /* get group information from Hesiod */
  hesinfo = (char **)hes_resolve(ALpw_name(session), "grplist");
  if (!hesinfo || !*hesinfo) ALreturnError(session, ALerrNoGroupInfo,
					   ALpw_name(session));

  /* count groups */
  ncolons=0;
  for (ptr=hesinfo[0]; *ptr; ptr++)
    if (*ptr == ':') ncolons++;
  ngroups = (ncolons+1)/2;
  if (ngroups > NGROUPS_MAX-1) ngroups = NGROUPS_MAX-1; /* XXX why -1? */

  /* allocate space for groups */
  session->groups = (ALgroup) malloc((ngroups * sizeof(ALgroupStruct)));
  if (!session->groups) ALreturnError(session, (long)ENOMEM, "");

  /* put hesiod info into useful structure */
  ALngroups(session) = ngroups;
  ptr=hesinfo[0];
  for (cnt=0; cnt < ngroups; cnt++)
    {
      ptr = ALcopySubstring(ptr, ALgroupName(session, cnt), (int)':');
      ptr = ALcopySubstring(ptr, ALgroupId(session, cnt), (int)':');
      ALgroupAdded(session, cnt)=0;
    }

  return 0L;
}

long
ALmodifyGroupAdd(ALsession session, char groupline[])
{
  int idx;
  char buf[64], *ptr;

  /* get the name of the group */
  groupline = ALcopySubstring(groupline, buf, (int)':');

  /* see if the user is in this group */
  for (idx=0; idx < ALngroups(session); idx++)
    {
      if (!strcmp(buf, ALgroupName(session, idx)))
	{
	  /* skip password field */
	  if (!(groupline = strchr(groupline, (int)':')))
	    return 0L;		/* skip this line if malformed */
	  groupline++;

	  /* skip group ID */
	  if (!(groupline = strchr(groupline, (int)':')))
	    return 0L;		/* skip this line if malformed */
	  groupline++;

	  /* read usernames until we reach '\0' */
	  do
	    {
	      /* copy username into buf */
	      groupline = ALcopySubstring(groupline, buf, ',');

	      /* remove newline from last username on line */
	      ptr = buf + strlen(buf) - 1;
	      if (*ptr == '\n') *ptr = '\0';

	      /* if user is already in group, we're done */
	      if (!strcmp(ALpw_name(session), buf))
		{
		  ALgroupAdded(session, idx)=1;
		  return 0L;
		}
	    } while (*groupline != '\0');

	  /* append username to line */
	  strcat(strcat(strcpy(groupline-1, ","),
			ALpw_name(session)), "\n");
	  ALgroupAdded(session, idx)=1;
	  return 0L;
	}
    }
  return 0L;
}

/* append groups that haven't already been added */

long
ALappendGroups(ALsession session, int fd)
{
  int idx;
  char groupline[4096];

  for (idx=0; idx < ALngroups(session); idx++)
    if (!ALgroupAdded(session, idx))
      {
	sprintf(groupline, "%s:*:%s:%s\n", ALgroupName(session, idx),
		ALgroupId(session, idx), ALpw_name(session));
	if (write(fd, groupline, strlen(groupline)) < 0)
	  return((long) errno);
      }

  return 0L;
}

/* Add groups to group file */

long
ALaddToGroupsFile(ALsession session)
{
  return(ALmodifyLinesOfFile(session, "/etc/group", "/etc/gtmp",
			     ALmodifyGroupAdd, ALappendGroups));
}
