/* $Id: acapsieve.c,v 1.1.1.1 2002-10-13 18:01:16 ghudson Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <assert.h>

#include <acap.h>
#include "acapsieve.h"


struct acapsieve_handle_s {
    acap_conn_t *conn;
    char *username;
};

void acapsieve_disconnect(acapsieve_handle_t *handle)
{
    acap_conn_close(handle->conn);
    free(handle->username);
    free(handle);
}

acap_conn_t *acapsieve_release_handle(acapsieve_handle_t *handle)
{
    free(handle->username);
    free(handle);
    return NULL;
}

acapsieve_handle_t *acapsieve_connect(const char *user, 
				      const char *acapserver, 
				      sasl_callback_t *cb)
{
    int r;
    char str[2048];
    acapsieve_handle_t *myconn = NULL;
    
    assert(user && acapserver && cb);

    myconn = (acapsieve_handle_t *) malloc(sizeof(acapsieve_handle_t));
    if (!myconn) return NULL;
    myconn->conn = NULL;
    
    if (user == NULL) {
	return NULL;
    }
    myconn->username = strdup(user);

    snprintf(str, sizeof(str), "acap://%s@%s/", user, acapserver);

    r = acap_conn_connect(str, cb, &(myconn->conn));
    if (r != ACAP_OK) {
	/* acap_conn_close(cached_conn->conn); xxx */
	myconn->conn = NULL;
	return myconn;
    }

    return myconn;
}

acapsieve_handle_t *acapsieve_convert(const char *user, acap_conn_t *conn)
{
    acapsieve_handle_t *myconn = NULL;

    assert(conn && user);
    myconn = (acapsieve_handle_t *) malloc(sizeof(acapsieve_handle_t));
    if (!myconn) return NULL;
    myconn->conn = conn;
    myconn->username = strdup(user);

    return myconn;
}

typedef struct listcb_struct_s {
  acapsieve_list_cb_t *cb;
  void *rock;
} listcb_struct_t;

static void myacap_entry(acap_entry_t *entry, void *rock)
{
    listcb_struct_t *s = (listcb_struct_t *) rock;
    acapsieve_list_cb_t *cb = s->cb;
    skipnode *node;
    acap_attribute_t *attr;

    attr = sfirst(entry->attrs, &node);
    while (attr) {
	cb(attr->name, 0, s->rock);

	attr = snext(&node);
    }
}

static void myacap_modtime(char *modtime, void *rock)
{

}

static struct acap_search_callback myacap_search_cb = {
    &myacap_entry, &myacap_modtime
};

static struct acap_requested myacap_request = {
    1, {{ "email.account.sieve.*", ATTRIBUTE }}
};

int acapsieve_list(acapsieve_handle_t *AC,
		   acapsieve_list_cb_t *cb,
		   void *rock)
{
    acap_cmd_t *cmd;
    int r;
    char *search_crit;
    listcb_struct_t cbst;
    char dset[1024];

    if (AC == NULL) return 0;

    if (AC->conn == NULL) {
	return -1;
    }

    /* create search criteria */
    search_crit = (char *) malloc(30);
    if (search_crit==NULL) return ACAP_NOMEM;

    sprintf(search_crit,"ALL");

    /* fill in struct passed to callback */
    cbst.cb = cb;
    cbst.rock = rock;

    /* make dataset string */
    snprintf(dset, sizeof(dset)-1,"/sieve/%s/",AC->username);

    r = acap_search_dataset(AC->conn, 
			    dset,
			    search_crit, 1,
			    &myacap_request, NULL,
			    NULL,
			    &myacap_search_cb,
			    NULL, NULL, 
			    &cbst,
			    &cmd);

    if (r != ACAP_OK) {
	return r;
    }

    r = acap_process_on_command(AC->conn, cmd, NULL);    
    if (r != ACAP_OK) {
	return r;
    }
    
    return ACAP_OK;
}


int acapsieve_put(acapsieve_handle_t *AC,
		  acapsieve_data_t *data)
{
    return acapsieve_put_simple(AC, data->name, data->data, data->datalen);
}

static int add_attr(skiplist *sl, char *name, char *value)
{
    acap_attribute_t *tmpattr;

    tmpattr = acap_attribute_new_simple (name, value);
    if (tmpattr) sinsert(sl, tmpattr);

    return 0;
}

