/*

  
  					W3C Sample Code Library libwww Request Access Methods


!
  Request Access Methods
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This module keeps a list of valid methods to be performed on a
request object, for example GET, HEAD,
PUT, POST, DELETE, etc.&nbsp;You can assign which method
to be performed on the request object directly or
you can go through the Access module for higher
level functions.

This module is implemented by HTMethod.c, and it
is a part of the  W3C Sample Code
Library.
*/
#ifndef HTMETHOD_H
#define HTMETHOD_H


/*

NOTE: The anchor list of allowed methods is a bitflag, not at list!
*/

typedef enum {
    METHOD_INVALID	= 0x0,
    METHOD_GET		= 0x1,
    METHOD_HEAD		= 0x2,    
    METHOD_POST		= 0x4,    
    METHOD_PUT		= 0x8,
    METHOD_PATCH	= 0x10,
    METHOD_DELETE	= 0x20,
    METHOD_TRACE	= 0x40,
    METHOD_OPTIONS	= 0x80,
    METHOD_LINK		= 0x100,
    METHOD_UNLINK	= 0x200
} HTMethod;

/*
(
  Get Method Enumeration
)

Gives the enumeration value of the method as a function of the (char *) name.
*/

extern HTMethod HTMethod_enum (const char * name);

/*
(
  Get Method String
)

The reverse of HTMethod_enum()
*/

extern const char * HTMethod_name (HTMethod method);

/*
(
  Is Method "Safe"?
)

If a method is safe, it doesn't produce any side effects on the server
*/

#define HTMethod_isSafe(me)	((me) & (METHOD_GET|METHOD_HEAD))

/*
(
  Does the Method Replace or Add to Metainformation
)

Most methods override the current set of metainformation stored in an
anchor. However, a few methods actually only
add to the anchor metainformation. We have a small macro to make the distinction.
*/

#define HTMethod_addMeta(me)  ((me) & (METHOD_TRACE | METHOD_OPTIONS))

/*
(
  Does a Method include Entity?
)

Does a method include an entity to be sent from the client to the server?
*/

#define HTMethod_hasEntity(me)	((me) & (METHOD_PUT | METHOD_POST))

/*
*/

#endif /* HTMETHOD_H */

/*

  

  @(#) $Id: HTMethod.h,v 1.1.1.1 2000-03-10 17:53:00 ghudson Exp $

*/
