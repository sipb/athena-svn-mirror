/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  liboaf: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
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
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

/* This is part of the per-app CORBA bootstrapping - we use this to get 
   hold of a running metaserver and such */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <string.h>

#include "liboaf-private.h"
#include "oaf-i18n.h"
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

static GSList *reglocs = NULL;

typedef struct
{
	int priority;
	const OAFRegistrationLocation *regloc;
	gpointer user_data;
}
RegInfo;

typedef struct
{
	int priority;
	OAFServiceActivator act_func;
}
ActInfo;

static gint
ri_compare (gconstpointer a, gconstpointer b)
{
	const RegInfo *ra, *rb;

	ra = a;
	rb = b;

	return (rb->priority - ra->priority);
}

void
oaf_registration_location_add (const OAFRegistrationLocation * regloc,
			       int priority, gpointer user_data)
{
	RegInfo *new_ri;

	g_return_if_fail (regloc);

	new_ri = g_new (RegInfo, 1);
	new_ri->priority = priority;
	new_ri->regloc = regloc;
	new_ri->user_data = user_data;

	reglocs = g_slist_insert_sorted (reglocs, new_ri, ri_compare);
}

CORBA_Object
oaf_registration_check (const OAFRegistrationCategory * regcat,
			CORBA_Environment * ev)
{
	GSList *cur;
	CORBA_Object retval = CORBA_OBJECT_NIL;
	int dist = INT_MAX;
	char *ior = NULL;

	for (cur = reglocs; cur; cur = cur->next) {
		RegInfo *ri;
		char *new_ior;
		int new_dist = dist;

		ri = cur->data;

		if (!ri->regloc->check)
			continue;

		new_ior = ri->regloc->check (ri->regloc, regcat, 
                                             &new_dist, ri->user_data);
		if (new_ior && (new_dist < dist)) {
			g_free (ior);
			ior = new_ior;
		} else if (new_ior) {
			g_free (new_ior);
		}
	}

	if (ior) {
		retval = CORBA_ORB_string_to_object (oaf_orb_get (), ior, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			retval = CORBA_OBJECT_NIL;

		g_free (ior);
	}

	return retval;
}

/* dumb marshalling hack */
static void
oaf_registration_iterate (const OAFRegistrationCategory * regcat,
			  CORBA_Object obj, CORBA_Environment * ev,
			  gulong offset, int nargs)
{
	GSList *cur;
	char *ior = NULL;

	if (nargs == 4)
		ior = CORBA_ORB_object_to_string (oaf_orb_get (), obj, ev);

	for (cur = reglocs; cur; cur = cur->next) {
		RegInfo *ri;
		void (*func_ptr) ();

		ri = cur->data;

		func_ptr = *(gpointer *) ((guchar *) ri->regloc + offset);

		if (!func_ptr)
			continue;

		switch (nargs) {
		case 4:
			func_ptr (ri->regloc, ior, regcat, ri->user_data);
			break;
		case 2:
			func_ptr (ri->regloc, ri->user_data);
			break;
		}
	}

	if (nargs == 4)
		CORBA_free (ior);
}

static int lock_count = 0;

static void
oaf_reglocs_lock (CORBA_Environment * ev)
{
	if (lock_count == 0)
		oaf_registration_iterate (NULL, CORBA_OBJECT_NIL, ev,
					  G_STRUCT_OFFSET
					  (OAFRegistrationLocation, lock), 2);
	lock_count++;
}

static void
oaf_reglocs_unlock (CORBA_Environment * ev)
{
	lock_count--;
	if (lock_count == 0)
		oaf_registration_iterate (NULL, CORBA_OBJECT_NIL, ev,
					  G_STRUCT_OFFSET
					  (OAFRegistrationLocation, unlock),
					  2);
}

void
oaf_registration_unset (const OAFRegistrationCategory * regcat,
			CORBA_Object obj, CORBA_Environment * ev)
{
	oaf_reglocs_lock (ev);
	oaf_registration_iterate (regcat, obj, ev,
				  G_STRUCT_OFFSET (OAFRegistrationLocation,
						   unregister), 4);
	oaf_reglocs_unlock (ev);
}

void
oaf_registration_set (const OAFRegistrationCategory * regcat,
		      CORBA_Object obj, CORBA_Environment * ev)
{
	oaf_reglocs_lock (ev);
	oaf_registration_iterate (regcat, obj, ev,
				  G_STRUCT_OFFSET (OAFRegistrationLocation,
						   register_new), 4);
	oaf_reglocs_unlock (ev);
}

/* Whacked from gnome-libs/libgnorba/orbitns.c */

typedef struct
{
	GMainLoop *mloop;
	char iorbuf[2048];
#ifdef OAF_DEBUG
	char *do_srv_output;
#endif
	FILE *fh;
}
EXEActivateInfo;

static gboolean
handle_exepipe (GIOChannel * source,
		GIOCondition condition, EXEActivateInfo * data)
{
	gboolean retval = TRUE;

	*data->iorbuf = '\0';
	if (!(condition & G_IO_IN)
	    || !fgets (data->iorbuf, sizeof (data->iorbuf), data->fh))
		retval = FALSE;

	if (retval && !strncmp (data->iorbuf, "IOR:", 4))
		retval = FALSE;

#ifdef OAF_DEBUG
	if (data->do_srv_output)
		g_message ("srv output[%d]: '%s'", retval, data->iorbuf);
#endif

	if (!retval)
		g_main_quit (data->mloop);

	return retval;
}

#ifdef OAF_DEBUG
static void
print_exit_status (int status)
{
	if (WIFEXITED (status))
		g_message ("Exit status was %d", WEXITSTATUS (status));

	if (WIFSIGNALED (status))
		g_message ("signal was %d", WTERMSIG (status));
}
#endif

static 
void oaf_setenv (const char *name, const char *value) 
{
#if HAVE_SETENV
        setenv (name, value, 1);
#else
        char *tmp;
                
        tmp = g_strconcat (name, "=", value, NULL);
        
        putenv (tmp);
#endif
}

CORBA_Object
oaf_server_by_forking (const char **cmd, 
                       int fd_arg, 
                       const char *display,
		       const char *od_iorstr,
                       CORBA_Environment * ev)
{
	gint iopipes[2];
	CORBA_Object retval = CORBA_OBJECT_NIL;
	OAF_GeneralError *errval;
        FILE *iorfh;
        EXEActivateInfo ai;
        GIOChannel *gioc;
        int childpid;
        int status;
        guint watchid;
        struct sigaction sa;
        sigset_t mask, omask;
                
     	pipe (iopipes);

        /* Block SIGCHLD so no one else can wait() on the child before us. */
        sigemptyset (&mask);
        sigaddset (&mask, SIGCHLD);
        sigprocmask (SIG_BLOCK, &mask, &omask);

	/* fork & get the IOR from the magic pipe */
	childpid = fork ();

	if (childpid < 0) {
                sigprocmask (SIG_SETMASK, &omask, NULL);
		errval = OAF_GeneralError__alloc ();
		errval->description = CORBA_string_dup (_("Couldn't fork a new process"));

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_OAF_GeneralError, errval);
		return CORBA_OBJECT_NIL;
	}

	if (childpid) {
                /* de-zombify */
                while (waitpid (childpid, &status, 0) == -1 && errno == EINTR)
                        ;
                sigprocmask (SIG_SETMASK, &omask, NULL);
                
		if (!WIFEXITED (status)) {
			OAF_GeneralError *errval;
			char cbuf[512];
                        
			errval = OAF_GeneralError__alloc ();

			if (WIFSIGNALED (status))
				g_snprintf (cbuf, sizeof (cbuf),
					    _("Child received signal %u (%s)"),
					    WTERMSIG (status),
					    g_strsignal (WTERMSIG
                                                         (status)));
			else
				g_snprintf (cbuf, sizeof (cbuf),
					    _("Unknown non-exit error (status is %u)"),
					    status);
			errval->description = CORBA_string_dup (cbuf);
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_OAF_GeneralError, errval);
			return CORBA_OBJECT_NIL;
		}
#ifdef OAF_DEBUG
		ai.do_srv_output = getenv ("OAF_DEBUG_EXERUN");
                
		if (ai.do_srv_output)
			print_exit_status (status);
#endif
                
		close (iopipes[1]);
		ai.fh = iorfh = fdopen (iopipes[0], "r");
                
		ai.iorbuf[0] = '\0';
		ai.mloop = g_main_new (FALSE);
		gioc = g_io_channel_unix_new (iopipes[0]);
		watchid = g_io_add_watch (gioc,
                                          G_IO_IN | G_IO_HUP | G_IO_NVAL |
                                          G_IO_ERR, (GIOFunc) & handle_exepipe,
                                          &ai);
		g_io_channel_unref (gioc);
		g_main_run (ai.mloop);
		g_main_destroy (ai.mloop);
		fclose (iorfh);

		g_strstrip (ai.iorbuf);
		if (!strncmp (ai.iorbuf, "IOR:", 4)) {
			retval = CORBA_ORB_string_to_object (oaf_orb_get (),
                                                             ai.iorbuf, ev);
			if (ev->_major != CORBA_NO_EXCEPTION)
				retval = CORBA_OBJECT_NIL;
#ifdef OAF_DEBUG
			if (ai.do_srv_output)
				g_message ("Did string_to_object on %s",
					   ai.iorbuf);
#endif
		} else {
			OAF_GeneralError *errval;

#ifdef OAF_DEBUG
			if (ai.do_srv_output)
				g_message ("string doesn't match IOR:");
#endif

			errval = OAF_GeneralError__alloc ();
			errval->description = CORBA_string_dup (ai.iorbuf);
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_OAF_GeneralError, errval);
			retval = CORBA_OBJECT_NIL;
		}
	} else if ((childpid = fork ())) {
		_exit (0);	/* de-zombifier process, just exit */
	} else {
                if (display)
		  oaf_setenv ("DISPLAY", display);
		if (od_iorstr)
		  oaf_setenv ("OAF_OD_IOR", od_iorstr);
                

		close (iopipes[0]);
                
                if (fd_arg != 0) {
                        cmd[fd_arg] = g_strdup_printf (cmd[fd_arg], iopipes[1]);
                }

		setsid ();
		memset (&sa, 0, sizeof (sa));
		sa.sa_handler = SIG_IGN;
		sigaction (SIGPIPE, &sa, 0);

		execvp (cmd[0], (char **) cmd);
		if (iopipes[1] != 1)
			dup2 (iopipes[1], 1);
		g_print (_("Exec failed: %d (%s)\n"), errno,
			 g_strerror (errno));
		_exit (1);
	}

	return retval;
}

