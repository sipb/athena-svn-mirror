/*
 * tree.c - a hash style dictionary for user's personal words
 *
 * Pace Willisson, 1983
 * Hash support added by Geoff Kuenning, 1987
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include "ispell.h"
#include "config.h"

char *getenv();
struct dent *lookup();
char *upcase();

static int cantexpand = 0;	/* NZ if an expansion fails */
static struct dent *htab = NULL; /* Hash table for our stuff */
static int hsize = 0;		/* Space available in hash table */
static int hcount = 0;		/* Number of items in hash table */

/*
 * Hash table sizes.  Prime is probably a good idea, though in truth I
 * whipped the algorithm up on the spot rather than looking it up, so
 * who knows what's really best?  If we overflow the table, we just
 * use a double-and-add-1 algorithm.
 *
 * The strange pattern in the table is because this table is sometimes
 * used with huge dictionaries, and we want to get the table bigger fast.
 * 23003 just happens to be such that the original dict.191 will fill
 * the table to just under 70%.  31469 is similarly selected for dict.191
 * combined with /usr/dict/words.  The other numbers are on 10000-word
 * intervals starting at 30000.  (The table is still valid if MAXPCT
 * is changed, but the dictionary sizes will no longer fall on neat
 * boundaries).
 */
static int goodsizes[] = {
	53, 223, 907,
#if ((BIG_DICT * 100) / MAXPCT) <= 23003
	23003,				/* ~16000 words */
#endif
#if ((BIG_DICT * 100) / MAXPCT) <= 31469
	31469,				/* ~22000 words */
#endif
#if ((BIG_DICT * 100) / MAXPCT) <= 42859
	42859,				/* ~30000 words */
#endif
#if ((BIG_DICT * 100) / MAXPCT) <= 57143
	57143,				/* ~40000 words */
#endif
	71429				/* ~50000 words */
};

struct dent *treeinsert();
struct dent *tinsert();
struct dent *treelookup();

static char personaldict[MAXPATHLEN];
static FILE *dictf;
static newwords = 0;


extern struct dent *hashtbl;
extern int hashsize;

treeinit (p)
char *p;
{
	char *h;
	char *orig;
	char buf[BUFSIZ];
	struct dent *dp;

	/*
	** if p exists and begins with '/' we don't really need HOME,
	** but it's not very likely that HOME isn't set anyway.
	*/
	orig = p;
	if (p == NULL)
		p = getenv (PDICTVAR);
	if ((h = getenv ("HOME")) == NULL)
		return(-1);

	if (p == NULL)
		(void) sprintf(personaldict,"%s/%s",h,DEFPDICT);
	else {
		if (*p == '/')
			(void) strcpy(personaldict,p);
		else {
			/*
			** The user gave us a relative pathname.  How we
			** interpret it depends on how it was given:
			**
			** -p switch:  as-is first, then $HOME/name
			** PDICTVAR:   $HOME/name first, then as-is
			**/
			if (orig == NULL)
				(void) sprintf (personaldict, "%s/%s", h, p);
			else			/* -p switch */
				(void) strcpy (personaldict, p);
		}
	}

	if ((dictf = fopen (personaldict, "r")) == NULL) {
		/* The file doesn't exist. */
		if (p != NULL) {
			/* If pathname is relative, try another place */
			if (*p != '/') {
				if (orig == NULL)
					(void) strcpy (personaldict, p);
				else			/* -p switch */
					(void) sprintf (personaldict, "%s/%s", h, p);
				dictf = fopen (personaldict, "r");
			}
			if (dictf == NULL) {
				(void) fprintf (stderr, "Couldn't open ");
				perror (p);
				if (*p != '/') {
					/*
					** Restore the preferred default, so
					** that output will go th the right
					** place.
					*/
					if (orig == NULL)
						(void) sprintf (personaldict,
						  "%s/%s", h, p);
					else			/* -p switch */
						(void) strcpy (personaldict, p);
				}
			}
		}
		/* If the name wasn't specified explicitly, we don't object */
		return(0);
	}

	while (fgets (buf, sizeof buf, dictf) != NULL) {
		int len = strlen (buf) - 1;

		if (buf [ len ] == '\n')
			buf [ len-- ] = '\0';
		if ((h = strchr (buf, '/')) != NULL)
			*h++ = '\0';
		dp = treeinsert (buf, 1);
		if (h != NULL) {
			while (*h != '\0'  &&  *h != '\n') {
				switch (*h++) {
				case 'D':
				case 'd':
					dp->d_flag = 1;
					break;
				case 'G':
				case 'g':
					dp->g_flag = 1;
					break;
				case 'H':
				case 'h':
					dp->h_flag = 1;
					break;
				case 'J':
				case 'j':
					dp->j_flag = 1;
					break;
				case 'M':
				case 'm':
					dp->m_flag = 1;
					break;
				case 'N':
				case 'n':
					dp->n_flag = 1;
					break;
				case 'P':
				case 'p':
					dp->p_flag = 1;
					break;
				case 'R':
				case 'r':
					dp->r_flag = 1;
					break;
				case 'S':
				case 's':
					dp->s_flag = 1;
					break;
				case 'T':
				case 't':
					dp->t_flag = 1;
					break;
				case 'V':
				case 'v':
					dp->v_flag = 1;
					break;
				case 'X':
				case 'x':
					dp->x_flag = 1;
					break;
				case 'Y':
				case 'y':
					dp->y_flag = 1;
					break;
				case 'Z':
				case 'z':
					dp->z_flag = 1;
					break;
				default:
					fprintf (stderr,
					  "Illegal flag in personal dictionary - %c (word %s)\n",
					  h[-1], buf);
					break;
				}
				/* Accept old-format dicts with extra slashes */
				if (*h == '/')
					h++;
			}
		}
	}

	(void) fclose (dictf);

	newwords = 0;

	if (!lflag && !aflag && access (personaldict, 2) < 0)
		printf ("Warning: Cannot update personal dictionary (%s)\r\n", personaldict);
}

