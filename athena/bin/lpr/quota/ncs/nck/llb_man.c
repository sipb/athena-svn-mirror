/*      llb_man.c
 *      Location Broker - server side - implementation of llb_ interface
 *          Also used by the non-replicated glbd.
 *
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
 */

#include "std.h"

#ifdef DSEE
#  include "$(nbase.idl).h"
#  include "$(lb.idl).h"
#  include "$(llb.idl).h"
#  include "$(socket.idl).h"
#else
#  include "nbase.h"
#  include "lb.h"
#  include "llb.h"
#  include "socket.h"
#endif

#include "lb_p.h"
#include "llb_man.h"

#include "pfm.h"

/*
 * Internal Types 
 */

/* struct lb_fields - used to note which fields are active in a comparison */
struct lb_fields {
    int flags;
    int f1;
    int f2;
    int f3;
    int f4;
};

struct file_hdr {
    int version;
};

struct db {
    int fd;                     /* file descriptor for the database file */
    int n_entries;
    int max_entries;
    struct db_data {
        int valid;
        lb_$entry_t info;
    } data[1];
};

static struct lb_fields lb_all_fields = {0, 0, 0, 0, 0,};

#define lbdb_$file_version 5

#ifdef vms
#  define llb_$database_pathname "ncs$exe:llbdbase.dat"
#endif
#ifdef UNIX
#  define llb_$database_pathname "/tmp/llbdbase.dat"
#endif
#ifdef MSDOS
#  define llb_$database_pathname NULL  /* no disk backup needed */
#endif

#ifdef MSDOS
#  define llb_$max_entries 50
#else
#  define llb_$max_entries 500
#endif

struct db *llb_$db;

#define llbd_$no_disk   -2    /* llbd does not need to survive crashes */

#ifdef __STDC__
static boolean eq_service(struct lb_fields f, lb_$entry_t *a, lb_$entry_t *b, boolean exact, status_$t *st);
static void open_database_file(struct db *h, char *pathname, status_$t *status);
static int init(status_$t *st);
static long find(struct db *h, struct lb_fields f, long startpos, lb_$entry_t *xentry, lb_$entry_t *result, boolean exact);
static void update_file(struct db *h, long slot, status_$t *status);
static void append(struct db *h, long slot, lb_$entry_t *xentry, status_$t *status);
static void chk_entry(lb_$entry_t *entry, status_$t *status);
#endif


static boolean eq_service(f, a, b, exact, st)
    struct lb_fields f;
    lb_$entry_t *a;
    lb_$entry_t *b;
    boolean exact;
    status_$t *st;
{
    boolean is_equal;

    is_equal = f.f1;
    is_equal = is_equal || EQ_UID(a->object, b->object);
    is_equal = is_equal || (!exact && IS_UIDNIL(b->object));
    if (!is_equal)
        return false;

    is_equal = f.f2;
    is_equal = is_equal || EQ_UID(a->obj_type, b->obj_type);
    is_equal = is_equal || (!exact && IS_UIDNIL(b->obj_type));
    if (!is_equal)
        return false;

    is_equal = f.f3;
    is_equal = is_equal || EQ_UID(a->obj_interface, b->obj_interface);
    is_equal = is_equal || (!exact && IS_UIDNIL(b->obj_interface));
    if (!is_equal)
        return false;

    if (!f.f4)
        return socket_$equal(&(a->saddr), (a->saddr_len), &(b->saddr),
                           (b->saddr_len), (u_long) socket_$eq_netaddr, st);
    else
        return true;
}

