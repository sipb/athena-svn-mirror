
/* Generated data (by glib-mkenums) */

#include "libgnomeui.h"

#include "gnometypebuiltins.h"



/* enumerations from "gnome-app-helper.h" */
static const GEnumValue _gnome_ui_info_type_values[] = {
  { GNOME_APP_UI_ENDOFINFO, "GNOME_APP_UI_ENDOFINFO", "endofinfo" },
  { GNOME_APP_UI_ITEM, "GNOME_APP_UI_ITEM", "item" },
  { GNOME_APP_UI_TOGGLEITEM, "GNOME_APP_UI_TOGGLEITEM", "toggleitem" },
  { GNOME_APP_UI_RADIOITEMS, "GNOME_APP_UI_RADIOITEMS", "radioitems" },
  { GNOME_APP_UI_SUBTREE, "GNOME_APP_UI_SUBTREE", "subtree" },
  { GNOME_APP_UI_SEPARATOR, "GNOME_APP_UI_SEPARATOR", "separator" },
  { GNOME_APP_UI_HELP, "GNOME_APP_UI_HELP", "help" },
  { GNOME_APP_UI_BUILDER_DATA, "GNOME_APP_UI_BUILDER_DATA", "builder-data" },
  { GNOME_APP_UI_ITEM_CONFIGURABLE, "GNOME_APP_UI_ITEM_CONFIGURABLE", "item-configurable" },
  { GNOME_APP_UI_SUBTREE_STOCK, "GNOME_APP_UI_SUBTREE_STOCK", "subtree-stock" },
  { GNOME_APP_UI_INCLUDE, "GNOME_APP_UI_INCLUDE", "include" },
  { 0, NULL, NULL }
};

GType
gnome_ui_info_type_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeUIInfoType", _gnome_ui_info_type_values);

  return type;
}


