/*

  
  					W3C Sample Code Library libwww HTML DTD


!
  HTML Plus DTD - Software Interface
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

SGML purists should excuse the use of the term "DTD" in this file to represent
DTD-related information which is not exactly a DTD itself. The C modular
structure doesn't work very well here, as the dtd is partly in the .h and
partly in the .c which are not very independent. Tant pis! There
are a couple of HTML-specific utility routines also defined.

This module is a part of the  W3C Sample
Code Library.
*/

#ifndef HTMLDTD_H
#define HTMLDTD_H

#include "HTStruct.h"
#include "SGML.h"

/*
.
  Number of HTML Entities
.

The entity names are defined in the C file. This gives the number of them.
Must Match all tables by element!
*/

#define HTML_ENTITIES 67

/*
.
  HTML Element Enumeration
.

These include tables in HTMLPDTD.c and
code in HTML.c. Note that not everything from
HTML 4.0 is there!
*/

typedef enum _HTMLElement {
	HTML_A = 0,
	HTML_ABBREV,
	HTML_ABSTRACT,
	HTML_ACRONYM,
	HTML_ADDED,
	HTML_ADDRESS,
	HTML_AREA,
	HTML_ARG,
	HTML_B,
	HTML_BASE,
	HTML_BLOCKQUOTE,
	HTML_BODY,
	HTML_BOX,
	HTML_BR,
	HTML_BYLINE,
	HTML_CAPTION,
	HTML_CHANGED,
	HTML_CITE,
	HTML_CMD,
	HTML_CODE,
	HTML_COMMENT,
	HTML_DD,
 	HTML_DFN,
	HTML_DIR,
 	HTML_DL,
 	HTML_DT,
	HTML_EM,
	HTML_FIG,
	HTML_FOOTNOTE,
	HTML_FORM,
	HTML_FRAME,
	HTML_FRAMESET,
	HTML_H1,
 	HTML_H2,
 	HTML_H3,
	HTML_H4,
	HTML_H5,
 	HTML_H6, 
	HTML_H7,
	HTML_HEAD,
 	HTML_HR,
	HTML_HTML,
	HTML_HTMLPLUS,
	HTML_I,
	HTML_IMAGE,
	HTML_IMG,
	HTML_INPUT,
	HTML_ISINDEX,
 	HTML_KBD,	
	HTML_L,
 	HTML_LI,
 	HTML_LINK,
	HTML_LISTING,
	HTML_LIT,
	HTML_MARGIN,
	HTML_MATH,
	HTML_MENU,
	HTML_META,
	HTML_NEXTID,
	HTML_NOFRAMES,
	HTML_NOTE,
	HTML_OBJECT,
	HTML_OL,
	HTML_OPTION,
	HTML_OVER,
	HTML_P,
	HTML_PERSON,
	HTML_PLAINTEXT,
	HTML_PRE,
	HTML_Q,
	HTML_QUOTE,
	HTML_RENDER,
	HTML_REMOVED,
	HTML_S,
	HTML_SAMP,
	HTML_SELECT,
	HTML_STRONG,
	HTML_SUB,
	HTML_SUP,
	HTML_TAB,
	HTML_TABLE,
	HTML_TD,
	HTML_TEXTAREA,
	HTML_TH,
	HTML_TITLE,
	HTML_TR,
	HTML_TT,
	HTML_U,
	HTML_UL,
	HTML_VAR,
	HTML_XMP,
	HTML_ELEMENTS		/* This must be the last entry */
} HTMLElement;

/*
.
  Element Attribute Enumerations
.

Identifier is HTML_<element>_<attribute>. These
must match the tables in
HTMLPDTD.c!
(
  A
)
*/

#define HTML_A_EFFECT 		0
#define HTML_A_HREF		1
#define HTML_A_ID		2
#define HTML_A_METHODS		3
#define HTML_A_NAME 		4
#define HTML_A_PRINT		5
#define HTML_A_REL		6
#define HTML_A_REV		7
#define HTML_A_SHAPE		8	
#define HTML_A_TITLE		9
#define HTML_A_ATTRIBUTES	10

/*
(
  AREA
)
*/