treeprint ()
{
	register int i;
	register struct dent *dp;
	register struct dent *cp;

	printf ("(");
	for (i = 0;  i < hsize;  i++) {
		dp = &htab[i];
		if (dp->used) {
			for (cp = dp;  cp != NULL;  cp = cp->next)
				printf ("%s ", cp->word);
		}
	}
	printf (")");
}

struct dent *
treeinsert (word, keep)
char *word;
{
	register int i;
	struct dent *dp;
	struct dent *olddp;
	struct dent *oldhtab;
	int oldhsize;
	char nword[BUFSIZ];

	(void) strcpy (nword, word);
	(void) upcase (nword);
	if ((dp = lookup (nword, strlen (nword), 0)) != NULL) {
		if (keep)
			dp->keep = 1;
		return dp;
	}
	/*
	 * Expand hash table when it is MAXPCT % full.
	 */
	if (!cantexpand  &&  (hcount * 100) / MAXPCT >= hsize) {
		oldhsize = hsize;
		oldhtab = htab;
		for (i = 0;  i < sizeof goodsizes / sizeof (goodsizes[0]);  i++)
			if (goodsizes[i] > hsize)
				break;
		if (i >= sizeof goodsizes / sizeof goodsizes[0])
			hsize += hsize + 1;
		else
			hsize = goodsizes[i];
		htab = (struct dent *) calloc (hsize, sizeof (struct dent));
		if (htab == NULL) {
			(void) fprintf (stderr,
			    "Ran out of space for personal dictionary\n");
			/*
			 * Try to continue anyway, since our overflow
			 * algorithm can handle an overfull (100%+) table,
			 * and the malloc very likely failed because we
			 * already have such a huge table, so small mallocs
			 * for overflow entries will still work.
			 */
			if (oldhtab == NULL)
				exit (1);	/* No old table, can't go on */
			(void) fprintf (stderr,
			    "Continuing anyway (with reduced performance).\n");
			cantexpand = 1;		/* Suppress further messages */
			hsize = oldhsize;	/* Put this back how the were */
			htab = oldhtab;		/* ... */
			newwords = 1;		/* And pretend it worked */
			return tinsert (nword, (struct dent *) NULL, keep);
		}
		/*
		 * Re-insert old entries into new table
		 */
		for (i = 0;  i < oldhsize;  i++) {
			dp = &oldhtab[i];
			if (oldhtab[i].used) {
				(void) tinsert ((char *) NULL, dp, 0);
				dp = dp->next;
				while (dp != NULL) {
					(void) tinsert ((char *) NULL, dp, 0);
					olddp = dp;
					dp = dp->next;
					free ((char *) olddp);
				}
			}
		}
		if (oldhtab != NULL)
			free ((char *) oldhtab);
	}
	newwords |= keep;
	return tinsert (nword, (struct dent *) NULL, keep);
}

