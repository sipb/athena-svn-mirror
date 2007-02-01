#ifndef EXP_TABLE_H_INCLUDED
#define EXP_TABLE_H_INCLUDED
/*
 * $Id: exp_table.h,v 1.1.1.1 2007-02-01 19:50:09 ghudson Exp $
 */

/*
 * Note: this header file was written by Edward Avis.  Thus, it is not
 * distributed under the same license as the rest of ispell.
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/04/26 22:40:07  geoff
 * Add double-inclusion protection.
 *
 * Revision 1.2  2005/04/14 15:19:37  geoff
 * Reformat to be more consistent with ispell style.
 *
 * Revision 1.1  2002/07/02 00:06:50  geoff
 * Initial revision
 *
 */

/* 
 * Provides the exp_table type, which stores the expansions of a word.
 * Use it like this:
 * 
 *     int i;
 *     struct exp_table t;
 *     exp_table_init (&t, "paint");
 *     add_expansion_copy (&t, "painted", mask0);
 *     add_expansion_copy (&t, "painting", mask1);
 *     add_expansion_copy (&t, "painter", mask2);
 *     for (i = 0; i < num_expansions (&t); i++)
 *         printf("expansion: %s\n", get_expansion (&t, i));
 *     exp_table_empty (&t);
 * 
 * where mask0 is a MASKTYPE with the flags used to add 'ed' set,
 * mask1 gives the flags for 'ing', etc.
 * 
 * Note that allocating the struct itself is up to you, but you
 * should initialize it with exp_table_init() before use and call
 * exp_table_empty() before you free it.
 */

/*
 * The structure itself.  Normally, it is better to use the accessors
 * below rather than access the structure directly.
 */
struct exp_table
  {
  char **	exps;		/* Table of expansions */
  MASKTYPE *	flags;		/* Flags used to get the expansions */
  int		size;		/* Current number of expansions */
  int		max_size;	/* Maximum number of expansions */
  ichar_t *	orig_word;	/* Root word that flags were applied to */
  };

/*
 * Initialize a struct exp_table.  After initialization the number of
 * expansions will be zero.  Pass in the original word from which the
 * expansions are generated - this will be stored by reference. 
 */
extern void	exp_table_init (struct exp_table * e, ichar_t * orig_name);

/* Return the original word in an expansion. */
extern const ichar_t * get_orig_word (const struct exp_table * e);

/* Return expansion number i (numbered from zero). */
extern const char *
		get_expansion (const struct exp_table * e, int i);

/* Return the flags used to get expansion number i. */
extern MASKTYPE	get_flags (const struct exp_table * e, int i);

/* Return number of expansions in the table. */
extern int	num_expansions (const struct exp_table * e);

/* Add a new expansion to the list, if it is not already in there.
 * Returns true iff the expansion was added.  Specify the result of
 * the expansion and the flags that were used.  Takes a copy of the
 * string passed in.
 */
extern int	add_expansion_copy (struct exp_table * e, const char * s,
		  MASKTYPE flags[]);

/*
 * Empty the table of expansions, freeing any resources allocated.
 * Returns a pointer to the now empty struct.
 */
extern struct exp_table * 
		exp_table_empty (struct exp_table * e);

/* Dump the contents of a table to stderr, for debugging. */
extern void	exp_table_dump (const struct exp_table * e);

#endif /* EXP_TABLE_H_INCLUDED */
