/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/gdss/lib/gdss.c,v $
 * $Author: jis $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/gdss/lib/gdss.c,v 1.5 1992-05-13 13:07:08 jis Exp $
 */
/*
 * GDSS The Generic Digital Signature Service
 *
 * gdss.c: Main interface routines
 */

#include <BigNum.h>
#include <BigRSA.h>
#include <krb.h>
#include <gdss.h>
#include <stdio.h>

static RSAKeyStorage gdss_pubkey;
static int gdss_have_key;

int GDSS_Sig_Info(Signature, aSigInfo)
unsigned char *Signature;
SigInfo *aSigInfo;
{
  int status;
  static int fetchkey();
  unsigned char hash[16];
  unsigned char *cp;

  cp = aSigInfo->rawsig;
  bzero(aSigInfo, sizeof(SigInfo));
  aSigInfo->rawsig = cp;
  aSigInfo->SigInfoVersion = 0;
  do {
    status = fetchkey();
    if (status) break;
    status = gdss_rverify(Signature, hash, &(*aSigInfo).pname,
			  &(*aSigInfo).pinst, &(*aSigInfo).prealm,
			  &gdss_pubkey, &(*aSigInfo).timestamp,
			  aSigInfo->rawsig);
  } while(0);
  return (status);
}

int GDSS_Verify(Data, DataLen, Signature)
unsigned char *Data;
unsigned int DataLen;
unsigned char *Signature;
{
  unsigned char hash[16], digest[16];
  SigInfo aSigInfo;
  int status;
  status = fetchkey();
  if (status) return (status);

  status = gdss_rverify(Signature, hash, &aSigInfo.pname,
			&aSigInfo.pinst, &aSigInfo.prealm,
			&gdss_pubkey, &aSigInfo.timestamp, NULL);
  if (status) return (status);
  RSA_MD2(Data, DataLen, digest);
  if(bcmp(digest, hash, 16)) return (GDSS_E_BADSIG);
  return (GDSS_SUCCESS);
}

int GDSS_Sig_Size()
{
  static int fetchkey();
  int status;
  int retval;
  status = fetchkey();
  if (status) retval = 512;	/* No provision for errors, so default value */
  retval = sizeof(SigInfo) + (gdss_pubkey.nl)*4 + GDSS_PAD + 5;
  return (retval);
}

static int fetchkey()
{
  FILE *keyf;
  unsigned char buffer[512];

  if (gdss_have_key) return (0);
  keyf = fopen("/etc/athena/gdss_public_key", "r");
  if (keyf == NULL) return (GDSS_E_NOPUBKEY);
  fread(buffer, 1, 512, keyf);
  fclose(keyf);
  DecodePublic(buffer, &gdss_pubkey);
  gdss_have_key++;
  return (GDSS_SUCCESS);
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

static struct timeval timeout = { CLIENT_KRB_TIMEOUT, 0 };
GDSS_Sign(Data, DataLen, Signature)
unsigned char *Data;
unsigned int DataLen;
unsigned char *Signature;
{
  KTEXT_ST authent;
  char lrealm[REALM_SZ];
  unsigned char hash[16];
  unsigned char dhash[16];	/* Second level hash */
  int s;			/* Socket to do i/o on */
  int status;
  register int i;
  unsigned int cksum;
  unsigned char packet[2048];
  int plen;
  unsigned char ipacket[2048];
  int iplen;
  struct hostent *hp;
  struct sockaddr_in sin, lsin;
  fd_set readfds;
  int trys;

  bzero(packet, sizeof(packet)); /* Zeroize Memory */
  bzero(ipacket, sizeof(ipacket));
  krb_get_lrealm(lrealm, 1);	/* Get our Kerberos realm */

  RSA_MD2(Data, DataLen, hash);
  RSA_MD2(hash, 16, dhash);	/* For use of Kerberos */
  bcopy(hash, packet, 16);

  cksum = 0;
  for (i = 0; i < 4; i++)	/* High order 32 bits of dhash is the
				   Kerberos checksum, I wish we could do
				   better, but this is all kerberos allows
				   us */
    cksum = (cksum << 8) + dhash[i];

  /* Use Hesiod to find service location of GDSS Server Here */

  hp = gethostbyname("big-screw");	/* Should use Hesiod  */

  if(hp == NULL) return (-1);	/* Could not find host, you lose */

  status = krb_mk_req(&authent, "gdss", "big-screw", lrealm, cksum);
  if (status != KSUCCESS) return (GDSS_E_KRBFAIL);
  packet[0] = 0;		/* Version 0 of protocol */
  (void) bcopy((char *)hash, (char *)&packet[1], 16);
  (void) bcopy((char *)&authent, (char *)&packet[17], sizeof(authent));
  plen = sizeof(authent) + 16 + 1;	/* KTEXT_ST plus the hash + version */

  s = -1;			/* "NULL" Value for socket */
  do {
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
      status = GDSS_E_NOSOCKET;
      break;
    }
    bzero(&sin, sizeof(sin));
    sin.sin_family = hp->h_addrtype;
    (void) bcopy((char *)hp->h_addr, (char *)&sin.sin_addr,
		 sizeof(hp->h_addr));
    sin.sin_port = htons(7201);	/* Should get this from services or Hesiod */
    if (connect(s, &sin, sizeof(sin)) < 0) {
      status = GDSS_E_NOCONNECT;
      break;
    }
    trys = 3;
    status = GDSS_E_TIMEDOUT;
    while (trys > 0) {
      if(send(s, packet, plen, 0) < 0) break;
      FD_ZERO(&readfds);
      FD_SET(s, &readfds);
      if ((select(s+1, &readfds, (fd_set *)0, (fd_set *)0, &timeout) < 1)
	  || !FD_ISSET(s, &readfds)) {
	trys--;
	continue;
      }
      if((iplen = recv(s, (char *)ipacket, 2048, 0)) < 0) break;
      status = GDSS_SUCCESS;
      break;
    }
  } while (0);
  shutdown(s);
  close(s);
  if (status != GDSS_SUCCESS) return (status);
  (void) bcopy((char *)ipacket, (char *)Signature, iplen);
  return (GDSS_SUCCESS);
}
