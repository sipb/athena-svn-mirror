%{
/*
 * addr.y -- RFC 822 address parser
 * Ken Murchison
 * $Id: addr.y,v 1.1.1.1 2002-10-13 18:00:28 ghudson Exp $
 */
/***********************************************************
        Copyright 1999 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Carnegie Mellon
University not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
******************************************************************/

#include <stdlib.h>
#include <string.h>

#include "addr.h"

int yyerror(char *msg);
extern int yylex(void);

#define YYERROR_VERBOSE /* i want better error messages! */
%}

%token ATOM QTEXT DTEXT

%start sieve_address

%%
address: mailbox			/* one addressee */
	| group				/* named list */
	;

group: phrase ':' ';'
	| phrase ':' mailboxes ';'
	;

mailboxes: mailbox
	| mailbox ',' mailboxes
	;

mailbox: addrspec			/* simple address */
	| phrase routeaddr		/* name & addr-spec */
	;

routeaddr: '<' addrspec '>'
	| '<' route ':' addrspec '>'
	;

route: '@' domain			/* path-relative */
	| '@' domain ',' route
	;

sieve_address: addrspec			/* simple address */
	| phrase '<' addrspec '>'	/* name & addr-spec */
	;

addrspec: localpart '@' domain		/* global-address */
	;

localpart: word				/* uninterpreted, case-preserved */
	| word '.' localpart
	;

domain: subdomain
	| subdomain '.' domain
	;

subdomain: domainref
	| domainlit
	;

domainref: ATOM				/* symbolic reference */
	;

domainlit: '[' DTEXT ']'
	;

phrase: word
	| word phrase
	;

word: ATOM
	| qstring
	;

qstring: '"' QTEXT '"'
	;

%%

/* copy address error message into buffer provided by sieve parser */
int yyerror(char *s)
{
extern char addrerr[];

    strcpy(addrerr, s);
    return 0;
}