const char *oaf_ac_cmd[] =
	{ "oafd", "--ac-activate", "--ior-output-fd=%d", NULL };
const char *oaf_od_cmd[] = { "oafd", "--ior-output-fd=%d", NULL };

struct SysServerInstance
{
	CORBA_Object already_running;
	char *username, *hostname, *domain;
};

struct SysServer
{
	const char *name;
	const char **cmd;
	int fd_arg;
	GSList *instances;
}
activatable_servers[] =
{
	{"IDL:OAF/ActivationContext:1.0", (const char **) oaf_ac_cmd,
         2, CORBA_OBJECT_NIL}, 
        {"IDL:OAF/ObjectDirectory:1.0", (const char **) oaf_od_cmd,
         1, CORBA_OBJECT_NIL},
	{ NULL}
};

#define STRMATCH(x, y) ((!x && !y) || (x && y && !strcmp(x, y)))
static CORBA_Object
existing_check (const OAFRegistrationCategory * regcat, struct SysServer *ss)
{
	GSList *cur;

	for (cur = ss->instances; cur; cur = cur->next) {
		struct SysServerInstance *ssi;

		ssi = cur->data;
		if (
		    (!ssi->username
		     || STRMATCH (ssi->username, regcat->username))
		    && (!ssi->hostname
			|| STRMATCH (ssi->hostname, regcat->hostname))
		    && (!ssi->domain
			|| STRMATCH (ssi->domain,
				     regcat->
				     domain))) {
                        return ssi->already_running;
                }
	}

	return CORBA_OBJECT_NIL;
}

