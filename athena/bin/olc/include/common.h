/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions used by "common.a".
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/common.h,v $
 *	$Id: common.h,v 1.2 1990-05-25 15:06:11 vanharen Exp $
 *	$Author: vanharen $
 */

#include <mit-copyright.h>

/*
 * This file must use the old-C compatible definitions, because it
 * will be included by C code as well as ANSI-C or C++.
 */

#ifndef __olc_common_h
#define __olc_common_h

#include <olc/lang.h>

#if is_cplusplus
extern "C" {
#endif

    extern char * cap OPrototype ((char *));
    extern int isnumber OPrototype ((char *));
    extern void make_temp_name OPrototype ((char *));
#if 0
    /*
     * this is in libcommon.a, but MAX_* are defined only for the
     * parser!
     */
    extern ERRCODE parse_command_line OPrototype ((char *command_line,
						   char arguments[MAX_ARGS]
						   [MAX_ARG_LENGTH]));
#endif
    extern int read_dbinfo OPrototype ((int, DBINFO *));
    extern ERRCODE read_int_from_fd OPrototype ((int, int *));
    extern ERRCODE read_response OPrototype ((int, RESPONSE *));
    extern char * read_text_from_fd OPrototype ((int));
    extern ERRCODE read_text_into_file OPrototype ((int, char *));
    extern int send_dbinfo OPrototype ((int, DBINFO *));
    extern ERRCODE send_response OPrototype ((int, RESPONSE));
    extern int sread OPrototype ((int, char *, int));
    extern int swrite OPrototype ((int, char *, int));
    extern void time_now OPrototype ((char *));
    extern char * format_time OPrototype ((struct tm *));
    extern void uncase OPrototype ((char *));
    extern ERRCODE write_file_to_fd OPrototype ((int, char *));
    extern ERRCODE write_int_to_fd OPrototype ((int, int));
    extern ERRCODE write_text_to_fd OPrototype ((int, char *));

#if is_cplusplus
};
#endif

#endif /* __olc_common_h */
