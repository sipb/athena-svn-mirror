/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains modified acl library code.
 *
 * Copyright (C) 1987,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: acl_files.c,v 1.18 1999-06-28 22:52:37 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: acl_files.c,v 1.18 1999-06-28 22:52:37 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

/* CHANGES from std. file:  fixed fd leak and null ptr. deref.
 *                          increased cache size
 *                          decreased lock time
 */

/*** Routines for manipulating access control list files ***/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <ctype.h>
#ifdef HAVE_KRB4
#include <krb.h>
#endif

#ifdef HAVE_KRB4
#ifndef KRB_REALM
#define KRB_REALM	"ATHENA.MIT.EDU"
#endif
#else
#include <sys/param.h> /* for MAXHOSTNAMELEN */
#define	      REALM_SZ        MAXHOSTNAMELEN
#define       INST_SZ         0               /* no instances w/o Kerberos */
#define       ANAME_SZ        9               /* size of a username + null */
#define       KRB_REALM       "ATHENA.MIT.EDU" /* your local "realm" */
#endif

/* "aname.inst@realm" */
#define MAX_PRINCIPAL_SIZE  (ANAME_SZ + INST_SZ + REALM_SZ + 3)
#define INST_SEP '.'
#define REALM_SEP '@'

#define LINESIZE 2048		/* Maximum line length in an acl file */

#define NEW_FILE "%s.~NEWACL~"	/* Format for name of altered acl file */
#define WAIT_TIME 3		/* Maximum time allowed write acl file */

#define CACHED_ACLS  45 /* How many acls to cache */
/* Each acl costs 1 open file descriptor */
#define ACL_LEN 32		/* Twice a reasonable acl length */

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define COR(a,b) ((a!=NULL)?(a):(b))

extern int errno;

extern time_t time();

/* Hash table stuff */

struct hashtbl {
  int size;			/* Max number of entries */
  int entries;		/* Actual number of entries */
  char **tbl;			/* Pointer to start of table */
};

static FILE *acl_lock_file (char *acl_file );
static int acl_abort (char *acl_file , FILE *f );
static int acl_commit (char *acl_file , FILE *f );
static void nuke_whitespace (char *buf );
static struct hashtbl *make_hash (int size );
static void destroy_hash (struct hashtbl *h );
static unsigned int hashval (char *s );
static void add_hash (struct hashtbl *h , char *el );
static int check_hash (struct hashtbl *h , char *el );
static int acl_load (char *name );

/* Canonicalize a principal name */
/* If instance is missing, it becomes "" */
/* If realm is missing, it becomes the local realm */
/* Canonicalized form is put in canon, which must be big enough to hold */
/* MAX_PRINCIPAL_SIZE characters */ 
     
void
acl_canonicalize_principal(principal, canon)
char *principal;
char *canon;
{
  char *dot, *atsign, *end;
  int len;
  
  dot = strchr(principal, INST_SEP);
  atsign = strchr(principal, REALM_SEP);
  
  /* Maybe we're done already */
  if(dot != NULL && atsign != NULL) {
    if(dot < atsign) {
      /* It's for real */
      /* Copy into canon */
      strncpy(canon, principal, MAX_PRINCIPAL_SIZE);
      canon[MAX_PRINCIPAL_SIZE-1] = '\0';
      return;
    } else {
      /* Nope, it's part of the realm */
      dot = NULL;
    }
  }
  
  /* No such luck */
  end = principal + strlen(principal);
  
  /* Get the principal name */
  len = MIN(ANAME_SZ, COR(dot, COR(atsign, end)) - principal);
  strncpy(canon, principal, len);
  canon += len;
  
  /* Add INST_SEP */
  *canon++ = INST_SEP;
  
  /* Get the instance, if it exists */
  if(dot != NULL) {
    ++dot;
    len = MIN(INST_SZ, COR(atsign, end) - dot);
    strncpy(canon, dot, len);
    canon += len;
  }
  
  /* Add REALM_SEP */
  *canon++ = REALM_SEP;
  
  /* Get the realm, if it exists */
  /* Otherwise, default to local realm */
  if(atsign != NULL) {
    ++atsign;
    len = MIN(REALM_SZ, end - atsign);
    strncpy(canon, atsign, len);
    canon += len;
    *canon++ = '\0';
  }
#ifdef HAVE_KRB4
  else if (krb_get_lrealm(canon, 1) != KSUCCESS) {
    strcpy(canon, KRB_REALM);
  }
#else
  else strcpy(canon, KRB_REALM);
#endif
}

/* Get a lock to modify acl_file */
/* Return new FILE pointer */
/* or NULL if file cannot be modified */
/* REQUIRES WRITE PERMISSION TO CONTAINING DIRECTORY */

