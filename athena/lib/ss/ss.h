/*
 * Copyright 1987, 1988 by MIT Student Information Processing Board
 *
 * For copyright information, see mit-sipb-copyright.h.
 */

#ifndef SS__H
#define SS__H 

#include <ss/mit-sipb-copyright.h>
#include <ss/ss_err.h>
#include <errno.h>

typedef struct _ss_request_entry {
    const char *const *command_names;
    void (*function)(int, const char *const *, int, void *);
    const char *info_string;	/* NULL */
    int flags;			/* 0 */
} ss_request_entry;

typedef struct _ss_request_table {
    int version;
    ss_request_entry *requests;
} ss_request_table;

#define SS_RQT_TBL_V2	2

typedef struct _ss_rp_options {	/* DEFAULT VALUES */
    int version;		/* SS_RP_V1 */
    void (*unknown)(int, const char *const *, int, void *);
    int allow_suspend;
    int catch_int;
} ss_rp_options;

#define SS_RP_V1 1

#define SS_OPT_DONT_LIST	0x0001
#define SS_OPT_DONT_SUMMARIZE	0x0002

void ss_help(int, const char *const *, int, void *);
char *ss_current_request();
char *ss_name();
void ss_error(int, long, char const *, ...);
void ss_perror(int, long, char const *);
void ss_abort_subsystem();
extern ss_request_table ss_std_requests;
#endif /* SS__H */
