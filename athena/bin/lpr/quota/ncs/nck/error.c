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
 * E R R O R
 *
 * Routines for munging status codes and texts.
 */

/*
 * The native Apollo version of this module uses mapping (mst_) instead of
 * Unix I/O primitives because (a) we want this module to have as few
 * dependencies as possible and (b) the support for the Unix I/O primitives
 * isn't available in the Apollo boot shell.
 */

#if ! defined(NCK) && defined(apollo) && ! defined(TEST_STREAMS)
#  define USE_MAPPING
#endif

#ifndef NCK
#  ifdef USE_MAPPING
#    include <apollo/sys/ubase.h>
#    include <apollo/sys/mst.h>
#    include <apollo/sys/file.h>
#    include <apollo/sys/name.h>
#  else
#    include <apollo/base.h>
#  endif 
#else
#  ifdef DSEE
#    include "$(nbase.idl).h"
#  else
#    include "nbase.h"
#  endif 
#  ifdef vms
#    include <file.h>
#  else
#    include <fcntl.h>
#  endif
#endif 

#include "error.h"

#ifdef USE_MAPPING

typedef struct fd {
    uid_$t uid;
    void *p;
    linteger len;
    linteger pos;
    file_$lock_desc_t ld;
} fd_t;

#define OKFD(fd) ((fd).p != 0)

#else

typedef int fd_t;
#define OKFD(fd) ((fd) > 0)

#ifdef MSDOS
#define OMODE O_BINARY
#else
#define OMODE 0
#endif

#define eOpen(fd, name)         *(fd) = open(name, OMODE | O_RDONLY)
#define eRead(fd, buff, len)    read(*(fd), buff, len)
#define eClose(fd)              close(*(fd))
#define eLseek(fd, offset)      lseek(*(fd), offset, 0)

#endif

#ifdef init_first

/*
 * "init_first" flags that we're being built for the Apollo boot shell / env, a
 * fairly primitive environment.
 */

static int
strlen(s)
    char *s;
{
    int i = 0;

    while (*s++)
        i++;

    return i;
}

static void 
bcopy(p1, p2, len)
    char *p1, *p2;
{
    int i;

    for (i = 0; i < len; i++)
        *p2++ = *p1++;

}

#endif

static void
xstrcpy(s1, s2, n)
    char *s1, *s2;
    long n;
{
    long i = 0;

    while (*s2 && ++i < n)
        *s1++ = *s2++;

    *s1 = 0;
}

#ifdef USE_MAPPING

static void
eOpen(fd, name)
    fd_t *fd;
    char *name;
{
    status_$t st;

    name_$resolve(name, strlen(name), &fd->uid, &st);
    if (st.all != status_$ok) {
        fd->p = 0;
        return;
    }

    file_$lock_d(fd->uid, file_$nr_xor_1w, file_$read, false, &fd->ld, &st);
    if (st.all != status_$ok) {
        fd->p = 0;
        return;
    }

    fd->p = mst_$map(fd->uid, 0, 1024, access_$r, fd->ld, false, &fd->len, &st);
    if (st.all != status_$ok) {
        fd->p = 0;
        return;
    }

    fd->pos = 0;
}

static int
eRead(fd, buff, len)
    fd_t *fd;
    void *buff;
    int len;
{
    linteger new_len;
    status_$t st;

    fd->p = mst_$remap(fd->p, fd->len, fd->pos, (linteger) len, &new_len, &st);
    if (st.all != status_$ok)
        return -1;

    fd->len = new_len;
    fd->pos += len;

    bcopy(fd->p, buff, len);
    return len;
}

static int
eLseek(fd, offset)
    fd_t *fd;
    long offset;
{
    fd->pos = offset;
    return 0;
}

static int
eClose(fd)
    fd_t *fd;
{
    status_$t st;

    mst_$unmap(fd->uid, fd->p, fd->len, &st);
    if (st.all != status_$ok)
        return -1;

    file_$unlock_d(fd->uid, fd->ld, file_$read, &st);
    if (st.all != status_$ok)
        return -1;

    return 0;
}

#endif


/*
 * E R R O R _ $ C _ G E T _ T E X T
 *
 * Find the texts for a status code.
 */

#define BATCH_SUBMODS 256

