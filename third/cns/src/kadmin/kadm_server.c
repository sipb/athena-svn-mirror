/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Kerberos administration server-side subroutines
 */

#include <mit-copyright.h>

#include <string.h>

#include <kadm.h>
#include <kadm_err.h>

int fascist_cpw = 0;		/* Be fascist about insecure passwords? */

char bad_pw_err[] =
	"\007\007\007ERROR: Insecure password not accepted.  Please choose another.\n\n";

char bad_pw_warn[] =
	"\007\007\007WARNING: You have chosen an insecure password.  You may wish to\nchoose a better password.\n\n";

char check_pw_msg[] =
	"You have entered an insecure password.  You should choose another.\n\n";

char pw_blurb[] =
	"A good password is something which is easy for you to remember, but\nthat people who know you won't easily guess.  Don't use a word found\nin the dictionary, because someone may run a program that tries all\nthose words.  That same program will probably try the names of all\nknown comic-strip characters, rock bands, etc.  Please do not let\nanyone else, including friends, know your password.  Remember, *YOU*\nare assumed to be responsible for anything done using your password.\n";

/* 
kadm_ser_cpw - the server side of the change_password routine
  recieves    : KTEXT, {key}
  returns     : CKSUM, RETCODE
  acl         : caller can change only own password

Replaces the password (i.e. des key) of the caller with that specified in key.
Returns no actual data from the master server, since this is called by a user
*/
kadm_ser_cpw(dat, len, ad, datout, outlen)
u_char *dat;
int len;
AUTH_DAT *ad;
u_char **datout;
int *outlen;
{
    unsigned KRB_INT32 keylow, keyhigh;
    char pword[MAX_KPW_LEN];
    int no_pword = 0;
    des_cblock newkey;
    int status, stvlen = 0;
    int	retval;
    extern char *malloc();
    extern int kadm_approve_pw();

    /* take key off the stream, and change the database */

    if ((status = stv_long(dat, &keyhigh, 0, len)) < 0)
	return(KADM_LENGTH_ERROR);
    stvlen += status;
    if ((status = stv_long(dat, &keylow, stvlen, len)) < 0)
	return(KADM_LENGTH_ERROR);
    stvlen += status;
    if ((stvlen = stv_string(dat, pword, stvlen, sizeof(pword), len)) < 0) {
	no_pword++;
	pword[0]='\0';
    }
    stvlen += status;

    keylow = ntohl(keylow);
    keyhigh = ntohl(keyhigh);
    memcpy((char *)(((KRB_INT32 *)newkey) + 1), (char *)&keyhigh, sizeof(KRB_INT32));
    memcpy((char *)newkey, (char *)&keylow, sizeof(KRB_INT32));
    if (retval = kadm_approve_pw(ad->pname, ad->pinst, ad->prealm,
			newkey, no_pword ? 0 : pword)) {
	    if (retval == KADM_PW_MISMATCH) {
		    /*
		     * Very strange!!!  This means that the cleartext
		     * password which was sent and the DES cblock
		     * didn't match!
		     */
		    (void) krb_log("'%s.%s@%s' sent a password string which didn't match with the DES key?!?",
			       ad->pname, ad->pinst, ad->prealm);
		    return(retval);
	    }
	    if (fascist_cpw) {
		    *outlen = strlen(bad_pw_err)+strlen(pw_blurb)+1;
		    if (*datout = (u_char *) malloc(*outlen)) {
			    strcpy((char *) *datout, bad_pw_err);
			    strcat((char *) *datout, pw_blurb);
		    } else
			    *outlen = 0;
		    (void) krb_log("'%s.%s@%s' tried to use an insecure password in changepw",
			       ad->pname, ad->pinst, ad->prealm);
#ifdef notdef
		    /* For debugging only, probably a bad idea */
		    if (!no_pword)
			    (void) krb_log("The password was %s\n", pword);
#endif
		    return(retval);
	    } else {
		    *outlen = strlen(bad_pw_warn) + strlen(pw_blurb)+1;
		    if (*datout = (u_char *) malloc(*outlen)) {
			    strcpy((char *) *datout, bad_pw_warn);
			    strcat((char *) *datout, pw_blurb);
		    } else
			    *outlen = 0;
		    (void) krb_log("'%s.%s@%s' used an insecure password in changepw",
			       ad->pname, ad->pinst, ad->prealm);
	    }
    } else {
	    *datout = 0;
	    *outlen = 0;
    }

    return(kadm_change(ad->pname, ad->pinst, ad->prealm, newkey));
}

