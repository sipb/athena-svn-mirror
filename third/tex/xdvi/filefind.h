/*
 * Copyright (c) 1996 Paul Vojta.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 *	filefind.h - Define the data structures that specify how to search
 *		for various types of files.
 */


/*
 *	First, some macro definitions.
 *	Optionally, filf_app.h may define these.
 */

#ifndef	True
typedef	char		Boolean;
#define	True	1
#define	False	0
#endif

#ifndef	ARGS
#if	NeedFunctionPrototypes
#define	ARGS(x)	x
#else
#define	ARGS(x)	()
#endif
#endif

/*
 *	Record describing how to search for this type of file.  Fields are
 *	described further below.
 */

struct	findrec {
	_Xconst	char		*path1;
#if	CFGFILE
	_Xconst	struct envrec	*envptr;
#endif
	_Xconst	char		*path2;
	_Xconst	char		*type;
	_Xconst	char		*fF_etc;
	char			x_var_char;
	int			n_var_opts;
	_Xconst	char		*no_f_str;
	_Xconst	char		*no_f_str_end;
	_Xconst	char		*abs_str;
#ifdef	PK_AND_GF
	int			no_f_str_flags;
	int			abs_str_flags;
	char			pk_opt_char;
	_Xconst	char		**pk_gf_addr;
#endif
	_Xconst	char		*pct_s_str;

	struct {
		struct steprec		*stephead;
		struct steprec		*pct_s_head;
		int			pct_s_count;
		_Xconst	struct atomrec	**pct_s_atom;
		struct rootrec		*rootp;
	} v;
};

/*
 *	And now, the star of our show.
 */

extern	FILE	*filefind ARGS((_Xconst char *, struct findrec *,
			_Xconst char **));

/*
 *	Here are the meanings of the various fields in struct findrec:
 *
 *	const char *path1
 *		Colon-separated list of path components to use in the search.
 *		This can be obtained, for example, from the environment
 *		variable 'XDVIFONTS.'
 *
 *	const struct envrec *envptr
 *		Pointer to linked list of local environment variable records
 *		for the appropriate variable.  These variables are only the
 *		ones defined in some config file.
 *
 *	const char *path2
 *		Secondary list of path components to use in the search.
 *		Normally, this is the compiled-in default, and is used at the
 *		point (if any) where an extra colon appears in path1.
 *
 *	const char *type
 *		The type of the search (font, vf, etc.).
 *
 *	const char *fF_etc
 *		Certain of the characters following '%' that are to be
 *		substituted with strings.  This should begin with 'f' and 'F'.
 *		It should not include 'q', 'Q', 't', 's', or 'S'.
 *
 *	char x_var_char
 *		If there is another character besides 'f' and 'F' whose
 *		corresponding string may change for different files of this
 *		type, then it should be third in fF_etc, and x_var_char
 *		should contain its value.  Otherwise, x_var_char should be 'f'.
 *
 *	int n_var_opts
 *		x_var_char == 'f' ? 2 : 3
 *
 *	const char *no_f_str
 *		The address of the string to be implicitly added to the path
 *		component if %f was not used.
 *
 *	const char *no_f_str_end
 *		The address of '\0' terminating the above string.
 *
 *	int no_f_str_flags
 *		Flags to be set if no_f_str is used.
 *
 *	const char *abs_str
 *		String to use for the path in case the font/file name begins
 *		with a '/' (indicating an absolute path).
 *
 *	int abs_str_flags
 *		Flags to be set if abs_str is used.
 *
 *	char pk_opt_char
 *		'p' for pk/gf file searching (signifying that %p is to be
 *		replaced alternately with "pk" and "gf"; 'f' otherwise
 *		(signifying that there is no such character).
 *
 *	const char **pk_gf_addr
 *		The address of a pointer that is to be alternately changed
 *		to point to "pk" or "gf", as above.
 *
 *	const char *pct_s_str
 *		The string to be substituted for %s occurring at the end of
 *		a path component.  This may not contain braces ({}), but it
 *		may consist of several strings, separated by colons.
 *
 *	The remaining fields store information about past searches.  They
 *	should be initialized to NULL or 0, as appropriate.
 *
 *	struct steprec		*stephead;
 *		Head of the first linked list of steprecs.
 *
 *	struct steprec		*pct_s_head;
 *		Head of the linked list of steprecs for %s.
 *
 *	int			pct_s_count;
 *		Length of the above linked list.
 *
 *	const	struct atomrec	**pct_s_atom;
 *		An array containing linked list of atomrecs for %s.
 *
 *	struct rootrec		*rootp;
 *		Head of the linked list of rootrecs.
 */

