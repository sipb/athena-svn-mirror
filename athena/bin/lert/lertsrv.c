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

static const char rcsid[] = "$Id: lertsrv.c,v 1.5 1999-12-09 22:24:24 danw Exp $";

#include <stdio.h>
#include <krb.h>
#include <des.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <hesiod.h>
#include <string.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <errno.h>
#include "lert.h"

int main(int argc, char **argv)
{
  int s;			/* The Socket */
  register int i;		/* Iterator variable */
  struct sockaddr_in sin;
  struct sockaddr *burp_me;
  struct sockaddr_in from;
  char packet[2048];
  int plen;
  char opacket[2048];
  int oplen;
  int fromlen;
  int status;
  int len;
  AUTH_DAT ad;
  KTEXT_ST authent;
  KTEXT_ST ktxt;
  int atime;
  char *tb, *cp;
  FILE *logf;
  /* for krb_mk_priv */
  des_key_schedule sched;		/* session key schedule */
  struct hostent *hp;
  struct utsname thishost;

#ifdef LOGGING
  setbuf(stdout, NULL);
#endif

  s = socket(AF_INET, SOCK_DGRAM, 0);

  /* Clear the socket structure. */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(LERT_PORT);
  burp_me = (struct sockaddr *)&sin;

  if (bind(s, burp_me, sizeof(sin)) < 0)
    {
      fprintf(stderr, "%s: Unable to bind socket: %s\n",
	      argv[0], strerror(errno));
      exit(1);
  }

  fromlen = sizeof(from);

  /* Get hostname for krb_mk_priv. */
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

  memcpy(&sin.sin_addr.s_addr, hp->h_addr, hp->h_length);

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

      if (packet[0] != LERT_VERSION)
	{
	  fprintf(stderr, "lertsrv: Received packet with bad version: %d\n",
		  packet[0]);
	  continue;
	}

      /* Copy authenticator into aligned region. */
      memcpy(&authent, &packet[LERT_LENGTH], sizeof(authent));

      status = krb_rd_req(&authent, LERT_SERVICE, LERT_HOME, 0, &ad,
			  LERTS_SRVTAB);

      if (status != KSUCCESS)
	{
	  fprintf(stderr, "lertsrv: Kerberos failure: %s\n",
		  krb_err_txt[status]);
	  continue;
	}

      opacket[0] = LERT_VERSION;
      if ((strlen(ad.pinst) != 0) || (strcmp(ad.prealm, "ATHENA.MIT.EDU")))
	{
	  fprintf(stderr, "lertsrv: %s.%s@%s -- Not null instance ATHENA "
		  "realm.\n", ad.pname, ad.pinst, ad.prealm);
	  opacket[1] = LERT_NOT_IN_DB;
	  opacket[2] = '\0';
	}
      else
	get_user(ad.pname, &(opacket[1]), ad.checksum);

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

      memset(&from, 0, sizeof(from));	/* Avoid confusion, zeroize now */
    }

  return 0;
}

int get_user(char *pname, char *result, int onetime)
{
  DBM *db;
  DBM *db2;
  datum data;
  datum key;
  datum data2;
  datum key2;
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
      fprintf(stdout, "lertsrv: user %s not in db\n", pname);
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
  fprintf(stdout, "lertsrv: user %s in db with groups :%s:\n", pname, result);
#endif
  return LERT_GOTCHA;
}


