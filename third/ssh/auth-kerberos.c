/*

   auth-kerberos.c

   Initally written by Dug Song <dugsong@umich.edu> for Kerberos V4.
   Mapped over to user Kerberos V5 by Glenn Machin - Sandia Natl Labs

   Kerberos authentication and ticket-passing routines.

*/
/*
 * $Id: auth-kerberos.c,v 1.5.2.2 1998-07-15 22:49:55 ghudson Exp $
 * $Log: not supported by cvs2svn $
 * Revision 1.7  1998/07/15 22:48:53  ghudson
 * Dan left out a right paren.
 *
 * Revision 1.5.2.1  1998/07/15 22:46:38  ghudson
 * Pull up change from rev 1.6 to Athena 8.2 branch.
 *
 * Revision 1.6  1998/07/14 17:34:49  danw
 * use krb5_build_principal_ext instead of krb5_build_principal in case
 * the realm data isn't NUL-terminated
 *
 * Revision 1.5  1998/01/24 01:47:20  danw
 * merge in changes for 1.2.22
 *
 * Revision 1.4  1997/11/19 20:44:42  danw
 * do chown later
 *
 * Revision 1.3  1997/11/15 00:04:12  danw
 * Use atexit() functions to destroy tickets and call al_acct_revert.
 * Work around Solaris lossage with libucb and grantpt.
 *
 * Revision 1.2  1997/11/12 21:16:08  danw
 * Athena-login changes (including some krb4 stuff)
 *
 * Revision 1.1.1.1  1997/10/17 22:26:14  danw
 * Import of ssh 1.2.21
 *
 * Revision 1.1.1.2  1998/01/24 01:25:35  danw
 * Import of ssh 1.2.22
 *
 * Revision 1.3  1998/01/02 06:13:56  kivinen
 * 	Fixed kerberos ticket allocation.
 *
 * Revision 1.2  1997/04/17 03:56:51  kivinen
 * 	Kept FILE: prefix in kerberos ticket filename as DCE cache
 * 	code requires it (patch from Doug Engert <DEEngert@anl.gov>).
 *
 * Revision 1.1  1997/03/27 03:09:29  kivinen
 * *** empty log message ***
 *
 *
 * $Endlog$
 */

#include "includes.h"
#include "packet.h"
#include "xmalloc.h"
#include "ssh.h"

#ifdef KERBEROS
#if defined (KRB5)
#include <krb5.h>
/* kludge to allow us to #include krb.h without namespace conflicts */
#define des_cbc_encrypt krb_des_cbc_encrypt
#include <krb.h>

extern  krb5_context ssh_context;
extern  krb5_auth_context auth_context;
extern  int havecred;
void	krb_cleanup(void);

