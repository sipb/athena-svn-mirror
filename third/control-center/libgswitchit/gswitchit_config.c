/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <X11/keysym.h>

#include <gdk/gdkx.h>
#include <gnome.h>
#include <libgnome/gnome-util.h>

#include <libxklavier/xklavier_config.h>

#include "gswitchit_applet_config.h"

#define GSWITCHIT_CONFIG_PREFIX "/apps/gswitchit"

#define GSWITCHIT_CONFIG_APPLET_PREFIX  GSWITCHIT_CONFIG_PREFIX "/Applet"

static const char GSWITCHIT_CONFIG_APPLET_DIR[] =
    GSWITCHIT_CONFIG_APPLET_PREFIX;
static const char GSWITCHIT_CONFIG_APPLET_KEY_DEFAULT_GROUP[] =
    GSWITCHIT_CONFIG_APPLET_PREFIX "/defaultGroup";
static const char GSWITCHIT_CONFIG_APPLET_KEY_GROUP_PER_WINDOW[] =
    GSWITCHIT_CONFIG_APPLET_PREFIX "/groupPerWindow";
static const char GSWITCHIT_CONFIG_APPLET_KEY_HANDLE_INDICATORS[] =
    GSWITCHIT_CONFIG_APPLET_PREFIX "/handleIndicators";
static const char GSWITCHIT_CONFIG_APPLET_KEY_LAYOUT_NAMES_AS_GROUP_NAMES[]
    = GSWITCHIT_CONFIG_APPLET_PREFIX "/layoutNamesAsGroupNames";
static const char GSWITCHIT_CONFIG_APPLET_KEY_SHOW_FLAGS[] =
    GSWITCHIT_CONFIG_APPLET_PREFIX "/showFlags";
static const char GSWITCHIT_CONFIG_APPLET_KEY_DEBUG_LEVEL[] =
    GSWITCHIT_CONFIG_APPLET_PREFIX "/debugLevel";
static const char GSWITCHIT_CONFIG_APPLET_KEY_SECONDARIES[] =
    GSWITCHIT_CONFIG_APPLET_PREFIX "/secondary";
static const char GSWITCHIT_CONFIG_APPLET_KEY_ENABLED_PLUGINS[] =
    GSWITCHIT_CONFIG_APPLET_PREFIX "/enabledPlugins";

static int gconfAppletListenerId = 0;

#define KEYBOARD_CONFIG_KEY_PREFIX "/desktop/gnome/peripherals/keyboard"
#define GSWITCHIT_CONFIG_XKB_KEY_PREFIX KEYBOARD_CONFIG_KEY_PREFIX "/xkb"

const char GSWITCHIT_CONFIG_XKB_DIR[] = GSWITCHIT_CONFIG_XKB_KEY_PREFIX;
const char GSWITCHIT_CONFIG_XKB_KEY_OVERRIDE_SETTINGS[] =
    GSWITCHIT_CONFIG_XKB_KEY_PREFIX "/overrideSettings";
const char GSWITCHIT_CONFIG_XKB_KEY_MODEL[] =
    GSWITCHIT_CONFIG_XKB_KEY_PREFIX "/model";
const char GSWITCHIT_CONFIG_XKB_KEY_LAYOUTS[] =
    GSWITCHIT_CONFIG_XKB_KEY_PREFIX "/layouts";
const char GSWITCHIT_CONFIG_XKB_KEY_OPTIONS[] =
    GSWITCHIT_CONFIG_XKB_KEY_PREFIX "/options";

const char *GSWITCHIT_CONFIG_XKB_ACTIVE[] = {
	GSWITCHIT_CONFIG_XKB_KEY_MODEL,
	GSWITCHIT_CONFIG_XKB_KEY_LAYOUTS,
	GSWITCHIT_CONFIG_XKB_KEY_OPTIONS
};

#define GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_PREFIX KEYBOARD_CONFIG_KEY_PREFIX "/xkb.sysbackup"

const char GSWITCHIT_CONFIG_XKB_SYSBACKUP_DIR[] =
    GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_PREFIX;
const char GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_MODEL[] =
    GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_PREFIX "/model";
const char GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_LAYOUTS[] =
    GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_PREFIX "/layouts";
const char GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_OPTIONS[] =
    GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_PREFIX "/options";

const char *GSWITCHIT_CONFIG_XKB_SYSBACKUP[] = {
	GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_MODEL,
	GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_LAYOUTS,
	GSWITCHIT_CONFIG_XKB_SYSBACKUP_KEY_OPTIONS
};

static int gconfXkbListenerId;

/**
 * static common functions
 */
static void
_GSwitchItConfigListReset (GSList ** plist)
{
	while (*plist != NULL) {
		GSList *p = *plist;
		*plist = (*plist)->next;
		g_free (p->data);
		g_slist_free_1 (p);
	}
}

