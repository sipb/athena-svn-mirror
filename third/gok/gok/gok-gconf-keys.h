/*
* gok-gconf-keys.h
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
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

/*
 * This file defines a number of constants for accessing gok
 * GConf keys.
 */

#define GOK_GCONF_ROOT                     "/apps/gok"
#define GOK_GCONF_LAYOUT                   "/apps/gok/layout"
#define GOK_GCONF_KEY_SPACING              "/apps/gok/layout/key_spacing"
#define GOK_GCONF_KEY_WIDTH                "/apps/gok/layout/key_width"
#define GOK_GCONF_KEY_HEIGHT               "/apps/gok/layout/key_height"
#define GOK_GCONF_KEYBOARD_X               "/apps/gok/layout/keyboard_x"
#define GOK_GCONF_KEYBOARD_Y               "/apps/gok/layout/keyboard_y"
#define GOK_GCONF_ACCESS_METHOD            "/apps/gok/access_method"
#define GOK_GCONF_WORD_COMPLETE            "/apps/gok/word_complete"
#define GOK_GCONF_NUMBER_PREDICTIONS       "/apps/gok/number_predictions"
#define GOK_GCONF_USE_GTKPLUS_THEME        "/apps/gok/use_gtkplus_theme"
#define GOK_GCONF_DRIVE_COREPOINTER        "/apps/gok/drive_corepointer"
#define GOK_GCONF_EXPAND                   "/apps/gok/expand"
#define GOK_GCONF_USE_XKB_KBD              "/apps/gok/use_xkb_geom"
#define GOK_GCONF_COMPOSE_KBD_TYPE         "/apps/gok/compose_kbd_type"
#define GOK_GCONF_CUSTOM_COMPOSE_FILENAME  "/apps/gok/custom_compose_kbd"
#define GOK_GCONF_DOCK_TYPE                "/apps/gok/dock_type"
#define GOK_GCONF_KEYBOARD_DIRECTORY       "/apps/gok/keyboard_directory"
#define GOK_GCONF_AUX_KEYBOARD_DIRECTORY   "/apps/gok/aux_keyboard_directory"
#define GOK_GCONF_ACCESS_METHOD_DIRECTORY  "/apps/gok/access_method_directory"
#define GOK_GCONF_DICTIONARY_DIRECTORY     "/apps/gok/dictionary_directory"
#define GOK_GCONF_PER_USER_DICTIONARY      "/apps/gok/per_user_dictionary"
#define GOK_GCONF_RESOURCE_DIRECTORY       "/apps/gok/resource_directory"
#define GOK_GCONF_PREFS_LOCKED             "/apps/gok/prefs_locked"

#define GOK_GCONF_SPY_SEARCH_DEPTH         "/apps/gok/spy/gui_search_depth"
#define GOK_GCONF_SPY_SEARCH_BREADTH       "/apps/gok/spy/gui_search_breadth"

#define GOK_GCONF_ACCESS_METHOD_SETTINGS   "/apps/gok/access_method_settings"
#define GOK_GCONF_ACCESS_METHOD_SETTINGS_WORKAROUND "/apps/gok/access_method_settings/workaround"

#define GOK_GCONF_ACTIONS                  "/apps/gok/actions"
#define GOK_GCONF_ACTIONS_WORKAROUND       "/apps/gok/actions/workaround"
#define GOK_GCONF_ACTION_DISPLAYNAME       "displayname"
#define GOK_GCONF_ACTION_TYPE              "type"
#define GOK_GCONF_ACTION_STATE             "state"
#define GOK_GCONF_ACTION_NUMBER            "number"
#define GOK_GCONF_ACTION_RATE              "rate"
#define GOK_GCONF_ACTION_PERMANENT         "permanent"
#define GOK_GCONF_ACTION_KEY_AVERAGING     "key_averaging"
#define GOK_GCONF_ACTION_TYPE_SWITCH       "switch"
#define GOK_GCONF_ACTION_TYPE_MOUSEBUTTON  "mousebutton"
#define GOK_GCONF_ACTION_TYPE_MOUSEPOINTER "mousepointer"
#define GOK_GCONF_ACTION_TYPE_DWELL        "dwell"
#define GOK_GCONF_ACTION_TYPE_BUTTON       "button"
#define GOK_GCONF_ACTION_TYPE_KEY          "key"
#define GOK_GCONF_ACTION_STATE_UNDEFINED   "undefined"
#define GOK_GCONF_ACTION_STATE_PRESS       "press"
#define GOK_GCONF_ACTION_STATE_RELEASE     "release"
#define GOK_GCONF_ACTION_STATE_CLICK       "click"
#define GOK_GCONF_ACTION_STATE_DOUBLECLICK "doubleclick"
#define GOK_GCONF_ACTION_STATE_ENTER       "enter"
#define GOK_GCONF_ACTION_STATE_LEAVE       "leave"
#define GOK_GCONF_INPUT_DEVICE             "/apps/gok/input_device"

#define GOK_GCONF_FEEDBACKS                "/apps/gok/feedbacks"
#define GOK_GCONF_FEEDBACKS_WORKAROUND     "/apps/gok/feedbacks/workaround"
#define GOK_GCONF_FEEDBACK_PERMANENT       "permanent"
#define GOK_GCONF_FEEDBACK_DISPLAYNAME     "displayname"
#define GOK_GCONF_FEEDBACK_FLASH           "flash"
#define GOK_GCONF_FEEDBACK_NUMBER_FLASHES  "number_flashes"
#define GOK_GCONF_FEEDBACK_SOUND           "sound"
#define GOK_GCONF_FEEDBACK_SOUNDNAME       "soundname"
#define GOK_GCONF_FEEDBACK_SPEECH          "speech"

#define GOK_GCONF_WORKAROUND_TEXT          "GConf seems to have problems accessing directories who do not have any siblings who are keys.  This key exists to ensure that the subdirectories in this directory do have a sibling who is a key - this key."

#define GOK_GCONF_USE_AUX_DICTS            "/apps/gok/use_aux_dicts"
#define GOK_GCONF_AUX_DICTS                "/apps/gok/aux_dictionaries"

#define GOK_GCONF_VALUATOR_SENSITIVITY     "/apps/gok/valuator_sensitivity"

#define GOK_GCONF_DEFAULT_BROWSER			"/desktop/gnome/url-handlers/http/command"
