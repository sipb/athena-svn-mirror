/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/gdss/lib/gdss.c,v $
 * $Author: jis $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/gdss/lib/gdss.c,v 1.1 1991-11-09 18:08:40 jis Exp $
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

int GDSS_Sig_Info(Signature, SignatureLen, aSigInfo)
unsigned char *Signature;
unsigned int SignatureLen;
SigInfo *aSigInfo;
{
  int status;
  static int fetchkey();
  unsigned char hash[16];

  bzero(aSigInfo, sizeof(SigInfo));
  aSigInfo->SigInfoVersion = 0;
  do {
    status = fetchkey();
    if (status) break;
    status = gdss_rverify(Signature, SignatureLen, hash, &(*aSigInfo).pname,
			  &(*aSigInfo).pinst, &(*aSigInfo).prealm,
			  &gdss_pubkey, &(*aSigInfo).timestamp);
  } while(0);
  return (status);
}

int GDSS_Verify(Data, DataLen, Signature, SignatureLen)
unsigned char *Data;
unsigned int DataLen;
unsigned char *Signature;
unsigned char SignatureLen;
{
  unsigned char hash[16], digest[16];
  SigInfo aSigInfo;
  int status;
  status = fetchkey();
  if (status) return (status);

  status = gdss_rverify(Signature, SignatureLen, hash, &aSigInfo.pname,
			&aSigInfo.pinst, &aSigInfo.prealm,
			&gdss_pubkey, &aSigInfo.timestamp);
  if (status) return (status);
  RSA_MD2(Data, DataLen, digest);
  if(bcmp(digest, hash, 16)) return (-1);
  return (0);
}

int GDSS_Sig_Size()
{
  static int fetchkey();
  int status;
  int retval;
  status = fetchkey();
  if (status) retval = 512;	/* No provision for errors, so default value */
  retval = sizeof(SigInfo) + (gdss_pubkey.nl)*4;
  return (retval);
}

static int fetchkey()
{
  FILE *keyf;
  unsigned char buffer[512];

  if (gdss_have_key) return (0);
  keyf = fopen("/tmp/gdss_public_key", "r");
  if (keyf == NULL) return (-1);
  fread(buffer, 1, 512, keyf);
  fclose(keyf);
  DecodePublic(buffer, &gdss_pubkey);
  gdss_have_key++;
  return (0);
}