static void
oaf_existing_set (const OAFRegistrationCategory * regcat, struct SysServer *ss,
	          CORBA_Object obj, CORBA_Environment * ev)
{
	GSList *cur;
	struct SysServerInstance *ssi;

        ssi = NULL;

	for (cur = ss->instances; cur; cur = cur->next) {
		ssi = cur->data;
		if (
		    (!ssi->username
		     || STRMATCH (ssi->username, regcat->username))
		    && (!ssi->hostname
			|| STRMATCH (ssi->hostname, regcat->hostname))
		    && (!ssi->domain
			|| STRMATCH (ssi->domain, regcat->domain))) break;
	}

	if (cur == NULL) {
		ssi = g_new0 (struct SysServerInstance, 1);
		ssi->already_running = obj;
		ssi->username =
			regcat->username ? g_strdup (regcat->username) : NULL;
		ssi->hostname =
			regcat->hostname ? g_strdup (regcat->hostname) : NULL;
		ssi->domain =
			regcat->domain ? g_strdup (regcat->domain) : NULL;
                ss->instances = g_slist_prepend (ss->instances, ssi);
	} else {
		CORBA_Object_release (ssi->already_running, ev);
		ssi->already_running = obj;
	}
}

static GSList *activator_list = NULL;

static gint
ai_compare (gconstpointer a, gconstpointer b)
{
	const ActInfo *ra, *rb;

	ra = a;
	rb = b;

	return (rb->priority - ra->priority);
}

