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

#ifndef __GSWITCHIT_APPLET_CONFIG_H__
#define __GSWITCHIT_APPLET_CONFIG_H__

#include "libgswitchit/gswitchit_xkb_config_private.h"
#include "libgswitchit/gswitchit_plugin.h"

/*
 * Applet configuration
 */
typedef struct _GSwitchItAppletConfig {
	int secondaryGroupsMask;
	int defaultGroup;

	gboolean groupPerApp;
	gboolean handleIndicators;
	gboolean layoutNamesAsGroupNames;
	gboolean showFlags;

	int debugLevel;

	GSList *enabledPlugins;
// transient part
	GConfClient *confClient;

	GdkPixbuf *images[XkbNumKbdGroups];

	GtkIconTheme *iconTheme;
} GSwitchItAppletConfig;

/**
 * Applet config functions - some of them require XKB config as well
 */
extern void GSwitchItAppletConfigInit (GSwitchItAppletConfig *
				       appletConfig,
				       GConfClient * confClient);
extern void GSwitchItAppletConfigTerm (GSwitchItAppletConfig *
				       appletConfig);

extern void GSwitchItAppletConfigLoad (GSwitchItAppletConfig *
				       appletConfig);
extern void GSwitchItAppletConfigSave (GSwitchItAppletConfig *
				       appletConfig,
				       GSwitchItXkbConfig * xkbConfig);

extern const char
*GSwitchItAppletConfigGetImagesFile (GSwitchItAppletConfig *
				     appletConfig,
				     GSwitchItXkbConfig * xkbConfig,
				     int group);

extern void GSwitchItAppletConfigLoadImages (GSwitchItAppletConfig *
					     appletConfig,
					     GSwitchItXkbConfig *
					     xkbConfig);
extern void GSwitchItAppletConfigFreeImages (GSwitchItAppletConfig *
					     appletConfig);

// should be updated on Applet/GConf and XKB/GConf configuration change
extern void GSwitchItAppletConfigUpdateImages (GSwitchItAppletConfig *
					       appletConfig,
					       GSwitchItXkbConfig *
					       xkbConfig);

extern void GSwitchItAppletConfigLockNextGroup (void);
extern void GSwitchItAppletConfigLockPrevGroup (void);
extern void GSwitchItAppletConfigRestoreGroup (void);

// should be updated on Applet/GConf configuration change
extern void GSwitchItAppletConfigActivate (GSwitchItAppletConfig *
					   appletConfig);

extern void GSwitchItAppletConfigStartListen (GSwitchItAppletConfig *
					      appletConfig,
					      GConfClientNotifyFunc
					      func, gpointer user_data);

extern void GSwitchItAppletConfigStopListen (GSwitchItAppletConfig *
					     appletConfig);

// affected by XKB and XKB/GConf configuration
extern void
 GSwitchItAppletConfigLoadGroupDescriptionsUtf8 (GSwitchItAppletConfig *
						 appletConfig,
						 GroupDescriptionsBuffer
						 namesToFill);

#endif
