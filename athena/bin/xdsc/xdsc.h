#define		MAX_BUTTONS		8
#define		BUFSIZE			512
#define		HELL_FROZEN_OVER	0
#define		LONGNAMELEN		100
#define		SHORTNAMELEN		50

#define		NEXT			1
#define		PREV			2
#define		NEXTNEWS		3
#define		PREVNEWS		4
#define		UPDATE			5
#define		NREF			6
#define		PREF			7
#define		FIRST			8
#define		LAST			9
#define		CURRENT			10
#define		HIGHESTSEEN		11
#define		INITIALIZE		12

#define		BUTTONS_UPDATE		1
#define		BUTTONS_OFF		2
#define		BUTTONS_ON		3

#define		MAIN			1
#define		EDITMTGS		2
#define		LISTTRNS		3

/*
** In a chain of entryrecs, the head will be either a command widget or
** a menubutton, and any following entries will be command children
** of the menubutton.
*/

typedef struct entryrec {
        Widget          button;
        struct entryrec *nextrec;
} EntryRec;

static char rcsidh[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xdsc/xdsc.h,v 1.3 1991-02-05 09:12:50 sao Exp $";
