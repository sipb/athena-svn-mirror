/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/zephyr.c,v $
 *	$Author: ghudson $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

#ifndef lint
static char rcsid_zephyr_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/zephyr.c,v 1.11 1998-04-17 20:14:32 ghudson Exp $";
#endif

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

static sig_catch zephyr_timeout()
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
    int opretval;
#ifdef POSIX
    struct sigaction newsig, oldsig;
#else
    sig_catch	(*oldsig)();
#endif

    if (setjmp(timeout)) {
	/* We timed out, punt */
	opretval = 1;
	goto cleanup;
    }


#ifdef POSIX
    newsig.sa_handler = zephyr_timeout;
    sigemptyset(&newsig.sa_mask);
    newsig.sa_flags = 0;
    sigaction(SIGALRM, &newsig, &oldsig);
#else
    oldsig = signal(SIGALRM, zephyr_timeout);
#endif
    if (inited < 0) {
	opretval = 1;
	goto cleanup;
    }

    if (!num_subs) {
	/* Can't lose if doing nothing */
	opretval = 0; 
	goto cleanup;
    }

    alarm(ZEPHYR_TIMEOUT);
    if (!inited) {
	if ((retval = ZInitialize()) != ZERR_NONE) {
	    com_err(progname, retval, "while intializing Zephyr library");
	    inited = -1;
	    opretval = 1;
	    goto cleanup;
	}
	if ((wgport = ZGetWGPort()) == -1) {
	    /*
	     * Be quiet about windowgram lossage
	     */
	    inited = -1;
	    opretval = 1;
	    goto cleanup;
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
	    opretval = 1;
	    goto cleanup;
	}
    }
    opretval = 0;
cleanup:
    alarm(0);
#ifdef POSIX
    sigaction(SIGALRM, &oldsig, (struct sigaction *)0);
#else
    (void) signal(SIGALRM, oldsig);
#endif
    return opretval;
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
