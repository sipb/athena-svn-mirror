/*
 * ==========================================================================
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 *
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ==========================================================================
 *
 * P F M
 *
 * Process fault manager for Unix.
 *
 * This module defines pieces of the Apollo PFM for non-Apollo systems.
 * Basically, we want the cleanup/signal (unwind-protect/catch/throw)
 * mechanism.
 */

#include "std.h"

#include "nbase.h"
#include "fault.h"

#include "pfm.h"

int pfm_$fault_inh_count = 0;                   /* Number of time pfm_$inhibit called */

static pfm_$cleanup_rec *pfm_$crec_list = 0;    /* Head of list of cleanup records */
static u_long pfm_$saved_mask = 0;              /* Signal mask saved by inhibit */

#if defined(SYS5_PRE_R3) | defined(MSDOS)
static (*pfm_$old_handlers[NSIG])();            /* Saved signal handlers */
#define sighold(s) pfm_$old_handlers[s] = signal((s), SIG_IGN)
#define sigrelse(s) signal((s), pfm_$old_handlers[s]);
#endif

#ifdef MSDOS
extern void alarm_$enable();
extern void alarm_$disable();
#endif

static int signal_handler_set = 0;

static signal_handler(sig)
int sig;
{
    status_$t st;

    switch (sig) {
        case SIGINT:
        case SIGTERM:
            st.all = fault_$interrupt;
            break;
        case SIGILL:
            st.all = fault_$illegal_inst;
            break;
        case SIGFPE:
            st.all = fault_$fp_exception;
            break;
#ifndef MSDOS
        case SIGHUP:
            st.all = fault_$interrupt;
            break;
        case SIGQUIT:
            st.all = fault_$quit;
            break;
        case SIGTRAP:
            st.all = fault_$trapv_inst;
            break;
        case SIGBUS:
        case SIGSEGV:
            st.all = fault_$address_error;
            break;
        case SIGSYS:
            st.all = fault_$illegal_svc_code;
            break;
#else
        case SIGABRT:
            st.all = fault_$trapv_inst;
            break;
#endif
    }

#if defined(SYS5) || defined(MSDOS)
    signal(sig, signal_handler);
#endif

    pfm_$signal(st);
}

static void remove_signal_handler()
{
    if (! signal_handler_set)
        return;

    signal(SIGINT,    SIG_DFL);
    signal(SIGILL,    SIG_DFL);
    signal(SIGFPE,    SIG_DFL);
    signal(SIGTERM,   SIG_DFL);
#ifndef MSDOS
    signal(SIGHUP,    SIG_DFL);
    signal(SIGQUIT,   SIG_DFL);
    signal(SIGTRAP,   SIG_DFL);
    signal(SIGBUS,    SIG_DFL);
    signal(SIGSEGV,   SIG_DFL);
    signal(SIGSYS,    SIG_DFL);
#endif
}

static void install_signal_handler()
{
    signal(SIGINT,    signal_handler);
    signal(SIGILL,    signal_handler);
    signal(SIGFPE,    signal_handler);
    signal(SIGTERM,   signal_handler);
#ifndef MSDOS
    signal(SIGHUP,    signal_handler);
    signal(SIGQUIT,   signal_handler);
    signal(SIGTRAP,   signal_handler);
    signal(SIGBUS,    signal_handler);
    signal(SIGSEGV,   signal_handler);
    signal(SIGSYS,    signal_handler);
#endif
    signal_handler_set = 1;
}


/*
 * P F M _ $ I N I T
 *
 * Initialize PFM package.
 */

void pfm_$init(flags)
u_long flags;
{
    if (flags & pfm_$init_signal_handlers)
        install_signal_handler();
}


/*
 * P F M _ $ _ C L E A N U P
 *
 * Internal routine called by "pfm_$cleanup" macro.  Add a new cleanup
 * handler to the list of cleanup handlers.
 */

status_$t pfm_$_cleanup(st, crec)
long st;
pfm_$cleanup_rec *crec;
{
    status_$t sjv;

    sjv.all = st;

    if (sjv.all == 0) {
        crec->next = pfm_$crec_list;
        crec->fault_inh_count = pfm_$fault_inh_count;
        pfm_$crec_list = crec;
        sjv.all = pfm_$cleanup_set;
    }
    else if (sjv.all == pfm_$signalled_zero)
        sjv.all = 0;

    return (sjv);
}


/*
 * P F M _ $ _ R L S _ C L E A N U P
 *
 * Remote a cleanup handler from the list of cleanup handlers.
 */