/*
 *	Values for flags.
 */

#define	F_FILE_USED	1		/* if %f was used */
#define	F_VARIABLE	2		/* if the string may change next time */
#define	F_PK_USED	4		/* if %p was used */
#define	F_PCT_T		8		/* component began with %t */
#define	F_PCT_S		16		/* put %s after this string of atoms */
#define	F_SLASH_SLASH	32		/* (atomrec) if this began with // */
#define	F_WILD		64		/* (atomrec) if wild cards present */
#define	F_QUICKFIND	128		/* (atomrec, rootrec) if %q or %Q */
#define	F_QUICKONLY	256		/* if %Q */

/*
 *	Values to put in for %x specifiers.
 */

#ifndef	MAX_N_OPTS
#define	MAX_N_OPTS	6
#endif

extern	_Xconst	char		*fF_values[MAX_N_OPTS];

/*
 *	Additional data structures.  These are included because they are
 *	referenced in the above structure, but really they are internal
 *	to filefind.c.
 */

/*
 *	This is the main workhorse data structure.  It is used (1) in a linked
 *	list, one for each component of the path; (2) in linked lists of
 *	{} alternatives; and (3) in linked lists of alternatives for %s.
 *	It contains precomputed (via lazy evaluation) information about how
 *	to lay down the candidate file name.
 */

struct	steprec	{
	struct steprec	*next;		/* link to next record in chain */
	_Xconst	char	*str;		/* the string we're now looking at */
	_Xconst	char	*strend;
	int		flags;		/* not cumulative */
	struct atomrec	*atom;
	struct steprec	*nextstep;
	_Xconst	char	*nextpart;	/* only for the main linked list */
	_Xconst	char	*home;		/* ~ expansion result (main list only)*/
};

/*
 *	Components of a path following a // or wild card.
 */

struct	atomrec {
	struct atomrec	*next;
	_Xconst	char	*p;		/* first char. of dir. name */
	_Xconst	char	*p_end;		/* last + 1 char. */
	int		flags;		/* F_PCT_S or F_SLASH_SLASH (so far) */
};

/*
 *	Roots of tree structures containing information on subdirectories.
 */

struct	rootrec {
	struct rootrec	*next;		/* link to next in list */
	struct treerec	*tree;		/* the tree */
	int		flags;		/* F_QUICKFIND and F_QUICKONLY */
	_Xconst	char	*fullpat;	/* xlat pattern for use with ls-R */
	_Xconst	char	*fullpat_end;
	_Xconst	char	*keypat;	/* key for ls-R database search */
	_Xconst	char	*keypat_end;
};

/*
 *	Nodes in the above-mentioned tree structure.
 */

struct	treerec {
	_Xconst	char	*dirname;
	struct treerec	*next;		/* link to next sibling */
	struct treerec	*child;		/* link to subdirectories */
	Boolean		tried_already;
	Boolean		isdir;
};

/*
 *	Config file environment variable.
 */

struct envrec {
	struct envrec	*next;
	_Xconst	char	*key;
	_Xconst	char	*value;
	Boolean		flag;
};

extern	struct envrec	*ffgetenv ARGS((_Xconst char *));
