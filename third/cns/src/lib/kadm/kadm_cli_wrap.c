/*
 * kadm_cli_wrap.c
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Kerberos administration server client-side routines
 * The client side wrapping of the calls to the admin server.
 */

#include <mit-copyright.h>

#include <string.h>

#define	DEFINE_SOCKADDR		/* Ask krb.h for struct sockaddr, netdb, etc */
#include "krb.h"

#include <kadm.h>
#include <kadm_err.h> 
#include <krb_err.h>
#include <krbports.h>

#ifndef NULL
#define NULL 0
#endif

Kadm_Client client_parm;
int default_client_port = 1;

/* Macros for use in returning data... used in kadm_cli_send */
#define RET_N_FREE(r) {clear_secrets(); free((char *)act_st); free((char *)priv_pak); return r;}

/* Keys for use in the transactions */
static des_cblock sess_key;	       /* to be filled in by kadm_cli_keyd */
static Key_schedule sess_sched;

static void
clear_secrets()
{
	memset((char *)sess_key, 0, sizeof(sess_key));
	memset((char *)sess_sched, 0, sizeof(sess_sched));
	return;
}

/*
 * kadm_init_link
 *	receives    : name, inst, realm
 *
 * initializes client parm, the Kadm_Client structure which holds the 
 * data about the connection between the server and client, the services 
 * used, the locations and other fun things 
 */
int INTERFACE
kadm_init_link(n, i, r)
char n[];
char i[];
char r[];
{
	struct servent *sep;	       /* service we will talk to */
	u_short sep_port;
	struct hostent *hop;	       /* host we will talk to */
	char adm_hostname[MAXHOSTNAMELEN];
	char *scol = 0;

#ifdef GLOBAL_ERROR_HANDLING
	(void) init_kadm_err_tbl();
	(void) init_krb_err_tbl();
#endif

	(void) strcpy(client_parm.sname, n);
	(void) strcpy(client_parm.sinst, i);
	(void) strcpy(client_parm.krbrlm, r);
	client_parm.admin_fd = -1;

	/* set up the admin_addr - fetch name of admin host */
	if (krb_get_admhst(adm_hostname, client_parm.krbrlm, 1) != KSUCCESS)
		return KADM_NO_HOST;
	scol = strchr(adm_hostname,':');
	if (scol) *scol = 0;
	if ((hop = gethostbyname(adm_hostname)) == NULL)
		return KADM_UNK_HOST;  /* couldnt find the admin servers
				        * address */
	if (scol) {
		sep_port = htons(atoi(scol+1)+1);
		default_client_port = 0;
	}
	else if (sep = getservbyname(KADM_SNAME, "tcp"))
		sep_port = sep->s_port;
	else
		sep_port = htons(KADM_PORT); /* KADM_SNAME = kerberos_master/tcp */
	memset((char *) &client_parm.admin_addr, 0,
	      sizeof(client_parm.admin_addr));
	client_parm.admin_addr.sin_family = hop->h_addrtype;
	memcpy((char *) &client_parm.admin_addr.sin_addr, (char *) hop->h_addr,
	       hop->h_length);
	client_parm.admin_addr.sin_port = sep_port;

	return KADM_SUCCESS;
}				       /* procedure kadm_init_link */

/*
 * kadm_change_pw
 * receives    : key
 *
 * Replaces the password (i.e. des key) of the caller with that specified in
 * key. Returns no actual data from the master server, since this is called
 * by a user 
 */
int INTERFACE
kadm_change_pw(newkey)
des_cblock newkey;		       /* The DES form of the users key */
{
	u_char	*ret_st;
	int	retval;

	retval = kadm_change_pw2(newkey, (char *)0, &ret_st);
	if (ret_st)
		free(ret_st);
	return(retval);
}

/*
 * kadm_change_pw2
 * recieves    : key, pw_string, ret_string
 *
 * Replaces the password (i.e. des key) of the caller with that specified in
 * key. Returns no actual data from the master server, since this is called
 * by a user 
 */
