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
 * U U I D
 *
 * This module contains routines that generate and manipulate UUIDs.
 */

#include "std.h"

#include "pbase.h"

#ifdef DSEE
#  include "$(socket.idl).h"
#  include "$(uuid.idl).h"
#else
#  include "socket.h"
#  include "uuid.h"
#endif

#ifdef INET 
#include <netinet/in.h>
#ifdef cray
struct sockaddr_in_ncs {
    short   sin_family;
    u_int   sin_port: 16;
    u_int   sin_addr: 32;
};
#else
#  define sockaddr_in_ncs sockaddr_in
#endif
#endif

#ifdef SOLARIS
/*
 * flock operations.
 */
#define LOCK_SH               1       /* shared lock */
#define LOCK_EX               2       /* exclusive lock */
#define LOCK_NB               4       /* don't block when locking */
#define LOCK_UN               8       /* unlock */
#endif

/*
 * Internal structure of UUIDs
 *
 * The first 48 bits are the number of 4 usec units of time that have passed
 * since 1/1/80 0000 GMT.  The next 16 bits are reserved for future use.
 * The next 8 bits are an address family.  The next 56 bits are a host
 * ID in the form allowed by the specified address family.
 *
 *       |<------------------- 32 bits --------------------->|
 *
 *       +---------------------------------------------------+
 *       |           high 32 bits of bit time                |
 *       +------------------------+--------------------------+
 *       |   low 16 bits of time  |     16 bits reserved     |
 *       +------------+-----------+--------------------------+
 *       | addr fam   |      1st 24 bits of host ID          |
 *       +------------+-----------+--------------------------+
 *       |            32 more bits of host ID                |
 *       +---------------------------------------------------+
 *
 */


#ifdef apollo

/*
 * Internal structure of Apollo 64 bit UIDs.
 */

struct uid_t {
    u_long clockh;
    u_int clockl: 4;
    u_char extra: 8;
    u_long node: 20;
};

#endif

#ifndef apollo

#ifdef UNIX
#  define UUID_FILE "/tmp/last_uuid"
#endif

#ifdef VMS
#  define UUID_FILE "sys$scratch:last_uuid.dat"
#endif

/*
 * C H E C K _ U U I D
 *
 * On a system wide basis, check to see if the passed UUID is the
 * same or older than the previously generated one. If it is, make sure
 * it becomes a little newer.  Write the UUID back to the "last UUID"
 * storage in any case. In the case of systems using a file as
 * the storage, fall back to "per process" checking in the event of
 * the inability to safely access the storage.
 */

static void check_uuid(uuid)
uuid_$t *uuid;
{
    int n, l = 0;
    static uuid_$t NEAR last_uuid;
#ifndef MSDOS
    int fd;
    extern errno;
#endif
#ifdef SYS5
    struct flock farg;
#endif

#ifdef MSDOS

    n = sizeof(uuid_$t);

#else

    /*
     * Get the last value generated from the UUID file.
     */

    if ((fd = open(UUID_FILE, O_RDWR)) == -1)
        if ((fd = creat(UUID_FILE, 0666)) != -1)
            chmod(UUID_FILE, 0666);

    /*
     * If the open/create failed, the lock request will fail (as is desired)
     * and we'll just fall into "per process" checking mode.
     */

#ifdef BSD
    while ((l = flock(fd, LOCK_EX)) == -1 && errno == EINTR)
        ;
#endif
#ifdef SYS5
    farg.l_type   = F_WRLCK;
    farg.l_whence = 0;
    farg.l_start  = 0;
    farg.l_len    = 0;
    while ((l = fcntl(fd, F_SETLKW, &farg)) == -1 && errno == EINTR)
        ;
#endif

    if (l == -1)
        n = sizeof(uuid_$t);    /* just use the last uuid of this process as a reference */
    else
        n = read(fd, &last_uuid, sizeof(uuid_$t));

#endif

    if (n == sizeof(uuid_$t) &&
       (uuid->time_high < last_uuid.time_high ||
       (uuid->time_high == last_uuid.time_high && uuid->time_low <= last_uuid.time_low)))
    {
        uuid->time_high = last_uuid.time_high;
        if ((uuid->time_low = (last_uuid.time_low + 1) & 0xffff) == 0)
            uuid->time_high++;
    }

    /*
     * Update the per process retained value (and the system wide value if appropriate).
     */

    last_uuid.time_high = uuid->time_high;
    last_uuid.time_low = uuid->time_low;

#ifndef MSDOS
    if (l != -1) {      /* don't risk corrupting the file */
        lseek(fd, 0, L_SET);
        write(fd, uuid, sizeof(uuid_$t));
#ifdef BSD
        flock(fd, LOCK_UN);
#endif
#ifdef SYS5
        /* close will unlock (will it for bsd?) */
#endif
    }

    close(fd);
#endif
}

