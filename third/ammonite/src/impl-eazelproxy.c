
/* $Id: impl-eazelproxy.c,v 1.1.1.1 2001-01-16 15:26:36 ghudson Exp $
 * 
 * Copyright (C) 2000 Eazel, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author:  Michael Fleming <mfleming@eazel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "impl-eazelproxy.h"

#include <orb/orbit.h>
#include <liboaf/liboaf.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "authenticate.h"
#include "log.h"
#include "utils.h"
#include "http-connection.h"
#include "digest.h"
#include "request.h"
#include "proxy.h"
#include "sock.h"
#include "util-gconf.h"
#include "eazelproxy-change-password.h"

/*******************************************************************
 * Types, Global Variables
 *******************************************************************/

#define AUTHN_PROMPT_RETRIES 2		/* three prompts total */

#define KEY_GCONF_NAUTILUS_USER_LEVEL "/apps/nautilus/user_level"

#define KEY_GCONF_DEFAULT_SERVICES_URI "/apps/eazel-trilobite/services-host"
#define KEY_GCONF_DEFAULT_LOGIN_PATH "/apps/eazel-trilobite/default-services-login-path"
#define KEY_GCONF_DEFAULT_USER "/apps/eazel-trilobite/default-services-user"

#define DEFAULT_SERVICES_URI "http://services.eazel.com/"
#define DEFAULT_SERVICES_SSL_URI "http://services.eazel.com:443/"

#define DEFAULT_LOGIN_PATH "account/login/post"

#define ANON_USER "anonymous"

/* The default eazel-proxy user */
static User *gl_user_default 			= NULL;
/* The list of currently active users */
static GList *gl_user_list 			= NULL;
/* The list of UserListener's */
static GList *gl_user_listener_list 		= NULL;

#ifdef ENABLE_REAUTHN
/* The PID of eazel-proxy-util -d, for the default reauthn dialog */
static pid_t gl_util_pid = 0;
#endif

/** The following variables are driven through gconf **/

/* The current Nautilus User Level setting */
/* exported for authenticate_add_default_headers:eazelproxy-authn.c */
gchar * gl_user_level;
gboolean gl_user_level_have 		= FALSE;

/* Default ammonite settings */
static gchar * gl_default_services_uri		= NULL;
static gchar * gl_default_login_path		= NULL;
static gchar * gl_default_service_username	= NULL;

struct User {
	EazelProxy_User user;
	GList *callback_list;
	DigestState *	p_digest;
	guint32	authn_count;		/* Increments when user is reauthned; so that 
					 * requests know when the user was reauthned
					 * while the request was in-progress
					 */
	EazelProxy_UserPrompter	prompter;	/* Present if user was created through
						 * a _prompt_ method
						 */
	guint32 prompt_count;		/* Number of times user has been prompted */
	gboolean prompt_username_specified; 	/* TRUE if a call to prompt_authenticate_user */
						/* originally specified a username */ 
};



/*******************************************************************
 * gconf notification stuff
 * 
 *******************************************************************/

static void /* UtilGConfCb */
watch_gconf_user_level_cb (
	const UtilGConfWatchVariable *watched, 
	const GConfValue *new_value
) {
	if (NULL == new_value) {
		gl_user_level_have = FALSE;
		log ("user level unset", gconf_value_get_string (new_value));
	} else if (GCONF_VALUE_STRING == new_value->type ) {
		gl_user_level_have = TRUE;
		log ("user level changed to '%s'", gconf_value_get_string (new_value));
	}
}


static void /* UtilGConfCb */
watch_gconf_services_uri_cb (
	const UtilGConfWatchVariable *watched, 
	const GConfValue *new_value
) {
	/* GConf variable should be in host:port format--convert to url */
	char *old;

	old = gl_default_services_uri;

	/* If we're in SSL mode, default to port 443 */
	if (config.use_ssl && NULL == strchr (gl_default_services_uri, ':' )) {
		gl_default_services_uri = g_strdup_printf("http://%s:443/", gl_default_services_uri);
	} else {
		gl_default_services_uri = g_strdup_printf("http://%s/", gl_default_services_uri);
	}
	g_free (old);
	
	if ( ! util_validate_url (gl_default_services_uri)) {
		log ("Invalid 'services-host' GConf sessing setting (must be host:(port)?)");

		g_free (gl_default_services_uri);
		gl_default_services_uri = NULL;
	} else {
		log ("default services URI set to %s", gl_default_services_uri);
	}
}

static void
get_defaults_from_gconf ()
{
	static const UtilGConfWatchVariable to_watch[] = {
		/* Nautilus user level -- so it can be appended to the proxied request headers */
		{KEY_GCONF_NAUTILUS_USER_LEVEL, GCONF_VALUE_STRING, {(gchar **)&gl_user_level}, watch_gconf_user_level_cb},
		/* Default services URI--where incoming HTTP requests should be redirected  */
		{KEY_GCONF_DEFAULT_SERVICES_URI, GCONF_VALUE_STRING, {(gchar **)&gl_default_services_uri}, watch_gconf_services_uri_cb},
		/* Default login path -- path below services URI where authn requests should be pointed */
		{KEY_GCONF_DEFAULT_LOGIN_PATH, GCONF_VALUE_STRING, {(gchar **)&gl_default_login_path}, NULL},
		/* Default eazel service user */
		{KEY_GCONF_DEFAULT_USER, GCONF_VALUE_STRING, {(gchar **)&gl_default_service_username}, NULL}
	};

	/* Nautilus User Level */
	util_gconf_watch_variable (to_watch + 0);

	/* Default Services URI */
	/*
	 * Override priority:
	 * 1. Specified to UserControl::authenticate_user
	 * 2. Specified on the command line
	 * 3. Specified in gconf
	 * 4. #define'd as default
	 */

	if (config.target_path) {
		gl_default_services_uri = g_strdup (config.target_path);

		if ( ! util_validate_url (gl_default_services_uri)) {
			log ("Invalid -t setting (must be URL)");

			g_free (gl_default_services_uri);
			gl_default_services_uri = NULL;
		}
		 
	} else {
		if (config.use_ssl) {
			gl_default_services_uri = g_strdup (DEFAULT_SERVICES_SSL_URI);
		} else {
			gl_default_services_uri = g_strdup (DEFAULT_SERVICES_URI);
		}
		util_gconf_watch_variable (to_watch + 1);
	}

	/* login path */
	gl_default_login_path = g_strdup (DEFAULT_LOGIN_PATH);
	util_gconf_watch_variable (to_watch + 2);

	/* Default Eazel Service User */
	util_gconf_watch_variable (to_watch + 3);

}




/*******************************************************************
 * EazelProxy methods
 *******************************************************************/

static EazelProxy_User *
impl_EazelProxy_UserControl_get_default_user (PortableServer_Servant servant, CORBA_Environment * ev);

static EazelProxy_UserList *
impl_EazelProxy_UserControl_get_active_users  (PortableServer_Servant servant, CORBA_Environment * ev);