static void open_database_file(h, database_file, status)
    struct db *h;
    char *database_file;
    status_$t *status;
{
    struct file_hdr hdr;
    int oumask;
    int num_bytes_read;

    SET_STATUS(status, status_$ok);

    if (database_file == NULL) {
        h->fd = llbd_$no_disk;
        return;
    }

    h->fd = open(database_file, O_RDWR | O_BINARY);
    if (h->fd < 0) {
        oumask = umask(0);
        h->fd = open(database_file, O_RDWR | O_CREAT | O_BINARY, 0666);
        (void) umask(oumask);
        if (h->fd >= 0) {
            hdr.version = lbdb_$file_version;
            write(h->fd, &hdr, sizeof(hdr));
#ifdef BSD
            ftruncate(h->fd, sizeof(hdr));
#endif
        }
        else {
            SET_STATUS(status, lb_$cant_access);
        }
    }
    else {
        read(h->fd, &hdr, sizeof(hdr));
        if (hdr.version != lbdb_$file_version) {
            close(h->fd);
            if (hdr.version < lbdb_$file_version) {
#ifndef vms
                if (unlink(database_file) != -1) {
#else
                if (delete(database_file) != -1) {
#endif
                    open_database_file(h, database_file, status);
                    return;
                }
            }
            h->fd = -1;
            SET_STATUS(status, lb_$database_invalid);
        }
        else {
            if (h->fd < 4) {
                int new_fd[4];
                int i, j;

                for (i = 0; i < 4; i++) {
                    new_fd[i] = dup(h->fd);
                    if (new_fd[i] > 3)
                        break;
                }

                close(h->fd);
                h->fd = new_fd[i];
                for (j = 0; j < i; j++)
                    close(new_fd[j]);
            }
            num_bytes_read = read(h->fd, (char *) h->data,
                                  sizeof(struct db_data) * h->max_entries);
            h->n_entries = num_bytes_read / sizeof(struct db_data);
        }
    }
}

void *lbdb_$init(pathname, max_entries, status)
    char *pathname;
    int max_entries;
    status_$t *status;
{
    struct db *db;
    int db_size = sizeof(struct db) + sizeof(struct db_data) * (max_entries - 1); 

    db = (struct db *) malloc(db_size);
    if (db == NULL) {
        status->all = lb_$update_failed;
        return NULL;
    }

    bzero(db, db_size);
    db->fd = -1;
    db->n_entries = 0;
    db->max_entries = max_entries;

    open_database_file(db, pathname, status);

    if (status->all != status_$ok) {
        free(db);
        return NULL;
    }

    return (void *) db; 
}

/*
 * returns the position of the record or -1 if not found 
 */
static long find(h, f, startpos, xentry, result, exact)
    struct db *h;
    struct lb_fields f;
    long startpos;
    lb_$entry_t *xentry;
    lb_$entry_t *result;
    boolean exact;
{
    status_$t status;
    long i;
    lb_$entry_t *p;

    for (i = startpos; i < h->n_entries; i++) {
        p = &h->data[i].info;
        if (h->data[i].valid && (f.flags || p->flags == xentry->flags) &&
            eq_service(f, xentry, p, exact, &status)) {
            if (result != NULL)
                *result = h->data[i].info;
            return i;
        }
    }
    return -1;
}


static void update_file(h, slot, status)
    struct db *h;
    long slot;
    status_$t *status;
{
    long pos;

    SET_STATUS(status, status_$ok);
    if (h->fd == llbd_$no_disk)
        return;

    pos = sizeof(struct file_hdr) + slot * sizeof(struct db_data);

    if (lseek(h->fd, pos, L_SET) == -1) {
        SET_STATUS(status, lb_$update_failed);
    }
    else {
        int len = write(h->fd, (char *) &h->data[slot], sizeof(struct db_data));
        if (len != sizeof(struct db_data)) {
            SET_STATUS(status, lb_$update_failed);
        }
        else {
#ifdef BSD
            fsync(h->fd);
#endif
        }
    }
}

static void append(h, slot, xentry, status)
    struct db *h;
    long slot;
    lb_$entry_t *xentry;
    status_$t *status;
{
    int i;

    SET_STATUS(status, status_$ok);

    if (slot == -1) {
        for (i = 0; i < h->max_entries; i++) {
            if (!h->data[i].valid) {
                slot = i;
                break;
            }
        }
    }

    if (slot >= h->max_entries) {
        SET_STATUS(status, lb_$update_failed);
    }
    else {
        if (slot >= h->n_entries) {
            h->n_entries = slot + 1;
        }
        h->data[slot].valid = TRUE;
        h->data[slot].info = *xentry;

        update_file(h, slot, status);
    }
}

static void chk_entry(entry, status)
    lb_$entry_t *entry;
    status_$t *status;
{
    if ((entry->saddr_len > sizeof(entry->saddr.family)) &&
        (entry->saddr_len <= sizeof(entry->saddr)))
        status->all = status_$ok;
    else
        status->all = lb_$bad_entry;
}


void lbdb_$insert(h, xentry, status)
    struct db *h;
    lb_$entry_t *xentry;
    status_$t *status;
{
    struct lb_fields all_but_flags;
    long pos;

    SET_STATUS(status, status_$ok);

    chk_entry(xentry, status);
    if (!STATUS_OK(status))
        return;

    all_but_flags = lb_all_fields;
    all_but_flags.flags = 1;

    pos = find(h, all_but_flags, (u_long) lb_$default_lookup_handle, xentry, NULL, true);
    append(h, pos, xentry, status);
}

void lbdb_$delete(h, xentry, status)
    struct db *h;
    lb_$entry_t *xentry;
    status_$t *status;
{
    long pos;
    struct lb_fields all_but_flags;
    lb_$entry_t buf;

    SET_STATUS(status, status_$ok);

    chk_entry(xentry, status);
    if (!STATUS_OK(status))
        return;

    all_but_flags = lb_all_fields;
    all_but_flags.flags = 1;

    pos = find(h, all_but_flags, (u_long) lb_$default_lookup_handle,
               xentry, &buf, true);
    if (pos != -1) {
        h->data[pos].valid = FALSE;
        update_file(h, pos, status);
    }
    else {
        SET_STATUS(status, lb_$not_registered);
    }
}


void lbdb_$lookup(h, object, obj_type, obj_interface, entry_handle,
                max_num_results, match, num_results, result_entries, status)
    struct db *h;
    uuid_$t *object;
    uuid_$t *obj_type;
    uuid_$t *obj_interface;
    lb_$lookup_handle_t *entry_handle;
    u_long max_num_results;
    boolean match;
    u_long *num_results;
    lb_$entry_t result_entries[];
    status_$t *status;
{
    struct lb_fields key_fields;
    u_long num = 0;
    long pos;
    lb_$entry_t *p;
    lb_$entry_t match_service;

#define INIT_KEY(KF,FN) \
    if (FN != NULL && !EQ_UID((*FN), uuid_$nil)) { \
        match_service.FN = *FN; \
        key_fields.KF = 0; \
    } else { \
        key_fields.KF = 1; \
    }

    key_fields.flags = TRUE;
    key_fields.f4 = TRUE;
    INIT_KEY(f1, object);
    INIT_KEY(f2, obj_type);
    INIT_KEY(f3, obj_interface);
#undef INIT_KEY

    SET_STATUS(status, status_$ok);

    p = &result_entries[0];
    for (pos = *entry_handle; pos != -1;) {
        pos = find(h, key_fields, pos, &match_service, p, match);
        if (pos != -1) {
            p++;
            num++;
            pos++;
            if (num == max_num_results)
                break;
        }
    }

    if (pos == -1)
        *entry_handle = lb_$default_lookup_handle;
    else
        *entry_handle = pos;

    *num_results = num;
    if (num == 0) {
        SET_STATUS(status, lb_$not_registered);
    }
}

static int init(status)
    status_$t *status;
{
    if (llb_$db != NULL)
        return 1;
    else {
        llb_$db = (struct db *) lbdb_$init(llb_$database_pathname, llb_$max_entries, status);
        return (status->all == status_$ok);
    }
}

void llb_$insert(h, xentry, status)
    handle_t h;
    lb_$entry_t *xentry;
    status_$t *status;
{
    if (! init(status))
        return;

    lbdb_$insert(llb_$db, xentry, status); 
}

void llb_$delete(h, xentry, status)
    handle_t h;
    lb_$entry_t *xentry;
    status_$t *status;
{
    if (! init(status))
        return;

    lbdb_$delete(llb_$db, xentry, status);
}

void llb_$lookup(h, object, obj_type, obj_interface, entry_handle,
                      max_num_results, num_results, result_entries, status)
    handle_t h;
    uuid_$t *object;
    uuid_$t *obj_type;
    uuid_$t *obj_interface;
    lb_$lookup_handle_t *entry_handle;
    u_long max_num_results;
    u_long *num_results;
    lb_$entry_t result_entries[];
    status_$t *status;
{
    if (! init(status))
        return;

    lbdb_$lookup(llb_$db, object, obj_type, obj_interface, entry_handle,
               max_num_results, true, num_results, result_entries, status);
}

void llb_$lookup_match(h, object, obj_type, obj_interface, entry_handle,
                      max_num_results, match, num_results, result_entries, status)
    handle_t h;
    uuid_$t *object;
    uuid_$t *obj_type;
    uuid_$t *obj_interface;
    lb_$lookup_handle_t *entry_handle;
    u_long max_num_results;
    boolean match;
    u_long *num_results;
    lb_$entry_t result_entries[];
    status_$t *status;
{
    if (! init(status))
        return;

    lbdb_$lookup(llb_$db, object, obj_type, obj_interface, entry_handle,
               max_num_results, match, num_results, result_entries, status);
}