#define HTML_AREA_ALT 		0
#define HTML_AREA_ACCESSKEY	1
#define HTML_AREA_COORDS	2
#define HTML_AREA_HREF		3
#define HTML_AREA_NOHREF	4
#define HTML_AREA_ONBLUR	5
#define HTML_AREA_ONFOCUS	6
#define HTML_AREA_SHAPE		7 
#define HTML_AREA_TABINDEX	8
#define HTML_AREA_ATTRIBUTES	9

/*
(
  BASE
)
*/

#define HTML_BASE_HREF		0
#define HTML_BASE_ATTRIBUTES	1

/*
(
  BODY
)
*/

#define HTML_BODY_BACKGROUND    0
#define HTML_BODY_ATTRIBUTES    1

/*
(
  FORM
)
*/

#define HTML_FORM_ACTION	0	/* WSM bug fix, added these five */
#define HTML_FORM_ID		1
#define HTML_FORM_INDEX		2
#define HTML_FORM_LANG		3
#define HTML_FORM_METHOD	4
#define HTML_FORM_ATTRIBUTES	5

/*
(
  FRAME
)
*/

#define HTML_FRAME_SRC          0
#define HTML_FRAME_ATTRIBUTES   1

/*
(
  FRAMESET
)
*/

#define HTML_FRAMESET_COLS              0
#define HTML_FRAMESET_ROWS              1
#define HTML_FRAMESET_BORDER            2
#define HTML_FRAMESET_BORDERCOLOR       3
#define HTML_FRAMESET_FRAMEBORDER       4
#define HTML_FRAMESET_ONBLUR            5
#define HTML_FRAMESET_ONFOCUS           6
#define HTML_FRAMESET_ONLOAD            7
#define HTML_FRAMESET_ONUNLOAD          8
#define HTML_FRAMESET_ATTRIBUTES        9

/*
(
  FIG
)
*/

#define HTML_FIG_ATTRIBUTES	6

/*
(
  GEN
)
*/

#define HTML_GEN_ATTRIBUTES	3

/*
(
  HTMLPLUS
)
*/

#define HTML_HTMLPLUS_ATTRIBUTES	2

/*
(
  IMAGE
)
*/

#define HTML_IMAGE_ATTRIBUTES	5

/*
(
  CHANGED
)
*/

#define HTML_CHANGED_ATTRIBUTES	2

/*
(
  DL
)
*/

#define HTML_DL_ID	 	0
#define HTML_DL_COMPACT 	1
#define HTML_DL_INDEX	 	2
#define HTML_DL_ATTRIBUTES	3

/*
(
  IMG
)
*/

#define HTML_IMG_ALIGN		0
#define HTML_IMG_ALT		1
#define HTML_IMG_ISMAP		2	/* Obsolete but supported */
#define HTML_IMG_LOWSRC	        3
#define HTML_IMG_SEETHRU	4
#define HTML_IMG_SRC 		5
#define HTML_IMG_ATTRIBUTES	6

/*
(
  INPUT
)
*/

#define HTML_INPUT_ALIGN	0
#define HTML_INPUT_CHECKED	1
#define HTML_INPUT_DISABLED	2
#define HTML_INPUT_ERROR	3
#define HTML_INPUT_MAX		4
#define HTML_INPUT_MIN		5
#define HTML_INPUT_NAME		6
#define HTML_INPUT_SIZE		7
#define HTML_INPUT_SRC		8
#define HTML_INPUT_TYPE		9
#define HTML_INPUT_VALUE	10
#define HTML_INPUT_ATTRIBUTES	11

/*
(
  L
)
*/

#define HTML_L_ATTRIBUTES	4

/*
(
  LI
)
*/

#define HTML_LI_ATTRIBUTES	4

/*
(
  LIST
)
*/

#define HTML_LIST_ATTRIBUTES	4

/*
(
  LINK
)
*/

#define HTML_LINK_CHARSET	0
#define HTML_LINK_HREF		1
#define HTML_LINK_HREFLANG	2
#define HTML_LINK_MEDIA		3
#define HTML_LINK_REL		4
#define HTML_LINK_REV		5
#define HTML_LINK_TYPE 		6
#define HTML_LINK_ATTRIBUTES	7