static const GEnumValue _gnome_ui_info_configurable_types_values[] = {
  { GNOME_APP_CONFIGURABLE_ITEM_NEW, "GNOME_APP_CONFIGURABLE_ITEM_NEW", "new" },
  { GNOME_APP_CONFIGURABLE_ITEM_OPEN, "GNOME_APP_CONFIGURABLE_ITEM_OPEN", "open" },
  { GNOME_APP_CONFIGURABLE_ITEM_SAVE, "GNOME_APP_CONFIGURABLE_ITEM_SAVE", "save" },
  { GNOME_APP_CONFIGURABLE_ITEM_SAVE_AS, "GNOME_APP_CONFIGURABLE_ITEM_SAVE_AS", "save-as" },
  { GNOME_APP_CONFIGURABLE_ITEM_REVERT, "GNOME_APP_CONFIGURABLE_ITEM_REVERT", "revert" },
  { GNOME_APP_CONFIGURABLE_ITEM_PRINT, "GNOME_APP_CONFIGURABLE_ITEM_PRINT", "print" },
  { GNOME_APP_CONFIGURABLE_ITEM_PRINT_SETUP, "GNOME_APP_CONFIGURABLE_ITEM_PRINT_SETUP", "print-setup" },
  { GNOME_APP_CONFIGURABLE_ITEM_CLOSE, "GNOME_APP_CONFIGURABLE_ITEM_CLOSE", "close" },
  { GNOME_APP_CONFIGURABLE_ITEM_QUIT, "GNOME_APP_CONFIGURABLE_ITEM_QUIT", "quit" },
  { GNOME_APP_CONFIGURABLE_ITEM_CUT, "GNOME_APP_CONFIGURABLE_ITEM_CUT", "cut" },
  { GNOME_APP_CONFIGURABLE_ITEM_COPY, "GNOME_APP_CONFIGURABLE_ITEM_COPY", "copy" },
  { GNOME_APP_CONFIGURABLE_ITEM_PASTE, "GNOME_APP_CONFIGURABLE_ITEM_PASTE", "paste" },
  { GNOME_APP_CONFIGURABLE_ITEM_CLEAR, "GNOME_APP_CONFIGURABLE_ITEM_CLEAR", "clear" },
  { GNOME_APP_CONFIGURABLE_ITEM_UNDO, "GNOME_APP_CONFIGURABLE_ITEM_UNDO", "undo" },
  { GNOME_APP_CONFIGURABLE_ITEM_REDO, "GNOME_APP_CONFIGURABLE_ITEM_REDO", "redo" },
  { GNOME_APP_CONFIGURABLE_ITEM_FIND, "GNOME_APP_CONFIGURABLE_ITEM_FIND", "find" },
  { GNOME_APP_CONFIGURABLE_ITEM_FIND_AGAIN, "GNOME_APP_CONFIGURABLE_ITEM_FIND_AGAIN", "find-again" },
  { GNOME_APP_CONFIGURABLE_ITEM_REPLACE, "GNOME_APP_CONFIGURABLE_ITEM_REPLACE", "replace" },
  { GNOME_APP_CONFIGURABLE_ITEM_PROPERTIES, "GNOME_APP_CONFIGURABLE_ITEM_PROPERTIES", "properties" },
  { GNOME_APP_CONFIGURABLE_ITEM_PREFERENCES, "GNOME_APP_CONFIGURABLE_ITEM_PREFERENCES", "preferences" },
  { GNOME_APP_CONFIGURABLE_ITEM_ABOUT, "GNOME_APP_CONFIGURABLE_ITEM_ABOUT", "about" },
  { GNOME_APP_CONFIGURABLE_ITEM_SELECT_ALL, "GNOME_APP_CONFIGURABLE_ITEM_SELECT_ALL", "select-all" },
  { GNOME_APP_CONFIGURABLE_ITEM_NEW_WINDOW, "GNOME_APP_CONFIGURABLE_ITEM_NEW_WINDOW", "new-window" },
  { GNOME_APP_CONFIGURABLE_ITEM_CLOSE_WINDOW, "GNOME_APP_CONFIGURABLE_ITEM_CLOSE_WINDOW", "close-window" },
  { GNOME_APP_CONFIGURABLE_ITEM_NEW_GAME, "GNOME_APP_CONFIGURABLE_ITEM_NEW_GAME", "new-game" },
  { GNOME_APP_CONFIGURABLE_ITEM_PAUSE_GAME, "GNOME_APP_CONFIGURABLE_ITEM_PAUSE_GAME", "pause-game" },
  { GNOME_APP_CONFIGURABLE_ITEM_RESTART_GAME, "GNOME_APP_CONFIGURABLE_ITEM_RESTART_GAME", "restart-game" },
  { GNOME_APP_CONFIGURABLE_ITEM_UNDO_MOVE, "GNOME_APP_CONFIGURABLE_ITEM_UNDO_MOVE", "undo-move" },
  { GNOME_APP_CONFIGURABLE_ITEM_REDO_MOVE, "GNOME_APP_CONFIGURABLE_ITEM_REDO_MOVE", "redo-move" },
  { GNOME_APP_CONFIGURABLE_ITEM_HINT, "GNOME_APP_CONFIGURABLE_ITEM_HINT", "hint" },
  { GNOME_APP_CONFIGURABLE_ITEM_SCORES, "GNOME_APP_CONFIGURABLE_ITEM_SCORES", "scores" },
  { GNOME_APP_CONFIGURABLE_ITEM_END_GAME, "GNOME_APP_CONFIGURABLE_ITEM_END_GAME", "end-game" },
  { 0, NULL, NULL }
};

GType
gnome_ui_info_configurable_types_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeUIInfoConfigurableTypes", _gnome_ui_info_configurable_types_values);

  return type;
}


static const GEnumValue _gnome_ui_pixmap_type_values[] = {
  { GNOME_APP_PIXMAP_NONE, "GNOME_APP_PIXMAP_NONE", "none" },
  { GNOME_APP_PIXMAP_STOCK, "GNOME_APP_PIXMAP_STOCK", "stock" },
  { GNOME_APP_PIXMAP_DATA, "GNOME_APP_PIXMAP_DATA", "data" },
  { GNOME_APP_PIXMAP_FILENAME, "GNOME_APP_PIXMAP_FILENAME", "filename" },
  { 0, NULL, NULL }
};

