/*	Created by:	Theodore Ts'o
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/attachtab.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *
 * Contains the routines which access the attachtab file.
 */

#ifndef lint
static char rcsid_attachtab_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/attachtab.c,v 1.4 1990-07-04 16:26:09 jfc Exp $";
#endif lint

#include "attach.h"
#include <sys/file.h>
#include <pwd.h>
#include <string.h>

#define TOKSEP " \t\r\n"

static int parse_attach();

struct	_attachtab	*attachtab_first, *attachtab_last;

int	old_attab = 0;

/*
 * LOCK the attachtab - wait for it if it's already locked
 */
static int attach_lock_fd = -1;
static int attach_lock_count = 0;

void lock_attachtab()
{
	char	*lockfn;
	
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
	struct _attachtab	*atprev = NULL;
	struct _attachtab	*at = NULL;
	FILE			*f;
	char			attach_buf[BUFSIZ];
	
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
 * put_attachtab - write out attachtab file from linked list and
 * 	unlocks attachtab
 */
put_attachtab()
{
	FILE	*f;
	struct	_attachtab	*at;
	char	*tempfn;
	int	i;
	
	if (!(tempfn = malloc(strlen(attachtab_fn)+6))) {
		fprintf(stderr,
			"Can't malloc while writing attachtab.\n");
		fprintf(stderr, abort_msg);
		exit(ERR_FATAL);
	}
	(void) strcpy(tempfn, attachtab_fn);
	(void) strcat(tempfn, ".temp");
	if (!(f = fopen(tempfn, "w"))) {
		fprintf(stderr,"Can't open %s for write: %s\n", attachtab_fn,
			sys_errlist[errno]);
		fprintf(stderr, abort_msg);
		exit(ERR_FATAL);
	}
	at = attachtab_first;
	while (at) {
		if (strcmp(at->version, ATTACH_VERSION))
			fprintf(f, "%s %c%c%s %s %s %s %s %d %s %d %c\n",
				at->version, at->explicit ? '1' : '0',
				at->status, (at->fs) ? at->fs->name : "---",
				at->hesiodname, at->host, at->hostdir,
				inet_ntoa(at->hostaddr), at->rmdir, at->mntpt,
				at->drivenum, at->mode);
		else {
			fprintf(f, "%s %c%c%s %s %s %s %s %d %s %d ",
				at->version, at->explicit ? '1' : '0',
				at->status, (at->fs) ? at->fs->name : "---",
				at->hesiodname, at->host, at->hostdir,
				inet_ntoa(at->hostaddr), at->rmdir, at->mntpt,
				at->flags);
			if (at->nowners)
				fprintf(f, "%d", at->owners[0]);
			else
				fprintf(f, ",");
			for (i=1; i < at->nowners; i++)
				fprintf(f, ",%d", at->owners[i]);
			fprintf(f, " %d %c\n", at->drivenum, at->mode);
		}
		at = at->next;
	}
	fclose(f);

	if (rename(tempfn, attachtab_fn)) {
		fprintf(stderr,"Can't rename %s to %s\n", tempfn,
			attachtab_fn);
		fprintf(stderr, abort_msg);
		exit(ERR_FATAL);
	}
}

void lint_attachtab()
{
	struct _attachtab	*atprev = NULL;
	struct _attachtab	*at = NULL;
	FILE			*f;
	char			attach_buf[BUFSIZ];
	
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
		if(parse_attach(attach_buf, at, 1) == FAILURE)
		  continue;	/* bad line -- ignore */
		at->prev = atprev;
		if (atprev)
			atprev->next = at;
		else
			attachtab_first = at;
		atprev = at;
	}
	attachtab_last = at;
	fclose(f);
	put_attachtab();
	free_attachtab();
}

/*
 * Convert an attachtab line to an attachtab structure
 */