int acapsieve_put_simple(acapsieve_handle_t *AC,
			 char *name,
			 char *data, 
			 int datalen)
{
    int result;
    acap_entry_t *newentry;
    acap_result_t acapres;

    acap_cmd_t *cmd;
    char fullname[1024];
    char attrname[1024];

    if (AC == NULL) return 0;

    if (AC->conn == NULL) {
	return -1;
    }

    /* create the new entry */
    snprintf(fullname,sizeof(fullname)-1,"/sieve/%s/default",AC->username);

    newentry = acap_entry_new(fullname);
    if (newentry == NULL) return ACAP_NOMEM;

    /* make and insert all our initial attributes */
    snprintf(attrname,sizeof(attrname)-1,"email.account.sieve.%s",name);
    add_attr(newentry->attrs, attrname, data);

    /* create the cmd; if it's the first time through, we ACAP_STORE_INITIAL */
    result = acap_store_entry(AC->conn,
			      newentry,
			      NULL,
			      NULL,
			      0,
			      &cmd);
    if (result == ACAP_OK) {
	result = acap_process_on_command(AC->conn, cmd, &acapres);
	if (result == ACAP_NO_CONNECTION) {
	    result = -1;
	} else if (acapres != ACAP_RESULT_OK) {
	    /* this is a likely but not certain error */
	    result = -1;
	}
    }

    return result;
}

char *getsievename(char *filename)
{
  char *ret, *ptr;

  ret=(char *) malloc( strlen(filename) + 2);
  if (!ret) return NULL;

  /* just take the basename of the file */
  ptr = strrchr(filename, '/');
  if (ptr == NULL) {
      ptr = filename;
  } else {
      ptr++;
  }

  strcpy(ret, ptr);

  if ( strcmp( ret + strlen(ret) - 7,".script")==0)
  {
    ret[ strlen(ret) - 7] = '\0';
  }

  return ret;
}

int acapsieve_put_file(acapsieve_handle_t *AC,
		       char *filename)
{
    FILE *stream;
    struct stat filestats;  /* returned by stat */
    int size;     /* size of the file */
    int result;
    char *sievename;
    char *sievedata;

    sievename=getsievename(filename);

    result=stat(filename,&filestats);

    if (result!=0)
    {
	perror("stat");
	return -1;
    }

    size=filestats.st_size;

    stream=fopen(filename, "r");

    if (stream==NULL)
    {
	printf("Couldn't open file\n");
	return -1;
    }

    sievedata = malloc(size+1);
    if (!sievedata) return ACAP_NOMEM;

    fread(sievedata, 1, size, stream);

    return acapsieve_put_simple(AC,
				sievename,
				sievedata, 
				size);
}

struct snarf {
    char *attr;
    char *data;
};

/* callback to snarf a simple attribute */
void snarfit(acap_entry_t *entry, void *rock)
{
    struct snarf *s = (struct snarf *) rock;

    assert(s && s->attr);
    s->data = strdup(acap_entry_getattr_simple(entry, s->attr));
}

struct acap_search_callback myacap_snarf = {
    &snarfit, NULL
};

/* return the current user's script.
   'script' is malloc'd, caller frees. NULL if no active script. */
int acapsieve_getactive(acapsieve_handle_t *AC, char **script)
{
    char dset[1024];
    char *attrname = NULL;
    struct snarf s;
    struct acap_requested myacap_request;
    char *activename;
    acap_cmd_t *cmd;
    int r = 0;

    s.attr = attrname;
    s.data = NULL;

    myacap_request.n_attrs = 1;
    myacap_request.attrs[0].attrname = attrname;
    myacap_request.attrs[0].ret = VALUE;
    
    snprintf(dset, sizeof(dset), "/sieve/%s/", AC->username);

    /* first, find the name of the active script */
    attrname = "email.sieve.script";
    s.data = NULL;
    r = acap_search_dataset(AC->conn,  dset, 
			    "EQUAL \"entry\" \"i;octet\" \"default\"", 1,
			    &myacap_request, NULL, NULL,
			    &myacap_snarf, NULL, NULL, &s, &cmd);
    if (r) return r;
    r = acap_process_on_command(AC->conn, cmd, NULL);
    if (r) return r;
    activename = s.data;

    /* now, find the active script */
    if (activename) {
	attrname = activename;
	s.data = NULL;
	r = acap_search_dataset(AC->conn,  dset, 
				"EQUAL \"entry\" \"i;octet\" \"default\"", 1,
				&myacap_request, NULL, NULL,
				&myacap_snarf, NULL, NULL, &s, &cmd);
	if (!r) r = acap_process_on_command(AC->conn, cmd, NULL);
	free(activename);

	if (r) return r;
	else {
	    *script = s.data;
	    return 0;
	}
    } else {
	*script = NULL;
	return 0;
    }
}

