/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-mime-handlers.c - Mime type handlers for the GNOME Virtual
   File System.

   Copyright (C) 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Maciej Stachowiak <mjs@eazel.com> */

#include <config.h>
#include "gnome-vfs-mime-handlers.h"

#include "eggdesktopentries.h"
#include "gnome-vfs-application-registry.h"
#include "gnome-vfs-mime-info.h"
#include "gnome-vfs-mime-info-cache.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-mime-private.h"
#include "gnome-vfs-result.h"
#include "gnome-vfs-private-utils.h"
#include "gnome-vfs-utils.h"
#include "gnome-vfs-i18n.h"
#include <bonobo-activation/bonobo-activation-activate.h>
#include <gconf/gconf-client.h>
#include <stdio.h>
#include <string.h>

#define GCONF_DEFAULT_VIEWER_EXEC_PATH   "/desktop/gnome/applications/component_viewer/exec"

extern GList * _gnome_vfs_configuration_get_methods_list (void);

static GnomeVFSResult expand_parameters                          (gpointer                  action,
								  GnomeVFSMimeActionType    type,
								  GList                    *uris,
								  int                      *argc,
								  char                   ***argv);
static GList *        Bonobo_ServerInfoList_to_ServerInfo_g_list (Bonobo_ServerInfoList    *info_list);
static GList *        copy_str_list                              (GList                    *string_list);


/**
 * gnome_vfs_mime_get_description:
 * @mime_type: the mime type
 *
 * Query the MIME database for a description of the specified MIME type.
 *
 * Return value: A description of MIME type @mime_type
 */
const char *
gnome_vfs_mime_get_description (const char *mime_type)
{
	return gnome_vfs_mime_get_value (mime_type, "description");
}

/**
 * gnome_vfs_mime_set_description:
 * @mime_type: A const char * containing a mime type
 * @description: A description of this MIME type
 * 
 * Set the description of this MIME type in the MIME database. The description
 * should be something like "Gnumeric spreadsheet".
 * 
 * Return value: GnomeVFSResult indicating the success of the operation or any
 * errors that may have occurred.
 **/
GnomeVFSResult
gnome_vfs_mime_set_description (const char *mime_type, const char *description)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_get_default_action_type:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * 
 * Query the MIME database for the type of action to be performed on a particular MIME type by default.
 * 
 * Return value: The type of action to be performed on a file of 
 * MIME type, @mime_type by default.
 **/
GnomeVFSMimeActionType
gnome_vfs_mime_get_default_action_type (const char *mime_type)
{
	const char *action_type_string;

	action_type_string = gnome_vfs_mime_get_value (mime_type, "default_action_type");

	if (action_type_string != NULL && g_ascii_strcasecmp (action_type_string, "application") == 0) {
		return GNOME_VFS_MIME_ACTION_TYPE_APPLICATION;
	} else if (action_type_string != NULL && g_ascii_strcasecmp (action_type_string, "component") == 0) {
		return GNOME_VFS_MIME_ACTION_TYPE_COMPONENT;
	} else {
		return GNOME_VFS_MIME_ACTION_TYPE_NONE;
	}

}

/**
 * gnome_vfs_mime_get_default_action:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * 
 * Query the MIME database for default action associated with a particular MIME type @mime_type.
 * 
 * Return value: A GnomeVFSMimeAction representing the default action to perform upon
 * file of type @mime_type.
 **/
GnomeVFSMimeAction *
gnome_vfs_mime_get_default_action (const char *mime_type)
{
	GnomeVFSMimeAction *action;

	action = g_new0 (GnomeVFSMimeAction, 1);

	action->action_type = gnome_vfs_mime_get_default_action_type (mime_type);

	switch (action->action_type) {
	case GNOME_VFS_MIME_ACTION_TYPE_APPLICATION:
		action->action.application = 
			gnome_vfs_mime_get_default_application (mime_type);
		if (action->action.application == NULL) {
			g_free (action);
			action = NULL;
		}
		break;
	case GNOME_VFS_MIME_ACTION_TYPE_COMPONENT:
		action->action.component = 
			gnome_vfs_mime_get_default_component (mime_type);
		if (action->action.component == NULL) {
			g_free (action);
			action = NULL;
		}
		break;
	case GNOME_VFS_MIME_ACTION_TYPE_NONE:
		g_free (action);
		action = NULL;
		break;
	default:
		g_assert_not_reached ();
	}

	return action;
}

static gboolean
mime_application_supports_uri_scheme (GnomeVFSMimeApplication *application,
				      const char *uri_scheme)
{
	/* The default supported uri scheme is "file" */
	if (application->supported_uri_schemes == NULL
	    && g_ascii_strcasecmp ((const char *) uri_scheme, "file") == 0) {
		return TRUE;
	}
	return g_list_find_custom (application->supported_uri_schemes,
				   uri_scheme,
				   (GCompareFunc)g_ascii_strcasecmp) != NULL;
}


/* This should be public, but we're in an API freeze */
GnomeVFSMimeApplication *
gnome_vfs_mime_get_default_application_for_scheme (const char *mime_type,
						   const char *uri_scheme)
{
	char *default_application_id;
	GnomeVFSMimeApplication *default_application;

	default_application = NULL;

	/* First, try the default for the mime type */
	default_application_id = gnome_vfs_mime_get_default_desktop_entry (mime_type);

	if (default_application_id != NULL
	    && default_application_id[0] != '\0') {
		default_application =
			gnome_vfs_mime_application_new_from_id (default_application_id);
		g_free (default_application_id);
	}

	if (default_application != NULL &&
	    !mime_application_supports_uri_scheme (default_application, uri_scheme)) {
		gnome_vfs_mime_application_free (default_application);
		default_application = NULL;
	}
	
	if (default_application == NULL) {
		GList *all_applications, *l;
			
		/* Failing that, try something from the complete list */
		all_applications = gnome_vfs_mime_get_all_desktop_entries (mime_type);

		for (l = all_applications; l != NULL; l = l->next) {
			default_application = 
				gnome_vfs_mime_application_new_from_id ((char *) l->data);
			
			if (default_application != NULL &&
			    !mime_application_supports_uri_scheme (default_application, uri_scheme)) {
				gnome_vfs_mime_application_free (default_application);
				default_application = NULL;
			}
			if (default_application != NULL)
				break;
		}
		g_list_foreach (all_applications, (GFunc) g_free, NULL);
		g_list_free (all_applications);
	}

	return default_application;
}