static void
impl_EazelProxy_UserControl_prompt_authenticate_user (PortableServer_Servant servant, 
							const EazelProxy_AuthnInfo *authinfo,
							const CORBA_boolean is_default,
							const EazelProxy_UserPrompter prompter,
							const EazelProxy_AuthnCallback callback,
							CORBA_Environment * ev);

static void
impl_EazelProxy_UserControl_authenticate_user (
	PortableServer_Servant servant,
	const EazelProxy_AuthnInfo *authinfo,
	const CORBA_boolean is_default,
	const EazelProxy_AuthnCallback callback,
	CORBA_Environment * ev
);

static CORBA_boolean
impl_EazelProxy_UserControl_set_new_user_password (
	PortableServer_Servant servant,
	const EazelProxy_AuthnInfo *authinfo,
	const CORBA_char * new_password,
	const EazelProxy_AuthnCallback callback,
	CORBA_Environment * ev
);

static CORBA_char *
impl_EazelProxy_UserControl_get_authn_header_for_port (
	PortableServer_Servant servant,
	const CORBA_unsigned_short logout_port,
	const CORBA_char * path,
	const CORBA_char * method,
	CORBA_Environment * ev
);

static CORBA_boolean
impl_EazelProxy_UserControl_logout_user(PortableServer_Servant servant, 
					const CORBA_unsigned_short logout_port,
					CORBA_Environment * ev);

static void
impl_EazelProxy_UserControl_add_listener(PortableServer_Servant servant,
					 const EazelProxy_UserListener listener,
					 CORBA_Environment * ev);

static void
impl_EazelProxy_UserControl_remove_listener(PortableServer_Servant servant,
					    const EazelProxy_UserListener to_remove,
					    CORBA_Environment * ev);

/*******************************************************************
 * EazelProxy epv's
 *******************************************************************/

EazelProxy_UserControl gl_object_usercontrol = CORBA_OBJECT_NIL;

static PortableServer_ServantBase__epv base_epv = {
	NULL,
	NULL,
	NULL
};

static POA_EazelProxy_UserControl__epv impl_EazelProxy_UserControl_epv = {
	NULL,
	impl_EazelProxy_UserControl_get_default_user,
	impl_EazelProxy_UserControl_get_active_users,
	impl_EazelProxy_UserControl_prompt_authenticate_user,
	impl_EazelProxy_UserControl_authenticate_user,
	impl_EazelProxy_UserControl_set_new_user_password,
	impl_EazelProxy_UserControl_get_authn_header_for_port,
	impl_EazelProxy_UserControl_logout_user,
	impl_EazelProxy_UserControl_add_listener,
	impl_EazelProxy_UserControl_remove_listener
};

static POA_EazelProxy_UserControl__vepv impl_EazelProxy_UserControl_vepv = { &base_epv, &impl_EazelProxy_UserControl_epv };

static POA_EazelProxy_UserControl impl_EazelProxy_UserControl_servant = { NULL, &impl_EazelProxy_UserControl_vepv };


void
init_impl_eazelproxy (CORBA_ORB orb)
{
	PortableServer_POA poa;
	CORBA_Environment ev;
	OAF_RegistrationResult result;
	
	CORBA_exception_init (&ev);

	/* listen on important variables */
	get_defaults_from_gconf();

	/* register our class */
	POA_EazelProxy_UserControl__init(&impl_EazelProxy_UserControl_servant, &ev);
	poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
	PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(poa, &ev), &ev);

	CORBA_free (PortableServer_POA_activate_object(poa, &impl_EazelProxy_UserControl_servant, &ev));
  
	gl_object_usercontrol = PortableServer_POA_servant_to_reference(poa,
                                                   &impl_EazelProxy_UserControl_servant,
                                                   &ev);
	if (CORBA_Object_is_nil(gl_object_usercontrol, &ev)) {
	      log("Failed to get object reference for EazelProxy_UserControl");
	      exit (-1);
	}

	result = oaf_active_server_register(IID_EAZELPROXY, gl_object_usercontrol);
	if (result != OAF_REG_SUCCESS) {
		switch (result) {
			case OAF_REG_NOT_LISTED:
	          		log("OAF doesn't know about our IID; indicates broken installation; can't register; exiting");
			break;
	          
			case OAF_REG_ALREADY_ACTIVE:
	          		log("Another eazel-proxy already registered with OAF; exiting");
			break;

			case OAF_REG_ERROR:
			default:
				log("Unknown error registering eazel-proxy with OAF; exiting");
			break;
		}
		exit (-1);
	}

#ifdef ENABLE_REAUTHN
	/* Start up eazel-proxy-util process to display login dialog */
	/* ...only if we have an X display ... */
	/* note that g_getenv return is not strdup'd */

	if (NULL != g_getenv ("DISPLAY")) {
		char *argv[] = { INSTALL_PATH_BIN "/eazel-proxy-util", "--reauthn-listen", NULL};
		gl_util_pid = util_fork_exec ( argv[0], (char *const*)argv);
		if ( -1 == gl_util_pid ){
			gl_util_pid = 0;
		}
	}
#endif /* ENABLE_REAUTHN */

}

void
shutdown_impl_eazelproxy (void)
{
#ifdef ENABLE_REAUTHN
	/* kill child eazel-proxy-util */
	if ( 0 != gl_util_pid ) {
		int child_status;
		int err;
		err = kill (gl_util_pid, SIGTERM);
		g_assert (0 == err);
		waitpid (gl_util_pid, &child_status, 0);
	}
#endif /* ENABLE_REAUTHN */
}

/*******************************************************************
 * EazelProxy::UserControl utilities
 *******************************************************************/

static EazelProxy_UserList *
EazelProxy_UserList_make_from_glist (GList *list)
{ 
	CORBA_unsigned_long i;
	GList *list_pos;
	EazelProxy_UserList *ret;

	ret = EazelProxy_UserList__alloc();

	if ( NULL == list ) {
		ret->_length = 0;
		ret->_buffer = NULL;
	} else {
		ret->_length = g_list_length (list);
		ret->_buffer = CORBA_sequence_EazelProxy_User_allocbuf (ret->_length);

		g_list_first (list);
		for (i = 0, list_pos = g_list_first (list) ;
		     i < ret->_length && NULL != list_pos;
		     i++, list_pos = g_list_next (list_pos)
		) {
			EazelProxy_User_copy (ret->_buffer+i, list_pos->data);
		}
	}
	
	return ret;
}

#if 0
static EazelProxy_AuthnInfo *
EazelProxy_AuthnInfo_dup (const EazelProxy_AuthnInfo * in)
{
	EazelProxy_AuthnInfo * ret;

	ret = EazelProxy_AuthnInfo__alloc();
	ret->username = CORBA_string_dup (in->username);
	ret->password = CORBA_string_dup (in->password);
	ret->services_redirect_uri = CORBA_string_dup (in->services_redirect_uri);
	ret->services_login_path = CORBA_string_dup (in->services_login_path);

	return ret;
}
#endif /* 0 */

/*******************************************************************
 * User object
 *******************************************************************/

#if 0
static User * user_for_username (const char *username);

static User * user_new (const CORBA_char *username);
#endif /* 0 */