/*
kadm_ser_add - the server side of the add_entry routine
  recieves    : KTEXT, {values}
  returns     : CKSUM, RETCODE, {values}
  acl         : su, sms (as alloc)

Adds and entry containing values to the database
returns the values of the entry, so if you leave certain fields blank you will
   be able to determine the default values they are set to
*/
kadm_ser_add(dat,len,ad, datout, outlen)
u_char *dat;
int len;
AUTH_DAT *ad;
u_char **datout;
int *outlen;
{
  Kadm_vals values, retvals;
  int status;

  if ((status = stream_to_vals(dat, &values, len)) < 0)
      return(KADM_LENGTH_ERROR);
  if ((status = kadm_add_entry(ad->pname, ad->pinst, ad->prealm,
			      &values, &retvals)) == KADM_DATA) {
      *outlen = vals_to_stream(&retvals,datout);
      return KADM_SUCCESS;
  } else {
      *outlen = 0;
      return status;
  }
}

/*
kadm_ser_del - the server side of the del_entry routine
  recieves    : KTEXT, {values}
  returns     : CKSUM, RETCODE, {values}
  acl         : su, sms (as alloc)

Deletess an entry containing values to the database
returns the values of the entry, so if you leave certain fields blank you will
   be able to determine the default values they are set to
*/
kadm_ser_del(dat,len,ad, datout, outlen)
u_char *dat;
int len;
AUTH_DAT *ad;
u_char **datout;
int *outlen;
{
  Kadm_vals values, retvals;
  int status;

  if ((status = stream_to_vals(dat, &values, len)) < 0)
      return(KADM_LENGTH_ERROR);
  if ((status = kadm_del_entry(ad->pname, ad->pinst, ad->prealm,
			      &values, &retvals)) == KADM_DATA) {
      *outlen = vals_to_stream(&retvals,datout);
      return KADM_SUCCESS;
  } else {
      *outlen = 0;
      return status;
  }
}

/*
kadm_ser_mod - the server side of the mod_entry routine
  recieves    : KTEXT, {values, values}
  returns     : CKSUM, RETCODE, {values}
  acl         : su, sms (as register or dealloc)

Modifies all entries corresponding to the first values so they match the
   second values.
returns the values for the changed entries
*/
kadm_ser_mod(dat,len,ad, datout, outlen)
u_char *dat;
int len;
AUTH_DAT *ad;
u_char **datout;
int *outlen;
{
  Kadm_vals vals1, vals2, retvals;
  int wh;
  int status;

  if ((wh = stream_to_vals(dat, &vals1, len)) < 0)
      return KADM_LENGTH_ERROR;
  if ((status = stream_to_vals(dat+wh,&vals2, len-wh)) < 0)
      return KADM_LENGTH_ERROR;
  if ((status = kadm_mod_entry(ad->pname, ad->pinst, ad->prealm, &vals1,
			       &vals2, &retvals)) == KADM_DATA) {
      *outlen = vals_to_stream(&retvals,datout);
      return KADM_SUCCESS;
  } else {
      *outlen = 0;
      return status;
  }
}

