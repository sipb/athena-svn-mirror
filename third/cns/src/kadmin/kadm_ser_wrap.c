/*
 * kadm_ser_wrap.c
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Kerberos administration server-side support functions
 * unwraps wrapped packets and calls the appropriate server subroutine
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#define	DEFINE_SOCKADDR
#include <krb.h>		/* For krb_db.h */
#include <krb_db.h>		/* For kdb_get_master_key, etc */
#include <kadm.h>
#include <kadm_err.h>
#include <krb_err.h>
#include <krbports.h>
#include "kadm_server.h"

Kadm_Server server_parm;

/* 
kadm_ser_init
set up the server_parm structure
*/
int
kadm_ser_init(inter, realm, kfile)
int inter;			/* interactive or from file */
char realm[];
char *kfile;
{
  struct servent *sep;
  u_short sep_port;
  struct hostent *hp;
  char hostname[MAXHOSTNAMELEN];

  (void) init_kadm_err_tbl();
  (void) init_krb_err_tbl();
  if (gethostname(hostname, sizeof(hostname)))
      return KADM_NO_HOSTNAME;

  (void) strcpy(server_parm.sname, PWSERV_NAME);
  (void) strcpy(server_parm.sinst, KRB_MASTER);
  (void) strcpy(server_parm.krbrlm, realm);

  server_parm.admin_fd = -1;
				/* setting up the addrs */
  if (sep = getservbyname(KADM_SNAME, "tcp")) 
    sep_port = sep->s_port;
  else
    sep_port = htons(KADM_PORT);	/* KADM_SNAME == kerberos_master/tcp */
  memset((char *)&server_parm.admin_addr, 0,sizeof(server_parm.admin_addr));
  server_parm.admin_addr.sin_family = AF_INET;
  if ((hp = gethostbyname(hostname)) == NULL)
      return KADM_NO_HOSTNAME;
  memcpy((char *) &server_parm.admin_addr.sin_addr.s_addr, hp->h_addr,
	 hp->h_length);
  server_parm.admin_addr.sin_port = sep_port;
				/* setting up the database */
  if (kdb_get_master_key_from((inter==1),server_parm.master_key,
			      server_parm.master_key_schedule, 0, kfile) != 0)
    return KADM_NO_MAST;
  if ((server_parm.master_key_version =
       kdb_verify_master_key(server_parm.master_key,
			     server_parm.master_key_schedule,stderr))<0)
      return KADM_NO_VERI;
  return KADM_SUCCESS;
}

static void
errpkt(dat, dat_len, code)
u_char **dat;
int *dat_len;
int code;
{
    unsigned KRB_INT32 retcode;
    char *pdat;
    extern char *malloc();

    free((char *)*dat);			/* free up req */
    *dat_len = KADM_VERSIZE + sizeof(retcode);
    *dat = (u_char *) malloc((unsigned)*dat_len);
    pdat = (char *) *dat;
    retcode = htonl((u_long) code);
    (void) strncpy(pdat, KADM_ULOSE, KADM_VERSIZE);
    memcpy(&pdat[KADM_VERSIZE], (char *)&retcode, sizeof(retcode));
    return;
}