void 
error_$c_get_text(status, sub_np, sub_max, mod_np, mod_max, err_p, err_max)
    status_$t status;
    char *sub_np;
    long sub_max;
    char *mod_np;
    long mod_max;
    char *err_p;
    long err_max;
{
    short code;
    fd_t fd;
    struct hdr_hdr_t hdrhdr;
    struct hdr_elt_t hdr[BATCH_SUBMODS];
    struct modhdr_t modhdr;
    register short i, j;
    short k, ccnt;
    short submod;
    char buff[256];
    int match;
    long offset;

    status.all &= ~0x80000000;

    *sub_np = 0;
    *mod_np = 0;
    *err_p = 0;               

    /*
     * Open the database file and read the "header header".
     */

    eOpen(&fd, ERROR_DATABASE);

    if (! OKFD(fd))
        return;

    if (eRead(&fd, &hdrhdr, sizeof hdrhdr) < sizeof hdrhdr)
        goto EXIT;

    swab_32(&hdrhdr.version);
    swab_32(&hdrhdr.count);

    if (hdrhdr.version != ERROR_VERSION)
        goto EXIT;

    /*
     * Read the header entries in batches looking for the specified
     * subsys/module.  Remember any entries that match just on subsystem
     * so in case we get no complete match we'll still be able to gen up
     * the subsystem name.
     */

    offset = -1;   
    match = 0;
    submod = (status.all >> 16) & 0xffff;

    for (i = 0, j = BATCH_SUBMODS; i < hdrhdr.count && ! match; i++, j++) {
        if (j == BATCH_SUBMODS) {
            if (eRead(&fd, hdr, sizeof hdr) < sizeof hdr)
                goto EXIT;
            j = 0;
        }

        swab_16(&hdr[j].submod);
        swab_32(&hdr[j].offset);

        if (submod == hdr[j].submod) {
            offset = hdr[j].offset;
            match = 1;
        }

        if ((submod & 0xff00) == (hdr[j].submod & 0xff00))
            offset = hdr[j].offset;
    }

    /*
     * If no subsys/module or subsys-only match, we lose.
     */

    if (offset == -1) 
        goto EXIT;

    /*
     * Found a match (or subsys-only match), seek to where the header says info for that
     * subsys/module starts.  Read out the module header and copy out the
     * subsys and module names into the output params.  If we got a subsys-only
     * match, copy out only the subsys name and then return.
     */

    eLseek(&fd, offset);
    eRead(&fd, &modhdr, sizeof modhdr);

    swab_16(&modhdr.min_code);
    swab_16(&modhdr.max_code);

    xstrcpy(sub_np, modhdr.ss_name, sub_max);

    if (! match) 
        goto EXIT;

    xstrcpy(mod_np, modhdr.mod_name, mod_max);

    /*
     * Check for specified code being in range.
     */

    code = status.all & 0xffff;

    if (code < modhdr.min_code || code > modhdr.max_code) 
        goto EXIT;

    /*
     * Scan the error texts following the module header until we count the
     * right number of NULLs (text ends).
     */ 

    ccnt = modhdr.min_code;

    while (true) {
        eRead(&fd, buff, sizeof buff);

        for (i = 0; i < sizeof buff; i++) {
            if (ccnt == code)
                goto FOUND_CODE;
            if (buff[i] == 0)
                ccnt++;
        }
    }

FOUND_CODE:

    /*
     * We're at the right spot now -- copy out the error text.
     */

    k = 0;

    while (true) {
        for (j = i; j < sizeof buff; j++) {
            if (++k >= err_max) {
                *err_p = 0;
                goto EXIT;
            }
            *err_p++ = buff[j];
            if (buff[j] == 0) {
                *err_p = 0;
                goto EXIT;
            }
        }

        eRead(&fd, buff, sizeof buff);
        i = 0;
    }

EXIT: 

    eClose(&fd);
}


#ifndef init_first

/*
 * E R R O R _ $ C _ T E X T
 *
 * Decode a status code into a string.
 */

char *
error_$c_text(st, buff, maxlen)
    status_$t st;
    char *buff;
    int maxlen;
{
    char sp[100], mp[100], ep[100];

    error_$c_get_text(st, sp, (long) sizeof sp, mp, (long) sizeof mp, ep, (long) sizeof ep);

    /* +++ Need to check against maxlen, but it's a bit of a pain to do +++ */

    if (*sp == 0)
        sprintf(buff, "status %lx", st.all);
    else if (*mp == 0)
        sprintf(buff, "status %lx (%s)", st.all, sp);
    else if (*ep == 0)
        sprintf(buff, "status %lx (%s/%s)", st.all, sp, mp);
    else
        sprintf(buff, "%s (%s/%s)", ep, sp, mp);

    return (buff);
}

#endif


/*
 * E R R O R _ $ F I N D _ T E X T
 *
 * In the old Pascal days, this used to be the primitive error text retriever.
 * Since the error texts used to be in static initialized storage (instead of a 
 * file), this procedure could simply return pointers to the text.  Now that the
 * text is in an external file, we have to copy the text into static storage
 * and return pointers to it.  Not highly re-entrant.  "error_$get_text" is
 * more recommended.
 */

struct {
    char sp[100];
    char mp[100];
    char ep[100];
} error_$find_text_static;

void 
error_$find_text(status, sub_np, sub_nl, mod_np, mod_nl, err_p, err_l)
    status_$t *status;
    char **sub_np;
    short *sub_nl;
    char **mod_np;
    short *mod_nl;
    char **err_p;
    short *err_l;
{
    error_$c_get_text(*status, 
                      error_$find_text_static.sp, (long) sizeof error_$find_text_static.sp, 
                      error_$find_text_static.mp, (long) sizeof error_$find_text_static.mp, 
                      error_$find_text_static.ep, (long) sizeof error_$find_text_static.ep);

    *sub_np = error_$find_text_static.sp;
    *mod_np = error_$find_text_static.mp;
    *err_p  = error_$find_text_static.ep;

    *sub_nl = strlen(error_$find_text_static.sp);
    *mod_nl = strlen(error_$find_text_static.mp);
    *err_l  = strlen(error_$find_text_static.ep);
}
