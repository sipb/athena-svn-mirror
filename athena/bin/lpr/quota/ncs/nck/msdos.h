#ifndef MSDOS_INCLUDED
#define MSDOS_INCLUDED

#define internal static

/*===========================================================================*/

typedef u_char xlong[8];

/*** Macros to select a sub-field of the 64 bits */
#define XLONG_$LONG(xl, byte_shift) (*((u_long*) &xl[byte_shift]))
#define XLONG_$SHORT(xl, byte_shift) (*((u_short*) &xl[byte_shift]))

extern void xlong_$set_long(xlong xl, u_long l);
extern void xlong_$set_xlong(xlong xl, u_long high, u_long low);

/*** Assembly code routines */
extern void xlong_$add_long(xlong xl, u_long l);
extern void xlong_$mult_long(xlong xl, u_long l);

/*===========================================================================*/

/*
 * Undefined the following constant if you suspect interrupts are causing
 * a problem.  This will make NCK completely synchronous.  The 18 Hz
 * system clock will not be used.  The periodic alarm will be emulated
 * by checking in often-called low-level routines which will call the handler
 * each second (or so).
 * When the interrupt is not used, an MSDOS client may not send request
 * acknowledges at the correct time.  An MSDOS server may not respond
 * to pings, and hence a long call (> 30 seconds) can time out.
 */
#define USE_INTERRUPT     /**/

/*
 * Note the precisions in the calculations below will not overflow
 * a 32 bit number, unless the application is in continuous operation
 * for about 390 days. (TICK_TO_SEC is limiting calculation)
 */
#define TICK_FREQx8      146   /* tick freq for PC is ~18.2 Hz */
#define TICK_PERIODx128    7
#define SEC_TO_TICK(secs)  (((((unsigned long) secs) * TICK_FREQx8)) >> 3)
#define TICK_TO_SEC(ticks) (((((unsigned long) ticks) * TICK_PERIODx128)) >> 7)

extern short NEAR alarm_$interrupt_on;

extern void  alarm_$start(void (*handler)(), int interval);
extern int   alarm_$on(void);
extern void  alarm_$open(char* stack_top, char* stack_bottom);
extern void  alarm_$enable(void);
extern void  alarm_$disable(void);
extern int   alarm_$in_alarm(void);
extern long  time_nck();
extern void  sleep(int secs);

/*** Assembly code routines */
extern long  alarm_$time(void);
extern unsigned short swab_$short(unsigned short);
extern unsigned long  swab_$long(unsigned long);

#define htons(s) swab_$short(s)
#define ntohs(s) swab_$short(s)
#define htonl(l) swab_$long(l)
#define ntohl(l) swab_$long(l)

extern unsigned char socket_$debug;
extern unsigned char socket_$max_debug;

struct farptr {
    unsigned short offset;
    unsigned short segment;
};

#define SEGMENT_OF(ptr) ((struct farptr *) &(ptr))->segment
#define OFFSET_OF(ptr)  ((struct farptr *) &(ptr))->offset

#endif