/*
kadm_ser_get
  recieves   : KTEXT, {values, flags}
  returns    : CKSUM, RETCODE, {count, values, values, values}
  acl        : su

gets the fields requested by flags from all entries matching values
returns this data for each matching recipient, after a count of how many such
  matches there were
*/
kadm_ser_get(dat,len,ad, datout, outlen)
u_char *dat;
int len;
AUTH_DAT *ad;
u_char **datout;
int *outlen;
{
  Kadm_vals values, retvals;
  u_char fl[FLDSZ];
  int loop,wh;
  int status;

  if ((wh = stream_to_vals(dat, &values, len)) < 0)
      return KADM_LENGTH_ERROR;
  if (wh + FLDSZ > len)
      return KADM_LENGTH_ERROR;
  for (loop=FLDSZ-1; loop>=0; loop--)
    fl[loop] = dat[wh++];
  if ((status = kadm_get_entry(ad->pname, ad->pinst, ad->prealm,
			      &values, fl, &retvals)) == KADM_DATA) {
      *outlen = vals_to_stream(&retvals,datout);
      return KADM_SUCCESS;
  } else {
      *outlen = 0;
      return status;
  }
}

/* 
kadm_ser_ckpw - the server side of the check_password routine
  recieves    : KTEXT, {key}
  returns     : CKSUM, RETCODE
  acl         : none

Checks to see if the des key passed from the caller is a "secure" password.
*/
kadm_ser_ckpw(dat, len, ad, datout, outlen)
u_char *dat;
int len;
AUTH_DAT *ad;
u_char **datout;
int *outlen;
{
    unsigned KRB_INT32 keylow, keyhigh;
    char pword[MAX_KPW_LEN];
    int no_pword = 0;
    des_cblock newkey;
    int stvlen;
    int	retval;
    extern char *malloc();
    extern int kadm_approve_pw();

    /* take key off the stream, and check it */

    if ((stvlen = stv_long(dat, &keyhigh, 0, len)) < 0)
	return(KADM_LENGTH_ERROR);
    if ((stvlen = stv_long(dat, &keylow, stvlen, len)) < 0)
	return(KADM_LENGTH_ERROR);
    if ((stvlen = stv_string(dat, pword, stvlen, sizeof(pword), len)) < 0) {
	no_pword++;
	pword[0]='\0';
    }

    keylow = ntohl(keylow);
    keyhigh = ntohl(keyhigh);
    memcpy((char *)(((KRB_INT32 *)newkey) + 1), (char *)&keyhigh, sizeof(KRB_INT32));
    memcpy((char *)newkey, (char *)&keylow, sizeof(KRB_INT32));
    if (retval = kadm_approve_pw(ad->pname, ad->pinst, ad->prealm, newkey,
			no_pword ? 0 : pword)) {
	    *outlen = strlen(check_pw_msg)+strlen(pw_blurb)+1;
	    if (*datout = (u_char *) malloc(*outlen)) {
		    strcpy((char *) *datout, check_pw_msg);
		    strcat((char *) *datout, pw_blurb);
	    } else
		    *outlen = 0;
	    (void) krb_log("'%s.%s@%s' sent an insecure password to be checked",
		       ad->pname, ad->pinst, ad->prealm);
	    return(retval);
    } else {
	    *datout = 0;
	    *outlen = 0;
	    (void) krb_log("'%s.%s@%s' sent a secure password to be checked",
		       ad->pname, ad->pinst, ad->prealm);
    }
    return(0);
}


/*
kadm_ser_stab - the server side of the change_srvtab routine
  recieves    : KTEXT, {values}
  returns     : CKSUM, RETCODE, {values}
  acl         : su, sms (as register or dealloc)

Creates or modifies the specified service principal to have a random
key, which is sent back to the client.  The key version is returned in
the max_life field of the values structure.  It's a hack, but it's a
backwards compatible hack....
*/
kadm_ser_stab(dat, len, ad, datout, outlen)
u_char *dat;
int len;
AUTH_DAT *ad;
u_char **datout;
int *outlen;
{
  Kadm_vals values;
  int status;

  if ((status = stream_to_vals(dat, &values, len)) < 0)
	  return KADM_LENGTH_ERROR;
  status = kadm_chg_srvtab(ad->pname, ad->pinst, ad->prealm, &values);
  if (status == KADM_DATA) {
      *outlen = vals_to_stream(&values,datout);
      values.key_low = values.key_high = 0;
      return KADM_SUCCESS;
  } else {
      *outlen = 0;
      return status;
  }
}