static gboolean user_activate (User *user);

static void user_free (User *user);

/*static*/ void user_deactivate (User *user);

static void /*AuthenticateCallbackFn*/ 
user_authenticate_cb (
		gpointer user_data, 
		DigestState *p_digest, 		/* NOTE: calleee must free! */
		gboolean success, 
		const EazelProxy_AuthnFailInfo *fail_info,
		char *authn_post_response	/* NOTE: calleee must free! */
);


static gpointer /* ProxyRequestCb */ user_proxy_request_cb (gpointer user_data, unsigned short port, HTTPRequest *request, GList **p_header_list);

static void /* ProxyResponseCb */ user_proxy_response_cb (gpointer user_data, gpointer connection_user_data, unsigned short port, char ** p_response_line, GList **p_header_list);

static void  /* ProxyCloseCb */ user_proxy_close_cb (gpointer user_data, unsigned short port);

#if 0
static void
impl_EazelProxy_AuthnCallback_do_fail_authnfailinfo (
	EazelProxy_AuthnCallback callback,
	const EazelProxy_User *user,
	const EazelProxy_AuthnFailInfo *info
);
#endif /* 0 */
static void
impl_EazelProxy_AuthnCallback_do_fail_authnfailinfo_list (
	GList *callback_list,
	const EazelProxy_User *user,
	const EazelProxy_AuthnFailInfo *info
);
static void impl_EazelProxy_AuthnCallback_do_fail_list (GList *callback_list, const EazelProxy_User *user, long code, const char * http_result);
#if 0
static void impl_EazelProxy_AuthnCallback_do_fail (EazelProxy_AuthnCallback callback, const EazelProxy_User *user, long code, const char * http_result);
static void impl_EazelProxy_AuthnCallback_do_fail_username (EazelProxy_AuthnCallback callback, const char *username, long code, const char * http_result);
#endif /* 0 */
static void impl_EazelProxy_AuthnCallback_do_succeeded_list (GList *callback_list, const EazelProxy_User * user);


void listener_broadcast_user_authenticated (const User * user);
void listener_broadcast_user_authenticated_no_longer (const User * user, const EazelProxy_AuthnFailInfo *fail_info);
void listener_broadcast_user_logout (const User * user);

#if 0
static gint
glist_compare_user_username (gconstpointer a, gconstpointer b)
{
	const User *user;
	const char *username;

	user = (const User *) a;
	username = (const char *) b;

	if ( NULL == user || NULL == username || NULL == user->user.user_name ) {
		return -1;
	}
	return strcmp (user->user.user_name, username);
}

static User *
user_for_username (const char *username)
{
	GList *result;

	g_return_val_if_fail (NULL != username, NULL);

	/* Of course, this should be a hashtable */

	result = g_list_find_custom (gl_user_list, (char *)username, glist_compare_user_username);

	return result ? (User *)result->data : NULL;
}
#endif /* 0 */

static gint
glist_compare_user_user (gconstpointer a, gconstpointer b)
{
	const User *user_a;
	const User *user_b;

	user_a = (const User *) a;
	user_b = (const User *) b;

	if ( 	NULL == user_a || NULL == user_b 
		|| NULL == user_a->user.user_name
		|| NULL == user_b->user.user_name
	) {
		return -1;
	}

	if (	0 == strcmp (user_a->user.user_name, user_b->user.user_name)
		&& 0 == strcmp (user_a->user.services_redirect_uri, user_b->user.services_redirect_uri)
		&& 0 == strcmp (user_a->user.services_login_path, user_b->user.services_login_path)
	) {
		return 0;
	} else {
		return -1;
	}
}

/**
 * user_for_user
 * Compares both user_name and services_{redirect_uri,login_path} and return
 * a matching entry in the list
 */

static User *
user_for_user (const User *user)
{
	GList *result;

	g_return_val_if_fail (NULL != user, NULL);

	result = g_list_find_custom (gl_user_list, (gpointer)user, glist_compare_user_user);

	return result ? (User *)result->data : NULL;
}

static gint
glist_compare_user_proxy_port (gconstpointer a, gconstpointer b)
{
	const User *user;
	CORBA_unsigned_short proxy_port;

	user = (const User *) a;
	proxy_port = *(const unsigned short *) b;

	if ( NULL == user ) {
		return -1;
	}
	return (user->user.proxy_port == proxy_port) ? 0 : -1;
}

static User *
user_for_port (CORBA_unsigned_short proxy_port)
{
	GList *result;

	result = g_list_find_custom (gl_user_list, &proxy_port, glist_compare_user_proxy_port);

	return result ? (User *)result->data : NULL;
}

static User *
user_new_no_list_add (const CORBA_char *username)
{
	User *ret;

	g_return_val_if_fail (NULL != username, NULL);

	ret = g_new0 (User,1);

	ret->user.user_name = CORBA_string_dup (username);
	ret->user.login_state = EazelProxy_UNAUTHENTICATED;


	ret->user.services_redirect_uri = CORBA_string_dup ("");
	ret->user.services_login_path = CORBA_string_dup ("");
	ret->user.login_http_response = g_strdup ("");

	ret->prompter = CORBA_OBJECT_NIL;

	return ret;

}

#if 0
static User *
user_new (const CORBA_char *username)
{
	User *ret;

	ret = user_new_no_list_add (username);

	gl_user_list = g_list_append (gl_user_list, ret);

	return ret;
}
#endif /* 0 */

static void
user_set_default (User *user)
{
	if (gl_user_default) {
		gl_user_default->user.is_default = FALSE;
	}
	gl_user_default = user;
}

static gboolean
user_activate (User *user)
{
	gboolean ret;

	static const ProxyCallbackInfo callbacks = {
		user_proxy_request_cb,
		user_proxy_response_cb,
		user_proxy_close_cb
	};

	g_return_val_if_fail (NULL != user, FALSE);
	g_assert (0 == user->user.proxy_port);
	g_assert (EazelProxy_AUTHENTICATING == user->user.login_state);

	user->user.proxy_port = proxy_listen (0, user, &callbacks, user->user.services_redirect_uri);

	if ( 0 != user->user.proxy_port ) {

		if (user->user.is_default) {
			/* Note: a new default overides an existing one */
			user_set_default (user);
		}

		user->user.login_state = EazelProxy_AUTHENTICATED;

		ret = TRUE;
	} else {
		ret = FALSE;
	}

	return ret;
}

static void
user_callback_list_free (GList **p_list)
{
	CORBA_Environment ev;
	GList *list_node;

	CORBA_exception_init (&ev);

	g_return_if_fail ( NULL != p_list);
	
	for (	list_node = g_list_first (*p_list) ;
		NULL != list_node ;
		list_node = g_list_next (list_node)
	) {
		CORBA_Object_release ((CORBA_Object)list_node->data, &ev);
	}
	g_list_free (*p_list);

	*p_list = NULL;

	CORBA_exception_free (&ev);
}

