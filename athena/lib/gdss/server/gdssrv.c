/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/gdss/server/gdssrv.c,v $
 * $Author: jis $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/gdss/server/gdssrv.c,v 1.2 1992-05-13 13:07:28 jis Exp $
 *
 * GDSS The Generic Digital Signature Service
 *
 * gdssrv.c: Top Level of the GDSS Signature Server Process
 */

#include <BigNum.h>
#include <BigRSA.h>
#include <krb.h>
#include <gdss.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

static RSAKeyStorage gdss_private_key;
static int gdss_have_key = 0;

main(argc, argv)
int argc;
char *argv[];
{
  int s;			/* The Socket */
  register int i;		/* Iterator variable */
  struct sockaddr_in sin;
  struct sockaddr_in from;
  char packet[2048];
  int plen;
  char opacket[2048];
  int oplen;
  int fromlen;
  extern char *sys_errlist[];
  extern int errno;
  static int fetchkey();
  int status;
  AUTH_DAT ad;
  KTEXT_ST authent;

  if (fetchkey()) {
    fprintf(stderr, "gdssrv: Unable to get GDSS Private Key\n");
    exit (1);
  }
  s = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&sin, sizeof(sin));	/* Zeroize socket structure */
  sin.sin_family = AF_INET;
  sin.sin_port = htons(7201);

  if(bind(s, &sin, sizeof(sin)) < 0) {
    fprintf(stderr, "gdssrv: Unable to bind socket: %s\n",
	    sys_errlist[errno]);
    exit (1);
  }
  for (;;) {
    fromlen = sizeof(from);
    plen = recvfrom(s, packet, 2048, 0, &from, &fromlen);
    if (plen < 0) {
      fprintf(stderr, "gdssrv: Error during recv: %s\n",
	      sys_errlist[errno]);
      sleep (1);		/* Prevent infinite cpu hog loop */
      continue;
    }
#ifdef DEBUG
    fprintf(stderr, "gdssrv: Received packet of length %d\n", plen);
#endif
    if (packet[0] != '\0') {
      fprintf(stderr, "gdssrv: Received packet with bad version: %d\n",
	      packet[0]);
      continue;
    }
    /* Call krb_rd_req here and verify hash of packet */
    
/* The bcopy below is necessary so that on the DECstation the authent
   is word aligned */
    (void) bcopy((char *)&packet[17], (char *) &authent, sizeof(authent));

    status = krb_rd_req(&authent, "gdss", "big-screw", from.sin_addr,
			&ad, "/etc/gdss.srvtab");

    if (status != KSUCCESS) {
      fprintf(stderr, "gdssrv: Kerberos failure: %s\n", krb_err_txt[status]);
      continue;
    }
#ifdef DEBUG
    fprintf(stderr, "gdssrv: Making Signature for: %s.%s@%s\n",
	    ad.pname, ad.pinst, ad.prealm);
#endif

    oplen = 2048;
    if(gdss_rsign(opacket, &packet[1], ad.pname, ad.pinst,
		  ad.prealm, &gdss_private_key) != 0) {
      fprintf(stderr, "gdssrv: gdss_rsign failed.\n");
      continue;
    }
    if(sendto(s, opacket, strlen(opacket) + 1, 0, &from, sizeof(from)) < 0) {
      fprintf(stderr, "gdssrv: sento failed: %s\n",
	      sys_errlist[errno]);
    }
    bzero(&from, sizeof(from));	/* Avoid confusion, zeroize now */
  }
  return (0);			/* Needless statement, make lint happy */
}

static int fetchkey()
{
  FILE *keyf;
  unsigned char buffer[512];

  if (gdss_have_key) return (0);
  keyf = fopen("/etc/athena/gdss_private_key", "r");
  if (keyf == NULL) return (-1);
  fread(buffer, 1, 512, keyf);
  fclose(keyf);
  DecodePrivate(buffer, &gdss_private_key);
  gdss_have_key++;
  return (0);
}