static int parse_attach(bfr, at, lint)
    char *bfr;
    struct _attachtab *at;
    int lint;
{
    char	*cp, *dp;
    int		old_version = 0; /* Backwards compat attachtab line */

    if (!*bfr)
	goto bad_line;
	
    if (bfr[strlen(bfr)-1] < ' ')
	bfr[strlen(bfr)-1] = '\0';
	
    if (!(cp = strtok(bfr, TOKSEP)))
	    goto bad_line;

    if (!strcmp(cp, "A0"))
	    old_version = 1;
    else if (strcmp(cp, ATTACH_VERSION)) {
	    fprintf(stderr, "Bad version number in %s\n", attachtab_fn);
	    if(lint) 
	      return FAILURE;
	    fprintf(stderr, abort_msg);
	    exit(ERR_FATAL);
    }
    strcpy(at->version, cp);
    
    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;

    if (*cp != '0' && *cp != '1')
	goto bad_line;
    at->explicit = *cp++-'0';
    
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
    at->hostaddr.s_addr = inet_addr(cp);

    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    at->rmdir = atoi(cp);

    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    (void) strcpy(at->mntpt, cp);

    if (!old_version) {
	    if (!(cp = strtok(NULL, TOKSEP)))
		    goto bad_line;
	    at->flags = atoi(cp);
    
	    if (!(cp = strtok(NULL, TOKSEP)))
		    goto bad_line;
	    at->nowners = 0;
	    if (*cp == ',')	/* If heading character is comma */
		    cp = 0;	/* we have a null list*/
	    while (cp) {
		    if (dp = index(cp, ','))
			    *dp++ = '\0';
		    at->owners[at->nowners++] = atoi(cp);
		    cp = dp;
	    }
    } else {
	    at->flags = 0;
	    at->nowners = 1;
	    at->owners[0] = 0;
	    old_attab++;
    }
    
    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    
    at->drivenum = atoi(cp);
    
    if (!(cp = strtok(NULL, TOKSEP)))
	    goto bad_line;
    at->mode = *cp;
    if (at->fs->good_flags) {
	    if (!index(at->fs->good_flags, at->mode))
		    goto bad_line;	/* Bad attach mode */
    }
    
    at->next = NULL;
    at->prev = NULL;
    return SUCCESS;
    
bad_line:
    fprintf(stderr,"Badly formatted entry in %s\n", attachtab_fn);
    if(lint)
      return FAILURE;
    fprintf(stderr, abort_msg);
    exit(ERR_FATAL);
}

/*
 * Routine to see if a host is in the attachtab - uses the linked list
 * attachtab in memory
 */
host_occurs(name)
    char *name;
{
	struct _attachtab	*p;

	p = attachtab_first;
	while (p) {
		if (!strcasecmp(p->host, name))
			return(1);
		p = p->next;
	}
	return (0);
}

/*
 * Lookup an entry in attachtab  --- uses the linked list stored in memory
 */
struct _attachtab *attachtab_lookup(hesiod)
    char *hesiod;
{
	struct _attachtab	*p;

	p = attachtab_first;
	while (p) {
		if (p->explicit == explicit && !hescmp(p, hesiod)) 
			return(p);
		p = p->next;
	}
	return (NULL);
}

/*
 * Lookup an entry in attachtab by mountpoint --- uses the linked list
 * 	stored in memory
 */
struct _attachtab *attachtab_lookup_mntpt(pt)
    char *pt;
{
	struct _attachtab	*p;

	p = attachtab_first;
	while (p) {
		if (!strcmp(p->mntpt, pt))
			return(p);
		p = p->next;
	}
	return (NULL);
}

/*
 * Delete an entry from the attachtab --- unlinks the passed attachtab
 * 	record in the in-core linked list; assumes attachtab is locked.
 * 	You must call put_attachtab to actually affect the copy on disk.
 */
attachtab_delete(at)
	struct _attachtab	*at;
{
	if (!at)
		return;
	
	if (at->prev)
		at->prev->next = at->next;
	else
		attachtab_first = at->next;
	if (at->next)
		at->next->prev = at->prev;
	else
		attachtab_last = at->prev;
	free(at);
}

/*
 * Append an entry to the linked list database in attachtab
 */
attachtab_append(at)
	struct _attachtab *at;
{

	struct _attachtab *atnew;

	if (!(atnew = (struct _attachtab *)
	      malloc(sizeof(struct _attachtab)))) {
		fprintf(stderr, "Can't malloc while adding to attachtab\n");
		fprintf(stderr, abort_msg);
		exit(ERR_FATAL);
	}
	*atnew = *at;

	/* If old_attab, write out entries in old format */
	strcpy(atnew->version, old_attab ? "A0" : ATTACH_VERSION);
	
	if (attachtab_last)
		attachtab_last ->next = atnew;
	else
		attachtab_first = atnew;
	atnew->prev = attachtab_last;
	atnew->next = NULL;
	attachtab_last = atnew;
}


/*
 * Replace an entry in the linked list 
 */
attachtab_replace(at)
	struct _attachtab *at;
{
	struct _attachtab *p, *next, *prev;

	/* If old_attab, write out entries in old format */
	strcpy(at->version, old_attab ? "A0" : ATTACH_VERSION);
		
	p = attachtab_first;

	while (p) {
		if (!hescmp(p, at->hesiodname) && p->explicit == explicit) {
			next = p->next;
			prev = p->prev;
			*p = *at;
			p->next = next;
			p->prev = prev;
		}
		p = p->next;
	}
}

