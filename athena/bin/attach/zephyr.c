/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/zephyr.c,v $
 *	$Author: probe $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

#ifndef lint
static char rcsid_zephyr_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/zephyr.c,v 1.7 1991-08-13 21:07:05 probe Exp $";
#endif lint

#include "attach.h"
#ifdef ZEPHYR
#include <zephyr/zephyr.h>
#include <setjmp.h>
#include <signal.h>

#ifdef __HIGHC__
#define	min(x,y)	_min(x,y)
#else
#define	min(x,y)	((x)<(y)?(x):(y))
#endif

#define ZEPHYR_MAXONEPACKET 5
Code_t ZSubscribeTo(), ZUnsubscribeTo();

static ZSubscription_t subs[ZEPHYR_MAXSUBS];
static int num_subs = 0;

static jmp_buf	timeout;

static int zephyr_timeout()
{
	longjmp(timeout, 1);
}

static int zephyr_op(func)
    Code_t (*func)();
{
    static int inited = 0;
    static int wgport = 0;
    int count, count2;
    ZSubscription_t shortsubs[ZEPHYR_MAXONEPACKET];
    Code_t retval;
    sig_catch	(*old_sig_func)();

    if (setjmp(timeout))
	    return 1;		/* We timed out, punt */

    old_sig_func = signal(SIGALRM, zephyr_timeout);
    alarm(ZEPHYR_TIMEOUT);
    
    if (inited < 0)
	return 1;

    if (!num_subs)
	return 0;	/* Can't lose if doing nothing */

    if (!inited) {
	if ((retval = ZInitialize()) != ZERR_NONE) {
	    com_err(progname, retval, "while intializing Zephyr library");
	    inited = -1;
	    return 1;
	}
	if ((wgport = ZGetWGPort()) == -1) {
	    /*
	     * Be quiet about windowgram lossage
	     */
	    inited = -1;
	    return 1;
	}
	inited = 1;
    }

    for (count=0; count<num_subs; count += ZEPHYR_MAXONEPACKET) {
	for (count2=0; count2<min(ZEPHYR_MAXONEPACKET,num_subs-count); count2++)
	    shortsubs[count2] = subs[count+count2];
	if ((retval = (func)(shortsubs,
			     min(ZEPHYR_MAXONEPACKET,num_subs-count),
			     wgport)) != ZERR_NONE) {
	    fprintf(stderr, "Error while subscribing: %s\n",
		    error_message(retval));
	    inited = -1;
	    return 1;
	}
    }
    alarm(0);
    (void) signal(SIGALRM, old_sig_func);
    return 0;
}

void zephyr_addsub(class)
    const char *class;
{
    if (num_subs == ZEPHYR_MAXSUBS)
	return;
    
    if (debug_flag)
	    printf("Subscribing to zephyr instance %s.\n", class);
    subs[num_subs].zsub_recipient = "*";
    subs[num_subs].zsub_classinst = strdup(class);
    subs[num_subs].zsub_class = ZEPHYR_CLASS;
    num_subs++;
}

int zephyr_sub(iszinit)
int iszinit;
{
    if(zephyr_op(ZSubscribeTo) && iszinit)
      {
	error_status = ERR_ZINITZLOSING;
	return FAILURE;
      }
    return SUCCESS;
}

int zephyr_unsub(iszinit)
int iszinit;
{
    if(zephyr_op(ZUnsubscribeTo) && iszinit)
      {
	error_status = ERR_ZINITZLOSING;
	return FAILURE;
      }
    return SUCCESS;
}
#endif
