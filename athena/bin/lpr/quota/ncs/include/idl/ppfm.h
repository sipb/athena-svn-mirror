/*
 * ========================================================================== 
 * Copyright  1987 by Apollo Computer Inc., Chelmsford, Massachusetts
 * 
 * All Rights Reserved
 * 
 * All Apollo source code software programs, object code software programs,
 * documentation and copies thereof shall contain the copyright notice above
 * and this permission notice.  Apollo Computer Inc. reserves all rights,
 * title and interest with respect to copying, modification or the
 * distribution of such software programs and associated documentation,
 * except those rights specifically granted by Apollo in a Product Software
 * Program License or Source Code License between Apollo and Licensee.
 * Without this License, such software programs may not be used, copied,
 * modified or distributed in source or object code form.  Further, the
 * copyright notice must appear on the media, the supporting documentation
 * and packaging.  A Source Code License does not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * 
 * Apollo disclaims all warranties, express or implied, with respect to
 * the Software Programs including the implied warranties of merchantability
 * and fitness, for a particular purpose.  In no event shall Apollo be liable
 * for any special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits whether in an
 * action of contract or tort, arising out of or in connection with the
 * use or performance of such software programs.
 * ========================================================================== 
 */

#ifndef ppfm_included
#define ppfm_included

/*
 * Portable Process Fault Manager definitions.
 *
 * Because this interface has undergone so many variants, we enumerate the
 * exact interface below.  Portable programs should use only what appears below.
 * (Note that some of them are actually macros.)
 *
 * void pfm_$init(unsigned long flags);
 * status_$t pfm_$cleanup(pfm_$cleanup_rec *crec);
 * void pfm_$signal(status_$t st);
 * void pfm_$reset_cleanup(pfm_$cleanup_rec *crec, status_$t *st);
 * void pfm_$rls_cleanup(pfm_$cleanup_rec *crec, status_$t *st);
 * void pfm_$inhibit_faults(void);
 * void pfm_$enable_faults(void);
 * void pfm_$inhibit(void);
 * void pfm_$enable(void);
 * void pgm_$exit(void);
 */

#ifdef apollo

#include <apollo/pfm.h>

#define pfm_$init(junk)                 /* No initialization required on Apollo */

#else

#ifdef MSDOS
#define _JBLEN 20                   /* Must match std.h */
#define setjmp  setjmp_nck
#define longjmp longjmp_nck
typedef char jmp_buf[_JBLEN];
extern long setjmp_nck(jmp_buf);
#endif

#ifndef _JBLEN
#  include <setjmp.h>
#  ifndef _JBLEN
#    define _JBLEN (sizeof(jmp_buf) / sizeof(int))
#  endif
#endif

#define pfm_$module_code                    0x03040000
#define pfm_$bad_rls_order                  (pfm_$module_code + 1)
#define pfm_$cleanup_set                    (pfm_$module_code + 3)
#define pfm_$signalled_zero                 (pfm_$module_code + 0xffff)

typedef struct jmp_buf_elt_t {
    struct jmp_buf_elt_t *next;
    int fault_inh_count;
    jmp_buf buf;
    long setjmp_val;        /* This is "long" and not "int" on purpose -- keep MS/DOS happy */
} pfm_$cleanup_rec;  

void pfm_$signal(
#ifdef __STDC__
    status_$t st
#endif
);

void pfm_$inhibit_faults(
#ifdef __STDC__
    void
#endif
);

void pfm_$enable_faults(
#ifdef __STDC__
    void
#endif
);

void pfm_$inhibit(
#ifdef __STDC__
    void
#endif
);

void pfm_$enable(
#ifdef __STDC__
    void
#endif
);

void pfm_$_rls_cleanup(
#ifdef __STDC__
    pfm_$cleanup_rec *crec, 
    status_$t *st
#endif
);

void pfm_$_reset_cleanup(
#ifdef __STDC__
    pfm_$cleanup_rec *crec, 
    status_$t *st
#endif
);

status_$t pfm_$_cleanup(
#ifdef __STDC__
    long st, 
    pfm_$cleanup_rec *crec
#endif
);

#define pfm_$__cleanup(crec) ( \
    (crec)->setjmp_val = setjmp((crec)->buf), \
    pfm_$_cleanup((crec)->setjmp_val, crec) \
)

void pfm_$init(
#ifdef __STDC__
    unsigned long flags
#endif
);

void pgm_$exit(
#ifdef __STDC__
    void
#endif
);

#ifndef pfm_included 
#define pfm_$cleanup            pfm_$__cleanup
#define pfm_$rls_cleanup        pfm_$_rls_cleanup
#define pfm_$reset_cleanup      pfm_$_reset_cleanup
#endif
                                     
#endif

/*
 * Flags to "pfm_$init"
 */  

#define pfm_$init_signal_handlers   0x00000001

#endif
