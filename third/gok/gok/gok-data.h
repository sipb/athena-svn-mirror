/* gok-data.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#ifndef __GOKDATA__H__
#define __GOKDATA__H__

#include <glib.h>
#include <gconf/gconf-client.h>
#include "gok-scanner.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* maximum length of a setting name */
#define MAX_SETTING_NAME 30

/* file that contains all the settings */
#define GOK_SETTINGS_FILE "gok-settings.xml"

/* maximum key spacing */
#define MAX_KEY_SPACING 100

/* maximum key width */
#define MAX_KEY_WIDTH 200

/* minimum key width */
#define MIN_KEY_WIDTH 5

/* maximum key height */
#define MAX_KEY_HEIGHT 100

/* minimum key height */
#define MIN_KEY_HEIGHT 5

/* a GOK setting */
/* if you modify this structure update gok_data_construct_setting */
typedef struct GokSetting{
	gchar Name[MAX_SETTING_NAME + 1];
	gchar NameAccessMethod[MAX_ACCESS_METHOD_NAME + 1];
	gchar* pValueString;
	gchar* pValueStringBackup;
	gint Value;
	gint ValueBackup;
	gboolean bIsAction;
	struct GokSetting* pSettingNext;
} GokSetting;
  
typedef enum {
  GOK_DOCK_NONE,
  GOK_DOCK_BOTTOM,
  GOK_DOCK_TOP
} GokDockType;
  
typedef enum {
  GOK_COMPOSE_DEFAULT,
  GOK_COMPOSE_XKB,
  GOK_COMPOSE_ALPHA,
  GOK_COMPOSE_ALPHAFREQ,
  GOK_COMPOSE_CUSTOM
} GokComposeType;
  
void gok_data_initialize (void);
GokSetting* gok_data_construct_setting (void);
gboolean gok_data_read_settings (void);
gboolean gok_data_write_settings (void);
gboolean gok_data_get_setting (gchar* NameAccessMethod, gchar* NameSetting, gint* Value, gchar** ValueString);
gboolean gok_data_create_setting (gchar* NameAccessMethod, gchar* NameSetting, gint Value, gchar* pValueString);
void gok_data_close (void);
gboolean gok_data_set_setting (gchar* NameAccessMethod, gchar* NameSetting, gint Value, gchar* ValueString);

gint gok_data_get_key_width (void);
void gok_data_set_key_width (gint Width);
gint gok_data_get_key_height (void);
void gok_data_set_key_height (gint Height);
gint gok_data_get_key_spacing (void);
void gok_data_set_key_spacing (gint Spacing);
gint gok_data_get_keyboard_x (void);
void gok_data_set_keyboard_x (gint X);
gint gok_data_get_keyboard_y (void);
void gok_data_set_keyboard_y (gint Y);
gboolean gok_data_get_keysize_priority (void);
void gok_data_set_keysize_priority (gboolean bFlag);
char* gok_data_get_name_accessmethod (void);
void	gok_data_set_name_accessmethod (const gchar* Name);
gboolean gok_data_get_wordcomplete (void);
void gok_data_set_wordcomplete (gboolean bTrueFalse);
gboolean gok_data_get_commandprediction();
void gok_data_set_commandprediction(gboolean bTrueFalse);
gint gok_data_get_num_predictions (void);
void gok_data_set_num_predictions(gint Number);
void gok_data_backup_settings (void);
gboolean gok_data_restore_settings (void);
void gok_data_backup_setting (GokSetting* pSetting);
gboolean gok_data_restore_setting (GokSetting* pSetting);
gboolean gok_data_get_control_values (GokControl* pControl);
gboolean gok_data_get_use_gtkplus_theme (void);
void gok_data_set_use_gtkplus_theme (gboolean bTrueFalse);
gboolean gok_data_get_drive_corepointer (void);
void gok_data_set_drive_corepointer (gboolean bTrueFalse);
gboolean gok_data_get_use_xkb_kbd (void);
void gok_data_set_use_xkb_kbd (gboolean bTrueFalse);
gboolean gok_data_get_expand (void);
void gok_data_set_expand (gboolean bTrueFalse);
gdouble gok_data_get_valuator_sensitivity (void);
void gok_data_set_valuator_sensitivity (gdouble multiplier);
GokDockType gok_data_get_dock_type (void);
void gok_data_set_dock_type (GokDockType dock_type);
gint gok_data_get_repeat_rate (void);
void gok_data_set_repeat_rate (gint rate);
GConfClient* gok_data_get_gconf_client (void);
gchar* gok_data_get_aux_dictionaries (void);
void gok_data_set_aux_dictionaries (gchar *dictionaries);
gboolean gok_data_get_use_aux_dictionaries (void);
void gok_data_set_use_aux_dictionaries (gboolean use_aux);
GokComposeType gok_data_get_compose_keyboard_type (void);
void gok_data_set_compose_keyboard_type (GokComposeType type);
gchar *gok_data_get_custom_compose_filename (void);
void gok_data_set_custom_compose_filename (const gchar *filename);
gchar *gok_data_get_aux_keyboard_directory (void);
void gok_data_set_aux_keyboard_directory (const gchar *filename);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKDATA__H__ */