void
oaf_registration_activator_add (OAFServiceActivator act_func, int priority)
{
	ActInfo *new_act;

	new_act = g_new (ActInfo, 1);
	new_act->priority = priority;
	new_act->act_func = act_func;
	activator_list =
		g_slist_insert_sorted (activator_list, new_act, ai_compare);
}

static CORBA_Object
oaf_activators_use (const OAFRegistrationCategory * regcat, const char **cmd,
		    int fd_arg, CORBA_Environment * ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	GSList *cur;

	for (cur = activator_list; CORBA_Object_is_nil (retval, ev) && cur;
	     cur = cur->next) {
		ActInfo *actinfo;
		actinfo = cur->data;

		retval = actinfo->act_func (regcat, cmd, fd_arg, ev);
	}

	return retval;
}

CORBA_Object
oaf_service_get (const OAFRegistrationCategory * regcat)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	int i;
	CORBA_Environment *ev, myev;
	gboolean ne;

	g_return_val_if_fail (regcat, CORBA_OBJECT_NIL);

	for (i = 0; activatable_servers[i].name; i++) {
		if (!strcmp (regcat->name, activatable_servers[i].name))
			break;
	}

	if (!activatable_servers[i].name)
		return retval;

	CORBA_exception_init (&myev);
	ev = &myev;
	retval = existing_check (regcat, &activatable_servers[i]);
	if (!CORBA_Object_non_existent (retval, ev))
		goto out;

	oaf_reglocs_lock (ev);

	retval = oaf_registration_check (regcat, &myev);
	ne = CORBA_Object_non_existent (retval, &myev);
	if (ne) {
		CORBA_Object race_condition;

		CORBA_Object_release (retval, &myev);

		oaf_reglocs_unlock (ev);	/* The activator may want to do Fancy Stuff, and 
                                                 * having the X server grabbed at this time
						 * is Broken (tm) */
		retval =
			oaf_activators_use (regcat,
					    activatable_servers[i].cmd,
					    activatable_servers[i].fd_arg,
					    ev);
		oaf_reglocs_lock (ev);

		race_condition = oaf_registration_check (regcat, &myev);

		if (!CORBA_Object_non_existent (race_condition, &myev)) {
			CORBA_Object_release (retval, &myev);
			retval = race_condition;
		} else if (!CORBA_Object_is_nil (retval, &myev))
			oaf_registration_set (regcat, retval, &myev);
	}

	oaf_reglocs_unlock (ev);

	if (!CORBA_Object_non_existent (retval, ev))
		oaf_existing_set (regcat, &activatable_servers[i], retval, ev);

      out:
	CORBA_exception_free (&myev);

	return retval;
}

/***** Implementation of the IOR registration system via plain files ******/
static int lock_fd = -1;

static void
rloc_file_lock (const OAFRegistrationLocation * regloc, gpointer user_data)
{
	char *fn;
	struct flock lockme;

	fn = oaf_alloca (sizeof ("/tmp/orbit-%s/oaf-register.lock") + 32);
	sprintf (fn, "/tmp/orbit-%s/oaf-register.lock", g_get_user_name ());

	while ((lock_fd = open (fn, O_CREAT | O_RDONLY, 0700)) < 0) {
		if (errno == EEXIST) {
#ifdef HAVE_USLEEP
			usleep (10000);
#elif defined(HAVE_NANOSLEEP)
			{
				struct timespec timewait;
				timewait.tv_sec = 0;
				timewait.tv_nsec = 1000000;
				nanosleep (&timewait, NULL);
			}
#else
			sleep (1);
#endif
		} else
			break;
	}

	fcntl (lock_fd, F_SETFD, FD_CLOEXEC);

	if (lock_fd >= 0) {
		lockme.l_type = F_RDLCK;
		lockme.l_whence = SEEK_SET;
		lockme.l_start = 0;
		lockme.l_len = 1;
		lockme.l_pid = getpid ();

		while (fcntl (lock_fd, F_SETLKW, &lockme) < 0
		       && errno == EINTR) /**/;
	}
}