/*
kadm_ser_in
unwrap the data stored in dat, process, and return it.
*/
int
kadm_ser_in(dat,dat_len)
u_char **dat;
int *dat_len;
{
    u_char *in_st;			/* pointer into the sent packet */
    int in_len,retc;			/* where in packet we are, for
					   returns */
    unsigned KRB_INT32 r_len;		/* length of the actual packet */
    KTEXT_ST authent;			/* the authenticator */
    AUTH_DAT ad;			/* who is this, klink */
    u_long ncksum;			/* checksum of encrypted data */
    des_key_schedule sess_sched;	/* our schedule */
    MSG_DAT msg_st;
    u_char *retdat, *tmpdat;
    KRB_INT32 retval;
    int retlen;
    extern char *malloc();

    if (strncmp(KADM_VERSTR, (char *)*dat, KADM_VERSIZE)) {
	errpkt(dat, dat_len, KADM_BAD_VER);
	return KADM_BAD_VER;
    }
    in_len = KADM_VERSIZE;
    /* get the length */
    if ((retc = stv_long(*dat, &r_len, in_len, *dat_len)) < 0)
	return KADM_LENGTH_ERROR;
    in_len += retc;
    authent.length = *dat_len - r_len - KADM_VERSIZE - sizeof(KRB_INT32);
    memcpy((char *)authent.dat, (char *)(*dat) + in_len, authent.length);
    authent.mbz = 0;
    /* service key should be set before here */
    if (retc = krb_rd_req(&authent, server_parm.sname, server_parm.sinst,
			  server_parm.recv_addr.sin_addr.s_addr, &ad, (char *)0))
    {
	errpkt(dat, dat_len,retc + krb_err_base);
	return retc + krb_err_base;
    }

#define clr_cli_secrets() {memset((char *)sess_sched, 0, sizeof(sess_sched)); memset((char *)ad.session, 0, sizeof(ad.session));}

    /* These two lines used to be
         in_st = *dat + *dat_len - r_len;
       but that was miscompiled by the HP/UX 9.01 C compiler.  */
    in_st = *dat + *dat_len;
    in_st -= r_len;
#ifdef NOENCRYPTION
    ncksum = 0;
#else
    ncksum = quad_cksum(in_st, (unsigned KRB_INT32 *)0, (long) r_len, 0, &ad.session);
#endif
    if (ncksum!=ad.checksum) {		/* yow, are we correct yet */
	clr_cli_secrets();
	errpkt(dat, dat_len,KADM_BAD_CHK);
	return KADM_BAD_CHK;
    }
#ifdef NOENCRYPTION
    memset(sess_sched, 0, sizeof(sess_sched));
#else
    des_key_sched(ad.session, sess_sched);
#endif
    if (retc = (int) krb_rd_priv(in_st, r_len, sess_sched, &ad.session, 
				 &server_parm.recv_addr,
				 &server_parm.admin_addr, &msg_st)) {
	clr_cli_secrets();
	errpkt(dat, dat_len,retc + krb_err_base);
	return retc + krb_err_base;
    }
    switch (msg_st.app_data[0]) {
    case CHANGE_PW:
	retval = kadm_ser_cpw(msg_st.app_data+1,(int) msg_st.app_length-1,&ad,
			      &retdat, &retlen);
	break;
    case ADD_ENT:
	retval = kadm_ser_add(msg_st.app_data+1,(int) msg_st.app_length-1,&ad,
			      &retdat, &retlen);
	break;
    case DEL_ENT:
	retval = kadm_ser_del(msg_st.app_data+1,(int) msg_st.app_length-1,&ad,
			      &retdat, &retlen);
	break;
    case GET_ENT:
	retval = kadm_ser_get(msg_st.app_data+1,(int) msg_st.app_length-1,&ad,
			      &retdat, &retlen);
	break;
    case MOD_ENT:
	retval = kadm_ser_mod(msg_st.app_data+1,(int) msg_st.app_length-1,&ad,
			      &retdat, &retlen);
	break;
    case CHECK_PW:
	retval = kadm_ser_ckpw(msg_st.app_data+1,(int) msg_st.app_length-1,&ad,
			       &retdat, &retlen);
	break;
    default:
	clr_cli_secrets();
	errpkt(dat, dat_len, KADM_NO_OPCODE);
	return KADM_NO_OPCODE;
    }
    /* Now seal the response back into a priv msg */
    free((char *)*dat);
    tmpdat = (u_char *) malloc((unsigned)(retlen + KADM_VERSIZE +
					  sizeof(retval)));
    (void) strncpy((char *)tmpdat, KADM_VERSTR, KADM_VERSIZE);
    retval = htonl((u_long)retval);
    memcpy((char *)tmpdat + KADM_VERSIZE, (char *)&retval, sizeof(retval));
    if (retlen) {
	memcpy((char *)tmpdat + KADM_VERSIZE + sizeof(retval), (char *)retdat,
	       retlen);
	free((char *)retdat);
    }
    /* slop for mk_priv stuff */
    *dat = (u_char *) malloc((unsigned) (retlen + KADM_VERSIZE +
					 sizeof(retval) + 200));
    if ((*dat_len = krb_mk_priv(tmpdat, *dat,
				(u_long) (retlen + KADM_VERSIZE +
					  sizeof(retval)),
				sess_sched,
				&ad.session, &server_parm.admin_addr,
				&server_parm.recv_addr)) < 0) {
	clr_cli_secrets();
	errpkt(dat, dat_len, KADM_NO_ENCRYPT);
	return KADM_NO_ENCRYPT;
    }
    clr_cli_secrets();
    return KADM_SUCCESS;
}