int INTERFACE
kadm_change_pw2(newkey, pwstring, retstring)
des_cblock newkey;		       /* The DES form of the users key */
char	*pwstring;
u_char	**retstring;
{
	int stsize, retc;	       /* stream size and return code */
	u_char *send_st;	       /* send stream */
	u_char *ret_st;
	int ret_sz;
	unsigned KRB_INT32 keytmp;
	extern char *malloc();

	*retstring = (u_char *) 0;

	if ((retc = kadm_cli_conn()) != KADM_SUCCESS)
	    return(retc);
	/* possible problem with vts_long on a non-multiple of four boundary */

	stsize = 0;		       /* start of our output packet */
	send_st = (u_char *) malloc(1);/* to make it reallocable */
	send_st[stsize++] = (u_char) CHANGE_PW;

	/* change key to stream */

	memcpy((char *) &keytmp, (char *) (((KRB_INT32 *) newkey) + 1), sizeof(KRB_INT32));
	keytmp = htonl(keytmp);
	stsize += vts_long(keytmp, &send_st, stsize);

	memcpy((char *) &keytmp, (char *) newkey, sizeof(KRB_INT32));
	keytmp = htonl(keytmp);
	stsize += vts_long(keytmp, &send_st, stsize);

	if (pwstring) {
		stsize += vts_string(pwstring, &send_st, stsize);
	}
	
	retc = kadm_cli_send(send_st, stsize, &ret_st, &ret_sz);
	free((char *)send_st);
	if (ret_sz)
		*retstring = ret_st;
	else {
		*retstring = 0;
		free(ret_st);
	}
	kadm_cli_disconn();
	return(retc);
}

/*
 * kadm_add
 * 	receives    : vals
 * 	returns     : vals 
 *
 * Adds an entry containing values to the database returns the values of the
 * entry, so if you leave certain fields blank you will be able to determine
 * the default values they are set to 
 */
kadm_add(vals)
Kadm_vals *vals;
{
	u_char *st, *st2;	       /* st will hold the stream of values */
	int st_len;		       /* st2 the final stream with opcode */
	int retc;		       /* return code from call */
	u_char *ret_st;
	int ret_sz;
	extern char *malloc();

	if ((retc = kadm_cli_conn()) != KADM_SUCCESS)
	    return(retc);
	st_len = vals_to_stream(vals, &st);
	st2 = (u_char *) malloc((unsigned)(1 + st_len));
	*st2 = (u_char) ADD_ENT;       /* here's the opcode */
	memcpy((char *) st2 + 1, (char *) st, st_len);	/* append st on */
	retc = kadm_cli_send(st2, st_len + 1, &ret_st, &ret_sz);
	free((char *)st);
	free((char *)st2);
	if (retc == KADM_SUCCESS) {
	    /* ret_st has vals */
	    if (stream_to_vals(ret_st, vals, ret_sz) < 0)
		retc = KADM_LENGTH_ERROR;
	    free((char *)ret_st);
	}
	kadm_cli_disconn();
	return(retc);
}

/*
 * kadm_delete
 * 	receives    : vals
 * 	returns     : vals 
 *
 * Deletes an entry containing values from the database.
 */
kadm_delete(vals)
Kadm_vals *vals;
{
	u_char *st, *st2;	       /* st will hold the stream of values */
	int st_len;		       /* st2 the final stream with opcode */
	int retc;		       /* return code from call */
	u_char *ret_st;
	int ret_sz;
	extern char *malloc();

	if ((retc = kadm_cli_conn()) != KADM_SUCCESS)
	    return(retc);
	st_len = vals_to_stream(vals, &st);
	st2 = (u_char *) malloc((unsigned)(1 + st_len));
	*st2 = (u_char) DEL_ENT;       /* here's the opcode */
	memcpy((char *) st2 + 1, (char *) st, st_len);	/* append st on */
	retc = kadm_cli_send(st2, st_len + 1, &ret_st, &ret_sz);
	free((char *)st);
	free((char *)st2);
	if (retc == KADM_SUCCESS) {
	    /* ret_st has vals */
	    if (stream_to_vals(ret_st, vals, ret_sz) < 0)
		retc = KADM_LENGTH_ERROR;
	    free((char *)ret_st);
	}
	kadm_cli_disconn();
	return(retc);
}
/*
 * kadm_mod
 * 	receives    : KTEXT, {values, values}
 *	returns     : CKSUM,  RETCODE, {values} 
 *	acl         : su, sms (as register or dealloc) 
 *
 * Modifies all entries corresponding to the first values so they match the
 * second values. returns the values for the changed entries in vals2
 */
