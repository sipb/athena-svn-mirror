/*
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#ifndef lint
#ifndef SABER
  static char rcsid[] ="$Id: acl_files.c,v 1.9 1999-06-28 22:52:50 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include "rpd.h"

#ifndef HAVE_KRB4
#define         ANAME_SZ        40
#define         REALM_SZ        40
#define         INST_SZ         40
#define		DEFAULT_REALM	"ATHENA.MIT.EDU"
#endif

/* "aname.inst@realm" */
#define MAX_PRINCIPAL_SIZE  (ANAME_SZ + INST_SZ + REALM_SZ + 3)
#define INST_SEP '.'
#define REALM_SEP '@'
#define LINESIZE 2048		/* Maximum line length in an acl file */
#define CACHED_ACLS  2	/* How many acls to cache */
			/* Each acl costs 1 open file descriptor */
#define ACL_LEN 128
#define COR(a,b) ((a!=NULL)?(a):(b))

struct hashtbl {
  int size;			/* Max number of entries */
  int entries;		/* Actual number of entries */
  char **tbl;			/* Pointer to start of table */
};

static int acl_load (char *name);
static void add_hash (struct hashtbl *h, char *el);
static int check_hash (struct hashtbl *h, char *el);
static void destroy_hash (struct hashtbl *h);
static unsigned int hashval (char *s);
static struct hashtbl *make_hash (int size);
static void nuke_whitespace (char *buf);

/* CHANGES from std. file:  fixed fd leak and null ptr. deref.
 *                          increased cache size
 *                          decreased lock time
 */


/* Canonicalize a principal name */
/* If instance is missing, it becomes "" */
/* If realm is missing, it becomes the local realm */
/* Canonicalized form is put in canon, which must be big enough to hold
   MAX_PRINCIPAL_SIZE characters */

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
  else strcpy(canon, DEFAULT_REALM);
#endif
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

static int acl_load(name)
     char *name;
{
  int i;
  FILE *f;
  struct stat s;
  char buf[MAX_PRINCIPAL_SIZE];
  char canon[MAX_PRINCIPAL_SIZE];
  
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
    f = fdopen(acl_cache[i].fd, "r");
    if(f == NULL) return(-1);
    if(acl_cache[i].acl) destroy_hash(acl_cache[i].acl);
    acl_cache[i].acl = make_hash(ACL_LEN);
    while(fgets(buf, sizeof(buf), f) != NULL) {
      nuke_whitespace(buf);
      acl_canonicalize_principal(buf, canon);
      add_hash(acl_cache[i].acl, canon);
    }
    fclose(f);
    acl_cache[i].status = s;
  }
  return(i);
}

/* Returns nonzero if it can be determined that acl contains principal */
/* Principal is not canonicalized, and no wildcarding is done */
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
