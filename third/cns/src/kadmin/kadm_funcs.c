/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Kerberos administration server-side database manipulation routines
 */

#include <mit-copyright.h>
/*
kadm_funcs.c
the actual database manipulation code
*/

#include <kadm.h>
#include <kadm_err.h>
#include <krb_db.h>
#include "kadm_server.h"

#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#ifdef NDBM
#include <ndbm.h>
#else /*NDBM*/
#include <dbm.h>
#endif /*NDBM*/

#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef POSIX
#include <unistd.h>
#endif
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#ifdef NO_STRERROR
extern char *sys_errlist[];
#define strerror(x)	sys_errlist[x]
#endif

extern char *malloc ();

extern Kadm_Server server_parm;
extern char *gecos_file;

#ifndef NDBM
typedef char DBM;

#define dbm_open(file, flags, mode) ((dbminit(file) == 0)?"":((char *)0))
#define dbm_fetch(db, key) fetch(key)
#define dbm_store(db, key, content, flag) store(key, content)
#define dbm_firstkey(db) firstkey()
#define dbm_next(db,key) nextkey(key)
#define dbm_close(db) dbmclose()
#else
#define dbm_next(db,key) dbm_nextkey(db)
#endif

check_access(pname, pinst, prealm, acltype)
char *pname;
char *pinst;
char *prealm;
enum acl_types acltype;
{
    char checkname[MAX_K_NAME_SZ];
    char filename[MAXPATHLEN];
    extern char *acldir;

    (void) sprintf(checkname, "%s.%s@%s", pname, pinst, prealm);
    
    switch (acltype) {
    case ADDACL:
	(void) sprintf(filename, "%s%s", acldir, ADD_ACL_FILE);
	break;
    case GETACL:
	(void) sprintf(filename, "%s%s", acldir, GET_ACL_FILE);
	break;
    case MODACL:
	(void) sprintf(filename, "%s%s", acldir, MOD_ACL_FILE);
	break;
    case STABACL:
	(void) sprintf(filename, "%s%s", acldir, STAB_ACL_FILE);
	break;
    case DELACL:
	(void) sprintf(filename, "%s%s", acldir, DEL_ACL_FILE);
	break;
    }
    return(acl_check(filename, checkname));
}

check_restrict_access (name, inst)
     char *name, *inst;
{
    extern char *acldir;
    static char filename[MAXPATHLEN];
    char checkname[MAX_K_NAME_SZ];

    if (filename[0] == 0)
	sprintf (filename, "%s%s", acldir, RESTRICT_ACL_FILE);
    if (access (filename, F_OK) != 0)
	return 1;
    if (access (filename, R_OK) != 0) {
	krb_log ("can't read %s, rejecting admin access", filename);
	return 0;
    }

    sprintf (checkname, "%s.%s", name, inst);
    return ! acl_check (filename, checkname);
}

int
wildcard(str)
char *str;
{
    if (!strcmp(str, WILDCARD_STR))
	return(1);
    return(0);
}

#define failadd(code) {  (void) krb_log("FAILED adding '%s.%s' (%s)", valsin->name, valsin->instance, error_message(code)); return code; }