int kadm_mod(vals1, vals2)
Kadm_vals *vals1;
Kadm_vals *vals2;
{
	u_char *st, *st2;	       /* st will hold the stream of values */
	int st_len, nlen;	       /* st2 the final stream with opcode */
	u_char *ret_st;
	int ret_sz;
	extern char *malloc();
	extern char *realloc();

	/* nlen is the length of second vals */
	int retc;		       /* return code from call */

	if ((retc = kadm_cli_conn()) != KADM_SUCCESS)
	    return(retc);

	st_len = vals_to_stream(vals1, &st);
	st2 = (u_char *) malloc((unsigned)(1 + st_len));
	*st2 = (u_char) MOD_ENT;       /* here's the opcode */
	memcpy((char *) st2 + 1, (char *) st, st_len++); /* append st on */
	free((char *)st);
	nlen = vals_to_stream(vals2, &st);
	st2 = (u_char *) realloc((char *) st2, (unsigned)(st_len + nlen));
	memcpy((char *) st2 + st_len, (char *) st, nlen); /* append st on */
	retc = kadm_cli_send(st2, st_len + nlen, &ret_st, &ret_sz);
	free((char *)st);
	free((char *)st2);
	if (retc == KADM_SUCCESS) {
	    /* ret_st has vals */
	    if (stream_to_vals(ret_st, vals2, ret_sz) < 0)
		retc = KADM_LENGTH_ERROR;
	    free((char *)ret_st);
	}
	kadm_cli_disconn();
	return(retc);
}

/*
 * kadm_get
 * 	receives   : KTEXT, {values, flags} 
 *	returns    : CKSUM, RETCODE, {count, values, values, values}
 *	acl        : su 
 *
 * gets the fields requested by flags from all entries matching values returns
 * this data for each matching recipient, after a count of how many such
 * matches there were 
 */
int kadm_get(vals, fl)
Kadm_vals *vals;
u_char fl[4];
{
	int loop;		       /* for copying the fields data */
	u_char *st, *st2;	       /* st will hold the stream of values */
	int st_len;		       /* st2 the final stream with opcode */
	int retc;		       /* return code from call */
	u_char *ret_st;
	int ret_sz;
	extern char *malloc();

	if ((retc = kadm_cli_conn()) != KADM_SUCCESS)
	    return(retc);
	st_len = vals_to_stream(vals, &st);
	st2 = (u_char *) malloc((unsigned)(1 + st_len + FLDSZ));
	*st2 = (u_char) GET_ENT;       /* here's the opcode */
	memcpy((char *) st2 + 1, (char *) st, st_len);	/* append st on */
	for (loop = FLDSZ - 1; loop >= 0; loop--)
		*(st2 + st_len + FLDSZ - loop) = fl[loop]; /* append the flags */
	retc = kadm_cli_send(st2, st_len + 1 + FLDSZ,  &ret_st, &ret_sz);
	free((char *)st);
	free((char *)st2);
	if (retc == KADM_SUCCESS) {
	    /* ret_st has vals */
	    if (stream_to_vals(ret_st, vals, ret_sz) < 0)
		retc = KADM_LENGTH_ERROR;
	    free((char *)ret_st);
	}
	kadm_cli_disconn();
	return(retc);
}

/*
 * kadm_check_pw
 * recieves    : key, pw_string, ret_string
 *
 * Sends the password to the server and asks it if the password is a
 * "secure" password or not.  Returns the error KADM_INSECURE_PW and
 * optionally returns a message if the password is not considered secure. 
 */
int kadm_check_pw(newkey, pwstring, retstring)
des_cblock newkey;		       /* The DES form of the users key */
char	*pwstring;
u_char	**retstring;
{
	int stsize, retc;	       /* stream size and return code */
	u_char *send_st;	       /* send stream */
	u_char *ret_st;
	int ret_sz;
	unsigned KRB_INT32 keytmp;
	extern char *malloc();

	if ((retc = kadm_cli_conn()) != KADM_SUCCESS)
	    return(retc);
	/* possible problem with vts_long on a non-multiple of four boundary */

	stsize = 0;		       /* start of our output packet */
	send_st = (u_char *) malloc(1);/* to make it reallocable */
	send_st[stsize++] = (u_char) CHECK_PW;

	/* change key to stream */

	memcpy((char *) &keytmp, (char *) (((KRB_INT32 *) newkey) + 1), sizeof(KRB_INT32));
	keytmp = htonl(keytmp);
	stsize += vts_long(keytmp, &send_st, stsize);

	memcpy((char *) &keytmp, (char *) newkey, sizeof(KRB_INT32));
	keytmp = htonl(keytmp);
	stsize += vts_long(keytmp, &send_st, stsize);
	
	if (pwstring) {
		stsize += vts_string(pwstring, &send_st, stsize);
	}
	
	retc = kadm_cli_send(send_st, stsize, &ret_st, &ret_sz);
	free((char *)send_st);
	if (ret_sz)
		*retstring = ret_st;
	else {
		*retstring = 0;
		free(ret_st);
	}
	kadm_cli_disconn();
	return(retc);
}

/*
 * kadm_change_srvtab
 * receives    : name, instance
 * returns     : values
 *
 * Asks the admin server to change the srvtab key for the specified
 * name, instance pair.  The name, instance pair must be a valid
 * server principal.
 */
