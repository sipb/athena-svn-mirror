/* gok-key.h
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

#ifndef __GOKKEY_H__
#define __GOKKEY_H__

#define XK_MISCELLANY
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gnome.h>
#include "gok-spy-priv.h"
#include "gok-output.h"
#include "gok-word-complete.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* general type of key */
typedef enum {
KEYTYPE_NORMAL,
KEYTYPE_MODIFIER,
KEYTYPE_BRANCH,
KEYTYPE_BRANCHBACK,
KEYTYPE_BRANCHMENUS,
KEYTYPE_BRANCHMENUITEMS,
KEYTYPE_MENUITEM,
KEYTYPE_BRANCHTOOLBARS,
KEYTYPE_TOOLBARITEM,
KEYTYPE_BRANCHWINDOWS,
KEYTYPE_RAISEAPPLICATION,
KEYTYPE_BRANCHGUI,
KEYTYPE_BRANCHGUIACTIONS,
KEYTYPE_BRANCHGUISELECTION,
KEYTYPE_BRANCHGUISELECTACTION,
KEYTYPE_PAGESELECTION,
KEYTYPE_BRANCHCOMPOSE,
KEYTYPE_BRANCHEDIT,
KEYTYPE_TEXTNAV,
KEYTYPE_EDIT,
KEYTYPE_SELECT,
KEYTYPE_TOGGLESELECT,
KEYTYPE_BRANCHALPHABET,
KEYTYPE_SETTINGS,
KEYTYPE_SPELL,
KEYTYPE_WORDCOMPLETE,
KEYTYPE_COMMANDPREDICT,
KEYTYPE_WINDOW,
KEYTYPE_MOVERESIZE,
KEYTYPE_POINTERCONTROL,
KEYTYPE_BRANCHHYPERTEXT,
KEYTYPE_BRANCHTEXT,
KEYTYPE_HYPERLINK,
KEYTYPE_VALUATOR,
KEYTYPE_HELP,
KEYTYPE_ABOUT,
KEYTYPE_DOCK,
KEYTYPE_BRANCHMODAL,
KEYTYPE_MOUSE,
KEYTYPE_MOUSEBUTTON,
KEYTYPE_BRANCHLISTITEMS,
KEYTYPE_BRANCHGUITABLE,
KEYTYPE_BRANCHGUIVALUATOR,
KEYTYPE_BRANCHCOMBO,
KEYTYPE_REPEATNEXT,
KEYTYPE_ADDWORD
} GokKeyActionType;

/* display style of key (from the .rc file) */
typedef enum {
KEYSTYLE_NORMAL,
KEYSTYLE_BRANCH,
KEYSTYLE_BRANCHBACK,
KEYSTYLE_GENERALDYNAMIC,
KEYSTYLE_BRANCHMENUS,
KEYSTYLE_BRANCHMENUITEMS,
KEYSTYLE_MENUITEM,
KEYSTYLE_BRANCHTOOLBARS,
KEYSTYLE_TOOLBARITEM,
KEYSTYLE_BRANCHGUI,
KEYSTYLE_BRANCHGUIACTIONS,
KEYSTYLE_PAGESELECTION,
KEYSTYLE_BRANCHCOMPOSE,
KEYSTYLE_TEXTNAV,
KEYSTYLE_EDIT,
KEYSTYLE_SELECT,
KEYSTYLE_TOGGLESELECT,
KEYSTYLE_BRANCHALPHABET,
KEYSTYLE_SETTINGS,
KEYSTYLE_SPELL,
KEYSTYLE_WORDCOMPLETE,
KEYSTYLE_POINTERCONTROL,
KEYSTYLE_BRANCHMODAL,
KEYSTYLE_HYPERLINK,
KEYSTYLE_BRANCHTEXT,
KEYSTYLE_HELP,
KEYSTYLE_ABOUT,
KEYSTYLE_DOCK,
KEYSTYLE_MOUSE,
KEYSTYLE_MOUSEBUTTON,
KEYSTYLE_REPEATNEXT,
KEYSTYLE_ADDWORD,
KEYSTYLE_INSENSITIVE
} KeyStyles;

/* font size groups */
#define FONT_SIZE_GROUP_UNDEFINED 0
#define FONT_SIZE_GROUP_UNIQUE -1
#define FONT_SIZE_GROUP_WORDCOMPLETE -2
#define FONT_SIZE_GROUP_GLYPH -3

#define GOK_MODMASK_CURRENT -1
#define GOK_MIN_FONT_SIZE 6000 


/* a key label */
/* if you add data members to this structure, initialize them in gok_keylabel_new */
typedef struct GokKeyLabel{
	gchar* Text;
        guint  level;
        guint  group;
	gchar* vmods; /* currently unused; see bug # */
	struct GokKeyLabel* pLabelNext;	
} GokKeyLabel;

typedef struct _GokUIState {
	guint active:1;
        guint radio:1;
        guint latched:1;
        guint locked:1;
	guint unused:12;
} GokUIState;

