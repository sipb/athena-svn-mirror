/* gok-keyboard.h
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

#ifndef __GOKKEYBOARD_H__
#define __GOKKEYBOARD_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gnome.h>
#include <libxml/xmlmemory.h>
#include <glib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>
#include "gok-spy.h"
#include "gok-key.h"
#include "gok-chunker.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	
/* keyboards can be assigned a both layout type and a shape type, eventually
	there will be implementations for the various combinations */

/* keyboard layouts */
typedef enum {
KEYBOARD_LAYOUT_NORMAL,
KEYBOARD_LAYOUT_QWERTY,
KEYBOARD_LAYOUT_CENTER,
KEYBOARD_LAYOUT_UPPERL,
KEYBOARD_LAYOUT_UNSPECIFIED
} KeyboardLayouts;

/* keyboard shapes */
typedef enum {
KEYBOARD_SHAPE_BEST,		/* use best shape for layout/access_method */
KEYBOARD_SHAPE_KEYSQUARE,	/* good for switch scanning users */
KEYBOARD_SHAPE_SQUARE,		/* good for pointer users */
KEYBOARD_SHAPE_WIDE,		/* use when docked top or bottom */
KEYBOARD_SHAPE_FITWINDOW,
KEYBOARD_SHAPE_UNSPECIFIED
} KeyboardShape;

/* keyboard types */
typedef enum {
KEYBOARD_TYPE_PLAIN,
KEYBOARD_TYPE_MAIN,
KEYBOARD_TYPE_APPLICATIONS,
KEYBOARD_TYPE_MENUS,
KEYBOARD_TYPE_MENUITEMS,
KEYBOARD_TYPE_ALLTOOLBARS,
KEYBOARD_TYPE_TOOLBAR,
KEYBOARD_TYPE_GUI,
KEYBOARD_TYPE_EDITTEXT,
KEYBOARD_TYPE_WINDOWS,
KEYBOARD_TYPE_UNSPECIFIED,
KEYBOARD_TYPE_MODAL
} KeyboardTypes;

typedef enum {
	GOK_DIRECTION_NONE,
	GOK_DIRECTION_E,
	GOK_DIRECTION_NE,
	GOK_DIRECTION_N,
	GOK_DIRECTION_NW,
	GOK_DIRECTION_W,
	GOK_DIRECTION_SW,
	GOK_DIRECTION_S,
	GOK_DIRECTION_SE,
	GOK_DIRECTION_FILL_EW,
	GOK_RESIZE_NARROWER,
	GOK_RESIZE_WIDER,
	GOK_RESIZE_SHORTER,
	GOK_RESIZE_TALLER
} GokKeyboardDirection; /* TODO: since we're including resize we should rename this enum */

typedef enum {
	GOK_VALUE_UNSPECIFIED,
	GOK_VALUE_LESS,
	GOK_VALUE_MORE,
	GOK_VALUE_MUCH_LESS,
	GOK_VALUE_MUCH_MORE,
	GOK_VALUE_MIN,
	GOK_VALUE_MAX,
	GOK_VALUE_DEFAULT
} GokKeyboardValueOp;

typedef enum {
	GOK_EXPAND_SOMETIMES,
	GOK_EXPAND_NEVER,
	GOK_EXPAND_ALWAYS
} GokKeyboardExpandPolicy;

/* maximum length of a keyboard name */
#define MAX_KEYBOARD_NAME 50

/* GokKeyboard structure */
/* If you add data members to this structure, initialize them in gok_keyboard_new */
typedef struct GokKeyboard {
	gchar Name[MAX_KEYBOARD_NAME + 1];
	KeyboardShape shape;
	KeyboardLayouts LayoutType;
	KeyboardTypes Type;
	gboolean bDynamicallyCreated;
	gint NumberRows;
	gint NumberColumns;
	gboolean bRequiresLayout;
	gboolean bLaidOut;
	gboolean bSupportsWordCompletion;
	gboolean bSupportsCommandPrediction;
	gboolean bWordCompletionKeysAdded;
	gboolean bCommandPredictionKeysAdded;
	GokKeyboardExpandPolicy expand;
	Accessible* pAccessible;
	GokSpySearchType search_type;
	GokSpyUIFlags flags;
	gboolean bFontCalculated;
	struct GokKey* pKeyFirst;
	struct GokKeyboard* pKeyboardNext;
	struct GokKeyboard* pKeyboardPrevious;
	gboolean bRequiresChunking;
	struct GokChunk* pChunkFirst;
} GokKeyboard;

extern int gok_xkb_base_event_type;

