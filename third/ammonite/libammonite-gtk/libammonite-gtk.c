/* $Id: libammonite-gtk.c,v 1.1.1.1 2001-01-16 15:26:35 ghudson Exp $
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

#include "libammonite-gtk.h"
#include <gtk/gtk.h>
#include <gnome.h>
#include <gconf/gconf.h>
#include <gconf/gconf-engine.h>

#define EAZEL_ACCOUNT_REGISTER_URI "eazel-services://anonymous/account/register/form"
/* FIXME this is not the real URI */
#define EAZEL_ACCOUNT_FORGOTPW_URI "eazel-services://anonymous/account/login/lost_pwd_form"

#define KEY_GCONF_TRILOBITE_DEFAULT_USER "/apps/eazel-trilobite/default-services-user"


#ifdef DEBUG
#define DEBUG_MSG(x) my_debug_printf x
#include <stdio.h>

static void
my_debug_printf(char *fmt, ...)
{
	va_list args;
	gchar * out;

	g_assert (fmt);

	va_start (args, fmt);

	out = g_strdup_vprintf (fmt, args);

	fprintf (stderr, "DEBUG: %s\n", out);

	g_free (out);
	va_end (args);
}

#else
#define DEBUG_MSG(x) 
#endif

typedef struct {
	gboolean 		done;
	EazelProxy_User *	user;
	EazelProxy_AuthnFailInfo * fail_info;
} LoginSignalState;

typedef struct {
	gpointer 	user_data; 
	AmmonitePromptLoginCb callback;
	gboolean	prompt_user_name_can_be_changed;
} PromptLoginState;


static void
authn_cb_succeeded (const EazelProxy_User *user, gpointer state, CORBA_Environment *ev)
{
	PromptLoginState *p_state;

	DEBUG_MSG (("%lu: In authn_cb_succeeded\n", (unsigned long) getpid()));
	
	g_assert (NULL != user);
	g_assert (NULL != state);

	p_state = (PromptLoginState *)state;

	if (p_state->callback) {
		p_state->callback (p_state->user_data, user, NULL);
	}
}

static void
authn_cb_failed (const EazelProxy_User *user, const EazelProxy_AuthnFailInfo *info, gpointer state, CORBA_Environment *ev)
{
	PromptLoginState *p_state;

	DEBUG_MSG (("%lu: In authn_cb_failed\n", (unsigned long) getpid()));
	
	g_assert (NULL != user);
	g_assert (NULL != state);

	p_state = (PromptLoginState *)state;

	if (p_state->callback) {
		p_state->callback (p_state->user_data, user, info);
	}
}


static CORBA_boolean
prompter_cb_prompt_authenticate (
	const EazelProxy_User *user,
	const EazelProxy_AuthnPromptKind kind, 
	EazelProxy_AuthnInfo **authninfo, 
	gpointer user_data, 
	CORBA_Environment *ev
) {
	gchar *password_glib = NULL;
	gchar *username_glib = NULL;
	CORBA_boolean ret;
	PromptLoginState *p_state;

	g_return_val_if_fail (NULL != user_data, FALSE);

	p_state = (PromptLoginState *)user_data;

	ret = FALSE;

	if ( 	EazelProxy_InitialFail == kind
		|| EazelProxy_ReauthnFail == kind
	) {
		ammonite_do_authn_error_dialog();

		*authninfo = EazelProxy_AuthnInfo__alloc();

		(*authninfo)->services_redirect_uri = CORBA_string_dup ("");
		(*authninfo)->services_login_path = CORBA_string_dup ("");
		(*authninfo)->username = CORBA_string_dup ( "" ); 
		(*authninfo)->password = CORBA_string_dup ( "" ); 
	} else {
		/* If there was a username specified, then we're not going to allow
		 * the user to change it
		 */
		if ( '\0' == user->user_name[0] ) {
			AmmonitePromptDialogFlags flags;
			flags = (kind == EazelProxy_Initial) ? 0 : Prompt_IsRetry; 
			
			ret = ammonite_do_prompt_dialog (NULL, NULL, flags, &username_glib, &password_glib);
		} else {
			AmmonitePromptDialogFlags flags;

			flags = (kind == EazelProxy_Initial) ?  0 : Prompt_IsRetry; 
			if (! p_state->prompt_user_name_can_be_changed) {
				flags |= Prompt_IsUsernameRO;
			} 

			/* robey 27oct2000: allow user to override username */
			ret = ammonite_do_prompt_dialog (
				user->user_name, NULL, 
				flags, &username_glib, &password_glib);
		}

		*authninfo = EazelProxy_AuthnInfo__alloc();

		(*authninfo)->services_redirect_uri = CORBA_string_dup ("");
		(*authninfo)->services_login_path = CORBA_string_dup ("");

		if (ret) {
			if ( '\0' == user->user_name[0] ) {
				(*authninfo)->username = CORBA_string_dup ( (NULL != username_glib) ? username_glib : "" );
			} else {
				/* robey 27oct2000: allow user to override username */
				(*authninfo)->username = CORBA_string_dup ( (NULL != username_glib) ? username_glib : user->user_name );
			}
			(*authninfo)->password = CORBA_string_dup ( (NULL != password_glib) ? password_glib : "" ); 

			g_free (username_glib);
			username_glib = NULL;
			g_free (password_glib);
			password_glib = NULL;
		} else {
			(*authninfo)->username = CORBA_string_dup ( "" ); 
			(*authninfo)->password = CORBA_string_dup ( "" ); 
		}	
	}

	return ret;
}

