
/*
 * pick.c -- search for messages by content
 *
 * $Id: pick.c,v 1.1.1.1 1999-02-07 18:14:15 danw Exp $
 */

#include <h/mh.h>
#include <zotnet/tws/tws.h>
#include <h/picksbr.h>

/*
 * We allocate space for messages (msgs array)
 * this number of elements at a time.
 */
#define MAXMSGS  256


static struct swit switches[] = {
#define	ANDSW                   0
    { "and", 0 },
#define	ORSW                    1
    { "or", 0 },
#define	NOTSW                   2
    { "not", 0 },
#define	LBRSW                   3
    { "lbrace", 0 },
#define	RBRSW                   4
    { "rbrace", 0 },
#define	CCSW                    5
    { "cc  pattern", 0 },
#define	DATESW                  6
    { "date  pattern", 0 },
#define	FROMSW                  7
    { "from  pattern", 0 },
#define	SRCHSW                  8
    { "search  pattern", 0 },
#define	SUBJSW                  9
    { "subject  pattern", 0 },
#define	TOSW                   10
    { "to  pattern", 0 },
#define	OTHRSW                 11
    { "-othercomponent  pattern", 0 },
#define	AFTRSW                 12
    { "after date", 0 },
#define	BEFRSW                 13
    { "before date", 0 },
#define	DATFDSW                14
    { "datefield field", 5 },
#define	SEQSW                  15
    { "sequence name", 0 },
#define	PUBLSW                 16
    { "public", 0 },
#define	NPUBLSW                17
    { "nopublic", 0 },
#define	ZEROSW                 18
    { "zero", 0 },
#define	NZEROSW                19
    { "nozero", 0 },
#define	LISTSW                 20
    { "list", 0 },
#define	NLISTSW                21
    { "nolist", 0 },
#define VERSIONSW              22
    { "version", 0 },
#define	HELPSW                 23
    { "help", 4 },
    { NULL, 0 }
};

static int listsw = 0;


