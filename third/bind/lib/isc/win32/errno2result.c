/*
 * Copyright (C) 2000-2002  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
 * INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: errno2result.c,v 1.1.1.2 2002-06-07 05:28:49 ghudson Exp $ */

#include <config.h>

#include <winsock2.h>
#include "errno2result.h"
#include <isc/result.h>
#include <isc/strerror.h>
#include <isc/util.h>

/*
 * Convert a POSIX errno value into an isc_result_t.  The
 * list of supported errno values is not complete; new users
 * of this function should add any expected errors that are
 * not already there.
 */
isc_result_t
isc__errno2result(int posixerrno) {
	char strbuf[ISC_STRERRORSIZE];

	switch (posixerrno) {
	case ENOTDIR:
	case WSAELOOP:
	case EINVAL:		/* XXX sometimes this is not for files */
	case ENAMETOOLONG:
	case EBADF:
		return (ISC_R_INVALIDFILE);
	case ENOENT:
		return (ISC_R_FILENOTFOUND);
	case EACCES:
	case EPERM:
		return (ISC_R_NOPERM);
	case EEXIST:
		return (ISC_R_FILEEXISTS);
	case EIO:
		return (ISC_R_IOERROR);
	case ENOMEM:
		return (ISC_R_NOMEMORY);
	case ENFILE:
	case EMFILE:
		return (ISC_R_TOOMANYOPENFILES);
	default:
		isc__strerror(posixerrno, strbuf, sizeof(strbuf));
		UNEXPECTED_ERROR(__FILE__, __LINE__,
				 "unable to convert errno "
				 "to isc_result: %d: %s",
				 posixerrno, strbuf);
		/*
		 * XXXDCL would be nice if perhaps this function could
		 * return the system's error string, so the caller
		 * might have something more descriptive than "unexpected
		 * error" to log with.
		 */
		return (ISC_R_UNEXPECTED);
	}
}

/*
 * Note this will cause a memory leak unless the memory allocated here
 * is freed by calling LocalFree
 */
char *
FormatError(int error) {
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		/* Default language */
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL); 

	return (lpMsgBuf);
}

char * __cdecl
NTstrMessage(int err) {
	char *retmsg = NULL;

	/* Copy the error value first in case of other errors */	
	DWORD errval = err; 

	/* Get the Winsock2 error messages */
	if (errval >= WSABASEERR && errval <= (WSABASEERR + 1015)) {
		retmsg = GetWSAErrorMessage(errval);
		if (retmsg != NULL)
			return (retmsg);
	}
	/*
	 * If it's not one of the standard Unix error codes,
	 * try a system error message
	 */
	if (errval > (DWORD) _sys_nerr) {
		return (FormatError(errval));
	} else {
		return (strerror(errval));
	}
}

char * __cdecl
NTstrerror(int err) {
	/* Copy the error value first in case of other errors */
	DWORD errval = err; 

	return (NTstrMessage(errval));
}

/*
 * This is a replacement for perror, but it also reports the error value.
 */
void __cdecl
NTperror(char *errmsg) {
	/* Copy the error value first in case of other errors */
	int errval = errno; 

	fprintf(stderr, "%s: %s\n", errmsg, NTstrMessage(errval));
}

/*
 * Return the error string related to Winsock2 errors.
 * This function is necessary since FormatMessage knows nothing about them
 * and there is no function to get them.
 */
