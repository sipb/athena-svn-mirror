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

#include <gnome.h>

#include <libxklavier/xklavier.h>

#include "libgswitchit/gswitchit_plugin_manager.h"

#define FOR_EACH_INITED_PLUGIN() \
{ \
  GSList *prec; \
  for( prec = manager->initedPluginRecs; prec != NULL; prec = prec->next ) \
  { \
    const GSwitchItPlugin *plugin = \
      ( ( GSwitchItPluginManagerRecord * ) ( prec->data ) )->plugin; \
    if( plugin != NULL ) \
    {

#define END_FOR_EACH_INITED_PLUGIN() \
    } \
  } \
}

static void
_GSwitchItPluginManagerAddPluginsDir (GSwitchItPluginManager *
				      manager, const char *dirname)
{
	GDir *dir = g_dir_open (dirname, 0, NULL);
	const gchar *filename;
	if (dir == NULL)
		return;

	XklDebug (100, "Scanning [%s]...\n", dirname);
	while ((filename = g_dir_read_name (dir)) != NULL) {
		gchar *fullPath =
		    g_build_filename (dirname, filename, NULL);
		XklDebug (100, "Loading plugin module [%s]...\n",
			  fullPath);
		if (fullPath != NULL) {
			GModule *module = g_module_open (fullPath, 0);
			if (module != NULL) {
				gpointer getPluginFunc;
				if (g_module_symbol
				    (module, "GetPlugin",
				     &getPluginFunc)) {
					const GSwitchItPlugin *plugin =
					    ((GSwitchItPluginGetPluginFunc)
					     getPluginFunc) ();
					if (plugin != NULL) {
						GSwitchItPluginManagerRecord
						    *rec =
						    g_new0
						    (GSwitchItPluginManagerRecord,
						     1);
						XklDebug (100,
							  "Loaded plugin from [%s]: [%s]/[%s]...\n",
							  fullPath,
							  plugin->name,
							  plugin->
							  description);
						rec->fullPath = fullPath;
						rec->module = module;
						rec->plugin = plugin;
						g_hash_table_insert
						    (manager->
						     allPluginRecs,
						     fullPath, rec);
						continue;
					}
				} else
					XklDebug (0,
						  "Bad plugin: [%s]\n",
						  fullPath);
				g_module_close (module);
			} else
				XklDebug (0, "Bad module: [%s], %s\n",
					  fullPath, g_module_error());
			g_free (fullPath);
		}
	}
}

static void
_GSwitchItPluginManagerLoadAll (GSwitchItPluginManager * manager)
{
	if (!g_module_supported ()) {
		XklDebug (0, "Modules are not supported - no plugins!\n");
		return;
	}
	_GSwitchItPluginManagerAddPluginsDir (manager, SYS_PLUGIN_DIR);
}

static void
_GSwitchItPluginManagerRecTerm (GSwitchItPluginManagerRecord *
				rec, void *userData)
{
	const GSwitchItPlugin *plugin = rec->plugin;
	if (plugin != NULL) {
		XklDebug (100, "Terminating plugin: [%s]...\n",
			  plugin->name);
		if (plugin->termCallback)
			(*plugin->termCallback) ();
	}
}

static void
_GSwitchItPluginManagerRecDestroy (GSwitchItPluginManagerRecord * rec)
{
	XklDebug (100, "Unloading plugin: [%s]...\n", rec->plugin->name);

	g_module_close (rec->module);
	g_free (rec);
}

void
GSwitchItPluginManagerInit (GSwitchItPluginManager * manager)
{
	manager->allPluginRecs =
	    g_hash_table_new_full (g_str_hash, g_str_equal,
				   (GDestroyNotify) g_free,
				   (GDestroyNotify)
				   _GSwitchItPluginManagerRecDestroy);
	_GSwitchItPluginManagerLoadAll (manager);
}

void
GSwitchItPluginManagerTerm (GSwitchItPluginManager * manager)
{
	GSwitchItPluginManagerTermInitializedPlugins (manager);
	if (manager->allPluginRecs != NULL) {
		g_hash_table_destroy (manager->allPluginRecs);
		manager->allPluginRecs = NULL;
	}
}

void
GSwitchItPluginManagerInitEnabledPlugins (GSwitchItPluginManager *
					  manager,
					  GSwitchItPluginContainer *
					  pc, GSList * enabledPlugins)
{
	GSList *pluginNameNode = enabledPlugins;
	if (manager->allPluginRecs == NULL)
		return;
	XklDebug (100, "Initializing all enabled plugins...\n");
	while (pluginNameNode != NULL) {
		const char *fullPath = pluginNameNode->data;
		if (fullPath != NULL) {
			GSwitchItPluginManagerRecord *rec =
			    (GSwitchItPluginManagerRecord *)
			    g_hash_table_lookup (manager->allPluginRecs,
						 fullPath);

			if (rec != NULL) {
				const GSwitchItPlugin *plugin =
				    rec->plugin;
				gboolean initialized = FALSE;
				XklDebug (100,
					  "Initializing plugin: [%s] from [%s]...\n",
					  plugin->name, fullPath);
				if (plugin->initCallback != NULL)
					initialized =
					    (*plugin->initCallback) (pc);
				else
					initialized = TRUE;

				manager->initedPluginRecs =
				    g_slist_append (manager->
						    initedPluginRecs, rec);
				XklDebug (100,
					  "Plugin [%s] initialized: %d\n",
					  plugin->name, initialized);
			}
		}
		pluginNameNode = g_slist_next (pluginNameNode);
	}
}

void
GSwitchItPluginManagerTermInitializedPlugins (GSwitchItPluginManager *
					      manager)
{

	if (manager->initedPluginRecs == NULL)
		return;
	g_slist_foreach (manager->initedPluginRecs,
			 (GFunc) _GSwitchItPluginManagerRecTerm, NULL);
	g_slist_free (manager->initedPluginRecs);
	manager->initedPluginRecs = NULL;
}