/**
 * gnome_vfs_mime_get_default_application:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Query the MIME database for the application to be executed on files of MIME type
 * @mime_type by default.
 * 
 * Return value: A GnomeVFSMimeApplication representing the default handler of @mime_type
 **/
GnomeVFSMimeApplication *
gnome_vfs_mime_get_default_application (const char *mime_type)
{
	char *default_application_id;
	GnomeVFSMimeApplication *default_application;
	GList *list;

	default_application = NULL;

	/* First, try the default for the mime type */
	default_application_id = gnome_vfs_mime_get_default_desktop_entry (mime_type);

	if (default_application_id != NULL
	    && default_application_id[0] != '\0') {
		default_application =
			gnome_vfs_mime_application_new_from_id (default_application_id);
		g_free (default_application_id);
	}

	if (default_application == NULL) {

		/* Failing that, try something from the complete list */
		list = gnome_vfs_mime_get_all_desktop_entries (mime_type);

		if (list != NULL) {
			default_application = 
				gnome_vfs_mime_application_new_from_id ((char *) list->data);

			g_list_foreach (list, (GFunc) g_free, NULL);
			g_list_free (list);
		}
	}

	return default_application;
}

/**
 * gnome_vfs_mime_get_icon:
 * @mime_type: A const char * containing a  MIME type
 *
 * Query the MIME database for an icon representing the specified MIME type.
 *
 * Return value: The filename of the icon as listed in the MIME database. This is
 * usually a filename without path information, e.g. "i-chardev.png", and sometimes
 * does not have an extension, e.g. "i-regular" if the icon is supposed to be image
 * type agnostic between icon themes. Icons are generic, and not theme specific. These
 * will not necessarily match with the icons a user sees in Nautilus, you have been warned.
 */
const char *
gnome_vfs_mime_get_icon (const char *mime_type)
{
	return gnome_vfs_mime_get_value (mime_type, "icon_filename");
}

/**
 * gnome_vfs_mime_set_icon:
 * @mime_type: A const char * containing a  MIME type
 * @filename: a const char * containing an image filename
 *
 * Set the icon entry for a particular MIME type in the MIME database. Note that
 * icon entries need not necessarily contain the full path, and do not necessarily need to
 * specify an extension. So "i-regular", "my-special-icon.png", and "some-icon"
 * are all valid icon filenames.
 *
 * Return value: A GnomeVFSResult indicating the success of the operation
 * or any errors that may have occurred.
 */
GnomeVFSResult
gnome_vfs_mime_set_icon (const char *mime_type, const char *filename)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}


/**
 * gnome_vfs_mime_can_be_executable:
 * @mime_type: A const char * containing a mime type
 * 
 * Check whether files of this MIME type might conceivably be executable.
 * Default for known types if FALSE. Default for unknown types is TRUE.
 * 
 * Return value: gboolean containing TRUE if some files of this MIME type
 * are registered as being executable, and false otherwise.
 **/
gboolean
gnome_vfs_mime_can_be_executable (const char *mime_type)
{
	const char *result_as_string;
	gboolean result;
	
	result_as_string = gnome_vfs_mime_get_value (mime_type, "can_be_executable");
	if (result_as_string != NULL) {
		result = strcmp (result_as_string, "TRUE") == 0;
	} else {
		/* If type is not known, we treat it as potentially executable.
		 * If type is known, we use default value of not executable.
		 */
		result = !gnome_vfs_mime_type_is_known (mime_type);

		if (!strncmp (mime_type, "x-directory", strlen ("x-directory"))) {
			result = FALSE;
		}
	}
	
	return result;
}

/**
 * gnome_vfs_mime_set_can_be_executable:
 * @mime_type: A const char * containing a mime type
 * @new_value: A boolean value indicating whether @mime_type could be executable.
 * 
 * Set whether files of this MIME type might conceivably be executable.
 * 
 * Return value: GnomeVFSResult indicating the success of the operation or any
 * errors that may have occurred.
 **/
GnomeVFSResult
gnome_vfs_mime_set_can_be_executable (const char *mime_type, gboolean new_value)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_get_default_component:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Query the MIME database for the default Bonobo component to be activated to 
 * view files of MIME type @mime_type.
 * 
 * Return value: An Bonobo_ServerInfo * representing the OAF server to be activated
 * to get a reference to the proper component.
 **/
