/*
 * good.c - see if a word or its root word
 * is in the dictionary.
 *
 * Pace Willisson, 1983
 */

#include <stdio.h>
#include <ctype.h>
#include "ispell.h"
#include "config.h"

struct dent *lookup();

static int wordok;

extern int cflag;

good (w)
register char *w;
{
	char nword[100];
	register char *p, *q;
	register n;

	for (p = w, q = nword; *p; p++, q++) {
		if (mylower (*p))
			*q = toupper (*p);
		else
			*q = *p;
	}
	*q = 0;

	rootword[0] = 0;

	if (cflag)
		printf ("%s\n", nword);
	else if (lookup (nword, q - nword, 1) != NULL) {
		return (1);
	}

	/* try stripping off suffixes */

	n = strlen (w);
	if (n == 1)
		return (1);

	if (n < 4)
		return 0;

	wordok = 0;

	/* this part from 'check.mid' */
	switch (q[-1]) {
	case 'D': d_ending (nword); break;	/* FOR "CREATED", "IMPLIED", "CROSSED" */
	case 'T': t_ending (nword); break;	/* FOR "LATEST", "DIRTIEST", "BOLDEST" */
	case 'R': r_ending (nword); break;	/* FOR "LATER", "DIRTIER", "BOLDER" */
	case 'G': g_ending (nword); break;	/* FOR "CREATING", "FIXING" */
	case 'H': h_ending (nword); break;	/* FOR "HUNDREDTH", "TWENTIETH" */
	case 'S': s_ending (nword); break;	/* FOR ALL SORTS OF THINGS ENDING IN "S" */
	case 'N': n_ending (nword); break;	/* "TIGHTEN", "CREATION", "MULIPLICATION" */
	case 'E': e_ending (nword); break;	/* FOR "CREATIVE", "PREVENTIVE" */
	case 'Y': y_ending (nword); break;	/* FOR "QUICKLY" */
	default:
		break;
	}
	
	if (wordok) {
		(void) strcpy (rootword, lastdent->word);
	}
	return (wordok);

}


g_ending (w)
char *w;
{
	char *p;
	struct dent *dent;

	p = w + strlen (w) - 3;	/* if the word ends in 'ing', then *p == 'i' */
	
	if (strcmp (p, "ING") != 0)
		return;

	*p = 'E';	/* change I to E, like in CREATING */
	*(p+1) = 0;

	if (strlen (w) < 2)
		return;

	if (cflag)
		printf ("%s/G\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL
	  &&  dent->g_flag) {
		wordok = 1;
		return;
	}


	*p = 0;

	if (strlen (w) < 2)
		return;

	if (p[-1] == 'E')
		return;	/* this stops CREATEING */

	if (cflag)
		printf ("%s/G\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL) {
		if (dent->g_flag)
			wordok = 1;
		return;
	}
	return;
}

d_ending (w)
char *w;
{
	char *p;
	struct dent *dent;

	p = w + strlen (w) - 2;

	if (strcmp (p, "ED") != 0)
		return;

	p[1] = 0;	/* kill 'D' */

	if (cflag)
		printf ("%s/D\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL) { /* eg CREATED */
		if (dent->d_flag)
			wordok = 1;
		return;
	}

	if (strlen (w) < 3)
		return;

	p[0] = 0;
	p--;

	/* ED is now completely gone */

	if (p[0] == 'I' && !vowel (p[-1])) {
		p[0] = 'Y';
		if (cflag)
			printf ("%s/D\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    &&  dent->d_flag) {
			wordok = 1;
			return;
		}
		p[0] = 'I';
	}

	if ((p[0] != 'E' && p[0] != 'Y') ||
	    (p[0] == 'Y' && vowel (p[-1]))) {
		if (cflag)
			printf ("%s/D\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL) {
			if (dent->d_flag)
				wordok = 1;
			return;
		}
	}
}

