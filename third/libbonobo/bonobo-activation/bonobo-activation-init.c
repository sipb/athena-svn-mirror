/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  bonobo-activation: A library for accessing bonobo-activation-server.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
 *  Copyright (C) 2003 Ximian, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *      Elliot Lee <sopwith@redhat.com>
 *      Michael Meeks <michael@ximian.com>
 */

#include <config.h>
#include "bonobo-activation-init.h"
#include "bonobo-activation-client.h"

#include "Bonobo_ActivationContext.h"
#include "bonobo-activation-i18n.h"
#include "bonobo-activation-private.h"
#include "bonobo-activation-register.h"
#include "bonobo-activation-version.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <glib.h>
#include <netdb.h>
#include <popt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

/****************** ORBit-specific stuff ****************/

#include <orbit/orbit.h>

GStaticRecMutex _bonobo_activation_guard = G_STATIC_REC_MUTEX_INIT;
static CORBA_ORB bonobo_activation_orb = CORBA_OBJECT_NIL;
static CORBA_Context bonobo_activation_context;
static gboolean is_initialized = FALSE;

/* prevent registering with OAF when bonobo_activation_active_server_register() */
gboolean bonobo_activation_private = FALSE;


/**
 * bonobo_activation_orb_get:
 *
 * Returns the ORB used by OAF.
 *
 * Return value: the ORB used by OAF.
 */
CORBA_ORB
bonobo_activation_orb_get (void)
{
	return bonobo_activation_orb;
}

const char *
bonobo_activation_hostname_get (void)
{
        /*
         * This tolerates a run-time change of
         * hostname [a-la DHCP], and we don't care
         * about this anyway; there is no cross-host
         * activation foo.
         */
        return "localhost";
#if 0
	static char hostname[256] = "";
	if (hostname[0] == '\0') {
		if (gethostname (hostname, sizeof (hostname) - 1))
                        strcpy (hostname, "localhost");
        }
	return hostname;
#endif
}


/**
 * bonobo_activation_context_get:
 * @void: 
 * 
 * Fetches the internal CORBA_Context used with every activation
 * request. This can be used to manipulate some of the associated
 * fields; particularly 'display'.
 *
 * This method is deprecated.
 * 
 * Return value: the CORBA context for activating with.
 **/
CORBA_Context
bonobo_activation_context_get (void)
{
	return bonobo_activation_context;
}

const char *
bonobo_activation_session_name_get (void)
{
	const char *dumbptr = "local";

	return dumbptr;
}

const char *
bonobo_activation_domain_get (void)
{
	return NULL;
}

CORBA_Object
bonobo_activation_internal_activation_context_get_extended (gboolean           existing_only,
                                                            CORBA_Environment *ev)
{
	BonoboActivationBaseService base_service = { NULL };

	base_service.name = "IDL:Bonobo/ActivationContext:1.0";
	base_service.session_name = bonobo_activation_session_name_get ();

	return bonobo_activation_internal_service_get_extended (&base_service, existing_only,
                                                   ev);
}

CORBA_Object
bonobo_activation_activation_context_get (void)
{
	BonoboActivationBaseService base_service = { NULL };

	base_service.name = "IDL:Bonobo/ActivationContext:1.0";
	base_service.session_name = bonobo_activation_session_name_get ();

	return bonobo_activation_service_get (&base_service);
}

static Bonobo_ObjectDirectory object_directory = CORBA_OBJECT_NIL;

