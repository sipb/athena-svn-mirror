/*
 * buildhash.c - make a hash table for ispell
 *
 * Pace Willisson, 1983
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "ispell.h"
#include "config.h"

#define NSTAT 100
struct stat dstat, cstat;

int numwords, hashsize;

char *malloc();

struct dent *hashtbl;

char *Dfile;
char *Hfile;

char Cfile[MAXPATHLEN];
char Sfile[MAXPATHLEN];

main (argc,argv)
int argc;
char **argv;
{
	FILE *countf;
	FILE *statf;
	int stats[NSTAT];
	int i;

	if (argc > 1) {
		++argv;
		Dfile = *argv;
		if (argc > 2) {
			++argv;
			Hfile = *argv;
		}
		else
			Hfile = DEFHASH;
	}
	else {
		Dfile = DEFDICT;
		Hfile = DEFHASH;
	}

	(void) sprintf(Cfile,"%s.cnt",Dfile);
	(void) sprintf(Sfile,"%s.stat",Dfile);

	if (stat (Dfile, &dstat) < 0) {
		fprintf (stderr, "No dictionary (%s)\n", Dfile);
		exit (1);
	}

	if (stat (Cfile, &cstat) < 0 || dstat.st_mtime > cstat.st_mtime)
		newcount ();

	if ((countf = fopen (Cfile, "r")) == NULL) {
		fprintf (stderr, "No count file\n");
		exit (1);
	}
	numwords = 0;
	(void) fscanf (countf, "%d", &numwords);
	(void) fclose (countf);
	if (numwords == 0) {
		fprintf (stderr, "Bad count file\n");
		exit (1);
	}
	hashsize = numwords;
	readdict ();

	if ((statf = fopen (Sfile, "w")) == NULL) {
		fprintf (stderr, "Can't create %s\n", Sfile);
		exit (1);
	}

	for (i = 0; i < NSTAT; i++)
		stats[i] = 0;
	for (i = 0; i < hashsize; i++) {
		struct dent *dp;
		int j;
		if (hashtbl[i].used == 0) {
			stats[0]++;
		} else {
			for (j = 1, dp = &hashtbl[i]; dp->next != NULL; j++, dp = dp->next)
				;
			if (j >= NSTAT)
				j = NSTAT - 1;
			stats[j]++;
		}
	}
	for (i = 0; i < NSTAT; i++)
		fprintf (statf, "%d: %d\n", i, stats[i]);
	(void) fclose (statf);

	filltable ();

	output ();
	exit(0);
}

output ()
{
	FILE *outfile;
	struct hashheader hashheader;
	int strptr, n, i;

	if ((outfile = fopen (Hfile, "w")) == NULL) {
		fprintf (stderr, "can't create %s\n",Hfile);
		return;
	}
	hashheader.magic = MAGIC;
	hashheader.stringsize = 0;
	hashheader.tblsize = hashsize;
	(void) fwrite (&hashheader, sizeof hashheader, 1, outfile);
	strptr = 0;
	for (i = 0; i < hashsize; i++) {
		n = strlen (hashtbl[i].word) + 1;
		(void) fwrite (hashtbl[i].word, n, 1, outfile);
		hashtbl[i].word = (char *)strptr;
		strptr += n;
	}
	for (i = 0; i < hashsize; i++) {
		if (hashtbl[i].next != 0) {
			int x;
			x = hashtbl[i].next - hashtbl;
			hashtbl[i].next = (struct dent *)x;
		} else {
			hashtbl[i].next = (struct dent *)-1;
		}
	}
	(void) fwrite (hashtbl, sizeof (struct dent), hashsize, outfile);
	hashheader.stringsize = strptr;
	rewind (outfile);
	(void) fwrite (&hashheader, sizeof hashheader, 1, outfile);
	(void) fclose (outfile);
}

filltable ()
{
	struct dent *freepointer, *nextword, *dp;
	int i;

	for (freepointer = hashtbl; freepointer->used; freepointer++)
		;
	for (nextword = hashtbl, i = numwords; i != 0; nextword++, i--) {
		if (nextword->used == 0) {
			continue;
		}
		if (nextword->next == NULL) {
			continue;
		}
		if (nextword->next >= hashtbl && nextword->next < hashtbl + hashsize) {
			continue;
		}
		dp = nextword;
		while (dp->next) {
			if (freepointer > hashtbl + hashsize) {
				fprintf (stderr, "table overflow\n");
				getchar ();
				break;
			}
			*freepointer = *(dp->next);
			dp->next = freepointer;
			dp = freepointer;

			while (freepointer->used)
				freepointer++;
		}
	}
}


readdict ()
{
	struct dent d;
	char lbuf[100];
	FILE *dictf;
	int i;
	int h;
	char *p;

	if ((dictf = fopen (Dfile, "r")) == NULL) {
		fprintf (stderr, "Can't open dictionary\n");
		exit (1);
	}

	hashtbl = (struct dent *) calloc (numwords, sizeof (struct dent));
	if (hashtbl == NULL) {
		fprintf (stderr, "couldn't allocate hash table\n");
		exit (1);
	}

	i = 0;
	while (fgets (lbuf, sizeof lbuf, dictf) != NULL) {
		if ((i & 1023) == 0) {
			printf ("%d ", i);
			fflush (stdout);
		}
		i++;

		p = &lbuf [ strlen (lbuf) - 1 ];
		if (*p == '\n')
			*p = 0;

		if (makedent (lbuf, &d) < 0)
			continue;

		d.word = malloc (strlen (lbuf) + 1);
		if (d.word == NULL) {
			fprintf (stderr, "couldn't allocate space for word %s\n", lbuf);
			exit (1);
		}
		(void) strcpy (d.word, lbuf);

		h = hash (lbuf, strlen (lbuf), hashsize);

		if (hashtbl[h].used == 0) {
			hashtbl[h] = d;

		} else {
			struct dent *dp;

			dp = (struct dent *) malloc (sizeof (struct dent));
			if (dp == NULL) {
				fprintf (stderr, "couldn't allocate space for collision\n");
				exit (1);
			}
			*dp = d;
			dp->next = hashtbl[h].next;
			hashtbl[h].next = dp;
		}
	}
	printf ("\n");
}

/*
 * fill in the flags in d, and put a null after the word in s
 */

