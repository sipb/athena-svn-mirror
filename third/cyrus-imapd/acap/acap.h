/* acap.h -- connection level ACAP API
 *
 * Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any other legal
 *    details, please contact  
 *	Office of Technology Transfer
 *	Carnegie Mellon University
 *	5000 Forbes Avenue
 *	Pittsburgh, PA  15213-3890
 *	(412) 268-4387, fax: (412) 268-7395
 *	tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: acap.h,v 1.1.1.1 2002-10-13 18:01:16 ghudson Exp $ */

#ifndef ACAP_H
#define ACAP_H

#include <sasl/sasl.h>

#include "skip-list.h"

/* -------- error codes -------- */
#define ACAP_OK (0)

#include <acap_err.h>

/* ------- initialization -------- */

/* be sure to call srand() before acap_init(); we use
   randomized data structures.  (eventually, use our own rand() to avoid
   clashes?) */
int acap_init(void);

/* -------- connections -------- */
struct acap_conn_s;
typedef struct acap_conn_s acap_conn_t;

/* application should setup callbacks for AUTHNAME and password
   if required; connect will set a connection callback for the user
   listed in the URL */
int acap_conn_connect(const char *url, 
		      sasl_callback_t *cb,
		      acap_conn_t **conn);
int acap_conn_close(acap_conn_t *conn);

/* it's only valid to use this socket to select() after
   acap_process with ACAP_PROCESS_NOBLOCK set returns ACAP_WOULD_BLOCK.
   otherwise, hands off. */
int acap_conn_get_sock(acap_conn_t *conn);

int acap_get_sasl_error(acap_conn_t *conn);

/* -------- acap data ---------- */
typedef struct acap_value_s {
    unsigned len;
    struct acap_value_s *next;
    char data[1];
} acap_value_t;

typedef struct {
    char *name;

    enum {
	ACAP_VALUE_SINGLE,
	ACAP_VALUE_LIST,
	ACAP_VALUE_DEFAULT,
	ACAP_VALUE_NIL
    } t;

    /* value */
    acap_value_t *v;

    /* should have space for metadata here */
} acap_attribute_t;

struct acap_entry_s {
    char *name; /* must be >= 1024 chars */
    int refcount;

    /* contains a list of attributes, ordered by attrname */
    skiplist *attrs;
};

typedef struct acap_entry_s acap_entry_t;

void acap_entry_free(acap_entry_t *e);
acap_entry_t *acap_entry_copy(acap_entry_t *e);
acap_entry_t *acap_entry_new(char *name);

char *acap_entry_getname(acap_entry_t *e);
acap_value_t *acap_entry_getattr(acap_entry_t *e, char *attrname);
char *acap_entry_getattr_simple(acap_entry_t *e, char *attrname);

acap_attribute_t *acap_attribute_new(char *attrname);
acap_attribute_t *acap_attribute_new_simple(char *attrname, char *attrvalue);
void acap_attribute_free(acap_attribute_t *a);

/* ------- command ----------- */
typedef enum {
    ACAP_RESULT_NOTDONE,
    ACAP_RESULT_OK,
    ACAP_RESULT_NO,
    ACAP_RESULT_BAD
} acap_result_t;

typedef struct acap_cmd_s acap_cmd_t;

typedef void acap_cb_completion_t(acap_result_t res, void *rock);

int acap_conn_noop(acap_conn_t *conn, acap_cb_completion_t *cmd_cb,
		   void *rock, acap_cmd_t **ret);

/* ------ searching -------- */

struct acap_search_callback {
    void (*entry_data)(acap_entry_t *entry, void *rock);
    void (*modtime)(char *modtime, void *rock);
};

struct acap_context_callback {
    void (*addto)(acap_entry_t *entry,
		  unsigned position,
		  void * rock);
    void (*removefrom)(acap_entry_t *entry,
		       unsigned position,
		       void *rock);
    void (*change)(acap_entry_t *entry,
		   unsigned old_position,
		   unsigned new_position,
		   void *rock);
    void (*modtime)(char *time,
		    void *rock);
};

typedef struct acap_context_s acap_context_t;

/* if search_cb == NULL, no data is returned but a context can be created */
/* if context_ptr != NULL, a context is created
      if context_cb != NULL, a notification context is created
      if sort_order != NULL, a enumerated context is created
*/

struct acap_request {
   char *attrname;
   enum {
      ATTRIBUTE = 0x01,
      VALUE = 0x02,
      SIZE = 0x04
   } ret;
};


struct acap_requested {
    int n_attrs;
    struct acap_request attrs[1];
};

struct acap_sort {
    char *attrname;
    char *comparator;
    struct acap_sort *next;
};

int acap_search_dataset(acap_conn_t *conn,
			const char *dataset, 
			char *spec,
			int depth,
			const struct acap_requested *ret_attrs,
			const struct acap_sort *sort_order,
			const acap_cb_completion_t *cmd_cb,
			const struct acap_search_callback *search_cb,
			acap_context_t **context_ptr,
			const struct acap_context_callback *context_cb,
			void *rock,
			acap_cmd_t **ret);

int acap_search_context(acap_conn_t *conn,
			acap_context_t *context, 
			char *spec,
			struct acap_requested *ret_attrs,
			struct acap_sort *sort_order,
			acap_cb_completion_t *cmd_cb,
			struct acap_search_callback *search_cb,
			void *rock,
			acap_cmd_t **ret);

int acap_updatecontext(acap_conn_t *conn,
		       acap_context_t *context,
		       acap_cb_completion_t *cmd_cb,
		       void *rock,
		       acap_cmd_t **ret);

/* --------- storing ----------- */

#define ACAP_STORE_INITIAL 0x01
#define ACAP_STORE_FORCE   0x02

int acap_store_entry(acap_conn_t *conn,
		     acap_entry_t *entry,
		     acap_cb_completion_t *cmd_cb,
		     void *rock,
		     int flags,
		     acap_cmd_t **ret);

int acap_store_attribute(acap_conn_t *conn,
			 const char *entry,
			 acap_attribute_t *attr,
			 char *unchangedsince,
			 acap_cb_completion_t *cmd_cb,
			 void *rock,
			 acap_cmd_t **ret);

/* -------- deleting --------- */

int acap_delete_entry_name(acap_conn_t *conn,
			   const char *name,
			   acap_cb_completion_t *cmd_cb,
			   void *rock,
			   acap_cmd_t **ret);

int acap_delete_entry(acap_conn_t *conn,
		      acap_entry_t *entry);

int acap_delete_attribute(acap_conn_t *conn,
			  char *entryname,
			  char *attrname,
			  acap_cb_completion_t *cmd_cb,
			  void *rock,
			  acap_cmd_t **ret);

/* -------- processing -------- */
#define ACAP_PROCESS_NOBLOCK 1

int acap_process_on_command(acap_conn_t *conn, acap_cmd_t *cmd,
			    acap_result_t *res);

int acap_process_outstanding(acap_conn_t *conn);

int acap_process_line(acap_conn_t *conn, int flag);

#endif /* ACAP_H */