static void
user_free (User *user)
{	
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	
	if (NULL != user) {
		/* Note that this removal may have already been done in user_deactivate */
		gl_user_list = g_list_remove (gl_user_list, user);

		CORBA_free (user->user.user_name);
		CORBA_free (user->user.services_redirect_uri);		
		CORBA_free (user->user.services_login_path);

		/* Note that login_http_response is g_malloc'd not CORBA_alloc'd */
		g_free (user->user.login_http_response);		

		digest_free (user->p_digest);

		user_callback_list_free (&(user->callback_list));

		CORBA_Object_release (user->prompter, &ev);
		
		g_free (user);
	}

	CORBA_exception_free (&ev);
}

/**
 * user_set_services_strings
 * Set service_redirect_uri and service_login_path to either the default
 * or the specficied value
 */
static void
user_set_services_strings (User *user, const EazelProxy_AuthnInfo *authninfo)
{
	if (authninfo->services_redirect_uri[0]) {
		if ( ! util_validate_url (authninfo->services_redirect_uri)) {
			log ("Invalid services_redirect_uri '%s' setting (must be URL)", authninfo->services_redirect_uri);
		} else {
			CORBA_free (user->user.services_redirect_uri);
			user->user.services_redirect_uri = CORBA_string_dup (authninfo->services_redirect_uri);
		}
	} else {
		CORBA_free (user->user.services_redirect_uri);
		user->user.services_redirect_uri = CORBA_string_dup (gl_default_services_uri);
	}

	if (authninfo->services_login_path[0]) {
		CORBA_free (user->user.services_login_path);
		user->user.services_login_path = CORBA_string_dup (authninfo->services_login_path);
	} else {
		CORBA_free (user->user.services_login_path);
		user->user.services_login_path = CORBA_string_dup (gl_default_login_path);
	}
}

void
user_set_login_state (User *user, EazelProxy_LoginState state)
{
	user->user.login_state = state;
}

EazelProxy_User *
user_get_EazelProxy_User (User *user)
{
	return &(user->user);
}

DigestState *
user_get_digest_state (User *user)
{
	return user->p_digest;
}

/* static */ void
user_deactivate (User *user)
{
	g_return_if_fail (NULL != user);
	g_assert (0 != user->user.proxy_port);
	g_assert (NULL == user->callback_list);
	g_assert (EazelProxy_AUTHENTICATING != user->user.login_state);

	if ( user == gl_user_default ) {
		user_set_default (NULL);
	}

	/* Note: this removal is also done in user_free for users that 
	 * were never activated.  That's ok--that one won't find anything
	 */
	gl_user_list = g_list_remove (gl_user_list, user);

	/* User * is free'd by user_proxy_close_cb */
	proxy_listen_close (user->user.proxy_port);	
}

static void
my_authenticate_user (User *user, const char *password, AuthenticateCallbackFn callback_fn);

/* NOTE: "p_digest" is NULL for the anonymous user */ 
static void /*AuthenticateCallbackFn*/ 
user_authenticate_cb (
		gpointer user_data, 
		DigestState *p_digest, 		/* NOTE: callee must free! */
		gboolean success, 
		const EazelProxy_AuthnFailInfo *fail_info,
		char *authn_post_response	/* NOTE: callee must free! */
) {
	CORBA_Environment ev;
	User *user;
	EazelProxy_AuthnFailInfo alt_fail_info = {0, ""};

	g_return_if_fail ( NULL != user_data );

	CORBA_exception_init (&ev);

	user = (User *)user_data;

	/* FIXME bugzilla.eazel.com 2847: Separate codes need to be returned for authn fail and
	 * network failure
	 */

	if (!success) {
		if ( 	CORBA_OBJECT_NIL != user->prompter
			&& fail_info && fail_info->code == EAZELPROXY_AUTHN_FAIL_AUTHN
		) {
			/* If we're in prompt_authenticate mode and we failed due to an AUTHN failure */
			if (user->prompt_count >= AUTHN_PROMPT_RETRIES) {
				EazelProxy_AuthnInfo *authninfo = NULL;
				/* We've prompted the user too many times */

				EazelProxy_UserPrompter_prompt_authenticate (
					user->prompter,
					&(user->user),
					EazelProxy_InitialFail, 
					&authninfo, 
					&ev
				);

				CORBA_free (authninfo);
			} else {
				CORBA_boolean has_info;
				EazelProxy_AuthnInfo *authninfo = NULL;

				/* if the caller to prompt_authenticate_user hadn't
				 * originally specified a username, we need to blow away
				 * the username before calling prompt_authenticate
				 * again
				 */
				if ( ! user->prompt_username_specified ) {
					CORBA_free (user->user.user_name);
					user->user.user_name = CORBA_string_dup ("");
				}
				
				has_info =	EazelProxy_UserPrompter_prompt_authenticate (
							user->prompter,
							&(user->user),
							EazelProxy_InitialRetry, 
							&authninfo, 
							&ev
						);
				if ( NO_EXCEPTION ( &ev ) ) {
					if (has_info) {
						user->prompt_count++;
						if (user->user.user_name) {
							CORBA_free (user->user.user_name);
						}
						user->user.user_name = CORBA_string_dup (authninfo->username);
						my_authenticate_user (user, authninfo->password, user_authenticate_cb);
						CORBA_free (authninfo);
						goto done;
					} else {
						/* User hit "Cancel" */
						alt_fail_info.code = EAZELPROXY_AUTHN_FAIL_USER_CANCELLED;
						fail_info = &alt_fail_info;
					}
					CORBA_free (authninfo);
				} else {
					/* <shrug> failure due to CORBA exception */
					alt_fail_info.code = EAZELPROXY_AUTHN_FAIL_USER_CANCELLED;
					fail_info = &alt_fail_info;
				}
			}
		}

		/* NOTE: This code block is executed when:
		 * 1) An non-prompt-authenticate authn has failed 
		 * 2) a FALSE has been returned from prompt_authenticate
		 *    (eg, because the user hit a cancel button in the prompt dialog) 
		 */
		user->user.login_state = EazelProxy_UNAUTHENTICATED;

		impl_EazelProxy_AuthnCallback_do_fail_authnfailinfo_list (
			user->callback_list,
			&(user->user), 
			fail_info
		);

		user_free (user);
	} else {

		user->p_digest = p_digest;
		user->user.login_http_response = authn_post_response;

		if( ! user_activate (user) ) {
			user->user.login_state = EazelProxy_UNAUTHENTICATED;
			log ("ERROR: problem activating user '%s'", user->user.user_name);

			impl_EazelProxy_AuthnCallback_do_fail_list (user->callback_list, &(user->user), EAZELPROXY_AUTHN_FAIL_RESOURCE, NULL);
			user_free (user);
		} else {
			log ("successful login for '%s' on port %d", user->user.user_name, user->user.proxy_port);
			impl_EazelProxy_AuthnCallback_do_succeeded_list (user->callback_list, &(user->user));
			user_callback_list_free (&(user->callback_list));
			listener_broadcast_user_authenticated (user);
		}
	}

done:
	CORBA_exception_free (&ev);
}