static Bool
_GSwitchItConfigGetDescriptions (const char *layoutName,
				 const char *variantName,
				 char **layoutShortDescr,
				 char **layoutDescr,
				 char **variantShortDescr,
				 char **variantDescr)
{
	static XklConfigItem litem;
	static XklConfigItem vitem;

	layoutName = g_strdup (layoutName);

	g_snprintf (litem.name, sizeof litem.name, "%s", layoutName);
	if (XklConfigFindLayout (&litem)) {
		*layoutShortDescr = litem.shortDescription;
		*layoutDescr = litem.description;
	} else
		*layoutShortDescr = *layoutDescr = NULL;

	if (variantName != NULL) {
		variantName = g_strdup (variantName);
		g_snprintf (vitem.name, sizeof vitem.name, "%s",
			    variantName);
		if (XklConfigFindVariant (layoutName, &vitem)) {
			*variantShortDescr = vitem.shortDescription;
			*variantDescr = vitem.description;
		} else
			*variantShortDescr = *variantDescr = NULL;

		g_free ((char *) variantName);
	} else
		*variantDescr = NULL;

	g_free ((char *) layoutName);
	return *layoutDescr != NULL;
}

static void
_GSwitchItConfigAddListener (GConfClient * confClient,
			     const gchar * key,
			     GConfClientNotifyFunc func,
			     gpointer userData, int *pid)
{
	GError *err = NULL;
	XklDebug (150, "Listening to [%s]\n", key);
	*pid = gconf_client_notify_add (confClient,
					key, func, userData, NULL, &err);
	if (0 == *pid) {
		g_warning ("Error listening for configuration: [%s]\n",
			   err->message);
		g_error_free (err);
	}
}

static void
_GSwitchItConfigRemoveListener (GConfClient * confClient, int *pid)
{
	if (*pid != 0) {
		gconf_client_notify_remove (confClient, *pid);
		*pid = 0;
	}
}

/**
 * extern common functions
 */
const char *
GSwitchItConfigMergeItems (const char *parent, const char *child)
{
	static char buffer[XKL_MAX_CI_NAME_LENGTH * 2 - 1];
	*buffer = '\0';
	if (parent != NULL) {
		if (strlen (parent) >= XKL_MAX_CI_NAME_LENGTH)
			return NULL;
		strcat (buffer, parent);
	}
	if (child != NULL) {
		if (strlen (child) >= XKL_MAX_CI_NAME_LENGTH)
			return NULL;
		strcat (buffer, "\t");
		strcat (buffer, child);
	}
	return buffer;
}

gboolean
GSwitchItConfigSplitItems (const char *merged, char **parent, char **child)
{
	static char pbuffer[XKL_MAX_CI_NAME_LENGTH];
	static char cbuffer[XKL_MAX_CI_NAME_LENGTH];
	int plen, clen;
	const char *pos;
	*parent = *child = NULL;

	if (merged == NULL)
		return FALSE;

	pos = strchr (merged, '\t');
	if (pos == NULL) {
		plen = strlen (merged);
		clen = 0;
	} else {
		plen = pos - merged;
		clen = strlen (pos + 1);
		if (clen >= XKL_MAX_CI_NAME_LENGTH)
			return FALSE;
		strcpy (*child = cbuffer, pos + 1);
	}
	if (plen >= XKL_MAX_CI_NAME_LENGTH)
		return FALSE;
	memcpy (*parent = pbuffer, merged, plen);
	pbuffer[plen] = '\0';
	return TRUE;
}

/**
 * static applet config functions
 */
static void
_GSwitchItAppletConfigFreeEnabledPlugins (GSwitchItAppletConfig *
					  appletConfig)
{
	GSList *pluginNode = appletConfig->enabledPlugins;
	if (pluginNode != NULL) {
		do {
			if (pluginNode->data != NULL) {
				g_free (pluginNode->data);
				pluginNode->data = NULL;
			}
			pluginNode = g_slist_next (pluginNode);
		} while (pluginNode != NULL);
		g_slist_free (appletConfig->enabledPlugins);
		appletConfig->enabledPlugins = NULL;
	}
}

/**
 * static xkb config functions
 */
static void
_GSwitchItXkbConfigOptionsAdd (GSwitchItXkbConfig * xkbConfig,
			       const gchar * fullOptionName)
{
	xkbConfig->options =
	    g_slist_append (xkbConfig->options, g_strdup (fullOptionName));
}

static void
_GSwitchItXkbConfigLayoutsAdd (GSwitchItXkbConfig * xkbConfig,
			       const gchar * fullLayoutName)
{
	xkbConfig->layouts =
	    g_slist_append (xkbConfig->layouts, g_strdup (fullLayoutName));
}