Bonobo_ServerInfo *
gnome_vfs_mime_get_default_component (const char *mime_type)
{
	const char *default_component_iid;
	Bonobo_ServerInfoList *info_list;
	Bonobo_ServerInfo *default_component;
	CORBA_Environment ev;
	char *supertype;
	char *query;
	char *sort[5];

	if (mime_type == NULL) {
		return NULL;
	}

	CORBA_exception_init (&ev);

	supertype = gnome_vfs_get_supertype_from_mime_type (mime_type);

	/* Find a component that supports either the exact mime type,
           the supertype, or all mime types. */

	/* First try the component specified in the mime database, if available. 
	   gnome_vfs_mime_get_value looks up the value for the mime type and the supertype.  */
	default_component_iid = gnome_vfs_mime_get_value
		(mime_type, "default_component_iid");

	query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
			     "', '", supertype,
			     "', '*'])", NULL);


	if (default_component_iid != NULL) {
		sort[0] = g_strconcat ("iid == '", default_component_iid, "'", NULL);
	} else {
		sort[0] = g_strdup ("true");
	}

	/* Prefer something that matches the exact type to something
           that matches the supertype */
	sort[1] = g_strconcat ("bonobo:supported_mime_types.has ('", mime_type, "')", NULL);

	/* Prefer something that matches the supertype to something that matches `*' */
	sort[2] = g_strconcat ("bonobo:supported_mime_types.has ('", supertype, "')", NULL);

	sort[3] = g_strdup ("name");
	sort[4] = NULL;

	info_list = bonobo_activation_query (query, sort, &ev);
	
	default_component = NULL;
	if (ev._major == CORBA_NO_EXCEPTION) {
		if (info_list != NULL && info_list->_length > 0) {
			default_component = Bonobo_ServerInfo_duplicate (&info_list->_buffer[0]);
		}
		CORBA_free (info_list);
	}

	g_free (supertype);
	g_free (query);
	g_free (sort[0]);
	g_free (sort[1]);
	g_free (sort[2]);
	g_free (sort[3]);

	CORBA_exception_free (&ev);

	return default_component;
}

/**
 * gnome_vfs_mime_get_short_list_applications:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Return an alphabetically sorted list of GnomeVFSMimeApplication data
 * structures for the requested mime type. GnomeVFS no longer supports the
 * concept of a "short list" of applications that the user might be interested
 * in.
 * 
 * Return value: A GList * where the elements are GnomeVFSMimeApplication *
 * representing various applications to display in the short list for @mime_type.
 *
 * @Deprecated: Use gnome_vfs_mime_get_all_applications() instead.
**/ 
GList *
gnome_vfs_mime_get_short_list_applications (const char *mime_type)
{
	return gnome_vfs_mime_get_all_applications (mime_type);
}


/**
 * gnome_vfs_mime_get_short_list_components:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Return an unsorted sorted list of Bonobo_ServerInfo * data structures for the
 * requested mime type.  GnomeVFS no longer supports the concept of a "short
 * list" of applications that the user might be interested in.
 * 
 * Return value: A GList * where the elements are Bonobo_ServerInfo *
 * representing various components to display in the short list for @mime_type.
 *
 * @Deprecated: Use gnome_vfs_mime_get_all_components() instead.
 **/ 
GList *
gnome_vfs_mime_get_short_list_components (const char *mime_type)
{
	Bonobo_ServerInfoList *info_list;
	GList *components_list;
	CORBA_Environment ev;
	char *supertype;
	char *query;
	char *sort[4];

	if (mime_type == NULL) {
		return NULL;
	}

	CORBA_exception_init (&ev);

	/* Find a component that supports either the exact mime type,
           the supertype, or all mime types. */

	/* FIXME bugzilla.eazel.com 1142: should probably check for
           the right interfaces too. Also slightly semantically
           different from nautilus in other tiny ways.
	*/
	supertype = gnome_vfs_get_supertype_from_mime_type (mime_type);
	query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
			     "', '", supertype,
			     "', '*'])", NULL);
	g_free (supertype);
	
        /* Prefer something that matches the exact type to something
           that matches the supertype */
	sort[0] = g_strconcat ("bonobo:supported_mime_types.has ('", mime_type, "')", NULL);

	/* Prefer something that matches the supertype to something that matches `*' */
	sort[1] = g_strconcat ("bonobo:supported_mime_types.has ('", supertype, "')", NULL);

	sort[2] = g_strdup ("name");
	sort[3] = NULL;

	info_list = bonobo_activation_query (query, sort, &ev);
	
	if (ev._major == CORBA_NO_EXCEPTION) {
		components_list = Bonobo_ServerInfoList_to_ServerInfo_g_list (info_list);
		CORBA_free (info_list);
	} else {
		components_list = NULL;
	}

	g_free (query);
	g_free (sort[0]);
	g_free (sort[1]);
	g_free (sort[2]);

	CORBA_exception_free (&ev);

	return components_list;
}


/**
 * gnome_vfs_mime_get_all_applications:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Return an alphabetically sorted list of GnomeVFSMimeApplication
 * data structures representing all applications in the MIME database registered
 * to handle files of MIME type @mime_type (and supertypes).
 * 
 * Return value: A GList * where the elements are GnomeVFSMimeApplication *
 * representing applications that handle MIME type @mime_type.
 **/ 
GList *
gnome_vfs_mime_get_all_applications (const char *mime_type)
{
	GList *applications, *node, *next;
	char *application_id;
	GnomeVFSMimeApplication *application;

	g_return_val_if_fail (mime_type != NULL, NULL);

	applications = gnome_vfs_mime_get_all_desktop_entries (mime_type);

	/* Convert to GnomeVFSMimeApplication's (leaving out NULLs) */
	for (node = applications; node != NULL; node = next) {
		next = node->next;

		application_id = node->data;
		application = gnome_vfs_mime_application_new_from_id (application_id);

		/* Replace the application ID with the application */
		if (application == NULL) {
			applications = g_list_remove_link (applications, node);
			g_list_free_1 (node);
		} else {
			node->data = application;
		}

		g_free (application_id);
	}

	return applications;
}

/**
 * gnome_vfs_mime_get_all_components:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Return an alphabetically sorted list of Bonobo_ServerInfo
 * data structures representing all Bonobo components registered
 * to handle files of MIME type @mime_type (and supertypes).
 * 
 * Return value: A GList * where the elements are Bonobo_ServerInfo *
 * representing components that can handle MIME type @mime_type.
 **/ 
