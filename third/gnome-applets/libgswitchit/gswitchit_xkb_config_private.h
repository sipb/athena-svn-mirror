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

#ifndef __GSWITCHIT_XKB_CONFIG_PRIVATE_H__
#define __GSWITCHIT_XKB_CONFIG_PRIVATE_H__

#include "libgswitchit/gswitchit_xkb_config.h"

/**
 * XKB config functions
 */
extern void GSwitchItXkbConfigSave (GSwitchItXkbConfig * xkbConfig);
extern void GSwitchItXkbConfigLoadCurrent (GSwitchItXkbConfig * xkbConfig);

extern void GSwitchItXkbConfigModelSet (GSwitchItXkbConfig * xkbConfig,
					const gchar * modelName);

extern void GSwitchItXkbConfigLayoutsReset (GSwitchItXkbConfig *
					    xkbConfig);
extern void GSwitchItXkbConfigLayoutsAdd (GSwitchItXkbConfig * xkbConfig,
					  const gchar * layoutName,
					  const gchar * variantName);

extern void GSwitchItXkbConfigLayoutsReset (GSwitchItXkbConfig *
					    xkbConfig);
extern void GSwitchItXkbConfigOptionsReset (GSwitchItXkbConfig *
					    xkbConfig);

extern void GSwitchItXkbConfigOptionsAdd (GSwitchItXkbConfig * xkbConfig,
					  const gchar * groupName,
					  const gchar * optionName);
extern gboolean GSwitchItXkbConfigOptionsIsSet (GSwitchItXkbConfig *
						xkbConfig,
						const gchar * groupName,
						const gchar * optionName);

extern gboolean GSwitchItXkbConfigDumpSettings (GSwitchItXkbConfig *
						xkbConfig,
						const char *fileName);

extern void GSwitchItXkbConfigStartListen (GSwitchItXkbConfig *
					   xkbConfig,
					   GConfClientNotifyFunc
					   func, gpointer user_data);

extern void GSwitchItXkbConfigStopListen (GSwitchItXkbConfig * xkbConfig);

#endif
