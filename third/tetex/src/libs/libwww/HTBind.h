/*

  
  					W3C Sample Code Library libwww File Suffix Binding


!
  File Suffix Binding Manager
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This module sets up the binding between a file suffix and a media type, language,
encoding etc. In a client application the suffixes are used in protocols
that does not directly support media types etc., like FTP, and in server
applications they are used to make the bindings between the server and the
local file store that the server can serve to the rest of the world (well
almost). The HTFormat module holds this information
against the accept headers received in a request and uses if for format
negotiation. All the binding management can all be replace by a database
interface.

This module is implemented by HTBind.c, and it is
a part of the  W3C Sample Code
Library.
*/

#ifndef HTBIND_H
#define HTBIND_H

#include "HTFormat.h"
#include "HTAnchor.h"

/*
.
  Initialization of the Module
.

These functions must be called on startup and termination of the application.
This is done automatically by HTLibInit() and
HTLibTerminate().
*/

extern BOOL HTBind_init		(void);
extern BOOL HTBind_deleteAll	(void);

/*
.
  Case Sensitivity
.

Should the search for suffixes be case sensitive or not? The default value
is case sensitive.
*/

extern void HTBind_caseSensitive	(BOOL sensitive);

/*
.
  Suffix Delimiters
.

Change the set of suffix delimiters. The default is a platform dependent
set defined in the tcp module.
*/

extern const char *HTBind_delimiters	(void);
extern void HTBind_setDelimiters	(const char * new_suffixes);

/*
.
  Set up Bindings Associated with a File Suffix
.

There are four types of bindings:
	 
	   o 
	     Content Type (media type)
	   o 
	     Language
	   o 
	     Content Encoding
	   o 
	     Content Transfer Encoding
	 
	 
And the associated set of methods is defined as:
*/

extern BOOL HTBind_add		(const char *	suffix,
				 const char *	representation,
				 const char *	encoding,
				 const char *	transfer,
				 const char *	language,
				 double		value);

extern BOOL HTBind_addType	(const char *	suffix,
				 const char *	format,
				 double		value);

extern BOOL HTBind_addEncoding	(const char *	suffix,
				 const char *	encoding,
				 double		value);

extern BOOL HTBind_addTransfer	(const char *	suffix,
				 const char *	transfer,
				 double		value);

extern BOOL HTBind_addLanguage	(const char *	suffix,
				 const char *	language,
				 double		value);

/*

The first method is a "super" method for binding information to a file suffic.
Any of the string values can be NULL. If filename suffix is
already defined its previous definition is overridden or modified. For example,
a HTBind_setType and HTBind_setEncoding can be
called with the same suffix.

Calling this with suffix set to "*" will set the default representation.
Calling this with suffix set to "*.*" will set the default representation
for unknown suffix files which contain a "."

NOTE: The suffixes can contain characters that must be escaped
in a URL. However, they should not be encoded when parsed as the
suffix parameter.
.
  Determine a suitable suffix
.

Use the set of bindings to find a suitable suffix (or index) for a certain
combination of language, media type and encoding given in the anchor. Returns
a pointer to a suitable suffix string that must be freed by the caller. If
more than one suffix is found they are all concatenated. If no suffix is
found, NULL is returned.
*/

extern char * HTBind_getSuffix	(HTParentAnchor * anchor);

/*
.
  Determine the content of an Anchor
.

Use the set of bindings to find the combination of language, media type and
encoding of a given anchor. If more than one suffix is found they are all
searched. The last suffix has highest priority, the first one lowest. Returns
the HTAnchor object with the representations
found. See also HTBind_getFormat
*/

extern BOOL HTBind_getAnchorBindings	(HTParentAnchor * anchor);

/*
.
  Determine the content of a Response
.

Use the set of bindings to find the combination of language, media type and
encoding of a given anchor. If more than one suffix is found they are all
searched. The last suffix has highest priority, the first one lowest. Returns
the HTResponse object with the representations
found. See also HTBind_getFormat
*/

extern BOOL HTBind_getResponseBindings	(HTResponse * response,
                                         const char * url);

/*
.
  Determine the content of File
.

Use the set of bindings to find the combination of language, media type and
encoding of a given anchor. If more than one suffix is found they are all
searched. The last suffix has highest priority, the first one lowest. Returns
the format, encoding, and language found. See also
HTBind_getBindings.
*/

extern BOOL HTBind_getFormat (const char *	filename,
			      HTFormat *	format,
			      HTEncoding *	enc,
			      HTEncoding *	cte,
			      HTLanguage *	lang,
			      double *		quality);

/*

End of declaration module
*/

#endif /* HTBIND_H */

/*

  

  @(#) $Id: HTBind.h,v 1.1.1.1 2000-03-10 17:52:55 ghudson Exp $

*/