kadm_add_entry (rname, rinstance, rrealm, valsin, valsout)
char *rname;				/* requestors name */
char *rinstance;			/* requestors instance */
char *rrealm;				/* requestors realm */
Kadm_vals *valsin;
Kadm_vals *valsout;
{
  long numfound;		/* check how many we get written */
  int more;			/* pointer to more grabbed records */
  Principal data_i, data_o;		/* temporary principal */
  u_char flags[4];
  des_cblock newpw;
  Principal default_princ;

  if (!check_access(rname, rinstance, rrealm, ADDACL)) {
    (void) krb_log("WARNING: '%s.%s@%s' tried to add an entry for '%s.%s'",
	       rname, rinstance, rrealm, valsin->name, valsin->instance);
    return KADM_UNAUTH;
  }
  
  /* Need to check here for "legal" name and instance */
  if (wildcard(valsin->name) || wildcard(valsin->instance)) {
      failadd(KADM_ILL_WILDCARD);
  }

  if (!check_restrict_access(valsin->name, valsin->instance)) {
    krb_log("rejected request from `%s.%s@%s' to add `%s.%s'",
	    rname, rinstance, rrealm, valsin->name, valsin->instance);
    return KADM_UNAUTH;
  }

  (void) krb_log("request to add an entry for '%s.%s' from '%s.%s@%s'",
		 valsin->name, valsin->instance, rname, rinstance, rrealm);
  
  numfound = kerb_get_principal(KERB_DEFAULT_NAME, KERB_DEFAULT_INST,
				&default_princ, 1, &more);
  if (numfound == -1) {
      failadd(KADM_DB_INUSE);
  } else if (numfound != 1) {
      failadd(KADM_UK_RERROR);
  }

  kadm_vals_to_prin(valsin->fields, &data_i, valsin);
  (void) strncpy(data_i.name, valsin->name, ANAME_SZ);
  (void) strncpy(data_i.instance, valsin->instance, INST_SZ);

  if (!IS_FIELD(KADM_EXPDATE,valsin->fields))
	  data_i.exp_date = default_princ.exp_date;
  if (!IS_FIELD(KADM_ATTR,valsin->fields))
      data_i.attributes = default_princ.attributes;
  if (!IS_FIELD(KADM_MAXLIFE,valsin->fields))
      data_i.max_life = default_princ.max_life; 

  memset((char *)&default_princ, 0, sizeof(default_princ));

  /* convert to host order */
  data_i.key_low = ntohl(data_i.key_low);
  data_i.key_high = ntohl(data_i.key_high);


  memcpy(newpw, &data_i.key_low, sizeof(KRB_INT32));
  memcpy((char *)(((KRB_INT32 *) newpw) + 1), &data_i.key_high, sizeof(KRB_INT32));

  /* encrypt new key in master key */
  kdb_encrypt_key (newpw, newpw, server_parm.master_key,
		     server_parm.master_key_schedule, ENCRYPT);
  memcpy(&data_i.key_low, newpw, sizeof(KRB_INT32));
  memcpy(&data_i.key_high, (char *)(((KRB_INT32 *) newpw) + 1), sizeof(KRB_INT32));
  memset((char *)newpw, 0, sizeof(newpw));

  data_o = data_i;
  numfound = kerb_get_principal(valsin->name, valsin->instance, 
				&data_o, 1, &more);
  if (numfound == -1) {
      failadd(KADM_DB_INUSE);
  } else if (numfound) {
      failadd(KADM_INUSE);
  } else {
    data_i.key_version++;
    data_i.kdc_key_ver = server_parm.master_key_version;
    (void) strncpy(data_i.mod_name, rname, sizeof(data_i.mod_name)-1);
    (void) strncpy(data_i.mod_instance, rinstance,
		   sizeof(data_i.mod_instance)-1);

    numfound = kerb_put_principal(&data_i, 1);
    if (numfound == -1) {
	failadd(KADM_DB_INUSE);
    } else if (numfound) {
	failadd(KADM_UK_SERROR);
    } else {
      numfound = kerb_get_principal(valsin->name, valsin->instance, 
				    &data_o, 1, &more);
      if ((numfound!=1) || (more!=0)) {
	  failadd(KADM_UK_RERROR);
      }
      memset((char *)flags, 0, sizeof(flags));
      SET_FIELD(KADM_NAME,flags);
      SET_FIELD(KADM_INST,flags);
      SET_FIELD(KADM_EXPDATE,flags);
      SET_FIELD(KADM_ATTR,flags);
      SET_FIELD(KADM_MAXLIFE,flags);
      kadm_prin_to_vals(flags, valsout, &data_o);
      (void) krb_log("'%s.%s' added.", valsin->name, valsin->instance);
      return KADM_DATA;		/* Set all the appropriate fields */
    }
  }
}
#undef failadd

#define faildel(code) {  (void) krb_log("FAILED deleting '%s.%s' (%s)", valsin->name, valsin->instance, error_message(code)); return code; }