#if 0
static gint /* GCompareFunc */
user_glist_string_cmp (gconstpointer a, gconstpointer b)
{
	if ( NULL != a && NULL != b) {
		return strcmp ( (const char *)a, (const char *)b);
	} else {
		return -1;
	}
}
#endif

static gpointer /* ProxyRequestCb */
user_proxy_request_cb (gpointer user_data, unsigned short port, HTTPRequest *request, GList **p_header_list)
{
	User *user;
	char *digest_header;
	GList *user_agent_node;
	GList *destination_node;

	user = (User *) user_data;

	g_return_val_if_fail (NULL != user, 0);
	g_assert (p_header_list);
	g_assert (request);

	/* Ensure that there's no other Authorzation: header */

	*p_header_list = g_list_remove_all_custom (*p_header_list, "Authorization: ", util_glist_string_starts_with);

	/* FIXME mfleming we can remove this now */
	/* Smack Mozilla User-Agent string (We have a problem setting the optional field in M18 */

	user_agent_node = g_list_find_custom (*p_header_list, "User-Agent: Mozilla", util_glist_string_starts_with);

	if (user_agent_node) {
		char * user_agent_string = (char *)user_agent_node->data;
		if ( ! strstr (user_agent_string, "Nautilus")) {
			char * insert_point;

			insert_point = strchr (user_agent_string, ')');
			if (insert_point) {
				char *new_user_agent_string;

				*insert_point = '\0';
				new_user_agent_string = g_strconcat ( user_agent_string, "; Nautilus/1.0)", insert_point+1, NULL );

				g_free (user_agent_string);
				user_agent_node->data = new_user_agent_string;
				new_user_agent_string = NULL;
			}
		}
	}

	/* Translate the DAV "Destination" header.  The problem is
	 * that Destination: requires an absolute URI.
	 * And, of course,in this context, the host:port will be wrong
	 */

	destination_node = g_list_find_custom (*p_header_list, "Destination: ", util_glist_string_starts_with);

	if (destination_node) {
		char *new_header;
		char *dest_url;
		HTTPRequest *dest_request;

		dest_url = (char *)(destination_node->data) + strlen ("Destination: ");

		dest_request = request_new();
		
		if ( request_parse_url (dest_url, dest_request)
			&& dest_request->host 
			&& (0 == strcmp ("localhost", dest_request->host) || 0 == strcmp("127.0.0.1", dest_request->host))
			&& dest_request->port == user->user.proxy_port
		) {
			new_header = g_strdup_printf ("Destination: %s://%s:%d%s",
					dest_request->uri,
					request->host,
					request->port,
					dest_request->path);
			g_free (destination_node->data);
			destination_node->data = new_header;
		}
		
		request_free (dest_request);
	}

	/* p_digest is NULL for "anonymous" */

	if (NULL != user->p_digest) {
		digest_header = digest_gen_response_header (user->p_digest, request->path, request->method);

		*p_header_list = g_list_append (*p_header_list, digest_header);
	}

	if (gl_user_level_have) {
		*p_header_list = g_list_append (*p_header_list, g_strdup_printf ("X-Eazel-User-Level: %s", gl_user_level));
	}

	return GUINT_TO_POINTER (user->authn_count);
}

#if ENABLE_REAUTN
/*
 * This code is commented out for now; we should never get a 401 during a
 * routine session and its semi-busted anyway
 */

static void /*AuthenticateCallbackFn*/
user_reauthenticate_cb (User *user, DigestState *p_digest, gboolean success)
{
	if ( ! success ) {
		user->user.login_state = EazelProxy_UNAUTHENTICATED;
		log ("reauthn failed for '%s' on port %d\n");
	
	} else {
		digest_free (user->p_digest);
		user->p_digest = p_digest;

		user->user.login_state = EazelProxy_AUTHENTICATED;
		user->authn_count++;
		log ("successful re-authn (%d) for '%s' on port %d",
			user->authn_count, user->user.user_name,
			user->user.proxy_port
		);
	}

}


static void /* ProxyResponseCb */
user_proxy_response_handle_reauthn_cb (
	gpointer user_data, 
	gpointer connection_user_data,
	unsigned short port, 
	const char *status_line, 
	GList **p_header_list
) {

	/*
	 * FIXME bugzilla.eazel.com 2069: 
	 * the proxy core needs a way "replay" for this to work 
	 * right.
	 * Note that this code is currently commented out
	 */

	/*
	 * NOTE: Multiple sessions can cause modal re-authn dialogs
	 * to appear.  If this happens, then the first one to appear
	 * won't reactivate it's session until the others are dismissed.
	 * Really, the only way to fix this is to allow the proxy-core
	 * to suspend this connection and drop all the way back to the 
	 * event loop
	 */

	gchar *status_line_dup;
	HttpStatusLine status;
	User * user;
	guint authn_count;

	user = (User *) user_data;

	g_return_if_fail ( NULL != status_line );
	g_return_if_fail ( NULL != user );

	authn_count = GPOINTER_TO_UINT (connection_user_data);

	status_line_dup = g_strdup (status_line);

	if (http_parse_status_line (status_line_dup, &status)
		&& HTTP_RESPONSE_AUTHN_REQUIRED == status.code
	) {

		/* Do not pop open a reauthn dialog if the user has been
		 * reauthn'd since we started
		 */

		if (EazelProxy_AUTHENTICATED == user->user.login_state
			&& authn_count == user->authn_count
		) {
			GList *current;
			CORBA_Environment ev;
			CORBA_boolean has_reauthned;
			CORBA_char *password;
			CORBA_long retry_count;

			CORBA_exception_init (&ev);

			for (retry_count = 0; retry_count < 3; retry_count++) {

				/* Note that we call listeners in reverse, so that
				 * newer listeners can over-ride older ones
				 * These semantics still aren't sufficiently powerful
				 */

				user->user.login_state = EazelProxy_AUTHENTICATING;

				for( current = gl_user_listener_list, has_reauthned = FALSE;
					NULL != current && ! has_reauthned ;
					current = g_list_next (current)
				) {
					EazelProxy_UserListener listener;
					
					listener = (EazelProxy_UserListener)current->data;
				
					has_reauthned = EazelProxy_UserListener_reauthenticate_user (listener, &(user->user), retry_count, &password, &ev);

					/* If FALSE is returned, the password is empty */
					if (!has_reauthned) {
						CORBA_free (password);
					}

					CORBA_exception_free (&ev);	
				}

				if (has_reauthned) {
					authenticate_user (user, password, user_reauthenticate_cb ); 
					CORBA_free (password);

					while ( EazelProxy_AUTHENTICATING 
						== (volatile EazelProxy_LoginState) 
							user->user.login_state 
					) {
						g_main_iteration (TRUE);
						/* dispatch socket events
						 * this should be moved to a central "modal event loop" function
						 */
						socket_event_pump ();
					}

					if (EazelProxy_AUTHENTICATED == user->user.login_state) {
						break;
					}
				} else {
					/* Nobody wants to authn the user */
					break;
				}
			}

			if ( EazelProxy_AUTHENTICATED != user->user.login_state ) {
				EazelProxy_AuthnFailInfo fail_info = {0, ""};
				user->user.login_state = EazelProxy_UNAUTHENTICATED;
				listener_broadcast_user_authenticated_no_longer (user, &fail_info); 
			}

		} else {

			/* FIXME bugzilla.eazel.com 2845: This is busted.  If a second caller waits in this 
			 * event loop, and the first password is incorrect (thus
			 * requireing a second prompt) then the second caller will
			 * incorrectly return with an error rather than prompting
			 * and waiting for the retry
			 */

			/* wait for authn triggered by another connection to finish */

			while ( EazelProxy_AUTHENTICATING 
				== (volatile EazelProxy_LoginState) 
					user->user.login_state
			) {
				g_main_iteration (TRUE);
				/* dispatch socket events
				 * this should be moved to a central "modal event loop" function
				 */
				socket_event_pump ();
			}
		}
	}


	/* FIXME bugzilla.eazel.com 2069: Should replay here on successful re-authentication */ 
	/* Note that this code should remained commented out until that is implemented */

	g_free (status_line_dup);
}