static void
_GSwitchItXkbConfigCopyFromXklConfig (GSwitchItXkbConfig *
				      xkbConfig, XklConfigRec * pdata)
{
	int i;
	char **p, **p1;
	GSwitchItXkbConfigModelSet (xkbConfig, pdata->model);
	XklDebug (150, "Loaded XKB model: [%s]\n", pdata->model);

	GSwitchItXkbConfigLayoutsReset (xkbConfig);
	p = pdata->layouts;
	p1 = pdata->variants;
	for (i = pdata->numLayouts; --i >= 0;) {
		if (*p1 == NULL || **p1 == '\0') {
			XklDebug (150, "Loaded XKB layout: [%s]\n", *p);
			_GSwitchItXkbConfigLayoutsAdd (xkbConfig, *p);
		} else {
			char fullLayout[XKL_MAX_CI_NAME_LENGTH * 2];
			g_snprintf (fullLayout, sizeof (fullLayout),
				    "%s\t%s", *p, *p1);
			XklDebug (150,
				  "Loaded XKB layout with variant: [%s]\n",
				  fullLayout);
			_GSwitchItXkbConfigLayoutsAdd (xkbConfig,
						       fullLayout);
		}
		p++;
		p1++;
	}

	GSwitchItXkbConfigOptionsReset (xkbConfig);
	p = pdata->options;
	for (i = pdata->numOptions; --i >= 0;) {
		char group[XKL_MAX_CI_NAME_LENGTH];
		char *option = *p;
		char *delim =
		    (option != NULL) ? strchr (option, ':') : NULL;
		int len;
		if ((delim != NULL) &&
		    ((len = (delim - option)) < XKL_MAX_CI_NAME_LENGTH)) {
			strncpy (group, option, len);
			group[len] = 0;
			XklDebug (150, "Loaded XKB option: [%s][%s]\n",
				  group, option);
			GSwitchItXkbConfigOptionsAdd (xkbConfig, group,
						      option);
		}
		p++;
	}
}

static void
_GSwitchItXkbConfigCopyToXklConfig (GSwitchItXkbConfig *
				    xkbConfig, XklConfigRec * pdata)
{
	int i;
	pdata->model =
	    (xkbConfig->model == NULL) ? NULL : strdup (xkbConfig->model);

	pdata->numLayouts = pdata->numVariants =
	    (xkbConfig->layouts ==
	     NULL) ? 0 : g_slist_length (xkbConfig->layouts);
	pdata->numOptions =
	    (xkbConfig->options ==
	     NULL) ? 0 : g_slist_length (xkbConfig->options);

	XklDebug (150, "Taking %d layouts\n", pdata->numLayouts);
	if (pdata->numLayouts != 0) {
		GSList *theLayout = xkbConfig->layouts;
		char **p1 = pdata->layouts =
		    calloc ((sizeof (char *)) * pdata->numLayouts, 1);
		char **p2 = pdata->variants =
		    calloc ((sizeof (char *)) * pdata->numVariants, 1);
		for (i = pdata->numLayouts; --i >= 0;) {
			char *layout, *variant;
			if (GSwitchItConfigSplitItems
			    (theLayout->data, &layout, &variant)
			    && variant != NULL) {
				*p1 =
				    (layout ==
				     NULL) ? NULL : strdup (layout);
				*p2 =
				    (variant ==
				     NULL) ? NULL : strdup (variant);
			} else {
				*p1 =
				    (theLayout->data ==
				     NULL) ? NULL : strdup (theLayout->
							    data);
				*p2 = NULL;
			}
			XklDebug (150, "Adding [%s]/%p and [%s]/%p\n",
				  *p1 ? *p1 : "(nil)", *p1,
				  *p2 ? *p2 : "(nil)", *p2);
			p1++;
			p2++;
			theLayout = theLayout->next;
		}
	}

	if (pdata->numOptions != 0) {
		GSList *theOption = xkbConfig->options;
		char **p = pdata->options =
		    calloc ((sizeof (char *)) * pdata->numOptions, 1);
		for (i = pdata->numOptions; --i >= 0;) {
			char *group, *option;
			if (GSwitchItConfigSplitItems
			    (theOption->data, &group, &option)
			    && option != NULL)
				*(p++) = strdup (option);
			else
				XklDebug (150, "Could not split [%s]\n",
					  theOption->data);
			theOption = theOption->next;
		}
	}
}

static gboolean
_GSwitchItXkbConfigDoWithSettings (GSwitchItXkbConfig *
				   xkbConfig,
				   gboolean activate,
				   const char *psFileName)
{
	gboolean rv = FALSE;

	XklConfigRec data;
	XklConfigRecInit (&data);

	_GSwitchItXkbConfigCopyToXklConfig (xkbConfig, &data);

	if (activate) {
		rv = XklConfigActivate (&data, NULL);
	} else {
		char *home = getenv ("HOME");
		char xkmFileName[PATH_MAX];
		char cmd[PATH_MAX * 2 + 20];
		int status;
		g_snprintf (xkmFileName, sizeof (xkmFileName),
			    "%s/.gnome_private/xkbpreview.xkm", home);
		rv = XklConfigWriteXKMFile (xkmFileName, &data, NULL);
		if (rv) {
			g_snprintf (cmd, sizeof (cmd),
				    "xkbprint -full -color %s %s",
				    xkmFileName, psFileName);
			status = system (cmd);
			XklDebug (100, "Res: [%d]\n", status);
			//unlink( xkmFileName );
		} else {
			XklDebug (10, "Could not create XKM file!\n");
		}
	}
	XklConfigRecDestroy (&data);
	return rv;
}

static gboolean
_GSListStrEqual (GSList * l1, GSList * l2)
{
	if (l1 == l2)
		return TRUE;
	while (l1 != NULL && l2 != NULL) {
		if ((l1->data != l2->data) &&
		    (l1->data != NULL) &&
		    (l2->data != NULL) &&
		    g_ascii_strcasecmp (l1->data, l2->data))
			return False;

		l1 = l1->next;
		l2 = l2->next;
	}
	return (l1 == NULL && l2 == NULL);
}