GList *
gnome_vfs_mime_get_all_components (const char *mime_type)
{
	Bonobo_ServerInfoList *info_list;
	GList *components_list;
	CORBA_Environment ev;
	char *supertype;
	char *query;
	char *sort[2];

	if (mime_type == NULL) {
		return NULL;
	}

	CORBA_exception_init (&ev);

	/* Find a component that supports either the exact mime type,
           the supertype, or all mime types. */

	/* FIXME bugzilla.eazel.com 1142: should probably check for
           the right interfaces too. Also slightly semantically
           different from nautilus in other tiny ways.
	*/
	supertype = gnome_vfs_get_supertype_from_mime_type (mime_type);
	query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
			     "', '", supertype,
			     "', '*'])", NULL);
	g_free (supertype);
	
	/* Alphebetize by name, for the sake of consistency */
	sort[0] = g_strdup ("name");
	sort[1] = NULL;

	info_list = bonobo_activation_query (query, sort, &ev);
	
	if (ev._major == CORBA_NO_EXCEPTION) {
		components_list = Bonobo_ServerInfoList_to_ServerInfo_g_list (info_list);
		CORBA_free (info_list);
	} else {
		components_list = NULL;
	}

	g_free (query);
	g_free (sort[0]);

	CORBA_exception_free (&ev);

	return components_list;
}
/**
 * gnome_vfs_mime_set_default_action_type:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * @action_type: A GnomeVFSMimeActionType containing the action to perform by default
 * 
 * Sets the default action type to be performed on files of MIME type @mime_type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_default_action_type (const char *mime_type,
					GnomeVFSMimeActionType action_type)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_set_default_application:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_id: A key representing an application in the MIME database 
 * (GnomeVFSMimeApplication->id, for example)
 * 
 * Sets the default application to be run on files of MIME type @mime_type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_default_application (const char *mime_type,
					const char *application_id)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_set_default_component:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @component_iid: The OAFIID of a component
 * 
 * Sets the default component to be activated for files of MIME type @mime_type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_default_component (const char *mime_type,
				      const char *component_iid)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_set_short_list_applications:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_ids: GList of const char * application ids
 * 
 * Set the short list of applications for the specified MIME type. The short list
 * contains applications recommended for possible selection by the user.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_short_list_applications (const char *mime_type,
					    GList *application_ids)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_set_short_list_components:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @component_iids: GList of const char * OAF IIDs
 * 
 * Set the short list of components for the specified MIME type. The short list
 * contains companents recommended for possible selection by the user. * 
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_short_list_components (const char *mime_type,
					  GList *component_iids)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/* FIXME bugzilla.eazel.com 1148: 
 * The next set of helper functions are all replicated in nautilus-mime-actions.c.
 * Need to refactor so they can share code.
 */
static gint
gnome_vfs_mime_application_has_id (GnomeVFSMimeApplication *application, const char *id)
{
	return strcmp (application->id, id);
}

static gint
gnome_vfs_mime_id_matches_application (const char *id, GnomeVFSMimeApplication *application)
{
	return gnome_vfs_mime_application_has_id (application, id);
}

static gint
gnome_vfs_mime_id_matches_component (const char *iid, Bonobo_ServerInfo *component)
{
	return strcmp (component->iid, iid);
}

static gint 
gnome_vfs_mime_application_matches_id (GnomeVFSMimeApplication *application, const char *id)
{
	return gnome_vfs_mime_id_matches_application (id, application);
}

static gint 
gnome_vfs_mime_component_matches_id (Bonobo_ServerInfo *component, const char *iid)
{
	return gnome_vfs_mime_id_matches_component (iid, component);
}

/**
 * gnome_vfs_mime_id_in_application_list:
 * @id: An application id.
 * @applications: A GList * whose nodes are GnomeVFSMimeApplications, such as the
 * result of gnome_vfs_mime_get_short_list_applications().
 * 
 * Check whether an application id is in a list of GnomeVFSMimeApplications.
 * 
 * Return value: TRUE if an application whose id matches @id is in @applications.
 */
gboolean
gnome_vfs_mime_id_in_application_list (const char *id, GList *applications)
{
	return g_list_find_custom
		(applications, (gpointer) id,
		 (GCompareFunc) gnome_vfs_mime_application_matches_id) != NULL;
}

/**
 * gnome_vfs_mime_id_in_component_list:
 * @iid: A component iid.
 * @components: A GList * whose nodes are Bonobo_ServerInfos, such as the
 * result of gnome_vfs_mime_get_short_list_components().
 * 
 * Check whether a component iid is in a list of Bonobo_ServerInfos.
 * 
 * Return value: TRUE if a component whose iid matches @iid is in @components.
 */
gboolean
gnome_vfs_mime_id_in_component_list (const char *iid, GList *components)
{
	return g_list_find_custom
		(components, (gpointer) iid,
		 (GCompareFunc) gnome_vfs_mime_component_matches_id) != NULL;
	return FALSE;
}

/**
 * gnome_vfs_mime_id_list_from_application_list:
 * @applications: A GList * whose nodes are GnomeVFSMimeApplications, such as the
 * result of gnome_vfs_mime_get_short_list_applications().
 * 
 * Create a list of application ids from a list of GnomeVFSMimeApplications.
 * 
 * Return value: A new list where each GnomeVFSMimeApplication in the original
 * list is replaced by a char * with the application's id. The original list is
 * not modified.
 */
GList *
gnome_vfs_mime_id_list_from_application_list (GList *applications)
{
	GList *result;
	GList *node;

	result = NULL;
	
	for (node = applications; node != NULL; node = node->next) {
		result = g_list_append 
			(result, g_strdup (((GnomeVFSMimeApplication *)node->data)->id));
	}

	return result;
}

/**
 * gnome_vfs_mime_id_list_from_component_list:
 * @components: A GList * whose nodes are Bonobo_ServerInfos, such as the
 * result of gnome_vfs_mime_get_short_list_components().
 * 
 * Create a list of component iids from a list of Bonobo_ServerInfos.
 * 
 * Return value: A new list where each Bonobo_ServerInfo in the original
 * list is replaced by a char * with the component's iid. The original list is
 * not modified.
 */