static
struct dent *
tinsert (word, proto, keep)
char *word;			/* One of word/proto must be null */
struct dent *proto;
{
	int hcode;
	register struct dent *hp; /* Next trial entry in hash table */
	struct dent *php;	/* Previous value of hp, for chaining */

	if (word == NULL)
		word = proto->word;
	hcode = hash (word, strlen (word), hsize);
	php = NULL;
	hp = &htab[hcode];
	if (hp->used) {
		while (hp != NULL) {
			if (strcmp (word, hp->word) == 0) {
				if (keep)
					hp->keep = 1;
				return hp;
			}
			php = hp;
			hp = hp->next;
		}
		hp = (struct dent *) calloc (1, sizeof (struct dent));
		if (hp == NULL) {
			(void) fprintf (stderr,
			    "Ran out of space for personal dictionary\n");
			exit (1);
		}
	}
	if (proto != NULL) {
		*hp = *proto;
		if (php != NULL)
			php->next = hp;
		hp->next = NULL;
		return &htab[hcode];
	} else {
		if (php != NULL)
			php->next = hp;
		hp->word = (char *) malloc (strlen (word) + 1);
		if (hp->word == NULL) {
			(void) fprintf (stderr,
			    "Ran out of space for personal dictionary\n");
			exit (1);
		}
		hp->used = 1;
		hp->next = NULL;
		hp->d_flag = 0;
		hp->g_flag = 0;
		hp->h_flag = 0;
		hp->j_flag = 0;
		hp->m_flag = 0;
		hp->n_flag = 0;
		hp->p_flag = 0;
		hp->r_flag = 0;
		hp->s_flag = 0;
		hp->t_flag = 0;
		hp->v_flag = 0;
		hp->x_flag = 0;
		hp->y_flag = 0;
		hp->z_flag = 0;
		(void) strcpy (hp->word, word);
		hp->keep = keep;
		hcount++;
		return (hp);
	}
}

struct dent *
treelookup (word)
char *word;
{
	int hcode;
	register struct dent *hp;
	char nword[BUFSIZ];

	if (hsize <= 0)
		return NULL;
	(void) strcpy (nword, word);
	hcode = hash (nword, strlen (nword), hsize);
	hp = &htab[hcode];
	while (hp != NULL  &&  hp->used) {
		if (strcmp (nword, hp->word) == 0)
			break;
		hp = hp->next;
	}
	if (hp != NULL  &&  hp->used)
		return hp;
	else
		return NULL;
}

treeoutput ()
{
	if (newwords == 0)
		return(0);

	if ((dictf = fopen (personaldict, "w")) == NULL) {
		fprintf (stderr, "Can't create %s\r\n", personaldict);
		return(-1);
	}

	toutput1 ();
	newwords = 0;

	(void) fclose (dictf);
}

static
toutput1 ()
{
	register struct dent *cent;	/* Current entry */
	register struct dent *lent;	/* Linked entry */

	for (cent = htab;  cent - htab < hsize;  cent++) {
		for (lent = cent;  lent != NULL;  lent = lent->next) {
			if (lent->used  &&  lent->keep)
				toutput2 (lent);
		}
	}
	for (cent = hashtbl, lent = hashtbl + hashsize;
	    cent < lent;
	    cent++) {
		if (cent->used  &&  cent->keep)
			toutput2 (cent);
	}
}

static int hasslash;

static
toutput2 (cent)
register struct dent *cent;
{

	hasslash = 0;
	fprintf (dictf, "%s", cent->word);
	if (cent->d_flag)
		flagout ('D');
	if (cent->g_flag)
		flagout ('G');
	if (cent->h_flag)
		flagout ('H');
	if (cent->j_flag)
		flagout ('J');
	if (cent->m_flag)
		flagout ('M');
	if (cent->n_flag)
		flagout ('N');
	if (cent->p_flag)
		flagout ('P');
	if (cent->r_flag)
		flagout ('R');
	if (cent->s_flag)
		flagout ('S');
	if (cent->t_flag)
		flagout ('T');
	if (cent->v_flag)
		flagout ('V');
	if (cent->x_flag)
		flagout ('X');
	if (cent->y_flag)
		flagout ('Y');
	if (cent->z_flag)
		flagout ('Z');
	fprintf (dictf, "\n");
}

static
flagout (flag)
{
	if (!hasslash)
		(void) putc ('/', dictf);
	hasslash = 1;
	(void) putc (flag, dictf);
}

char *
upcase (s)
register char *s;
{
	register char *os = s;

	while (*s) {
		if (mylower (*s))
			*s = toupper (*s);
		s++;
	}
	return (os);
}
