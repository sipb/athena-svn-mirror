/*LINTLIBRARY*/
/**********************************************************************
 * Access Control List Library
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/libacl.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/libacl.c,v 1.4 1996-09-20 04:36:17 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_libacl_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/libacl.c,v 1.4 1996-09-20 04:36:17 ghudson Exp $";
#endif /* lint */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <krb.h>
#include <sys/errno.h>
#include <netdb.h>
#include "memory.h"

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

/* If ACL_ALREADY != 0, it is considered a failure to try to add
 * a principal to an acl that already contains it or to delete a
 * principal from an acl that doesn't contain it.
 */
#define ACL_ALREADY 1
char *
acl_canonicalize_principal(principal, buf)
     char *principal;
     char *buf;      /* RETVAL */
{
  char name[ANAME_SZ], instance[INST_SZ], realm[REALM_SZ];
  char *s1, *s2;

  s1 = strchr(principal, '@');
  if (s1) {
    (void) strcpy(realm, s1);
    s2 = strchr(principal, '.');
    if (s2 != NULL && s2 < s1) {
      (void) strncpy(instance, s2, s1 - s2);
      instance[s1 - s2] = '\0';
      (void) strncpy(name, principal, s2 - principal);
      name[s2 - principal] = '\0';
    }
    else {   /* s2 */
      (void) strcpy(instance, ".");
      (void) strncpy(name, principal, s1 - principal);
      name[s1 - principal] = '\0';
    }
  }    
  else {   /* s1 */
    (void) strcat(strcpy(realm, "@"), KRB_REALM);
    s2 = strchr(principal, '.');
    if (s2) {
      (void) strcpy(instance, s2);
      (void) strncpy(name, principal, s2 - principal);
      name[s2 - principal] = '\0';
    }
    else {   /* s2 */
      (void) strcpy(instance, ".");
      (void) strcpy(name, principal);
    }
  }
  return(strcat(strcat(strcpy(buf, name), instance), realm));
}

_acl_match(criterion, sample)
     char *criterion, *sample;
{
  register char *c, *s;

  c = criterion; s = sample;

  while(TRUE) {
    if (*c == *s && *s == '\0') return(TRUE);
    if (*c == '*') {
      if (*(c+1) == '\0') return(TRUE);
      while(*s != '\0')
        if (_acl_match(c+1, s++)) return(TRUE);
      return(FALSE);
    }
    if (*c != *s) return(FALSE);
    c++; s++;
  }
}

acl_check(acl, principal)
     char *acl;
     char *principal;
{
  FILE *fp;
  char buf[MAX_K_NAME_SZ], canon[MAX_K_NAME_SZ];

  fp = fopen(acl, "r");
  if (!fp) return(0);
  (void) acl_canonicalize_principal(principal, canon);

  while(fgets(buf, MAX_K_NAME_SZ, fp)) {
    buf[strlen(buf)-1] = '\0';   /* strip trailing newline */
    if (_acl_match(buf, canon)) {
      (void) fclose(fp);
      return(1);
    }
  }
  (void) fclose(fp);
  return(0);
}

acl_exact_match(acl, principal)
     char *acl;
     char *principal;
{
  FILE *fp;
  char buf[MAX_K_NAME_SZ];

  fp = fopen(acl, "r");
  if (!fp) return(0);

  while(fgets(buf, MAX_K_NAME_SZ, fp)) {
    buf[strlen(buf)-1] = '\0';
    if (!strcmp(buf, principal)) {
      (void) fclose(fp);
      return(1);
    }
  }
  (void) fclose(fp);
  return(0);
}

acl_add(acl, principal)
     char *acl;
     char *principal;
{
  FILE *fp;
  char canon[MAX_K_NAME_SZ];

  (void) acl_canonicalize_principal(principal, canon);
  if (acl_exact_match(acl, principal)) return(ACL_ALREADY);

  fp = fopen(acl, "a");
  if (!fp) return(1);
  
  fputs(canon, fp);
  fputs("\n", fp);
  if (fclose(fp) == EOF) return(1);
  return(0);
}

acl_delete(acl, principal)
     char *acl;
     char *principal;
{
  FILE *fp1, *fp2;
  char canon[MAX_K_NAME_SZ], buf[MAX_K_NAME_SZ];
  char *tmpf;
  int retval = ACL_ALREADY;

  fp1 = fopen(acl, "r");
  if (!fp1) return(1);

  if ((tmpf = (char *)malloc((unsigned)strlen(acl)+5)) == NULL)
    return(1);
  fp2 = fopen(strcat(strcpy(tmpf, acl), ".tmp"), "w");
  if (!fp2) return(1);

  (void) strcat(acl_canonicalize_principal(principal, canon), "\n");
  while(fgets(buf, MAX_K_NAME_SZ, fp1)) {
    if (strcmp(buf, canon)) {
      fputs(buf, fp2);
    }
    else retval = 0;
  }
  (void) fclose(fp1);
  if (fclose(fp2) == EOF) {
    (void) unlink(tmpf);
    free(tmpf);
    return(1);
  }
  retval |= rename(tmpf, acl);
  free(tmpf);
  return(retval);
}

acl_initialize(acl, mode)
     char *acl;
     int mode;
{
  FILE *fp;

  if ((fp = fopen(acl, "w")) == NULL) return(1);
  (void) fclose(fp);

  return(chmod(acl, mode));
}
