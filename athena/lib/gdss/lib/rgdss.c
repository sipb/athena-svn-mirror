/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/gdss/lib/rgdss.c,v $
 * $Author: jis $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/gdss/lib/rgdss.c,v 1.2 1992-05-13 13:06:57 jis Exp $
 */
/*
 * GDSS The Generic Digital Signature Service
 *
 * rgdss.c: Raw signature signing and verification routines.
 */

#include <BigNum.h>
#include <BigRSA.h>
#include <krb.h>
#include <gdss.h>

#define	NULL	(unsigned char *)0

int gdss_rsign(signature, hash, name, instance, realm, key)
unsigned char *signature;
unsigned char *hash;
char *name;
char *instance;
char *realm;
RSAKeyStorage *key;
{
  unsigned char *cp, *ip;
  unsigned int the_time;
  register int i;
  void time();
  register int status;
  int loopcnt;
  int siglen;

#ifdef notdef
  /* asigblen = max name sizes plus modulus size (signature) + hash + time */
  sigblen = ANAME_SZ + REALM_SZ + INST_SZ 
    + 16 /* hash size */  + 4 /* time size */
      + 4*(key->nl) /* modulus size in bytes */
	+ 5 /* null byte at end plus 4 bytes of fudge factor */
	  + GDSS_PAD; /* padding needed for removing zeros from signature */
#endif

  for (loopcnt = 0; loopcnt < 10; loopcnt++) {
    cp = signature;
    *cp++ = 0x43;   /* Version Number */
    ip = (unsigned char *) name;
    while (*cp++ = *ip++);
    ip = (unsigned char *) instance;
    while (*cp++ = *ip++);
    ip = (unsigned char *) realm;
    while (*cp++ = *ip++);
    time(&the_time);
    *cp++ = ((the_time) >> 24) & 0xff;
    *cp++ = ((the_time) >> 16) & 0xff;
    *cp++ = ((the_time) >> 8) & 0xff;
    *cp++ = the_time & 0xff;
    for (i = 0; i < 16; i++)
      *cp++ = hash[i];
    if(!RSASign(signature, cp - signature, key, &signature[cp - signature],
		&siglen)) return (-1);
    status = gdss_rpadout(signature, cp - signature + siglen);
    if ((status == GDSS_SUCCESS) || (status != GDSS_E_PADTOOMANY))
      return (status);
    sleep(1);			/* Allow time to change */
  }
  return (GDSS_E_PADTOOMANY);
}

/* gdss_rpadout: Remove null bytes from signature by replacing them with
   the sequence GDSS_ESCAPE, GDSS_NULL. Keep track of how much bigger
   the signature block is getting and abort if too many bytes (more than
   GDSS_PAD) would be required.
*/

int gdss_rpadout(signature, siglen)
unsigned char *signature;
int siglen;
{
  register unsigned char *cp;
  register unsigned char *bp;
  unsigned char *buf;
  register int i;
  register int c;
  buf = (unsigned char *)malloc(siglen + GDSS_PAD + 1); /* 1 for the null! */
  if (buf == NULL) return (GDSS_E_ALLOC);
  bzero(buf, siglen + GDSS_PAD + 1); /* Just to be safe */
  bp = buf;
  cp = signature;
  c = 0;
  for (i = 0; i < siglen; i++) {
    if ((*cp != '\0') && (*cp != GDSS_ESCAPE)) {
      *bp++ = *cp++;
      continue;
    }
    if (c++ > GDSS_PAD) {
      free(buf);		/* Don't have to zeroize, nothing
				   confidential */
      return (GDSS_E_PADTOOMANY);
    }
    *bp++ = GDSS_ESCAPE;
    *bp++ = (*cp == '\0') ? GDSS_NULL : GDSS_ESCAPE;
    cp++;
  }
  *bp++ = '\0';			/* Null Terminate */
  bcopy(buf, signature, bp - buf);
  free(buf);
  return (GDSS_SUCCESS);
}
  
int gdss_rpadin(signature, outlen)
unsigned char *signature;
int *outlen;
{
  unsigned char *buf;
  register unsigned char *cp;
  register unsigned char *bp;
  buf = (unsigned char *) malloc(strlen(signature));
  if (buf == NULL) return (GDSS_E_ALLOC);
  bp = buf;
  cp = signature;
  while (*cp) {
    if (*cp != GDSS_ESCAPE) {
      *bp++ = *cp++;
      continue;
    }
    if (*(++cp) == GDSS_NULL) {
      *bp++ = '\0';
    } else *bp++ = GDSS_ESCAPE;
    cp++;
  }
  *outlen = bp - buf;
  bcopy(buf, signature, *outlen);
  free (buf);
  return (GDSS_SUCCESS);
}

int gdss_rverify(isignature, hash, name, instance,
		 realm, key, the_time, rawsig)
unsigned char *isignature;
unsigned char *hash;
char *name;
char *instance;
char *realm;
RSAKeyStorage *key;
unsigned int *the_time;
unsigned char *rawsig;
{
  unsigned char *cp, *ip;
  register int i;
  int status;
  int siglen;
  unsigned char *signature;

  if (*isignature != 0x43) return (GDSS_E_BVERSION); /* Bad Version */

  signature = (unsigned char *) malloc (strlen(isignature) + 1);
  strcpy(signature, isignature);

  status = gdss_rpadin(signature, &siglen);
  if (status) return (status);
  
  cp = signature;
  if (*cp++ != 0x43) return (GDSS_E_BVERSION); /* Bad Version */
  ip = (unsigned char *) name;
  while (*ip++ = *cp++);
  ip = (unsigned char *) instance;
  while (*ip++ = *cp++);
  ip = (unsigned char *) realm;
  while (*ip++ = *cp++);
  *the_time = 0;
  *the_time |= *cp++ << 24;
  *the_time |= *cp++ << 16;
  *the_time |= *cp++ << 8;
  *the_time |= *cp++;
  for (i = 0; i < 16; i++)
    hash[i] = *cp++;
  if(!RSAVerify(signature, cp - signature, key, &signature[cp - signature],
	      siglen - (cp - signature))) {
    free (signature);
    return (GDSS_E_BADSIG);
  }
  if (rawsig == NULL) {
    free (signature);
    return (GDSS_SUCCESS);
  }
  bcopy(&signature[cp - signature], rawsig, siglen - (cp - signature));
  status = gdss_rpadout(rawsig, siglen - (cp - signature));
  free (signature);
  return (status);
}
    
