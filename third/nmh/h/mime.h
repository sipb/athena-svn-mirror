
/*
 * mime.h -- definitions for MIME
 *
 * $Id: mime.h,v 1.1.1.1 1999-02-07 18:14:06 danw Exp $
 */

#define	VRSN_FIELD	"MIME-Version"
#define	VRSN_VALUE	"1.0"
#define	XXX_FIELD_PRF	"Content-"
#define	TYPE_FIELD	"Content-Type"
#define	ENCODING_FIELD	"Content-Transfer-Encoding"
#define	ID_FIELD	"Content-ID"
#define	DESCR_FIELD	"Content-Description"
#define	MD5_FIELD	"Content-MD5"

#define	isatom(c)   (!isspace (c) && !iscntrl (c) && (c) != '(' \
	             && (c) != ')' && (c) != '<'  && (c) != '>' \
	             && (c) != '@' && (c) != ','  && (c) != ';' \
	             && (c) != ':' && (c) != '\\' && (c) != '"' \
	             && (c) != '.' && (c) != '['  && (c) != ']')

/*
 * Test for valid characters used in "token"
 * as defined in RFC2045
 */
#define	istoken(c)  (!isspace (c) && !iscntrl (c) && (c) != '(' \
	             && (c) != ')' && (c) != '<'  && (c) != '>' \
	             && (c) != '@' && (c) != ','  && (c) != ';' \
	             && (c) != ':' && (c) != '\\' && (c) != '"' \
	             && (c) != '/' && (c) != '['  && (c) != ']' \
	             && (c) != '?' && (c) != '=')

#define	CPERLIN	76
#define	BPERLIN	(CPERLIN / 4)
#define	LPERMSG	632
#define	CPERMSG	(LPERMSG * CPERLIN)