/*
(
  ID
)
*/

#define HTML_ID_ATTRIBUTE	1

/*
(
  META
)
*/

#define HTML_META_CONTENT	0
#define HTML_META_HTTP_EQUIV	1
#define HTML_META_NAME   	2
#define HTML_META_SCHEME	3
#define HTML_META_ATTRIBUTES	4

/*
(
  NEXTID
)
*/

#define HTML_NEXTID_ATTRIBUTES  1
#define HTML_NEXTID_N 0

/*
(
  NOTE
)
*/

#define HTML_NOTE_ATTRIBUTES	4

/*
(
  OBJECT
)
*/

#define HTML_OBJECT_ARCHIVE     0
#define HTML_OBJECT_CLASSID	1
#define HTML_OBJECT_CODEBASE	2
#define HTML_OBJECT_CODETYPE	3
#define HTML_OBJECT_DATA	4
#define HTML_OBJECT_DECLARE     5
#define HTML_OBJECT_HIGHT       6
#define HTML_OBJECT_NAME        7
#define HTML_OBJECT_STANDBY	8
#define HTML_OBJECT_TABINDEX	9
#define HTML_OBJECT_TYPE        10
#define HTML_OBJECT_USEMAP	11
#define HTML_OBJECT_WIDTH       12
#define HTML_OBJECT_ATTRIBUTES  13

/*
(
  OPTION
)
*/

#define HTML_OPTION_DISABLED	0	/* WSM bug fix, added these 4 */
#define HTML_OPTION_LANG	1
#define HTML_OPTION_SELECTED	2
#define HTML_OPTION_ATTRIBUTES  3

/*
(
  RENDER
)
*/

#define HTML_RENDER_ATTRIBUTES 	2

/*
(
  SELECT
)
*/

#define HTML_SELECT_ERROR	0	/* WSM bug fix, added these 5 */
#define HTML_SELECT_LANG	1
#define HTML_SELECT_MULTIPLE	2
#define HTML_SELECT_NAME	3
#define HTML_SELECT_SIZE	4
#define HTML_SELECT_ATTRIBUTES  5

/*
(
  TAB
)
*/

#define HTML_TAB_ATTRIBUTES	2

/*
(
  TABLE
)
*/

#define HTML_TABLE_ATTRIBUTES	4

/*
(
  TD
)
*/

#define HTML_TD_ATTRIBUTES	4

/*
(
  TEXTAREA
)
*/

#define HTML_TEXTAREA_COLS		0
#define HTML_TEXTAREA_DISABLED		1
#define HTML_TEXTAREA_ERROR		2
#define HTML_TEXTAREA_LANG		3
#define HTML_TEXTAREA_NAME		4
#define HTML_TEXTAREA_ROWS		5
#define HTML_TEXTAREA_ATTRIBUTES	6

/*
(
  TH
)
*/

#define HTML_TH_ATTRIBUTES	4

/*
(
  UL
)
*/

#define HTML_UL_ATTRIBUTES	6

/*
.
  The C Representation of the SGML DTD
.
*/

extern SGML_dtd * HTML_dtd (void);
extern BOOL HTML_setDtd (const SGML_dtd * dtd);

/*
.
  Utitity Functions
.
(
  Start anchor element
)

It is kinda convenient to have a particular routine for starting an anchor
element, as everything else for HTML is simple anyway.
*/

extern void HTStartAnchor (
		HTStructured * targetstream,
		const char *  	name,
		const char *  	href);

/*
(
  Put image element
)

This is the same idea but for images
*/

extern void HTMLPutImg (HTStructured *obj,
		 	       const char *src,
			       const char *alt,
			       const char *align);


/*
(
  Specify next ID to be used
)

This is another convenience routine, for specifying the next ID to be used
by an editor in the series z1. z2,...
*/

extern void HTNextID (HTStructured * targetStream, const char * s);

/*
*/

#endif /* HTMLDTD_H */

/*

  

  @(#) $Id: HTMLPDTD.h,v 1.1.1.1 2000-03-10 17:53:00 ghudson Exp $

*/