static FILE
  *acl_lock_file(acl_file)
char *acl_file;
{
  struct stat s;
  char new[LINESIZE];
  int nfd;
  FILE *nf;
  int mode;
  
  if(stat(acl_file, &s) < 0) return(NULL);
  mode = s.st_mode;
  sprintf(new, NEW_FILE, acl_file);
  for(;;) {
    /* Open the new file */
    nfd = open(new, O_WRONLY|O_CREAT|O_EXCL, mode);
    if(nfd < 0) {
      if(errno == EEXIST) {
	/* Maybe somebody got here already, maybe it's just old */
	if(stat(new, &s) < 0) return(NULL);
	if(time(0) - s.st_ctime > WAIT_TIME) {
	  /* File is stale, kill it */
	  unlink(new);
	  continue;
	} else {
	  /* Wait and try again */
	  sleep(1);
	  continue;
	}
      } else {
	/* Some other error, we lose */
	return(NULL);
      }
    }
    
    /* If we got to here, the lock file is ours and ok */
    /* Reopen it under stdio */
    nf = fdopen(nfd, "w");
    if(nf == NULL) {
      /* Oops, clean up */
      unlink(new);
    }
    return(nf);
  }
}

/* Abort changes to acl_file written onto FILE *f */
/* Returns 0 if successful, < 0 otherwise */
/* Closes f */

static int
  acl_abort(acl_file, f)
char *acl_file;
FILE *f;     
{
  char new[LINESIZE];
  int ret;
  struct stat s;
  
  /* make sure we aren't nuking someone else's file */
  if(fstat(fileno(f), &s) < 0
     || s.st_nlink == 0) {
    fclose(f);
    return(-1);
  } else {
    sprintf(new, NEW_FILE, acl_file);
    ret = unlink(new);
    fclose(f);
    return(ret);
  }
}

/* Commit changes to acl_file written onto FILE *f */
/* Returns zero if successful */
/* Returns > 0 if lock was broken */
/* Returns < 0 if some other error occurs */
/* Closes f */

static int
  acl_commit(acl_file, f)
char *acl_file;
FILE *f;     
{
  char new[LINESIZE];
  char buf[LINESIZE];
  int ret;
  struct stat s;
  
  if(fflush(f) < 0
     || fstat(fileno(f), &s) < 0
     || s.st_nlink == 0) {
    perror("strange");
    acl_abort(acl_file, f);
    return(-1);
  }
  
  /*
   * FOLLOW THE SYMLINK
   */
  if (lstat(acl_file, &s) < 0)
    {
      perror("strange");
      acl_abort(acl_file, f);
      return(-1);
    }
  if ((s.st_mode & S_IFMT) == S_IFLNK)
    {
      int cc;
      char *ptr;

      /* strip off everything after the last slash */
      strcpy(buf, acl_file);
      ptr = strrchr(buf, '/');
      if (ptr != NULL)
	*(ptr+1) = '\0';

      /* read in the link name */
      cc = readlink(acl_file, new, LINESIZE);
      if (cc >= 0) {
	new[cc] = 0;
      }

      if (new[0] == '/')
	/* if leading char is slash, use it */
	strcpy(buf, new);
      else
	/* else tack it onto the path from above */
	strcat(buf, new);
    }
  else
    {
      strcpy(buf, acl_file);
    }
  /*
   * END... FOLLOW THE SYMLINK
   */

  sprintf(new, NEW_FILE, acl_file);
  ret = rename(new, buf  /* was: acl_file */);
  fclose(f);
  return(ret);
}

/* Initialize an acl_file */
/* Creates the file with permissions perm if it does not exist */
/* Erases it if it does */
/* Returns return value of acl_commit */

int
  acl_initialize(acl_file, perm)
char *acl_file;
int perm;
{
  FILE *new;
  int fd;
  
  /* Check if the file exists already */
  new = acl_lock_file(acl_file);
  if(new != NULL) {
    return(acl_commit(acl_file, new));
  } else {
    /* File must be readable and writable by owner */
    fd = open(acl_file, O_CREAT|O_EXCL, perm|0600);
    if(fd < 0) {
      return(-1);
    } else {
      close(fd);
      return(0);
    }
  }
}

/* Eliminate all whitespace character in buf */
/* Modifies its argument */
static void
  nuke_whitespace(buf)
char *buf;
{
  register char *pin, *pout;
  
  for(pin = pout = buf; *pin != '\0'; pin++)
    if(!isspace(*pin)) *pout++ = *pin;
  *pout = '\0';		/* Terminate the string */
}

/* Make an empty hash table of size s */