#endif /* ENABLE_REAUTHN */

static void /* ProxyResponseCb */
user_proxy_response_cb (
	gpointer user_data, 
	gpointer connection_user_data,
	unsigned short port, 
	char **p_status_line, 
	GList **p_header_list
) {
	char * status_line_copy;
	HttpStatusLine status_line_parsed;
	User *user;
	
	/* If a response status was 401, then there's a possibility that
	 * the client could have gotten out of sync with the server (eg, the
	 * user changes his password on another client or via the web browser)
	 * Since we don't support re-authentication right now, the best thing
	 * for us to do is to log out the user, thus forcing them to log-in anew
	 * with the next request.
	 * 
	 * Unfortunately, this request will still fail.  Since we don't want
	 * to confuse the client with a 401 (embedded Mozilla might pop up its
	 * own password window), we change the response to a 404.
	 */

	user = (User *) user_data;

	status_line_copy = g_strdup (*p_status_line);

	http_parse_status_line (status_line_copy, &status_line_parsed);

	if (HTTP_RESPONSE_AUTHN_REQUIRED == status_line_parsed.code) {
		GList *current_node;

		g_free (*p_status_line);
		*p_status_line = g_strdup_printf ("HTTP/1.0 404 Not Found");

		/* Remove a WWW-Authenticate: header */
		for (	current_node = g_list_first (*p_header_list) ;
			NULL != current_node ;
			current_node = g_list_next (current_node)
		) {
			if ( STRING_STARTS_WITH ((char *)current_node->data, HTTP_AUTHENTICATE_HEADER)) {
				*p_header_list = g_list_remove_link (*p_header_list, current_node);
				g_free (current_node->data);
				g_list_free (current_node);
				break;
			}
		}

		log ("Got 401 during proxy, forcing logout user '%s' on port %d", user->user.user_name, user->user.proxy_port);

		listener_broadcast_user_logout (user);
		
		user_deactivate (user);
	}

	g_free (status_line_copy);
}

static void  /* ProxyCloseCb */
user_proxy_close_cb (gpointer user_data, unsigned short port)
{
	User *user;

	g_return_if_fail (NULL!=user_data);

	user = (User *) user_data;

	log ("Freeing user '%s'; last connection closed", user->user.user_name);

	user_free (user);
}

static void
impl_EazelProxy_AuthnCallback_do_fail_authnfailinfo_list (
	GList *callback_list,
	const EazelProxy_User *user,
	const EazelProxy_AuthnFailInfo *info
) {
	CORBA_Environment ev;
	GList *list_node;

	static const EazelProxy_AuthnFailInfo alt_fail_info = {0, ""};

	CORBA_exception_init (&ev);

	if (NULL == info) {
		log ("AuthnCallback::failed for user '%s' for reason '0' ''", user->user_name);
	} else {
		log ("AuthnCallback::failed for user '%s' for reason '%d' '%s'", 
			user->user_name,
			info->code, 
			info->http_result ? info->http_result : ""
		);
	}

	for (	list_node = g_list_first (callback_list) ;
		NULL != list_node ;
		list_node = g_list_next (list_node)
	) {
		EazelProxy_AuthnCallback callback;

		callback = (EazelProxy_AuthnCallback) list_node->data;

		if (CORBA_OBJECT_NIL != callback) {
			if (NULL == info) {
				EazelProxy_AuthnCallback_failed (callback, user, &alt_fail_info, &ev);
			} else {
				EazelProxy_AuthnCallback_failed (callback, user, info, &ev);
			}
		}
	}

	CORBA_exception_free (&ev);
}

#if 0
static void
impl_EazelProxy_AuthnCallback_do_fail_authnfailinfo (
	EazelProxy_AuthnCallback callback,
	const EazelProxy_User *user,
	const EazelProxy_AuthnFailInfo *info
) {
	CORBA_Environment ev;

	static const EazelProxy_AuthnFailInfo alt_fail_info = {0, ""};

	CORBA_exception_init (&ev);

	if (CORBA_OBJECT_NIL != callback) {
		if (NULL == info) {
			log ("AuthnCallback::failed for user '%s' for reason '0' ''", user->user_name);
			EazelProxy_AuthnCallback_failed (callback, user, &alt_fail_info, &ev);
		} else {
			log ("AuthnCallback::failed for user '%s' for reason '%d' '%s'", 
				user->user_name,
				info->code, 
				info->http_result ? info->http_result : ""
			);
			EazelProxy_AuthnCallback_failed (callback, user, info, &ev);
		}
	}

	CORBA_exception_free (&ev);
}

static void
impl_EazelProxy_AuthnCallback_do_fail (
	EazelProxy_AuthnCallback callback, 
	const EazelProxy_User *user,
	long code,
	const char * http_result
) {
	EazelProxy_AuthnFailInfo fail_info;

	fail_info.code = code;

	if (http_result) {
		fail_info.http_result = (char *) http_result;
	} else {
		fail_info.http_result = "";
	}

	impl_EazelProxy_AuthnCallback_do_fail_authnfailinfo (
		callback,
		user,
		&fail_info
	);
	
}
#endif /* 0 */

static void
impl_EazelProxy_AuthnCallback_do_fail_list (
	GList * callback_list, 
	const EazelProxy_User *user,
	long code,
	const char * http_result
) {
	EazelProxy_AuthnFailInfo fail_info;

	fail_info.code = code;

	if (http_result) {
		fail_info.http_result = (char *) http_result;
	} else {
		fail_info.http_result = "";
	}

	impl_EazelProxy_AuthnCallback_do_fail_authnfailinfo_list (
		callback_list,
		user,
		&fail_info
	);
	
}

#if 0
static void
impl_EazelProxy_AuthnCallback_do_fail_username (
	EazelProxy_AuthnCallback callback, 
	const char *username, 
	long code, 
	const char *http_result
) {
	EazelProxy_User user = {(char *)username, 0, 0, 0};

	impl_EazelProxy_AuthnCallback_do_fail (callback, &user, code, http_result);
}
#endif /* 0 */