int
main (int argc, char **argv)
{
    int publicsw = -1, zerosw = 1, seqp = 0, vecp = 0;
    int nummsgs, maxmsgs, lo, hi, msgnum;
    char *maildir, *folder = NULL, buf[100];
    char *cp, **argp, **arguments;
    char **msgs, *seqs[NUMATTRS + 1], *vec[MAXARGS];
    struct msgs *mp;
    register FILE *fp;

#ifdef LOCALE
    setlocale(LC_ALL, "");
#endif
    invo_name = r1bindex (argv[0], '/');

    /* read user profile/context */
    context_read();

    arguments = getarguments (invo_name, argc, argv, 1);
    argp = arguments;

    /*
     * Allocate the initial space to record message
     * names, ranges, and sequences.
     */
    nummsgs = 0;
    maxmsgs = MAXMSGS;
    if (!(msgs = (char **) malloc ((size_t) (maxmsgs * sizeof(*msgs)))))
	adios (NULL, "unable to allocate storage");

    while ((cp = *argp++)) {
	if (*cp == '-') {
	    if (*++cp == '-') {
		vec[vecp++] = --cp;
		goto pattern;
	    }
	    switch (smatch (cp, switches)) {
	    case AMBIGSW: 
		ambigsw (cp, switches);
		done (1);
	    case UNKWNSW: 
		adios (NULL, "-%s unknown", cp);

	    case HELPSW: 
		snprintf (buf, sizeof(buf), "%s [+folder] [msgs] [switches]",
			  invo_name);
		print_help (buf, switches, 1);
		listsw = 0;	/* HACK */
		done (1);
	    case VERSIONSW:
		print_version(invo_name);
		done (1);

	    case CCSW: 
	    case DATESW: 
	    case FROMSW: 
	    case SUBJSW: 
	    case TOSW: 
	    case DATFDSW: 
	    case AFTRSW: 
	    case BEFRSW: 
	    case SRCHSW: 
		vec[vecp++] = --cp;
	    pattern:
		if (!(cp = *argp++))/* allow -xyz arguments */
		    adios (NULL, "missing argument to %s", argp[-2]);
		vec[vecp++] = cp;
		continue;
	    case OTHRSW: 
		adios (NULL, "internal error!");

	    case ANDSW:
	    case ORSW:
	    case NOTSW:
	    case LBRSW:
	    case RBRSW:
		vec[vecp++] = --cp;
		continue;

	    case SEQSW: 
		if (!(cp = *argp++) || *cp == '-')
		    adios (NULL, "missing argument to %s", argp[-2]);

		/* check if too many sequences specified */
		if (seqp >= NUMATTRS)
		    adios (NULL, "too many sequences (more than %d) specified", NUMATTRS);
		seqs[seqp++] = cp;
		listsw = 0;
		continue;
	    case PUBLSW: 
		publicsw = 1;
		continue;
	    case NPUBLSW: 
		publicsw = 0;
		continue;
	    case ZEROSW: 
		zerosw++;
		continue;
	    case NZEROSW: 
		zerosw = 0;
		continue;

	    case LISTSW: 
		listsw++;
		continue;
	    case NLISTSW: 
		listsw = 0;
		continue;
	    }
	}
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULL, "only one folder at a time!");
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	} else {
	    /*
	     * Check if we need to allocate more space
	     * for message name/ranges/sequences.
	     */
	    if (nummsgs >= maxmsgs) {
		maxmsgs += MAXMSGS;
		if (!(msgs = (char **) realloc (msgs,
						(size_t) (maxmsgs * sizeof(*msgs)))))
		    adios (NULL, "unable to reallocate msgs storage");
	    }
	    msgs[nummsgs++] = cp;
	}
    }
    vec[vecp] = NULL;

    if (!context_find ("path"))
	free (path ("./", TFOLDER));

    /*
     * If we didn't specify which messages to search,
     * then search the whole folder.
     */
    if (!nummsgs)
	msgs[nummsgs++] = "all";

    if (!folder)
	folder = getfolder (1);
    maildir = m_maildir (folder);

    if (chdir (maildir) == NOTOK)
	adios (maildir, "unable to change directory to");

    /* read folder and create message structure */
    if (!(mp = folder_read (folder)))
	adios (NULL, "unable to read folder %s", folder);

    /* check for empty folder */
    if (mp->nummsg == 0)
	adios (NULL, "no messages in %s", folder);

    /* parse all the message ranges/sequences and set SELECTED */
    for (msgnum = 0; msgnum < nummsgs; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    done (1);
    seq_setprev (mp);	/* set the previous-sequence */

    /*
     * If we aren't saving the results to a sequence,
     * we need to list the results.
     */
    if (seqp == 0)
	listsw++;

    if (publicsw == 1 && is_readonly(mp))
	adios (NULL, "folder %s is read-only, so -public not allowed", folder);

    if (!pcompile (vec, NULL))
	done (1);

    lo = mp->lowsel;
    hi = mp->hghsel;

    /*
     * Scan through all the SELECTED messages and check for a
     * match.  If the message does not match, then unselect it.
     */
    for (msgnum = mp->lowsel; msgnum <= mp->hghsel; msgnum++) {
	if (is_selected (mp, msgnum)) {
	    if ((fp = fopen (cp = m_name (msgnum), "r")) == NULL)
		admonish (cp, "unable to read message");
	    if (fp && pmatches (fp, msgnum, 0L, 0L)) {
		if (msgnum < lo)
		    lo = msgnum;
		if (msgnum > hi)
		    hi = msgnum;
	    } else {
		/* if it doesn't match, then unselect it */
		unset_selected (mp, msgnum);
		mp->numsel--;
	    }
	    if (fp)
		fclose (fp);
	}
    }

    mp->lowsel = lo;
    mp->hghsel = hi;

    if (mp->numsel <= 0)
	adios (NULL, "no messages match specification");

    seqs[seqp] = NULL;

    /*
     * Add the matching messages to sequences
     */
    for (seqp = 0; seqs[seqp]; seqp++)
	if (!seq_addsel (mp, seqs[seqp], publicsw, zerosw))
	    done (1);

    /*
     * Print the name of all the matches
     */
    if (listsw) {
	for (msgnum = mp->lowsel; msgnum <= mp->hghsel; msgnum++)
	    if (is_selected (mp, msgnum))
		printf ("%s\n", m_name (msgnum));
    } else {
	printf ("%d hit%s\n", mp->numsel, mp->numsel == 1 ? "" : "s");
    }

    context_replace (pfolder, folder);	/* update current folder         */
    seq_save (mp);			/* synchronize message sequences */
    context_save ();			/* save the context file         */
    folder_free (mp);			/* free folder/message structure */
    done (0);
}


void
done (int status)
{
    if (listsw && status && !isatty (fileno (stdout)))
	printf ("0\n");
    exit (status);
}