kadm_del_entry (rname, rinstance, rrealm, valsin, valsout)
char *rname;				/* requestors name */
char *rinstance;			/* requestors instance */
char *rrealm;				/* requestors realm */
Kadm_vals *valsin;
Kadm_vals *valsout;
{
  long numfound;		/* check how many we get written */
  int more;			/* pointer to more grabbed records */
  Principal data_i, data_o;		/* temporary principal */
  u_char flags[4];
  des_cblock newpw;
  Principal default_princ;

  if (!check_access(rname, rinstance, rrealm, DELACL)) {
    (void) krb_log("WARNING: '%s.%s@%s' tried to delete an entry for '%s.%s'",
	       rname, rinstance, rrealm, valsin->name, valsin->instance);
    return KADM_UNAUTH;
  }
  
  /* Need to check here for "legal" name and instance */
  if (wildcard(valsin->name) || wildcard(valsin->instance)) {
      faildel(KADM_ILL_WILDCARD);
  }

  if (!check_restrict_access(valsin->name, valsin->instance)) {
    krb_log("rejected request from `%s.%s@%s' to add `%s.%s'",
	    rname, rinstance, rrealm, valsin->name, valsin->instance);
    return KADM_UNAUTH;
  }

  (void) krb_log("request to delete an entry for '%s.%s' from '%s.%s@%s'",
		 valsin->name, valsin->instance, rname, rinstance, rrealm);
  
  numfound = kerb_get_principal(valsin->name, valsin->instance, 
				&data_o, 1, &more);
  if (numfound == -1) {
    faildel(KADM_DB_INUSE);
  } else if (numfound) {
    kerb_del_principal(valsin->name, valsin->instance, &data_o, 1, &more);
    memset((char *)flags, 0, sizeof(flags));
    SET_FIELD(KADM_NAME,flags);
    SET_FIELD(KADM_INST,flags);
    SET_FIELD(KADM_EXPDATE,flags);
    SET_FIELD(KADM_ATTR,flags);
    SET_FIELD(KADM_MAXLIFE,flags);
    kadm_prin_to_vals(flags, valsout, &data_o);
    (void) krb_log("'%s.%s' deleted.", valsin->name, valsin->instance);
    return KADM_DATA;		/* Set all the appropriate fields */
  } else {
    faildel(KADM_INUSE);
  }
}
#undef faildel

#define failget(code) {  (void) krb_log("FAILED retrieving '%s.%s' (%s)", valsin->name, valsin->instance, error_message(code)); return code; }

kadm_get_entry (rname, rinstance, rrealm, valsin, flags, valsout)
char *rname;				/* requestors name */
char *rinstance;			/* requestors instance */
char *rrealm;				/* requestors realm */
Kadm_vals *valsin;			/* what they wannt to get */
u_char *flags;				/* which fields we want */
Kadm_vals *valsout;			/* what data is there */
{
  long numfound;		/* check how many were returned */
  int more;			/* To point to more name.instances */
  Principal data_o;		/* Data object to hold Principal */

  
  if (!check_access(rname, rinstance, rrealm, GETACL)) {
    (void) krb_log("WARNING: '%s.%s@%s' tried to get '%s.%s's entry",
	    rname, rinstance, rrealm, valsin->name, valsin->instance);
    return KADM_UNAUTH;
  }
  
  if (wildcard(valsin->name) || wildcard(valsin->instance)) {
      failget(KADM_ILL_WILDCARD);
  }

  (void) krb_log("retrieve '%s.%s's entry for '%s.%s@%s'",
	     valsin->name, valsin->instance, rname, rinstance, rrealm);
  
  /* Look up the record in the database */
  numfound = kerb_get_principal(valsin->name, valsin->instance, 
				&data_o, 1, &more);
  if (numfound == -1) {
      failget(KADM_DB_INUSE);
  }  else if (numfound) {	/* We got the record, let's return it */
    kadm_prin_to_vals(flags, valsout, &data_o);
    (void) krb_log("'%s.%s' retrieved.", valsin->name, valsin->instance);
    return KADM_DATA;		/* Set all the appropriate fields */
  } else {
      failget(KADM_NOENTRY);	/* Else whimper and moan */
  }
}
#undef failget

#define failmod(code) {  (void) krb_log("FAILED modifying '%s.%s' (%s)", valsin1->name, valsin1->instance, error_message(code)); return code; }

kadm_mod_entry (rname, rinstance, rrealm, valsin1, valsin2, valsout)
char *rname;				/* requestors name */
char *rinstance;			/* requestors instance */
char *rrealm;				/* requestors realm */
Kadm_vals *valsin1, *valsin2;		/* holds the parameters being
					   passed in */
