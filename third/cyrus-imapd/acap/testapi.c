/* testapi.c -- simple test client for ACAP servers
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

/* $Id: testapi.c,v 1.1.1.1 2002-10-13 18:01:16 ghudson Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "acap.h"

void fatal(char *s, int code)
{
    fprintf(stderr, "fatal error: %s\n", s);
    exit(code);
}

static void myacap_entry(acap_entry_t *entry, void *rock)
{
    skipnode *node;
    acap_attribute_t *attr;

    printf("\tentry = %s\n", entry->name);
    attr = sfirst(entry->attrs, &node);
    while (attr) {
	printf("\t\t%s = %s\n", attr->name, attr->v->data);

	attr = snext(&node);
    }
}

static void myacap_modtime(char *modtime, void *rock)
{
    printf("\tmodtime = %s\n", modtime);
}

static struct acap_search_callback myacap_search_cb = {
    &myacap_entry, &myacap_modtime
};

static struct acap_requested myacap_request = {
    1, { "*" }
};

void list_dataset(acap_conn_t *conn, char *dataset)
{
    acap_cmd_t *cmd;
    int r;

    printf("listing '%s'...\n", dataset);
    r = acap_search_dataset(conn, dataset, "ALL", 1,
			    &myacap_request, NULL,
			    NULL,
			    &myacap_search_cb,
			    NULL, NULL, NULL, &cmd);
    if (r != ACAP_OK) {
	printf("acap_search_dataset() = %d\n", r);
	fatal("acap_search_dataset() failed", 3);
    }

    r = acap_process_on_command(conn, cmd, NULL);
    if (r != ACAP_OK) {
	printf("acap_process_on_command() = %d\n", r);
	fatal("acap_process_on_command() failed", 3);
    }

    printf("done\n");
}

int main(int argc, char *argv[])
{
    int r;
    acap_conn_t *conn;

    r = acap_init();
    if (r != ACAP_OK) {
	fatal("acap_init failed()", 1);
    }

    r = sasl_client_init(NULL);
    if (r != SASL_OK) {
	fatal("sasl_client_init() failed", 1);
    }

    r = acap_conn_connect("acap://leg@acap.andrew.cmu.edu/", &conn);
    if (r != SASL_OK) {
	printf("acap_conn_connect() returned %d\n", r);
	fatal("acap_conn_connect() failed", 1);
    }

    list_dataset(conn, "/");

    r = acap_conn_close(conn);
    if (r != SASL_OK) {
	printf("acap_conn_close() returned %d\n", r);
	fatal("acap_conn_close() failed", 1);
    }

    return 0;
}
