/* m_fmsg.c - free a folder */
#ifndef	lint
static char ident[] = "@(#)$Id: m_fmsg.c,v 1.1.1.1 1996-10-07 07:13:51 ghudson Exp $";
#endif	/* lint */

#include "../h/mh.h"
#include <stdio.h>


void	m_fmsg (mp)
register struct msgs *mp;
{
    register int    i;

    if (mp == NULL)
	return;

    if (mp -> foldpath)
	free (mp -> foldpath);
#ifdef	MTR
    free ((char *) mp -> msgbase);
#endif	/* MTR */
    for (i = 0; mp -> msgattrs[i]; i++)
	free (mp -> msgattrs[i]);
    free ((char *) mp);
}