GList *
gnome_vfs_mime_id_list_from_component_list (GList *components)
{
	GList *list = NULL;
	GList *node;

	for (node = components; node != NULL; node = node->next) {
		list = g_list_prepend 
			(list, g_strdup (((Bonobo_ServerInfo *)node->data)->iid));
	}
	return g_list_reverse (list);
}

/**
 * gnome_vfs_mime_add_application_to_short_list:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_id: const char * containing the application's id in the MIME database
 * 
 * Add an application to the short list for MIME type @mime_type. The short list contains
 * applications recommended for display as choices to the user for a particular MIME type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_add_application_to_short_list (const char *mime_type,
					      const char *application_id)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_remove_application_from_list:
 * @applications: A GList * whose nodes are GnomeVFSMimeApplications, such as the
 * result of gnome_vfs_mime_get_short_list_applications().
 * @application_id: The id of an application to remove from @applications.
 * @did_remove: If non-NULL, this is filled in with TRUE if the application
 * was found in the list, FALSE otherwise.
 * 
 * Remove an application specified by id from a list of GnomeVFSMimeApplications.
 * 
 * Return value: The modified list. If the application is not found, the list will 
 * be unchanged.
 */
GList *
gnome_vfs_mime_remove_application_from_list (GList *applications, 
					     const char *application_id,
					     gboolean *did_remove)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return NULL;
}

/**
 * gnome_vfs_mime_remove_application_from_short_list:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_id: const char * containing the application's id in the MIME database
 * 
 * Remove an application from the short list for MIME type @mime_type. The short list contains
 * applications recommended for display as choices to the user for a particular MIME type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_remove_application_from_short_list (const char *mime_type,
						   const char *application_id)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_add_component_to_short_list:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @iid: const char * containing the component's OAF IID
 * 
 * Add a component to the short list for MIME type @mime_type. The short list contains
 * components recommended for display as choices to the user for a particular MIME type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_add_component_to_short_list (const char *mime_type,
					    const char *iid)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_remove_component_from_list:
 * @components: A GList * whose nodes are Bonobo_ServerInfos, such as the
 * result of gnome_vfs_mime_get_short_list_components().
 * @iid: The iid of a component to remove from @components.
 * @did_remove: If non-NULL, this is filled in with TRUE if the component
 * was found in the list, FALSE otherwise.
 * 
 * Remove a component specified by iid from a list of Bonobo_ServerInfos.
 * 
 * Return value: The modified list. If the component is not found, the list will 
 * be unchanged.
 */
GList *
gnome_vfs_mime_remove_component_from_list (GList *components, 
					   const char *iid,
					   gboolean *did_remove)
{
	GList *matching_node;
	
	matching_node = g_list_find_custom 
		(components, (gpointer)iid,
		 (GCompareFunc) gnome_vfs_mime_component_matches_id);
	if (matching_node != NULL) {
		components = g_list_remove_link (components, matching_node);
		gnome_vfs_mime_component_list_free (matching_node);
	}

	if (did_remove != NULL) {
		*did_remove = matching_node != NULL;
	}
	return components;
}

/**
 * gnome_vfs_mime_remove_component_from_short_list:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @iid: const char * containing the component's OAF IID
 * 
 * Remove a component from the short list for MIME type @mime_type. The short list contains
 * components recommended for display as choices to the user for a particular MIME type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_remove_component_from_short_list (const char *mime_type,
						 const char *iid)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_add_extension:
 * @extension: The extension to add (e.g. "txt")
 * @mime_type: The mime type to add the mapping to.
 * 
 * Add a file extension to the specificed MIME type in the MIME database.
 * 
 * Return value: GnomeVFSResult indicating the success of the operation or any
 * errors that may have occurred.
 **/
GnomeVFSResult
gnome_vfs_mime_add_extension (const char *mime_type, const char *extension)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_remove_extension:
 * @extension: The extension to remove
 * @mime_type: The mime type to remove the extension from
 * 
 * Removes a file extension from the specificed MIME type in the MIME database.
 * 
 * Return value: GnomeVFSResult indicating the success of the operation or any
 * errors that may have occurred.
 **/
GnomeVFSResult
gnome_vfs_mime_remove_extension (const char *mime_type, const char *extension)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_extend_all_applications:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_ids: a GList of const char * containing application ids
 * 
 * Register @mime_type as being handled by all applications list in @application_ids.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_extend_all_applications (const char *mime_type,
					GList *application_ids)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}

/**
 * gnome_vfs_mime_remove_from_all_applications:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_ids: a GList of const char * containing application ids
 * 
 * Remove @mime_type as a handled type from every application in @application_ids
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_remove_from_all_applications (const char *mime_type,
					     GList *application_ids)
{
	g_warning (_("Deprecated function.  User modifications to the MIME database are no longer supported."));
	return GNOME_VFS_ERROR_DEPRECATED_FUNCTION;
}


/**
 * gnome_vfs_mime_application_copy:
 * @application: The GnomeVFSMimeApplication to be duplicated.
 * 
 * Creates a newly referenced copy of a GnomeVFSMimeApplication object.
 * 
 * Return value: A copy of @application
 **/
GnomeVFSMimeApplication *
gnome_vfs_mime_application_copy (GnomeVFSMimeApplication *application)
{
	GnomeVFSMimeApplication *result;

	if (application == NULL) {
		return NULL;
	}
	
	result = g_new0 (GnomeVFSMimeApplication, 1);
	result->id = g_strdup (application->id);
	result->name = g_strdup (application->name);
	result->command = g_strdup (application->command);
	result->can_open_multiple_files = application->can_open_multiple_files;
	result->expects_uris = application->expects_uris;
	result->supported_uri_schemes = copy_str_list (application->supported_uri_schemes);
	result->requires_terminal = application->requires_terminal;

	return result;
}

