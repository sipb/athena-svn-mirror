/* Copyright 1994, 1999 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This is the server for the lert system. */

static const char rcsid[] = "$Id: lertsrv.c,v 1.10 2009/03/26 19:30:39 zacheiss Exp $";

#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_KRB4
#include <krb.h>
#include <des.h>
#endif
#include <krb5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <hesiod.h>
#include <string.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <errno.h>
#include "lert.h"

int get_user(char *pname, char *result, int onetime, char *ip);

krb5_context context = NULL;

int main(int argc, char **argv)
{
  int s;			/* The Socket */
  struct sockaddr_in sin;
  struct sockaddr_in from;
  char packet[2048];
  char *ip;
  char *username = NULL;
  int plen;
  char opacket[2048];
  int fromlen;
  int status;
  int len;
#ifdef HAVE_KRB4
  AUTH_DAT ad;
  KTEXT_ST authent;
  KTEXT_ST ktxt;
  char instance[INST_SZ];
  /* for krb_mk_priv */
  des_key_schedule sched;		/* session key schedule */
#endif
  struct hostent *hp;
  struct utsname thishost;
  /* for the krb5-authenticated v2 of the protocol */
  krb5_error_code problem;
  krb5_auth_context auth_con = NULL;
  krb5_data auth, inbuf, outbuf;
  krb5_principal server = NULL, client = NULL;
  krb5_ticket *ticket;
  krb5_keytab keytab = NULL;

  ticket = NULL;
  memset(&outbuf, 0, sizeof(krb5_data));

#ifdef LOGGING
  setbuf(stdout, NULL);
#endif

  s = socket(AF_INET, SOCK_DGRAM, 0);

  if (uname(&thishost) == -1)
    {
      fprintf(stderr, "%s: Unable to get system information: %s\n",
	      argv[0], strerror(errno));
      exit(1);
    }
  hp = gethostbyname(thishost.nodename);
  if (hp == NULL) {
    fprintf(stderr, "%s: Unable to get host name information: %s\n",
	    argv[0], strerror(errno));
    exit(1);
  }
  
  /* Clear the socket structure. */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(LERT_PORT);
  memcpy(&sin.sin_addr.s_addr, hp->h_addr, hp->h_length);
  
  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
      fprintf(stderr, "%s: Unable to bind socket: %s\n",
	      argv[0], strerror(errno));
      exit(1);
  }

  fromlen = sizeof(from);

  for (;;)
    {
      plen = recvfrom(s, packet, sizeof(packet), 0,
		      (struct sockaddr *)&from, &fromlen);
      if (plen < 0)
	{
	  fprintf(stderr, "lertsrv: Error during recv: %s\n",
		  strerror(errno));
	  sleep(1);		/* Prevent infinite cpu hog loop */
	  continue;
	}

#ifdef DEBUG
      fprintf(stderr, "lertsrv: Received packet of length %d\n", plen);
#endif

#ifdef HAVE_KRB4
      if (packet[0] == '1')
	{
	  /* krb4-authenticated version of the protocol. */

	  /* Copy authenticator into aligned region. */
	  memcpy(&authent, &packet[LERT_LENGTH], sizeof(authent));
	  
	  strcpy(instance, "*");
	  status = krb_rd_req(&authent, LERT_SERVICE, instance, 0, &ad,
			      LERTS_SRVTAB);
	  
	  if (status != KSUCCESS)
	    {
	      fprintf(stderr, "lertsrv: Kerberos failure: %s\n",
		      krb_err_txt[status]);
	      continue;
	    }
	  
	  opacket[0] = '1';
	  if ((strlen(ad.pinst) != 0) || (strcmp(ad.prealm, "ATHENA.MIT.EDU")))
	    {
	      fprintf(stderr, "lertsrv: %s.%s@%s -- Not null instance ATHENA "
		      "realm.\n", ad.pname, ad.pinst, ad.prealm);
	      opacket[1] = LERT_NOT_IN_DB;
	      opacket[2] = '\0';
	    }
	  else
	    get_user(ad.pname, &(opacket[1]), ad.checksum,
		     inet_ntoa(from.sin_addr));
	  
	  /* Prepare krb_mk_priv message */
	  des_key_sched(ad.session, sched);
	  
	  /* Make the encrypted message:
	   * version, code, data, plus terminating '\0'
	   */
	  len = krb_mk_priv(opacket, ktxt.dat,
			    LERT_CHECK + strlen(opacket + LERT_CHECK) + 1,
			    sched, ad.session, &sin, &from);
	  
	  /* Send it. */
	  if (sendto(s, ktxt.dat, len, 0,
		     (struct sockaddr *) &from, sizeof(from)) < 0)
	    fprintf(stderr, "lertsrv: sendto failed: %s\n", strerror(errno));
	  
	  /* Avoid confusion, zeroize now */
	  memset(&from, 0, sizeof(from));
	}
      else
#endif
      if (packet[0] == LERT_VERSION)
	{
	  /* krb5-authenticated version of the protocol. */
	  if (!context)
	    {
	      problem = krb5_init_context(&context);
	      if (problem)
		goto out;
	    }
	  problem = krb5_auth_con_init(context, &auth_con);
	  if (problem)
	    goto out;

	  problem =
	    krb5_auth_con_genaddrs(context, auth_con, s,
				   KRB5_AUTH_CONTEXT_GENERATE_LOCAL_ADDR);
	  if (problem)
	    goto out;

	  problem = krb5_sname_to_principal(context, NULL, LERT_SERVICE,
					    KRB5_NT_SRV_HST, &server);
	  if (problem)
	    goto out;

	  problem = krb5_kt_resolve(context, LERTS_KEYTAB, &keytab);
	  if (problem)
	    goto out;

	  /* Get the authenticator. */
	  auth.data = packet + LERT_LENGTH;
	  auth.length = sizeof(packet) - sizeof(int) - LERT_LENGTH;

	  problem = krb5_rd_req(context, &auth_con, &auth, server, keytab,
				NULL, &ticket);
	  if (problem)
	    goto out;

	  problem = krb5_copy_principal(context, ticket->enc_part2->client,
					&client);
	  if (problem)
	    goto out;

	  opacket[0] = LERT_VERSION;

	  /* If the client is something other than null instance principal
	   * or is from a realm other than the server's local realm, punt
	   * them.
	   */
	  if ((krb5_princ_size(context, client) != 1) ||
	      krb5_realm_compare(context, client, server) == 0)
	    {
	      krb5_unparse_name(context, client, &username);
	      if (username)
		fprintf(stderr, "lertsrv: %s not null instance in realm "
			"%s.\n", username, 
			krb5_princ_realm(context, server)->data);
	      opacket[1] = LERT_NOT_IN_DB;
	      opacket[2] = '\0';
	    }
	  else
	  /* packet[1] will be a 0 or 1 depending on whether or not the
	   * user called lert with the -n flag to indicate they don't
	   * want to see the message anymore.
	   *
	   * The krb4 code passes in the checksum member of the AUTH_DAT
	   * structure filled in by krb_rd_req().  While it will happen to
	   * be 0 if the user didn't call lert with the -n flag, using 
	   * the value from the packet directly seems a bit more sane.
	   */
	    {
	      username = malloc(krb5_princ_component(context, client, 0)->length + 1);
	      if (!username)
		goto out;
	      strncpy(username, krb5_princ_component(context, client, 0)->data,
		      krb5_princ_component(context, client, 0)->length);
	      username[krb5_princ_component(context, client, 0)->length] = '\0';

	      get_user(username, &(opacket[1]), packet[1],
		       inet_ntoa(from.sin_addr));
	    }

	  inbuf.data = opacket;
	  inbuf.length = strlen(opacket);

	  problem = krb5_mk_priv(context, auth_con, &inbuf, &outbuf, NULL);
	  if (problem)
	    goto out;

	  if (sendto(s, outbuf.data, outbuf.length, 0,
		     (struct sockaddr *)&from, sizeof(from)) < 0)
	    fprintf(stderr, "lertsrv: sendto failed: %s\n", strerror(errno));

	  /* Avoid confusion, zeroize now */
	  memset(&from, 0, sizeof(from));

    out:
	  if (problem)
	    fprintf(stderr, "lertsrv: Kerberos error: %s\n",
		    error_message(problem));
	  /* krb5 library checks if this is allocated for us. */
	  krb5_free_data_contents(context, &outbuf);
	  memset(&outbuf, 0, sizeof(krb5_data));
	  if (client)
	    krb5_free_principal(context, client);
	  if (server)
	    krb5_free_principal(context, server);
	  if (ticket)
	    krb5_free_ticket(context, ticket);
	  if (auth_con)
	    krb5_auth_con_free(context, auth_con);
	  if (username)
	    free(username);
	  /* reset to NULL after freeing since we're going to go through
	   * this loop again.
	   */
	  client = server = NULL;
	  ticket = NULL;
	  auth_con = NULL;
	  username = NULL;
	}
      else
	{
	  fprintf(stderr, "lertsrv: Received packet with bad version: %d\n",
		  packet[0]);
	}
    }

  return 0;
}