static void
rloc_file_unlock (const OAFRegistrationLocation * regloc, gpointer user_data)
{
#if 0
	char *fn;


	fn = oaf_alloca (sizeof ("/tmp/orbit-%s/oaf-register.lock") + 32);
	sprintf (fn, "/tmp/orbit-%s/oaf-register.lock", g_get_user_name ());

	unlink (fn);
#endif

	if (lock_fd >= 0) {
		struct flock lockme;

		lockme.l_type = F_UNLCK;
		lockme.l_whence = SEEK_SET;
		lockme.l_start = 0;
		lockme.l_len = 1;
		lockme.l_pid = getpid ();

		fcntl (lock_fd, F_SETLKW, &lockme);
		close (lock_fd);
		lock_fd = -1;
	}
}

static void
filename_fixup (char *fn)
{
	while (*(fn++)) {
		if (*fn == '/')
			*fn = '_';
	}
}

static char *
rloc_file_check (const OAFRegistrationLocation * regloc,
		 const OAFRegistrationCategory * regcat, int *ret_distance,
		 gpointer user_data)
{
	FILE *fh;
	char fn[PATH_MAX], *uname;
	char *namecopy;

	namecopy = oaf_alloca (strlen (regcat->name) + 1);
	strcpy (namecopy, regcat->name);
	filename_fixup (namecopy);

	uname = g_get_user_name ();

	sprintf (fn, "/tmp/orbit-%s/reg.%s-%s",
		 uname,
		 namecopy,
		 regcat->session_name ? regcat->session_name : "local");
	fh = fopen (fn, "r");
	if (fh)
		goto useme;

	sprintf (fn, "/tmp/orbit-%s/reg.%s", uname, namecopy);
	fh = fopen (fn, "r");
	if (fh)
		goto useme;

	useme:
	if (fh) {
		char iorbuf[8192];

		iorbuf[0] = '\0';
		while (fgets (iorbuf, sizeof (iorbuf), fh)
		       && strncmp (iorbuf, "IOR:", 4))
			/**/;
		g_strstrip (iorbuf);

		fclose (fh);

		if (!strncmp (iorbuf, "IOR:", 4)) {
			*ret_distance = 0;
			return g_strdup (iorbuf);
		}
	}

	return NULL;
}

static void
rloc_file_register (const OAFRegistrationLocation * regloc, const char *ior,
		    const OAFRegistrationCategory * regcat,
		    gpointer user_data)
{
	char fn[PATH_MAX], fn2[PATH_MAX], *uname;
	FILE *fh;
	char *namecopy;

	namecopy = oaf_alloca (strlen (regcat->name) + 1);
	strcpy (namecopy, regcat->name);
	filename_fixup (namecopy);

	uname = g_get_user_name ();

	sprintf (fn, "/tmp/orbit-%s/reg.%s-%s",
		 uname,
		 namecopy,
		 regcat->session_name ? regcat->session_name : "local");

	sprintf (fn2, "/tmp/orbit-%s/reg.%s", uname, namecopy);

	fh = fopen (fn, "w");
	fprintf (fh, "%s\n", ior);
	fclose (fh);

	symlink (fn, fn2);
}

static void
rloc_file_unregister (const OAFRegistrationLocation * regloc, const char *ior,
		      const OAFRegistrationCategory * regcat,
		      gpointer user_data)
{
	char fn2[PATH_MAX], fn3[PATH_MAX];
	char fn[PATH_MAX];
	char *uname;
	char *namecopy;

	namecopy = oaf_alloca (strlen (regcat->name) + 1);
	strcpy (namecopy, regcat->name);
	filename_fixup (namecopy);

	uname = g_get_user_name ();

	sprintf (fn, "/tmp/orbit-%s/reg.%s-%s",
		 uname,
		 namecopy,
		 regcat->session_name ? regcat->session_name : "local");
	unlink (fn);

	sprintf (fn2, "/tmp/orbit-%s/reg.%s", uname, namecopy);

	if (readlink (fn2, fn3, sizeof (fn3) < 0))
		return;

	if (!strcmp (fn3, fn))
		unlink (fn2);
}

static const OAFRegistrationLocation rloc_file = {
	rloc_file_lock,
	rloc_file_unlock,
	rloc_file_check,
	rloc_file_register,
	rloc_file_unregister
};

void
oaf_rloc_file_register (void)
{
	oaf_registration_location_add (&rloc_file, 0, NULL);
}