int kadm_change_srvtab(name, instance, values)
	char	*name, *instance;
	Kadm_vals      	*values;
{
	u_char *st, *st2;	       /* st will hold the stream of values */
	int st_len;		       /* st2 the final stream with opcode */
	int retc;		       /* return code from call */
	u_char *ret_st;
	int ret_sz;
	Kadm_vals	vals;
	extern char *malloc();

	memset((char *) &vals, 0, sizeof(vals));
	strncpy(vals.name, name, sizeof(vals.name));
	strncpy(vals.instance, instance, sizeof(vals.instance));
	SET_FIELD(KADM_NAME,vals.fields);
	SET_FIELD(KADM_INST,vals.fields);

	if ((retc = kadm_cli_conn()) != KADM_SUCCESS)
	    return(retc);
	st_len = vals_to_stream(&vals, &st);
	st2 = (u_char *) malloc((unsigned)(1 + st_len));
	*st2 = (u_char) CHG_STAB;       /* here's the opcode */
	memcpy((char *) st2 + 1, (char *) st, st_len);	/* append st on */
	retc = kadm_cli_send(st2, st_len + 1, &ret_st, &ret_sz);
	free((char *)st);
	free((char *)st2);
	if (retc == KADM_SUCCESS) {
	    /* ret_st has vals */
	    if (stream_to_vals(ret_st, values, ret_sz) < 0)
		retc = KADM_LENGTH_ERROR;
	    free((char *)ret_st);
	}
	kadm_cli_disconn();
	return(retc);
}

/*
 * kadm_cli_send
 *	recieves   : opcode, packet, packet length, serv_name, serv_inst
 *	returns    : return code from the packet build, the server, or
 *			 something else 
 *
 * It assembles a packet as follows:
 *	 8 bytes    : VERSION STRING
 *	 4 bytes    : LENGTH OF MESSAGE DATA and OPCODE
 *		    : KTEXT
 *		    : OPCODE       \
 *		    : DATA          > Encrypted (with make priv)
 *		    : ......       / 
 *
 * If it builds the packet and it is small enough, then it attempts to open the
 * connection to the admin server.  If the connection is succesfully open
 * then it sends the data and waits for a reply. 
 */
