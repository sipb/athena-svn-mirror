/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/pathcan.c,v $
 *	$Author: probe $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/pathcan.c,v 1.3 1992-01-06 15:55:00 probe Exp $
 */

#ifndef lint
static char *rcsid_pathcan_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/pathcan.c,v 1.3 1992-01-06 15:55:00 probe Exp $";
#endif

#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#define MAXLINKS	16

extern int	errno;
extern	char	*malloc();

int	pc_symlinkcnt;
int	pc_depthcnt;

#ifdef DEBUG
static char	*strdup();
#else
extern char	*strdup();
#endif

struct filestack {
	char	*path;
	char	*ptr;		/* pointer to the next thing to fix */
};

char	*path_canon(infile)
	char	*infile;
{
	extern char *getwd();
	static struct filestack	stack[MAXLINKS];
	int			stkcnt = 0;	/* Stack counter */
	struct filestack	*stkptr = &stack[0]; /*Always = &stack[stkcnt]*/
	register int	i;
	register char	*cp, *token;
	static char	outpath[MAXPATHLEN];
	char	*outpathend = outpath; /* Always points to the end of outpath*/
	char		bfr[MAXPATHLEN];
	struct	stat	statbuf;
	int		errorflag = 0;
	
	pc_symlinkcnt = pc_depthcnt = 0;
	
	if (!infile)
		return(NULL);

	stkptr->ptr = stkptr->path = strdup(infile);

	if (*infile == '/') {
		/*
		 * We're starting at the root; normal case
		 */
		stkptr->ptr++;
		outpath[0]='\0';
	} else {
		/*
		 * We're being asked to interpret a relative pathname;
		 * assume this is happening relative to the current
		 * directory.
		 */
		if (getwd(outpath) == NULL) {
#ifdef TEST
			printf("getwd returned error, %s", outpath);
#endif
			return(NULL);
		}
		outpathend += strlen(outpathend);
	}
	
	while (stkcnt >= 0) {
		/*
		 * If there's no more pathname elements in this level
		 * of recursion, pop the stack and continue.
		 */
		if (!stkptr->ptr || !*stkptr->ptr) {
#ifdef TEST
			printf("Popping.... stkcnt = %d\n", stkcnt);
#endif
			free(stkptr->path);
			stkcnt--;
			stkptr--;
			continue;
		}
#ifdef TEST
		printf("stkcnt = %d, ptr = %s, out = '%s'\n", stkcnt,
		       stkptr->ptr, outpath);
#endif

		/*
		 * Peel off the next token and bump the pointer
		 */
		token = stkptr->ptr;
		if (cp = index(stkptr->ptr, '/')) {
			*cp = '\0';
			stkptr->ptr = cp+1;
		} else
			stkptr->ptr = NULL;

		/*
		 * If the token is "" or ".", then just continue
		 */
		if (!*token || !strcmp(token, "."))
			continue;
		
		/*
		 * If the token is "..", then lop off the last part of
		 * outpath, and continue.
		 */
		if (!strcmp(token, "..")) {
			if (cp = rindex(outpath, '/'))
				*(outpathend = cp) = '\0';
			continue;
		}

		/*
		 * Tack on the new token, but don't advance outpathend
		 * yet (we may need to back out).
		 */
		*outpathend = '/';
		(void) strcpy(outpathend+1, token);
		if (!errorflag && lstat(outpath, &statbuf)) {
#ifdef TEST
			if (errno)
				perror(outpath);
#endif
			/*
			 * If we get a file not found, or the file is
			 * not a directory, set a flag so that
			 * lstat() is skipped from being called, since
			 * there's no point in trying any future
			 * lstat()'s. 
			 */
			if (errno == ENOTDIR || errno == ENOENT) {
				errorflag = errno;
				outpathend += strlen(outpathend);
				continue; 	/* Go back and pop stack */
			}
			return(NULL);
		}

		/*
		 * If outpath expanded to a symlink, we're going to
		 * expand it.  This entails: 1) reading the value of
		 * the symlink.  2) Removing the appended token to
		 * outpath.  3) Recursively expanding the value of the
		 * symlink by pushing it onto the stack.
		 */
		if (!errorflag && (statbuf.st_mode & S_IFMT) == S_IFLNK) {
			pc_symlinkcnt++;

			if ((i = readlink(outpath, bfr, sizeof(bfr))) < 0) {
#ifdef TEST
				perror("readlink");
#endif
				return(NULL);
			}
			bfr[i] = '\0';
#ifdef TEST
			printf("stkcnt = %d, found symlink to %s\n",
			       stkcnt, bfr);
#endif
			*outpathend = '\0'; /* Back it out */
			stkcnt++;
			stkptr++;
			if (stkcnt >= MAXLINKS) {
				errno = ELOOP;
#ifdef TEST
				printf("Stack limit exceeded! Aborting...\n");
#endif
				return(NULL);
			}
			stkptr->ptr = stkptr->path = strdup(bfr);
			if (bfr[0] == '/') {
				/* This is a sym link to root, we can */
				/* blast outpath and start over */
				outpathend = outpath;
				*outpath = '\0';
				stkptr->ptr++; /* Bump past / */
			}
			if (stkcnt > pc_depthcnt)
				pc_depthcnt = stkcnt;
			continue;
		}
		/*
		 * This is a normal case.  Extend out outpathend,
		 * and continue to the next path element
		 */
		outpathend += strlen(outpathend);
	}
	/*
	 * Special case: if outpath is empty, this means we're at the
	 * filesystem root
	 */
	if (!*outpath)
		return("/");
	else
		return(outpath);
}


#ifdef DEBUG
/*
 * Duplicate a string in malloc'ed memory
 */
static char *strdup(s)
	char	*s;
{
	register char	*cp;
	
	if (!(cp = malloc(strlen(s)+1))) {
		printf("Out of memory!!!\n");
		abort();
	}
	return(strcpy(cp,s));
}

main (argc, argv)
	int argc;
	char **argv;
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s pathname\n", argv[0]);
		exit(1);
	}
	printf("Result: %s\n", path_canon(argv[1]));
	printf("Number of symlinks traversed: %d\n", pc_symlinkcnt);
	printf("Maximum depth of symlink traversal: %d\n", pc_depthcnt);
	exit(0);
}
#endif
