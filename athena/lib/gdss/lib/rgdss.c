/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/gdss/lib/rgdss.c,v $
 * $Author: jis $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/gdss/lib/rgdss.c,v 1.1 1991-11-09 18:08:48 jis Exp $
 */
/*
 * GDSS The Generic Digital Signature Service
 *
 * rgdss.c: Raw signature signing and verification routines.
 */

#include <BigNum.h>
#include <BigRSA.h>
#include <krb.h>

int gdss_rsign(signature, siglen, hash, name, instance, realm, key)
unsigned char *signature;
int *siglen;
unsigned char *hash;
char *name;
char *instance;
char *realm;
RSAKeyStorage *key;
{
  unsigned char *cp, *ip;
  int sigblen;
  unsigned int the_time;
  register int i;
  void time();

  /* asigblen = max name sizes plus modulus size (signature) + hash + time */
  sigblen = ANAME_SZ + REALM_SZ + INST_SZ + 16 + 4 + 4*(key->nl) + 5;
  if (*siglen < sigblen) return (-1);
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
	      siglen)) return (-1);
  *siglen += cp - signature;
  return (0);
}

int gdss_rverify(signature, siglen, hash, name, instance, realm, key, the_time)
unsigned char *signature;
int siglen;
unsigned char *hash;
char *name;
char *instance;
char *realm;
RSAKeyStorage *key;
unsigned int *the_time;
{
  unsigned char *cp, *ip;
  register int i;

  cp = signature;
  if (*cp++ != 0x43) return (-1); /* Bad Version */
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
	      siglen - (cp - signature))) return (-1);
  return (0);
}
    