static gboolean
evil_init_gconf (void)
{
	GError *error = NULL;
	
	if (!gconf_is_initialized ()) {
		char		  *argv[] = { "libammonite", NULL };
		
		if (!gconf_init (1, argv, &error)) {
			g_assert (error != NULL);
			
			g_warning ("GConf init failed:\n  %s", error->message);
			
			g_error_free (error);
			
			error = NULL;
			
			return FALSE;
		}
	}
	return TRUE;
}

static char *
get_default_username (void)
{
	GConfValue *value_gconf;
	GConfEngine *engine_gconf;
	GError * err_gconf = NULL;
	char * ret;

	if ( ! evil_init_gconf () ) {
		return NULL;
	}

	engine_gconf = gconf_engine_get_default();

	value_gconf = gconf_engine_get (engine_gconf, KEY_GCONF_TRILOBITE_DEFAULT_USER, &err_gconf);

	if (NULL == value_gconf || NULL != err_gconf 
	    || GCONF_VALUE_STRING != value_gconf->type) {
		ret = NULL;
	} else {
		ret = g_strdup (gconf_value_get_string (value_gconf));
		gconf_value_free (value_gconf);
		value_gconf = NULL;
	}

	gconf_engine_unref (engine_gconf);

	return ret;
}

gboolean
ammonite_do_prompt_login_async (
	const char *username, 
	const char *services_redirect_uri, 
	const char *services_login_path,
	gpointer user_data,
	AmmonitePromptLoginCb callback
) {

	CORBA_Environment ev;
	EazelProxy_UserPrompter userprompter = CORBA_OBJECT_NIL;
	EazelProxy_AuthnCallback authn_callback = CORBA_OBJECT_NIL;
	PromptLoginState *p_state;
	EazelProxy_AuthnInfo *authinfo;
	gboolean success;

	AmmoniteAuthCallbackWrapperFuncs authn_cb_funcs = {
		authn_cb_succeeded, authn_cb_failed
	};
	AmmoniteUserPrompterWrapperFuncs up_cb_funcs = {
		prompter_cb_prompt_authenticate
	};

	CORBA_exception_init (&ev);

	p_state = g_new0 (PromptLoginState, 1);
	p_state->user_data = user_data;
	p_state->callback = callback;

	userprompter = ammonite_userprompter_wrapper_new (ammonite_get_poa(), &up_cb_funcs, p_state);
	authn_callback = ammonite_auth_callback_wrapper_new (ammonite_get_poa(), &authn_cb_funcs, p_state);

	if (CORBA_OBJECT_NIL == authn_callback) {
		g_warning ("Couldn't create AuthnCallback");
		success = FALSE;
		goto error;
	}

	if (CORBA_OBJECT_NIL == userprompter) {
		g_warning ("Couldn't create UserPrompter\n");
		success = FALSE;
		goto error;
	}

	authinfo = EazelProxy_AuthnInfo__alloc ();
	authinfo->password = CORBA_string_dup ("");
	authinfo->services_redirect_uri = CORBA_string_dup (services_redirect_uri ? services_redirect_uri : "");
	authinfo->services_login_path = CORBA_string_dup (services_login_path ? services_login_path : "");

	/* If a username wasn't supplied, see if we can get what the user last used */
	if (NULL == username) {
		char * default_username;

		default_username = get_default_username();
		if (NULL == default_username) {
			authinfo->username = CORBA_string_dup ("");
		} else {
			authinfo->username = CORBA_string_dup (default_username);
			g_free (default_username);
			p_state->prompt_user_name_can_be_changed = TRUE;
		}
	} else {
		/* For some cases (such as prompting after an eazel-service://user/path URL)
		 * we can't allow the user to change the username in the prompt dialog
		 */
		authinfo->username = CORBA_string_dup (username);
		p_state->prompt_user_name_can_be_changed = FALSE;
	}

	DEBUG_MSG (("%lu: Calling prompt_authenticate\n", (unsigned long) getpid()));

	EazelProxy_UserControl_prompt_authenticate_user (ammonite_get_user_control(), authinfo, TRUE, userprompter, authn_callback, &ev);

	DEBUG_MSG (("%lu: Back from prompt_authenticate\n", (unsigned long) getpid()));

	if (CORBA_NO_EXCEPTION != ev._major) {
		g_warning ("Exception during prompt_authenticate_user");
		success = FALSE;
		goto error;
	}

	success = TRUE;

error:
	if (! success ) {
		ammonite_auth_callback_wrapper_free (ammonite_get_poa(), authn_callback);
		ammonite_userprompter_wrapper_free (ammonite_get_poa(), userprompter);
		g_free (p_state);
	}
	
	CORBA_exception_free (&ev);

	return success;
}