Kadm_vals *valsout;		/* the actual record which is returned */
{
  long numfound;
  int more;
  Principal data_o, temp_key;
  u_char fields[4];
  des_cblock newpw;

  if (wildcard(valsin1->name) || wildcard(valsin1->instance)) {
      failmod(KADM_ILL_WILDCARD);
  }
  
  if (!check_access(rname, rinstance, rrealm, MODACL)) {
    (void) krb_log("WARNING: '%s.%s@%s' tried to change '%s.%s's entry",
	       rname, rinstance, rrealm, valsin1->name, valsin1->instance);
    return KADM_UNAUTH;
  }
  
  if (!check_restrict_access(valsin1->name, valsin1->instance)) {
    krb_log("rejected request from `%s.%s@%s' to add `%s.%s'",
	    rname, rinstance, rrealm, valsin1->name, valsin1->instance);
    return KADM_UNAUTH;
  }

  (void) krb_log("request to modify '%s.%s's entry from '%s.%s@%s' ",
	     valsin1->name, valsin1->instance, rname, rinstance, rrealm);
  
  numfound = kerb_get_principal(valsin1->name, valsin1->instance, 
				&data_o, 1, &more);
  if (numfound == -1) {
      failmod(KADM_DB_INUSE);
  } else if (numfound) {
      kadm_vals_to_prin(valsin2->fields, &temp_key, valsin2);
      (void) strncpy(data_o.name, valsin1->name, ANAME_SZ);
      (void) strncpy(data_o.instance, valsin1->instance, INST_SZ);
      if (IS_FIELD(KADM_EXPDATE,valsin2->fields))
	  data_o.exp_date = temp_key.exp_date;
      if (IS_FIELD(KADM_ATTR,valsin2->fields))
	  data_o.attributes = temp_key.attributes;
      if (IS_FIELD(KADM_MAXLIFE,valsin2->fields))
	  data_o.max_life = temp_key.max_life; 
      if (IS_FIELD(KADM_DESKEY,valsin2->fields)) {
	  data_o.key_version++;
	  data_o.kdc_key_ver = server_parm.master_key_version;


	  /* convert to host order */
	  temp_key.key_low = ntohl(temp_key.key_low);
	  temp_key.key_high = ntohl(temp_key.key_high);


	  memcpy(newpw, &temp_key.key_low, sizeof(KRB_INT32));
	  memcpy((char *)(((KRB_INT32 *) newpw) + 1), &temp_key.key_high, sizeof(KRB_INT32));

	  /* encrypt new key in master key */
	  kdb_encrypt_key (newpw, newpw, server_parm.master_key,
			   server_parm.master_key_schedule, ENCRYPT);
	  memcpy(&data_o.key_low, newpw, sizeof(KRB_INT32));
	  memcpy(&data_o.key_high, (char *)(((KRB_INT32 *) newpw) + 1), sizeof(KRB_INT32));
	  memset((char *)newpw, 0, sizeof(newpw));
      }
      memset((char *)&temp_key, 0, sizeof(temp_key));

      (void) strncpy(data_o.mod_name, rname, sizeof(data_o.mod_name)-1);
      (void) strncpy(data_o.mod_instance, rinstance,
		     sizeof(data_o.mod_instance)-1);
      more = kerb_put_principal(&data_o, 1);

      memset((char *)&data_o, 0, sizeof(data_o));

      if (more == -1) {
	  failmod(KADM_DB_INUSE);
      } else if (more) {
	  failmod(KADM_UK_SERROR);
      } else {
	  numfound = kerb_get_principal(valsin1->name, valsin1->instance, 
					&data_o, 1, &more);
	  if ((more!=0)||(numfound!=1)) {
	      failmod(KADM_UK_RERROR);
	  }
	  memset((char *) fields, 0, sizeof(fields));
	  SET_FIELD(KADM_NAME,fields);
	  SET_FIELD(KADM_INST,fields);
	  SET_FIELD(KADM_EXPDATE,fields);
	  SET_FIELD(KADM_ATTR,fields);
	  SET_FIELD(KADM_MAXLIFE,fields);
	  kadm_prin_to_vals(fields, valsout, &data_o);
	  (void) krb_log("'%s.%s' modified.", valsin1->name, valsin1->instance);
	  return KADM_DATA;		/* Set all the appropriate fields */
      }
  }
  else {
      failmod(KADM_NOENTRY);
  }
}
#undef failmod

#define failchange(code) {  (void) krb_log("FAILED changing key for '%s.%s@%s' (%s)", rname, rinstance, rrealm, error_message(code)); return code; }