t_ending (w)
char *w;
{

	char *p;
	struct dent *dent;

	p = w + strlen (w) - 3;

	if (strcmp (p, "EST") != 0)
		return;

	p[1] = 0;	/* kill 'S' */

	if (cflag)
		printf ("%s/T\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL
	    &&  dent->t_flag) {
		wordok = 1;
		return;
	}

	if (strlen (w) < 3)
		return;

	p[0] = 0;
	p--;

	/* EST is now completely gone */

	if (p[0] == 'I' && !vowel (p[-1])) {
		p[0] = 'Y';
		if (cflag)
			printf ("%s/T\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    &&  dent->t_flag) {
			wordok = 1;
			return;
		}
		p[0] = 'I';
	}

	if ((p[0] != 'E' && p[0] != 'Y') ||
	    (p[0] == 'Y' && vowel (p[-1]))) {
		if (cflag)
			printf ("%s/T\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL) {
			if (dent->t_flag)
				wordok = 1;
			return;
		}
	}

}


r_ending (w)
char *w;
{
	char *p;
	struct dent *dent;

	p = w + strlen (w) - 2;

	if (strcmp (p, "ER") != 0)
		return;

	p[1] = 0;	/* kill 'R' */

	if (cflag)
		printf ("%s/R\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL
	    &&  dent->r_flag) {
		wordok = 1;
		return;
	}

	if (strlen (w) < 3)
		return;

	p[0] = 0;
	p--;

	/* ER is now completely gone */

	if (p[0] == 'I' && !vowel (p[-1])) {
		p[0] = 'Y';
		if (cflag)
			printf ("%s/R\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    &&  dent->r_flag) {
			wordok = 1;
			return;
		}
		p[0] = 'I';
	}

	if ((p[0] != 'E' && p[0] != 'Y') ||
	    (p[0] == 'Y' && vowel (p[-1]))) {
		if (cflag)
			printf ("%s/R\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL) {
			if (dent->r_flag)
				wordok = 1;
			return;
		}
	}

}