static void
_GSwitchItXkbConfigLoadParams (GSwitchItXkbConfig * xkbConfig,
			       const char *paramNames[])
{
	GError *gerror = NULL;
	gchar *pc;
	GSList *pl;

	pc = gconf_client_get_string (xkbConfig->confClient,
				      paramNames[0], &gerror);
	if (pc == NULL || gerror != NULL) {
		if (gerror != NULL) {
			g_warning ("Error reading configuration:%s\n",
				   gerror->message);
			g_error_free (gerror);
			gerror = NULL;
		}
		GSwitchItXkbConfigModelSet (xkbConfig, NULL);
	} else {
		GSwitchItXkbConfigModelSet (xkbConfig, pc);
	}
	XklDebug (150, "Loaded XKB model: [%s]\n", xkbConfig->model);

	GSwitchItXkbConfigLayoutsReset (xkbConfig);

	pl = gconf_client_get_list (xkbConfig->confClient,
				    paramNames[1],
				    GCONF_VALUE_STRING, &gerror);
	if (pl == NULL || gerror != NULL) {
		if (gerror != NULL) {
			g_warning ("Error reading configuration:%s\n",
				   gerror->message);
			g_error_free (gerror);
			gerror = NULL;
		}
	}

	while (pl != NULL) {
		XklDebug (150, "Loaded XKB layout: [%s]\n", pl->data);
		_GSwitchItXkbConfigLayoutsAdd (xkbConfig, pl->data);
		pl = pl->next;
	}

	GSwitchItXkbConfigOptionsReset (xkbConfig);

	pl = gconf_client_get_list (xkbConfig->confClient,
				    paramNames[2],
				    GCONF_VALUE_STRING, &gerror);
	if (pl == NULL || gerror != NULL) {
		if (gerror != NULL) {
			g_warning ("Error reading configuration:%s\n",
				   gerror->message);
			g_error_free (gerror);
			gerror = NULL;
		}
	}

	while (pl != NULL) {
		XklDebug (150, "Loaded XKB option: [%s]\n", pl->data);
		_GSwitchItXkbConfigOptionsAdd (xkbConfig,
					       (const char *) pl->data);
		pl = pl->next;
	}
}

static void
_GSwitchItXkbConfigSaveParams (GSwitchItXkbConfig * xkbConfig,
			       GConfChangeSet * cs,
			       const char *paramNames[])
{
	GSList *pl;

	gconf_change_set_set_string (cs, paramNames[0], xkbConfig->model);
	XklDebug (150, "Saved XKB model: [%s]\n", xkbConfig->model);

	pl = xkbConfig->layouts;
	while (pl != NULL) {
		XklDebug (150, "Saved XKB layout: [%s]\n", pl->data);
		pl = pl->next;
	}
	gconf_change_set_set_list (cs,
				   paramNames[1],
				   GCONF_VALUE_STRING, xkbConfig->layouts);
	pl = xkbConfig->options;
	while (pl != NULL) {
		XklDebug (150, "Saved XKB option: [%s]\n", pl->data);
		pl = pl->next;
	}
	gconf_change_set_set_list (cs,
				   paramNames[2],
				   GCONF_VALUE_STRING, xkbConfig->options);
}

/**
 * extern xkb config functions
 */
