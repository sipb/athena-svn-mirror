/*

  					W3C Sample Code Library libwww HTML PARSER


!
  Declaration of W3C Sample Code HTML MODULE
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This is the include file for the basic HTML module that can be used together
with libwww. It contains all HTML specific modules which are required to
compile and use the default libwww HTML parser
*/

#ifndef WWWHTML_H
#define WWWHTML_H

/*
*/

#ifdef __cplusplus
extern "C" { 
#endif

/*
.
  System dependencies
.

The wwwsys.h file includes system-specific include
files and flags for I/O to network and disk. The only reason for this file
is that the Internet world is more complicated than Posix and ANSI.
*/

#include "wwwsys.h"

/*
.
  Library Includes
.
*/

#include "HText.h"
#include "HTMLPDTD.h"
#include "SGML.h"
#include "HTMLGen.h"
#include "HTTeXGen.h"

#ifdef HT_EXPAT
#include "HTXML.h"
#endif

/*

End of HTML module
*/

#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif

/*

  

  @(#) $Id: WWWHTML.h,v 1.1.1.1 2000-03-10 17:53:04 ghudson Exp $

*/
