/* $Id: acapsieve.h,v 1.1.1.1 2002-10-13 18:01:16 ghudson Exp $ */

#ifndef ACAPSIEVE_H_
#define ACAPSIEVE_H_

#include <stdio.h>

#include "acap.h"

int acapsieve_init(void);

typedef struct acapsieve_handle_s acapsieve_handle_t;

/*
 * get a handle.  all returns (including NULL) are valid!
 * may be a noop for non-acap-enabled installs.
 */

typedef struct acapsieve_data_s {

    char *name;

    int datalen;
    char *data;

} acapsieve_data_t;

acapsieve_handle_t *acapsieve_connect(const char *user,
				      const char *acapserver, 
				      sasl_callback_t *cb);
acapsieve_handle_t *acapsieve_convert(const char *user, acap_conn_t *conn);

void acapsieve_disconnect(acapsieve_handle_t *conn);

acap_conn_t *acapsieve_release_handle(acapsieve_handle_t *handle);

/*
 * Get the data for an acap entry
 * 
 */
int acapsieve_get(acapsieve_handle_t *AC,
		  char *name,
		  char **data);

char *getsievename(char *filename);

/*
 * Put a script on the ACAP server
 */

int acapsieve_put_file(acapsieve_handle_t *AC,
		       char *filepath);

int acapsieve_put(acapsieve_handle_t *AC,
		  acapsieve_data_t *data);

int acapsieve_put_simple(acapsieve_handle_t *AC,
			 char *name,
			 char *data, 
			 int datalen);

/*
 * Set the script 'name' as the active script
 */

int acapsieve_activate(acapsieve_handle_t *AC,
		       char *name);

/*
 * Delete the script with 'name'
 */

int acapsieve_delete(acapsieve_handle_t *AC,
		     char *name);

/*
 * List the scripts that exist
 */

typedef void *acapsieve_list_cb_t(char *name, int isactive, void *rock);

int acapsieve_list(acapsieve_handle_t *AC,
		   acapsieve_list_cb_t *cb,
		   void *rock);

#endif /* ACAPSIEVE_H_ */