/**
 * gnome_vfs_mime_application_free:
 * @application: The GnomeVFSMimeApplication to be freed
 * 
 * Frees a GnomeVFSMimeApplication *.
 * 
 **/
void
gnome_vfs_mime_application_free (GnomeVFSMimeApplication *application) 
{
	if (application != NULL) {
		g_free (application->name);
		g_free (application->command);
		g_list_foreach (application->supported_uri_schemes,
				(GFunc) g_free,
				NULL);
		g_list_free (application->supported_uri_schemes);
		g_free (application->id);
		g_free (application);
	}
}

/**
 * gnome_vfs_mime_action_free:
 * @action: The GnomeVFSMimeAction to be freed
 * 
 * Frees a GnomeVFSMimeAction *.
 * 
 **/
void
gnome_vfs_mime_action_free (GnomeVFSMimeAction *action) 
{
	switch (action->action_type) {
	case GNOME_VFS_MIME_ACTION_TYPE_APPLICATION:
		gnome_vfs_mime_application_free (action->action.application);
		break;
	case GNOME_VFS_MIME_ACTION_TYPE_COMPONENT:
		CORBA_free (action->action.component);
		break;
	default:
		g_assert_not_reached ();
	}

	g_free (action);
}

/**
 * gnome_vfs_mime_application_list_free:
 * @list: a GList of GnomeVFSApplication * to be freed
 * 
 * Frees lists of GnomeVFSApplications, as returned from functions such
 * as gnome_vfs_get_all_applications().
 * 
 **/
void
gnome_vfs_mime_application_list_free (GList *list)
{
	g_list_foreach (list, (GFunc) gnome_vfs_mime_application_free, NULL);
	g_list_free (list);
}

/**
 * gnome_vfs_mime_component_list_free:
 * @list: a GList of Bonobo_ServerInfo * to be freed
 * 
 * Frees lists of Bonobo_ServerInfo * (as returned from functions such
 * as @gnome_vfs_get_all_components)
 * 
 **/
void
gnome_vfs_mime_component_list_free (GList *list)
{
	g_list_foreach (list, (GFunc) CORBA_free, NULL);
	g_list_free (list);
}

/**
 * gnome_vfs_mime_application_new_from_id:
 * @id: A const char * containing an application id
 * 
 * Fetches the GnomeVFSMimeApplication associated with the specified
 * application ID from the MIME database.
 *
 * Return value: GnomeVFSMimeApplication * corresponding to @id
 **/
GnomeVFSMimeApplication *
gnome_vfs_mime_application_new_from_id (const char *id)
{
	EggDesktopEntries *entries;
	GError *entries_error;
	GnomeVFSMimeApplication *application;
	char *filename, *p;

	application = NULL;
	entries_error = NULL;

	filename = g_build_filename ("applications", id, NULL);

	entries =
		egg_desktop_entries_new_from_file (NULL,
				EGG_DESKTOP_ENTRIES_GENERATE_LOOKUP_MAP |
			        EGG_DESKTOP_ENTRIES_DISCARD_COMMENTS,
				filename,
				NULL);
	g_free (filename);
	
	if (entries == NULL)
		return NULL;

	application = g_new0 (GnomeVFSMimeApplication, 1);

	application->id = g_strdup (id);
	application->name = egg_desktop_entries_get_locale_string (entries,
								   egg_desktop_entries_get_start_group (entries),
								   "Name", NULL, NULL);
	if (application->name == NULL) 
		goto error;

	application->command = egg_desktop_entries_get_string (entries,
			                                       egg_desktop_entries_get_start_group (entries),
							       "Exec", NULL);

	if (application->command == NULL) 
		goto error;

	application->requires_terminal = egg_desktop_entries_get_boolean (entries,
			                                       egg_desktop_entries_get_start_group (entries),
							       "Terminal", &entries_error);

	if (entries_error != NULL) {
		g_error_free (entries_error);
                application->requires_terminal = FALSE;
	}

	egg_desktop_entries_free (entries);
	entries = NULL;

	/* Guess on these last fields based on parameters passed to Exec line
	 */
	if ((p = strstr (application->command, "%f")) != NULL
		|| (p = strstr (application->command, "%d")) != NULL
		|| (p = strstr (application->command, "%n")) != NULL) {
		*p = '\0';
		application->can_open_multiple_files = FALSE;
		application->expects_uris = GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS; 
		application->supported_uri_schemes = NULL;
	} else if ((p = strstr (application->command, "%F")) != NULL
		   || (p = strstr (application->command, "%D")) != NULL
		   || (p = strstr (application->command, "%N")) != NULL) {
		*p = '\0';
		application->can_open_multiple_files = TRUE;
		application->expects_uris = GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS; 
		application->supported_uri_schemes = NULL;
	} else if ((p = strstr (application->command, "%u")) != NULL) {
		*p = '\0';
		application->can_open_multiple_files = FALSE;
		application->expects_uris = GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS; 
		application->supported_uri_schemes = _gnome_vfs_configuration_get_methods_list ();
	} else if ((p = strstr (application->command, "%U")) != NULL) {
		*p = '\0';
		application->can_open_multiple_files = TRUE;
		application->expects_uris = GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS; 
		application->supported_uri_schemes = _gnome_vfs_configuration_get_methods_list ();
	} else {
		application->can_open_multiple_files = FALSE;
		application->expects_uris = GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES; 
		application->supported_uri_schemes = _gnome_vfs_configuration_get_methods_list ();
	} 

	return application;

error:
	if (entries) 
		egg_desktop_entries_free (entries);

	if (application) 
		gnome_vfs_mime_application_free (application);

	return NULL;
}