#endif


#ifdef apollo

/*
 * U U I D _ $ T O _ U I D
 *
 * Convert a UUID (128 bits) to an Apollo UID (64 bits).
 */

void uuid_$to_uid(uuid, _uid, st)
uuid_$t *uuid;
uid_$t *_uid;
status_$t *st;
{
    struct uid_t *uid = (struct uid_t *) _uid;

    if (((uuid_$t *) uuid)->family != socket_$dds) {
        st->all = -1;
        return;
    }

    uid->clockh = uuid->time_high;
    uid->clockl = uuid->time_low >> 12;
    uid->extra  = 0;
    uid->node   = (uuid->host[0] << 24) | (uuid->host[1] << 16) |
                  (uuid->host[2] <<  8) | (uuid->host[3]);

    st->all = status_$ok;
}


/*
 * U U I D _ $ F R O M _ U I D
 *
 * Convert an Apollo UID to a UUID.
 */

void uuid_$from_uid(_uid, uuid)
uid_$t *_uid;
uuid_$t *uuid;
{
    struct uid_t *uid = (struct uid_t *) _uid;

    bzero(uuid, sizeof(*uuid));

    uuid->time_high = uid->clockh;
    uuid->time_low  = uid->clockl << 12;
    uuid->reserved  = 0;
    uuid->family    = socket_$dds;
    uuid->host[0]   = uid->node >> 24;
    uuid->host[1]   = uid->node >> 16;
    uuid->host[2]   = uid->node >>  8;
    uuid->host[3]   = uid->node;
}

#endif


/*
 * U U I D _ $ G E N
 *
 * Generate a new UUID.
 */

void uuid_$gen(uuid)
uuid_$t *uuid;
{
#ifdef apollo

    std_$call void uid_$gen();
    struct uid_t uid;

    uid_$gen(uid);
    uuid_$from_uid((uid_$t *) &uid, uuid);

#else

#ifndef MSDOS
    struct timeval tv;
    struct timezone tz;
    double usec4;       /* Floating 4-usec ticks since 1/1/80 */
#endif
    socket_$net_addr_t naddr;
#ifdef INET
    struct sockaddr_in_ncs saddr;
#else
    socket_$addr_t saddr;
#endif
    u_long nlen, slen, hlen;
    status_$t st;
    static socket_$host_id_t host;
    static u_long dlen;

#ifdef MSDOS

    alarm_$4usec_time(&uuid->time_high, &uuid->time_low);

#else

    /*
     * The difference between Aegis time and Unix time is 10 years.
     * Aegis starts at Jan 1, 1980, and Unix starts at Jan 1, 1970.
     * This constant is roughly:
     *
     *      10 years * 365 days * 24 hours * 60 minutes * 60 seconds
     */

#   define decade_1970  (3652L * 24 * 60 * 60)

    gettimeofday(&tv, &tz);
    tv.tv_sec -= decade_1970;

    usec4 = tv.tv_sec * (1000000.0 / 4.0) + (tv.tv_usec / 4);

    uuid->time_high = usec4 / (unsigned long) 0xffff;
    uuid->time_low  = tv.tv_usec / 4;

#endif

    check_uuid(uuid);

    if (dlen == 0) {
        u_long n_families = socket_$num_families;
        socket_$addr_family_t families[socket_$num_families];

        socket_$valid_families(&n_families, families, &st);
        socket_$inq_my_netaddr((u_long) families[0], &naddr, &nlen, &st);
        socket_$set_netaddr(&saddr, &slen, &naddr, nlen, &st);
        bzero(&host, sizeof host);
        socket_$inq_hostid(&saddr, slen, &host, &hlen, &st);
        dlen = hlen - sizeof(host.family);
    }

    uuid->reserved = 0;
    uuid->family = host.family;

    bzero(uuid->host, sizeof(uuid->host));
    bcopy(host.data, uuid->host,
          (int) ((dlen < sizeof(uuid->host)) ? dlen : sizeof(uuid->host)));

#endif
}


/*
 * U U I D _ $ E N C O D E
 *
 * Encode a UUID into a printable string.
 */

