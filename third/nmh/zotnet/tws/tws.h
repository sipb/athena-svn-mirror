
/*
 * tws.h
 *
 * $Id: tws.h,v 1.1.1.1 1999-02-07 18:14:11 danw Exp $
 */

/* DST vs. GMT nonsense */
#define	DSTXXX

struct tws {
    int tw_sec;		/* seconds after the minute - [0, 61] */
    int tw_min;		/* minutes after the hour - [0, 59]   */
    int tw_hour;	/* hour since midnight - [0, 23]      */
    int tw_mday;	/* day of the month - [1, 31]         */
    int tw_mon;		/* months since January - [0, 11]     */
    int tw_year;	/* 4 digit year (ie, 1997)            */
    int tw_wday;	/* days since Sunday - [0, 6]         */
    int tw_yday;	/* days since January 1 - [0, 365]    */
    int tw_zone;
    time_t tw_clock;	/* if != 0, corresponding calendar value */
    int tw_flags;
};

#define	TW_NULL	 0x0000

#define	TW_SDAY	 0x0003	/* how day-of-week was determined */
#define	TW_SNIL	 0x0000	/*   not given                    */
#define	TW_SEXP	 0x0001	/*   explicitly given             */
#define	TW_SIMP	 0x0002	/*   implicitly given             */

#define	TW_SZONE 0x0004	/* how timezone was determined    */
#define	TW_SZNIL 0x0000	/*   not given                    */
#define	TW_SZEXP 0x0004	/*   explicitly given             */

#define	TW_DST	 0x0010	/* daylight savings time          */
#define	TW_ZONE	 0x0020	/* use numeric timezones only     */

#define	dtwszone(tw) dtimezone (tw->tw_zone, tw->tw_flags)

extern char *tw_dotw[];
extern char *tw_ldotw[];
extern char *tw_moty[];

/*
 * prototypes
 */
char *dtime (time_t *, int);
char *dtimenow (int);
char *dctime (struct tws *);
struct tws *dlocaltimenow (void);
struct tws *dlocaltime (time_t *);
struct tws *dgmtime (time_t *);
char *dasctime (struct tws *, int);
char *dtimezone (int, int);
void twscopy (struct tws *, struct tws *);
int twsort (struct tws *, struct tws *);
time_t dmktime (struct tws *);
void set_dotw (struct tws *);

struct tws *dparsetime (char *);