/** 
 * gnome_vfs_mime_action_launch:
 * @action: the GnomeVFSMimeAction to launch
 * @uris: parameters for the GnomeVFSMimeAction
 *
 * Launches the given mime action with the given parameters. If 
 * the action is an application the command line parameters will
 * be expanded as required by the application. The application
 * will also be launched in a terminal if that is required. If the
 * application only supports one argument per instance then multile
 * instances of the application will be launched.
 *
 * If the default action is a component it will be launched with
 * the component viewer application defined using the gconf value:
 * /desktop/gnome/application/component_viewer/exec. The parameters
 * %s and %c in the command line will be replaced with the list of
 * parameters and the default component IID respectively.
 *
 * Return value: GNOME_VFS_OK if the action was launched,
 * GNOME_VFS_ERROR_BAD_PARAMETERS for an invalid action.
 * GNOME_VFS_ERROR_NOT_SUPPORTED if the uri protocol is
 * not supported by the action.
 * GNOME_VFS_ERROR_PARSE if the action command can not be parsed.
 * GNOME_VFS_ERROR_LAUNCH if the action command can not be launched.
 * GNOME_VFS_ERROR_INTERNAL for other internal and GConf errors.
 *
 * Since: 2.4
 */
GnomeVFSResult
gnome_vfs_mime_action_launch (GnomeVFSMimeAction *action,
			      GList              *uris)
{
	return gnome_vfs_mime_action_launch_with_env (action, uris, NULL);
}

/**
 * gnome_vfs_mime_action_launch_with_env:
 *
 * Same as gnome_vfs_mime_action_launch except that the
 * application or component viewer will be launched with
 * the given environment.
 *
 * Return value: same as gnome_vfs_mime_action_launch
 *
 * Since: 2.4
 */
