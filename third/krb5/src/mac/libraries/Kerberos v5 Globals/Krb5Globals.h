/*
 * Decalrations for Kerberos v5 systemwide globals
 *
 * $Header: /afs/dev.mit.edu/source/repository/third/krb5/src/mac/libraries/Kerberos v5 Globals/Krb5Globals.h,v 1.1.1.1 1999-10-05 16:15:53 ghudson Exp $
 */

#ifndef __Krb5Globals_h__
#define __Krb5Globals_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
#	pragma import on
#endif

/*
 * Set the default cache name
 *
 * inName is a C string with the name of the new cache
 *
 * returns: noErr, memFullErr
 */

OSStatus
Krb5GlobalsSetDefaultCacheName (
	char*	inName);

/*
 * Retrieve the default cache name
 *
 * inName should point to at least inLength bytes of storage
 * if inName is nil, just returns the length
 * 
 * returns: length of default cache name
 */

UInt32
Krb5GlobalsGetDefaultCacheName (
	char*	inName,
	UInt32	inLength);

/*
 * Set the default cache name to a unique string
 *
 * Sets the default cache name to a string that is not the name
 * of an existing cache
 */

OSStatus
Krb5GlobalsSetUniqueDefaultCacheName ();

/*
 * Get modification number
 *
 * Modification number changes whenever default cache name changes
 */

UInt32
Krb5GlobalsGetDefaultCacheNameModification ();
	
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
#	pragma import reset
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __Krb5Globals_h__ */