static void /* AmmonitePromptLoginCb */
prompt_login_sync_cb (gpointer user_data, const EazelProxy_User *user, const EazelProxy_AuthnFailInfo *fail_info)
{
	LoginSignalState *p_state;

	p_state = (LoginSignalState *)user_data;

	p_state->done = TRUE;
	p_state->user = EazelProxy_User_duplicate (user);
	p_state->fail_info = EazelProxy_AuthnFailInfo_duplicate (fail_info);
}

EazelProxy_User *
ammonite_do_prompt_login (
	const char *username, 
	const char *services_redirect_uri, 
	const char *services_login_path,
	/*OUT*/ CORBA_long *p_fail_code
) {

	volatile LoginSignalState state;
	EazelProxy_User *user = NULL;
 	CORBA_long fail_code = 0;
	gboolean success;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	memset ((void *)&state, 0, sizeof(state) );

	success = ammonite_do_prompt_login_async (
		username, 
		services_redirect_uri, 
		services_login_path,
		(gpointer)&state,
		prompt_login_sync_cb
	);

	if (success) {
		EazelProxy_UserControl user_control;

		user_control = ammonite_get_user_control();
		while ( ! state.done && ! CORBA_Object_non_existent (user_control, &ev) ) {
			gtk_main_iteration();
		}

		/* Clean up events after the dialog */
		while (gtk_events_pending()) {
			gtk_main_iteration();
		}

		if (CORBA_Object_non_existent (user_control, &ev)) {
			DEBUG_MSG (("%lu: CORBA server disappeared\n",(unsigned long) getpid()));
			goto error;
		}

		DEBUG_MSG (("%lu: Response iteration complete, success=%s\n", (unsigned long) getpid(), state.fail_info ? "FALSE" : "TRUE"));

		if (state.fail_info ) {
			if (p_fail_code) { 
				*p_fail_code = state.fail_info->code;
			}
			fail_code = state.fail_info->code;
			CORBA_free (state.fail_info);
			CORBA_free (state.user);
		} else {
			user = state.user;
			/* and fail_info is NULL */
		}
	}

	if ( ! user
		  && ( EAZELPROXY_AUTHN_FAIL_NETWORK == fail_code
		  		|| EAZELPROXY_AUTHN_FAIL_SERVER == fail_code
		  )
	) {
		ammonite_do_network_error_dialog();
	}

error:
	CORBA_exception_free (&ev);

	return user;
}