void pfm_$_rls_cleanup(crec, st)
pfm_$cleanup_rec *crec;
status_$t *st;
{
    if (crec != pfm_$crec_list) {
        st->all = pfm_$bad_rls_order;
        return;
    }

    pfm_$crec_list = pfm_$crec_list->next;
    st->all = status_$ok;
}


/*
 * P F M _ $ _ R E S E T _ C L E A N U P
 *
 * Called from within a cleanup handler, this call causes
 * a previously established cleanup handler to be re-established.
 */

void pfm_$_reset_cleanup(crec, st)
pfm_$cleanup_rec *crec;
status_$t *st;
{
    crec->next = pfm_$crec_list;
    pfm_$crec_list = crec;
    st->all = status_$ok;
}


/*
 * I N H I B I T _ F A U L T S
 *
 * Internal routine to turn off faults.
 */

static void inhibit_faults()
{
#ifdef BSD
    pfm_$saved_mask = sigsetmask(sigmask(SIGHUP) | sigmask(SIGINT) | sigmask(SIGQUIT) |
                                 sigmask(SIGTERM) | sigmask(SIGCLD));
#endif

#if defined(SYS5) || defined(MSDOS)
    sighold(SIGINT);
    sighold(SIGTERM);
#ifndef MSDOS
    sighold(SIGHUP);
    sighold(SIGQUIT);
    sighold(SIGCLD);
#endif
#endif

#ifdef MSDOS
    alarm_$disable();
#endif
}


/*
 * P F M _ $ I N H I B I T
 *
 * Inhibit asynchronous faults.
 */

void pfm_$inhibit()
{
    pfm_$inhibit_faults(); 
}


/*
 * P F M _ $ E N A B L E
 *
 * Enable asynchronous faults.
 */

void pfm_$enable()
{
    pfm_$enable_faults();
}


/*
 * P F M _ $ I N H I B I T _ F A U L T S
 *
 * Inhibit asynchronous faults.
 */

void pfm_$inhibit_faults()
{
    if (pfm_$fault_inh_count++ != 0)
#if defined(MSDOS) && defined(MAX_DEBUG)
    {
        extern volatile short NEAR alarm_$disabled;
        ASSERT(alarm_$disabled > 0);
        return;
    }
#else
        return;
#endif

    inhibit_faults();
}


/*
 * P F M _ $ E N A B L E _ F A U L T S
 *
 * Enable asynchronous faults.
 */

void pfm_$enable_faults()
{
    ASSERT(pfm_$fault_inh_count > 0);
#if defined(MSDOS) && defined(MAX_DEBUG)
    {
        extern volatile short NEAR alarm_$disabled;
        ASSERT(alarm_$disabled > 0);
    }
#endif
    if (--pfm_$fault_inh_count != 0)
        return;

#ifdef BSD
    sigsetmask(pfm_$saved_mask);
#endif

#if defined(SYS5) || defined(MSDOS)
    sigrelse(SIGINT);
    sigrelse(SIGTERM);
#ifndef MSDOS
    sigrelse(SIGHUP);
    sigrelse(SIGQUIT);
    sigrelse(SIGCLD);
#endif
#endif

#ifdef MSDOS
    alarm_$enable();
#endif
}


/*
 * P F M _ $ S I G N A L
 *
 * Raise a syncronous exception.
 */

void pfm_$signal(st)
status_$t st;
{
    pfm_$cleanup_rec *crec;
    char buff[100];
    extern char *error_$c_text();

    crec = pfm_$crec_list;
    if (crec == 0) {
        if (st.all == status_$ok)
            exit(0);
        else {
            remove_signal_handler();
#ifdef MSDOS
            fprintf(stderr, "*** Exiting: exception caught by PFM system cleanup handler\n*** status %08lx\n",
                    st.all);
#else
            fprintf(stderr, "*** Exiting: exception caught by PFM system cleanup handler\n*** %s\n",
                    error_$c_text(st, buff, sizeof buff));
#endif
            exit(-1);
        }
    }

    pfm_$crec_list = pfm_$crec_list->next;

    if (pfm_$fault_inh_count == 0)
        inhibit_faults();

    pfm_$fault_inh_count = crec->fault_inh_count + 1;

    longjmp(crec->buf, (st.all == 0 ? pfm_$signalled_zero : st.all));
}


/*
 * P G M _ $ E X I T
 */

void pgm_$exit()
{
    status_$t st;

    st.all = status_$ok;
    pfm_$signal(st);
}
