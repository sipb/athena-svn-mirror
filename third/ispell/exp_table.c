#ifndef lint
static char Rcs_Id[] =
    "$Id: exp_table.c,v 1.1.1.1 2007-02-01 19:50:38 ghudson Exp $";
#endif

/*
 * Note: this file was written by Edward Avis.  Thus, it is not
 * distributed under the same license as the rest of ispell.
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/06/11 22:43:53  geoff
 * Don't try to malloc zero elements during initialization.
 *
 * Revision 1.3  2005/04/14 15:19:37  geoff
 * Get rid of a compiler warning.
 *
 * Revision 1.2  2005/04/14 14:38:23  geoff
 * Add RCS keywords.  Reformat to be more consistent with ispell style.
 * This may also include some bug fixes; I unfortunately don't really
 * remember.
 *
 * Revision 1.1  2002/07/02 00:06:50  geoff
 * Initial revision
 */

#include "config.h"
#include "ispell.h"
#include "msgs.h"
#include "proto.h"
#include "exp_table.h"

void exp_table_init (e, orig_word)
    struct exp_table *
			e;
    ichar_t *		orig_word;
    {

    e->size = 0;
    e->max_size = 1;
    e->exps = malloc (e->max_size * sizeof (*e->exps));
    e->flags = malloc (e->max_size * sizeof (*e->flags) * MASKSIZE);
    e->orig_word = orig_word;
    }

const ichar_t * get_orig_word (e)
    const struct exp_table *
			e;
    {

    return e->orig_word;
    }

const char * get_expansion (e, i)
    const struct exp_table *
			e;
    int			i;
    {

    return e->exps[i];
    }

MASKTYPE get_flags (e, i)
    const struct exp_table *
			e;
    int			i;
    {

    return e->flags[i * MASKSIZE];
    }

int num_expansions (e)
    const struct exp_table *
			e;
    {
    return e->size;
    }

int add_expansion_copy (e, s, flags)
    struct exp_table *	e;
    const  char *	s;
    MASKTYPE		flags[];
    {
    char *		copy;
    int			copy_size;
    int			i;
    
    /* 
     * Check not already there.
     */
    for (i = 0; i < e->size; i++)
	{
	if (strcmp (e->exps[i], s) == 0)
	    return 0;
	}

    /*
     * Grow the pointer table if necessary.
     */
    if (e->size == e->max_size)
	{
	e->max_size *= 2;
	e->exps = realloc(e->exps, e->max_size * sizeof (*e->exps));
	e->flags =
	  realloc(e->flags, e->max_size * sizeof (*e->flags) * MASKSIZE);

	if (e->exps == NULL  ||  e->flags == NULL)
	    {
	    (void) fprintf (stderr, TGOOD_C_NO_SPACE);
	    exit (1);
	    }
	}

    copy_size = strlen (s) + 1;
    copy = malloc (copy_size * sizeof copy[0]);
    if (copy == NULL)
	{
	(void) fprintf (stderr, TGOOD_C_NO_SPACE);
	exit (1);
	}

    strncpy (copy, s, copy_size);
    e->exps[e->size] = copy;
    BCOPY ((char *) &flags[0], &e->flags[e->size * MASKSIZE],
      MASKSIZE * sizeof flags[0]);
    ++e->size;
    return 1;
    }
    
struct exp_table * exp_table_empty (e)
    struct exp_table *	e;
    {
    int			i;

    for (i = 0; i < e->size; i++)
	free (e->exps[i]);
    e->size = 0;
    return e;
    }

void exp_table_dump (e)
    const  struct exp_table *
			e;
    {
    int			i;

    /*
     * BUGS: assumes 32-bit masks; assumes MASKSIZE = 1
     */

    fprintf(stderr, "original word: %s\n", ichartosstr(e->orig_word, 0));
    fprintf(stderr, "%d expansions\n", e->size);
    for (i = 0; i < e->size; i++)
	fprintf(stderr, "flags %lx generate expansion %s\n",
		(long) e->flags[i * MASKSIZE], e->exps[i]);
    }
