/*
 * Definitions for globally shared data used by the Kerberos v5 library
 *
 * $Header: /afs/dev.mit.edu/source/repository/third/krb5/src/mac/libraries/Kerberos v5 Globals/Krb5GlobalsData.c,v 1.1.1.2 1999-12-26 03:33:47 ghudson Exp $
 */
 
#include "Krb5GlobalsData.h"

UInt32	gKerberos5GlobalsRefCount = 0;
UInt32	gKerberos5SystemDefaultCacheNameModification = 0;
char*	gKerberos5SystemDefaultCacheName = nil;