void
GSwitchItXkbConfigInit (GSwitchItXkbConfig * xkbConfig,
			GConfClient * confClient)
{
	GError *gerror = NULL;

	memset (xkbConfig, 0, sizeof (*xkbConfig));
	xkbConfig->confClient = confClient;
	g_object_ref (xkbConfig->confClient);

	gconf_client_add_dir (xkbConfig->confClient,
			      GSWITCHIT_CONFIG_XKB_DIR,
			      GCONF_CLIENT_PRELOAD_NONE, &gerror);
	if (gerror != NULL) {
		g_warning ("err: %s\n", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}
}

void
GSwitchItXkbConfigTerm (GSwitchItXkbConfig * xkbConfig)
{
	GSwitchItXkbConfigModelSet (xkbConfig, NULL);

	GSwitchItXkbConfigLayoutsReset (xkbConfig);

	g_object_unref (xkbConfig->confClient);
	xkbConfig->confClient = NULL;
}

void
GSwitchItXkbConfigLoad (GSwitchItXkbConfig * xkbConfig)
{
	GError *gerror = NULL;

	xkbConfig->overrideSettings =
	    gconf_client_get_bool (xkbConfig->confClient,
				   GSWITCHIT_CONFIG_XKB_KEY_OVERRIDE_SETTINGS,
				   &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		xkbConfig->overrideSettings = FALSE;
		g_error_free (gerror);
		gerror = NULL;
	}
	XklDebug (150, "Loaded XKB override cmd: [%s]\n",
		  xkbConfig->overrideSettings ? "true" : "false");

	_GSwitchItXkbConfigLoadParams (xkbConfig,
				       GSWITCHIT_CONFIG_XKB_ACTIVE);
}

void
GSwitchItXkbConfigLoadSysBackup (GSwitchItXkbConfig * xkbConfig)
{
	_GSwitchItXkbConfigLoadParams (xkbConfig,
				       GSWITCHIT_CONFIG_XKB_SYSBACKUP);
}

void
GSwitchItXkbConfigLoadCurrent (GSwitchItXkbConfig * xkbConfig)
{
	XklConfigRec data;
	XklConfigRecInit (&data);
	if (XklConfigGetFromServer (&data))
		_GSwitchItXkbConfigCopyFromXklConfig (xkbConfig, &data);
	else
		XklDebug (150,
			  "Could not load xkb config from server: [%s]\n",
			  XklGetLastError ());
	XklConfigRecDestroy (&data);
}

void
GSwitchItXkbConfigLoadInitial (GSwitchItXkbConfig * xkbConfig)
{
	XklConfigRec data;
	XklConfigRecInit (&data);
	if (XklConfigGetFromBackup (&data))
		_GSwitchItXkbConfigCopyFromXklConfig (xkbConfig, &data);
	else
		XklDebug (150,
			  "Could not load xkb config from backup: [%s]\n",
			  XklGetLastError ());
	XklConfigRecDestroy (&data);
}

gboolean
GSwitchItXkbConfigEquals (GSwitchItXkbConfig * xkbConfig1,
			  GSwitchItXkbConfig * xkbConfig2)
{
	if (xkbConfig1 == xkbConfig2)
		return True;
	if ((xkbConfig1->model != xkbConfig2->model) &&
	    (xkbConfig1->model != NULL) &&
	    (xkbConfig2->model != NULL) &&
	    g_ascii_strcasecmp (xkbConfig1->model, xkbConfig2->model))
		return False;
	return _GSListStrEqual (xkbConfig1->layouts, xkbConfig2->layouts)
	    && _GSListStrEqual (xkbConfig1->options, xkbConfig2->options);
}

void
GSwitchItXkbConfigSave (GSwitchItXkbConfig * xkbConfig)
{
	GConfChangeSet *cs;
	GError *gerror = NULL;

	cs = gconf_change_set_new ();
	gconf_change_set_set_bool (cs,
				   GSWITCHIT_CONFIG_XKB_KEY_OVERRIDE_SETTINGS,
				   xkbConfig->overrideSettings);
	XklDebug (150, "Saved XKB override cmd: [%s]\n",
		  xkbConfig->overrideSettings ? "true" : "false");

	_GSwitchItXkbConfigSaveParams (xkbConfig, cs,
				       GSWITCHIT_CONFIG_XKB_ACTIVE);

	gconf_client_commit_change_set (xkbConfig->confClient, cs, TRUE,
					&gerror);
	if (gerror != NULL) {
		g_warning ("Error saving active configuration: %s\n",
			   gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}
	gconf_change_set_unref (cs);
}

void
GSwitchItXkbConfigSaveSysBackup (GSwitchItXkbConfig * xkbConfig)
{
	GConfChangeSet *cs;
	GError *gerror = NULL;

	cs = gconf_change_set_new ();

	_GSwitchItXkbConfigSaveParams (xkbConfig, cs,
				       GSWITCHIT_CONFIG_XKB_SYSBACKUP);

	gconf_client_commit_change_set (xkbConfig->confClient, cs, TRUE,
					&gerror);
	if (gerror != NULL) {
		g_warning ("Error saving backup configuration: %s\n",
			   gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}
	gconf_change_set_unref (cs);
}

void
GSwitchItXkbConfigModelSet (GSwitchItXkbConfig * xkbConfig,
			    const gchar * modelName)
{
	if (xkbConfig->model != NULL)
		g_free (xkbConfig->model);
	xkbConfig->model =
	    (modelName == NULL) ? NULL : g_strdup (modelName);
}

void
GSwitchItXkbConfigLayoutsAdd (GSwitchItXkbConfig * xkbConfig,
			      const gchar * layoutName,
			      const gchar * variantName)
{
	const char *merged;
	if (layoutName == NULL)
		return;
	merged = GSwitchItConfigMergeItems (layoutName, variantName);
	if (merged == NULL)
		return;
	_GSwitchItXkbConfigLayoutsAdd (xkbConfig, merged);
}

void
GSwitchItXkbConfigLayoutsReset (GSwitchItXkbConfig * xkbConfig)
{
	_GSwitchItConfigListReset (&xkbConfig->layouts);
}

void
GSwitchItXkbConfigOptionsReset (GSwitchItXkbConfig * xkbConfig)
{
	_GSwitchItConfigListReset (&xkbConfig->options);
}

void
GSwitchItXkbConfigOptionsAdd (GSwitchItXkbConfig * xkbConfig,
			      const gchar * groupName,
			      const gchar * optionName)
{
	const char *merged;
	if (groupName == NULL || optionName == NULL)
		return;
	merged = GSwitchItConfigMergeItems (groupName, optionName);
	if (merged == NULL)
		return;
	_GSwitchItXkbConfigOptionsAdd (xkbConfig, merged);
}

gboolean
GSwitchItXkbConfigOptionsIsSet (GSwitchItXkbConfig * xkbConfig,
				const gchar * groupName,
				const gchar * optionName)
{
	const char *merged =
	    GSwitchItConfigMergeItems (groupName, optionName);
	if (merged == NULL)
		return FALSE;

	return NULL != g_slist_find_custom (xkbConfig->options, (gpointer)
					    merged, (GCompareFunc)
					    g_ascii_strcasecmp);
}

gboolean
GSwitchItXkbConfigActivate (GSwitchItXkbConfig * xkbConfig)
{
	return _GSwitchItXkbConfigDoWithSettings (xkbConfig, TRUE, NULL);
}

gboolean
GSwitchItXkbConfigDumpSettings (GSwitchItXkbConfig * xkbConfig,
				const char *fileName)
{
	return _GSwitchItXkbConfigDoWithSettings (xkbConfig, FALSE,
						  fileName);
}

void
GSwitchItXkbConfigStartListen (GSwitchItXkbConfig * xkbConfig,
			       GConfClientNotifyFunc func,
			       gpointer userData)
{
	_GSwitchItConfigAddListener (xkbConfig->confClient,
				     GSWITCHIT_CONFIG_XKB_DIR, func,
				     userData, &gconfXkbListenerId);
}

void
GSwitchItXkbConfigStopListen (GSwitchItXkbConfig * xkbConfig)
{
	_GSwitchItConfigRemoveListener (xkbConfig->confClient,
					&gconfXkbListenerId);
}

/**
 * extern applet config functions
 */
void
GSwitchItAppletConfigFreeImages (GSwitchItAppletConfig * appletConfig)
{
	int i;
	GdkPixbuf **pi = appletConfig->images;
	for (i = XkbNumKbdGroups; --i >= 0; pi++) {
		if (*pi) {
			gdk_pixbuf_unref (*pi);
			*pi = NULL;
		}
	}
}

const char *
GSwitchItAppletConfigGetImagesFile (GSwitchItAppletConfig *
				    appletConfig,
				    GSwitchItXkbConfig *
				    xkbConfig, int group)
{
	char *imageFile = NULL;
	GtkIconInfo *iconInfo = NULL;

	if (!appletConfig->showFlags)
		return NULL;

	if ((xkbConfig->layouts != NULL) &&
	    (g_slist_length (xkbConfig->layouts) > group)) {
		char *fullLayoutName =
		    (char *) g_slist_nth_data (xkbConfig->layouts, group);

		if (fullLayoutName != NULL) {
			char *l, *v;
			GSwitchItConfigSplitItems (fullLayoutName, &l, &v);
			if (l != NULL) {
				// probably there is something in theme?
				iconInfo = gtk_icon_theme_lookup_icon
				    (appletConfig->iconTheme, l, 48, 0);
			}
		}
	}
	// fallback to the default value
	if (iconInfo == NULL) {
		iconInfo = gtk_icon_theme_lookup_icon
		    (appletConfig->iconTheme, "applet-error", 48, 0);
	}
	if (iconInfo != NULL) {
		imageFile =
		    g_strdup (gtk_icon_info_get_filename (iconInfo));
		gtk_icon_info_free (iconInfo);
	}

	return imageFile;
}

void
GSwitchItAppletConfigLoadImages (GSwitchItAppletConfig * appletConfig,
				 GSwitchItXkbConfig * xkbConfig)
{
	GdkPixbuf **image = appletConfig->images;
	int i, j = 0;
	if (!appletConfig->showFlags)
		return;

	for (i = XkbNumKbdGroups; --i >= 0; j++, image++) {
		const char *imageFile =
		    GSwitchItAppletConfigGetImagesFile (appletConfig,
							xkbConfig, j);

		if (imageFile != NULL) {
			GError *err = NULL;
			*image =
			    gdk_pixbuf_new_from_file (imageFile, &err);
			if (*image == NULL) {
				gnome_error_dialog (err->message);
				g_error_free (err);
			}
			XklDebug (150,
				  "Image %d[%s] loaded -> %p[%dx%d]\n",
				  i, imageFile, *image,
				  gdk_pixbuf_get_width (*image),
				  gdk_pixbuf_get_height (*image));
		}
	}
}

void
GSwitchItAppletConfigInit (GSwitchItAppletConfig * appletConfig,
			   GConfClient * confClient)
{
	GError *gerror = NULL;
	gchar *sp, *datadir;

	memset (appletConfig, 0, sizeof (*appletConfig));
	appletConfig->confClient = confClient;
	g_object_ref (appletConfig->confClient);

	gconf_client_add_dir (appletConfig->confClient,
			      GSWITCHIT_CONFIG_APPLET_DIR,
			      GCONF_CLIENT_PRELOAD_NONE, &gerror);
	if (gerror != NULL) {
		g_warning ("err1:%s\n", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}

	appletConfig->iconTheme = gtk_icon_theme_get_default ();

	datadir =
	    gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_DATADIR,
				       "", FALSE, NULL);
	gtk_icon_theme_append_search_path (appletConfig->iconTheme, sp =
					   g_build_filename (g_get_home_dir
							     (),
							     ".icons/flags",
							     NULL));
	g_free (sp);

	gtk_icon_theme_append_search_path (appletConfig->iconTheme,
					   sp =
					   g_build_filename (datadir,
							     "pixmaps/flags",
							     NULL));
	g_free (sp);

	gtk_icon_theme_append_search_path (appletConfig->iconTheme,
					   sp =
					   g_build_filename (datadir,
							     "icons/flags",
							     NULL));
	g_free (sp);
	g_free (datadir);
}

void
GSwitchItAppletConfigTerm (GSwitchItAppletConfig * appletConfig)
{
#if 0
	g_object_unref (G_OBJECT (appletConfig->iconTheme));
#endif
	appletConfig->iconTheme = NULL;

	GSwitchItAppletConfigFreeImages (appletConfig);

	_GSwitchItAppletConfigFreeEnabledPlugins (appletConfig);
	g_object_unref (appletConfig->confClient);
	appletConfig->confClient = NULL;
}

void
GSwitchItAppletConfigUpdateImages (GSwitchItAppletConfig * appletConfig,
				   GSwitchItXkbConfig * xkbConfig)
{
	GSwitchItAppletConfigFreeImages (appletConfig);
	GSwitchItAppletConfigLoadImages (appletConfig, xkbConfig);
}

void
GSwitchItAppletConfigLoad (GSwitchItAppletConfig * appletConfig)
{
	GError *gerror = NULL;

	appletConfig->secondaryGroupsMask =
	    gconf_client_get_int (appletConfig->confClient,
				  GSWITCHIT_CONFIG_APPLET_KEY_SECONDARIES,
				  &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		appletConfig->secondaryGroupsMask = 0;
		g_error_free (gerror);
		gerror = NULL;
	}

	appletConfig->groupPerApp =
	    gconf_client_get_bool (appletConfig->confClient,
				   GSWITCHIT_CONFIG_APPLET_KEY_GROUP_PER_WINDOW,
				   &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		appletConfig->groupPerApp = FALSE;
		g_error_free (gerror);
		gerror = NULL;
	}

	appletConfig->handleIndicators =
	    gconf_client_get_bool (appletConfig->confClient,
				   GSWITCHIT_CONFIG_APPLET_KEY_HANDLE_INDICATORS,
				   &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		appletConfig->handleIndicators = FALSE;
		g_error_free (gerror);
		gerror = NULL;
	}

	appletConfig->layoutNamesAsGroupNames =
	    gconf_client_get_bool (appletConfig->confClient,
				   GSWITCHIT_CONFIG_APPLET_KEY_LAYOUT_NAMES_AS_GROUP_NAMES,
				   &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		appletConfig->layoutNamesAsGroupNames = TRUE;
		g_error_free (gerror);
		gerror = NULL;
	}

	appletConfig->showFlags =
	    gconf_client_get_bool (appletConfig->confClient,
				   GSWITCHIT_CONFIG_APPLET_KEY_SHOW_FLAGS,
				   &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		appletConfig->showFlags = FALSE;
		g_error_free (gerror);
		gerror = NULL;
	}

	appletConfig->debugLevel =
	    gconf_client_get_int (appletConfig->confClient,
				  GSWITCHIT_CONFIG_APPLET_KEY_DEBUG_LEVEL,
				  &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		appletConfig->debugLevel = -1;
		g_error_free (gerror);
		gerror = NULL;
	}

	appletConfig->defaultGroup =
	    gconf_client_get_int (appletConfig->confClient,
				  GSWITCHIT_CONFIG_APPLET_KEY_DEFAULT_GROUP,
				  &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		appletConfig->defaultGroup = -1;
		g_error_free (gerror);
		gerror = NULL;
	}

	if (appletConfig->defaultGroup < -1
	    || appletConfig->defaultGroup >= XkbNumKbdGroups)
		appletConfig->defaultGroup = -1;

	_GSwitchItAppletConfigFreeEnabledPlugins (appletConfig);
	appletConfig->enabledPlugins =
	    gconf_client_get_list (appletConfig->confClient,
				   GSWITCHIT_CONFIG_APPLET_KEY_ENABLED_PLUGINS,
				   GCONF_VALUE_STRING, &gerror);

	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		appletConfig->enabledPlugins = NULL;
		g_error_free (gerror);
		gerror = NULL;
	}
}

void
GSwitchItAppletConfigSave (GSwitchItAppletConfig * appletConfig,
			   GSwitchItXkbConfig * xkbConfig)
{
	GConfChangeSet *cs;
	GError *gerror = NULL;

	cs = gconf_change_set_new ();

	gconf_change_set_set_int (cs,
				  GSWITCHIT_CONFIG_APPLET_KEY_SECONDARIES,
				  appletConfig->secondaryGroupsMask);
	gconf_change_set_set_bool (cs,
				   GSWITCHIT_CONFIG_APPLET_KEY_GROUP_PER_WINDOW,
				   appletConfig->groupPerApp);
	gconf_change_set_set_bool (cs,
				   GSWITCHIT_CONFIG_APPLET_KEY_HANDLE_INDICATORS,
				   appletConfig->handleIndicators);
	gconf_change_set_set_bool (cs,
				   GSWITCHIT_CONFIG_APPLET_KEY_LAYOUT_NAMES_AS_GROUP_NAMES,
				   appletConfig->layoutNamesAsGroupNames);
	gconf_change_set_set_bool (cs,
				   GSWITCHIT_CONFIG_APPLET_KEY_SHOW_FLAGS,
				   appletConfig->showFlags);
	gconf_change_set_set_int (cs,
				  GSWITCHIT_CONFIG_APPLET_KEY_DEBUG_LEVEL,
				  appletConfig->debugLevel);
	gconf_change_set_set_int (cs,
				  GSWITCHIT_CONFIG_APPLET_KEY_DEFAULT_GROUP,
				  appletConfig->defaultGroup);
	gconf_change_set_set_list (cs,
				   GSWITCHIT_CONFIG_APPLET_KEY_ENABLED_PLUGINS,
				   GCONF_VALUE_STRING,
				   appletConfig->enabledPlugins);

	gconf_client_commit_change_set (appletConfig->confClient, cs, TRUE,
					&gerror);
	if (gerror != NULL) {
		g_warning ("Error saving configuration: %s\n",
			   gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}
	gconf_change_set_unref (cs);
}

void
GSwitchItAppletConfigLockNextGroup ()
{
	int group = XklGetNextGroup ();
	XklLockGroup (group);
}

void
GSwitchItAppletConfigLockPrevGroup ()
{
	int group = XklGetPrevGroup ();
	XklLockGroup (group);
}

void
GSwitchItAppletConfigRestoreGroup ()
{
	int group = XklGetRestoreGroup ();
	XklLockGroup (group);
}

void
GSwitchItAppletConfigActivate (GSwitchItAppletConfig * appletConfig)
{
	XklSetGroupPerApp (appletConfig->groupPerApp);
	XklSetIndicatorsHandling (appletConfig->handleIndicators);
	XklSetSecondaryGroupsMask (appletConfig->secondaryGroupsMask);
	XklSetDefaultGroup (appletConfig->defaultGroup);
	if (appletConfig->debugLevel != -1)
		XklSetDebugLevel (appletConfig->debugLevel);
}

void
GSwitchItAppletConfigStartListen (GSwitchItAppletConfig * appletConfig,
				  GConfClientNotifyFunc func,
				  gpointer userData)
{
	_GSwitchItConfigAddListener (appletConfig->confClient,
				     GSWITCHIT_CONFIG_APPLET_DIR, func,
				     userData, &gconfAppletListenerId);
}

void
GSwitchItAppletConfigStopListen (GSwitchItAppletConfig * appletConfig)
{
	_GSwitchItConfigRemoveListener (appletConfig->confClient,
					&gconfAppletListenerId);
}

Bool
GSwitchItConfigGetDescriptions (const char *name,
				char **layoutShortDescr,
				char **layoutDescr,
				char **variantShortDescr,
				char **variantDescr)
{
	char *layoutName = NULL, *variantName = NULL;
	if (!GSwitchItConfigSplitItems (name, &layoutName, &variantName))
		return FALSE;
	return _GSwitchItConfigGetDescriptions (layoutName, variantName,
						layoutShortDescr,
						layoutDescr,
						variantShortDescr,
						variantDescr);
}

const char *
GSwitchItConfigFormatFullLayout (const char *layoutDescr,
				 const char *variantDescr)
{
	static char fullDescr[XKL_MAX_CI_DESC_LENGTH * 2];
	if (variantDescr == NULL)
		g_snprintf (fullDescr, sizeof (fullDescr), "%s",
			    layoutDescr);
	else
		g_snprintf (fullDescr, sizeof (fullDescr), "%s %s",
			    layoutDescr, variantDescr);
	return fullDescr;
}

void
GSwitchItAppletConfigLoadGroupDescriptionsUtf8 (GSwitchItAppletConfig *
						appletConfig,
						GroupDescriptionsBuffer
						namesToFill)
{
	int i;
	const char **pNativeNames = XklGetGroupNames ();
	char *pDest = (char *) namesToFill;
	// first fill with defaults
	for (i = XklGetNumGroups (); --i >= 0;
	     pDest += sizeof (namesToFill[0]))
		g_snprintf (pDest, sizeof (namesToFill[0]), "%s",
			    *pNativeNames++);

	pDest = (char *) namesToFill;
	if (XklMultipleLayoutsSupported ()
	    && appletConfig->layoutNamesAsGroupNames) {
		XklConfigRec xklConfig;
		if (XklConfigGetFromServer (&xklConfig)) {
			char **pl = xklConfig.layouts;
			char **pv = xklConfig.variants;
			for (i = xklConfig.numLayouts; --i >= 0;
			     pDest += sizeof (namesToFill[0])) {
				char *lSDescr;
				char *lDescr;
				char *vSDescr;
				char *vDescr;
				if (_GSwitchItConfigGetDescriptions
				    (*pl++, *pv++, &lSDescr, &lDescr,
				     &vSDescr, &vDescr)) {
					char *nameUtf =
					    g_locale_to_utf8
					    (GSwitchItConfigFormatFullLayout
					     (lDescr, vDescr), -1, NULL,
					     NULL, NULL);
					g_snprintf (pDest,
						    sizeof (namesToFill
							    [0]), "%s",
						    nameUtf);
					g_free (nameUtf);
				}
			}
		}
	}
}