kadm_change (rname, rinstance, rrealm, newpw)
char *rname;
char *rinstance;
char *rrealm;
des_cblock newpw;
{
  long numfound;
  int more;
  Principal data_o;
  des_cblock local_pw;

  if (strcmp(server_parm.krbrlm, rrealm)) {
      (void) krb_log("change key request from wrong realm, '%s.%s@%s'!\n",
		 rname, rinstance, rrealm);
      return(KADM_WRONG_REALM);
  }

  if (wildcard(rname) || wildcard(rinstance)) {
      failchange(KADM_ILL_WILDCARD);
  }
  (void) krb_log("'%s.%s@%s' wants to change its password",
	     rname, rinstance, rrealm);
  
  memcpy(local_pw, newpw, sizeof(local_pw));
  
  /* encrypt new key in master key */
  kdb_encrypt_key (local_pw, local_pw, server_parm.master_key,
		     server_parm.master_key_schedule, ENCRYPT);

  numfound = kerb_get_principal(rname, rinstance, 
				&data_o, 1, &more);
  if (numfound == -1) {
      failchange(KADM_DB_INUSE);
  } else if (numfound) {
    memcpy(&data_o.key_low, local_pw, sizeof(KRB_INT32));
    memcpy(&data_o.key_high, (char *)(((KRB_INT32 *) local_pw) + 1), sizeof(KRB_INT32));
    data_o.key_version++;
    data_o.kdc_key_ver = server_parm.master_key_version;
    (void) strncpy(data_o.mod_name, rname, sizeof(data_o.mod_name)-1);
    (void) strncpy(data_o.mod_instance, rinstance,
		   sizeof(data_o.mod_instance)-1);
    more = kerb_put_principal(&data_o, 1);
    memset((char *) local_pw, 0, sizeof(local_pw));
    memset((char *) &data_o, 0, sizeof(data_o));
    if (more == -1) {
	failchange(KADM_DB_INUSE);
    } else if (more) {
	failchange(KADM_UK_SERROR);
    } else {
	(void) krb_log("'%s.%s@%s' password changed.", rname, rinstance, rrealm);
	return KADM_SUCCESS;
    }
  }
  else {
      failchange(KADM_NOENTRY);
  }
}
#undef failchange

static void sfree (s)
	char *s;
{
	if (s == NULL)
		return;
	memset (s, 0, strlen (s));
	free (s);
}

check_pw(newpw, checkstr)
	des_cblock	newpw;
	char		*checkstr;
{
	des_cblock	checkdes;

#ifdef NOENCRYPTION
	memcpy((char *) checkdes, (char *)newpw, sizeof(newpw));
#else
	(void) des_string_to_key(checkstr, checkdes);
#endif
	return(!memcmp(checkdes, newpw, sizeof(des_cblock)));
}

char *reverse(str)
	char	*str;
{
	static char newstr[80];
	char	*p, *q;
	int	i;

	i = strlen(str);
	if (i >= sizeof(newstr))
		i = sizeof(newstr)-1;
	p = str+i-1;
	q = newstr;
	q[i]='\0';
	for(; i > 0; i--) 
		*q++ = *p--;
	
	return(newstr);
}

int lower(str)
	char	*str;
{
	register char	*cp;
	int	effect=0;

	for (cp = str; *cp; cp++) {
		if (isupper(*cp)) {
			*cp = tolower(*cp);
			effect++;
		}
	}
	return(effect);
}

#define CLEANUP
#define BADPW(REASON)	{ /* krb_log("rejecting password change: %s", (REASON)); */ CLEANUP; return KADM_INSECURE_PW; }

des_check_gecos(gecos, newpw)
	char	*gecos;
	des_cblock newpw;
{
	char		*cp, *ncp, *tcp;
	
	for (cp = gecos; *cp; ) {
		/* Skip past punctuation */
		for (; *cp; cp++)
			if (isalnum(*cp))
				break;
		/* Skip to the end of the word */
		for (ncp = cp; *ncp; ncp++)
			if (!isalnum(*ncp) && *ncp != '\'')
				break;
		/* Delimit end of word */
		if (*ncp)
			*ncp++ = '\0';
		/* Check word to see if it's the password */
		if (*cp) {
			if (check_pw(newpw, cp))
				BADPW ("matches gecos field");
			tcp = reverse(cp);
			if (check_pw(newpw, tcp))
				BADPW ("matches reversed gecos field");
			if (lower(cp)) {
				if (check_pw(newpw, cp))
					BADPW ("matches gecos field (lowercase)");
				tcp = reverse(cp);
				if (check_pw(newpw, tcp))
					BADPW ("matches reversed gecos field (lowercase)");
			}
			cp = ncp;				
		} else
			break;
	}
	return(0);
}

