/*  uname.h, /us/lib/ddslib/src, pato, 05/26/88
**      UUID - name translation functions
**
** ==========================================================================
** Confidential and Proprietary.  Copyright 1988 by Apollo Computer Inc.,
** Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
** Copyright Laws Of The United States.
**
** Apollo Computer Inc. reserves all rights, title and interest with respect
** to copying, modification or the distribution of such software programs
** and associated documentation, except those rights specifically granted
** by Apollo in a Product Software Program License, Source Code License
** or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
** Apollo and Licensee.  Without such license agreements, such software
** programs may not be used, copied, modified or distributed in source
** or object code form.  Further, the copyright notice must appear on the
** media, the supporting documentation and packaging as set forth in such
** agreements.  Such License Agreements do not grant any rights to use
** Apollo Computer's name or trademarks in advertising or publicity, with
** respect to the distribution of the software programs without the specific
** prior written permission of Apollo.  Trademark agreements may be obtained
** in a separate Trademark License Agreement.
** ==========================================================================
**
** UUID name lookup functions.  The current implementation is a hack waiting
** for the name server to be available.
*/

#include "std.h"

#ifdef MSDOS
#  include "b_trees.h"
#else
#  include "balanced_trees.h"
#endif

#ifdef DSEE
#   include "$(nbase.idl).h"
#   include "$(uuid.idl).h"
#else
#   include "nbase.h"
#   include "uuid.h"
#endif

#ifdef UNIX
#   include <pwd.h>

#   ifdef SYS5
        extern struct passwd *getpwuid (
#       ifdef __STDC__
            int uid
#       endif
        );
#   endif

#endif

#define private static
#define public

static boolean uuid_naming_inited = false;

static sym_$tree_handle  uuid_tree = NULL;
static sym_$tree_handle  name_tree = NULL;

private boolean match_command ( key, str, min_len )
    char *key;
    char *str;
    int min_len;
{
    int i = 0;

    if (*key) while (*key == *str) {
        i++;
        key++;
        str++;
        if (*str == '\0' || *key == '\0')
            break;
    }
    if (*str == '\0' && i >= min_len)
        return true;

    return false;
}


private boolean uuid_lookup ( uuid, string )
    uuid_$t *uuid;
    char    **string;
{
    sym_$datum_t *data;
    sym_$datum_t key;

    key.dptr = (char *) uuid;
    key.dsize = sizeof(*uuid);
    data = sym_$fetch_data(uuid_tree, &key);

    if (data && data->dsize > 0) {
        *string = data->dptr;
        return true;
    }

    return false;
}

private boolean name_lookup ( string, uuid )
    char    *string;
    uuid_$t *uuid;
{
    sym_$datum_t *data;
    sym_$datum_t key;

    key.dptr = string;
    key.dsize = strlen(string);
    data = sym_$fetch_data(name_tree, &key);

    if (data && data->dsize > 0) {
        bcopy(data->dptr, uuid, sizeof(*uuid));
        return true;
    }

    return false;
}

private FILE * openfile ( fname )
    char    *fname;
{
    FILE                *fp;
    sym_$datum_t        key;
    sym_$datum_t        data;
    static sym_$tree_handle  file_tree = NULL;

    fp = NULL;

    if (fname && *fname) {
        key.dptr    = fname;
        key.dsize   = strlen(fname);

        data.dptr   = NULL;
        data.dsize  = 0;

        /*
        ** Guard against multiple inclusion of the same file
        */
        if (sym_$insert_data((long) false, &file_tree, &key, &data)) {
            fp = fopen(fname, "r");
            if (fp == NULL) {
                (void) sym_$delete_data(file_tree, &key);
            }
        }
    }

    return fp;
}

private void process_uuid_names ( fp )
    FILE    *fp;
{
    char            line[1024];
    char            **tokens;
    int             num_tokens;
    FILE            *lfp;
    uuid_$t         uuid;
    status_$t       st;
    sym_$datum_t    key;
    sym_$datum_t    data;
    int             i;

    while (fgets(line, sizeof(line), fp) != NULL) {
        /* get rid of trailing newline */
        line[strlen(line)-1] = '\0';

        args_$get(line, &num_tokens, &tokens);
        if (num_tokens > 1) {
            if (match_command("#include", tokens[0], 8)) {
                lfp = openfile(tokens[1]);
                if (lfp != NULL) {
                    process_uuid_names(lfp);
                    fclose(lfp);
                }
            } else if (line[0] == '#') {
                continue;
            } else {
                uuid_$decode((ndr_$char *) tokens[0], &uuid, &st);
                if (st.all == status_$ok) {
                    key.dptr = (char *) &uuid;
                    key.dsize = sizeof(uuid);

                    data.dptr = tokens[1];
                    data.dsize = strlen(data.dptr) + 1;

                    sym_$insert_data((long) false, &uuid_tree, &key, &data);

                    data = key;
                    for (i = 1; i < num_tokens; i++) {
                        key.dptr = tokens[i];
                        key.dsize = strlen(key.dptr);
                        sym_$insert_data((long) false, &name_tree, &key, &data);
                    }
                }
            }
        }
    }
}

#if defined(apollo) && defined(pre_sr10)

#define UUIDNAME_FILE "/sys/ncs/uuidname.txt"

#else

#ifdef UNIX
#  define UUIDNAME_FILE "/etc/ncs/uuidname.txt"
#endif
#ifdef vms
#  define UUIDNAME_FILE "ncs$exe:uuidname.txt"
#endif
#ifdef MSDOS
#  define UUIDNAME_FILE "c:\\ncs\\uuidname.txt"
#endif

#endif

private void init_uuid_naming ( )
{
    FILE            *fp;
#ifdef UNIX
    char            pathname[1024];
    struct passwd   *pwd_p;
#endif

    fp = openfile(UUIDNAME_FILE);

    if (fp != NULL) {
        process_uuid_names(fp);
        fclose(fp);
    }

#ifdef UNIX
    pwd_p = getpwuid(getuid());
    if (pwd_p != NULL) {
        sprintf(pathname, "%s/uuidname.txt", pwd_p->pw_dir);
        fp = openfile(pathname);
    }

    if (fp != NULL) {
        process_uuid_names(fp);
        fclose(fp);
    }
#endif

    uuid_naming_inited = true;
}

/*
** Public Interface
*/

public void uname_$uuid_from_name ( name, uuid, st )
    char        *name;
    uuid_$t     *uuid;
    status_$t   *st;
{
    if (!uuid_naming_inited)
        init_uuid_naming();

    st->all = status_$ok;

    if (!name_lookup(name, uuid)) {
        uuid_$decode((ndr_$char *) name, uuid, st);
    }
}

public void uname_$uuid_to_name ( uuid, name, maxname_len )
    uuid_$t     *uuid;
    char        *name;
    long        maxname_len;
{
    char            *p;
    uuid_$string_t  uuid_string;

    if (!uuid_naming_inited)
        init_uuid_naming();

    if (!uuid_lookup(uuid, &p)) {
        p = (char *) uuid_string;
        uuid_$encode(uuid, uuid_string);
    }

    strncpy(name, p, (int) maxname_len);
    if (strlen(p) > maxname_len) {
        name[maxname_len-1] = '\0';
    }
}