int get_user(char *pname, char *result, int onetime, char *ip)
{
  DBM *db;
  DBM *db2;
  datum data;
  datum key;
  datum data2;
  char result2[128];

  /* Prepare nil return. */
  result[0] = LERT_FREE;
  result[1] = '\0';

  /* Open the database. */
  db = dbm_open(LERTS_DATA, O_RDWR, 0600);
  if (db == NULL)
    {
      fprintf(stderr, "Unable to open lert's database file %s: %s.\n",
	      LERTS_DATA, strerror(errno));
      return LERT_NO_DB;
    }

  key.dptr = pname;
  key.dsize = strlen(pname) + 1;

  /* Get the user. */
  data = dbm_fetch(db, key);
  if (data.dptr == NULL)
    {
      /* Not in db. */
      dbm_close(db);
#ifdef LOGGING
      fprintf(stdout, "lertsrv: %s user %s not in db\n", ip, pname);
#endif
      return LERT_NOT_IN_DB;
    }
  else
    {
      strncpy(&(result[1]), data.dptr, data.dsize);
      result[data.dsize + 1] = '\0';

      if (onetime)
	{
	  /* Add them to the log. */
	  db2 = dbm_open(LERTS_LOG, O_RDWR | O_CREAT, 0600);
	  if (db2 == NULL)
	    {
	      fprintf(stderr, "Unable to open lert's log database %s: %s.\n",
		      LERTS_LOG, strerror(errno));
	    }
	  else
	    {
	      data2 = dbm_fetch(db2, key);
	      if (data2.dptr == NULL)
		dbm_store(db2, key, data, DBM_INSERT);
	      else
		{
		  strncpy(result2, data2.dptr, data2.dsize);
		  strncpy(&(result2[data2.dsize]), data.dptr, data.dsize);
		  data2.dptr = result2;
		  data2.dsize += data.dsize;
		  dbm_store(db2, key, data2, DBM_REPLACE);
		}
	      dbm_close(db2);
	    }
	  dbm_delete(db, key);
	}
      dbm_close(db);
    }

  result[0] = LERT_MSG;
#ifdef LOGGING
  fprintf(stdout, "lertsrv: %s user %s in db with groups :%s:\n", ip, pname,
	  result);
#endif
  return LERT_GOTCHA;
}