static void
impl_EazelProxy_AuthnCallback_do_succeeded_list (
	GList *callback_list, 
	const EazelProxy_User *user
) {
	CORBA_Environment ev;
	GList *list_node;

	CORBA_exception_init (&ev);

	for (	list_node = g_list_first (callback_list) ;
		NULL != list_node ;
		list_node = g_list_next (list_node)
	) {
		EazelProxy_AuthnCallback callback;

		callback = (EazelProxy_AuthnCallback) list_node->data;
		if (CORBA_OBJECT_NIL != callback) {
			EazelProxy_AuthnCallback_succeeded (callback, user, &ev);
		}
	}

	CORBA_exception_free (&ev);
}

void
listener_broadcast_user_authenticated (const User * user)
{
	GList *current;
	EazelProxy_UserListener listener;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	for( current = gl_user_listener_list;
		NULL != current;
		current = g_list_next (current)
	) {
		listener = (EazelProxy_UserListener)current->data;
	
		EazelProxy_UserListener_user_authenticated (listener, &(user->user), &ev);
		CORBA_exception_free (&ev);	
	}
}

void
listener_broadcast_user_authenticated_no_longer (const User * user, const EazelProxy_AuthnFailInfo *fail_info)
{
	GList *current;
	EazelProxy_UserListener listener;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	for( current = gl_user_listener_list;
		NULL != current;
		current = g_list_next (current)
	) {
		listener = (EazelProxy_UserListener)current->data;

		EazelProxy_UserListener_user_authenticated_no_longer (listener, &(user->user), fail_info, &ev);
		CORBA_exception_free (&ev);	
	}
}

void
listener_broadcast_user_logout (const User * user)
{
	GList *current;
	EazelProxy_UserListener listener;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	for( current = gl_user_listener_list;
		NULL != current;
		current = g_list_next (current)
	) {
		listener = (EazelProxy_UserListener) current->data;

		EazelProxy_UserListener_user_logout (listener, &(user->user), &ev);
		CORBA_exception_free (&ev);	
	}
}



/*******************************************************************
 * EazelProxy::UserControl implementation
 *******************************************************************/

static EazelProxy_User *
impl_EazelProxy_UserControl_get_default_user (PortableServer_Servant servant, CORBA_Environment * ev)
{
	if (gl_user_default) {
	        return EazelProxy_User_duplicate (&(gl_user_default->user));
	} else {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_EazelProxy_NoDefaultUser, EazelProxy_NoDefaultUser__alloc());
		return NULL;
	}
}

static EazelProxy_UserList *
impl_EazelProxy_UserControl_get_active_users  (PortableServer_Servant servant, CORBA_Environment * ev)
{
	return EazelProxy_UserList_make_from_glist (gl_user_list);
}

static void
my_authenticate_user (User *user, const char *password, AuthenticateCallbackFn callback_fn)
{
	HTTPRequest *request;

	/* The "anonymous" user is a special case.  When the username is
	 * "anonymous", the normal login process is skipped and authentication
	 * headers are not appended.  The "anonymous" user can never be the
	 * default user.
	 */

	if ( 0 == strcmp (user->user.user_name, ANON_USER)) {
		user->user.is_default = FALSE;
		callback_fn ((gpointer) user, NULL, TRUE, NULL, g_strdup(""));
	} else {
		request = request_new();

		request_parse_url (user->user.services_redirect_uri, request);

		u_concat_replace_string (&request->path, user->user.services_login_path);

		authenticate_user (user->user.user_name, password, request, (gpointer) user, callback_fn);

		request_free (request);
	}
}

/**
 * impl_EazelProxy_UserControl_prompt_authenticate_user
 *
 * Prompt and authenticate a given service user, if necessary.
 * If the user is already logged in, AuthnCallback callback is called back
 * synchronously.
 * 
 * prompter may be called asynchronously if the user mistypes the login information.
 * 
 * Note that if the passed User object contains an empty user name and an is_default
 * set to TRUE, then the default user is returned if there is one, or the prompter
 * is called back if there is no existing default user
 */

static void
impl_EazelProxy_UserControl_prompt_authenticate_user (
	PortableServer_Servant servant, 
	const EazelProxy_AuthnInfo *authninfo,
	const CORBA_boolean is_default,
	const EazelProxy_UserPrompter prompter,
	const EazelProxy_AuthnCallback callback,
	CORBA_Environment * ev
) {
	User *user;
	User *user_listed;

	user = user_new_no_list_add (authninfo->username);

	user->callback_list = g_list_prepend (user->callback_list, CORBA_Object_duplicate (callback, ev));
	user->user.login_state = EazelProxy_AUTHENTICATING;
	/* Note that this is a *lie* until user_activate is called*/
	user->user.is_default = is_default;
	user_set_services_strings (user, authninfo);

	user_listed = NULL;
	if ( '\0' == authninfo->username[0] && is_default) {
		/* Find an existing default User */
		if (NULL != gl_user_default) {
			user_listed = gl_user_default;
		} 
	} else {
		/* Find an existing User with the same username and service strings */
		user_listed = user_for_user (user);
	}

	if (user_listed) {
		if (EazelProxy_AUTHENTICATING == user_listed->user.login_state) {
			user_listed->callback_list = 
						g_list_prepend (
							user_listed->callback_list, 
							CORBA_Object_duplicate (callback, ev)
						);
		} else {
			EazelProxy_AuthnCallback_succeeded (callback, &(user_listed->user), ev);
		}
		user_free (user);
		user = NULL;
	} else if ( 0 == strcmp (authninfo->username, ANON_USER) ) {
		/* The anonymous user gets logged in implicitly; it is not prompted for a password */
		gl_user_list = g_list_prepend (gl_user_list, user);

		my_authenticate_user (user, "", user_authenticate_cb);
	} else {
		CORBA_boolean has_password;
		EazelProxy_AuthnInfo *new_authninfo;

		user->prompt_username_specified = ! ('\0' == authninfo->username[0]);

		gl_user_list = g_list_prepend (gl_user_list, user);

		log ("Prompting user '%s' for authn", authninfo->username);

		has_password = EazelProxy_UserPrompter_prompt_authenticate (prompter, &(user->user), EazelProxy_Initial, &new_authninfo, ev);

		if ( NO_EXCEPTION (ev) && ! has_password ) {
			CORBA_free (new_authninfo);
			new_authninfo = NULL;
		} else if ( ! NO_EXCEPTION (ev) ) {
			has_password = FALSE;
			new_authninfo = NULL;
			CORBA_exception_free (ev);
		}
	
		user->prompter = CORBA_Object_duplicate (prompter, ev);

		if ( ! NO_EXCEPTION (ev) ) {
			has_password = FALSE;
			CORBA_exception_free (ev);
		}
		
		if (has_password) {
			/* If there was no user name specified by the caller;
			 * then the user was supposed to have specified one via
			 * the prompt
			 */
			/* robey 27oct2000: let the user override the username always */
			/* if ( '\0' == user->user.user_name[0] ) {
				CORBA_free (user->user.user_name);
				user->user.user_name = CORBA_string_dup (new_authninfo->username);
				} */
			if (new_authninfo->username[0] != '\0') {
				CORBA_free (user->user.user_name);
				user->user.user_name = CORBA_string_dup (new_authninfo->username);
			}
		
			log ("Starting authenticate process for user '%s'", authninfo->username);
			my_authenticate_user ( user, new_authninfo->password, user_authenticate_cb);
			CORBA_free (new_authninfo);
		} else {
			EazelProxy_AuthnFailInfo info = {EAZELPROXY_AUTHN_FAIL_USER_CANCELLED , ""};

			CORBA_Object_release (user->prompter, ev);
			CORBA_exception_free (ev);
			user->prompter = CORBA_OBJECT_NIL;

			user_authenticate_cb ((gpointer) user, NULL, FALSE, &info, NULL);
		}
	}
}