int auth_kerberos(char *server_user, krb5_data *auth, krb5_principal *client)
{
  krb5_error_code problem;
  krb5_ticket *ticket;
  krb5_data reply;
  krb5_principal tkt_client;
  char *server = 0;
  
  memset(&reply, 0, sizeof(reply));
  
  if (auth_context)
    krb5_auth_con_free(ssh_context, auth_context);

  auth_context = 0;
  krb5_auth_con_init(ssh_context, &auth_context);
  problem =
    krb5_auth_con_genaddrs(ssh_context, auth_context,
			   packet_get_connection_in(),
			   KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR);
  if (problem)
    {
      if (auth_context)
	{
	  krb5_auth_con_free(ssh_context, auth_context);
	  auth_context = 0;
	}
      log_msg("Kerberos ticket authentication of user %s failed: %s",
	      server_user, error_message(problem));
      
      debug("Kerberos krb5_auth_con_genaddrs (%s).", error_message(problem));
      packet_send_debug("Kerberos krb5_auth_con_genaddrs: %s",
			error_message(problem));
      return 0;
    }
  problem = krb5_rd_req(ssh_context, &auth_context, auth,
			NULL, NULL, NULL, &ticket);
  if (problem)
    {
      if (auth_context)
	{
	  krb5_auth_con_free(ssh_context, auth_context);
	  auth_context = 0;  
	}
      log_msg("Kerberos ticket authentication of user %s failed: %s",
	      server_user, error_message(problem));
      
      debug("Kerberos V5 rd_req failed (%s).", error_message(problem));
      packet_send_debug("Kerberos V5 krb5_rd_req: %s", error_message(problem));
      return 0;
    }
  
  /* Verify from ticket that the server used was of the form host/system */
  problem = krb5_unparse_name(ssh_context, ticket->server, &server);
  if (problem)
    {
      krb5_free_ticket(ssh_context, ticket);
      log_msg("Kerberos ticket authentication of user %s failed: %s",
	      server_user, error_message(problem));
      
      debug("Kerberos krb5_unparse_name failed (%s).", error_message(problem));
      packet_send_debug("Kerberos krb5_unparse_name: %s",
			error_message(problem));
      return 0;
    }
  if (strncmp(server, "host/", strlen("host/")))
    {
      krb5_free_ticket(ssh_context, ticket);
      log_msg("Kerberos ticket authentication of user %s failed: invalid service name (%s)",
	      server_user, server);
      
      debug("Kerberos invalid service name (%s).", server);
      packet_send_debug("Kerberos invalid service name (%s).", server);
      krb5_xfree(server);
      return 0;
    }
  krb5_xfree(server);
  
  /* Extract the users name from the ticket client principal */
  problem = krb5_copy_principal(ssh_context, ticket->enc_part2->client,
				&tkt_client);
  
  krb5_free_ticket(ssh_context, ticket);
  
  if (problem)
    {
      log_msg("Kerberos ticket authentication of user %s failed: %s",
	      server_user, error_message(problem));
      debug("Kerberos krb5_copy_principal failed (%s).", 
	    error_message(problem));
      packet_send_debug("Kerberos krb5_copy_principal: %s", 
			error_message(problem));
      return 0;
    }
  *client = tkt_client;
  
  /* Make the reply - so that mutual authentication can be done */
  if ((problem = krb5_mk_rep(ssh_context, auth_context, &reply)))
    {
      log_msg("Kerberos ticket authentication of user %s failed: %s",
	      server_user, error_message(problem));
      debug("Kerberos krb5_mk_rep failed (%s).",
	    error_message(problem));
      packet_send_debug("Kerberos krb5_mk_rep failed: %s",
			error_message(problem));
      return 0;
    }
  
  packet_start(SSH_SMSG_AUTH_KERBEROS_RESPONSE);
  packet_put_string((char *) reply.data, reply.length);
  packet_send();
  packet_write_wait();
  krb5_xfree(reply.data);
  return 1;
}
#endif /* KRB5 */
#endif /* KERBEROS */

