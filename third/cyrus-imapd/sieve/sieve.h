typedef union {
    int nval;
    char *sval;
    stringlist_t *sl;
    test_t *test;
    testlist_t *testl;
    commandlist_t *cl;
    struct vtags *vtag;
    struct aetags *aetag;
    struct htags *htag;
    struct ntags *ntag;
    struct dtags *dtag;
} YYSTYPE;
#define	NUMBER	257
#define	STRING	258
#define	IF	259
#define	ELSIF	260
#define	ELSE	261
#define	REJCT	262
#define	FILEINTO	263
#define	REDIRECT	264
#define	KEEP	265
#define	STOP	266
#define	DISCARD	267
#define	VACATION	268
#define	REQUIRE	269
#define	SETFLAG	270
#define	ADDFLAG	271
#define	REMOVEFLAG	272
#define	MARK	273
#define	UNMARK	274
#define	NOTIFY	275
#define	DENOTIFY	276
#define	ANYOF	277
#define	ALLOF	278
#define	EXISTS	279
#define	SFALSE	280
#define	STRUE	281
#define	HEADER	282
#define	NOT	283
#define	SIZE	284
#define	ADDRESS	285
#define	ENVELOPE	286
#define	COMPARATOR	287
#define	IS	288
#define	CONTAINS	289
#define	MATCHES	290
#define	REGEX	291
#define	COUNT	292
#define	VALUE	293
#define	OVER	294
#define	UNDER	295
#define	ALL	296
#define	LOCALPART	297
#define	DOMAIN	298
#define	USER	299
#define	DETAIL	300
#define	DAYS	301
#define	ADDRESSES	302
#define	SUBJECT	303
#define	MIME	304
#define	METHOD	305
#define	ID	306
#define	OPTIONS	307
#define	LOW	308
#define	NORMAL	309
#define	HIGH	310
#define	MESSAGE	311


extern YYSTYPE yylval;