static void
impl_EazelProxy_UserControl_authenticate_user (
	PortableServer_Servant servant,
	const EazelProxy_AuthnInfo *authninfo,
	const CORBA_boolean is_default,
	const EazelProxy_AuthnCallback callback,
	CORBA_Environment * ev
) {
	User *user;
	User *user_listed;

	user = user_new_no_list_add (authninfo->username);

	user->callback_list = g_list_prepend (user->callback_list, CORBA_Object_duplicate (callback, ev));
	user->user.login_state = EazelProxy_AUTHENTICATING;
	/* Note that this is a *lie* until user_activate is called*/
	user->user.is_default = is_default;
	user_set_services_strings (user, authninfo);

	/* Does the user already exist? */
	if (NULL != (user_listed = user_for_user (user))) {
		log ("user_authenticate called for already logged in user '%s'", authninfo->username);

		EazelProxy_AuthnCallback_succeeded (callback, &(user_listed->user), ev);
		CORBA_exception_free (ev);

		user_free (user);
		return;
	}

	/* Add new user to the user list */
	gl_user_list = g_list_prepend (gl_user_list, user);

	log ("Starting authenticate process for user '%s'", authninfo->username);

	my_authenticate_user (user, authninfo->password, user_authenticate_cb);
}

static CORBA_boolean
impl_EazelProxy_UserControl_set_new_user_password (
	PortableServer_Servant servant,
	const EazelProxy_AuthnInfo *authninfo,
	const CORBA_char * new_password,
	const EazelProxy_AuthnCallback callback,
	CORBA_Environment * ev
) {
	/* 1. If the user is not logged in, log the user in
	 * 2. Submit the password change request
	 * 3. Perform the callback
	 */

	User *user;
	User *user_listed;
	EazelProxy_AuthnCallback callback_copy;

	user = user_new_no_list_add (authninfo->username);

	user->callback_list = g_list_prepend (user->callback_list, CORBA_Object_duplicate (callback, ev));
	user->user.login_state = EazelProxy_AUTHENTICATING;
	user_set_services_strings (user, authninfo);

	/* Find an existing User with the same username and service strings */
	user_listed = user_for_user (user);
	user_free (user);
	user = NULL;

	callback_copy = CORBA_Object_duplicate (callback, ev);
	
	if (! NO_EXCEPTION (ev)) {
		goto error;
	}

	if ( 	NULL == user_listed 
		|| EazelProxy_AUTHENTICATING == user_listed->user.login_state
		|| NULL == user_listed->p_digest	/* anonymous user can't change password */
	) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_EazelProxy_NoSuchUser, EazelProxy_NoSuchUser__alloc());
		goto error;
	} else {
		EazelProxy_AuthnInfo *authninfo_used;
		
		authninfo_used = EazelProxy_AuthnInfo__alloc ();

		authninfo_used->username = CORBA_string_dup (user_listed->user.user_name);
		authninfo_used->password = CORBA_string_dup (authninfo->password);
		authninfo_used->services_redirect_uri = CORBA_string_dup (user_listed->user.services_redirect_uri);
		authninfo_used->services_login_path = CORBA_string_dup (user_listed->user.services_login_path); 

		set_user_password (user_listed, authninfo_used, new_password, callback_copy);
	}

error:
	return FALSE;
}

static CORBA_char *
impl_EazelProxy_UserControl_get_authn_header_for_port (
	PortableServer_Servant servant,
	const CORBA_unsigned_short logout_port,
	const CORBA_char * path,
	const CORBA_char * method,
	CORBA_Environment * ev
) {
	User *user;

	user = user_for_port (logout_port);

	if ( NULL == user || NULL == user->p_digest ) {
		return CORBA_string_dup ("");
	} else {
		return digest_gen_response_header (user->p_digest, path, method);
	}
}	

static CORBA_boolean
impl_EazelProxy_UserControl_logout_user(
	PortableServer_Servant servant, 
	const CORBA_unsigned_short logout_port,
	CORBA_Environment * ev
) {
	User * user;
	CORBA_boolean ret;

	user = user_for_port (logout_port);

	if ( NULL == user ) {
		log ("WARN: request to logout inactive port '%d'", logout_port);
		ret = FALSE;		
	} else if (user->user.login_state == EazelProxy_AUTHENTICATING) {
		log ("WARN: request to logout authenticating user '%s' on port %d(ignored)", 
			user->user.user_name, 
			logout_port
		);
		ret = FALSE;
	} else {
		log ("logging out user '%s' on port %d", user->user.user_name, logout_port);

		listener_broadcast_user_logout (user);
		
		user_deactivate (user);
		ret = TRUE;
	}

	return ret;
}

static void
impl_EazelProxy_UserControl_add_listener(PortableServer_Servant servant,
					 const EazelProxy_UserListener listener,
					 CORBA_Environment * ev)
{
	EazelProxy_UserListener listener_copy;

	if (CORBA_OBJECT_NIL != listener) {

		listener_copy = CORBA_Object_duplicate (listener, ev);

		gl_user_listener_list = g_list_prepend (gl_user_listener_list, listener_copy);

		log ("Added UserListener %0x08x", listener_copy);
	
	}
}


static gint /* GCompareFunc */
usercontrol_glist_find_object (gconstpointer a, gconstpointer b)
{
	CORBA_Object obj_a;
	CORBA_Object obj_b;
	CORBA_Environment ev;
	gint ret;

	CORBA_exception_init (&ev);

	obj_a = (CORBA_Object) a;
	obj_b = (CORBA_Object) a;

	ret = (CORBA_Object_is_equivalent (obj_a, obj_b, &ev)) ? 0 : -1;

	CORBA_exception_free (&ev);

	return ret;
}

static void
impl_EazelProxy_UserControl_remove_listener(PortableServer_Servant servant,
					    const EazelProxy_UserListener to_remove,
					    CORBA_Environment * ev)
{
	GList *list_item;

	list_item = g_list_find_custom (gl_user_listener_list, (gpointer) &to_remove, usercontrol_glist_find_object);

	if (list_item) {
		log ("Removed UserListener 0x%08x", (unsigned int)list_item->data);

		CORBA_Object_release ((CORBA_Object)list_item->data, ev);

		gl_user_listener_list = g_list_remove_link (gl_user_listener_list, list_item);
		g_list_free (list_item);
	}
}