static struct hashtbl *
  make_hash(size)
int size;
{
  struct hashtbl *h;
  
  if(size < 1) size = 1;
  h = (struct hashtbl *) malloc(sizeof(struct hashtbl));
  h->size = size;
  h->entries = 0;
  h->tbl = (char **) calloc(size, sizeof(char *));
  return(h);
}

/* Destroy a hash table */
static void
  destroy_hash(h)
struct hashtbl *h;
{
  int i;
  
  for(i = 0; i < h->size; i++) {
    if(h->tbl[i] != NULL) free(h->tbl[i]);
  }
  free(h->tbl);
  free(h);
}

/* Compute hash value for a string */
static unsigned int
  hashval(s)
register char *s;
{
  register unsigned hv;
  
  for(hv = 0; *s != '\0'; s++) {
    hv ^= ((hv << 3) ^ *s);
  }
  return(hv);
}

/* Add an element to a hash table */
static void
  add_hash(h, el)
struct hashtbl *h;
char *el;
{
  unsigned hv;
  char *s;
  char **old;
  int i;
  
  /* Make space if it isn't there already */
  if(h->entries + 1 > (h->size >> 1)) {
    old = h->tbl;
    h->tbl = (char **) calloc(h->size << 1, sizeof(char *));
    for(i = 0; i < h->size; i++) {
      if(old[i] != NULL) {
	hv = hashval(old[i]) % (h->size << 1);
	while(h->tbl[hv] != NULL) hv = (hv+1) % (h->size << 1);
	h->tbl[hv] = old[i];
      }
    }
    h->size = h->size << 1;
    free(old);
  }
  
  hv = hashval(el) % h->size;
  while(h->tbl[hv] != NULL && strcmp(h->tbl[hv], el)) hv = (hv+1) % h->size;
  s = (char *) malloc(strlen(el)+1);
  strcpy(s, el);
  h->tbl[hv] = s;
  h->entries++;
}

/* Returns nonzero if el is in h */
static int
  check_hash(h, el)
struct hashtbl *h;
char *el;
{
  unsigned hv;
  
  for(hv = hashval(el) % h->size;
      h->tbl[hv] != NULL;
      hv = (hv + 1) % h->size) {
    if(!strcmp(h->tbl[hv], el)) return(1);
  }
  return(0);
}

struct acl {
  char filename[LINESIZE];	/* Name of acl file */
  int fd;			/* File descriptor for acl file */
  struct stat status;		/* File status at last read */
  struct hashtbl *acl;	/* Acl entries */
};

static struct acl acl_cache[CACHED_ACLS];

static int acl_cache_count = 0;
static int acl_cache_next = 0;

/* Returns < 0 if unsuccessful in loading acl */
/* Returns index into acl_cache otherwise */
/* Note that if acl is already loaded, this is just a lookup */

static int
  acl_load(name)
char *name;
{
  int i;
  FILE *f;
  struct stat s;
  char buf[MAX_PRINCIPAL_SIZE];
  char canon[MAX_PRINCIPAL_SIZE];
  int acl_fd;

  /* See if it's there already */
  for(i = 0; i < acl_cache_count; i++)
    if(!strcmp(acl_cache[i].filename, name)) {
      if (acl_cache[i].fd >= 0) 
	goto got_it;
      else  /* Exists in cache, but wasn't able to be opened last time */
	goto in_cache;
    }
  
  /* It isn't, load it in */
  /* maybe there's still room */
  if(acl_cache_count < CACHED_ACLS) {
    i = acl_cache_count++;
  } else {
    /* No room, clean one out */
    i = acl_cache_next;
    acl_cache_next = (acl_cache_next + 1) % CACHED_ACLS;
    close(acl_cache[i].fd);
    if(acl_cache[i].acl) {
      destroy_hash(acl_cache[i].acl);
      acl_cache[i].acl = (struct hashtbl *) 0;
    } 
  }
  
  /* Set up the acl */
  strcpy(acl_cache[i].filename, name);
 in_cache:
  acl_cache[i].fd = open(name, O_RDONLY, 0);
  if(acl_cache[i].fd < 0) return(-1);
  /* Force reload */
  acl_cache[i].acl = (struct hashtbl *) 0;
  
 got_it:
  /*
   * See if the stat matches
   *
   * Use stat(), not fstat(), as the file may have been re-created by
   * acl_add or acl_delete.  If this happens, the old inode will have
   * no changes in the mod-time and the following test will fail.
   */
  if(stat(acl_cache[i].filename, &s) < 0) return(-1);
  if(acl_cache[i].acl == (struct hashtbl *) 0
     || s.st_nlink != acl_cache[i].status.st_nlink
     || s.st_mtime != acl_cache[i].status.st_mtime
     || s.st_ctime != acl_cache[i].status.st_ctime) {
    /* Gotta reload */
    if(acl_cache[i].fd >= 0) close(acl_cache[i].fd);
    acl_cache[i].fd = open(name, O_RDONLY, 0);
    if(acl_cache[i].fd < 0) return(-1);
    /* If we just fdopen() but never fclose(), we leak memory.
     * On the other hand, if we fclose(), we lose the file descriptor.
     * We can either reload the ACL every time or use dup()...
     */
    acl_fd = dup(acl_cache[i].fd);
    if(acl_fd < 0) return(-1);
    f = fdopen(acl_fd, "r");
    if(f == NULL) return(-1);
    if(acl_cache[i].acl) destroy_hash(acl_cache[i].acl);
    acl_cache[i].acl = make_hash(ACL_LEN);
    while(fgets(buf, sizeof(buf), f) != NULL) {
      nuke_whitespace(buf);
      acl_canonicalize_principal(buf, canon);
      add_hash(acl_cache[i].acl, canon);
    }
    /* Yes, close the file.  See above. */
    fclose(f);
    acl_cache[i].status = s;
  }
  return(i);
}