int acapsieve_activate(acapsieve_handle_t *AC,
		       char *name)
{
    int result;
    acap_entry_t *newentry;
    acap_result_t acapres;

    acap_cmd_t *cmd;
    char fullname[1024];
    char attrvalue[1024];

    if (AC == NULL) return 0;

    if (AC->conn == NULL) {
	return -1;
    }

    /* create the new entry */

    snprintf(fullname,sizeof(fullname)-1, "/sieve/%s/default",AC->username);

    newentry = acap_entry_new(fullname);
    if (newentry == NULL) return ACAP_NOMEM;

    /* make and insert all our initial attributes */
    snprintf(attrvalue,sizeof(attrvalue)-1,"email.account.sieve.%s",name);
    add_attr(newentry->attrs, "email.sieve.script", attrvalue);

    result = acap_store_entry(AC->conn,
			      newentry,
			      NULL,
			      NULL,
			      0,
			      &cmd);
    if (result == ACAP_OK) {
	result = acap_process_on_command(AC->conn, cmd, &acapres);
	if (result == ACAP_NO_CONNECTION) {
	    result = -1;
	} else if (acapres != ACAP_RESULT_OK) {
	    /* this is a likely but not certain error */
	    result = -1;
	}
    }

    return result;
}

int acapsieve_delete(acapsieve_handle_t *AC,
		     char *name)
{
    int result;
    acap_entry_t *newentry;
    acap_result_t acapres;

    acap_cmd_t *cmd;
    char fullname[1024];
    char attrname[1024];

    if (AC == NULL) return 0;

    if (AC->conn == NULL) {
	return -1;
    }

    /* create the new entry */

    sprintf(fullname, "/sieve/%s/default",AC->username);

    newentry = acap_entry_new(fullname);
    if (newentry == NULL) return ACAP_NOMEM;

    snprintf(attrname,sizeof(attrname)-1,"email.account.sieve.%s",name);

    /* create the cmd; if it's the first time through, we ACAP_STORE_INITIAL */
    result = acap_delete_attribute(AC->conn,
				   fullname,
				   attrname,
				   NULL,
				   NULL,
				   &cmd);

    /* xxx what about active script??? */

    if (result == ACAP_OK) {
	result = acap_process_on_command(AC->conn, cmd, &acapres);
	if (result == ACAP_NO_CONNECTION) {
	    result = -1;
	} else if (acapres != ACAP_RESULT_OK) {
	    /* this is a likely but not certain error */
	    result = -1;
	}
    }

    return result;
}

static void myacap_entry_get(acap_entry_t *entry, void *rock)
{
    char **data = (char **) rock;
    skipnode *node;
    acap_attribute_t *attr;

    attr = sfirst(entry->attrs, &node);
    if ((attr) && (attr->v)) {

	*data = malloc(attr->v->len+1);
	if (!*data) return;
	
	memcpy(*data, attr->v->data, attr->v->len);
	(*data)[attr->v->len] = '\0';

	attr = snext(&node);
    } else {
	*data = NULL;
    }
}

static struct acap_search_callback myacap_search_get_cb = {
    &myacap_entry_get, NULL
};


int acapsieve_get(acapsieve_handle_t *AC,
		  char *name,
		  char **data)
{
    struct acap_requested myacap_req;
    int r;
    acap_cmd_t *cmd;
    char dset[1024];

    myacap_req.n_attrs = 1;
    myacap_req.attrs[0].attrname = malloc(strlen(name)+30);
    if (!myacap_req.attrs[0].attrname) return ACAP_NOMEM;

    sprintf(myacap_req.attrs[0].attrname, "email.account.sieve.%s",name);
    myacap_req.attrs[0].ret = 2; /* just the value */

    snprintf(dset,sizeof(dset)-1,"/sieve/%s/",AC->username);

    r = acap_search_dataset(AC->conn, 
			    dset,
			    "ALL", 1,
			    &myacap_req, NULL,
			    NULL,
			    &myacap_search_get_cb,
			    NULL, NULL, 
			    data,
			    &cmd);

    if (r != ACAP_OK) {
	return r;
    }

    r = acap_process_on_command(AC->conn, cmd, NULL);    
    if (r != ACAP_OK) {
	return r;
    }
    
    return ACAP_OK;    
}