/* POLICY_FILL is used only when there's no text, or with PLACEMENT_BEHIND */
typedef enum {
        IMAGE_TYPE_INDICATOR,
	IMAGE_TYPE_STOCK,
	IMAGE_TYPE_FIXED,
	IMAGE_TYPE_FIT,
	IMAGE_TYPE_FILL 
} GokImageType;

typedef enum {
	IMAGE_PLACEMENT_LEFT,
	IMAGE_PLACEMENT_RIGHT,
	IMAGE_PLACEMENT_TOP,
	IMAGE_PLACEMENT_BOTTOM,
	IMAGE_PLACEMENT_BEHIND /* i.e. label overlaid on image */
} GokImagePlacementPolicy;

/* a key image */
/* if you add data members to this structure, initialize them in gok_keyimage_new */
typedef struct GokKeyImage {
	gchar* Filename;
	GokImageType type;
	gint   stock_size; /* should be GtkIconSize */
	GokImagePlacementPolicy placement_policy; /* ignored for now */
	gint w; /* -1 == unspecified */
	gint h; /* -1 == unspecified */
	struct GokKeyImage* pImageNext;
} GokKeyImage;

/* GokKey structure */
/* If you add data members to this structure, initialize them in gok_key_new */

typedef struct GokKey{
	GokKeyLabel* pLabel;
	GokKeyImage* pImage;
	gchar* Target;
	GokOutput* pOutput;
	GokOutput* pOutputWrapperPre;
	GokOutput* pOutputWrapperPost;
	gchar* ModifierName;
	GtkWidget* pButton;
	GokKeyActionType Type; /* e.g. normal or word completion */
	gint Style; /* display style from the .rc file */
	guint has_text:1;
	guint has_image:1;
	guint is_repeatable:1;
        guint unused:13; /* nicer alignment in case compiler is weird */
	AccessibleNode* accessible_node;
	gint CellsRequired;
	gint FontSizeGroup;
	gint FontSize;
        GokUIState ComponentState;
	gint State; /* highlight state */
	gint StateWhenNotFlashed;

	/* general use pointer */
	void* pGeneral;
		
	/* section */
	gint Section;
  
	/* cell coordinates */
	gint Top;
	gint Bottom;
	gint Left;
	gint Right;

	/* window coordinates */
	gint TopWin;
	gint BottomWin;
	gint LeftWin;
	gint RightWin;
	struct GokKey* pMimicKey;

	struct GokKey* pKeyNext;
	struct GokKey* pKeyPrevious;
} GokKey;

gboolean gok_key_initialize (GokKey* pKey, xmlNode* pNode);
gboolean gok_key_add_label (GokKey* pKey, gchar* pLabelText, guint level, guint group, const gchar *vmods);
void gok_key_set_output (GokKey* pKey, gint Type, gchar* pName, AccessibleKeySynthType Flag);
void gok_key_add_output (GokKey* pKey, gint Type, gchar* pName, AccessibleKeySynthType Flag);
void gok_key_change_label (GokKey* pKey, gchar* LabelText);
void gok_key_update_label (GokKey* pKey);
gint gok_key_get_label_lengthpercell (GokKey* pKey);
gint gok_key_get_label_heightpercell (GokKey* pKey);
gint gok_key_calculate_font_size (GokKey* pKey, gboolean width, gboolean bHeight);
void gok_key_set_font_size (GokKey* pKey, gint Size);
void gok_key_set_button_name (GokKey* pKey);
void gok_key_set_button_label (GokKey* pKey, gchar* LabelText);
gint gok_key_get_default_border_width (GokKey* pKey);
GtkWidget* gok_key_create_image_widget (GokKey* pKey);
GokKeyImage* gok_key_get_image (GokKey* pKey);
gchar* gok_key_get_image_filename (GokKey* pKey);
gchar* gok_key_get_label (GokKey* pKey);
GokKeyLabel* gok_keylabel_new (GokKey* pKey, gchar* pLabelText, guint level, guint group, const gchar *vmods);
void gok_keylabel_delete (GokKeyLabel* pKeyLabel);
void gok_key_update_toggle_state (GokKey *pKey);
void gok_key_set_cells(GokKey* pKey, gint top, gint bottom, gint left, gint right);
GokKey* gok_key_duplicate(GokKey* pKey);
GokKey* gok_feedback_get_key_flashing (void);
gboolean gok_key_make_html_safe (gchar* pString, gchar* pSafeString, gint SafeStringLength);
gboolean gok_key_contains_point (GokKey *pKey, gint x, gint y);
void gok_key_set_effective_group (gint group);
gint gok_key_get_effective_group (void);
int gok_key_get_xkb_type_index (XkbDescPtr xkb, KeyCode keycode, guint group);
int gok_key_level_for_type (Display *display, XkbDescRec *kbd, int type, unsigned int *modmask);
GokKeyImage* gok_keyimage_new (GokKey *pKey, gchar* Filename);
void gok_keyimage_delete (GokKeyImage* pKeyImage);
gboolean gok_key_isRepeatable(GokKey* pKey);
GokOutput* gok_key_wordcomplete_output (GokKey *pKey, GokWordComplete *complete);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKKEY_H__ */
