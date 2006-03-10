/* $Id: libjwgc_types.h,v 1.1.1.1 2006-03-10 15:32:57 ghudson Exp $ */

#ifndef _LIBJWGC_TYPES_H_
#define _LIBJWGC_TYPES_H_ 1

#include "libxode.h"


/* --------------------------------------------------------- */
/* JContact.c                                                */
/* Contact management                                        */
/* --------------------------------------------------------- */
#define AGENT_NONE      0x0000
#define AGENT_SEARCH    0x0001
#define AGENT_REGISTER  0x0010
#define AGENT_GROUPCHAT 0x0100
#define AGENT_TRANSPORT 0x1000



/* --------------------------------------------------------- */
/* JForm.c                                                   */
/* Form handling routines                                    */
/* --------------------------------------------------------- */
#define JFORM_INSTRUCTIONS		"instructions"
#define JFORM_TITLE			"title"
#define JFORM_FIELD			"field"

#define JFORM_FIELD_TEXT_SINGLE		"text-single"
#define JFORM_FIELD_TEXT_PRIVATE	"text-private"
#define JFORM_FIELD_TEXT_MULTI		"text-multi"
#define JFORM_FIELD_LIST_SINGLE		"list-single"
#define JFORM_FIELD_LIST_MULTI		"list-multi"
#define JFORM_FIELD_BOOLEAN		"boolean"
#define JFORM_FIELD_FIXED		"fixed"
#define JFORM_FIELD_HIDDEN		"hidden"
#define JFORM_FIELD_JID_SINGLE		"jid-single"
#define JFORM_FIELD_JID_MULTI		"jid-multi"



/* --------------------------------------------------------- */
/* JHashTable.c                                              */
/* Hash table functions                                      */
/* --------------------------------------------------------- */
#ifdef XML_UNICODE
#ifdef XML_UNICODE_WCHAR_T
typedef const wchar_t *KEY;
#else /* not XML_UNICODE_WCHAR_T */
typedef const unsigned short *KEY;
#endif /* not XML_UNICODE_WCHAR_T */
#else /* not XML_UNICODE */
typedef const char *KEY;
#endif /* not XML_UNICODE */

typedef struct {
	KEY			name;
} NAMED;

typedef struct {
	NAMED			**v;
	size_t			size;
	size_t			used;
	size_t			usedLim;
} HASH_TABLE;

typedef struct {
	NAMED			**p;
	NAMED			**end;
} HASH_TABLE_ITER;



/* --------------------------------------------------------- */
/* JNetSock.c                                                */
/* Network socket routines                                   */
/* --------------------------------------------------------- */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN		64
#endif

#define NETSOCKET_SERVER	0
#define NETSOCKET_CLIENT	1
#define NETSOCKET_UDP		2



/* --------------------------------------------------------- */
/* JSha.c                                                    */
/* SHA calculations                                          */
/* --------------------------------------------------------- */
typedef struct {
	unsigned long		H[5];
	unsigned long		W[80];
	int			lenW;
	unsigned long		sizeHi;
	unsigned long		sizeLo;
} j_SHA_CTX;



/* --------------------------------------------------------- */
/* JVariables.c                                              */
/* Variable management routines                              */
/* --------------------------------------------------------- */
#ifndef DEFVARS
#define DEFVARS "jwgc.vars"
#endif

#ifndef USRVARS
#define USRVARS ".jwgc.vars"
#endif

#ifndef FIXEDVARS
#define FIXEDVARS "jwgc.vars.fixed"
#endif

typedef enum jVar_name {
	jVarUsername,
	jVarPassword,
	jVarServer,
	jVarResource,
	jVarPort,
	jVarPriority,
#ifdef USE_SSL
	jVarUseSSL,
#endif /* USE_SSL */
	jVarInitProgs,
	jVarJID,
	jVarPresence,
#ifdef USE_GPGME
	jVarUseGPG,
	jVarGPGPass,
	jVarGPGKeyID,
#endif /* USE_GPGME */
	jNumVars
} jVar;

typedef enum jVar_type {
	jTypeString,
	jTypeNumber,
	jTypeBool,
	jNumTypes
} jVarType;



/* --------------------------------------------------------- */
/* JXMLComm.c                                                */
/* XML communication routines                                */
/* --------------------------------------------------------- */
typedef struct jwgpacket_struct {
	int			type;
	xode			x;
	xode_pool		p;
} *jwgpacket, _jwgpacket;

#define JWGPACKET_UNKNOWN	0
#define JWGPACKET_MESSAGE	1
#define JWGPACKET_LOCATE	2
#define JWGPACKET_STATUS	3
#define JWGPACKET_SHUTDOWN	4
#define JWGPACKET_CHECK		5
#define JWGPACKET_REREAD	6
#define JWGPACKET_SHOWVAR	7
#define JWGPACKET_SUBSCRIBE	8
#define JWGPACKET_UNSUBSCRIBE	9
#define JWGPACKET_NICKNAME	10
#define JWGPACKET_GROUP		11
#define JWGPACKET_REGISTER	12
#define JWGPACKET_SETVAR	13
#define JWGPACKET_JOIN		14
#define JWGPACKET_LEAVE		15
#define JWGPACKET_DEBUG		16
#define JWGPACKET_SEARCH	17
#define JWGPACKET_PING		18

#define JWGCONN_STATE_OFF       0
#define JWGCONN_STATE_CONNECTED 1
#define JWGCONN_STATE_ON        2
#define JWGCONN_STATE_AUTH      3

typedef struct jwgconn_struct {
	/* Core structure */
	xode_pool		p;		/* Memory allocation pool */
	int			state;		/* Connection state flag */
	int			fd;		/* Connection file descriptor */
	int			sckfd;		/* Socket file descriptor */

	/* Stream stuff */
	XML_Parser		parser;		/* Parser instance */
	xode			current;	/* Current parsing node */

	/* Event callback ptrs */
	void (*on_packet)(struct jwgconn_struct *j, jwgpacket p);
} *jwgconn, jwgconn_struct;

typedef void (*jwgconn_packet_h)(jwgconn j, jwgpacket p);


#endif /* _LIBJWGC_H_ */