char *
GetWSAErrorMessage(int errval) {
	char *msg;

	switch (errval) {

	case WSAEINTR:
		msg = "Interrupted system call";
		break;

	case WSAEBADF:
		msg = "Bad file number";
		break;

	case WSAEACCES:
		msg = "Permission denied";
		break;

	case WSAEFAULT:
		msg = "Bad address";
		break;

	case WSAEINVAL:
		msg = "Invalid argument";
		break;

	case WSAEMFILE:
		msg = "Too many open sockets";
		break;

	case WSAEWOULDBLOCK:
		msg = "Operation would block";
		break;

	case WSAEINPROGRESS:
		msg = "Operation now in progress";
		break;

	case WSAEALREADY:
		msg = "Operation already in progress";
		break;

	case WSAENOTSOCK:
		msg = "Socket operation on non-socket";
		break;

	case WSAEDESTADDRREQ:
		msg = "Destination address required";
		break;

	case WSAEMSGSIZE:
		msg = "Message too long";
		break;

	case WSAEPROTOTYPE:
		msg = "Protocol wrong type for socket";
		break;

	case WSAENOPROTOOPT:
		msg = "Bad protocol option";
		break;

	case WSAEPROTONOSUPPORT:
		msg = "Protocol not supported";
		break;

	case WSAESOCKTNOSUPPORT:
		msg = "Socket type not supported";
		break;

	case WSAEOPNOTSUPP:
		msg = "Operation not supported on socket";
		break;

	case WSAEPFNOSUPPORT:
		msg = "Protocol family not supported";
		break;

	case WSAEAFNOSUPPORT:
		msg = "Address family not supported";
		break;

	case WSAEADDRINUSE:
		msg = "Address already in use";
		break;

	case WSAEADDRNOTAVAIL:
		msg = "Can't assign requested address";
		break;

	case WSAENETDOWN:
		msg = "Network is down";
		break;

	case WSAENETUNREACH:
		msg = "Network is unreachable";
		break;

	case WSAENETRESET:
		msg = "Net connection reset";
		break;

	case WSAECONNABORTED:
		msg = "Software caused connection abort";
		break;

	case WSAECONNRESET:
		msg = "Connection reset by peer";
		break;

	case WSAENOBUFS:
		msg = "No buffer space available";
		break;

	case WSAEISCONN:
		msg = "Socket is already connected";
		break;

	case WSAENOTCONN:
		msg = "Socket is not connected";
		break;

	case WSAESHUTDOWN:
		msg = "Can't send after socket shutdown";
		break;

	case WSAETOOMANYREFS:
		msg = "Too many references: can't splice";
		break;

	case WSAETIMEDOUT:
		msg = "Connection timed out";
		break;

	case WSAECONNREFUSED:
		msg = "Connection refused";
		break;

	case WSAELOOP:
		msg = "Too many levels of symbolic links";
		break;

	case WSAENAMETOOLONG:
		msg = "File name too long";
		break;

	case WSAEHOSTDOWN:
		msg = "Host is down";
		break;

	case WSAEHOSTUNREACH:
		msg = "No route to host";
		break;

	case WSAENOTEMPTY:
		msg = "Directory not empty";
		break;

	case WSAEPROCLIM:
		msg = "Too many processes";
		break;

	case WSAEUSERS:
		msg = "Too many users";
		break;

	case WSAEDQUOT:
		msg = "Disc quota exceeded";
		break;

	case WSAESTALE:
		msg = "Stale NFS file handle";
		break;

	case WSAEREMOTE:
		msg = "Too many levels of remote in path";
		break;

	case WSASYSNOTREADY:
		msg = "Network system is unavailable";
		break;

	case WSAVERNOTSUPPORTED:
		msg = "Winsock version out of range";
		break;

	case WSANOTINITIALISED:
		msg = "WSAStartup not yet called";
		break;

	case WSAEDISCON:
		msg = "Graceful shutdown in progress";
		break;
/*
	case WSAHOST_NOT_FOUND:
		msg = "Host not found";
		break;

	case WSANO_DATA:
		msg = "No host data of that type was found";
		break;
*/
	default:
		msg = NULL;
		break;
	}
	return (msg);
}

/*
 * These error messages are more informative about CryptAPI Errors than the
 * standard error messages
 */

char *
GetCryptErrorMessage(int errval) {
	char *msg;

	switch (errval) {

	case NTE_BAD_FLAGS:
		msg = "The dwFlags parameter has an illegal value.";
		break;
	case NTE_BAD_KEYSET:
		msg = "The Registry entry for the key container "
			"could not be opened and may not exist.";
		break;
	case NTE_BAD_KEYSET_PARAM:
		msg = "The pszContainer or pszProvider parameter "
			"is set to an illegal value.";
		break;
	case NTE_BAD_PROV_TYPE:
		msg = "The value of the dwProvType parameter is out "
			"of range. All provider types must be from "
			"1 to 999, inclusive.";
		break;
	case NTE_BAD_SIGNATURE:
		msg = "The provider DLL signature did not verify "
			"correctly. Either the DLL or the digital "
			"signature has been tampered with.";
		break;
	case NTE_EXISTS:
		msg = "The dwFlags parameter is CRYPT_NEWKEYSET, but the key"
		      " container already exists.";
		break;
	case NTE_KEYSET_ENTRY_BAD:
		msg = "The Registry entry for the pszContainer key container "
		      "was found (in the HKEY_CURRENT_USER window), but is "
		      "corrupt. See the section System Administration for "
		      " etails about CryptoAPI's Registry usage.";
		break;
	case NTE_KEYSET_NOT_DEF:
		msg = "No Registry entry exists in the HKEY_CURRENT_USER "
			"window for the key container specified by "
			"pszContainer.";
		break;
	case NTE_NO_MEMORY:
		msg = "The CSP ran out of memory during the operation.";
		break;
	case NTE_PROV_DLL_NOT_FOUND:
		msg = "The provider DLL file does not exist or is not on the "
		      "current path.";
		break;
	case NTE_PROV_TYPE_ENTRY_BAD:
		msg = "The Registry entry for the provider type specified by "
		      "dwProvType is corrupt. This error may relate to "
		      "either the user default CSP list or the machine "
		      "default CSP list. See the section System "
		      "Administration for details about CryptoAPI's "
		      "Registry usage.";
		break;
	case NTE_PROV_TYPE_NO_MATCH:
		msg = "The provider type specified by dwProvType does not "
		      "match the provider type found in the Registry. Note "
		      "that this error can only occur when pszProvider "
		      "specifies an actual CSP name.";
		break;
	case NTE_PROV_TYPE_NOT_DEF:
		msg = "No Registry entry exists for the provider type "
		      "specified by dwProvType.";
		break;
	case NTE_PROVIDER_DLL_FAIL:
		msg = "The provider DLL file could not be loaded, and "
		      "may not exist. If it exists, then the file is "
		      "not a valid DLL.";
		break;
	case NTE_SIGNATURE_FILE_BAD:
		msg = "An error occurred while loading the DLL file image, "
		      "prior to verifying its signature.";
		break;

	default:
		msg = NULL;
		break;
	}
	return msg;
}
