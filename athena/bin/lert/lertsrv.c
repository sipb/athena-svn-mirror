/*

   file: lertsrv.c
   provides the core of the lert msg service
   take a message, check the authentication, grab a user record, and return.

   based on cmisrv.c: checkmyid server process

 */

#include <stdio.h>
#include <krb.h>
#include <des.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <ndbm.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <hesiod.h>
#include <strings.h>
#include <pwd.h>
#include <sys/utsname.h>
#include "lert.h"

extern char *sys_errlist[];
extern int errno;

main(argc, argv)
int argc;
char *argv[];
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
/*
  char hostname[256];
 */
  struct utsname thishost;

#ifdef LOGGING
    setbuf(stdout, NULL);
#endif
  
  s = socket(AF_INET, SOCK_DGRAM, 0);

  /* 
    clear the socket structure 
    */
  bzero(&sin, sizeof(sin));	
  sin.sin_family = AF_INET;
  sin.sin_port = htons(LERT_PORT);
  burp_me = (struct sockaddr *)&sin;

  if(bind(s, burp_me, sizeof(sin)) < 0) {
    fprintf(stderr, "%s: Unable to bind socket: %s\n",
	    argv[0], sys_errlist[errno]);
    exit (1);
  }

  fromlen = sizeof(from);


  /* 
    for krb_mk_priv
   */

/*
  gethostname(hostname, sizeof(hostname));
 */
  if (uname(&thishost) == -1) {
    fprintf(stderr, "%s: Unable to get system information: %s\n",
	    argv[0], sys_errlist[errno]);
    exit (1);
  }
  hp = gethostbyname(thishost.nodename);
  if (hp == NULL) {
    fprintf(stderr, "%s: Unable to get host name information: %s\n",
	    argv[0], sys_errlist[errno]);
    exit (1);
  }

  bcopy(hp->h_addr, (char *) &sin.sin_addr.s_addr, hp->h_length);
 

  for (;;) {
    plen = recvfrom(s, packet, 2048, 0, (struct sockaddr *) &from, &fromlen);
    if (plen < 0) {
      fprintf(stderr, "lertsrv: Error during recv: %s\n",
	      sys_errlist[errno]);
      sleep (1);		/* Prevent infinite cpu hog loop */
      continue;
    }

#ifdef DEBUG
    fprintf(stderr, "lertsrv: Received packet of length %d\n", plen);
#endif

    if (packet[0] != LERT_VERSION) {
      fprintf(stderr, "lertsrv: Received packet with bad version: %d\n",
	      packet[0]);
      continue;
    }
    
    /* 
      The bcopy below is necessary so that on the DECstation the authent
      is word aligned  
      */
    (void) bcopy((char *)&packet[LERT_LENGTH], (char *) &authent, sizeof(authent));

    status = krb_rd_req(&authent, LERT_SERVICE, LERT_HOME, 
			/*
			  from.sin_addr,
			  but other code showed 0 for any from address
			  */
			0,
			/*
			  if you get a srvtab, make sure you put it in,
			  if default, use ""
			  */
			&ad, LERTS_SRVTAB);
    
    if (status != KSUCCESS) {
      fprintf(stderr, "lertsrv: Kerberos failure: %s\n", krb_err_txt[status]);
      continue;
    }

    opacket[0] = LERT_VERSION;
    if ((strlen(ad.pinst) != 0) || (strcmp(ad.prealm, "ATHENA.MIT.EDU"))) {
      fprintf(stderr, "lertsrv: %s.%s@%s -- Not null instance ATHENA realm.\n",
	      ad.pname, ad.pinst, ad.prealm);
      opacket[1] = LERT_NOT_IN_DB;
      opacket[2] = '\0';
    } else {
      get_user(ad.pname, &(opacket[1]), ad.checksum);
    }
    /* 
      PREPARE KRB_MK_PRIV MESSAGE
      */

    /* 
      Get key schedule for session key
      for a while, inadvertently fed empty ad.session in
      and it seemed to be okay!
      resulted in client not being able to decode, with
      41 - data stream modified - error return
      */
    des_key_sched(ad.session, sched);

    /* 
      Make the encrypted message
      length - version, code, data, plus terminating '\0'
      */
    len = krb_mk_priv(opacket, 
		      ktxt.dat, 
		      LERT_CHECK + (strlen(&(opacket[LERT_CHECK]))) + 1,
		      sched, 
		      ad.session, 
		      &sin,
		      &from);

    /* 
      Send it
      */

    if(sendto(s, 
	      (char *)ktxt.dat, 
	      len, 
	      0,   /* flags! */
	      (struct sockaddr *) &from, 
	      sizeof(from)) < 0) {
      fprintf(stderr, "lertsrv: sendto failed: %s\n",
	      sys_errlist[errno]);
    }
    bzero(&from, sizeof(from));	/* Avoid confusion, zeroize now */
  }
  return (0);
}

/*
   options:
   1.  open, read, and close db each access
   2.  open db and hold state.  do fstat or other testing to
       see about changes, close and reopen if needed.
   3.  open it.  hope to hell administrators don't forget to kill and
       reboot if they make any changes.  keep lucky rabbit's foot
       close for rubbing during inevitable problems...
   4.  probably others, if I spent more time thinking about it.

selected: open, read, close.  no state information needed.  may be
slow if we start getting lots of use (30,000 logins...that could be a
lot of use...) but it's safe and easy.

speed up--open and do fstat's, I suppose.  could read the db into
memory, then make the update program signal us to reread, I guess, but
let's do simple first.

 */

get_user(pname, result, onetime)
char * pname;
char * result;
int onetime;
{
  DBM *dbm_open();
  DBM *db;
  DBM *db2;
  datum data;
  datum key;
  datum data2;
  datum key2;
  char result2[128];

  /*
    prepare nil return
    */
  result[0] = LERT_FREE;
  result[1] = '\0';

  /*
    open the database
    */
  db = dbm_open(LERTS_DATA, O_RDWR, 0600);
  if (db == NULL) {
    fprintf(stderr, "Unable to open lert's database file %s: %s.\n",
	    LERTS_DATA, sys_errlist[errno]);
    return(LERT_NO_DB);
  }

  key.dptr = pname;
  key.dsize = strlen(pname) + 1;

  /*
    get the user
    */

  data = dbm_fetch(db, key);
  if (data.dptr == NULL) {
    /* not in db */
    dbm_close(db);
#ifdef LOGGING
    fprintf(stdout, "lertsrv: user %s not in db\n", pname);
#endif
    return(LERT_NOT_IN_DB);
  } else {
    /*
      gotta
      */
    strncpy(&(result[1]), data.dptr, data.dsize);
    result[data.dsize + 1] = '\0';
    /*
      conditional on the user requesting one time
      as long as they are live in the regular db, leave them there.
      */
    if (onetime) {
      /*
	log them in db
	*/
      db2 = dbm_open(LERTS_LOG, O_RDWR|O_CREAT, 0600);
      if (db2 == NULL) {
	fprintf(stderr, "Unable to open lert's log database %s: %s.\n",
		LERTS_LOG, sys_errlist[errno]);
      } else {
	data2 = dbm_fetch(db2, key);
	if (data2.dptr == NULL) {
	  /* not in db */
	  dbm_store(db2, key, data, DBM_INSERT);
	} else {
	  /* in db ... build a string... */
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
  return(LERT_GOTCHA);
}