int kadm_cli_send(st_dat, st_siz, ret_dat, ret_siz)
u_char *st_dat;				/* the actual data */
int st_siz;				/* length of said data */
u_char **ret_dat;			/* to give return info */
int *ret_siz;				/* length of returned info */
{
	int act_len;		       /* current offset into packet, return */
	KRB_INT32 retdat;	       /* data */
	KTEXT_ST authent;	       /* the authenticator we will build */
	u_char *act_st;		       /* the pointer to the complete packet */
	u_char *priv_pak;	       /* private version of the packet */
	int priv_len;		       /* length of private packet */
	u_long cksum;		       /* checksum of the packet */
	MSG_DAT mdat;
	u_char *return_dat;
	extern char *malloc();
	extern char *realloc();

	act_st = (u_char *) malloc(KADM_VERSIZE); /* verstr stored first */
	(void) strncpy((char *)act_st, KADM_VERSTR, KADM_VERSIZE);
	act_len = KADM_VERSIZE;

	if ((retdat = kadm_cli_keyd(sess_key, sess_sched)) != KADM_SUCCESS) {
		free((char *)act_st);
		return retdat;	       /* couldnt get key working */
	}
	priv_pak = (u_char *) malloc((unsigned)(st_siz + 200));
	/* 200 bytes for extra info case */
	if ((priv_len = krb_mk_priv(st_dat, priv_pak, (u_long)st_siz,
				    sess_sched, &sess_key, &client_parm.my_addr,
				    &client_parm.admin_addr)) < 0)
		RET_N_FREE(KADM_NO_ENCRYPT);	/* whoops... we got a lose
						 * here */
	/* here is the length of priv data.  receiver calcs
	 size of authenticator by subtracting vno size, priv size, and
	 sizeof(u_long) (for the size indication) from total size */

	act_len += vts_long((unsigned KRB_INT32) priv_len, &act_st, act_len);
#ifdef NOENCRYPTION
	cksum = 0;
#else
	cksum = quad_cksum(priv_pak, (unsigned KRB_INT32 *)0, (long)priv_len,
			   0, &sess_key);
#endif
	if (retdat = krb_mk_req(&authent, client_parm.sname, client_parm.sinst,
				client_parm.krbrlm, (long)cksum)) {
	    /* authenticator? */
	    RET_N_FREE(retdat + krb_err_base);
	}

	act_st = (u_char *) realloc((char *) act_st,
				    (unsigned) (act_len + authent.length
						+ priv_len));
	if (!act_st) {
	    clear_secrets();
	    free((char *)priv_pak);
	    return(KADM_NOMEM);
	}
	memcpy((char *)act_st + act_len, (char *)authent.dat, authent.length);
	memcpy((char *)act_st + act_len + authent.length, (char *)priv_pak, 
	       priv_len);
	free((char *)priv_pak);
	if ((retdat = kadm_cli_out(act_st,
				   act_len + authent.length + priv_len,
				   ret_dat, ret_siz)) != KADM_SUCCESS)
	    RET_N_FREE(retdat);
	free((char *)act_st);
#define RET_N_FREE2(r) {free((char *)*ret_dat); *ret_dat = 0; *ret_siz = 0; clear_secrets(); return(r);}

	/* first see if it's a YOULOUSE */
	if ((*ret_siz >= KADM_VERSIZE) &&
	    !strncmp(KADM_ULOSE, (char *)*ret_dat, KADM_VERSIZE))
	  {
	    unsigned KRB_INT32 errcode;
	    /* it's a youlose packet */
	    if (*ret_siz < KADM_VERSIZE + sizeof(unsigned KRB_INT32))
		RET_N_FREE2(KADM_BAD_VER);
	    memcpy((char *)&errcode, (char *)(*ret_dat) + KADM_VERSIZE, 
		   sizeof(unsigned KRB_INT32));
	    retdat = (int) ntohl(errcode);
	    RET_N_FREE2(retdat);
	  }
	/* need to decode the ret_dat */
	if (retdat = krb_rd_priv(*ret_dat, (u_long)*ret_siz, sess_sched,
				 &sess_key, &client_parm.admin_addr,
				 &client_parm.my_addr, &mdat))
	    RET_N_FREE2(retdat+krb_err_base);
	if (mdat.app_length < KADM_VERSIZE + 4)
	    /* too short! */
	    RET_N_FREE2(KADM_BAD_VER);
	if (strncmp((char *)mdat.app_data, KADM_VERSTR, KADM_VERSIZE))
	    /* bad version */
	    RET_N_FREE2(KADM_BAD_VER);
	memcpy((char *)&retdat, (char *)mdat.app_data+KADM_VERSIZE, 
	       sizeof(unsigned KRB_INT32));
	retdat = ntohl((u_long)retdat);
	if ((mdat.app_length - KADM_VERSIZE - sizeof(unsigned KRB_INT32)) != 0) {
	  if (!(return_dat = (u_char *)
		malloc((unsigned)(mdat.app_length - KADM_VERSIZE -
				  sizeof(unsigned KRB_INT32)))))
	    RET_N_FREE2(KADM_NOMEM);
	  memcpy((char *)return_dat,
		 (char *) mdat.app_data + KADM_VERSIZE + sizeof(unsigned KRB_INT32),
		 (int)mdat.app_length - KADM_VERSIZE - sizeof(unsigned KRB_INT32));
	} else {
	  /* If it's zero length, still need to malloc a 1 byte string; */
	  /* malloc's of zero will return NULL on AIX & A/UX */
	  if (!(return_dat = (u_char *) malloc((unsigned) 1)))
	    RET_N_FREE2(KADM_NOMEM);
	  *return_dat = '\0';
	}
	free((char *)*ret_dat);
	clear_secrets();
	*ret_dat = return_dat;
	*ret_siz = mdat.app_length - KADM_VERSIZE - sizeof(unsigned KRB_INT32);
	return retdat;
}

/* takes in the sess_key and key_schedule and sets them appropriately */
int kadm_cli_keyd(s_k, s_s)
des_cblock s_k;			       /* session key */
des_key_schedule s_s;		       /* session key schedule */
{
	CREDENTIALS cred;	       /* to get key data */
	int stat;

	/* want .sname and .sinst here.... */
	if (stat = krb_get_cred(client_parm.sname, client_parm.sinst,
				client_parm.krbrlm, &cred))
		return stat + krb_err_base;
	memcpy((char *) s_k, (char *) cred.session, sizeof(des_cblock));
	memset((char *) cred.session, 0, sizeof(des_cblock));
#ifdef NOENCRYPTION
	memset(s_s, 0, sizeof(des_key_schedule));
#else
	if (stat = key_sched(s_k, s_s))
		return stat + krb_err_base;
#endif
	return KADM_SUCCESS;
}				       /* This code "works" */

/* Networking-specific routines have been moved out to kadm_net.c.  */
