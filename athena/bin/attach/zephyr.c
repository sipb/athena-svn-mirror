/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/zephyr.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

#ifndef lint
static char rcsid_zephyr_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/zephyr.c,v 1.1 1990-04-19 12:47:32 jfc Exp $";
#endif lint

#include "attach.h"
#ifdef ZEPHYR
#include <zephyr/zephyr.h>
#include <setjmp.h>
#include <signal.h>

#define min(x,y) ((x)<(y)?(x):(y))

#define ZEPHYR_MAXONEPACKET 5
Code_t ZSubscribeTo(), ZUnsubscribeTo();

static ZSubscription_t subs[ZEPHYR_MAXSUBS];
static int num_subs = 0;

zephyr_addsub(class)
    char *class;
{
    if (num_subs == ZEPHYR_MAXSUBS)
	return;
    
    if (debug_flag)
	    printf("Subscribing to zepyr instance %s.\n", class);
    subs[num_subs].recipient = "*";
    subs[num_subs].classinst = strdup(class);
    subs[num_subs].class = ZEPHYR_CLASS;
    num_subs++;
}

zephyr_sub()
{
    zephyr_op(ZSubscribeTo);
}

zephyr_unsub()
{
    zephyr_op(ZUnsubscribeTo);
}

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
	    return;		/* We timed out, punt */

    old_sig_func = signal(SIGALRM, zephyr_timeout);
    alarm(ZEPHYR_TIMEOUT);
    
    if (inited < 0)
	return;

    if (!num_subs)
	return;

    if (!inited) {
	if ((retval = ZInitialize()) != ZERR_NONE) {
	    fprintf(stderr, "Can't intialize Zephyr library!\n");
	    inited = -1;
	    return;
	}
	if ((wgport = ZGetWGPort()) == -1) {
	    /*
	     * Be quiet about windowgram lossage
	     */
	    inited = -1;
	    return;
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
	    return;
	}
    }
    alarm(0);
    (void) signal(SIGALRM, old_sig_func);
}
#endif
