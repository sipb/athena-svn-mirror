
/*
 * m_backup.c -- construct a backup file
 *
 * $Id: m_backup.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


char *
m_backup (char *file)
{
    char *cp;
    static char buffer[BUFSIZ];

    if ((cp = r1bindex(file, '/')) == file)
	snprintf(buffer, sizeof(buffer), "%s%s",
		BACKUP_PREFIX, cp);
    else
	snprintf(buffer, sizeof(buffer), "%.*s%s%s", cp - file, file,
		BACKUP_PREFIX, cp);

    unlink(buffer);
    return buffer;
}