void
GSwitchItPluginManagerTogglePlugins (GSwitchItPluginManager * manager,
				     GSwitchItPluginContainer * pc,
				     GSList * enabledPlugins)
{
	GSwitchItPluginManagerTermInitializedPlugins (manager);
	GSwitchItPluginManagerInitEnabledPlugins (manager, pc,
						  enabledPlugins);
}

void
GSwitchItPluginManagerGroupChanged (GSwitchItPluginManager *
				    manager, int newGroup)
{
	FOR_EACH_INITED_PLUGIN ();
	if (plugin->groupChangedCallback)
		(*plugin->groupChangedCallback) (newGroup);
	END_FOR_EACH_INITED_PLUGIN ();
}

const GSwitchItPlugin *
GSwitchItPluginManagerGetPlugin (GSwitchItPluginManager * manager,
				 const char *fullPath)
{
	GSwitchItPluginManagerRecord *rec =
	    (GSwitchItPluginManagerRecord *) g_hash_table_lookup (manager->
								  allPluginRecs,
								  fullPath);
	if (rec == NULL)
		return NULL;
	return rec->plugin;
}

void
GSwitchItPluginManagerPromotePlugin (GSwitchItPluginManager *
				     manager,
				     GSList * enabledPlugins,
				     const char *fullPath)
{
	GSList *theNode = enabledPlugins;
	GSList *prevNode = NULL;

	while (theNode != NULL) {
		if (!strcmp (theNode->data, fullPath)) {
			if (prevNode != NULL) {
				char *tmp = (char *) prevNode->data;
				prevNode->data = theNode->data;
				theNode->data = tmp;
			}
			break;
		}
		prevNode = theNode;
		theNode = g_slist_next (theNode);
	}
}

void
GSwitchItPluginManagerDemotePlugin (GSwitchItPluginManager *
				    manager,
				    GSList * enabledPlugins,
				    const char *fullPath)
{
	GSList *theNode = g_slist_find_custom (enabledPlugins, fullPath,
					       (GCompareFunc) strcmp);
	if (theNode != NULL) {
		GSList *nextNode = g_slist_next (theNode);
		if (nextNode != NULL) {
			char *tmp = (char *) nextNode->data;
			nextNode->data = theNode->data;
			theNode->data = tmp;
		}
	}
}

void
GSwitchItPluginManagerEnablePlugin (GSwitchItPluginManager *
				    manager,
				    GSList ** pEnabledPlugins,
				    const char *fullPath)
{
	if (NULL != GSwitchItPluginManagerGetPlugin (manager, fullPath)) {
		*pEnabledPlugins =
		    g_slist_append (*pEnabledPlugins,
				    (gpointer) g_strdup (fullPath));
	}
}

extern void
GSwitchItPluginManagerDisablePlugin (GSwitchItPluginManager *
				     manager,
				     GSList ** pEnabledPlugins,
				     const char *fullPath)
{
	GSList *theNode = g_slist_find_custom (*pEnabledPlugins, fullPath,
					       (GCompareFunc) strcmp);
	if (theNode != NULL) {
		g_free (theNode->data);
		*pEnabledPlugins =
		    g_slist_delete_link (*pEnabledPlugins, theNode);
	}
}

int
GSwitchItPluginManagerWindowCreated (GSwitchItPluginManager *
				     manager, Window win, Window parent)
{
	FOR_EACH_INITED_PLUGIN ();
	if (plugin->windowCreatedCallback) {
		int groupToAssign =
		    (*plugin->windowCreatedCallback) (win, parent);
		if (groupToAssign != -1) {
			XklDebug (100,
				  "Plugin [%s] assigned group %d to new window %ld\n",
				  plugin->name, groupToAssign, win);
			return groupToAssign;
		}
	}
	END_FOR_EACH_INITED_PLUGIN ();
	return -1;
}

GtkWidget *
GSwitchItPluginManagerDecorateWidget (GSwitchItPluginManager *
				      manager,
				      GtkWidget * widget,
				      const gint group,
				      const char
				      *groupDescription,
				      GSwitchItXkbConfig * config)
{
	FOR_EACH_INITED_PLUGIN ();
	if (plugin->decorateWidgetCallback) {
		GtkWidget *decoratedWidget =
		    (*plugin->decorateWidgetCallback) (widget, group,
						       groupDescription,
						       config);
		if (decoratedWidget != NULL) {
			XklDebug (100,
				  "Plugin [%s] decorated widget %p to %p\n",
				  plugin->name, widget, decoratedWidget);
			return decoratedWidget;
		}
	}
	END_FOR_EACH_INITED_PLUGIN ();
	return NULL;
}

void
GSwitchItPluginManagerConfigurePlugin (GSwitchItPluginManager *
				       manager,
				       GSwitchItPluginContainer * pc,
				       const char *fullPath,
				       GtkWindow * parent)
{
	const GSwitchItPlugin *plugin =
	    GSwitchItPluginManagerGetPlugin (manager, fullPath);
	if (plugin->configurePropertiesCallback != NULL)
		plugin->configurePropertiesCallback (pc, parent);
}

void
GSwitchItPluginContainerInit (GSwitchItPluginContainer * pc,
			      GConfClient * confClient)
{
	pc->confClient = confClient;
	g_object_ref (pc->confClient);
}

void
GSwitchItPluginContainerTerm (GSwitchItPluginContainer * pc)
{
	g_object_unref (pc->confClient);
}

guint
GSwitchItPluginGetNumGroups (GSwitchItPluginContainer * pc)
{
	return XklGetNumGroups ();
}