#ifdef KERBEROS_TGT_PASSING
#if defined (KRB5)
int auth_kerberos_tgt( char *server_user, krb5_data *krb5data)
{
  krb5_creds **creds;
  krb5_error_code retval;
  static char ccname[128], tktname[128];
  krb5_ccache ccache = NULL;
  struct passwd *pwd;
  extern char *ticket;
  static krb5_principal rcache_server = 0;
  static krb5_rcache rcache;
  struct sockaddr_in local, foreign;
  krb5_address *local_addr, *remote_addr;
  int s;
  krb5_data *realm;
  krb5_creds increds, *v5creds;
  CREDENTIALS v4creds;
  
  if (!(pwd = (struct passwd *) getpwnam(server_user)))
    {
      log_msg("Kerberos V5 tgt rejected for user %.100s", server_user);
      packet_send_debug("Kerberos V5 tgt rejected for %.100s", server_user);
      packet_start(SSH_SMSG_FAILURE);
      packet_send();
      packet_write_wait();
      return 0;
    }
  
  if (!auth_context)
    {
      if (!rcache_server)
	krb5_parse_name(ssh_context,"sshd", &rcache_server);
      krb5_auth_con_init(ssh_context, &auth_context);
      
      /* Set the addresses for local and remote systems, and replay
         cache */
      
      /* GDM : We need to establish the local addresses and remote addresses
         within auth_context, in order to to TGT forwarding. Normally
         when the authentication credentials are passed, these are 
         established then, however in SSH the forwarded TGT is 
         passed prior to authentication. (Needed by AFS) */
      
      s = packet_get_connection_in();
      krb5_auth_con_genaddrs(ssh_context, auth_context, s, 
			     KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR |
			     KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR);
      krb5_get_server_rcache(ssh_context,
			     krb5_princ_component(context, rcache_server, 0),
			     &rcache);
      krb5_auth_con_setrcache( ssh_context, auth_context, rcache);
      
    }
  
  if (retval = krb5_rd_cred(ssh_context, auth_context, krb5data, &creds, NULL))
    {
      log_msg("Kerberos V5 tgt rejected for user %.100s : %s", server_user,
	      error_message(retval));
      packet_send_debug("Kerberos V5 tgt rejected for %.100s : %s",
			server_user,
			error_message(retval));
      packet_start(SSH_SMSG_FAILURE);
      packet_send();
      packet_write_wait();
      return 0;
    }
  
  sprintf(ccname, "FILE:/tmp/krb5cc_p%d", getpid());
  
  if (retval = krb5_cc_resolve(ssh_context, ccname, &ccache))
    goto errout;
  
  if (retval = krb5_cc_initialize(ssh_context, ccache, (*creds)->client))
    goto errout;
  
  if (retval = krb5_cc_store_cred(ssh_context, ccache, *creds))
    goto errout;
  
  ticket = xmalloc(strlen(ccname + 5) + 1);
  (void) sprintf(ticket, "%s", ccname);
  
  /* Now try to get krb4 tickets */
  krb524_init_ets(ssh_context);
  realm = krb5_princ_realm(ssh_context, (*creds)->client);
  memset(&increds, 0, sizeof(increds));
  if (retval = krb5_build_principal_ext(ssh_context, &(increds.server),
					realm->length, realm->data, 6,
					"krbtgt", realm->length, realm->data,
					NULL));
    goto errout2;

  increds.client = (*creds)->client;
  increds.times.endtime = 0;
  increds.keyblock.enctype = ENCTYPE_DES_CBC_CRC;
  if (retval = krb5_get_credentials(ssh_context, 0, ccache, &increds,
				    &v5creds))
    goto errout2;
  
  if (retval = krb524_convert_creds_kdc(ssh_context, v5creds, &v4creds))
    goto errout2;

  sprintf(tktname, "KRBTKFILE=/tmp/tkt_p%d", getpid());
  putenv(xstrdup(tktname));
  if (retval = in_tkt(v4creds.pname, v4creds.pinst))
    goto errout2;

  if (retval = krb_save_credentials(v4creds.service, v4creds.instance,
				    v4creds.realm, v4creds.session,
				    v4creds.lifetime, v4creds.kvno,
				    &(v4creds.ticket_st), v4creds.issue_date))
    goto errout2;

  /* Successful */
  packet_start(SSH_SMSG_SUCCESS);
  packet_send();
  packet_write_wait();
  havecred = 1;
  atexit(krb_cleanup);
  return 1;
  
errout3:
  dest_tkt();
errout2:
  krb5_cc_destroy(ssh_context, ccache);
errout:
  krb5_free_tgt_creds(ssh_context, creds);
  log_msg("Kerberos V5 tgt rejected for user %.100s :%s", server_user,
	  error_message(retval));
  packet_send_debug("Kerberos V5 tgt rejected for %.100s : %s", server_user,
		    error_message(retval));
  packet_start(SSH_SMSG_FAILURE);
  packet_send();
  packet_write_wait();
  return 0;
  
}
#endif /* KRB5 */
#endif /* KERBEROS_TGT_PASSING */