static pid_t
util_fork_exec (const char *path, char *const argv[])
{
	pid_t pid;

	pid = fork ();

	if ( 0 == pid ) {
		execvp (path, argv);
		exit (-1);	
	}
	
	return pid;

}

gboolean
ammonite_do_prompt_dialog (
	const char *user, 
	const char *pw,
	AmmonitePromptDialogFlags flags,
	char **p_user, 
	char **p_pw
) {

	GtkWidget *dialog;
	LoginDialogReturn dialog_return;
	gboolean ret;

	g_return_val_if_fail ( NULL != user || NULL != p_user, FALSE);
	g_return_val_if_fail ( NULL != p_pw, FALSE);

	if (p_user) {
		*p_user = NULL;
	}
	*p_pw = NULL;

	DEBUG_MSG (("Opening Dialog\n"));

	dialog = ammonite_login_dialog_new ( 
			(flags & Prompt_IsRetry) ? EazelProxy_InitialRetry : EazelProxy_Initial, 
			"", 
			"", 
			flags & Prompt_IsUsernameRO
		 );

	if ( NULL != user && '\0' != user[0] ) {
		ammonite_login_dialog_set_username ( 
			AMMONITE_LOGIN_DIALOG (dialog),
			user
		);
		ammonite_login_dialog_set_readonly_username ( 
			AMMONITE_LOGIN_DIALOG (dialog),
			flags & Prompt_IsUsernameRO
		);
	}

	dialog_return = ammonite_login_dialog_run_and_block (AMMONITE_LOGIN_DIALOG (dialog));

	if (BUTTON_OK == dialog_return) {
		if ( NULL != p_user ) {
			*p_user = ammonite_login_dialog_get_username (AMMONITE_LOGIN_DIALOG (dialog));
		}
		if ( NULL != p_pw ) {
			*p_pw = ammonite_login_dialog_get_password (AMMONITE_LOGIN_DIALOG (dialog));		
		}
		ret = TRUE;
	} else if (BUTTON_REGISTER == dialog_return) {
		char * const args[] = {"nautilus", EAZEL_ACCOUNT_REGISTER_URI, NULL};

		/* Run nautilus for the "register" button */
		util_fork_exec ("nautilus", args);
		
		ret = FALSE;
	} else if (BUTTON_FORGOTPPW == dialog_return) {
		char * const args[] = {"nautilus", EAZEL_ACCOUNT_FORGOTPW_URI, NULL};

		/* Run nautilus for the "register" button */
		util_fork_exec ("nautilus", args);
		
		ret = FALSE;
	} else {
		DEBUG_MSG (("User cancelled...\n"));

		ret = FALSE;
	}

	gtk_widget_destroy (dialog);

	return ret;
}

void
ammonite_do_authn_error_dialog ()
{
	GtkWidget *dialog;
	LoginDialogReturn dialog_return;

	DEBUG_MSG (("Opening Dialog\n"));

	dialog = ammonite_login_dialog_new ( 
			EazelProxy_InitialFail, 
			"", 
			"", 
			FALSE
		 );

	dialog_return = ammonite_login_dialog_run_and_block (AMMONITE_LOGIN_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	/* Clean up events after the dialog */
	while (gtk_events_pending()) {
		gtk_main_iteration();
	}

	if (BUTTON_REGISTER == dialog_return) {
		char * const args[] = {"nautilus", EAZEL_ACCOUNT_REGISTER_URI , NULL};

		/* Run nautilus for the "register" button */
		util_fork_exec ("nautilus", args);
	} else if (BUTTON_FORGOTPPW == dialog_return) {
		char * const args[] = {"nautilus", EAZEL_ACCOUNT_FORGOTPW_URI , NULL};

		/* Run nautilus for the "register" button */
		util_fork_exec ("nautilus", args);
	}
	
}

void
ammonite_do_network_error_dialog ()
{
	GtkWidget *dialog;

	DEBUG_MSG (("Opening Network Error Dialog\n"));

	dialog = gnome_error_dialog ("I'm sorry, network problems are preventing you from connecting to Eazel Services.");

	gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

	/* Clean up events after the dialog */
	while (gtk_events_pending()) {
		gtk_main_iteration();
	}
}