static char *
copy_downcase (s)
	char *s;
{
	char *new, *s2;
	new = malloc (1 + strlen (s));
	if (new == NULL)
		return NULL;
	s2 = new;
	while (1) {
		*s2 = (isalpha (*s) && isupper (*s)) ? tolower (*s) : *s;
		if (*s == 0)
			break;
		s++, s2++;
	}
	return new;
}

#define DEBUG_PW 0
static int
check_substrings (buf, s, min)
	char *buf, *s;
	int min;
{
	char *x;
	if (DEBUG_PW)
	  fprintf (stderr, "check_substrings(%s,%s,%d):", buf, s, min);
	if (strlen (s) < min) {
		if (DEBUG_PW)
		  fprintf (stderr, "ok\n");
		return 0;
	}
	x = s + strlen (s) - min;
	while (x >= s) {
		if (DEBUG_PW)
		  fprintf (stderr, " `%s'", x);
		if (strstr (buf, x)) {
			if (DEBUG_PW)
			  fprintf (stderr, "fail\n");
			return 1;
		}
		x[min-1] = 0;
		/* @@ Technically, pointing to the byte before the
		   string is a no-no, but I think it'll be okay.  */
		x--;
	}
	if (DEBUG_PW)
	  fprintf (stderr, "ok\n");
	return 0;
}

str_check_gecos(gecos, pwstr)
	char	*gecos;
	char	*pwstr;
{
	char		*cp, *ncp, *tcp;

#define COPY_DOWN(X) \
	if (NULL == (X = copy_downcase(X))) return errno;

	COPY_DOWN (gecos);
	COPY_DOWN (pwstr);

#undef CLEANUP
#define CLEANUP { sfree (gecos); sfree (pwstr); }

	for (cp = gecos; *cp; ) {
		/* Skip past punctuation */
		for (; *cp; cp++)
			if (isalnum(*cp))
				break;
		/* Have we hit the end?  */
		if (!*cp)
			break;
		/* Skip to the end of the word */
		for (ncp = cp; *ncp; ncp++)
			if (!isalnum(*ncp) && *ncp != '\'')
				break;
		/* Delimit end of word */
		if (*ncp)
			*ncp++ = '\0';
		/* Check word to see if it's the password */
		if (!strcmp(pwstr, cp))
			BADPW ("matches GECOS field");
		tcp = reverse(cp);
		if (!strcmp(pwstr, tcp))
			BADPW ("matches reversed GECOS field");
		/* If it's long enough, check to see if it's a
		   substring of the password.  (Minimum length 3, or
		   4-char substrings.)

		   This test overwrites the word of the GECOS field
		   we're examining, so keep it at the end or make it
		   operate on a copy.  */
		if (strlen (cp) >= 3) {
			if (strstr (pwstr, cp))
				BADPW ("contains GECOS word");
			if (strstr (pwstr, reverse (cp)))
				BADPW ("contains reversed GECOS word");
		}
		if (strlen (cp) > 4 && check_substrings (pwstr, cp, 4))
			BADPW ("contains GECOS substring");

		/* Move on to next word.  */
		cp = ncp;
	}
	return 0;
}
#undef CLEANUP

