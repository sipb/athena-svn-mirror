/* $Id: globalmessage.h,v 1.8 1998-12-03 19:38:30 ghudson Exp $ */

/* Copyright 1988, 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#ifndef __GLOBALMESSAGE_H__
#define __GLOBALMESSAGE_H__

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "globalmessage_err.h"
	/* Function return code */
typedef int Code_t;

#define GMS_NAME_CLASS "globalmessage"
#define GMS_NAME_TYPE "sloc"

#define GMS_SERV_NAME "globalmessage"
#define GMS_SERV_PROTO "udp"

#define GMS_MESSAGE_NAME "/usr/tmp/.messages"

#define GMS_USERFILE_NAME "/.message_times"
#define GMS_USERFILE_NAME_LEN (sizeof(GMS_USERFILE_NAME)-1)

#define GMS_VERSION_STRING "GMS:0"
#define GMS_VERSION_STRING_LEN (sizeof(GMS_VERSION_STRING)-1)

#define GMS_MAX_MESSAGE_LEN 2048

#define GMS_TIMEOUT_SEC 5
#define GMS_TIMEOUT_USEC 0

/* log_10(2^32) >= 10, plus a NL, plus a NUL */
#define GMS_TIMESTAMP_LEN 12

#define BFSZ 1024

void view_message_by_zephyr(char *message);
void view_message_by_tty(char *message);
Code_t get_servername(char ***ret_name);
Code_t get_message_from_server(char **ret_message, int *ret_message_size,
			       char *server);
Code_t get_a_message(char **buf);
Code_t check_viewable(char *message, int checktime, int updateuser);
Code_t gethost_error(void);
Code_t hesiod_error(void);
Code_t read_to_memory(char **ret_block, int *ret_size, int filedesc);
Code_t put_fallback_file(char *message_data, int message_size,
			 char *message_filename);
Code_t get_fallback_file(char **ret_data, int *ret_size,
			 char *message_filename);

#endif /* __GLOBALMESSAGE_H__ */
