/*	Created by:	Theodore Ts'o
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/quota/attachtab.c,v $
 *	$Author: probe $
 *
 *	Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *
 * Contains the routines which access the attachtab file.
 */

#ifndef lint
static char rcsid_attachtab_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/quota/attachtab.c,v 1.1 1991-07-18 22:06:13 probe Exp $";
#endif lint

#include "attach.h"
#include <sys/file.h>
#include <pwd.h>
#include <string.h>

#define TOKSEP " \t\r\n"

static int parse_attach();
int debug_flag=0;
char *attachtab_fn = "/usr/tmp/attachtab";
char *abort_msg = "Operation aborted\n";

struct	_attachtab	*attachtab_first, *attachtab_last;
int	old_attab = 0;


struct _fstypes fstypes[] = {
    { "---", 0},
    { "NFS", TYPE_NFS},
    { "RVD", TYPE_RVD},
    { "UFS", TYPE_UFS},
    { "AFS", TYPE_AFS},
    { "ERR", TYPE_ERR},
    { "MUL", TYPE_MUL},
    { 0, -1}
};


/*
 * LOCK the attachtab - wait for it if it's already locked
 */
static int attach_lock_fd = -1;
static int attach_lock_count = 0;

void lock_attachtab()
{
	register char	*lockfn;
	
	if (debug_flag)
		printf("Locking attachtab....");
	if (attach_lock_count == 0) {
		if (!(lockfn = malloc(strlen(attachtab_fn)+6))) {
			fprintf(stderr,
				"Can't malloc for lockfile filename\n");
			fprintf(stderr, abort_msg);
			exit(ERR_FATAL);
		}
		(void) strcpy(lockfn, attachtab_fn);
		(void) strcat(lockfn, ".lock");
	
		if (attach_lock_fd < 0) {
			attach_lock_fd = open(lockfn, O_CREAT, 0644);
			if (attach_lock_fd < 0) {
				fprintf(stderr,"Can't open %s: %s\n", lockfn,
					sys_errlist[errno]);
				fprintf(stderr, abort_msg);
				exit(ERR_FATAL);
			}
		}
		flock(attach_lock_fd, LOCK_EX);
		free(lockfn);
	}
	attach_lock_count++;
}


/*
 * UNLOCK the attachtab
 */
void unlock_attachtab()
{
	if (debug_flag)
		printf("Unlocking attachtab\n");
	attach_lock_count--;
	if (attach_lock_count == 0) {
		close(attach_lock_fd);
		attach_lock_fd = -1;
	} else if (attach_lock_count < 0) {
		fprintf(stderr,
	"Programming botch!  Tried to unlock unlocked attachtab\n");
	}
}

/*
 * get_attachtab - Reads in the attachtab file and sets up the doubly
 * 	linked list of attachtab entries.  Assumes attachtab is
 * 	already locked.
 */
void get_attachtab()
{
	register struct _attachtab	*atprev = NULL;
	register struct _attachtab	*at = NULL;
	register FILE			*f;
	char				attach_buf[BUFSIZ];
	
	free_attachtab();
	if ((f = fopen(attachtab_fn, "r")) == NULL)
		return;
	
	while (fgets(attach_buf, sizeof attach_buf, f)) {
		if (!(at = (struct _attachtab *) malloc(sizeof(*at)))) {
			fprintf(stderr,
				"Can't malloc while parsing attachtab\n");
			fprintf(stderr, abort_msg);
			exit(ERR_FATAL);
		}
		parse_attach(attach_buf, at, 0);
		at->prev = atprev;
		if (atprev)
			atprev->next = at;
		else
			attachtab_first = at;
		atprev = at;
	}
	attachtab_last = at;
	fclose(f);
}

/*
 * free the linked list of attachtab entries
 */
void free_attachtab()
{
	register struct _attachtab     	*at, *next;
	
	if (!(at = attachtab_first))
		return;
	while (at) {
		next = at->next;
		free(at);
		at = next;
	}
	attachtab_first = attachtab_last = NULL;
}	


/*
 * Convert an attachtab line to an attachtab structure
 */
static int parse_attach(bfr, at, lintflag)
    register char *bfr;
    register struct _attachtab *at;
    int lintflag;
{
    register char *cp;
    int	old_version = 0; 	/* Backwards compat attachtab line */

    if (!*bfr)
	goto bad_line;
	
    if (bfr[strlen(bfr)-1] < ' ')
	bfr[strlen(bfr)-1] = '\0';
	
    if (!(cp = strtok(bfr, TOKSEP)))
	    goto bad_line;

    if (!strcmp(cp, "A0"))
	    old_version = 1;
    else
    
    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;

    if (*cp != '0' && *cp != '1')
	goto bad_line;
    cp++;
    
    at->status = *cp++;
    if (at->status != STATUS_ATTACHED && at->status != STATUS_ATTACHING &&
	at->status != STATUS_DETACHING)
	goto bad_line;

    at->fs = get_fs(cp);
    if (at->fs == NULL)
	goto bad_line;

    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    (void) strcpy(at->hesiodname, cp);

    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    (void) strcpy(at->host, cp);

    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    (void) strcpy(at->hostdir, cp);

    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;

    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;

    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    (void) strcpy(at->mntpt, cp);

    if (!old_version) {
	    if (!(cp = strtok(NULL, TOKSEP)))
		    goto bad_line;
    
	    if (!(cp = strtok(NULL, TOKSEP)))
		    goto bad_line;

	    if (*cp == ',')	/* If heading character is comma */
		    cp = 0;	/* we have a null list*/
	    while (cp) {
		    register char *dp;
		    
		    if (dp = index(cp, ','))
			    *dp++ = '\0';
		    cp = dp;
	    }
    } else {
	    old_attab++;
    }
    
    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    
    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    
    at->next = NULL;
    at->prev = NULL;
    return SUCCESS;
    
bad_line:
    fprintf(stderr,"Badly formatted entry in %s\n", attachtab_fn);
    if (lintflag)
      return FAILURE;
    fprintf(stderr, abort_msg);
    exit(ERR_FATAL);
    /* NOTREACHED */
}

/*
 * Lookup an entry in attachtab by mountpoint --- uses the linked list
 *      stored in memory
 */
struct _attachtab *attachtab_lookup_mntpt(pt)
    register char *pt;
{
        register struct _attachtab      *p;

        p = attachtab_first;
        while (p) {
                if (!strcmp(p->mntpt, pt))
                        return(p);
                p = p->next;
        }
        return (NULL);
}

/*
 * Convert a string type to a filesystem type entry
 */
struct _fstypes *get_fs(s)
    char *s;
{
    int i;

    if (s && *s) {
            for (i=0;fstypes[i].name;i++) {
                    if (!strcasecmp(fstypes[i].name, s))
                            return (&fstypes[i]);
            }
    }
    return (NULL);
}
