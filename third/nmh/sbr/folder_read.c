
/*
 * folder_read.c -- initialize folder structure and read folder
 *
 * $Id: folder_read.c,v 1.1.1.1 1999-02-07 18:14:08 danw Exp $
 */

#include <h/mh.h>

/* We allocate the `mi' array 1024 elements at a time */
#define	NUMMSGS  1024

/*
 * 1) Create the folder/message structure
 * 2) Read the directory (folder) and temporarily
 *    record the numbers of the messages we have seen.
 * 3) Then allocate the array for message attributes and
 *    set the initial flags for all messages we've seen.
 * 4) Read and initialize the sequence information.
 */

struct msgs *
folder_read (char *name)
{
    int msgnum, prefix_len, len, *mi;
    struct msgs *mp;
    struct stat st;
    struct dirent *dp;
    DIR *dd;

    name = m_mailpath (name);
    if (!(dd = opendir (name))) {
	free (name);
	return NULL;
    }

    if (stat (name, &st) == -1) {
	free (name);
	return NULL;
    }

    /* Allocate the main structure for folder information */
    if (!(mp = (struct msgs *) malloc ((size_t) sizeof(*mp))))
	adios (NULL, "unable to allocate folder storage");

    clear_folder_flags (mp);
    mp->foldpath = name;
    mp->lowmsg = 0;
    mp->hghmsg = 0;
    mp->curmsg = 0;
    mp->lowsel = 0;
    mp->hghsel = 0;
    mp->numsel = 0;
    mp->nummsg = 0;

    if (access (name, W_OK) == -1 || st.st_uid != getuid())
	set_readonly (mp);
    prefix_len = strlen(BACKUP_PREFIX);

    /*
     * Allocate a temporary place to record the
     * name of the messages in this folder.
     */
    len = NUMMSGS;
    if (!(mi = (int *) malloc ((size_t) (len * sizeof(*mi)))))
	adios (NULL, "unable to allocate storage");

    while ((dp = readdir (dd))) {
	if ((msgnum = m_atoi (dp->d_name))) {
	    /*
	     * Check if we need to allocate more
	     * temporary elements for message names.
	     */
	    if (mp->nummsg >= len) {
		len += NUMMSGS;
		if (!(mi = (int *) realloc (mi,
			(size_t) (len * sizeof(*mi))))) {
		    adios (NULL, "unable to allocate storage");
		}
	    }

	    /* Check if this is the first message we've seen */
	    if (mp->nummsg == 0) {
		mp->lowmsg = msgnum;
		mp->hghmsg = msgnum;
	    } else {
		/* Check if this is it the highest or lowest we've seen? */
		if (msgnum < mp->lowmsg)
		   mp->lowmsg = msgnum;
		if (msgnum > mp->hghmsg)
		   mp->hghmsg = msgnum;
	    }

	    /*
	     * Now increment count, and record message
	     * number in a temporary place for now.
	     */
	    mi[mp->nummsg++] = msgnum;

	} else {
	    switch (dp->d_name[0]) {
		case '.': 
		case ',': 
#ifdef MHE
		case '+': 
#endif /* MHE */
		    continue;

		default: 
		    /* skip any files beginning with backup prefix */
		    if (!strncmp (dp->d_name, BACKUP_PREFIX, prefix_len))
			continue;

		    /* skip the LINK file */
		    if (!strcmp (dp->d_name, LINK))
			continue;

		    /* indicate that there are other files in folder */
		    set_other_files (mp);
		    continue;
	    }
	}
    }

    closedir (dd);
    mp->lowoff = max (mp->lowmsg, 1);

    /* Go ahead and allocate space for 100 additional messages. */
    mp->hghoff = mp->hghmsg + 100;

    /* for testing, allocate minimal necessary space */
    /* mp->hghoff = max (mp->hghmsg, 1); */

    /*
     * Allocate space for status of each message.
     */
    if (!(mp->msgstats = malloc (MSGSTATSIZE(mp, mp->lowoff, mp->hghoff))))
	adios (NULL, "unable to allocate storage for msgstats");

    /*
     * Clear all the flag bits for all the message
     * status entries we just allocated.
     */
    for (msgnum = mp->lowoff; msgnum <= mp->hghoff; msgnum++)
	clear_msg_flags (mp, msgnum);

    /*
     * Scan through the array of messages we've seen and
     * setup the initial flags for those messages in the
     * newly allocated mp->msgstats area.
     */
    for (msgnum = 0; msgnum < mp->nummsg; msgnum++)
	set_exists (mp, mi[msgnum]);

    free (mi);		/* We don't need this anymore    */

    /*
     * Read and initialize the sequence information.
     */
    seq_read (mp);

    return mp;
}