kadm_approve_pw(rname, rinstance, rrealm, newpw, pwstring)
char *rname;
char *rinstance;
char *rrealm;
des_cblock newpw;
char *pwstring;
{
	static DBM *pwfile = NULL;
	static FILE *gecos = NULL;
	int		retval;
	char *rname2 = 0, *pwstring2 = 0, *copy = 0;
	datum		passwd, entry;
	struct passwd	*ent;
#ifdef HESIOD
	extern struct passwd *hes_getpwnam();
#endif
	
#define CLEANUP { sfree (rname2); sfree (pwstring2); sfree (copy); }

	if (pwstring && !check_pw(newpw, pwstring))
		/*
		 * Someone's trying to toy with us....
		 */
		return(KADM_PW_MISMATCH);
	if (pwstring) {
		char *s, *s2;
		int n_alpha, n_nonalpha, diff;

		if (strlen (pwstring) < 6)
			/* Too short.  */
			BADPW ("less than six chars long");

		copy = malloc (1 + strlen (pwstring));
		if (copy == NULL)
			/* ??? */
			return errno;

		/* Check for at least two alpha and one non-alpha
		   character.  */
		n_alpha = 0, n_nonalpha = 0;
		for (s = pwstring, s2 = copy; *s; s++, s2++) {
			if (isalpha (*s)) {
				n_alpha++;
				*s2 = isupper (*s) ? tolower (*s) : *s;
			} else {
				n_nonalpha++;
				*s2 = *s;
			}
		}
		*s2 = 0;
		if (n_alpha < 2 || n_nonalpha < 1)
			BADPW ("needs at least 2 alpha, 1 non-alpha chars");
		/* Check for no runs.  Checking s[2] is okay here
		   since by the previous test we know that there must
		   be at least three characters in the string.  But if
		   you change that test, be careful checking here.  */
		for (s = copy; s[2]; s++) {
			diff = s[0] - s[1];
			if (diff != 1 && diff != -1)
				continue;
			if (diff == s[1] - s[2])
				BADPW ("contains a run");
		}
	}
	/* Check against a database of bad passwords.  */
	if (!pwfile) {
		pwfile = dbm_open(PW_CHECK_FILE, O_RDONLY, 0644);
	}
	if (pwfile) {
		passwd.dptr = (char *) newpw;
		passwd.dsize = 8;
		entry = dbm_fetch(pwfile, passwd);
		if (entry.dptr)
			BADPW ("in bad-password dictionary");
	}
	if (pwstring) {
		if (check_pw  (pwstring, rname))
			BADPW ("matches principal name");
		if (check_pw (pwstring, reverse (rname)))
			BADPW ("matches reversed principal name");
	} else {
		/* check against string2key(rname) et al. later. */
	}
	if (pwstring) {
		rname2 = copy_downcase (rname);
		pwstring2 = copy_downcase (pwstring);
		if (strstr (pwstring2, rname2))
			BADPW ("contains principal name");
		if (strstr (pwstring2, reverse (rname2)))
			BADPW ("contains reverse of principal name");
		if (check_substrings (pwstring2, rname2, 3))
			BADPW ("contains substring of principal name");
	}
	if (gecos_file) {
#ifndef HAVE_FGETPWENT
		/* Shouldn't get here.  */
		abort ();
#else
		extern struct passwd *fgetpwent ();
		ent = NULL;
		if (gecos == NULL) {
			gecos = fopen (gecos_file, "r");
			if (gecos == NULL) {
				krb_log ("couldn't open GECOS file %s: %s",
					 gecos_file, strerror (errno));
				goto no_pwent;
			}
		}
		else
			rewind (gecos);
		while (ent = fgetpwent (gecos))
			if (!strcmp (ent->pw_name, rname))
				break;
	      no_pwent:
		;
#endif
	} else {
#ifdef HESIOD
		ent = hes_getpwnam(rname);
#else
		ent = getpwnam(rname);
#endif
	}
	if (ent && ent->pw_gecos) {
		if (pwstring)
			retval = str_check_gecos(ent->pw_gecos, pwstring);
		else
			retval = des_check_gecos(ent->pw_gecos, newpw);
		if (retval)
			return retval;
	}
	CLEANUP;
	return 0;
}
#undef CLEANUP
#define CLEANUP

/*
 * This routine checks to see if a principal should be considered an
 * allowable service name which can be changed by kadm_change_srvtab.
 *
 * We do this check by using the ACL library.  This makes the
 * (relatively) reasonable assumption that both the name and the
 * instance will  not contain '.' or '@'. 
 */
kadm_check_srvtab(name, instance)
	char	*name;
	char	*instance;
{
	FILE	*f;
	char filename[MAXPATHLEN];
	char buf[ANAME_SZ], *cp;
	extern char *acldir;

	(void) sprintf(filename, "%s%s", acldir, STAB_SERVICES_FILE);
	if (!acl_check(filename, name))
		return(KADM_NOT_SERV_PRINC);

	(void) sprintf(filename, "%s%s", acldir, STAB_HOSTS_FILE);
	if (acl_check(filename, instance))
		return(KADM_NOT_SERV_PRINC);
	return 0;
}