h_ending (w)
char *w;
{
	char *p;
	struct dent *dent;

	p = w + strlen (w) - 2;

	if (strcmp (p, "TH") != 0)
		return;

	*p = 0;

	p -= 2;

	if (p[1] != 'Y') {
		if (cflag)
			printf ("%s/H\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    &&  dent->h_flag)
			wordok = 1;
	}

	if (strcmp (p, "IE") != 0)
		return;

	p[0] = 'Y';
	p[1] = 0;

	if (cflag)
		printf ("%s/H\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL)
		if (dent->h_flag)
			wordok = 1;

}

/*
 * check for flags: X, J, Z, S, P, M
 *
 * X	-ions or -ications or -ens
 * J	-ings
 * Z	-ers or -iers
 * S	-ies or -es or -s
 * P	-iness or -ness
 * M	-'S
 */

s_ending (w)
char *w;
{
	char *p;
	struct dent *dent;

	p = w + strlen (w);

	p[-1] = 0;

	if (strchr ("SXZHY", p[-2]) == NULL || (p[-2] == 'Y' && vowel (p[-3]))) {
		if (cflag)
			printf ("%s/S\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    &&  dent->s_flag) {
			wordok = 1;
			return;
		}
	}


	switch (p[-2]) {	/* letter before S */
	case 'N':	/* X */
		if (strcmp (p-4, "ION") == 0) {
			p[-4] = 'E';
			p[-3] = 0;
			if (cflag)
				printf ("%s/X\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL
			  &&  dent->x_flag) {
				wordok = 1;
				return;
			}
		}
		if (strcmp (p-8, "ICATE") == 0) {
			p[-8] = 'Y';
			p[-7] = 0;
			if (cflag)
				printf ("%s/X\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL
			    && dent->x_flag)
				wordok = 1;
			return;
		}
		if (strcmp (p-3, "EN") == 0 && p[-4] != 'E' && p[-4] != 'Y') {
			p[-3] = 0;
			if (cflag)
				printf ("%s/X\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL
			    && dent->x_flag)
				wordok = 1;
			return;
		}
		return;
	case 'G':	/* J */
		if (strcmp (p-4, "ING") != 0)
			return;
		p[-4] = 'E';
		p[-3] = 0;
		if (cflag)
			printf ("%s/J\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    &&  dent->j_flag) {
			wordok = 1;
			return;
		}
		if (p[-5] == 'E')
			return;		/* This stops CREATEING */
		p[-4] = 0;
		if (cflag)
			printf ("%s/J\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    && dent->j_flag)
			wordok = 1;
		return;
	case 'R':	/* Z */
		if (strcmp (p-3, "ER") != 0)
			return;

		p[-2] = 0;
		if (cflag)
			printf ("%s/Z\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    &&  dent->z_flag) {
			wordok = 1;
			return;
		}
		if (p[-4] == 'I'  &&  !vowel (p[-5])) {
			p[-4] = 'Y';
			p[-3] = 0;
			if (cflag)
				printf ("%s/Z\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL
			    && dent->z_flag) {
				wordok = 1;
				return;
			}
			p[-4] = 'I';
		}
		if ((p[-4] != 'E' && p[-4] != 'Y') ||
		    (p[-4] == 'Y' && vowel (p[-5]))) {
			p[-3] = 0;
			if (cflag)
				printf ("%s/Z\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL
			  && dent->z_flag)
				wordok = 1;
		}
		return;
	case 'E': /* S (except simple adding of an S) */
		p[-2] = 0;	/* drop the ES */
		if (strchr ("SXZH", p[-3]) != NULL) {
			if (cflag)
				printf ("%s/S\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL) {
				if (dent->s_flag)
					wordok = 1;;
				return;
			}
		}
		if (p[-3] == 'I') {
			p[-3] = 'Y';
			if (cflag)
				printf ("%s/S\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL
			    && dent->s_flag)
				wordok = 1;
			return;
		}
		return;

	case 'S':	/* P */
		if (strcmp (p-4, "NES") != 0)
			return;

		p[-4] = 0;	/* kill 'N' */
		if (p[-5] != 'Y' || vowel (p[-6])) {
			if (cflag)
				printf ("%s/P\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL
			    &&  dent->p_flag) {
				wordok = 1;
				return;
			}
		}
		if (p[-5] == 'I') {
			p[-5] = 'Y';
			if (cflag)
				printf ("%s/P\n", w);
			else if ((dent = lookup (w, strlen (w), 1)) != NULL
			    && dent->p_flag)
				wordok = 1;
		}
		return;
	case '\'':	/* M */
		p[-2] = '\0';
		if (cflag)
			printf ("%s/M\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    &&  dent->m_flag)
			wordok = 1;
		return;
	}
}

/* only the N flag */
n_ending (w)
char *w;
{
	char *p;
	struct dent *dent;

	p = w + strlen (w);

	if (p[-2] == 'E') {
		if (p[-3] == 'E' || p[-3] == 'Y')
			return;
		p[-2] = 0;
		if (cflag)
			printf ("%s/N\n", w);
		else if ((dent = lookup (w, strlen (w), 1)) != NULL
		    && dent->n_flag)
			wordok = 1;
		return;
	}

	if (strcmp (p-3, "ION") != 0)
		return;

	p[-3] = 'E';
	p[-2] = 0;

	if (cflag)
		printf ("%s/N\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL) {
		if (dent->n_flag)
			wordok = 1;
		return;
	}

	if (strcmp (p-7, "ICATE") != 0)	/* check is really against "ICATION" */
		return;

	p[-7] = 'Y';
	p[-6] = 0;
	
	if (cflag)
		printf ("%s/N\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL && dent->n_flag)
		wordok = 1;
	return;
}

/* flags: v */
e_ending (w)
char *w;
{
	char *p;
	struct dent *dent;

	p = w + strlen (w);

	if (strcmp (p-3, "IVE") != 0)
		return;
	p[-3] = 'E';
	p[-2] = 0;

	if (cflag)
		printf ("%s/V\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL
	  &&  dent->v_flag) {
		wordok = 1;
		return;
	}

	if (p[-4] == 'E')
		return;

	p[-3] = 0;

	if (cflag)
		printf ("%s/V\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL && dent->v_flag)
		wordok = 1;
	return;
}

/* flags: y */
y_ending (w)
char *w;
{
	char *p;
	struct dent *dent;

	p = w + strlen (w);

	if (strcmp (p-2, "LY") != 0)
		return;

	p[-2] = 0;

	if (cflag)
		printf ("%s/Y\n", w);
	else if ((dent = lookup (w, strlen (w), 1)) != NULL && dent->y_flag)
		wordok = 1;
	return;
}

vowel (c)
char c;
{
	return (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U');
}