GType
gnome_ui_pixmap_type_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeUIPixmapType", _gnome_ui_pixmap_type_values);

  return type;
}



/* enumerations from "gnome-client.h" */
static const GEnumValue _gnome_interact_style_values[] = {
  { GNOME_INTERACT_NONE, "GNOME_INTERACT_NONE", "none" },
  { GNOME_INTERACT_ERRORS, "GNOME_INTERACT_ERRORS", "errors" },
  { GNOME_INTERACT_ANY, "GNOME_INTERACT_ANY", "any" },
  { 0, NULL, NULL }
};

GType
gnome_interact_style_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeInteractStyle", _gnome_interact_style_values);

  return type;
}


static const GEnumValue _gnome_dialog_type_values[] = {
  { GNOME_DIALOG_ERROR, "GNOME_DIALOG_ERROR", "error" },
  { GNOME_DIALOG_NORMAL, "GNOME_DIALOG_NORMAL", "normal" },
  { 0, NULL, NULL }
};

GType
gnome_dialog_type_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeDialogType", _gnome_dialog_type_values);

  return type;
}


static const GEnumValue _gnome_save_style_values[] = {
  { GNOME_SAVE_GLOBAL, "GNOME_SAVE_GLOBAL", "global" },
  { GNOME_SAVE_LOCAL, "GNOME_SAVE_LOCAL", "local" },
  { GNOME_SAVE_BOTH, "GNOME_SAVE_BOTH", "both" },
  { 0, NULL, NULL }
};

GType
gnome_save_style_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeSaveStyle", _gnome_save_style_values);

  return type;
}


static const GEnumValue _gnome_restart_style_values[] = {
  { GNOME_RESTART_IF_RUNNING, "GNOME_RESTART_IF_RUNNING", "if-running" },
  { GNOME_RESTART_ANYWAY, "GNOME_RESTART_ANYWAY", "anyway" },
  { GNOME_RESTART_IMMEDIATELY, "GNOME_RESTART_IMMEDIATELY", "immediately" },
  { GNOME_RESTART_NEVER, "GNOME_RESTART_NEVER", "never" },
  { 0, NULL, NULL }
};

GType
gnome_restart_style_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeRestartStyle", _gnome_restart_style_values);

  return type;
}


static const GEnumValue _gnome_client_state_values[] = {
  { GNOME_CLIENT_IDLE, "GNOME_CLIENT_IDLE", "idle" },
  { GNOME_CLIENT_SAVING_PHASE_1, "GNOME_CLIENT_SAVING_PHASE_1", "saving-phase-1" },
  { GNOME_CLIENT_WAITING_FOR_PHASE_2, "GNOME_CLIENT_WAITING_FOR_PHASE_2", "waiting-for-phase-2" },
  { GNOME_CLIENT_SAVING_PHASE_2, "GNOME_CLIENT_SAVING_PHASE_2", "saving-phase-2" },
  { GNOME_CLIENT_FROZEN, "GNOME_CLIENT_FROZEN", "frozen" },
  { GNOME_CLIENT_DISCONNECTED, "GNOME_CLIENT_DISCONNECTED", "disconnected" },
  { GNOME_CLIENT_REGISTERING, "GNOME_CLIENT_REGISTERING", "registering" },
  { 0, NULL, NULL }
};

GType
gnome_client_state_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeClientState", _gnome_client_state_values);

  return type;
}


static const GFlagsValue _gnome_client_flags_values[] = {
  { GNOME_CLIENT_IS_CONNECTED, "GNOME_CLIENT_IS_CONNECTED", "is-connected" },
  { GNOME_CLIENT_RESTARTED, "GNOME_CLIENT_RESTARTED", "restarted" },
  { GNOME_CLIENT_RESTORED, "GNOME_CLIENT_RESTORED", "restored" },
  { 0, NULL, NULL }
};

GType
gnome_client_flags_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_flags_register_static ("GnomeClientFlags", _gnome_client_flags_values);

  return type;
}