GokKeyboard* gok_keyboard_new (void);
GokKeyboard* gok_keyboard_read (const gchar* Filename);
GokKeyboard* gok_keyboard_get_core (void);
GokKeyboard* gok_keyboard_get_alpha (void);
GokKeyboard* gok_keyboard_get_alpha_by_frequency (void);
Display*     gok_keyboard_get_display (void);
void gok_keyboard_delete (GokKeyboard* pKeyboard, gboolean bForce);
void gok_keyboard_delete_key (GokKey* pKey, GokKeyboard* pKeyboard);
void gok_keyboard_count_rows_columns (GokKeyboard* pKeyboard);
gint gok_keyboard_get_number_rows (GokKeyboard* pKeyboard);
gint gok_keyboard_get_number_columns (GokKeyboard* pKeyboard);
gboolean gok_keyboard_add_keys (GokKeyboard* pKeyboard, xmlDoc* pDoc);
gchar* gok_keyboard_get_name (GokKeyboard* pKeyboard);
void gok_keyboard_set_name (GokKeyboard* pKeyboard, gchar* Name);
gboolean gok_keyboard_get_supports_wordcomplete (GokKeyboard* pKeyboard);
gboolean gok_keyboard_get_supports_commandprediction (GokKeyboard* pKeyboard);
void gok_keyboard_set_wordcomplete_keys_added (GokKeyboard* pKeyboard, gboolean bTrueFalse);
gboolean gok_keyboard_get_wordcomplete_keys_added (GokKeyboard* pKeyboard);
void gok_keyboard_set_commandpredict_keys_added (GokKeyboard* pKeyboard, gboolean bTrueFalse);
gboolean gok_keyboard_get_commandpredict_keys_added (GokKeyboard* pKeyboard);
void gok_keyboard_paint_pointer (GokKeyboard *pKeyboard, GtkWidget *pWindowMain, gint x, gint y);
void gok_keyboard_unpaint_pointer (GokKeyboard *pKeyboard, GtkWidget *pWindowMain);
gboolean gok_keyboard_display (GokKeyboard* pKeyboard, GokKeyboard* pKeyboardCurrent, GtkWidget* pWindowMain, gboolean CallbackScanner);
xmlNode* gok_keyboard_find_node (xmlNode* pNode, gchar* NameNode);
void gok_keyboard_position_keys (GokKeyboard* pKeyboard, GtkWidget* pWindow);
void gok_keyboard_initialize (void);
gint gok_keyboard_get_cell_width (GokKeyboard *pKeyboard);
void gok_key_delete (GokKey* pKey, GokKeyboard* pKeyboard, gboolean bDeleteButton);
GokKey* gok_key_new (GokKey* pKeyPrevious, GokKey* pKeyNext, GokKeyboard* pKeyboard);
GokKey* gok_key_from_xkb_key (GokKey* prevKey, GokKeyboard *pKeyboard, Display *display, XkbGeometryPtr pGeom, XkbRowPtr pRow, XkbSectionPtr pXkbSection, XkbKeyPtr keyp, gint section, gint row, gint col);
XkbDescPtr gok_keyboard_get_xkb_desc (void);
gboolean gok_keyboard_xkb_select (Display *display);
void gok_keyboard_notify_keys_changed (void);
void gok_keyboard_notify_xkb_event (XkbEvent* event);
gboolean gok_keyboard_layout (GokKeyboard* pKeyboard, KeyboardLayouts layout, KeyboardShape shape, gboolean force);
gboolean gok_keyboard_branch_byKey (GokKey* pKey);
gboolean gok_keyboard_branch_gui (AccessibleNode* pNodeAccessible, GokSpySearchType type);
gboolean gok_keyboard_branch_gui_actions (AccessibleNode* pNodeAccessible);
gboolean gok_keyboard_branch_edittext (void);
gboolean gok_chunker_chunk (GokKeyboard* pKeyboard);
gboolean gok_chunker_chunk_rows_ttb (GokKeyboard* pKeyboard, gint ChunkOrder);
gboolean gok_chunker_chunk_rows_btt (GokKeyboard* pKeyboard, gint ChunkOrder);
gboolean gok_chunker_chunk_cols_ltr (GokKeyboard* pKeyboard, gint ChunkOrder);
gboolean gok_chunker_chunk_cols_rtl (GokKeyboard* pKeyboard, gint ChunkOrder);
gboolean gok_chunker_chunk_recursive (GokKeyboard* pKeyboard, gint ChunkOrder, gint Groups);
GokKey* gok_chunker_find_center (GokKeyboard* pKeyboard, gint centerRow, gint centerColumn, gint* pRowsDistant, gint* pColumnsDistant);
GokKey* gok_keyboard_output_selectedkey (void);
GokKey* gok_keyboard_output_key(GokKey* pKeySelected);
gboolean gok_keyboard_validate_dynamic_keys (Accessible* pAccessibleForeground);
void gok_keyboard_fill_row (GokKeyboard* pKeyboard, gint RowNumber);
void gok_keyboard_insert_array (GokKey* pKey);
void gok_keyboard_on_window_resize (void);
gint gok_keyboard_get_keywidth_for_window (gint WidthWindow, GokKeyboard* pKeyboard);
gint gok_keyboard_get_keyheight_for_window (gint HeightWindow, GokKeyboard* pKeyboard);
void gok_keyboard_set_ignore_resize (gboolean bFlag);
Accessible* gok_keyboard_get_accessible (GokKeyboard* pKeyboard);
void gok_keyboard_set_accessible (GokKeyboard* pKeyboard, Accessible* pAccessible);
void gok_keyboard_calculate_font_size (GokKeyboard* pKeyboard);
void gok_keyboard_calculate_font_size_group (GokKeyboard* pKeyboard, gint GroupNumber, gboolean bOverride);
gboolean gok_keyboard_update_dynamic (GokKeyboard* pKeyboard);
void gok_keyboard_update_labels (void);
gint gok_keyboard_get_keyboards(void);
GokKey *gok_keyboard_find_key_at (GokKeyboard *pKeyboard, gint x, gint y, GokKey *prev);
GokKeyboardDirection gok_keyboard_parse_direction (const gchar *string);
gint gok_keyboard_add_predictions (GokKeyboard *pKeyboard, gchar **list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKKEYBOARD_H__ */