/* Returns nonzero if it can be determined that acl contains principal */
/* Principal is not canonicalized, and no wildcarding is done */

int
  acl_exact_match(acl, principal)
char *acl;
char *principal;
{
  int idx;
  
  return((idx = acl_load(acl)) >= 0
	 && check_hash(acl_cache[idx].acl, principal));
}

/* Returns nonzero if it can be determined that acl contains principal */
/* Recognizes wildcards in acl of the form
   name.*@realm, *.*@realm, and *.*@* */

int
  acl_check(acl, principal)
char *acl;
char *principal;
{
  char buf[MAX_PRINCIPAL_SIZE];
  char canon[MAX_PRINCIPAL_SIZE];
  char *realm;
  
  acl_canonicalize_principal(principal, canon);
  
  /* Is it there? */
  if(acl_exact_match(acl, canon)) return(1);
  
  /* Try the wildcards */
  realm = strchr(canon, REALM_SEP);
  *strchr(canon, INST_SEP) = '\0';	/* Chuck the instance */
  /* TODO: can strchr ever return NULL in this context?  --bert 29jan1997 */

  sprintf(buf, "%s.*%s", canon, realm);
  if(acl_exact_match(acl, buf)) return(1);
  
  sprintf(buf, "*.*%s", realm);
  if(acl_exact_match(acl, buf) || acl_exact_match(acl, "*.*@*")) return(1);
  
  return(0);
}

/* Adds principal to acl */
/* Wildcards are interpreted literally */
int
  acl_add(acl, principal)
char *acl;
char *principal;
{
  int idx;
  int i;
  FILE *new;
  char canon[MAX_PRINCIPAL_SIZE];
  
  acl_canonicalize_principal(principal, canon);
  
  new = acl_lock_file(acl);
  if(new == NULL) return(-1);
  if((acl_exact_match(acl, canon))
     || (idx = acl_load(acl)) < 0) {
    acl_abort(acl, new);
    return(-1);
  }
  /* It isn't there yet, copy the file and put it in */
  for(i = 0; i < acl_cache[idx].acl->size; i++) {
    if(acl_cache[idx].acl->tbl[i] != NULL) {
      if((fputs(acl_cache[idx].acl->tbl[i], new) == 0)
	 || (putc('\n', new) != '\n')) {
	acl_abort(acl, new);
	return(-1);
      }
    }
  }
  fputs(canon, new);
  putc('\n', new);
  return(acl_commit(acl, new));
}

/* Removes principal from acl */
/* Wildcards are interpreted literally */
int
  acl_delete(acl, principal)
char *acl;
char *principal;
{
  int idx;
  int i;
  FILE *new;
  char canon[MAX_PRINCIPAL_SIZE];
  
  acl_canonicalize_principal(principal, canon);
  
  new = acl_lock_file(acl);
  if(new == NULL) return(-1);
  if((!acl_exact_match(acl, canon))
     || (idx = acl_load(acl)) < 0) {
    acl_abort(acl, new);
    return(-1);
  }
  /* It isn't there yet, copy the file and put it in */
  for(i = 0; i < acl_cache[idx].acl->size; i++) {
    if(acl_cache[idx].acl->tbl[i] != NULL
       && strcmp(acl_cache[idx].acl->tbl[i], canon)) {
      fputs(acl_cache[idx].acl->tbl[i], new);
      putc('\n', new);
    }
  }
  return(acl_commit(acl, new));
}