/* enumerations from "gnome-dateedit.h" */
static const GFlagsValue _gnome_date_edit_flags_values[] = {
  { GNOME_DATE_EDIT_SHOW_TIME, "GNOME_DATE_EDIT_SHOW_TIME", "show-time" },
  { GNOME_DATE_EDIT_24_HR, "GNOME_DATE_EDIT_24_HR", "24-hr" },
  { GNOME_DATE_EDIT_WEEK_STARTS_ON_MONDAY, "GNOME_DATE_EDIT_WEEK_STARTS_ON_MONDAY", "week-starts-on-monday" },
  { 0, NULL, NULL }
};

GType
gnome_date_edit_flags_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_flags_register_static ("GnomeDateEditFlags", _gnome_date_edit_flags_values);

  return type;
}



/* enumerations from "gnome-druid-page-edge.h" */
static const GEnumValue _gnome_edge_position_values[] = {
  { GNOME_EDGE_START, "GNOME_EDGE_START", "start" },
  { GNOME_EDGE_FINISH, "GNOME_EDGE_FINISH", "finish" },
  { GNOME_EDGE_OTHER, "GNOME_EDGE_OTHER", "other" },
  { GNOME_EDGE_LAST, "GNOME_EDGE_LAST", "last" },
  { 0, NULL, NULL }
};

GType
gnome_edge_position_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeEdgePosition", _gnome_edge_position_values);

  return type;
}



/* enumerations from "gnome-font-picker.h" */
static const GEnumValue _gnome_font_picker_mode_values[] = {
  { GNOME_FONT_PICKER_MODE_PIXMAP, "GNOME_FONT_PICKER_MODE_PIXMAP", "pixmap" },
  { GNOME_FONT_PICKER_MODE_FONT_INFO, "GNOME_FONT_PICKER_MODE_FONT_INFO", "font-info" },
  { GNOME_FONT_PICKER_MODE_USER_WIDGET, "GNOME_FONT_PICKER_MODE_USER_WIDGET", "user-widget" },
  { GNOME_FONT_PICKER_MODE_UNKNOWN, "GNOME_FONT_PICKER_MODE_UNKNOWN", "unknown" },
  { 0, NULL, NULL }
};

GType
gnome_font_picker_mode_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeFontPickerMode", _gnome_font_picker_mode_values);

  return type;
}



/* enumerations from "gnome-icon-list.h" */
static const GEnumValue _gnome_icon_list_mode_values[] = {
  { GNOME_ICON_LIST_ICONS, "GNOME_ICON_LIST_ICONS", "icons" },
  { GNOME_ICON_LIST_TEXT_BELOW, "GNOME_ICON_LIST_TEXT_BELOW", "text-below" },
  { GNOME_ICON_LIST_TEXT_RIGHT, "GNOME_ICON_LIST_TEXT_RIGHT", "text-right" },
  { 0, NULL, NULL }
};

GType
gnome_icon_list_mode_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeIconListMode", _gnome_icon_list_mode_values);

  return type;
}



/* enumerations from "gnome-mdi.h" */
static const GEnumValue _gnome_mdi_mode_values[] = {
  { GNOME_MDI_NOTEBOOK, "GNOME_MDI_NOTEBOOK", "notebook" },
  { GNOME_MDI_TOPLEVEL, "GNOME_MDI_TOPLEVEL", "toplevel" },
  { GNOME_MDI_MODAL, "GNOME_MDI_MODAL", "modal" },
  { GNOME_MDI_DEFAULT_MODE, "GNOME_MDI_DEFAULT_MODE", "default-mode" },
  { 0, NULL, NULL }
};

GType
gnome_mdi_mode_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomeMDIMode", _gnome_mdi_mode_values);

  return type;
}



/* enumerations from "gnome-types.h" */
static const GEnumValue _gnome_preferences_type_values[] = {
  { GNOME_PREFERENCES_NEVER, "GNOME_PREFERENCES_NEVER", "never" },
  { GNOME_PREFERENCES_USER, "GNOME_PREFERENCES_USER", "user" },
  { GNOME_PREFERENCES_ALWAYS, "GNOME_PREFERENCES_ALWAYS", "always" },
  { 0, NULL, NULL }
};

GType
gnome_preferences_type_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GnomePreferencesType", _gnome_preferences_type_values);

  return type;
}



/* Generated data ends here */

