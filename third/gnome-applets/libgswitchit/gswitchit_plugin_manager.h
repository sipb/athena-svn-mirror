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

#ifndef __GSWITCHIT_PLUGIN_MANAGER_H__
#define __GSWITCHIT_PLUGIN_MANAGER_H__

#include <gmodule.h>
#include <libgswitchit/gswitchit_plugin.h>

typedef struct _GSwitchItPluginManager {
	GHashTable *allPluginRecs;
	GSList *initedPluginRecs;
} GSwitchItPluginManager;

typedef struct _GSwitchItPluginManagerRecord {
	const char *fullPath;
	GModule *module;
	const GSwitchItPlugin *plugin;
} GSwitchItPluginManagerRecord;

extern void GSwitchItPluginManagerInit (GSwitchItPluginManager * manager);

extern void GSwitchItPluginManagerTerm (GSwitchItPluginManager * manager);

extern void
GSwitchItPluginManagerInitEnabledPlugins (GSwitchItPluginManager * manager,
					  GSwitchItPluginContainer * pc,
					  GSList * enabledPlugins);

extern void
 GSwitchItPluginManagerTermInitializedPlugins (GSwitchItPluginManager *
					       manager);

extern void GSwitchItPluginManagerTogglePlugins (GSwitchItPluginManager *
						 manager,
						 GSwitchItPluginContainer *
						 pc,
						 GSList * enabledPlugins);

extern const GSwitchItPlugin
    * GSwitchItPluginManagerGetPlugin (GSwitchItPluginManager * manager,
				       const char *fullPath);

extern void GSwitchItPluginManagerPromotePlugin (GSwitchItPluginManager *
						 manager,
						 GSList * enabledPlugins,
						 const char *fullPath);

extern void GSwitchItPluginManagerDemotePlugin (GSwitchItPluginManager *
						manager,
						GSList * enabledPlugins,
						const char *fullPath);

extern void GSwitchItPluginManagerEnablePlugin (GSwitchItPluginManager *
						manager,
						GSList ** pEnabledPlugins,
						const char *fullPath);

extern void GSwitchItPluginManagerDisablePlugin (GSwitchItPluginManager *
						 manager,
						 GSList ** pEnabledPlugins,
						 const char *fullPath);

extern void GSwitchItPluginManagerConfigurePlugin (GSwitchItPluginManager *
						   manager,
						   GSwitchItPluginContainer
						   * pc,
						   const char *fullPath,
						   GtkWindow * parent);

// actual calling plugin notification methods

extern void GSwitchItPluginManagerGroupChanged (GSwitchItPluginManager *
						manager, int newGroup);

extern int GSwitchItPluginManagerWindowCreated (GSwitchItPluginManager *
						manager,
						Window win, Window parent);

extern GtkWidget
    *GSwitchItPluginManagerDecorateWidget (GSwitchItPluginManager *
					   manager, GtkWidget * widget,
					   const gint group, const char
					   *groupDescription,
					   GSwitchItXkbConfig * config);

#endif