/*
 * Routine to allow some people to change the key of a srvtab
 * principal to a random key, which the admin server will return to
 * the client.
 */
#define failsrvtab(code) {  (void) krb_log("change_srvtab: FAILED changing '%s.%s' by '%s.%s@%s' (%s)", values->name, values->instance, rname, rinstance, rrealm, error_message(code)); return code; }

kadm_chg_srvtab(rname, rinstance, rrealm, values)
	char *rname;				/* requestors name */
	char *rinstance;			/* requestors instance */
	char *rrealm;				/* requestors realm */
	Kadm_vals *values;
{
	int	numfound, more, ret;
	des_cblock new_key;
	Principal principal;
	
	if (!check_access(rname, rinstance, rrealm, STABACL))
		failsrvtab(KADM_UNAUTH);
	if (wildcard(rname) || wildcard(rinstance))
		failsrvtab(KADM_ILL_WILDCARD);
	if (ret = kadm_check_srvtab(values->name, values->instance))
		failsrvtab(ret);

	/*
	 * OK, get the entry
	 */
	numfound = kerb_get_principal(values->name, values->instance, 
				      &principal, 1, &more);
	if (numfound == -1) {
		failsrvtab(KADM_DB_INUSE);
	} else if (numfound) {
		principal.key_version++;
	} else {
		/*
		 * This is a new srvtab entry that we're creating
		 */
		strncpy(principal.name, values->name, ANAME_SZ);
		strncpy(principal.instance, values->instance, INST_SZ);
		
		principal.exp_date = 946702799+((365*10+3)*24*60*60);
		strncpy(principal.exp_date_txt, "12/31/2009", DATE_SZ);

		principal.attributes = 0;
		principal.max_life = 255;

		principal.key_version = 1;
	}
		
#ifdef NOENCRYPTION
	memset(new_key, 0, sizeof(new_key));
	new_key[0] = 127;
#else
	des_new_random_key(new_key);
#endif
	/*
	 * Store the new key in the return structure; also fill in the
	 * rest of the fields.
	 */
	memcpy(&values->key_low, new_key, sizeof(KRB_INT32));
	memcpy(&values->key_high, (char *)(((KRB_INT32 *) new_key) + 1), sizeof(KRB_INT32));

	/* convert to network order */
	values->key_low = htonl(values->key_low);
	values->key_high = htonl(values->key_high);

	values->max_life = principal.key_version;
	values->exp_date = principal.exp_date;
	values->attributes = principal.attributes;
	memset(values->fields, 0, sizeof(values->fields));
	SET_FIELD(KADM_NAME, values->fields);
	SET_FIELD(KADM_INST, values->fields);
	SET_FIELD(KADM_EXPDATE, values->fields);
	SET_FIELD(KADM_ATTR, values->fields);
	SET_FIELD(KADM_MAXLIFE, values->fields);
	SET_FIELD(KADM_DESKEY, values->fields);
	
	/*
	 * Encrypt the new key with the master key, and then update
	 * the database record
	 */
	kdb_encrypt_key (new_key, new_key, server_parm.master_key,
		     server_parm.master_key_schedule, ENCRYPT);
	memcpy(&principal.key_low, new_key, sizeof(KRB_INT32));
	memcpy(&principal.key_high, (char *)(((KRB_INT32 *) new_key) + 1), sizeof(KRB_INT32));
	memset(new_key, 0, sizeof(des_cblock));

	principal.kdc_key_ver = server_parm.master_key_version;

	principal.mod_date = time(0);
	principal.old = 0;
	strncpy(principal.mod_name, rname, ANAME_SZ);
	strncpy(principal.mod_instance, rinstance, INST_SZ);
  
	more = kerb_put_principal(&principal, 1);
	if (more == -1) {
		failsrvtab(KADM_DB_INUSE);
	} else if (more) {
		failsrvtab(KADM_UK_SERROR);
	} else {
		(void) krb_log("change_srvtab: service '%s.%s' %s by %s.%s@%s.",
			   values->name, values->instance,
			   numfound ? "changed" : "created",
			   rname, rinstance, rrealm);
		return KADM_DATA;
	}
}

#undef failsrvtab