makedent (lbuf, d)
char *lbuf;
struct dent *d;
{
	char *p;

	d->next = NULL;
	d->used = 1;
	d->v_flag = 0;
	d->n_flag = 0;
	d->x_flag = 0;
	d->h_flag = 0;
	d->y_flag = 0;
	d->g_flag = 0;
	d->j_flag = 0;
	d->d_flag = 0;
	d->t_flag = 0;
	d->r_flag = 0;
	d->z_flag = 0;
	d->s_flag = 0;
	d->p_flag = 0;
	d->m_flag = 0;
	d->keep = 0;
	d->keep = 0;

	p = strchr (lbuf, '/');
	if (p != NULL)
		*p = 0;
	if (strlen (lbuf) > WORDLEN - 1) {
		printf ("%s: word too big\n");
		return (-1);
	}

	if (p == NULL)
		return (0);

	p++;
	while (*p != '\0'  &&  *p != '\n') {
		switch (*p) {
		case 'V': d->v_flag = 1; break;
		case 'N': d->n_flag = 1; break;
		case 'X': d->x_flag = 1; break;
		case 'H': d->h_flag = 1; break;
		case 'Y': d->y_flag = 1; break;
		case 'G': d->g_flag = 1; break;
		case 'J': d->j_flag = 1; break;
		case 'D': d->d_flag = 1; break;
		case 'T': d->t_flag = 1; break;
		case 'R': d->r_flag = 1; break;
		case 'Z': d->z_flag = 1; break;
		case 'S': d->s_flag = 1; break;
		case 'P': d->p_flag = 1; break;
		case 'M': d->m_flag = 1; break;
		case 0:
 			fprintf (stderr, "no flags on word %s\n", lbuf);
			continue;
		default:
			fprintf (stderr, "unknown flag %c word %s\n", 
					*p, lbuf);
			break;
		}
		p++;
		if (*p == '/')		/* Handle old-format dictionaries too */
			p++;
	}
	return (0);
}

newcount ()
{
	char buf[200];
	FILE *d;
	int i;

	fprintf (stderr, "Counting words in dictionary ...\n");

	if ((d = fopen (Dfile, "r")) == NULL) {
		fprintf (stderr, "Can't open dictionary\n");
		exit (1);
	}

	for (i = 0; fgets (buf, sizeof buf, d); )
		if ((++i & 1023) == 0) {
			printf ("%d ", i);
			fflush (stdout);
		}
	(void) fclose (d);
	printf ("\n%d words\n", i);
	if ((d = fopen (Cfile, "w")) == NULL) {
		fprintf (stderr, "can't create %s\n", Cfile);
		exit (1);
	}
	fprintf (d, "%d\n", i);
	(void) fclose (d);
}
