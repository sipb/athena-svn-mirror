/*

  
  					W3C Sample Code Library libwww MIME/RFC822 Parsers


!
  MIME Parsers
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

The MIME parser stream presents a MIME document with a header and possibly
a footer. It recursively invokes the format manager to handle embedded formats
like MIME multipart. As well as stripping off and parsing the headers, the
MIME parser has to parse any weird MIME encodings it may meet within the
body parts of messages, and must deal with multipart messages.

This module is implemented to the level necessary for operation with WWW,
but is not currently complete for any arbitrary MIME message.

This module is implemented by HTMIME.c, and it is
a part of the  W3C Sample Code
Library.
*/

#ifndef HTMIME_H
#define HTMIME_H

#include "HTStream.h"
#include "HTFormat.h"

/*
(
  MIME header parser stream
)

This stream parses a complete MIME header and if a content type header is
found then the stream stack is called. Any left over data is pumped right
through the stream.
*/

extern HTConverter HTMIMEConvert;

/*
(
  MIME Header ONLY parser stream
)

This stream parses a complete MIME header and then returnes HT_PAUSE. It
does not set up any streams and resting data stays in the buffer. This can
be used if you only want to parse the headers before you decide what to do
next. This is for example the case in a server app.
*/

extern HTConverter HTMIMEHeader;

/*
(
  MIME Footer ONLY parser
)

Parse only a footer, for example after a chunked encoding.
*/

extern HTConverter HTMIMEFooter;

/*
(
  MIME 1xx Continue Header Parser
)

When parsed the header it returns HT_CONTINUE
*/

extern HTConverter HTMIMEContinue;

/*
(
  Partial MIME Response parser
)

In case we sent a Range conditional GET we may get back a partial
response. This response must be appended to the already existing cache entry
before presented to the user. That is, first we load the cached object and
pump it down the pipe and then the new data follows. Only the latter part
gets appended to the cache, of course.
*/

extern HTConverter HTMIMEPartial;

/*
*/

#endif

/*

  

  @(#) $Id: HTMIME.h,v 1.1.1.1 2000-03-10 17:52:59 ghudson Exp $

*/