void uuid_$encode(uuid, s)
uuid_$t *uuid;
uuid_$string_t s;
{
    sprintf((char *) s, "%08lx%04x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x",
            uuid->time_high, uuid->time_low,
            uuid->family,
            (unsigned char) uuid->host[0], (unsigned char) uuid->host[1],
            (unsigned char) uuid->host[2], (unsigned char) uuid->host[3],
            (unsigned char) uuid->host[4], (unsigned char) uuid->host[5],
            (unsigned char) uuid->host[6]);

}

/* tttttttttttt.ff.h1.h2.h3.h4.h5.h6.h7 */

/*
 * U U I D _ $ D E C O D E
 *
 * Decode a UUID from a printable string.
 */

void uuid_$decode(s, uuid, st)
uuid_$string_t s;
uuid_$t *uuid;
status_$t *st;
{
    u_short host[7];
    u_short family;
    u_long time_high;
    u_short time_low;

    int i;

    i = sscanf((char *) s, "%8lx%4hx.%2hx.%2hx.%2hx.%2hx.%2hx.%2hx.%2hx.%2hx",
            &time_high, &time_low, &family,
            &host[0], &host[1], &host[2], &host[3], &host[4], &host[5], &host[6]);

    if (i != 10) {
        st->all = -1;
        return;
    }

    st->all = status_$ok;

    uuid->time_high = time_high;
    uuid->time_low = time_low;

    uuid->family = family;
    uuid->reserved = 0;

    uuid->host[0] = host[0];
    uuid->host[1] = host[1];
    uuid->host[2] = host[2];
    uuid->host[3] = host[3];
    uuid->host[4] = host[4];
    uuid->host[5] = host[5];
    uuid->host[6] = host[6];
}


/*
 * U U I D _ $ E Q U A L
 *
 * Compare two UUIDs
 */

boolean uuid_$equal(u1, u2)
uuid_$t *u1, *u2;
{
#ifdef CONVENTIONAL_ALIGNMENT
    return(bcmp(u1, u2, sizeof(uuid_$t)) == 0);
#else
    return(
        u1->time_high == u2->time_high
        && u1->time_low == u2->time_low
        && u1->reserved == u2->reserved
        && u1->family == u2->family
        && u1->host[0] == u2->host[0]
        && u1->host[1] == u2->host[1]
        && u1->host[2] == u2->host[2]
        && u1->host[3] == u2->host[3]
        && u1->host[4] == u2->host[4]
        && u1->host[5] == u2->host[5]
        && u1->host[6] == u2->host[6]
        );
#endif
}

long uuid_$cmp(u1, u2)
uuid_$t *u1, *u2;
{
    long val, i;
    register u_char *tu1 = (u_char *)u1;
    register u_char *tu2 = (u_char *)u2;
    
    for (i = 0; i < sizeof(uuid_$t); i++)
    {
        val = *tu1++ - *tu2++;
        if (val != 0) return(val);
    }
    return(val);
}

u_long uuid_$hash(u, modulus)
    uuid_$t *u;
    u_long modulus;
{
    unsigned long *lp = (unsigned long *) u;
    u_long hash = lp[0] ^ lp[1] ^ lp[2] ^ lp[3];
    hash = ((hash >> 16) ^ hash) & 0x0000ffff;
    return hash % modulus;
}

#ifdef FTN_INTERLUDES

void uuid_$gen_(uuid)
uuid_$t *uuid;
{
    uuid_$gen(uuid);
}

void uuid_$encode_(uuid, s)
uuid_$t *uuid;
uuid_$string_t s;
{
    uuid_$encode(uuid, s);
}

void uuid_$decode_(s, uuid, st)
uuid_$string_t s;
uuid_$t *uuid;
status_$t *st;
{
    uuid_$decode(s, uuid, st);
}

boolean uuid_$equal_(u1, u2)
uuid_$t *u1, *u2;
{
    return uuid_$equal(u1, u2);
}

long uuid_$cmp_(u1, u2)
uuid_$t *u1, *u2;
{
    return uuid_$cmp(u1, u2);
}

u_long uuid_$hash_(u, modulus)
    uuid_$t *u;
    u_long *modulus;
{
    return uuid_$hash(u, *modulus);
}

#ifdef apollo

void uuid_$to_uid_(uuid, _uid, st)
uuid_$t *uuid;
uid_$t *_uid;
status_$t *st;
{
    uuid_$to_uid(uuid, _uid, st);
}

void uuid_$from_uid_(_uid, uuid)
uid_$t *_uid;
uuid_$t *uuid;
{
    uuid_$from_uid(_uid, uuid);
}

#endif

#endif