CORBA_Object
bonobo_activation_object_directory_get (const char *username,
                                        const char *hostname)
{
        CORBA_Environment ev;
        Bonobo_ActivationContext new_ac;
        Bonobo_ObjectDirectoryList *od_list;
        static Bonobo_ActivationContext ac = CORBA_OBJECT_NIL;

        new_ac = bonobo_activation_activation_context_get ();
        if (ac == new_ac)
                return object_directory;
        ac = new_ac;

        CORBA_exception_init (&ev);

        od_list = Bonobo_ActivationContext__get_directories (ac, &ev);
        if (ev._major != CORBA_NO_EXCEPTION) {
                CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

        if (od_list->_length != 1) {
                g_warning ("Extremely strange, strange object directories (%d)"
                           "registered with the activation context", od_list->_length);
                CORBA_free (od_list);
                CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

        object_directory = CORBA_Object_duplicate (od_list->_buffer[0], &ev);
        
        CORBA_free (od_list);
        CORBA_exception_free (&ev);

        return object_directory;
}

static int   bonobo_activation_ior_fd = 1;
static char *bonobo_activation_activate_iid = NULL;

struct poptOption bonobo_activation_popt_options[] = {
        { NULL, '\0', POPT_ARG_INTL_DOMAIN, GETTEXT_PACKAGE, 0, NULL, NULL },
        { "oaf-ior-fd", '\0', POPT_ARG_INT, &bonobo_activation_ior_fd, 0,
          N_("File descriptor to print IOR on"), N_("FD") },
        { "oaf-activate-iid", '\0', POPT_ARG_STRING, &bonobo_activation_activate_iid, 0,
          N_("IID to activate"), "IID" },
        { "oaf-private", '\0', POPT_ARG_NONE, &bonobo_activation_private, 0,
          N_("Prevent registering of server with OAF"), NULL },
        { NULL }
};

/**
 * bonobo_activation_activation_iid_get:
 *
 * If this process was launched to activate an exe server, this
 * function gives the IID of the server requested, otherwise it
 * returns NULL.
 * 
 * Return value: The IID of the activated server or NULL.
 */
const char *
bonobo_activation_iid_get (void)
{
	return bonobo_activation_activate_iid;
}

int
bonobo_activation_ior_fd_get (void)
{
	return bonobo_activation_ior_fd;
}

void
bonobo_activation_preinit (gpointer app, gpointer mod_info)
{
        bindtextdomain (GETTEXT_PACKAGE, BONOBO_ACTIVATION_LOCALEDIR);
}

void
bonobo_activation_postinit (gpointer app, gpointer mod_info)
{
	bonobo_activation_base_service_init ();

	if (bonobo_activation_ior_fd > 2)
		fcntl (bonobo_activation_ior_fd, F_SETFD, FD_CLOEXEC);

        if (bonobo_activation_activate_iid)
                g_timeout_add_full (G_PRIORITY_LOW,
                                    BONOBO_ACTIVATION_FACTORY_TIMEOUT,
                                    bonobo_activation_timeout_reg_check,
                                    NULL, NULL);
        else
                bonobo_activation_timeout_reg_check_set (FALSE);

	is_initialized = TRUE;
}

#ifdef BONOBO_ACTIVATION_DEBUG
static void
do_barrier (int signum)
{
	volatile int barrier = 1;

	while (barrier);
}
#endif

/**
 * bonobo_activation_is_initialized:
 *
 * Tells you whether or not bonobo-activation is initialized.
 *
 * Return value: whether bonobo-activation is initialized or not.
 */
gboolean
bonobo_activation_is_initialized (void)
{
	return is_initialized;
}


/**
 * bonobo_activation_get_popt_table_name:
 *
 * Get the table name to use for the oaf popt options table when
 * registering with libgnome
 * 
 * Return value: A localized copy of the string "bonobo activation options"
 */
char *
bonobo_activation_get_popt_table_name (void)
{
        bindtextdomain (PACKAGE, BONOBO_ACTIVATION_LOCALEDIR);
        return _("Bonobo activation options");
}


/**
 * bonobo_activation_init:
 * @argc: number of command-line arguments passed to the program.
 * @argv: array of strings containing the command-line 
 *        arguments of the program.
 *
 * Initializes bonobo-activation. Should be called before any other
 * call to the library.
 *
 * Return value: the ORB used by bonobo-activation.
 */
CORBA_ORB
bonobo_activation_init (int argc, char **argv)
{
	CORBA_ORB retval;
	int i;

	g_return_val_if_fail (is_initialized == FALSE, bonobo_activation_orb);

        bindtextdomain (PACKAGE, BONOBO_ACTIVATION_LOCALEDIR);

	bonobo_activation_preinit (NULL, NULL);

	retval = bonobo_activation_orb_init (&argc, argv);

	/* Handle non-popt case */
	for (i = 1; i < argc; i++) {
                if (!strncmp ("--oaf-ior-fd=", argv[i],
                              strlen ("--oaf-ior-fd="))) {
                        bonobo_activation_ior_fd =
                                atoi (argv[i] + strlen ("--oaf-ior-fd="));
                        if (!bonobo_activation_ior_fd)
                                bonobo_activation_ior_fd = 1;
                } else if (!strncmp
                           ("--oaf-activate-iid=", argv[i],
                            strlen ("--oaf-activate-iid="))) {
			bonobo_activation_activate_iid =
				g_strdup (argv[i] + strlen ("--oaf-activate-iid="));
                } else if (!strcmp
                           ("--oaf-private", argv[i])) {
                        bonobo_activation_private = TRUE;
                }     
	}

	bonobo_activation_postinit (NULL, NULL);

	return retval;
}

/**
 * bonobo_activation_orb_init:
 * @argc: pointer to program's argument count
 * @argv: argument array
 * 
 * Initialize Bonobo Activation's ORB - do this once centrally
 * so it can be easily shared.
 * 
 * Return value: the ORB.
 **/
CORBA_ORB
bonobo_activation_orb_init (int *argc, char **argv)
{
        CORBA_Context def_ctx;
	CORBA_Environment ev;
	const char *hostname;

	CORBA_exception_init (&ev);

	bonobo_activation_orb = CORBA_ORB_init (argc, argv, "orbit-local-mt-orb", &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	bonobo_activation_init_activation_env ();

	/* Set values in default context */
	CORBA_ORB_get_default_context (bonobo_activation_orb, &def_ctx, &ev);
        CORBA_Context_create_child (def_ctx, "activation", &bonobo_activation_context, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
        CORBA_Object_release ((CORBA_Object) def_ctx, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	hostname = bonobo_activation_hostname_get ();
	CORBA_Context_set_one_value (bonobo_activation_context, "hostname",
				     (char *) hostname, &ev);
	CORBA_Context_set_one_value (bonobo_activation_context, "username",
				     (char *) g_get_user_name (), &ev);

	CORBA_exception_free (&ev);

#ifdef BONOBO_ACTIVATION_DEBUG
	if (getenv ("BONOBO_ACTIVATION_TRAP_SEGV")) {
		struct sigaction sa;
		sa.sa_handler = do_barrier;
		sigaction (SIGSEGV, &sa, NULL);
		sigaction (SIGPIPE, &sa, NULL);
	}
	if (getenv ("BONOBO_ACTIVATION_BARRIER_INIT")) {
		volatile int barrier = 1;
		while (barrier);
	}
#endif

	return bonobo_activation_orb;
}

/**
 * bonobo_activation_debug_shutdown:
 * 
 *   A debugging function to shutdown the ORB and process
 * any reference count leaks that may have occured.
 * 
 * Return value: FALSE if there were leaks detected, else TRUE
 **/
gboolean
bonobo_activation_debug_shutdown (void)
{
        int retval = TRUE;
        CORBA_Environment ev;

        if (is_initialized) {
                CORBA_exception_init (&ev);

                bonobo_activation_base_service_debug_shutdown (&ev);
                if (ev._major != CORBA_NO_EXCEPTION) {
                        retval = FALSE;
                }

                if (bonobo_activation_context != CORBA_OBJECT_NIL) {
                        CORBA_Object_release (
                                (CORBA_Object) bonobo_activation_context, &ev);
                        bonobo_activation_context = CORBA_OBJECT_NIL;
                }

                bonobo_activation_release_corba_client ();

                if (object_directory != CORBA_OBJECT_NIL) {
                        CORBA_Object_release (object_directory, &ev);
                        object_directory = CORBA_OBJECT_NIL;
                }

                if (bonobo_activation_orb != CORBA_OBJECT_NIL) {
                        CORBA_ORB_destroy (bonobo_activation_orb, &ev);
                        if (ev._major != CORBA_NO_EXCEPTION) {
                                retval = FALSE;
                        }
                        CORBA_Object_release (
                                (CORBA_Object) bonobo_activation_orb, &ev);
                        is_initialized = FALSE;
                }

                CORBA_exception_free (&ev);

        }

        return retval;
}

const char  bonobo_activation_version []    = VERSION;
const guint bonobo_activation_major_version = BONOBO_ACTIVATION_MAJOR_VERSION;
const guint bonobo_activation_minor_version = BONOBO_ACTIVATION_MINOR_VERSION;
const guint bonobo_activation_micro_version = BONOBO_ACTIVATION_MICRO_VERSION;