GnomeVFSResult
gnome_vfs_mime_action_launch_with_env (GnomeVFSMimeAction *action,
				       GList              *uris,
				       char              **envp)
{
	GnomeVFSResult result;
	char **argv;
	int argc;

	g_return_val_if_fail (action != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (uris != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	switch (action->action_type) {
	case GNOME_VFS_MIME_ACTION_TYPE_APPLICATION:
	
		return gnome_vfs_mime_application_launch_with_env
			 		(action->action.application,
			 		 uris, envp);
					 
	case GNOME_VFS_MIME_ACTION_TYPE_COMPONENT:
	
		result = expand_parameters (action->action.component, action->action_type,
					    uris, &argc, &argv);
					    
		if (result != GNOME_VFS_OK) {
			return result;
		}
		
		if (!g_spawn_async (NULL /* working directory */,
	                            argv,
        	                    envp,
                	            G_SPAWN_SEARCH_PATH /* flags */,
                        	    NULL /* child_setup */,
				    NULL /* data */,
	                            NULL /* child_pid */,
        	                    NULL /* error */)) {
			g_strfreev (argv);
			return GNOME_VFS_ERROR_LAUNCH;
		}
		g_strfreev (argv);
		
		return GNOME_VFS_OK;		
	
	default:
		g_assert_not_reached ();
	}
	
	return GNOME_VFS_ERROR_BAD_PARAMETERS;
}

/**
 * gnome_vfs_mime_application_launch:
 * @app: the GnomeVFSMimeApplication to launch
 * @uris: parameters for the GnomeVFSMimeApplication
 *
 * Launches the given mime application with the given parameters.
 * Command line parameters will be expanded as required by the
 * application. The application will also be launched in a terminal
 * if that is required. If the application only supports one argument 
 * per instance then multiple instances of the application will be 
 * launched.
 *
 * Return value: 
 * GNOME_VFS_OK if the application was launched.
 * GNOME_VFS_ERROR_NOT_SUPPORTED if the uri protocol is not
 * supported by the application.
 * GNOME_VFS_ERROR_PARSE if the application command can not
 * be parsed.
 * GNOME_VFS_ERROR_LAUNCH if the application command can not
 * be launched.
 * GNOME_VFS_ERROR_INTERNAL for other internal and GConf errors.
 *
 * Since: 2.4
 */
GnomeVFSResult
gnome_vfs_mime_application_launch (GnomeVFSMimeApplication *app,
                                   GList                   *uris)
{
	return gnome_vfs_mime_application_launch_with_env (app, uris, NULL);
}

/**
 * gnome_vfs_mime_application_launch_with_env:
 *
 * Same as gnome_vfs_mime_application_launch except that
 * the application will be launched with the given environment.
 *
 * Return value: same as gnome_vfs_mime_application_launch
 *
 * Since: 2.4
 */
GnomeVFSResult 
gnome_vfs_mime_application_launch_with_env (GnomeVFSMimeApplication *app,
                                            GList                   *uris,
                                            char                   **envp)
{
	GnomeVFSResult result;
	GList *u;
	char *scheme;
	char **argv;
	int argc;
	
	g_return_val_if_fail (app != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (uris != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	
	/* check that all uri schemes are supported */
	if (app->supported_uri_schemes != NULL) {
		for (u = uris; u != NULL; u = u->next) {

			scheme = gnome_vfs_get_uri_scheme (u->data);
		
			if (!g_list_find_custom (app->supported_uri_schemes,
						 scheme, (GCompareFunc) strcmp)) {
				g_free (scheme);
				return GNOME_VFS_ERROR_NOT_SUPPORTED;
			}
			
			g_free (scheme);
		}
	}

	while (uris != NULL) {
		
		result = expand_parameters (app, GNOME_VFS_MIME_ACTION_TYPE_APPLICATION,
				            uris, &argc, &argv);
		
		if (result != GNOME_VFS_OK) {
			return result;
		}

		if (app->requires_terminal) {
			if (!_gnome_vfs_prepend_terminal_to_vector (&argc, &argv)) {
				g_strfreev (argv);
				return GNOME_VFS_ERROR_INTERNAL;
			}
		}
		
		if (!g_spawn_async (NULL /* working directory */,
				    argv,
				    envp,
				    G_SPAWN_SEARCH_PATH /* flags */,
				    NULL /* child_setup */,
				    NULL /* data */,
				    NULL /* child_pid */,
				    NULL /* error */)) {
			g_strfreev (argv);
			return GNOME_VFS_ERROR_LAUNCH;
		}
		
		g_strfreev (argv);
		uris = uris->next;
		
		if (app->can_open_multiple_files) {
			break;
		}
	}
	
	return GNOME_VFS_OK;		
}

static GnomeVFSResult
expand_parameters (gpointer                 action,
		   GnomeVFSMimeActionType   type,
                   GList                   *uris,
		   int                     *argc,
		   char                  ***argv)		   
{
	GnomeVFSMimeApplication *app = NULL;
	Bonobo_ServerInfo *server = NULL;
	GConfClient *client;
	char *path;
	char *command = NULL;
	char **c_argv, **r_argv;
	int c_argc, max_r_argc;
	int i, c;
	gboolean added_arg;

	/* figure out what command to parse */	
	switch (type) {
	case GNOME_VFS_MIME_ACTION_TYPE_APPLICATION:
		app = (GnomeVFSMimeApplication *) action;
		command = g_strdup (app->command);
		break;
		
	case GNOME_VFS_MIME_ACTION_TYPE_COMPONENT:
		if (!gconf_is_initialized ()) {
			if (!gconf_init (0, NULL, NULL)) {
				return GNOME_VFS_ERROR_INTERNAL;
			}
		}
	
		client = gconf_client_get_default ();
		g_return_val_if_fail (client != NULL, GNOME_VFS_ERROR_INTERNAL);
	
		command = gconf_client_get_string (client, GCONF_DEFAULT_VIEWER_EXEC_PATH, NULL);
		g_object_unref (client);
		
		if (command == NULL) {
			g_warning ("No default component viewer set\n");
			return GNOME_VFS_ERROR_INTERNAL;
		}
		
		server = (Bonobo_ServerInfo *) action;
		
		break;

	default:
		g_assert_not_reached ();
	}

	if (!g_shell_parse_argv (command,
				 &c_argc,
				 &c_argv,
				 NULL)) {
		return GNOME_VFS_ERROR_PARSE;
	}
	g_free (command);

	/* figure out how many parameters we can max have */
	max_r_argc = g_list_length (uris) + c_argc + 1;
	r_argv = g_new0 (char *, max_r_argc + 1);

	added_arg = FALSE;
	i = 0;
	for (c = 0; c < c_argc; c++) {
		/* replace %s with the uri parameters */
		if (strcmp (c_argv[c], "%s") == 0) {
			while (uris != NULL) {
				if (app != NULL) {
					switch (app->expects_uris) {
					case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS:
						r_argv[i] = g_strdup (uris->data);
						break;
	
					case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS:
						r_argv[i] = gnome_vfs_get_local_path_from_uri (uris->data);
						if (r_argv[i] == NULL) {
							g_strfreev (c_argv);
							g_strfreev (r_argv);
							return GNOME_VFS_ERROR_NOT_SUPPORTED;
						}
						break;
			
					case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES:
						/* this really means use URIs for non local files */
						path = gnome_vfs_get_local_path_from_uri (uris->data);
			
						if (path != NULL) {
							r_argv[i] = path;
						} else {
							r_argv[i] = g_strdup (uris->data);
						}				
			
						break;
				
					default:
						g_assert_not_reached ();
					}	
				} else {
					r_argv[i] = g_strdup (uris->data);
				}
				i++;
				uris = uris->next;
				
				if (app != NULL && !app->can_open_multiple_files) {
					break;
				}
			}
			added_arg = TRUE;
			
		/* replace %c with the component iid */
		} else if (server != NULL && strcmp (c_argv[c], "%c") == 0) {
			r_argv[i++] = g_strdup (server->iid);
			added_arg = TRUE;
			
		/* otherwise take arg from command */
		} else {
			r_argv[i++] = g_strdup (c_argv[c]);
		}
	}
	g_strfreev (c_argv);
	
	/* if there is no %s or %c, append the parameters to the end */
	if (!added_arg) {
		while (uris != NULL) {
			if (app != NULL) {
				switch (app->expects_uris) {
				case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS:
					r_argv[i] = g_strdup (uris->data);
					break;

				case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS:
					r_argv[i] = gnome_vfs_get_local_path_from_uri (uris->data);
					if (r_argv[i] == NULL) {
						g_strfreev (r_argv);
						return GNOME_VFS_ERROR_NOT_SUPPORTED;
					}
					break;
		
				case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES:
					/* this really means use URIs for non local files */
					path = gnome_vfs_get_local_path_from_uri (uris->data);
		
					if (path != NULL) {
						r_argv[i] = path;
					} else {
						r_argv[i] = g_strdup (uris->data);
					}				
		
					break;
			
				default:
					g_assert_not_reached ();
				}
			} else {
				r_argv[i] = g_strdup (uris->data);
			}
			i++;
			uris = uris->next;
			
			if (app != NULL && !app->can_open_multiple_files) {
				break;
			}
		}
	}		
	
	*argv = r_argv;
	*argc = i;

	return GNOME_VFS_OK;
}

static GList *
Bonobo_ServerInfoList_to_ServerInfo_g_list (Bonobo_ServerInfoList *info_list)
{
	GList *retval;
	int i;
	
	retval = NULL;
	if (info_list != NULL && info_list->_length > 0) {
		for (i = 0; i < info_list->_length; i++) {
			retval = g_list_prepend (retval, Bonobo_ServerInfo_duplicate (&info_list->_buffer[i]));
		}
		retval = g_list_reverse (retval);
	}

	return retval;
}

static GList *
copy_str_list (GList *string_list)
{
	GList *copy, *node;
       
	copy = NULL;
	for (node = string_list; node != NULL; node = node->next) {
		copy = g_list_prepend (copy, 
				       g_strdup ((char *) node->data));
				       }
	return g_list_reverse (copy);
}
