/* gok-keyboard.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define XK_LATIN1
#include <X11/keysymdef.h> /* ugh, needed by link following hack */
#include <assert.h>
#include <gdk/gdk.h>
#include <math.h>
#include "gok-keyboard.h"
#include "gok-mousecontrol.h"
#include "callbacks.h"
#include "gok-branchback-stack.h"
#include "gok-data.h"
#include "main.h"
#include "gok-button.h"
#include "gok-spy-priv.h"
#include "gok-spy.h"
#include "gok-sound.h"
#include "gok-word-complete.h"
#include "gok-log.h"
#include "gok-modifier.h"
#include "gok-output.h"
#include "gok-predictor.h"
#include "gok-composer.h"
#include "gok-windowlister.h"
#include "gok-settings-dialog.h"
#include "gok-page-keysizespace.h"
#include "gok-feedback.h"
#include "gok-repeat.h"

#define NUM_INTERESTING_ROLES 5
#define GOK_KEYBOARD_MAX_ROWS 24
#define MAX_BREADTH_ACTION_LEAF_SEARCH 20

#define BUG_133323_LIVES

/* TODO: i18n */
#define CORE_KEYBOARD "Keyboard"

#define ALPHA_KEYBOARD "Alpha"

#if (!defined CSPI_1_5 || (CSPI_1_5 == 0))
/* missing from cspi headers in 1.4 series, but present in libcspi */
SPIBoolean
AccessibleTable_addRowSelection (AccessibleTable *obj,
				 long int row);
SPIBoolean
AccessibleTable_addColumnSelection (AccessibleTable *obj,
				    long int column);

SPIBoolean
AccessibleTable_removeRowSelection (AccessibleTable *obj,
				    long int row);
SPIBoolean
AccessibleTable_removeColumnSelection (AccessibleTable *obj,
				       long int column);
#endif

#define FREQ_KEYBOARD "Alpha-Frequency"

#define GOK_KEYBOARD_DOCK_BORDERPIX 5

int gok_xkb_base_event_type;

/* if this flag is set then ignore the resize event (because we generated it) */
static gint m_bIgnoreResizeEvent = TRUE;

/* keeps track of the key size so we know when to resize the font */
static gint m_WidthKeyFont = -1;
static gint m_HeightKeyFont = -1;

/* storage for the last known GOK pointer location. */
static gint m_oldPointerX;
static gint m_oldPointerY;

/* keep track of number of keyboards - for debugging */
static gint m_NumberOfKeyboards = 0;

/* the XKB keyboard description structure and display */
static XkbDescPtr m_XkbDescPtr = NULL;
static Display   *m_XkbDisplay = NULL;
static int       *m_RowLastColumn;
static int       *m_RowTop;
static int       *m_SectionRowStart;
static int       *m_SectionCols;
static int       *m_SectionColStart;
static int        m_MinSectionTop = G_MAXINT;
static int        m_MinSectionLeft = G_MAXINT;

static GokKeyboard *_core_compose_keyboard;

/* private prototypes */
static gboolean gok_keyboard_focus_object (Accessible *accessible);
static gboolean gok_keyboard_branch_gui_selectaction (GokKeyboard *keyboard, AccessibleNode *node, gint action_ndx);
static gboolean gok_keyboard_branch_gui_valuator (AccessibleNode* node);
static gboolean gok_keyboard_do_leaf_action (Accessible *parent);
static gboolean gok_keyboard_branch_or_invoke_actions (GokKeyboard *keyboard, AccessibleNode *node, gint action_ndx);
static GokKeyboard* gok_keyboard_get_compose (void);

/**
* gok_keyboard_initialize
*
* Initializes global data for all keyboards.
* Call this once at the beginning of the program.
*
* returns: void
**/
void gok_keyboard_initialize ()
{
	m_WidthKeyFont = -1;
	m_HeightKeyFont = -1;
	m_bIgnoreResizeEvent = TRUE;
}

/**
 * gok_keyboard_get_xkb_desc: 
 *
 * Returns: a pointer to the XkbDesc structure for the core keyboard.
 **/
XkbDescPtr 
gok_keyboard_get_xkb_desc (void)
{
	if (m_XkbDescPtr == NULL) {
		int ir, reason_return;
		char *display_name = getenv ("DISPLAY");
		m_XkbDisplay = XkbOpenDisplay (display_name,
					       &gok_xkb_base_event_type,
					       &ir, NULL, NULL, 
					       &reason_return);
		if (m_XkbDisplay == NULL)
		        g_warning (_("Xkb extension could not be initialized! (error code %x)"), reason_return);
		else 
			m_XkbDescPtr = XkbGetMap (m_XkbDisplay, 
						  XkbAllComponentsMask,
						  XkbUseCoreKbd);
		if (m_XkbDescPtr == NULL)
		        g_warning (_("keyboard description not available!"));
		else {
			int status = XkbGetGeometry (m_XkbDisplay, m_XkbDescPtr);

			if (status != Success)
			{
				g_warning (_("Keyboard Geometry cannot be read from your X Server."));
			}	
			XkbGetNames (m_XkbDisplay, XkbAllNamesMask, m_XkbDescPtr);
		}
	}
	return m_XkbDescPtr;
}

gboolean
gok_keyboard_xkb_select (Display *display)
{
	int opcode_rtn, error_rtn;
	gboolean retval;
	retval = XkbQueryExtension (display, &opcode_rtn, &gok_xkb_base_event_type, 
			   &error_rtn, NULL, NULL);
	if (retval) 
               retval = XkbSelectEvents (display, XkbUseCoreKbd, XkbStateNotifyMask | XkbAccessXNotifyMask, XkbStateNotifyMask | XkbAccessXNotifyMask | XkbMapNotifyMask);
	return retval;
}

void
gok_keyboard_notify_keys_changed (void)
{
    if (gok_data_get_use_xkb_kbd ())
    {
	GokKeyboard *compose_kbd = gok_main_keyboard_find_byname ("Keyboard");
	GokKeyboard *prev, *next;

	if (m_XkbDisplay)
	{
	    m_XkbDescPtr = XkbGetMap (m_XkbDisplay, 
				      XkbAllComponentsMask,
				      XkbUseCoreKbd);
	    if (m_XkbDescPtr == NULL)
		g_warning (_("keyboard description not available!"));
	    else {
		int status = XkbGetGeometry (m_XkbDisplay, m_XkbDescPtr);
		
		if (status != Success)
		{
		    g_warning (_("Keyboard Geometry cannot be read from your X Server."));
		}	
		XkbGetNames (m_XkbDisplay, XkbAllNamesMask, m_XkbDescPtr);
	    }
	}
	if (compose_kbd) 
	{
	    GokKeyboard *tmp = compose_kbd;
	    prev = compose_kbd->pKeyboardPrevious;
	    next = compose_kbd->pKeyboardNext;
	    gok_log ("recreating core keyboard");
	    compose_kbd = gok_keyboard_get_core ();
	    compose_kbd->pKeyboardPrevious = prev;
	    compose_kbd->pKeyboardNext = next;
	    if (prev) prev->pKeyboardNext = compose_kbd;
	    if (next) next->pKeyboardPrevious = compose_kbd;
	    gok_main_set_first_keyboard (compose_kbd);
	    if (gok_main_get_current_keyboard () == tmp) 
	    {
		gok_main_display_scan_previous ();
		gok_main_display_scan (gok_keyboard_get_compose (), "Keyboard",
				       KEYBOARD_TYPE_UNSPECIFIED, 
				       KEYBOARD_LAYOUT_UNSPECIFIED,
				       KEYBOARD_SHAPE_UNSPECIFIED);
	    }
	    gok_keyboard_delete (tmp, TRUE);
	}
    }
}

void
gok_keyboard_notify_xkb_event (XkbEvent *event)
{
	if (event->any.xkb_type == XkbStateNotify) {
		XkbStateNotifyEvent *sevent = &event->state;
		if (sevent->changed & XkbGroupStateMask) {
			gok_key_set_effective_group (sevent->group);
		}
		else
		{
		    gok_log ("XKB event changed:%x\n", sevent->changed);
		}
	} 
#ifdef USE_XKB_MAPPING_EVENTS
	else if (event->any.xkb_type == XkbMapNotify || event->any.type == MappingNotify) 
	{
		g_warning ("XKB Map Notify changed:%x", event->map.changed);
		XkbGetUpdatedMap (m_XkbDisplay, event->map.changed, m_XkbDescPtr);
		/* XkbRefreshKeyboardMapping (event); */
		/* rebuild GOK's 'Compose' keyboard */
		if (gok_data_get_use_xkb_kbd () && event->map.changed & XkbKeySymsMask)
		{
			GokKeyboard *compose_kbd = gok_keyboard_get_compose ();
			GokKeyboard *prev, *next;
			if (compose_kbd) 
			{
				GokKeyboard *tmp = compose_kbd;
				prev = compose_kbd->pKeyboardPrevious;
				next = compose_kbd->pKeyboardNext;
				g_warning ("recreating core keyboard");
				compose_kbd = gok_keyboard_get_core ();
				compose_kbd->pKeyboardPrevious = prev;
				compose_kbd->pKeyboardNext = next;
				if (prev) prev->pKeyboardNext = compose_kbd;
				if (next) next->pKeyboardPrevious = compose_kbd;
				gok_main_set_first_keyboard (compose_kbd);
				gok_keyboard_delete (tmp, TRUE);
			}
		}
	}
#endif
	/* 
	 * TODO: change GOK's shift state notification 
	 * to use XKB instead of at-spi ?
	 */
}

static int
gok_keyboard_get_section_row (gint i)
{
	return m_SectionRowStart[i];
}

static gint
gok_keyboard_section_row_columns (XkbGeometryPtr pGeom, XkbRowPtr rowp)
{
	int i, ncols = 0;
	for (i = 0; i < rowp->num_keys; i++) {
		XkbBoundsRec *pBounds;
		pBounds = &pGeom->shapes[rowp->keys[i].shape_ndx].bounds;
		ncols += MAX (1, (pBounds->x2 - pBounds->x1)/
			      (pBounds->y2 - pBounds->y1));
	}
	return ncols;
}

static gint
gok_keyboard_get_section_column (gint i)
{
	int retval;

	retval = m_SectionColStart[i];
	return retval;
}

static void
gok_keyboard_xkb_geom_sections_init (XkbSectionPtr sections, int n_sections)
{
        int i, j, num_rows = 0;
	float avg_row_height = 0;
	int min_col_width = G_MAXINT;
	gboolean use_column = TRUE;

	m_RowLastColumn = g_malloc (sizeof (int) * GOK_KEYBOARD_MAX_ROWS);
	m_RowTop = g_malloc (sizeof (int) * GOK_KEYBOARD_MAX_ROWS);
	m_SectionRowStart = g_malloc (sizeof (int) * n_sections);
	m_SectionColStart = g_malloc (sizeof (int) * n_sections);
	m_SectionCols = g_malloc (sizeof (int) * n_sections);

	for (i = 0; i < GOK_KEYBOARD_MAX_ROWS; i++) {
		m_RowLastColumn[i] = 0;
		m_RowTop[i] = G_MININT;
	} 

	m_RowLastColumn[0] = 1;

	for (i = 0; i < n_sections; i++) {
		m_MinSectionTop = MIN (m_MinSectionTop, sections[i].top);
		m_MinSectionLeft = MIN (m_MinSectionLeft, sections[i].left);
		avg_row_height += sections[i].height;
		num_rows += sections[i].num_rows;
		m_SectionCols[i] = 0;
		for (j = 0; j < sections[i].num_rows; j++ ) {
			min_col_width = MIN ((sections[i].width)/
					     sections[i].rows[j].num_keys, 
					     min_col_width);
			m_SectionCols[i] = MAX (m_SectionCols[i], 
						floor (sections[i].rows[j].left/min_col_width)/
						sections[i].rows[j].num_keys);
		}
	}

	avg_row_height /= num_rows;

	for (i = 0; i < n_sections; i++) {
		m_SectionRowStart[i] = floor ((sections[i].top - m_MinSectionTop)/avg_row_height);
		m_SectionColStart[i] = floor ((sections[i].left - m_MinSectionLeft)/min_col_width);
	}

	/* now, do one more pass to remove gaps between sections */
	for (i = 0; i < n_sections; i++) 
	{
	    if (!use_column && (m_SectionRowStart[i] == 0) && (m_SectionColStart[i] == 0)) 
	    {
		m_SectionColStart[i] = 1; /* leave room for 'Back' key */
	    }
	    /* and move other sections aside, if need be */
	    if (sections[i].rows[0].num_keys == m_SectionCols[i]) { 
		int n;
		for (n = 0; n < n_sections; n++) {
		    int startRow = m_SectionRowStart[n];
		    int endRow = startRow + sections[n].num_rows;
		    if ((n != i) &&
			((startRow >= m_SectionRowStart[i]) &&
			 (startRow < m_SectionRowStart[i] 
			  + sections[i].num_rows)) ||
			((endRow >= m_SectionRowStart[i]) &&
			 (endRow < m_SectionRowStart[i]
			  + sections[i].num_rows))) {
			if (m_SectionRowStart[n] > m_SectionRowStart[i]) {
			    ++m_SectionRowStart[n];
			}
		    }
		}
	    }
	} 
}

static void
gok_keyboard_add_compose_aux_keys (GokKeyboard *keyboard)
{
    GokKey *pKey, *pKeyLast = keyboard->pKeyFirst;
    gint row, firstrow, lastrow;
    gint col;
    gboolean use_column = TRUE;

    g_assert (keyboard);
    
    /* seek to end */
    while (pKeyLast) 
    {
	if (use_column)
	{
	    /* move the whole keyboard over one column */
	    pKeyLast->Left += 1;
	    pKeyLast->Right += 1;
	}
	if (pKeyLast->pKeyNext) pKeyLast = pKeyLast->pKeyNext;
	else break;
    }

    firstrow = 0;
    if (use_column) {
	row = firstrow + 1; /* top available row is used by 'back' */
	keyboard->NumberColumns++;
    }
    else
    {
	keyboard->NumberRows++;
	row = keyboard->NumberRows;
    }
    lastrow = keyboard->NumberRows;

    /* back key must be first, for keyboards which will be laid out again */
    pKey = gok_key_new (NULL, keyboard->pKeyFirst, keyboard);
    pKey->Type = KEYTYPE_BRANCHBACK;
    pKey->Style = KEYSTYLE_BRANCHBACK;
    pKey->FontSize = -1;
    pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
    pKey->Top = 0;
    pKey->Bottom = 1;
    pKey->Left = 0;
    pKey->Right = 1;
    gok_key_add_label (pKey, _("Back"), 0, 0, NULL);
    if (keyboard->pKeyFirst) keyboard->pKeyFirst = pKey;

    /* add a repeat next key */
    pKey = gok_key_new (pKeyLast, NULL, keyboard);
    pKey->FontSizeGroup = use_column ? FONT_SIZE_GROUP_GLYPH : FONT_SIZE_GROUP_UNIQUE;
    pKey->Style = KEYSTYLE_REPEATNEXT;
    pKey->Type = KEYTYPE_REPEATNEXT;
    pKey->has_image = TRUE;
    pKey->pImage = gok_keyimage_new (pKey, NULL);
    pKey->pImage->type = IMAGE_TYPE_INDICATOR;
    pKey->ComponentState.active = FALSE;
    pKey->Target = (gchar*)g_malloc (strlen("repeat-next") + 1);
    strcpy (pKey->Target, "repeat-next");
    pKey->Top = row;
    pKey->Bottom = row + 1;
    pKey->Left = 0;
    pKey->Right = use_column ? 1 : 2;
    gok_key_add_label (pKey, (use_column ? "\342\206\272" : _("Repeat Next")), 0, 0, NULL);
    
    if (use_column) row++;
    /* add a branch key for special text edit functions */
    pKey = gok_key_new (pKey, NULL, keyboard);
    pKey->Style = KEYSTYLE_GENERALDYNAMIC;
    pKey->Type = KEYTYPE_BRANCHEDIT;
    pKey->Target = (gchar*)g_malloc (strlen("text-operations") + 1);
    strcpy (pKey->Target, "text-operations");
    pKey->Top = row;
    pKey->Bottom = row + 1;
    pKey->Left = use_column ? 0 : 2;
    pKey->Right = use_column ? 1 : keyboard->NumberColumns - 2;
    gok_key_add_label (pKey, _("Edit"), 0, 0, NULL);
    
    if (use_column) row++;
    /* add a branch key for the "numberpad" keyboard  */
    pKey = gok_key_new (pKey, NULL, keyboard);
    pKey->Style = KEYSTYLE_BRANCH;
    pKey->Type = KEYTYPE_BRANCH;
    pKey->Target = g_strdup ("numberpad");
    pKey->Top = row;
    pKey->Bottom = row + 1;
    pKey->Left = use_column ? 0 : keyboard->NumberColumns - 3;
    pKey->Right = use_column ? 1 : keyboard->NumberColumns - 1;
    /* translators: Abbreviation/mnemonic for "numeric keypad", but must be <= 10 chars */
    gok_key_add_label (pKey, _("Num\nPad"), 0, 0, NULL);

    if (use_column) row = keyboard->NumberRows;
    /* add a branch key for the "hide" keyboard: useful 
       for getting the compose kbd out of the way temporarily */
    pKey = gok_key_new (pKey, NULL, keyboard);
    pKey->Style = KEYSTYLE_BRANCH;
    pKey->Type = KEYTYPE_BRANCH;
    pKey->Target = g_strdup ("hide");
    pKey->Top = row;
    pKey->Bottom = row + 1;
    pKey->Left = use_column ? 0 : keyboard->NumberColumns - 1;
    pKey->Right = use_column ? 1 : keyboard->NumberColumns;
    /* translators: Abbreviation/mnemonic for "numeric keypad", but must be <= 10 chars */
    gok_key_add_label (pKey, _("Hide"), 0, 0, NULL);
}

static gboolean
gok_keyboard_add_keys_from_xkb_geom (GokKeyboard *pKeyboard, XkbDescPtr kbd)
{
	GokKey *pKey = pKeyboard->pKeyFirst;
	XkbGeometryPtr geom;
	int row, col, i, rightmost = 0, bottommost = 0, gok_row = 1, gok_col = 0;

	if (kbd && kbd->geom) 
	{
		geom = kbd->geom;
	}
	else 
	{
		g_warning (_("Keyboard Geometry cannot be read from your X Server."));
		return FALSE;
	}

	gok_modifier_add ("shift");
	gok_modifier_add ("capslock");
	gok_modifier_add ("ctrl");
	gok_modifier_add ("alt");
	gok_modifier_add ("mod2");
	gok_modifier_add ("mod3");
	gok_modifier_add ("mod4");
	gok_modifier_add ("mod5");
        gok_log_x ("core xkb keyboard has %d sections\n",
		 geom->num_sections);
	gok_keyboard_xkb_geom_sections_init (geom->sections, geom->num_sections);
	
	for (i = 0; i < geom->num_sections; i++) {
		XkbSectionPtr section = &geom->sections[i];
		int gok_section_first_col = 
			gok_keyboard_get_section_column (i);
		gok_row = gok_keyboard_get_section_row (i);
		for (row = 0; row < section->num_rows; row++, gok_row++) {
			XkbRowPtr rowp = &section->rows[row];
			gok_col = gok_section_first_col;
			if (i == 0 && row == 0 && gok_main_get_login ())
			{ 
			    gok_col += 1; /* special-case: 'Menus' key at login */
			}
			for (col = 0; col < rowp->num_keys; col++) {
				pKey = gok_key_from_xkb_key (pKey, pKeyboard, 
							     gok_keyboard_get_display (),
							     kbd->geom,
							     rowp,
							     section,
							     &rowp->keys[col], 
							     i, gok_row, gok_col);
				if (rowp->vertical == False)
					gok_col = pKey->Right;
				else
					gok_row = pKey->Bottom;
				rightmost = ( gok_col > rightmost ) ? gok_col : rightmost;
				bottommost = ( gok_row > bottommost ) ? gok_row : bottommost;
			}
		}
	}

	pKeyboard->NumberColumns = rightmost;
	pKeyboard->NumberRows = bottommost;

	return TRUE;
}

Display *
gok_keyboard_get_display ()
{
        if (m_XkbDisplay == NULL)
	{
	    m_XkbDescPtr = gok_keyboard_get_xkb_desc ();
	}
	return m_XkbDisplay;
}

void
gok_keyboard_clear_completion_keys (GokKeyboard *keyboard)
{	
	/* clear the current word predictions */
	GokKey *pKey = keyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey->Type == KEYTYPE_WORDCOMPLETE || pKey->Type == KEYTYPE_ADDWORD)
		{
			gok_key_set_button_label (pKey, "");
			gok_key_add_label (pKey, "", 0, 0, NULL);
			gok_key_set_output (pKey, 0, NULL, 0);
			if (pKey->Type == KEYTYPE_ADDWORD)
			{
			    if (pKey->Target) 
			    {
				g_free (pKey->Target);
				pKey->Target = NULL;
			    }
			}
		}
		pKey = pKey->pKeyNext;
	}
}


/**
 * gok_keyboard_add_keys_from_charstrings:
 *
 *
 **/
static void
gok_keyboard_add_keys_from_charstrings (GokKeyboard *pKeyboard, 
					gchar *level0_string,
					gchar *level1_string,
					gchar *level2_string,
					gchar *level3_string)
{
	GokKey *pKey = pKeyboard->pKeyFirst;
	gboolean has_level_1 = level1_string && *level1_string;
	gboolean has_level_2 = level2_string && *level2_string;
	gboolean has_level_3 = level3_string && *level3_string;

	if (pKey)
	{
	    while (pKey->pKeyNext) 
	    {
		pKey = pKey->pKeyNext;
	    }

	}
	if (level0_string)
	{
	        level0_string = g_utf8_strchr (level0_string, -1, '|');
		if (level0_string) ++level0_string; /* works because '|' is one byte */
	}

	if (!(level0_string && g_utf8_validate (level0_string, -1, NULL)) || 
	    (level1_string && !g_utf8_validate (level1_string, -1, NULL)) || 
	    (level2_string && !g_utf8_validate (level2_string, -1, NULL)) || 
	    (level3_string && !g_utf8_validate (level3_string, -1, NULL))) 
	{
		return;
	}

	if (has_level_1) 
	{
		gok_modifier_add ("shift");
	        level1_string = g_utf8_strchr (level1_string, -1, '|');
		if (level1_string) ++level1_string; /* works because '|' is one byte */
		has_level_1 = strlen (level3_string) > 0;
	}
	if (has_level_2)
	{
		gok_modifier_add ("level2");
	        level2_string = g_utf8_strchr (level2_string, -1, '|');
		if (level2_string) ++level2_string; /* works because '|' is one byte */
		has_level_2 = strlen (level2_string) > 0;
	}
	if (has_level_3)
	{
		gok_modifier_add ("level3");
	        level3_string = g_utf8_strchr (level3_string, -1, '|');
		if (level3_string) ++level3_string; /* works because '|' is one byte */
		has_level_3 = strlen (level3_string) > 0;
	}

	while (*level0_string)
	{
		gchar utf8_char[7];
		pKey = gok_key_new (pKey, NULL, pKeyboard);
		pKey->has_text = TRUE;
		pKey->Type = KEYTYPE_NORMAL;
		pKey->is_repeatable = TRUE;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->FontSize = -1;
		pKey->FontSizeGroup = FONT_SIZE_GROUP_GLYPH;
		utf8_char [g_unichar_to_utf8 (g_utf8_get_char (level0_string), utf8_char)] = '\0';
		gok_keylabel_new (pKey, utf8_char, 0, 0, NULL);
		pKey->pOutput = gok_output_new (OUTPUT_KEYSTRING, utf8_char, SPI_KEY_STRING);
		/* FIXME: need this to work for other levels too! */
		level0_string = g_utf8_find_next_char (level0_string, NULL);
		if (*level1_string)
		{
			utf8_char [g_unichar_to_utf8 (g_utf8_get_char (level1_string), utf8_char)] = '\0';
			gok_keylabel_new (pKey, utf8_char, 1, 0, NULL);
			level1_string = g_utf8_find_next_char (level1_string, NULL);
		}
		if (*level2_string)
		{
		  /* FIXME: get the appropriate modifiers and mod masks! */
			utf8_char [g_unichar_to_utf8 (g_utf8_get_char (level2_string), utf8_char)] = '\0';
			gok_keylabel_new (pKey, utf8_char, 2, 0, NULL);
			level2_string = g_utf8_find_next_char (level2_string, NULL);
		}
		if (*level3_string)
		{
			utf8_char [g_unichar_to_utf8 (g_utf8_get_char (level3_string), utf8_char)] = '\0';
			gok_keylabel_new (pKey, utf8_char, 3, 0, NULL);
			level3_string = g_utf8_find_next_char (level3_string, NULL);
		}
		if (!pKeyboard->pKeyFirst) pKeyboard->pKeyFirst = pKey;
	}

	if (has_level_1) {
		pKey = gok_key_new (pKey, NULL, pKeyboard);
		pKey->has_text = TRUE;
		pKey->Type = KEYTYPE_MODIFIER;
		pKey->is_repeatable = TRUE;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->FontSize = -1;
		pKey->FontSizeGroup = FONT_SIZE_GROUP_GLYPH;
		/* translators: "shift" as in "the shift modifier key" */
		gok_keylabel_new (pKey, _("shift"), 0, 0, NULL);
		pKey->pOutput = gok_output_new (OUTPUT_KEYSYM, "Shift_L", SPI_KEY_PRESSRELEASE);
	}
	if (has_level_2) {
		pKey = gok_key_new (pKey, NULL, pKeyboard);
		pKey->has_text = TRUE;
		pKey->Type = KEYTYPE_MODIFIER;
		pKey->is_repeatable = TRUE;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->FontSize = -1;
		pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
		/* translators: The context is "key level" as in shift/caps status on keyboard */
		gok_keylabel_new (pKey, _("Level 2"), 0, 0, NULL);
		pKey->pOutput = gok_output_new (OUTPUT_KEYSYM, "ISO_Level2_Latch", SPI_KEY_PRESSRELEASE);
	}
	if (has_level_3) {
		pKey = gok_key_new (pKey, NULL, pKeyboard);
		pKey->has_text = TRUE;
		pKey->Type = KEYTYPE_MODIFIER;
		pKey->is_repeatable = TRUE;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->FontSize = -1;
		pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
		/* translators: see note for "Level 2" */
		gok_keylabel_new (pKey, _("Level 3"), 0, 0, NULL);
		pKey->pOutput = gok_output_new (OUTPUT_KEYSYM, "ISO_Level3_Latch", SPI_KEY_PRESSRELEASE);
	}

	pKey = gok_key_new (pKey, NULL, pKeyboard);
	pKey->has_text = TRUE;
	pKey->Type = KEYTYPE_NORMAL;
	pKey->is_repeatable = TRUE;
	pKey->Style = KEYSTYLE_NORMAL;
	pKey->FontSize = -1;
	pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
	/* translators: this is a label for a 'Back space' key */
	gok_keylabel_new (pKey, _("Back\nSpace"), 0, 0, NULL);
	pKey->pOutput = gok_output_new (OUTPUT_KEYSYM, "BackSpace", SPI_KEY_PRESSRELEASE);

	pKey = gok_key_new (pKey, NULL, pKeyboard);
	pKey->has_text = TRUE;
	pKey->Type = KEYTYPE_NORMAL;
	pKey->is_repeatable = TRUE;
	pKey->Style = KEYSTYLE_NORMAL;
	pKey->FontSize = -1;
	pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
	/* translators: this is a label for a "Tab" key, for instance on a keyboard */
	gok_keylabel_new (pKey, _("Tab"), 0, 0, NULL);
	pKey->pOutput = gok_output_new (OUTPUT_KEYSYM, "Tab", SPI_KEY_PRESSRELEASE);

	pKey = gok_key_new (pKey, NULL, pKeyboard);
	pKey->has_text = TRUE;
	pKey->Type = KEYTYPE_NORMAL;
	pKey->is_repeatable = TRUE;
	pKey->Style = KEYSTYLE_NORMAL;
	pKey->FontSize = -1;
	pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
	/* translators: this is a label for a "spacebar" key, for instance on a keyboard */
	gok_keylabel_new (pKey, _("space"), 0, 0, NULL);
	pKey->pOutput = gok_output_new (OUTPUT_KEYSYM, "space", SPI_KEY_PRESSRELEASE);

	pKey = gok_key_new (pKey, NULL, pKeyboard);
	pKey->has_text = TRUE;
	pKey->Type = KEYTYPE_NORMAL;
	pKey->is_repeatable = TRUE;
	pKey->Style = KEYSTYLE_NORMAL;
	pKey->FontSize = -1;
	pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
	/* translators: this is a label for an "Enter" or Return key, for instance on a keyboard */
	gok_keylabel_new (pKey, _("Enter"), 0, 0, NULL);
	pKey->pOutput = gok_output_new (OUTPUT_KEYSYM, "Return", SPI_KEY_PRESSRELEASE);
}

/**
 * gok_keyboard_compose_create:
 *
 * Creates a new compose (i.e. alphanumeric) keyboard with name
 * @keyboard_name, including word-completion and 'back' keys if
 * appropriate in the current GOK context/configuration.
 **/
static GokKeyboard*
gok_keyboard_compose_create (gchar *keyboard_name, KeyboardLayouts layout_type)
{
	GokKeyboard *pKeyboard = gok_keyboard_new ();
	GokKey *pKey;
	Accessible *acc_with_text;
	gok_keyboard_set_name (pKeyboard, _(keyboard_name));
	pKeyboard->bRequiresLayout = FALSE;
	pKeyboard->bLaidOut = TRUE;
	pKeyboard->bSupportsWordCompletion = TRUE;
	pKeyboard->bSupportsCommandPrediction = FALSE;
	pKeyboard->LayoutType = layout_type;
	gok_log("gok_main_get_login");

	acc_with_text = gok_spy_get_accessibleWithText ();
       
	if (gok_main_get_login ()) {
		/* add menu grabbing */
		pKey = gok_key_new (NULL, NULL, pKeyboard);
		pKey->Type = KEYTYPE_BRANCHMENUS;
		pKey->Style = KEYSTYLE_BRANCHMENUS;
		pKey->FontSize = -1;
		pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
		pKey->Top = 0;
		pKey->Bottom = 1;
		pKey->Left = 0;
		pKey->Right = 1;
		gok_keylabel_new (pKey, _("Menus"), 0, 0, NULL); 
		pKeyboard->pKeyFirst = pKey;
	}
	else if (acc_with_text && 
		 (Accessible_getRole (acc_with_text) == SPI_ROLE_PASSWORD_TEXT))
	{
	    pKeyboard->bSupportsWordCompletion = FALSE;
	}
	return pKeyboard;
}


/**
 * gok_keyboard_get_alpha:
 *
 * Returns: a pointer to a #GokKeyboard which contains the current locale/LANG's
 * alphabet in "sorted" (e.g. alphabetical) order.
 **/
GokKeyboard*
gok_keyboard_get_alpha ()
{
	GokKeyboard *pKeyboard;
	GokKey *pKey;
	gok_log_enter();
	pKeyboard = gok_keyboard_compose_create (ALPHA_KEYBOARD, KEYBOARD_LAYOUT_NORMAL);
	/* The third string is not used in the C locale but corresponds to a shift
	 * level of '2' which is often associated with the AltGr key on physical keyboards.
	 *
	 * The order in which punctuation occurs is not critical, but if there is a 
	 * "standard" layout for your locale's physical keyboards it is nice to place
	 * the punctuation in the order in which it appears on physical keycaps.
	 * For instance, since Shift-plus-digits for "1,2,3,4" on a US keyboard 
	 * produce "!, \", 3, $", etc., it is convenient to list these punctuation marks
	 * in the same order in the "level 1" string as we do below for the C locale.
	 *
	 * For languages
	 * which use shift or level modifiers to select between different glyphs, the meaning
	 * of 'level' above, and as interpreted by the X server, may be slightly different.
	 * If you have questions or comments about the intent of this string please contact
	 * billh@gnome.org or david.bolter@utoronto.ca for more information.
	 *
	 */
	gok_keyboard_add_keys_from_charstrings (pKeyboard, 
						/*
						 * Note to Translators: the following strings should contain your LANG/locale's 
						 * alphabet or, in the case of LANGs with a very large glyph set, a set of
						 * character-primitives which can be used to compose your language's character set.
						 * Each string below corresponds to the characters associated with a particular
						 * "shift level" in the XKB keyboard definition.
						 * The prefix before the '|' character is just a context string and need not be
						 * translated.
						 *
						 * For languages where 'case' is used, the first string should contain 
						 * the lowercase alphabet. 
						 *
						 * Note that unless your locale clearly requires that digits and/or punctuation
						 * precede alphabetic characters, digits and punctuation should be placed 
						 * at the end of the string. 
						 */
						_("level 0|abcdefghijklmnopqrstuvwxyz1234567890-=[];'#\\,./"),
						/* The substring "level 1|" should not be translated.
						 * For languages/locales which use 'upper case', this string should 
						 * correspond to uppercase versions of characters in the 'level 0' string.
						 */
						_("level 1|ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"3$%^&*()_+{}:@~<>?"),
						/* Not used in C locale: this string can contain a third set of characters
						 * at another 'shift level'.  It can be used to provide a second/alternate
						 * glyph/character set for the locale, separately or in conjunction with
						 * 'level 3'.  At the translator's discretion, accented characters can be
						 * placed here and in 'level 3' as well. 
						 */
						_("level 2|"),
						/* For locales which need an even larger character set, or offer uppercase versions
						 * of the 'level2' characters, add them to 'level 3' */
						_("level 3|")); 

	if (!gok_main_get_login ()) 
	{
	    gok_keyboard_add_compose_aux_keys (pKeyboard);
	}
	pKeyboard->bRequiresLayout = TRUE;
	pKeyboard->bLaidOut = FALSE;
	gok_keyboard_layout (pKeyboard, pKeyboard->LayoutType, KEYBOARD_SHAPE_KEYSQUARE, FALSE);
	gok_keyboard_count_rows_columns (pKeyboard);
	gok_log ("created core keyboard with %d rows and %d columns\n",
		 gok_keyboard_get_number_rows (pKeyboard), 
		 gok_keyboard_get_number_columns (pKeyboard));
	gok_log_leave();
	return pKeyboard;
}

/**
 * gok_keyboard_get_alpha_by_frequency:
 *
 * Returns: a pointer to a #GokKeyboard which contains the current locale/LANG's
 * alphabet arranged with the most-frequently-occurring characters at the 
 * upper left; reduces total user input effort when in scanning input modes.
 **/
GokKeyboard *
gok_keyboard_get_alpha_by_frequency ()
{
	/* 
	 * TODO: Lay out the keys according to scan-operations required, 
	 *   instead of in the order in which they are added
	 */
	GokKeyboard *pKeyboard;
	GokKey *pKey;
	gok_log_enter();
	pKeyboard = gok_keyboard_compose_create (FREQ_KEYBOARD, KEYBOARD_LAYOUT_UPPERL);

	gok_keyboard_add_keys_from_charstrings (pKeyboard, 
	/*
	 * Note to Translators: the following strings should contain your LANG/locale's 
	 * alphabet or, in the case of LANGs with a very large glyph set, a set of
	 * character-primitives which can be used to compose your language's character set. 
	 * This string should contain all of the glyphs in the "level #|abcde..." strings
	 * but they should appear in 'frequency order', that is, the most frequently occurring
	 * characters in your locale should appear at the front of the list.
	 * (Put digits after characters, and punctuation last).
	 * If level 0 and level 1 refer to upper-and-lower-case in your locale, 
	 * the characters in these two strings should occur in the same relative order.
	 */
						_("level 0|etaonrishdlfcmugypwbvkxjqz`1234567890-=\\[];'<,./"),
						_("level 1|ETAONRISHDLFCMUGYPWBVKXJQZ~!@#$%^&*()_+|{}:\"><>?"),
						/* 
						 * Seldom-used or alternate characters can appear in levels 2 and 3 if necessary.
						 */
						_("level 2|\0"),
						_("level 3|\0")); /* no level 2 or 3 in C locale */
	if (!gok_main_get_login ()) 
	{
	    gok_keyboard_add_compose_aux_keys (pKeyboard);
	}
	pKeyboard->bRequiresLayout = TRUE;
	pKeyboard->bLaidOut = FALSE;
	gok_keyboard_layout (pKeyboard, pKeyboard->LayoutType, KEYBOARD_SHAPE_KEYSQUARE, FALSE);
	gok_keyboard_count_rows_columns (pKeyboard);
	gok_log ("created core keyboard with %d rows and %d columns\n",
		 gok_keyboard_get_number_rows (pKeyboard), 
		 gok_keyboard_get_number_columns (pKeyboard));
	gok_log_leave();
	return pKeyboard;
}

static GokKeyboard*
gok_keyboard_get_compose ()
{
	const gchar *compose_name;

	switch (gok_data_get_compose_keyboard_type ())
	{
	case GOK_COMPOSE_XKB:
	case GOK_COMPOSE_DEFAULT:
		compose_name = "Keyboard";
		break;
	case GOK_COMPOSE_ALPHA:
		compose_name = "Alpha";
		break;
	case GOK_COMPOSE_ALPHAFREQ:
		compose_name = "Alpha-Frequency";
		break;
	case GOK_COMPOSE_CUSTOM:
		compose_name = gok_main_get_custom_compose_kbd_name ();
		break;
	}
	return gok_main_keyboard_find_byname (compose_name);
}

/**
* gok_keyboard_get_core:
*
* Returns: a pointer to a #GokKeyboard representing the core system 
* keyboard device, with the same row/column geometry and key symbols.
**/
GokKeyboard *
gok_keyboard_get_core ()
{
	GokKeyboard *pKeyboard;
	gok_log_enter();
	pKeyboard = gok_keyboard_compose_create (CORE_KEYBOARD, KEYBOARD_LAYOUT_NORMAL);
	if (gok_keyboard_add_keys_from_xkb_geom (pKeyboard, 
						  gok_keyboard_get_xkb_desc ()))
	{
		if (!gok_main_get_login ()) 
		{
			gok_keyboard_add_compose_aux_keys (pKeyboard);
		}
		gok_keyboard_count_rows_columns (pKeyboard);
		gok_log ("created core keyboard with %d rows and %d columns\n",
			 gok_keyboard_get_number_rows (pKeyboard), 
			 gok_keyboard_get_number_columns (pKeyboard));
	}
	else
	{	  
		g_free (pKeyboard);
		pKeyboard = NULL;
	}
	gok_log_leave();
	return pKeyboard;
}

/**
* gok_keyboard_read
* @Filename: Name of the keyboard file.
*
* Reads in the given keyboard file. 
* Note: Call 'gok_keyboard_delete' on this keyboard when done with it.
*
* Returns: A pointer to the new keyboard, NULL if not created.
**/
GokKeyboard* gok_keyboard_read (const gchar* Filename)
{
	GokKeyboard* pKeyboard;
	xmlDoc* pDoc;
	xmlNode* pNodeFirst;
	xmlNs* pNamespace;

	g_assert (Filename != NULL);

	/* read in the file and create a DOM */
	pDoc = xmlParseFile (Filename);
	if (pDoc == NULL)
	{
		gok_log_x ("Error: gok_keyboard_read failed - xmlParseFile failed. Filename: '%s'", Filename);
		return NULL;
	}

	/* check if the document is empty */
	pNodeFirst = xmlDocGetRootElement (pDoc);
    if (pNodeFirst == NULL)
	 {
		gok_log_x ("Error: gok_keyboard_read failed - first node empty. Filelname: %s", Filename);
		xmlFreeDoc (pDoc);
		return NULL;
	}

	/* check if the document has the correct namespace */
	pNamespace = xmlSearchNsByHref (pDoc, pNodeFirst, 
							(const xmlChar *) "http://www.gnome.org/GOK");
	if (pNamespace == NULL)
	{
		gok_log_x ("Error: Can't create new keyboard '%s'- does not have GOK Namespace.", Filename);
		xmlFreeDoc (pDoc);
		return NULL;
	}

	/* check if this is a "GokFile" */
	if (xmlStrcmp (pNodeFirst->name, (const xmlChar *) "GokFile") != 0)
	{
		gok_log_x ("Error: Can't create new keyboard '%s'- root node is not 'GokFile'.", Filename);
		xmlFreeDoc (pDoc);
		return NULL;
    }

	/* create a new keyboard structure */
    pKeyboard = gok_keyboard_new();
    if (pKeyboard == NULL)
	 {
		gok_log_x ("Error: Can't create new keyboard '%s'!", Filename);
		xmlFreeDoc (pDoc);
		return NULL;
    }

	/* add the keys to the keyboard */
	gok_keyboard_add_keys (pKeyboard, pDoc);

	/* free up the XML doc cause we're done with it */
	xmlFreeDoc (pDoc);

	/* count the number of rows & columns in the keyboard */
	gok_keyboard_count_rows_columns (pKeyboard);

	gok_modifier_add ("shift"); /* always track shift */

	/* the keyboard is OK, return a pointer to it */
	gok_log("new keyboard created");
	return pKeyboard;
}

/**
* gok_keyboard_add_keys
* @pKeyboard: Pointer to the keyboard that will contain the keys.
* @pDoc: Pointer to the XML document that describes the keys.
*
* Adds the keys from the given DOM to this keyboard.
* The keys will all be deleted when gok_keyboard_delete is called.
*
* returns: TRUE if the keys were added, FALSE if not.
**/
gboolean gok_keyboard_add_keys (GokKeyboard* pKeyboard, xmlDoc* pDoc)
{
	xmlNode* pNodeRoot;
	xmlNode* pNodeKeyboard;
	xmlNode* pNodeKey;
	GokKey* pKeyPrevious;
	GokKey* pKeyNew;
	xmlChar* pStringAttributeValue;

	g_assert (pKeyboard != NULL);
	g_assert (pDoc != NULL);

	/* get the root node of the XML doc */
	pNodeRoot = xmlDocGetRootElement (pDoc);
	if (pNodeRoot == NULL)
	{
		gok_log_x ("Error: gok_keyboard_add_keys failed, pDoc is NULL!");
		return FALSE;
	}

	/* find the 'keyboard' node */
	pNodeKeyboard = gok_keyboard_find_node (pNodeRoot, "keyboard");
	if (pNodeKeyboard == NULL)
	{
		gok_log_x ("Error: gok_keyboard_add_keys failed, can't find 'keyboard' node!");
		return FALSE;	
	}

	/* get the name and type of the keyboard */
	pStringAttributeValue = xmlGetProp (pNodeKeyboard, (const xmlChar *) "name");
	if (pStringAttributeValue != NULL)
	{
		gok_keyboard_set_name (pKeyboard, (char *) pStringAttributeValue);
	}
	else
	{
		/* keyboard must have a name attribute*/
		gok_log_x ("Error: gok_keyboard_add_keys failed: can't find 'name' attribute for keyboard.");
		return FALSE;
	}

	/* get the word completion on/off flag */
	pStringAttributeValue = xmlGetProp (pNodeKeyboard, (const xmlChar *) "wordcompletion");
	if (pStringAttributeValue != NULL)
	{
		if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "yes") == 0)
		{
			pKeyboard->bSupportsWordCompletion = TRUE;
		}
		else
		{
			pKeyboard->bSupportsWordCompletion = FALSE;
		}
	}

	/* get the command prediction on/off flag */
	pStringAttributeValue = xmlGetProp (pNodeKeyboard, (const xmlChar *) "commandprediction");
	if (pStringAttributeValue != NULL)
	{
		if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "yes") == 0)
		{
			pKeyboard->bSupportsCommandPrediction = TRUE;
		}
		else
		{
			pKeyboard->bSupportsCommandPrediction = FALSE;
		}
	}

	/* get the type of the keyboard (this attribute is optional) */
	pStringAttributeValue = xmlGetProp (pNodeKeyboard, (const xmlChar *) "layouttype");
	if (pStringAttributeValue != NULL)
	{
		pKeyboard->LayoutType = atoi ((char *)pStringAttributeValue);
	}

	/* get the expansion policy of the keyboard (this attribute is optional) */
	pStringAttributeValue = xmlGetProp (pNodeKeyboard, (const xmlChar *) "expand");
	if (pStringAttributeValue != NULL)
	{
		if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "never") == 0)
		{
			pKeyboard->expand = GOK_EXPAND_NEVER;
		}
		else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "always") == 0)
		{
			pKeyboard->expand = GOK_EXPAND_ALWAYS;
		}
	}

	/* go through all the keys */
	pKeyPrevious = NULL;
	pNodeKey = pNodeKeyboard->xmlChildrenNode;
	while (pNodeKey != NULL)
	{
		/* is this a "key" node ? */
		if (xmlStrcmp (pNodeKey->name, (const xmlChar *) "key") != 0)
		{
			/* not a "key" node, move on to the next node */
			pNodeKey = pNodeKey->next;
			continue;
		}
		
		/* create a new key */
		pKeyNew = gok_key_new (pKeyPrevious, NULL, pKeyboard);
		if (pKeyNew == NULL)
		{
			return FALSE;
		}
		pKeyPrevious = pKeyNew;

		/* initialize the key with data from the XML DOM */
		gok_key_initialize (pKeyNew, pNodeKey);

		/* get the next key */
		pNodeKey = pNodeKey->next;
	}

	return TRUE;
}

/**
* gok_keyboard_delete
* @pKeyboard: Pointer to the keyboard that's getting deleted.
* @bForce: TRUE if the keyboard should be deleted even if it is in the stack.
*
* Deletes the given keyboard. This must be called on every keyboard that has
* been created. Don't use the given keyboard after calling this.
**/
void gok_keyboard_delete (GokKeyboard* pKeyboard, gboolean bForce)
{
	GokKey* pKey;
	GokKey* pKeyTemp;

	gok_log_enter();
		
	/* handle NULL pointers */
	if (pKeyboard == NULL)
	{
		gok_log_leave();
		return;
	}

	if ((bForce == FALSE) && (gok_branchbackstack_contains(pKeyboard) == TRUE))
	{
		gok_log_x("keyboard is in the stack, you must force deletion!");
		gok_log_leave();
		return;
	}

	gok_log("deleting keyboard: %s",pKeyboard->Name);
	
	if (pKeyboard->pAccessible != NULL)
	{
		gok_spy_accessible_unref(pKeyboard->pAccessible);
	}
	
	/* delete all the keys on the keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		pKeyTemp = pKey;
		pKey = pKey->pKeyNext;
		gok_log ("deleting key with accessible=%x", 
			 pKeyTemp->accessible_node ? pKeyTemp->accessible_node->paccessible : NULL);
		gok_key_delete (pKeyTemp, NULL, FALSE);
	}

	/* delete any chunks on the keyboard */
	gok_chunker_delete_chunks (pKeyboard->pChunkFirst, TRUE);
	
	/* unhook it from the keyboard list */
	if (gok_main_get_first_keyboard() == pKeyboard)
	{
		if (pKeyboard->pKeyboardPrevious != NULL)
		{
			gok_main_set_first_keyboard (pKeyboard->pKeyboardPrevious);
		}
		else
		{
			gok_main_set_first_keyboard (pKeyboard->pKeyboardNext);
		}
	}
	
	if (pKeyboard->pKeyboardPrevious != NULL)
	{
		pKeyboard->pKeyboardPrevious->pKeyboardNext = pKeyboard->pKeyboardNext;
	}
	
	if (pKeyboard->pKeyboardNext != NULL)
	{
		pKeyboard->pKeyboardNext->pKeyboardPrevious = pKeyboard->pKeyboardPrevious;
	}

	m_NumberOfKeyboards--;

	g_free (pKeyboard);
	gok_log_leave();
}

/**
* gok_keyboard_new
*
* Allocates memory for a new keyboard and initializes the GokKeyboard structure.
* Call gok_keyboard_delete on this when done with it.
*
* returns: A pointer to the new keyboard, NULL if it can't be created.
**/
GokKeyboard* gok_keyboard_new ()
{
	GokKeyboard* pGokKeyboardNew;

	/* allocate memory for the new keyboard structure */
	pGokKeyboardNew = (GokKeyboard*) g_malloc(sizeof(GokKeyboard));
	
	/* initialize the data members of the structure */
	strcpy(pGokKeyboardNew->Name, "unknown");
	pGokKeyboardNew->LayoutType = KEYBOARD_LAYOUT_NORMAL;
	pGokKeyboardNew->shape = KEYBOARD_SHAPE_BEST;
	pGokKeyboardNew->Type = KEYBOARD_TYPE_PLAIN;
	pGokKeyboardNew->bDynamicallyCreated = FALSE;
	pGokKeyboardNew->NumberRows = 0;
	pGokKeyboardNew->NumberColumns = 0;
	pGokKeyboardNew->bRequiresLayout = TRUE;
	pGokKeyboardNew->bLaidOut = FALSE;
	pGokKeyboardNew->bFontCalculated = FALSE;
	pGokKeyboardNew->pKeyFirst = NULL;
	pGokKeyboardNew->pKeyboardNext = NULL;
	pGokKeyboardNew->pKeyboardPrevious = NULL;
	pGokKeyboardNew->bRequiresChunking = FALSE;
	pGokKeyboardNew->pChunkFirst = NULL;
	pGokKeyboardNew->bSupportsWordCompletion = FALSE;
	pGokKeyboardNew->bSupportsCommandPrediction = FALSE;
	pGokKeyboardNew->bWordCompletionKeysAdded = FALSE;
	pGokKeyboardNew->bCommandPredictionKeysAdded = FALSE;
	pGokKeyboardNew->expand = GOK_EXPAND_SOMETIMES;
	pGokKeyboardNew->pAccessible = NULL;
	pGokKeyboardNew->keyWidth = 0;
	pGokKeyboardNew->keyHeight = 0;
	pGokKeyboardNew->flags.value = 0;
	
	m_NumberOfKeyboards++;
	
	return pGokKeyboardNew;
}

/**
* gok_keyboard_get_keyboards
* 
* Returns: The number of keyboards loaded.
**/
gint gok_keyboard_get_keyboards ()
{
	return m_NumberOfKeyboards;
}

/**
* gok_keyboard_get_wordcomplete_keys_added
* @pKeyboard: Pointer to the keyboard that we're testing.
*
* Returns: TRUE if the given keyboard has the word completion keys added, FALSE if not.
**/
gboolean gok_keyboard_get_wordcomplete_keys_added (GokKeyboard* pKeyboard)
{
	return pKeyboard->bWordCompletionKeysAdded;	
}

/**
* gok_keyboard_set_wordcomplete_keys_added
* @pKeyboard: Pointer to the keyboard that is changed.
* @bTrueFalse: TRUE if you want the predictor keys added, FALSE if not.
**/
void gok_keyboard_set_wordcomplete_keys_added (GokKeyboard* pKeyboard, gboolean bTrueFalse)
{
	pKeyboard->bWordCompletionKeysAdded = bTrueFalse;	
}

/**
* gok_keyboard_get_commandpredict_keys_added
* @pKeyboard: Pointer to the keyboard that we're testing.
*
* returns: TRUE if the given keyboard has the word completion keys added, FALSE if not.
**/
gboolean gok_keyboard_get_commandpredict_keys_added (GokKeyboard* pKeyboard)
{
	return pKeyboard->bCommandPredictionKeysAdded;	
}

/**
* gok_keyboard_set_commandpredict_keys_added
* @pKeyboard: Pointer to the keyboard that is changed.
* @bTrueFalse: TRUE if you want the prediction keys added, FALSE if not.
**/
void gok_keyboard_set_commandpredict_keys_added (GokKeyboard* pKeyboard, gboolean bTrueFalse)
{
	pKeyboard->bCommandPredictionKeysAdded = bTrueFalse;	
}

/**
* gok_keyboard_get_accessible
* @pKeyboard: Pointer to the keyboard that we're using
*
* Returns: pointer to the accessible (probably shared by keys on this keyboard)
**/
Accessible* gok_keyboard_get_accessible (GokKeyboard* pKeyboard)
{
	return pKeyboard->pAccessible;
}

/**
* gok_keyboard_set_accessible
* @pKeyboard: Pointer to the keyboard that is to be changed.
* @pAccessible: Pointer to the new accessible interface.
**/
void gok_keyboard_set_accessible (GokKeyboard* pKeyboard, Accessible* pAccessible)
{
	g_assert (pKeyboard != NULL);
	
	if (pKeyboard->pAccessible != NULL)
	{
		if (pKeyboard->pAccessible != pAccessible)
		{
			gok_spy_accessible_unref(pKeyboard->pAccessible);
			gok_spy_accessible_ref (pAccessible);
			gok_log("setting keyboard accessible with address: [%#x]",pAccessible);
			pKeyboard->pAccessible = pAccessible;
		}
		else
		{
			/* do nothing */
			gok_log("tried to set keyboard accessible to the same value it already has");
		}
	}
	else
	{
		gok_log("setting keyboard accessible with address: [%#x]",pAccessible);
		pKeyboard->pAccessible = pAccessible;
		gok_spy_accessible_ref(pAccessible);
	}	
}

/**
* gok_keyboard_get_supports_wordcomplete
* @pKeyboard: Pointer to the keyboard that we're testing.
*
* returns: TRUE if the given keyboard supports word completion.
* Only alphabetic keyboards should support word completion.
**/
gboolean gok_keyboard_get_supports_wordcomplete (GokKeyboard* pKeyboard)
{
	gok_log_enter();
	g_assert (pKeyboard != NULL);
	gok_log_leave();
	return pKeyboard->bSupportsWordCompletion;	
}

/**
* gok_keyboard_get_supports_commandprediction
* @pKeyboard: Pointer to the keyboard that we're testing.
*
* returns: TRUE if the given keyboard supports command prediction.
* Any keyboard can support command prediction..
**/
gboolean gok_keyboard_get_supports_commandprediction (GokKeyboard* pKeyboard)
{
	gok_log_enter();
	g_assert (pKeyboard != NULL);
	gok_log_leave();
	return pKeyboard->bSupportsCommandPrediction;	
}

/**
* gok_keyboard_count_rows_columns
* @pKeyboard: Pointer to the keyboard that we want to get the rows and columns for.
*
* Counts the number of rows and columns in the keyboard and updates members
* of the GokKeyboard structure.
**/
void gok_keyboard_count_rows_columns (GokKeyboard* pKeyboard)
{
	GokKey* pKey;
	gint rows;
	gint columns;

	g_assert (pKeyboard != NULL);

	/* look through all the keys and find the leftmost and bottommost cells */
	rows = 0;
	columns = 0;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
	    if (pKey->Right > columns)
	    {
		columns = pKey->Right;
	    }
	    
	    if (pKey->Bottom > rows)
	    {
		rows = pKey->Bottom;
	    }
	    
	    pKey = pKey->pKeyNext;
	}
	pKeyboard->NumberRows = rows;
	pKeyboard->NumberColumns = columns;
}

/**
* gok_keyboard_get_number_rows
* @pKeyboard: Pointer to the keyboard that you're concerned about.
*
* returns: The number of rows in the given keyboard.
**/
gint gok_keyboard_get_number_rows (GokKeyboard* pKeyboard)
{
	g_assert (pKeyboard != NULL);
	return pKeyboard->NumberRows;
}

/**
* gok_keyboard_get_number_columns
* @pKeyboard: Pointer to the keyboard you want to know about.
*
* returns: The number of columns in the given keyboard.
**/
gint gok_keyboard_get_number_columns (GokKeyboard* pKeyboard)
{
	g_assert (pKeyboard != NULL);
	return pKeyboard->NumberColumns;
}

/**
* gok_keyboard_find_node
* @pNode: Pointer to the XML node that may contain the node you're looking for.
* @NameNode: Name of the node you're looking for.
*
* returns: A pointer to the first node that has the given name, NULL if it can't be found.
* Note: This is recursive.
**/
xmlNode* gok_keyboard_find_node (xmlNode* pNode, gchar* NameNode)
{
	xmlNode* pNodeChild;
	xmlNode* pNodeReturned;

	g_assert (pNode != NULL);
	g_assert (NameNode != NULL);

	if (xmlStrcmp (pNode->name, (const xmlChar *)NameNode) == 0)
	{
		return pNode;
	}
	
	pNodeChild = pNode->xmlChildrenNode;
	while (pNodeChild != NULL)
	{
		pNodeReturned = gok_keyboard_find_node (pNodeChild, NameNode);
		if (pNodeReturned != NULL)
		{
			return pNodeReturned;
		}
		pNodeChild = pNodeChild->next;
	}

	return NULL;
}

/**
* gok_keyboard_set_name
* @pKeyboard: Pointer to the keyboard that's getting named.
* @Name: Name for the keyboard.
**/
void gok_keyboard_set_name (GokKeyboard* pKeyboard, char* Name)
{
	g_assert (pKeyboard != NULL);
	g_assert (Name != NULL);
	g_assert (strlen (Name) <= MAX_KEYBOARD_NAME);

	strcpy (pKeyboard->Name, Name);
}

/**
* gok_keyboard_get_name
* @pKeyboard: Pointer to the keyboard to get the name from.
*
* returns: gchar* name of keyboard
**/
gchar* gok_keyboard_get_name (GokKeyboard* pKeyboard)
{
	g_assert (pKeyboard != NULL);
	return pKeyboard->Name;
}

/**
* gok_keyboard_calculate_font_size
* @pKeyboard: Pointer to the keyboard that gets the new font size.
* 
* Sets the font size for each key on the given keyboard.
* Each key may be assigned to a a font size group (FSG). If the FSG is
* not specified then the key belongs to group FONT_SIZE_GROUP_UNDEFINED. 
* If the FSG is FONT_SIZE_GROUP_UNIQUE then the key does not belong to 
* any group and calculate a font size for that key.
*
**/
void gok_keyboard_calculate_font_size (GokKeyboard* pKeyboard)
{
	GokKeyboard* pKeyboardTemp;
	GokKey* pKey;
	gint sizeFont;

	gint key_width;
	gint key_height;

	/* if this keyboard doesn't have its own width/height values, use prefs */
	key_width = pKeyboard->keyWidth ? 
	    pKeyboard->keyWidth : gok_data_get_key_width ();
	key_height = pKeyboard->keyHeight ? 
	    pKeyboard->keyHeight : gok_data_get_key_height ();

	/* any time the key size changes, recalculate the font size */
	if ((m_WidthKeyFont != key_width) ||
		(m_HeightKeyFont != key_height))
	{
	        pKeyboard->bFontCalculated = FALSE;
	}
	
	/* check this flag before doing the work */
	if (pKeyboard->bFontCalculated == TRUE)
	{
		return;
	}
	
	/* clear the font size for each key */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		pKey->FontSize = -1;
		pKey = pKey->pKeyNext;
	}	
	
	/* go through all the keys on the keyboard, setting their font size */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* calculate the font size for each key that has 'unique font size' */
		if (pKey->FontSizeGroup == FONT_SIZE_GROUP_UNIQUE)
		{
			sizeFont = gok_key_calculate_font_size (pKey, TRUE, TRUE);
			gok_key_set_font_size (pKey, sizeFont);
		}
		else /* calculate the font size for the font group */
		{
			/* look at keys that haven't had their font size set yet */
			if (pKey->FontSize == -1)
			{
				gok_keyboard_calculate_font_size_group (pKeyboard, pKey->FontSizeGroup, FALSE);
			}
		}
		pKey = pKey->pKeyNext;
	}				

	pKeyboard->bFontCalculated = TRUE;
}

/**
 * gok_keyboard_get_cell_width:
 * @keyboard: a #GokKeyboard which is about to be displayed.
 * 
 * Get the cell width, that is, the width of one column of @keyboard.
 * If the keyboard is a "width expanding" keyboard, this will be the number of
 * columns divided by the screen width; otherwise it will equal the GOK key width.
 *
 * Return value: a #gint indicating the width of a single "column" in 
 * @keyboard.
 **/
gint
gok_keyboard_get_cell_width (GokKeyboard *keyboard)
{
    if (keyboard && (keyboard->expand == GOK_EXPAND_ALWAYS || 
		     (keyboard->expand == GOK_EXPAND_SOMETIMES && gok_data_get_expand ())))
    {
	GtkWidget *window = gok_main_get_main_window ();
	GdkScreen *screen;
	gint width_window;
	gboolean expand;

	if (window && window->window) 
	{
	    screen = gdk_drawable_get_screen (window->window); 
	}
	else 
	{
	    screen = gdk_screen_get_default ();
	}

	width_window = gdk_screen_get_width (screen);

	expand = ((keyboard->expand == GOK_EXPAND_ALWAYS) || 
		(gok_data_get_expand () && (keyboard->expand != GOK_EXPAND_NEVER)));

	if ((gok_data_get_dock_type () != GOK_DOCK_NONE) && expand)
	{
		width_window -= GOK_KEYBOARD_DOCK_BORDERPIX * 2;
	}

	return (width_window - (gok_data_get_key_spacing () * (keyboard->NumberColumns - 1)) 
		/ keyboard->NumberColumns);
    }
    else
    {
	return gok_data_get_key_width ();
    }
}

/**
* gok_keyboard_calculate_font_size_group
* @pKeyboard: Pointer to the keyboard that gets the new font size.
* @GroupNumber: Number of the font size group.
* @bOverride: If TRUE then the font size is set for the key even if it
* already has a font size set. If FALSE then the font size is not set for
* the key if it is already set.
* 
* Sets the font size for each key that belongs to the given group on the 
* given keyboard.
**/
void gok_keyboard_calculate_font_size_group (GokKeyboard* pKeyboard, gint GroupNumber, gboolean bOverride)
{
	GokKey* pKey;
	GokKey* pKeyWidest;
	GokKey* pKeyHighest;
	gint widthLabel;
	gint widthTemp;
	gint heightLabel;
	gint heightTemp;
	gint sizeWidestFont;
	gint sizeHighestFont;
	
	pKeyWidest = NULL;
	widthLabel = 0;
	pKeyHighest = NULL;
	heightLabel = 0;
	sizeWidestFont = GOK_MIN_FONT_SIZE; /* mimum font size */
	sizeHighestFont = GOK_MIN_FONT_SIZE; /* minimum font size */
	
	/* get font for key width */
	/* go through all the keys on the keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* look at keys that belong to the given group */
		if (pKey->FontSizeGroup == GroupNumber)
		{
			/* change font only if key hasn't had its font size set yet */
			/* or the bOverride flag is TRUE */
			if ((bOverride == TRUE) ||
				(pKey->FontSize == -1))
			{
				widthTemp = gok_key_get_label_lengthpercell (pKey);
				if (widthTemp > widthLabel)
				{
					widthLabel = widthTemp;
					pKeyWidest = pKey;
				}
				
				heightTemp = gok_key_get_label_heightpercell (pKey);
				if (heightTemp > heightLabel)
				{
					heightLabel = heightTemp;
					pKeyHighest = pKey;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}				

	if (pKeyWidest != NULL)
	{
		/* calculate the font for the longest key */
		sizeWidestFont = gok_key_calculate_font_size (pKeyWidest, TRUE, FALSE);
	}
	
	if (pKeyHighest != NULL)
	{
		/* calculate the font for the highest key */
		sizeHighestFont = gok_key_calculate_font_size (pKeyHighest, FALSE, TRUE);
	}
	
	/* sizeWidestFont will be the final font size */
	if (sizeHighestFont < sizeWidestFont)
	{
		sizeWidestFont = sizeHighestFont;
	}
	
	/* set font for all keys that belong to the same group and */
	/* haven't had their font size set yet */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey->FontSizeGroup == GroupNumber)
		{
			if ((bOverride == TRUE) ||
				(pKey->FontSize == -1))
			{
				gok_key_set_font_size (pKey, sizeWidestFont);
			}
		}
			
		pKey = pKey->pKeyNext;
	}
}

/**
* gok_keyboard_paint_pointer:
* @pKeyboard: Pointer to the keyboard where the pointer is painted.
* @pWindowMain: Pointer to the main window that holds the keyboards.
* @x : The x coordinate of the GOK pointer relative to the keyboard window.
* @y : The y coordinate of the GOK pointer relative to the keyboard window.
*
* Displays a GOK pointer at the specified location relative to the keyboard window.
*
**/
void
gok_keyboard_paint_pointer (GokKeyboard *pKeyboard, GtkWidget *pWindowMain, 
			    gint x, gint y)
{
	if (pWindowMain->window) {
		GdkGC *gc;
		GdkGCValues values;
		values.function = GDK_INVERT;
		values.line_width = 2;
		gc = gdk_gc_new_with_values (pWindowMain->window, &values, 
					     GDK_GC_FUNCTION | GDK_GC_LINE_WIDTH);	 
		m_oldPointerX = x;
		m_oldPointerY = y;
		gdk_draw_line (pWindowMain->window, gc, x-6, y, x+6, y);
		gdk_draw_line (pWindowMain->window, gc, x, y-6, x, y+6);
	}
}

/**
* gok_keyboard_unpaint_pointer:
* @pKeyboard: Pointer to the keyboard where the pointer is painted.
* @pWindowMain: Pointer to the main window that holds the keyboards.
*
* Hides the GOK pointer if it's currently in a GOK keyboard window.
*
**/
void
gok_keyboard_unpaint_pointer (GokKeyboard *pKeyboard, GtkWidget *pWindowMain)
{
	if (pWindowMain->window) {
		GdkRectangle rect;
		rect.x = m_oldPointerX - 6;
		rect.y = m_oldPointerY - 6;
		rect.width = 13;
		rect.height = 13;
		gdk_window_invalidate_rect (pWindowMain->window, 
					    &rect, True);
		gdk_window_process_updates (pWindowMain->window, True);

	}
}


/**
* gok_keyboard_display:
* @pKeyboard: Pointer to the keyboard that gets displayed.
* @pKeyboardCurrent: Pointer to the current keyboard.
* @pWindowMain: Pointer to the main window that holds the keyboards.
* @CallbackScanner: If TRUE then the keyboard is used by the GOK. If FALSE
* then the keyboard is used by the editor.
*
* Displays the given keyboard in the GOK window.
*
* returns: TRUE if the keyboard was displayed, FALSE if not.
**/
gboolean gok_keyboard_display (GokKeyboard* pKeyboard, GokKeyboard* pKeyboardCurrent, GtkWidget* pWindowMain, gboolean CallbackScanner)
{
	gchar titleWindow[MAX_KEYBOARD_NAME + 10];
	GtkWidget* pFixedContainer;
	GtkWidget* pNewButton;
	GokKey* pKey;
	gint heightWindow;
	gint widthWindow;
	gint frameX;
	gint frameY;
	gint winX;
	gint winY;
	gint widthMax;
	gint heightMax;
	gint widthKeyHold;
	gint heightKeyHold;
	gint widthKeyTemp;
	gint heightKeyTemp;
	gint borderWidth = 0;
	gint borderHeight = 0;
	GdkRectangle rectFrame;
	GdkRectangle rectTemp;
	GokButton* pGokButton;
	GtkButton* pButton;
	gboolean bKeySizeChanged = FALSE;
	gboolean expand = FALSE;

	g_assert (pKeyboard != NULL);
	g_assert (pWindowMain != NULL);

	/* hide any buttons from the previous keyboard */
	if (pKeyboardCurrent != NULL)
	{
		pKey = pKeyboardCurrent->pKeyFirst;
		while (pKey != NULL)
		{
			if (pKey->pButton != NULL)
			{
				gtk_widget_hide (pKey->pButton);
			}
			pKey = pKey->pKeyNext;
		}
	}

	/* change the name of the window to the keyboard name */
	strcpy (titleWindow, _("GOK - "));
	strcat (titleWindow, pKeyboard->Name);
	gtk_window_set_title (GTK_WINDOW(pWindowMain), titleWindow);
	
	/* get the "fixed container" that holds the buttons */
	pFixedContainer = GTK_BIN(pWindowMain)->child;
	
	if ((gok_data_get_dock_type () != GOK_DOCK_NONE) && gok_data_get_expand ()) {
		borderWidth = GOK_KEYBOARD_DOCK_BORDERPIX;
		borderHeight = GOK_KEYBOARD_DOCK_BORDERPIX;
	}

	/* create all the buttons and add them to the container */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* create a new GTK button for the key (if it's not already created) */
		if (pKey->pButton == NULL)
		{
			/* create a new GOK button */
			if (pKey->has_text)
			{
				pNewButton = gok_button_new_with_label (gok_key_get_label (pKey), IMAGE_PLACEMENT_LEFT); 
				if (pKey->has_image) {
					gok_button_set_image (GOK_BUTTON (pNewButton), 
							      GTK_IMAGE (gok_key_create_image_widget (pKey)));
					if (pKey->pImage->type != IMAGE_TYPE_INDICATOR) {
						GOK_BUTTON (pNewButton)->indicator_type = NULL;
					}
				}
			}
			else if (pKey->has_image)
			{
				pNewButton = gok_button_new_with_image (gok_key_create_image_widget (pKey), 
									pKey->pImage->placement_policy);
				if (pKey->pImage->type != IMAGE_TYPE_INDICATOR) {
					GOK_BUTTON (pNewButton)->indicator_type = NULL;
				}
			}
			else {
			        pNewButton = g_object_new (GOK_TYPE_BUTTON, NULL);
			}

			/* for modifier keys, set the indicator type */
			if (pKey->Type == KEYTYPE_MODIFIER) {
				GOK_BUTTON (pNewButton)->indicator_type = "shift";
			}
			else if (pKey->Type == KEYTYPE_REPEATNEXT) {
			        GOK_BUTTON (pNewButton)->indicator_type = "checkbox";
			}

			/* associate the button with the key */
			pKey->pButton = pNewButton;
			gtk_object_set_data (GTK_OBJECT(pNewButton), "key", pKey);

			/* set the initial state of the button from pKey state*/
			if (pNewButton && GTK_IS_TOGGLE_BUTTON (pNewButton))
			{
			        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pNewButton),
							      pKey->ComponentState.active);
			}

			/* add these signal handlers to the button */
			if (CallbackScanner == TRUE) /* button is used by GOK */
			{
				gtk_signal_connect (GTK_OBJECT (pNewButton), "button_press_event",
	                      GTK_SIGNAL_FUNC (on_window1_button_press_event),
	                      pKey);

				gtk_signal_connect (GTK_OBJECT (pNewButton), "button_release_event",
	                      GTK_SIGNAL_FUNC (on_window1_button_release_event),
	                      pKey);

				/* this next signal never occurs? */
				gtk_signal_connect (GTK_OBJECT (pNewButton), "toggled",
	                      GTK_SIGNAL_FUNC (on_window1_button_toggle_event),
	                      pKey);
	
				gtk_signal_connect (GTK_OBJECT (pNewButton), "enter_notify_event",
	                      GTK_SIGNAL_FUNC (gok_button_enter_notify),
	                      NULL);

				gtk_signal_connect (GTK_OBJECT (pNewButton), "leave_notify_event",
	                      GTK_SIGNAL_FUNC (gok_button_leave_notify),
	                      NULL);

				gtk_signal_connect (GTK_OBJECT (pNewButton), "state_changed",
						GTK_SIGNAL_FUNC (gok_button_state_changed),
						NULL);

			}
			else /* button is used by editor */
			{
				gtk_signal_connect (GTK_OBJECT (pNewButton), "button_press_event",
	                      GTK_SIGNAL_FUNC (on_editor_button_press_event),
	                      NULL);
			}

			/* set the 'name' of the button */
			/* the 'name' determines the .rc style to apply to the button */
			gok_key_set_button_name (pKey);
		}

		/* ensure all non-modifier buttons are in the 'inactive' state */
		gok_key_update_toggle_state (pKey);

		/* reset the highlight state of this key  - if we need to optimize this then only do it for branch keys... */	
		gok_feedback_unhighlight(pKey, FALSE);
		
		/* show the button */
		gtk_widget_show (pKey->pButton);

		/* get the next key in the list */
		pKey = pKey->pKeyNext;
	}	

	/* calculate the font size for this keyboard */
	gok_keyboard_calculate_font_size (pKeyboard);

	/* calculate the size of the window needed for the keys */
	widthWindow = (gok_keyboard_get_number_columns (pKeyboard) * gok_data_get_key_width()) + 
									((gok_keyboard_get_number_columns (pKeyboard) - 1) * gok_data_get_key_spacing() + borderWidth * 2);
	heightWindow = (gok_keyboard_get_number_rows (pKeyboard) * gok_data_get_key_height()) +
									((gok_keyboard_get_number_rows (pKeyboard) - 1) * gok_data_get_key_spacing() + borderHeight * 2);

	/* store the current key width and height */
	widthKeyHold = gok_data_get_key_width();
	heightKeyHold = gok_data_get_key_height();

	/* is this window bigger than the screen (or screen geometry)? */
	/* get the frame size */
	gdk_window_get_frame_extents ((GdkWindow*)pWindowMain->window, &rectFrame);
	gdk_window_get_position (pWindowMain->window, &winX, &winY);
	if ((winX != 0) &&
		(winY != 0))
	{
		frameX = (winX - rectFrame.x);
		frameY = (winY - rectFrame.y);
	}
	else
	{
		/* TODO: how can I get the frame size before the window is shown? */
		frameX = 0;
		frameY = 0;
	}
	
	if (gok_main_get_use_geometry() == TRUE)
	{
		gok_main_get_geometry (&rectTemp);
		widthMax = rectTemp.width;
		heightMax = rectTemp.height;
	}
	else if (pKeyboard->shape == KEYBOARD_SHAPE_FITWINDOW)
	{
		gdk_window_get_size ((GdkWindow*)pWindowMain->window, &widthMax,
				     &heightMax);
	}
	else
	{
		widthMax = gdk_screen_width();
		heightMax = gdk_screen_height();
	}

	expand = ((pKeyboard->expand == GOK_EXPAND_ALWAYS) || 
		  (gok_data_get_expand () && (pKeyboard->expand != GOK_EXPAND_NEVER)));

	if (((widthWindow + frameX + borderWidth) > widthMax) || 
	    (pKeyboard->shape == KEYBOARD_SHAPE_FITWINDOW) || expand)

	{
		/* change the key width (for this keyboard) to fit within the screen */
		widthKeyTemp = gok_keyboard_get_keywidth_for_window (widthMax - (frameX * 2) - borderWidth, pKeyboard);
		gok_data_set_key_width (widthKeyTemp);
		
		/* calculate a new window size */
		widthWindow = (gok_keyboard_get_number_columns (pKeyboard) * gok_data_get_key_width()) + 
									((gok_keyboard_get_number_columns (pKeyboard) - 1) * gok_data_get_key_spacing() + borderWidth * 2);
		bKeySizeChanged = TRUE;
	}
	
	if (((heightWindow + frameY - borderHeight * 2) > heightMax) ||
	    (pKeyboard->shape == KEYBOARD_SHAPE_FITWINDOW))
	{
		/* change the key height (for this keyboard) to fit within the screen */
		heightKeyTemp = gok_keyboard_get_keyheight_for_window (heightMax - frameY - (borderHeight * 2), pKeyboard);
		gok_data_set_key_height (heightKeyTemp);
		
		/* calculate a new window size */
		heightWindow = (gok_keyboard_get_number_rows (pKeyboard) * gok_data_get_key_height()) +
									((gok_keyboard_get_number_rows (pKeyboard) - 1) * gok_data_get_key_spacing() + frameY + (borderHeight * 2));
		bKeySizeChanged = TRUE;
	}
	
	/* if window resizing forced key resize, resize fonts*/
	if (bKeySizeChanged) 
	       gok_keyboard_calculate_font_size (pKeyboard);

	/* resize the window to hold all the keys */
	gok_main_resize_window (pWindowMain, pKeyboard, widthWindow, heightWindow);

	/* position and resize all the buttons */
	gok_keyboard_position_keys (pKeyboard, pWindowMain);
	
	/* replace the key width and height */
	/* (because we may have changed it for this keyboard only */
	gok_data_set_key_width (widthKeyHold);
	gok_data_set_key_height (heightKeyHold);

	/* update the indicators on all modifier keys */
	gok_modifier_update_modifier_keys (pKeyboard);

	return TRUE;
}

/**
* gok_keyboard_position_keys
* @pKeyboard: Pointer to the keyboard that contains the keys.
* @pWindow: Pointer to the window that displays the keys.
*
* Positions the keys on the keyboard. The key cell coordinates are converted into
* window locations.
**/
void gok_keyboard_position_keys (GokKeyboard* pKeyboard, GtkWidget* pWindow)
{
	GokKey* pKey;
	GtkWidget* pContainer;
	gint left, top, width, height;
	gint widthKey;
	gint heightKey;
	gint spacingKey;
	gint left_pad = 0, top_pad = 0;

	g_assert (pKeyboard != NULL);
	g_assert (pWindow != NULL);

	/* get the key size */
	/* start with the key size from the keyboard or gok_data */
	widthKey = pKeyboard->keyWidth ? pKeyboard->keyWidth : gok_data_get_key_width();
	heightKey = pKeyboard->keyHeight ? pKeyboard->keyHeight : gok_data_get_key_height();
	spacingKey = gok_data_get_key_spacing();

	/* if this is an 'expand' keyboard, calculate the effective key width */
	if ((gok_data_get_expand () && (pKeyboard->expand != GOK_EXPAND_NEVER))
	    || (pKeyboard->expand == GOK_EXPAND_ALWAYS))
	{
	    if (pWindow->window) 
	    {
		gint widthWindow, heightWindow;
		gdk_window_get_size (pWindow->window, &widthWindow, &heightWindow);
		widthKey = (double) widthWindow / pKeyboard->NumberColumns - spacingKey;
	    }
	}
		
	/* get the container from the window */
	pContainer = GTK_BIN(pWindow)->child;
	g_assert (pContainer != NULL);

	/* loop through all the keys */
	pKey = pKeyboard->pKeyFirst;
	if ((gok_data_get_dock_type () != GOK_DOCK_NONE) && gok_data_get_expand ()) {
		left_pad = GOK_KEYBOARD_DOCK_BORDERPIX;
		top_pad = GOK_KEYBOARD_DOCK_BORDERPIX;
	}
	while (pKey != NULL)
	{
		/* change the size of the button */
		width = (pKey->Right - pKey->Left) * widthKey;
		width += (pKey->Right - pKey->Left - 1) * spacingKey;
		height = (pKey->Bottom - pKey->Top) * heightKey;
		height += (pKey->Bottom - pKey->Top - 1) * spacingKey;
		gtk_widget_set_size_request (pKey->pButton, width, height);

		/* position the button */
		left = pKey->Left * (widthKey + spacingKey) + left_pad;
		top = pKey->Top * (heightKey + spacingKey) + top_pad;
		
		/* if the button has been previously 'put' then 'move' it */
		if (gtk_widget_get_parent (pKey->pButton) == NULL)
		{
			gtk_fixed_put (GTK_FIXED(pContainer), pKey->pButton, left, top);
		}
		else
		{
			/* gtk_fixed_move generates a 'resize' event so set flag to ignore it */
			m_bIgnoreResizeEvent = TRUE;
			gtk_fixed_move (GTK_FIXED(pContainer), pKey->pButton, left, top);
		}

		/* store the new position of the key */
		pKey->TopWin = top;
		pKey->BottomWin = top + height;
		pKey->LeftWin = left;
		pKey->RightWin = left + width;

		pKey = pKey->pKeyNext;
	}
}

static gboolean
gok_keyboard_page_select (AccessibleNode *node) 
{
	AccessibleSelection *selection;
	Accessible *parent;
	int index;
	gboolean retval = FALSE;
       
	parent = Accessible_getParent (node->paccessible);
	if (parent) {
		index = Accessible_getIndexInParent (node->paccessible);
		selection = Accessible_getSelection (parent);
		g_assert (selection != NULL);
		retval = AccessibleSelection_selectChild (selection, index);
	}

	return retval;
}


/**
* gok_keyboard_branch_byKey
* @keyboard: the keyboard containing the key.
* @pKey: The key that is causes the branch.
*
* Branch to another keyboard specified by given key.
* The previous keyboard is stored on the "branch back stack".
*
* returns: TRUE if keyboard branched, FALSE if not.
**/
gboolean 
gok_keyboard_branch_byKey (GokKeyboard *keyboard, GokKey* pKey)
{
	gboolean is_branched, is_active;
	AccessibleStateSet *pStateSet;

	gok_log("gok_keyboard_branch_byKey:");
	/* branch according to type */
	switch (pKey->Type)
	{
		case KEYTYPE_BRANCHBACK:
			gok_log("branch back");		
			return gok_main_display_scan_previous();/*_premade();*/
			break;

		case KEYTYPE_BRANCHMENUS:
			gok_log("branch gui MENUS");		
			return gok_keyboard_branch_gui (pKey->accessible_node, 
							GOK_SPY_SEARCH_MENU);
			break;

		case KEYTYPE_BRANCHMENUITEMS:
			gok_log("branch gui MENUS");
			return gok_keyboard_branch_gui (pKey->accessible_node, 
							GOK_SPY_SEARCH_CHILDREN);
			break;

		case KEYTYPE_BRANCHGUITABLE:
			gok_log("branch gui TABLE");
			return gok_keyboard_branch_gui (pKey->accessible_node, 
							GOK_SPY_SEARCH_TABLE_CELLS);
			break;
			
		case KEYTYPE_BRANCHGUISELECTION:
			gok_log("branch gui SELECTION");
			return gok_keyboard_branch_gui (pKey->accessible_node, 
							GOK_SPY_SEARCH_CHILDREN);
			break;
			
		case KEYTYPE_BRANCHLISTITEMS:
			gok_log("branch gui LIST");
			return gok_keyboard_branch_gui (pKey->accessible_node, 
							GOK_SPY_SEARCH_LISTITEMS);
			break;

		case KEYTYPE_BRANCHCOMBO:
			gok_log("branch gui COMBO");
			return gok_keyboard_branch_gui (pKey->accessible_node, 
							GOK_SPY_SEARCH_COMBO);
			break;

		case KEYTYPE_BRANCHTOOLBARS:
			gok_log("branch gui TOOLBAR");		
			return gok_keyboard_branch_gui (pKey->accessible_node, 
							GOK_SPY_SEARCH_TOOLBARS);
			break;

		case KEYTYPE_BRANCHGUI:
			gok_log("branch gui GOK_SPY_SEARCH_UI");		
			return gok_keyboard_branch_gui (pKey->accessible_node, 
							GOK_SPY_SEARCH_UI);
			break;			
		case KEYTYPE_PAGESELECTION:
			gok_log("page select");
			gok_keyboard_page_select (pKey->accessible_node);
			return FALSE;
			break;

	        case KEYTYPE_BRANCHGUISELECTACTION:	
			gok_log("branch gui_actions");
			is_branched = gok_keyboard_branch_gui_selectaction (keyboard, 
									    pKey->accessible_node,
									    pKey->action_ndx);
			return is_branched;
			break;

		case KEYTYPE_BRANCHGUIVALUATOR:
			gok_log("branch gui_valuator");
			is_branched = gok_keyboard_branch_gui_valuator (pKey->accessible_node);
			return is_branched;
			break;
		case KEYTYPE_BRANCHGUIACTIONS:
			gok_log("branch gui_actions");
			is_branched = gok_keyboard_branch_gui_actions (gok_main_get_current_keyboard (),
								       pKey->accessible_node, pKey->action_ndx);
			if (!is_branched) 
			{
				if (pKey->accessible_node)
				{	
					pStateSet = Accessible_getStateSet (pKey->accessible_node->paccessible);
					is_active = AccessibleStateSet_contains (pStateSet, 
										 SPI_STATE_CHECKED);
					pKey->ComponentState.active = is_active;
					AccessibleStateSet_unref (pStateSet);
				}
			}
			return is_branched;
			break;
		case KEYTYPE_BRANCHHYPERTEXT:
		        gok_log ("HYPERTEXT!");
			break;
		default:
			/* should not be here */
			gok_log ("Unknown branch type");
			break;
	}
	return FALSE;
}


static KeyStyles 
gok_style_if_enabled (AccessibleStateSet *states, KeyStyles style) 
{
        /* 
	 * We use 'SENSITIVE' here because 'ENABLED' has slightly 
	 * surprising semantics, i.e. ENABLED==FALSE for
	 * some actionable elements such as radiobuttons in the
	 * INCONSISTENT state.
	 */
	if (AccessibleStateSet_contains (states, SPI_STATE_SENSITIVE) || AccessibleStateSet_contains (states, SPI_STATE_ENABLED))
		return style;
	else
		return KEYSTYLE_INSENSITIVE;
}

static KeyStyles 
gok_style_if_selectable (AccessibleStateSet *states, KeyStyles style) 
{
	if (AccessibleStateSet_contains (states, SPI_STATE_SELECTABLE))
		return style;
	else
		return KEYSTYLE_INSENSITIVE;
}

/**
* gok_keyboard_update_dynamic
* @pKeyboard: Pointer to the keyboard that gets updated.
*
* Creates all the keys for the given dynamic keyboard.
*
* returns: TRUE if the keyboard was updated, FALSE if not.
**/
gboolean 
gok_keyboard_update_dynamic (GokKeyboard* pKeyboard)
{
	AccessibleNode* pNodeAccessible;
	Accessible* list_parent = NULL;
	GSList *nodes = NULL;
	Accessible* editbox = NULL;
	AccessibleStateSet *pStateSet;
	GokKey* pKey;
	GokKey* pKeyTemp;
	GokKey* pKeyPrevious;
	gint column;
	gboolean is_active;
	gboolean did_actionkeys = FALSE;
	
	gok_log_enter();
	g_assert(pKeyboard != NULL);

	if (pKeyboard->bDynamicallyCreated == FALSE)
	{
		gok_log_x ("Warning: Keyboard is not dynamic!");
		gok_log_leave();
		return FALSE;
	}

	/* delete any keys that are currently on the keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		pKeyTemp = pKey;
		pKey = pKey->pKeyNext;
		gok_key_delete (pKeyTemp, NULL, TRUE);
	}
	pKeyboard->pKeyFirst = NULL;

#if REINSTATE_APPLICATIONS
	if (pKeyboard->search_type == SEARCH_TYPE_APPLICATION)
	{
		pNodeAccessible = gok_spy_get_applications();
	}
	else
	{
#endif

	/* create new keys for the keyboard */
	switch (pKeyboard->search_type)
	{
	case GOK_SPY_SEARCH_CHILDREN:
		nodes = gok_spy_get_children (pKeyboard->pAccessible);
		break;
	case GOK_SPY_SEARCH_TABLE_CELLS:
		nodes = gok_spy_get_table_nodes (pKeyboard->pAccessible);
		break;
        case GOK_SPY_SEARCH_COMBO:
		/* sometimes a combobox has an editbox child which we should expose */
		editbox = gok_spy_get_editable (pKeyboard->pAccessible);
		/* fall-through... */
        case GOK_SPY_SEARCH_LISTITEMS:
		list_parent = gok_spy_get_list_parent (pKeyboard->pAccessible);
		if (list_parent) 
		{
			/* TODO: careful of MANAGES_DESCENDANTS here... 
			   may need function for 'visible children' */
			nodes = gok_spy_get_children (list_parent);
			gok_spy_accessible_unref (list_parent);
		}
		if (editbox) 
		{
			AccessibleNodeFlags flags;
			flags.value = 0;
			flags.data.is_ui = TRUE;
			nodes = gok_spy_append_node (nodes, editbox, flags);
			gok_spy_accessible_unref (editbox);
		}
		break;
	case GOK_SPY_SEARCH_ACTIONABLE:
		nodes =	gok_spy_get_actionable_descendants (pKeyboard->pAccessible, NULL);
		break;
	default:
		gok_log("calling get list with accessible [%#x]", pKeyboard->pAccessible);
		nodes =	gok_spy_get_list (pKeyboard->pAccessible);
		break;
	}

	
	/* add the new keys to the dynamic keyboard */
	/* first, add a 'back' key */
	pKey = gok_key_new (NULL, NULL, pKeyboard);
	pKey->Type = KEYTYPE_BRANCHBACK;
	pKey->Style = KEYSTYLE_BRANCHBACK;
	pKey->Top = 0;
	pKey->Bottom = 1;
	pKey->Left = 0;
	pKey->Right = 1;
	/* "back" means go to previous keyboard */
	gok_key_add_label (pKey, _("back"), 0, 0, NULL);

	pKeyPrevious = pKey;
		
	/* the keys will be repositioned in gok_keyboard_layout */
	pKeyboard->bLaidOut = FALSE;

	if (nodes == NULL)
	{
		gok_log_x ("Warning: no nodes found!");
		gok_log_leave();
		return TRUE;
		/* return FALSE; */
		/* we need a good (tested) fail gracefully here */
	}
	
	/* create all the gui keys as one long row */
	column = 1;

	while (nodes)
	{
	        pNodeAccessible = nodes->data;
		if (gok_spy_node_match (pNodeAccessible, pKeyboard->search_type)) {
			pKey = gok_key_new (pKeyPrevious, NULL, pKeyboard);
			pKeyPrevious = pKey;
			
			pKey->Style = KEYSTYLE_GENERALDYNAMIC;
			
			gok_log("node has role: [%s]",Accessible_getRoleName(pNodeAccessible->paccessible));

			pStateSet = Accessible_getStateSet (pNodeAccessible->paccessible);
			if (pKeyboard->search_type == GOK_SPY_SEARCH_MENU && 
			    pNodeAccessible->flags.data.has_context_menu) 
			{
				gok_log("setting key type for context menu key to BRANCHGUIACTIONS");
				pKey->Type = KEYTYPE_BRANCHGUIACTIONS;
				pKey->Style = KEYSTYLE_BRANCHGUIACTIONS;
			}
			else if ((pKeyboard->search_type == GOK_SPY_SEARCH_ACTIONABLE) && !did_actionkeys) 
			{
				gint action_count, i;
				did_actionkeys = TRUE;
				AccessibleAction *action = 
					Accessible_getAction (pNodeAccessible->paccessible);
				if (!action) 
					break;
				action_count = AccessibleAction_getNActions (action);
				for (i = 0; i < action_count; ++i)
				{
					gchar *action_name;
					if (i)
					{
						pKey = gok_key_new (pKeyPrevious, NULL, pKeyboard);
					}
#ifdef GOK_SHOW_ONLY_ACTIONS					
					else
					{
						AccessibleStateSet_unref (pStateSet);
					}
#endif /* GOK_SHOW_ONLY_ACTIONS */
					pKeyPrevious = pKey;					
					gok_log("setting key type for context menu key to BRANCHGUIACTIONS");
					pKey->Type = KEYTYPE_BRANCHGUIACTIONS;
					pKey->Style = KEYSTYLE_BRANCHGUIACTIONS;
					pKey->Top = 0;
					pKey->Bottom = 1;
					pKey->Left = column;
					pKey->Right = column + 1;
					pKey->action_ndx = i;
					gok_spy_accessible_ref (pNodeAccessible->paccessible);
					pKey->accessible_node = pNodeAccessible;
					action_name = AccessibleAction_getName (action, i);
					gok_key_add_label (pKey, action_name ? g_strdup (action_name) : g_strdup (""), 0, 0, NULL);
				}
#ifdef GOK_SHOW_ONLY_ACTIONS					
				break;
#endif /* GOK_SHOW_ONLY_ACTIONS */
				}
			else
			{
				switch (Accessible_getRole(pNodeAccessible->paccessible))
				{
					/*
					  case SPI_ROLE_INVALID:
					  gok_log_x("invalid role in accessible node list!");
					  pKey->Type = KEYTYPE_NORMAL;
					  break;
					*/
				case SPI_ROLE_MENU:
					gok_log("setting key type BRANCHMENUITEMS");
					pKey->Type = KEYTYPE_BRANCHMENUITEMS;
					pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHMENUS);
					break;
				case SPI_ROLE_CHECK_BOX:
				case SPI_ROLE_CHECK_MENU_ITEM:
				case SPI_ROLE_TOGGLE_BUTTON:
					pKey->Type = KEYTYPE_BRANCHGUIACTIONS;
					pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
					pKey->has_image = TRUE;
					pKey->pImage = gok_keyimage_new (pKey, NULL);
					pKey->pImage->type = IMAGE_TYPE_INDICATOR;
					is_active = AccessibleStateSet_contains (pStateSet, 
										 SPI_STATE_CHECKED);
					pKey->ComponentState.active = is_active;
					break;
				case SPI_ROLE_RADIO_BUTTON:
				case SPI_ROLE_RADIO_MENU_ITEM:
					pKey->Type = KEYTYPE_BRANCHGUIACTIONS;
					pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
					pKey->has_image = TRUE;
					pKey->pImage = gok_keyimage_new (pKey, NULL);
					pKey->pImage->type = IMAGE_TYPE_INDICATOR;
					is_active = AccessibleStateSet_contains (pStateSet, 
										 SPI_STATE_CHECKED);
					pKey->ComponentState.active = is_active;
					pKey->ComponentState.radio = TRUE;
					break;
				case SPI_ROLE_PAGE_TAB:
					/* no action implemented,, must use selection API on parent */
					pKey->Type = KEYTYPE_PAGESELECTION;
					pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_PAGESELECTION);
					break;
				case SPI_ROLE_SPIN_BUTTON:
				case SPI_ROLE_TEXT:
					pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
					if (pNodeAccessible->flags.data.is_link) {
						pKey->Type = KEYTYPE_HYPERLINK;
						pKey->Style = KEYSTYLE_HYPERLINK;
						is_active = FALSE;
					}
					else {
						/* should only be in the list if it's editable...*/
						pKey->Type = KEYTYPE_BRANCHTEXT;
						if (AccessibleStateSet_contains (pStateSet, SPI_STATE_EDITABLE)) { 
							pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHTEXT);
							is_active = TRUE;
						}
						else {
							is_active = FALSE;
							pKey->Style = KEYSTYLE_INSENSITIVE;
						}
					}
					pKey->ComponentState.active = is_active;
					break;
				case SPI_ROLE_HTML_CONTAINER: /* TODO: check Hypertext interface instead */
					pKey->Type = KEYTYPE_BRANCHHYPERTEXT;
					pKey->Style = KEYSTYLE_BRANCHHYPERTEXT; /* reuse 'normal' branch style for now. */
					is_active = AccessibleStateSet_contains (pStateSet, 
										 SPI_STATE_SENSITIVE);
					pKey->ComponentState.active = is_active;
					break;
				case SPI_ROLE_ICON:
					gok_log("setting key type for icon key to BRANCHGUIACTIONS");
					if (Accessible_isAction (pNodeAccessible->paccessible))
					{
						pKey->Type = KEYTYPE_BRANCHGUISELECTACTION;
						pKey->Style = gok_style_if_selectable (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
					}
					break;
				case SPI_ROLE_COMBO_BOX:
					gok_log ("setting key type for combobox key to BRANCHCOMBO");
					pKey->Type = KEYTYPE_BRANCHCOMBO;
					pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHMENUS);
					break;
				case SPI_ROLE_LIST_ITEM:
				case SPI_ROLE_TABLE_CELL:
					gok_log ("list item key!");
					pKey->Type = KEYTYPE_BRANCHGUISELECTACTION;
					/* TODO: create a list-item style or gui-selection style for these */
					pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_MENUITEM);
					break;
				default:
					if (Accessible_isHypertext (pNodeAccessible->paccessible)) {
						pKey->Type = KEYTYPE_BRANCHHYPERTEXT;
						pKey->Style = KEYSTYLE_BRANCH; /* reuse 'normal' branch style for now. */
						is_active = TRUE; /* FIXME, might not be enabled? */
						pKey->ComponentState.active = is_active;
					}
					else if (Accessible_isTable (pNodeAccessible->paccessible)) {
						pKey->Type = KEYTYPE_BRANCHGUITABLE;
						pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
					}
					else if (Accessible_isSelection (pNodeAccessible->paccessible)) {
						pKey->Type = KEYTYPE_BRANCHGUISELECTION;
						pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
					}
					/* since Java/StarOffice buttons expose Value (broken!) we must check Action first */
					else if (Accessible_isAction (pNodeAccessible->paccessible)) {
						gok_log("setting key type BRANCHGUIACTIONS");
						pKey->Type = KEYTYPE_BRANCHGUIACTIONS;
						pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
					}
					else if (Accessible_isValue (pNodeAccessible->paccessible)) {
						pKey->Type = KEYTYPE_BRANCHGUIVALUATOR;
						pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
					}
					else {
						/* this might be a dangerous catch all... */
						gok_log("setting key type BRANCHGUIACTIONS");
						if (list_parent)
						{
						    pKey->Type = KEYTYPE_BRANCHGUISELECTACTION;
						}
						else
						{
						    pKey->Type = KEYTYPE_BRANCHGUIACTIONS;
						}
						pKey->Style = gok_style_if_selectable (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
						/* for "generic" case we allow selectable but not enabled... */
						if (pKey->Style != KEYSTYLE_BRANCHGUIACTIONS) {
						    pKey->Style = gok_style_if_enabled (pStateSet, KEYSTYLE_BRANCHGUIACTIONS);
						    if (pKey->Style == KEYSTYLE_BRANCHGUIACTIONS) {
							pKey->Type = KEYTYPE_BRANCHGUIACTIONS;
						    }
						}
					}
					break;
				}
			}
			if (pNodeAccessible->flags.data.inside_html_container) {
				if (pKey->FontSizeGroup == FONT_SIZE_GROUP_UNDEFINED) {
					pKey->FontSizeGroup = FONT_SIZE_GROUP_CONTENT; 
				}
				if (pNodeAccessible->flags.data.is_link) {
						if (pKey->Style != KEYSTYLE_INSENSITIVE)
						    pKey->Style = KEYSTYLE_HYPERLINK;
				}
				else if (pKey->Style == KEYSTYLE_BRANCHGUIACTIONS) {
						pKey->Style = KEYSTYLE_HTMLACTION;
				}
			}
			AccessibleStateSet_unref (pStateSet);
			/* is the latest key an action key? (already initialized) */
			if (!pKey->action_ndx) {
				/* not an action key so we need to configure. */
				pKey->Top = 0;
				pKey->Bottom = 1;
				pKey->Left = column;
				pKey->Right = column + 1;
				gok_spy_accessible_ref (pNodeAccessible->paccessible);
				pKey->accessible_node = pNodeAccessible;
				
				gok_log("adding label %s to dynamic key",pNodeAccessible->pname);
				gok_key_add_label (pKey, pNodeAccessible->pname, 0, 0, NULL);
			}
			
			column++;
		}
		nodes = g_slist_next (nodes);
	}	

	gok_log_leave();
	return TRUE;
}

/**
* gok_keyboard_branch_gui
* @pNodeAccessible: Pointer to the accessible node (parent object).
* @type: Type of dynamic keyboard to branch to (the role of the children).
*
* Displays the generic gui keyboard - currently used for widgets inside windowish things
*
* Returns: TRUE if the keyboard was displayed, FALSE if not.
**/
gboolean 
gok_keyboard_branch_gui (AccessibleNode *pNodeAccessible, 
			 GokSpySearchType type)
{
	Accessible* pAccessibleRoot;
	GokKeyboard* pKeyboard;
	GokKeyboard* pKeyboardTemp;

	gok_log_enter();

	pAccessibleRoot = NULL;

	if (pNodeAccessible != NULL)
	{
		gok_log("using accessible from key as root accessible");
		pAccessibleRoot = pNodeAccessible->paccessible;
	}
	
	if (pAccessibleRoot == NULL)
	{
		gok_log("using accessible for the foregound application as root accessible");
		/* get the accessible interface for the foregound application */
		pAccessibleRoot = gok_main_get_foreground_window_accessible();
		
		if (pAccessibleRoot == NULL)
		{
			gok_log_x ("Warning: Can't create gui keyboard because foreground accessible is NULL!");
			gok_log_leave();
			return FALSE;
		}
	}

	/* create a new keyboard */
	pKeyboard = gok_keyboard_new();
	if (pKeyboard == NULL)
	{
		gok_log_leave();
		return FALSE;
	}
		
	/* mark this as a dynamically created keyboard */
	pKeyboard->bDynamicallyCreated = TRUE;
	
	/* store the accessible pointer on the keyboard */
	gok_keyboard_set_accessible(pKeyboard, pAccessibleRoot);

	/* add the new keyboard to the list of keyboards (at the end)*/
	pKeyboardTemp = gok_main_get_first_keyboard();
	g_assert (pKeyboardTemp != NULL);
	while (pKeyboardTemp->pKeyboardNext != NULL)
	{
		pKeyboardTemp = pKeyboardTemp->pKeyboardNext;
	}
	pKeyboardTemp->pKeyboardNext = pKeyboard;
	pKeyboard->pKeyboardPrevious = pKeyboardTemp;
	pKeyboard->search_type = type;

	/* set the name and type of the keyboard */
	switch (type)
	{
	case GOK_SPY_SEARCH_UI:
		pKeyboard->Type = KEYBOARD_TYPE_GUI;
		pKeyboard->flags.data.gui = 1;
		gok_keyboard_set_name (pKeyboard, _("GUI"));
		break;
	case GOK_SPY_SEARCH_TABLE_CELLS:
		pKeyboard->Type = KEYBOARD_TYPE_GUI;
		pKeyboard->flags.data.gui = 1;
		gok_keyboard_set_name (pKeyboard, _("Table"));
		break;
	case GOK_SPY_SEARCH_TOOLBARS:
		pKeyboard->Type = KEYBOARD_TYPE_TOOLBAR; 
		pKeyboard->flags.data.toolbars = 1;
		gok_keyboard_set_name (pKeyboard, _("Toolbars"));
		break;
	case GOK_SPY_SEARCH_APPLICATIONS:
		pKeyboard->Type = KEYBOARD_TYPE_APPLICATIONS;
		gok_keyboard_set_name (pKeyboard, _("Applications"));   
		break;
	case GOK_SPY_SEARCH_ACTIONABLE:
		pKeyboard->Type = KEYBOARD_TYPE_ACTIONS;
		gok_keyboard_set_name (pKeyboard, "Actions"); /* I18N TODO in HEAD, mark for xlation */
		break;
	case GOK_SPY_SEARCH_MENU:
	default:
		pKeyboard->Type = KEYBOARD_TYPE_MENUS;
		pKeyboard->flags.data.menus = 1;
		gok_keyboard_set_name (pKeyboard, _("Menu"));
		break;
	}
	
	/* set this flag so the keyboard will be laid out when it's displayed */
	pKeyboard->bLaidOut = FALSE;
	pKeyboard->bFontCalculated = FALSE;

	/* display and scan the dynamic keyboard */
	/* note: keys are added in gok_keyboard_update_dynamic which is */
	/* called by gok_main_display_scan */	
	gok_main_display_scan ( pKeyboard, pKeyboard->Name, 
		KEYBOARD_TYPE_UNSPECIFIED, KEYBOARD_LAYOUT_UNSPECIFIED, 
		KEYBOARD_SHAPE_UNSPECIFIED);
	
	gok_log_leave();
	return TRUE;
}

/**
* gok_keyboard_branch_gui_selectaction
* @node: the node which represents the gui widget
*
* Select the given child and invoke the first available action.
*
* returns: TRUE if we branched to a new keyboard, FALSE if not.
**/
static gboolean 
gok_keyboard_branch_gui_selectaction (GokKeyboard *keyboard, AccessibleNode* node, gint action_ndx)
{
	AccessibleSelection* aselection;
	Accessible *parent;
	AccessibleTable *table;
	gboolean retval = FALSE;
	gboolean selected = FALSE;

	parent = Accessible_getParent (node->paccessible);
	if (parent) 
	{
		gint index = Accessible_getIndexInParent (node->paccessible);
		aselection = Accessible_getSelection (parent);
		if (aselection) 
		{
			if ((index >= 0) && 
			    ((selected = AccessibleSelection_selectChild (aselection, index)) != FALSE))
			{
				if (gok_keyboard_branch_or_invoke_actions (keyboard, node, action_ndx))
				{
					gok_log ("SELECTACTION succeeded");
				    Accessible_unref (parent);
				    AccessibleSelection_unref (aselection);
				    return TRUE;
				}
			}
			AccessibleSelection_unref (aselection);
		}
		if (!selected)
		{
		    table = Accessible_getTable (parent);
		    if (table != NULL) 
		    {
			gint row = AccessibleTable_getRowAtIndex (table, index);
			if (row >= 0)
			{
			    AccessibleComponent *component = Accessible_getComponent (node->paccessible);
			    retval = AccessibleTable_addRowSelection (table, row);
			    gok_log ("row selection added; grabbing focus");
			    if (component)
			    {
				AccessibleComponent_grabFocus (component);
				AccessibleComponent_unref (component);
			    }
			    if (Accessible_isAction (node->paccessible))
			    {
				retval = gok_keyboard_branch_or_invoke_actions (keyboard, node, action_ndx);
			    }
			    else
			    {
				gok_keyboard_do_leaf_action (node->paccessible);
			    }
			}
			AccessibleTable_unref (table);
		    }
		}
		Accessible_unref (parent);
	}
	
	if (retval) 
	{
	    gok_log ("SELECTACTION branched");
	}
	return retval;
}

/* 
 * N.B. For speed, this method returns the 'potentially actionable' children 
 * e.g. it does not check for visibility/selectability/enabled
 */
static gint
gok_keyboard_get_actionable_child_count (Accessible *parent)
{
    gint child_count;
    gint max_actionable = 20;
    gint actionable_count = 0;
    g_assert (parent);
    child_count = Accessible_getChildCount (parent);
    if (child_count > 0) 
    {
	int i;
	/* treat selectable children as "actionable" for our purposes */
	if (Accessible_isSelection (parent) && child_count > 1)
	{
	    return child_count;
	}
	else
	{
	    for (i = 0; i < child_count && i < max_actionable; ++i)
	    {
		Accessible *child = Accessible_getChildAtIndex (parent, i);
		if (child && Accessible_isAction (child))
		{
		    ++actionable_count;
		}
		else 
		{
		    actionable_count += gok_keyboard_get_actionable_child_count (child);
		}
		Accessible_unref (child);
	    }
	    return actionable_count;
	}
    }
    else
    {
	return 0;
    }
}

static gboolean
gok_keyboard_do_leaf_action (Accessible *parent)
{
    Accessible *child;
    gint max_children = MAX_BREADTH_ACTION_LEAF_SEARCH;
    gint i, child_count = Accessible_getChildCount (parent);
    for (i = 0; i < child_count && i < max_children; ++i)
    {
	child = Accessible_getChildAtIndex (parent, i);
	if (Accessible_isAction (child))
	{
	    AccessibleAction *action = Accessible_getAction (child);
	    gchar *action_name = AccessibleAction_getName (action, 0);
	    gboolean retval;
	    gok_log ("invoking action %s", action_name);
	    retval = AccessibleAction_doAction (action, 0);
	    AccessibleAction_unref (action);
	    Accessible_unref (child);
	    return retval;
	}
	else if (gok_keyboard_do_leaf_action (child)) 
	{
	    return TRUE;
	}
	Accessible_unref (child);
    } 
    return FALSE;
}

/* helper */
static gboolean 
gok_keyboard_has_multi_useful_actions (Accessible* acc, AccessibleAction* action)
{
	gint nactions;
	gboolean retval = FALSE;
	
	g_assert (action);
	nactions = AccessibleAction_getNActions (action);
	if (nactions <= 1) {
	                retval = FALSE;
        }
	else if ((Accessible_getRole(acc) != SPI_ROLE_PUSH_BUTTON)) {
			retval = TRUE;
	}
	else {
		/* do the actions all reduce to click? */
		char* action_name = NULL;
		action_name = AccessibleAction_getName (action, 0);
		if (!action_name) { 
			gok_log_x ("Action has no name!");
			retval = TRUE; 
		}
		else if (strcmp (action_name, "click") != 0) {
			SPI_freeString (action_name);
			retval = TRUE; 
		}
		else {
			SPI_freeString (action_name);
			while (nactions > 1) {
				action_name = AccessibleAction_getName (action, 1);
				if (!action_name) { 
					gok_log_x ("Action has no name!");
					retval = TRUE; 
					break;
				}
				if ((strcmp (action_name, "press") != 0) && 
				(strcmp (action_name, "release") != 0)) {
					SPI_freeString (action_name);
					retval = TRUE; 
					break;
				}
				SPI_freeString (action_name);
				nactions--;
			}
		}
	}
	return retval;
}

/**
* gok_keyboard_branch_or_invoke_actions 
* @node: the AccessibleNode which represents the gui widget
*
* If the component associated with @Accessible has only one action, and
* no actionable children, invoke the singleton, otherwise build a keyboard 
* showing action(s) and/or actionable children.
*
* returns: TRUE if we branch here, false if we do not (i.e. if we invoke instead).
**/
static gboolean 
gok_keyboard_branch_or_invoke_actions (GokKeyboard *keyboard, AccessibleNode *node, gint action_ndx)
{
    gboolean retval = FALSE;
    AccessibleAction *action;
    gchar *action_name;

    g_assert (keyboard);
    g_assert (node);
    g_assert (node->paccessible);

    action = Accessible_getAction (node->paccessible);
    if (action) 
    {
	if (keyboard && (keyboard->search_type == GOK_SPY_SEARCH_ACTIONABLE))
	{
	    action_name = AccessibleAction_getName (action, action_ndx);
	    gok_log ("invoking action %s", action_name);
	    retval = AccessibleAction_doAction (action, action_ndx);
	}
	else if ( gok_keyboard_has_multi_useful_actions (node->paccessible, action) ||
		 gok_keyboard_get_actionable_child_count (node->paccessible))
	{
	    /* 
	     * branch, don't invoke : note that we don't set retval here, 
	     * as a branch isn't the same as invocation 
	     */
	    gok_keyboard_branch_gui (node, GOK_SPY_SEARCH_ACTIONABLE);
	}
	else
	{
	    action_name = AccessibleAction_getName (action, action_ndx);
	    gok_log ("invoking action %s", action_name);
	    retval = AccessibleAction_doAction (action, action_ndx);
	}
	AccessibleAction_unref (action);
    }
    return retval;
}

/**
* gok_keyboard_branch_gui_actions
* @pNodeAccessible: the node which represents the gui widget
*
* Widgets can have multiple actions - build a keyboard of them.
*
* returns: TRUE if the keyboard was displayed, FALSE if not.
**/
gboolean 
gok_keyboard_branch_gui_actions (GokKeyboard *keyboard, AccessibleNode* node, gint action_ndx)
{
	AccessibleAction* paaction, *child_action;
	Accessible* parent = NULL;
	AccessibleStateSet *stateset = NULL;
	gint i = 0;
	gboolean branched = FALSE;
	paaction = NULL;
		
	gok_log_enter();

	g_assert (keyboard);
	g_assert(node != NULL);

	parent = Accessible_getParent (node->paccessible);
	/* handle the "selection" case: always attempt to select the current item */
	if (Accessible_isSelection (node->paccessible) || 
         Accessible_isSelection (parent))
	{
	    branched = gok_keyboard_branch_gui_selectaction (keyboard, node, action_ndx);
	}
	if (parent)
         Accessible_unref (parent);
	
	/* Editable text fields: branch to the composer if we've successfully invoked an action */
	if (!branched && Accessible_isEditableText (node->paccessible) &&
	    ((stateset = Accessible_getStateSet (node->paccessible)) != NULL) &&
	    (AccessibleStateSet_contains (stateset, SPI_STATE_EDITABLE)) &&
	    (gok_keyboard_focus_object (node->paccessible)))
	{
	    gok_log ("branching to Compose kbd...\n");

	    branched = gok_main_display_scan (gok_keyboard_get_compose (), "Keyboard", 
					      KEYBOARD_TYPE_UNSPECIFIED,
					      KEYBOARD_LAYOUT_UNSPECIFIED, 
					      KEYBOARD_SHAPE_UNSPECIFIED);
	}
	else if (!branched)
	{
	     branched = gok_keyboard_branch_or_invoke_actions (keyboard, node, action_ndx);
	}

	if (stateset)
	    AccessibleStateSet_unref (stateset);

	/* branch back when a menu item is activated*/
	if (!branched && gok_spy_is_menu_role(Accessible_getRole(node->paccessible))){
		gok_log_leave();
		return gok_main_display_scan_previous();
	}
	
	gok_log_leave();
	return branched;	
}


/**
* gok_keyboard_branch_gui_valuator:
* @pNodeAccessible: the node thich represents the gui widget
*
* Branch to a keyboard for controlling a valuator.
*
* returns: TRUE if the keyboard was displayed, FALSE if not.
**/
gboolean 
gok_keyboard_branch_gui_valuator (AccessibleNode* node)
{
	AccessibleValue* value;
	AccessibleComponent *component;
	GokKeyboard *valuator_kbd;
	gok_log_enter();

	/* for now, just grab focus; from there compose+keynav is quicker than multiple tab traversal */

        component = Accessible_getComponent (node->paccessible);
	if (component) 
	{
		AccessibleComponent_grabFocus (component);
		AccessibleComponent_unref (component);
	}

	g_assert(node != NULL);

	gok_log_leave();
	
	valuator_kbd = gok_main_keyboard_find_byname ("valuator");
	if (valuator_kbd->pAccessible != NULL)
	{
		gok_spy_accessible_unref (valuator_kbd->pAccessible);
	}
	if (node->paccessible && Accessible_isValue (node->paccessible)) 
	{
		gok_spy_accessible_ref (node->paccessible);
		valuator_kbd->pAccessible = node->paccessible;
	}
	else
	{
		valuator_kbd->pAccessible = NULL;
	}
	gok_main_display_scan ( valuator_kbd, "valuator", KEYBOARD_TYPE_UNSPECIFIED,
				KEYBOARD_LAYOUT_UNSPECIFIED, KEYBOARD_SHAPE_UNSPECIFIED);
	return TRUE;	
}


gboolean 
gok_keyboard_branch_editableTextAction (GokKeyboard* pKeyboard, GokKey* pKey)
{
	pKeyboard->pAccessible = gok_spy_get_accessibleWithText ();
	if (pKeyboard->pAccessible) {
		Accessible_ref (pKeyboard->pAccessible);
	}
	return gok_composer_branch_textAction (pKeyboard, pKey);
}


/**
* gok_keyboard_layout
* @pKeyboard: Pointer to the keyboard that is getting laid out.
* @layout: Can be used to specify a layout for the keys
* @shape: Can be used to specify a shape of the keyboard window
* @force: If TRUE, perform a layout even if performed previously
*
* Arranges the keys on the keyboard.
* Predefined keyboards are already laid out. Runtime keyboards require this.
*
* returns: TRUE if the keyboard was laid out, FALSE if not.
**/
gboolean 
gok_keyboard_layout (GokKeyboard* pKeyboard, KeyboardLayouts layout, KeyboardShape shape, gboolean force)
{
	PangoLayout* pPangoLayout;
	PangoRectangle rectInk;
	PangoRectangle rectLogical;
	GokKey* pKey;
	GokKey* pKeyPrevious;
	GtkLabel* pLabel;
	gint maxTextPerCell;
	gint maxCellsRequired;
	gint totalKeys;
	gint totalCells;
	gint maxKeysPerRow;
	gint maxCellsPerRow;
	gint row;
	gint column;
	gint countKeys;
	gint widthFactor;
	gint diag;
	
	if (!force) {
		if ((pKeyboard->bRequiresLayout == FALSE) || 
			(pKeyboard->bLaidOut == TRUE))
			return TRUE;
	}

	pLabel = (GtkLabel*)gtk_label_new ("");
	totalKeys = 0;
	totalCells = 0;
	maxCellsRequired = 1;
	
	/* calculate all the cells required for this keyboard */
	pKey = pKeyboard->pKeyFirst;
	if (pKey) {
		/* maximum size of text per cell */
		maxTextPerCell = gok_data_get_key_width() - 
			gok_key_get_default_border_width (pKey);
	}

	while (pKey != NULL)
	{
		/* create a label using the key text*/
		gtk_label_set_text (pLabel, gok_key_get_label (pKey));

		/* get the size of the text in the label */
		pPangoLayout = gtk_label_get_layout (pLabel);
		pango_layout_get_pixel_extents (pPangoLayout, &rectInk, &rectLogical);
		
		/* calculate the cells required for this label */
		pKey->CellsRequired = (rectInk.width / maxTextPerCell) + 1;

		totalCells += pKey->CellsRequired;
		if (pKey->CellsRequired > maxCellsRequired)
		{
			maxCellsRequired = pKey->CellsRequired;
		}
		
		totalKeys++;
		
		pKey = pKey->pKeyNext;
	}

	
	row = 0;
	column = 0;
	countKeys = 0;
	widthFactor = 1;

	switch (shape) {
		case KEYBOARD_SHAPE_WIDE:
			widthFactor = 4;
		case KEYBOARD_SHAPE_SQUARE:
			maxCellsPerRow = (int)sqrt (totalCells) * widthFactor;
			if (totalCells - (maxCellsPerRow * maxCellsPerRow) > 0)
			{
				maxCellsPerRow++;
			}
			/* make sure the keyboard will hold the longest key */
			if (maxCellsRequired > maxCellsPerRow)
			{
				maxCellsPerRow= maxCellsRequired;
			}
			/* assign a row and column to each key */
			pKey = pKeyboard->pKeyFirst;
			pKeyPrevious = pKey;
			while (pKey != NULL) {
				pKey->Top = row;
				pKey->Bottom = row + 1;
				
				pKey->Left = column;
				column += pKey->CellsRequired;
				pKey->Right = column;
						
				if (column > maxCellsPerRow)
				{
					pKeyPrevious->Right = maxCellsPerRow;
					row++;
					column = 0;
					pKey->Top = row;
					pKey->Bottom = row + 1;
					pKey->Left = column;
					column += pKey->CellsRequired;
					pKey->Right = column;
				}
				else if (column == maxCellsPerRow)
				{
					row++;
					column = 0;
				}
				
				pKeyPrevious = pKey;
				pKey = pKey->pKeyNext;
			}
			/* make the last key fill the last row */
			pKeyPrevious->Right = maxCellsPerRow;
			break;
			
		case KEYBOARD_SHAPE_KEYSQUARE:
			if (layout == KEYBOARD_LAYOUT_UPPERL) {
				/* FIXME - we assume this is the frequency case */
				maxKeysPerRow = (gint)sqrt (totalKeys) + 1;
				pKey = pKeyboard->pKeyFirst;
				/* fill top left triangle of keyboard */
				for (diag=0; diag <= maxKeysPerRow; diag++) {
					for (row=0; row <= diag; row++) {
						if (pKey != NULL) {
							column = diag - row;
							pKey->Top = row;
							pKey->Bottom = row + 1;
							pKey->Left = column;
							pKey->Right = column + 1;
							pKey = pKey->pKeyNext;
						}
					}
				}
				/* fill bottom right triangle of keyboard */
				for (diag=1; diag < maxKeysPerRow; diag++) {
					for (row = diag; row < maxKeysPerRow; row++) {
						if (pKey != NULL) {
							column = maxKeysPerRow - row + diag;
							pKey->Top = row;
							pKey->Bottom = row + 1;
							pKey->Left = column;
							pKey->Right = column + 1;
							pKey = pKey->pKeyNext;
						}
					}
				}
				/* FIXME - now bang out required cells */
				break;
			}
			else ; /* fall through */
		default:
			maxKeysPerRow = (gint)sqrt (totalKeys);
			if (totalKeys - (maxKeysPerRow * maxKeysPerRow) > 0) {
				maxKeysPerRow++;
			}
			/* assign a row and column to each key */
			pKey = pKeyboard->pKeyFirst;
			while (pKey != NULL) {
				pKey->Top = row;
				pKey->Bottom = row + 1;
				pKey->Left = column;
				column += pKey->CellsRequired;
				pKey->Right = column;
		
				countKeys++;
				if (countKeys >= maxKeysPerRow) {
					countKeys = 0;
					row++;
					column = 0;
				}
				
				pKey = pKey->pKeyNext;
			}
			break;
	}

	/* set the number of rows and columns on the keyboard */
	gok_keyboard_count_rows_columns (pKeyboard);
	
	/* fill any empty space at the end of the rows */
	for (row = 0; row < pKeyboard->NumberRows; row++)
	{
		gok_keyboard_fill_row (pKeyboard, row);
	}
	
	/* keyboard is now laid out */
	pKeyboard->bLaidOut = TRUE;
	
/* causes hang - but where is this freed? */	
/*	free (pLabel);*/
		
	return TRUE;
}

/* this array is used to order the keys */
static GokKey* arrayGokKeyPointers[300]; /* largest number of keys in a row */
/**
* gok_keyboard_fill_row
* @pKeyboard: Pointer to the keyboard that contains the row.
* @RowNumber: Number of the row you want filled.
*
* This function resizes the keys in the given row so they fill the entire row.
* This should be used only on keyboards that are synamically created (not predefined).
**/
void gok_keyboard_fill_row (GokKeyboard* pKeyboard, gint RowNumber)
{
	GokKey* pKey;
	gint countColumns;
	gint countKeys;
	gint extraColumns;
	gint perKeyExtraColumns;
	gint x;
	
	g_assert (pKeyboard != NULL);
	g_assert (RowNumber >= 0);
	g_assert (RowNumber < 100);
	
	/* count the number of columns and keys in the target row */
	countColumns = 0;
	countKeys = 0;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey->Top == RowNumber)
		{
			countColumns += (pKey->Right - pKey->Left);
			countKeys ++;
		}
		pKey = pKey->pKeyNext;
	}
	
	if (countColumns >= pKeyboard->NumberColumns)
	{
		return;
	}
	
	/* calculate the number of columms that get added to each key */
	extraColumns = pKeyboard->NumberColumns - countColumns;
	perKeyExtraColumns = extraColumns / countKeys;
	if (perKeyExtraColumns < 1)
	{
		perKeyExtraColumns = 1;
	}
	
	/* make a list of the keys in order of right to left */
	arrayGokKeyPointers[0] = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey->Top == RowNumber)
		{
			gok_keyboard_insert_array (pKey);
		}
		pKey = pKey->pKeyNext;
	}

	/* add extra columns to keys until there are no more extra columns */
	for (x = 0; x < 300; x++)
	{
		if (arrayGokKeyPointers[x] == NULL)
		{
			break;
		}
		
		arrayGokKeyPointers[x]->Right += extraColumns;
		extraColumns -= perKeyExtraColumns;
		if (arrayGokKeyPointers[x + 1] == NULL)
		{
			arrayGokKeyPointers[x]->Left = 0;
		}
		else
		{			
			arrayGokKeyPointers[x]->Left += extraColumns;
		}
		
		if (extraColumns <= 0)
		{
			break;
		}
	}
	
	if (arrayGokKeyPointers[0] != NULL)
	{
		arrayGokKeyPointers[0]->Right = pKeyboard->NumberColumns;
	}
}

/**
* gok_keyboard_insert_array
* @pKey: Pointer to the key you want added to the array.
*
* Adds the given key to our array in order of the rightmost key location.
**/
void gok_keyboard_insert_array (GokKey* pKey)
{
	GokKey* pKeyTemp;
	gint x;
	
	for (x = 0; x < 300; x++)
	{
		if (arrayGokKeyPointers[x] == NULL)
		{
			arrayGokKeyPointers[x] = pKey;
			arrayGokKeyPointers[x + 1] = NULL;
			return;
		}
		
		/* if you want the array in left-to-right order then change '<' to '>' here */
		if (arrayGokKeyPointers[x]->Left < pKey->Left)
		{
			pKeyTemp = arrayGokKeyPointers[x];
			arrayGokKeyPointers[x] = pKey;
			pKey = pKeyTemp;
		}
	}
}

static gboolean
gok_keyboard_focus_object (Accessible *accessible)
{
	gboolean retval = FALSE;
	AccessibleComponent *component;

	gok_log ("Attempting to focus object %p :", accessible);

	if (accessible) {
		component = Accessible_getComponent (accessible);
		if (component) {
			retval = AccessibleComponent_grabFocus (component);
			AccessibleComponent_unref (component);
		}
	}

	gok_log ("%s\n", (retval) ? "succeeded" : "failed");

	return retval;
}

/* FIXME: this method is a hack necessitated by the fact that link objects aren't
 * actionable" as they should be, i.e. they don't implement AccessibleAction. 
 * When that's fixed, we should be able to do away with this kludge
 * (which manually focussed the text object, places the cursor, and synthesizes
 * a spacebar key!)
 */
static gboolean
gok_keyboard_follow_link (GokKey *key) 
{
	AccessibleHypertext *hypertext = NULL; 
	AccessibleText *text = NULL;
	AccessibleNode *node = key->accessible_node;
	AccessibleHyperlink *link = NULL;
	gboolean is_editable = FALSE;
	gboolean activated = FALSE;
	if (node && node->paccessible) {
		hypertext = Accessible_getHypertext (node->paccessible);
		if (hypertext) {
			Accessible *target = NULL;
			AccessibleAction *action = NULL;
			gboolean is_action = FALSE;
			if (node->link >= 0) {
				link = AccessibleHypertext_getLink (hypertext, node->link);
			}
			if (link) {
				target = AccessibleHyperlink_getObject (link, 0);
				if (target) 
					action = Accessible_getAction (target);
			}
			if (!action && link) { /* try the parent object? */
				action = Accessible_getAction (link);
				if (action) gok_log_x ("activating the link object.\n");
				else gok_log_x ("link not actionable.\n");
			}
			if (action) {
				activated = AccessibleAction_doAction (action, 0);
				/* TODO: should check the action name first */
				gok_log ("activating link\n");
				AccessibleAction_unref (action);
			}
			if (target) {
				Accessible_unref (target);
			}
		}
		if (!activated) { /* fallback hack if text is focussable */
			text = Accessible_getText (node->paccessible);
			is_editable = Accessible_isEditableText (node->paccessible);
			if (link && text && node->flags.data.is_link && !is_editable) {
				gboolean is_focussed = FALSE;
				long int start, end;
				AccessibleHyperlink_getIndexRange (link, &start, &end);
				
				/* can't activate editable text this way, it will insert a space instead! */
				is_focussed = gok_keyboard_focus_object (node->paccessible);
				if (!is_focussed) gok_log_x ("Could not focus object\n");
				else {
					AccessibleText_setCaretOffset (text, start);
					SPI_generateKeyboardEvent ((long) XK_space, NULL, SPI_KEY_SYM);
					activated = TRUE;
				}
				AccessibleText_unref (text);
			}
		}
		if (link) 
			AccessibleHyperlink_unref (link);
		if (hypertext)
			AccessibleHypertext_unref (hypertext);
	}
	return activated;
}


/* TODO: merge with duplicate code in gok-settings-dialog.c */

/**
 * gok_keyboard_help:
 *
 * @pKey: pointer to the invoking GokKey structure. 
 *
 * Displays the GOK Help text in the gnome-help browser.
 *
 **/
void
gok_keyboard_help (GokKey *pKey) 
{
	GError *error = NULL;
	/* TODO: detect error launching help, and give informative message */
	gnome_help_display_desktop (NULL, "gok", "gok.xml", NULL, &error);
}

/**
 * gok_keyboard_about:
 *
 * @pKey: pointer to the invoking GokKey structure. 
 *
 * Displays the GOK About window.
 *
 **/
void
gok_keyboard_about (GokKey *pKey) 
{
	static GtkWidget *about = NULL, *hbox = NULL;
	GdkPixbuf	 *pixbuf = NULL;
	GError		 *error = NULL;
	gchar		 *file = NULL;
	
	
	static const gchar *authors [] = {
		"David Bolter <david.bolter@utoronto.ca>",
		"Bill Haneman <bill.haneman@sun.com>",
		"Chris Ridpath <chris.ridpath@utoronto.ca>",
		"Simon Bates  <simon.bates@utoronto.ca>",
		"Gnome Accessibility <gnome-accessibility-devel@gnome.org>",
		NULL
	};
	
	const gchar *documenters[] = {
		NULL
	};
	
	const gchar *translator_credits = _("translator_credits");
	
	if (about) {
		gtk_window_set_screen (
			GTK_WINDOW (about),
			gtk_widget_get_screen (GTK_WIDGET (gok_main_get_main_window ())));
		gtk_window_present (GTK_WINDOW (about));
		return;
	}
		
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_DATADIR, 
					  "gok.png", 
					  FALSE, NULL);
	if (file) {
		pixbuf = gdk_pixbuf_new_from_file (file, &error);
		if (error) {
		    g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
		    g_error_free (error);
		}
		g_free (file);
	}
	
	about = gnome_about_new (
		_("GOK"), VERSION,
		"Copyright (C) 2001-2003 Sun Microsystems, Copyright (C) 2001-2003 University of Toronto", 
		_("Dynamic virtual keyboards for the GNOME desktop"),
		authors,
		documenters,
		strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
		pixbuf);
	
	/* stealing a hack from gnumeric */
	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox),
		gnome_href_new ("http://www.gok.ca/credits.html", _("Full Credits")),
		FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (about)->vbox),
				hbox, TRUE, FALSE, 0);
	gtk_widget_show_all (hbox);
		
	if (pixbuf)
		gdk_pixbuf_unref (pixbuf);
			
	gtk_window_set_wmclass (GTK_WINDOW (about), "gok", "GOK");
	gtk_window_set_screen (GTK_WINDOW (about),
				   gtk_widget_get_screen (GTK_WIDGET (gok_main_get_main_window ())));
	g_signal_connect (about, "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &about);
	/* make it 'normal' to avoid occluding GOK main window */
	gtk_window_set_type_hint (GTK_WINDOW (about), GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_window_set_position (GTK_WINDOW (about), GTK_WIN_POS_CENTER);
	gtk_widget_show (about);
}

/**
 * gok_keyboard_dock:
 *
 * @pKey: pointer to the GokKey structure whose activation
 *        should relocate the GokKeyboard's docking position 
 *        onscreen.
 *
 * Docks the GOK keyboard to the top or bottom of the screen,
 *        or "floats" the keyboard if "none" is specified as the
 *        docking direction.
 *
 **/
void
gok_keyboard_dock (GokKey *pKey) 
{
	GokKeyboardDirection dir = GOK_DIRECTION_NONE;
	GokDockType old_type = gok_data_get_dock_type ();
	gint top = 0, bottom = 0;

	if (pKey && pKey->pGeneral) 
		dir = *(GokKeyboardDirection *) pKey->pGeneral;
	switch (dir)
	{
	case GOK_DIRECTION_N:
		gok_data_set_dock_type (GOK_DOCK_TOP);
		break;
	case GOK_DIRECTION_S:
		gok_data_set_dock_type (GOK_DOCK_BOTTOM);
		break;
	default:
		gok_main_update_struts (0, 0, 0, 0);
		gok_data_set_dock_type (GOK_DOCK_NONE);
	        break;
	}
	if (dir == GOK_DIRECTION_NONE) {
		GtkWidget *widget = gok_main_get_main_window ();
		if (widget) 
			gdk_window_raise (widget->window); 
/* FIXME: why do we need this? */ 
		gtk_widget_show_now (gok_main_get_main_window ());
	}
}

static gdouble
gok_keyboard_round_epsilon (gdouble range)
{
	gdouble epslog = log10 (range);
	gdouble epslogfloor = floor (epslog);
	
	return 0.01 * pow (10.0, epslogfloor) * ceil ((epslog - epslogfloor) * 10 + 0.001);
}
/**
 * gok_keyboard_add_word:
 *
 * @pKey: pointer to the GokKey structure whose activation
 *        should add a word to the word prediction dictionary.
 *
 * Adds the word displayed on the specified key to the word completion dictionary.
 *
 **/
void
gok_keyboard_add_word (GokKeyboard *keyboard, GokKey *pKey) 
{
    gchar *word = pKey->Target; /* we overload the meaning of 'target' somewhat */
    if (word)
	gok_wordcomplete_add_new_word (gok_wordcomplete_get_default (), word);
}

/**
 * gok_keyboard_modify_value:
 *
 * @pKey: pointer to the GokKey structure whose activation
 *        should modify the currently specified valuator.
 *
 * Changes the value in an object which implements AccessibleValue,
 * according to the value of the 'GokValueOp' data in 'pKey->pGeneral'.
 *
 **/
void
gok_keyboard_modify_value (GokKeyboard *keyboard, GokKey *pKey) 
{
	AccessibleValue *value;
	if (keyboard && keyboard->pAccessible && (value = Accessible_getValue (keyboard->pAccessible)))
	{
		GokKeyboardValueOp *op = (GokKeyboardValueOp *) pKey->pGeneral;
		gdouble range = AccessibleValue_getMaximumValue (value) - AccessibleValue_getMinimumValue (value);
		gdouble epsilon = gok_keyboard_round_epsilon (range);
		if (op) 
		{
			switch (*op)
			{
			case GOK_VALUE_LESS:
				AccessibleValue_setCurrentValue (value, AccessibleValue_getCurrentValue (value)
					- epsilon);
				break;
			case GOK_VALUE_MORE:
				AccessibleValue_setCurrentValue (value, AccessibleValue_getCurrentValue (value)
					+ epsilon);
				break;
			case GOK_VALUE_MUCH_LESS:
				AccessibleValue_setCurrentValue (value, AccessibleValue_getCurrentValue (value)
					- epsilon * 5);
				break;
			case GOK_VALUE_MUCH_MORE:
				AccessibleValue_setCurrentValue (value, AccessibleValue_getCurrentValue (value)
					+ epsilon * 5);
				break;
			case GOK_VALUE_MIN:
				AccessibleValue_setCurrentValue (value, AccessibleValue_getMinimumValue (value));
				break;
			case GOK_VALUE_MAX:
				AccessibleValue_setCurrentValue (value, AccessibleValue_getMaximumValue (value));
				break;
			default:
				break;
			}
		}
		AccessibleValue_unref (value);
	}
	else
	{
		g_warning ("Attempting to set a value on something that doesn't implement AccessibleValue");
	}
}

/**
 * gok_keyboard_move_resize:
 *
 * @pKey: pointer to the GokKey structure whose activation
 *        should relocate the GokKeyboard's default position 
 *        onscreen.
 *
 * Moves the GOK keyboard some distance, or resizes its keys, 
 * according to the value of the 'direction' data in 'pKey->pGeneral'.
 *
 **/
void
gok_keyboard_move_resize (GokKey *pKey) 
{
	GokKeyboardDirection dir = GOK_DIRECTION_NONE;
	GtkWidget *window = gok_main_get_main_window ();
	gboolean resize = FALSE;
	gint x, y;

	gok_log_enter();
	
	if (pKey && pKey->pGeneral) 
		dir = *(GokKeyboardDirection *) pKey->pGeneral;
	gtk_window_get_position (GTK_WINDOW (window), &x, &y);
	switch (dir)
	{
	case GOK_DIRECTION_NE:
		y -= 10;
	case GOK_DIRECTION_E:
		x += 10;
		break;
	case GOK_DIRECTION_NW:
		x -= 10;
	case GOK_DIRECTION_N:
		y -= 10;
		break;
	case GOK_DIRECTION_SW:
		y += 10;
	case GOK_DIRECTION_W:
		x -= 10;
		break;
	case GOK_DIRECTION_SE:
		x += 10;
	case GOK_DIRECTION_S:
		y += 10;
		break;
	case GOK_DIRECTION_FILL_EW:
		/* toggle expand */
		gok_data_set_expand (!gok_data_get_expand ());
		break;
	case GOK_RESIZE_NARROWER:
		x = gok_data_get_key_width ();
		x -= 2;
		gok_data_set_key_width (MAX (0, x));
		resize = TRUE;
		break;
	case GOK_RESIZE_WIDER:
		gok_data_set_key_width (gok_data_get_key_width () + 2); 
		resize = TRUE;
		break;
	case GOK_RESIZE_SHORTER:
		y = gok_data_get_key_height ();
		y -= 2;
		gok_data_set_key_height (MAX (0, y));
		resize = TRUE;
		break;
	case GOK_RESIZE_TALLER:
		gok_data_set_key_height (gok_data_get_key_height () + 2);
		resize = TRUE;
		break;
	default:
	        break;
	}
	if (resize) {
		gok_keyboard_display (gok_main_get_current_keyboard (),
				      gok_main_get_current_keyboard (), 
				      gok_main_get_main_window (), TRUE);
	}
	else {
		gtk_window_move (GTK_WINDOW (window), x, y);
	}
	gok_log_leave();
}

/**
* gok_keyboard_output_selectedkey
*
* Performs the events associated with the currently selected key
*
* returns: Always 0.
**/
GokKey* gok_keyboard_output_selectedkey (void)
{
	GokKey* pKeySelected = NULL;
	
	gok_log_enter();

/* FIXME: figure out why we need this kludge. (refactor?) */	
	if (strcmp ("directed", gok_data_get_name_accessmethod()) == 0) {
		gok_feedback_set_selected_key(gok_feedback_get_highlighted_key());
	}

	/* get the key selected */
	pKeySelected = gok_feedback_get_selected_key();
	
	/* is a key selected? */
	if (pKeySelected == NULL)
	{
		gok_log ("Currently selected key is NULL!");
		gok_log_leave();
		return pKeySelected;
	}
	
	
	gok_log_leave();
	return gok_keyboard_output_key(gok_main_get_current_keyboard (), pKeySelected);
}

/**
* gok_keyboard_output_key
* @pKeySelected: Pointer to the key that will be output.
*
* Synthesize the keyboard output.
* Possible side effects: makes sound, word completion prediction update.
**/
GokKey* gok_keyboard_output_key(GokKeyboard *keyboard, GokKey* pKey)
{
	GokOutput    delim_output;
	GokKeyboard* pPredictedKeyboard;
	GokKeyboard* pKeyboardTemp;
	GokKey* pKeyTemp;
	const gchar *str;
	
	gok_log_enter();

	g_assert (pKey != NULL);

	/* ignore disabled keys */
	if (strcmp (gtk_widget_get_name (pKey->pButton), "StyleButtonDisabled") == 0)
	{
		gok_log_leave();
		return NULL;
	}

	switch (pKey->Type)
	{
		case KEYTYPE_BRANCHEDIT: 
			if (! gok_spy_get_accessibleWithText ()) {
				gok_log_x ("object isn't a text object, can't branch to compose");
				gok_log_leave ();
				return NULL;
			}
			gok_composer_validate (pKey->Target, gok_spy_get_accessibleWithText ());
			/* else fall-through */
		case KEYTYPE_BRANCH:
			gok_main_display_scan ( NULL, pKey->Target, 
				KEYBOARD_TYPE_UNSPECIFIED, KEYBOARD_LAYOUT_UNSPECIFIED, 
				KEYBOARD_SHAPE_UNSPECIFIED);
			break;

		case KEYTYPE_BRANCHCOMPOSE: 
			gok_main_display_scan ( gok_keyboard_get_compose (), NULL, 
				KEYBOARD_TYPE_UNSPECIFIED, KEYBOARD_LAYOUT_UNSPECIFIED, 
				KEYBOARD_SHAPE_UNSPECIFIED);
			break;
		    
		case KEYTYPE_BRANCHMODAL:
			gok_main_display_scan ( NULL, pKey->Target, 
				KEYBOARD_TYPE_MODAL, KEYBOARD_LAYOUT_UNSPECIFIED, 
				KEYBOARD_SHAPE_FITWINDOW);
			break;

		case KEYTYPE_REPEATNEXT:
			if (pKey->pButton && GTK_IS_TOGGLE_BUTTON (pKey->pButton))
			{
			      gok_repeat_toggle_armed (pKey);
			}
			pKey = NULL; /* useful hack */
			break;
				
		case KEYTYPE_BRANCHWINDOWS:
			gok_windowlister_show();
			break;
			
		case KEYTYPE_WINDOW:
			gok_windowlister_onKey(pKey);
			break;

		case KEYTYPE_MOUSE:
		case KEYTYPE_MOUSEBUTTON:
			gok_mouse_control (pKey);
			break;

		case KEYTYPE_BRANCHBACK:
		case KEYTYPE_BRANCHMENUS:
		case KEYTYPE_BRANCHTOOLBARS:
		case KEYTYPE_BRANCHCOMBO:
		case KEYTYPE_BRANCHGUI:
		case KEYTYPE_BRANCHHYPERTEXT:
			gok_keyboard_branch_byKey (gok_main_get_current_keyboard (), pKey);
			break;

		case KEYTYPE_BRANCHALPHABET:
			gok_main_display_scan ( gok_keyboard_get_compose (), CORE_KEYBOARD, 
				KEYBOARD_TYPE_UNSPECIFIED, KEYBOARD_LAYOUT_UNSPECIFIED,
				KEYBOARD_SHAPE_UNSPECIFIED);
			break;

		case KEYTYPE_BRANCHTEXT:
			if (pKey->accessible_node && 
				pKey->accessible_node->paccessible) {
					if (gok_keyboard_focus_object 
						(pKey->accessible_node->paccessible))
					{
						gok_main_display_scan ( gok_keyboard_get_compose (), 
									"Keyboard", 
									KEYBOARD_TYPE_UNSPECIFIED, 
									KEYBOARD_LAYOUT_UNSPECIFIED,
									KEYBOARD_SHAPE_UNSPECIFIED);
					}
			}
			break;

		case KEYTYPE_SETTINGS:
			gok_settingsdialog_show();
			break;
				
		case KEYTYPE_COMMANDPREDICT:
		        return gok_keyboard_output_key (gok_main_get_current_keyboard (),
							pKey->pMimicKey);
			break;
			
		case KEYTYPE_POINTERCONTROL:	
			gok_data_set_drive_corepointer (
			!gok_data_get_drive_corepointer ());
			gok_main_set_cursor (NULL);
			break;

		case KEYTYPE_MOVERESIZE:	
			gok_keyboard_move_resize (pKey);
			break;

		case KEYTYPE_DOCK:	
			gok_keyboard_dock (pKey);
			break;

		case KEYTYPE_HELP:	
			gok_keyboard_help (pKey);
			break;

		case KEYTYPE_ABOUT:	
			gok_keyboard_about (pKey);
			break;

		case KEYTYPE_HYPERLINK:
		        gok_keyboard_follow_link (pKey);
			break;

		case KEYTYPE_VALUATOR:
		        gok_keyboard_modify_value (keyboard, pKey);
			break;

		case KEYTYPE_ADDWORD:
		        gok_keyboard_add_word (keyboard, pKey);
			break;

		case KEYTYPE_WORDCOMPLETE:
			/* output any modifier keys */
			gok_modifier_output_pre();
			
			/* send the key output to the system */
			gok_output_send_to_system (gok_key_wordcomplete_output (pKey,
							   gok_wordcomplete_get_default ()), 
						   FALSE);
			
			if (str = gok_wordcomplete_get_delimiter (gok_wordcomplete_get_default ()))
			{
				if (strlen (str)) {
					delim_output.Type = OUTPUT_KEYSTRING;
					delim_output.Flag = SPI_KEY_STRING;
					delim_output.Name = (gchar *) str; 
					delim_output.pOutputNext = NULL;
					gok_output_send_to_system (&delim_output, FALSE);
				}
			}
			
			/* output any modifier keys */
			gok_modifier_output_post();
			
			/* turn off any modifier keys that are not locked on */
			gok_modifier_all_off();
			gok_wordcomplete_increment_word_frequency (gok_wordcomplete_get_default (), 
								   gok_key_get_label (pKey));
			
			/* reset the word completor */
			gok_wordcomplete_reset (gok_wordcomplete_get_default ());
			gok_keyboard_clear_completion_keys (keyboard);
			break;
			
		case KEYTYPE_BRANCHMENUITEMS:
		case KEYTYPE_BRANCHLISTITEMS:
		case KEYTYPE_MENUITEM:
		case KEYTYPE_PAGESELECTION:
		case KEYTYPE_BRANCHGUIACTIONS:
		case KEYTYPE_BRANCHGUIVALUATOR:
	        case KEYTYPE_BRANCHGUISELECTION:
	        case KEYTYPE_BRANCHGUITABLE:
	        case KEYTYPE_BRANCHGUISELECTACTION:
			if (!gok_keyboard_branch_byKey (gok_main_get_current_keyboard (), pKey))
				gok_main_display_scan_reset ();
			break;
			
	        case KEYTYPE_TEXTNAV:
	        case KEYTYPE_EDIT:
	        case KEYTYPE_SELECT:
	        case KEYTYPE_TOGGLESELECT:
			gok_keyboard_branch_editableTextAction 
				(gok_main_get_current_keyboard(), pKey);
			break;
			
		default:/* a regular key */
			/* output any modifier keys */
			gok_modifier_output_pre();
		
			/* TODO: add check to see if command prediciton turned on ?*/
	#ifdef WHENCOMMANDPREDICTIONINGOKDATAISHOOKEDUPTOPREFS
			{			
				pPredictedKeyboard = NULL;
				pKeyboardTemp = gok_main_get_first_keyboard();
				g_assert (pKeyboardTemp != NULL);
				while (pKeyboardTemp != NULL)
				{
					pKeyTemp = pKeyboardTemp->pKeyFirst;
					while (pKeyTemp != NULL)
					{
							if (pKeyTemp == pKey)
							{
								pPredictedKeyboard = pKeyboardTemp;
								break;
							}
							pKeyTemp = pKeyTemp->pKeyNext;
					}
					if (pPredictedKeyboard != NULL)
					{
						break;
					}
					pKeyboardTemp = pKeyboardTemp->pKeyboardNext;
				}
				if (pPredictedKeyboard != NULL)
				{								
					gok_predictor_log(gok_main_get_command_predictor(),
						gok_keyboard_get_name(pPredictedKeyboard), 
							gok_key_get_label(pKey) );
				}
			}
	#endif /* WHENCOMMANDPREDICTIONINGOKDATAISHOOKEDUPTOPREFS */
			/* send the key output to the system */
			gok_output_send_to_system (pKey->pOutput, TRUE);
			
			/* output any modifier keys */
			gok_modifier_output_post();
			
			/* turn off any modifier keys that are not locked on */
			if (pKey->Type != KEYTYPE_MODIFIER)
			{
				gok_modifier_all_off();
			}
			break;
	}
	
	gok_log_leave();
	return pKey;
}	

/**
* gok_keyboard_update_dynamic_keys
*
* Enables or disables the keys that branch to the dynamic keyboards
* keyboards.
*
* Returns: TRUE if any of the keys have changed their state (disable/active).
* Returns FALSE if none of the keys change state.
**/
gboolean gok_keyboard_update_dynamic_keys (GokKeyboard *pKeyboard, GokSpyUIFlags change_mask, GokSpyUIFlags flags)
{
	GokKey* pKey;
	gboolean bChanged = FALSE;

	gok_log_enter ();

	/* enable/disable the branch keys */
	flags.value &= change_mask.value;
	pKeyboard->flags.value &= ~change_mask.value;
	pKeyboard->flags.value |= flags.value;

	pKey = pKeyboard->pKeyFirst;
	
	while (pKey != NULL)
	{
		if (pKey->Type == KEYTYPE_BRANCHMENUS && change_mask.data.menus)
		{
			if (pKeyboard->flags.data.menus || pKeyboard->flags.data.context_menu)
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonBranchMenus") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonBranchMenus");
				if (((GokButton*)pKey->pButton)->pLabel != NULL)
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextNormal");
			}
			else
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonDisabled") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonDisabled"); 
				if (((GokButton*)pKey->pButton)->pLabel != NULL) 
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextDisabled");
			}
		}
		else if (pKey->Type == KEYTYPE_BRANCHTOOLBARS && change_mask.data.toolbars)
		{
			if (flags.data.toolbars)
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonBranchToolbars") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonBranchToolbars"); 
				if (((GokButton*)pKey->pButton)->pLabel != NULL)
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextNormal");
			}
			else
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonDisabled") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonDisabled"); 
				if (((GokButton*)pKey->pButton)->pLabel != NULL)
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextDisabled");
			}
		}
		else if (pKey->Type == KEYTYPE_BRANCHGUI && change_mask.data.gui)
		{
			if (flags.data.gui)
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonBranchGUI") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonBranchGUI"); 
				if (((GokButton*)pKey->pButton)->pLabel != NULL)
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextNormal");
			}
			else
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonDisabled") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonDisabled"); 
				if (((GokButton*)pKey->pButton)->pLabel != NULL)
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextDisabled");
			}
		}
		else if (pKey->Type == KEYTYPE_BRANCHEDIT && change_mask.data.editable_text)
		{
			if (flags.data.editable_text)
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonBranchGUI") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonBranchGUI"); 
				if (((GokButton*)pKey->pButton)->pLabel != NULL)
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextNormal");
			}
			else
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonDisabled") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonDisabled"); 
				if (((GokButton*)pKey->pButton)->pLabel != NULL)
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextDisabled");
			}
		}
		else if ((pKey->Type == KEYTYPE_RAISEAPPLICATION)
			 || (pKey->Type == KEYTYPE_SETTINGS)
			 || (pKey->Type == KEYTYPE_HELP)
			 || (pKey->Type == KEYTYPE_ABOUT)
			 || (pKey->Type == KEYTYPE_NORMAL && pKey->pOutput && pKey->pOutput->Type == OUTPUT_EXEC))
		{
			if (gok_main_safe_mode ())
			{
				/* set this flag if the button state changes */
				if (strcmp (gtk_widget_get_name(pKey->pButton), "StyleButtonDisabled") != 0)
				{
					bChanged = TRUE;
				}
				gtk_widget_set_name (pKey->pButton, "StyleButtonDisabled"); 
				if (((GokButton*)pKey->pButton)->pLabel != NULL)
					gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, 
							     "StyleTextDisabled");
			}
			else
			{
				gok_key_set_button_name (pKey);
			}
		}
		pKey = pKey->pKeyNext;
	}
	gok_log_leave ();

	return bChanged;
}


/**
* gok_keyboard_validate_dynamic_keys
* @pAccessibleForeground: Pointer to the foreground accessible pointer.
*
* Enables or disables the keys that branch to the dynamic keyboards
* keyboards.
*
* Returns: TRUE if any of the keys have changed their state (disable/active).
* Returns FALSE if none of the keys change state.
**/
gboolean gok_keyboard_validate_dynamic_keys (Accessible* pAccessibleForeground)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokSpyUIFlags ui_flags, keyboard_ui_flags, all_flags_mask;
	gboolean bChanged;

	gok_log_enter();

	pKeyboard = gok_main_get_current_keyboard();
	keyboard_ui_flags.value = pKeyboard->flags.value; /* safer than initializing to 0 */
    
	/* are there any dynamic keys, or keys that branch to a dynamic keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey->Type == KEYTYPE_BRANCHMENUS)
		{
			keyboard_ui_flags.data.menus = TRUE;
		}
		else if (pKey->Type == KEYTYPE_BRANCHTOOLBARS)
		{
			keyboard_ui_flags.data.toolbars = TRUE;
		}
		else if (pKey->Type == KEYTYPE_BRANCHGUI)
		{
			keyboard_ui_flags.data.gui = TRUE;
		}
		else if (pKey->Type == KEYTYPE_BRANCHEDIT)
		{
			keyboard_ui_flags.data.editable_text = TRUE;
		}
		/* TODO: what about data.context_menu ? */
		pKey = pKey->pKeyNext;
	}
	
	/* if we have branch to dynamic, build a list of 'relevant' UI components */
	if (keyboard_ui_flags.value)
	{	
	    ui_flags = gok_spy_update_component_list (pAccessibleForeground, keyboard_ui_flags);
	}

	/* enable/disable the branch keys */
	all_flags_mask.value = ~0;
	bChanged = gok_keyboard_update_dynamic_keys (pKeyboard, all_flags_mask, ui_flags);

	gok_log_leave();
	
	return bChanged;
}

/**
* gok_keyboard_on_window_resize
*
* This will be called when the window has been resized.
* Change the key size, update the gok_data and settings dialog with the
* new key size.
* If we resize the window (by branching) then the m_bIgnoreResizeEvent flag
* will be set so we ignore the resize. This flag is needed because we can't get
* a message from the system letting us know that it was the user that resized
* the window.
**/
void gok_keyboard_on_window_resize ()
{
	GokKeyboard* pKeyboard;
	GtkWidget* pWindow;
	gint widthWindow;
	gint heightWindow;
	gint widthWindowPrevious;
	gint heightWindowPrevious;
	gint widthKey;
	gint heightKey;

	gok_log_enter();

	/* ignore this resize event if we caused it (by branching the keyboard) */
	/* we want only the resize events generated by user resizing the window */
	if (m_bIgnoreResizeEvent == TRUE)
	{
		m_bIgnoreResizeEvent = FALSE;
		gok_log("we're supposed to ignore this event");
		gok_log_leave();
		return;
	}
		
	/* these pointers will be NULL at the start of the program so we can't resize */
	pKeyboard = gok_main_get_current_keyboard();
	if (pKeyboard == NULL)
	{
		gok_log("main keyboard is NULL");
		gok_log_leave();
		return;
	}
	pWindow = gok_main_get_main_window();
	if (pWindow == NULL)
	{
		gok_log("main windows is NULL");
		gok_log_leave();
		return;
	}
	
	/* compare the size of the window to the size we made the window */
	gdk_window_get_size (pWindow->window, &widthWindow, &heightWindow);
	gok_main_get_our_window_size (&widthWindowPrevious, &heightWindowPrevious);

#ifndef BUG_133323_LIVES
	/* FIXME: see bug 133323 - we need to forcibly resize to work around this */
        /* if the size has not changed then don't do anything */
	if ((widthWindow == widthWindowPrevious) &&
		(heightWindow == heightWindowPrevious))
	{
		gok_log("bug 133323 must be fixed if you see this?");
		gok_log_leave();
		return;
	}
#endif	
	
	/* change the key size */
	widthKey = gok_keyboard_get_keywidth_for_window (widthWindow, pKeyboard);
	if (widthKey < MIN_KEY_WIDTH)
	{
		widthKey = MIN_KEY_WIDTH;
	}
	else if (widthKey > MAX_KEY_WIDTH)
	{
		widthKey = MAX_KEY_WIDTH;
	}

	heightKey = gok_keyboard_get_keyheight_for_window (heightWindow, pKeyboard);
	if (heightKey < MIN_KEY_HEIGHT)
	{
		heightKey = MIN_KEY_HEIGHT;
	}
	else if (heightKey > MAX_KEY_HEIGHT)
	{
		heightKey = MAX_KEY_HEIGHT;
	}

	if ((gok_data_get_dock_type () == GOK_DOCK_NONE) || 
		(pKeyboard->expand == GOK_EXPAND_NEVER)) {
	    if ((widthKey != pKeyboard->keyWidth) || 
		(heightKey != pKeyboard->keyHeight)) 
	    {
		 pKeyboard->bFontCalculated = FALSE;
	         pKeyboard->keyWidth = widthKey;
	         pKeyboard->keyHeight = heightKey;
	    }
	}
	
	/* calculate a new font size for the current keyboard */
	gok_keyboard_calculate_font_size (pKeyboard);
	
	/* redraw all the keys */
	gok_keyboard_position_keys (pKeyboard, pWindow);
	
	/* update the settings dialog */
	gok_settings_page_keysizespace_refresh();

	gok_log_leave();

}

/**
* gok_keyboard_get_keywidth_for_window
* @WidthWindow: Width of the target window.
* @pKeyboard: Pointer to the keyboard that will be displayed.
*
* Calculates a key width for the current keyboard given the window width.
*
* returns: The key width.
**/
int gok_keyboard_get_keywidth_for_window (gint WidthWindow, GokKeyboard* pKeyboard)
{
	if (gok_data_get_dock_type () != GOK_DOCK_NONE)
		WidthWindow -= GOK_KEYBOARD_DOCK_BORDERPIX * 2;
	return (WidthWindow - ((gok_keyboard_get_number_columns (pKeyboard) - 1) * gok_data_get_key_spacing())) / gok_keyboard_get_number_columns (pKeyboard);
}

/**
* gok_keyboard_get_keyheight_for_window
* @HeightWindow: Height of the target window.
* @pKeyboard: Pointer to the keyboard that will be displayed.
*
* Calculates a key height for the current keyboard given the window height.
*
* returns: The key height.
**/
int gok_keyboard_get_keyheight_for_window (gint HeightWindow, GokKeyboard* pKeyboard)
{
	if (gok_data_get_dock_type () != GOK_DOCK_NONE)
		HeightWindow -= GOK_KEYBOARD_DOCK_BORDERPIX * 2;
	return (HeightWindow - ((gok_keyboard_get_number_rows (pKeyboard) - 1) * gok_data_get_key_spacing())) / gok_keyboard_get_number_rows (pKeyboard);
}

/**
* gok_keyboard_set_ignore_resize
* @bFlag: State of the resize flag.
*
* Sets/clears a flag so that the next resize event will be ignored.
**/
void gok_keyboard_set_ignore_resize (gboolean bFlag)
{
	m_bIgnoreResizeEvent = bFlag;
}

/**
* gok_keyboard_update_labels
*
* Redraws the labels on all the keys. This should be called whenever
* a modifier key changes state.
**/
void gok_keyboard_update_labels ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	
	pKeyboard = gok_main_get_first_keyboard();
	g_assert (pKeyboard != NULL);
	
	while (pKeyboard != NULL)
	{
		pKey = pKeyboard->pKeyFirst;
		while (pKey != NULL)
		{
			gok_key_update_label (pKey);
			pKey = pKey->pKeyNext;
		}
		pKeyboard = pKeyboard->pKeyboardNext;
	}
}

/**
* gok_keyboard_find_key_at:
* @keyboard: a pointer to a #GokKeyboard structure.
* @x: An x coordinate, in the keyboard window's coordinate system. 
* @y: A y coordinate in the keyboard window's coordinate system.
* @prev: A pointer to the #GokKey where the point is suspected to lie, or
*     NULL to start the search "from scratch".  This is a useful performance
*     aid when searching for a moving pointer, for instance, whose previous
*     containing #GokKey is known.
*
* Find the keyboard key corresponding to @x, @y in window coordinates.
* returns: A pointer to a #GokKey, or NULL if no key in the keyboard 
*          contains the point.
**/
GokKey *
gok_keyboard_find_key_at (GokKeyboard *pKeyboard, gint x, gint y, GokKey *prev)
{
	GokKey *pKey = NULL;
	gint row, col, n_rows, n_cols;

	if (prev && gok_key_contains_point (prev, x, y)) {
		return prev;
	}
	else if (pKeyboard) {
		pKey = pKeyboard->pKeyFirst;
		while (pKey) {
			if (gok_key_contains_point (pKey, x, y)) {
				return pKey;
			}
			pKey = pKey->pKeyNext;
		}
	}
	return NULL;
}

/**
 **/
gboolean
gok_keyboard_modifier_is_set (guint modifier)
{
	return FALSE;
}

/**
 **/
GokKeyboardValueOp 
gok_keyboard_parse_value_op (const gchar *string)
{
	if (!strcmp (string, "less"))
		return GOK_VALUE_LESS;
	else if (!strcmp (string, "more"))
		return GOK_VALUE_MORE;
	else if (!strcmp (string, "fast-less"))
		return GOK_VALUE_MUCH_LESS;
	else if (!strcmp (string, "fast-more"))
		return GOK_VALUE_MUCH_MORE;
	else if (!strcmp (string, "min"))
		return GOK_VALUE_MIN;
	else if (!strcmp (string, "max"))
		return GOK_VALUE_MAX;
	else if (!strcmp (string, "default"))
		return GOK_VALUE_DEFAULT;
	else
		return GOK_VALUE_UNSPECIFIED;
}

/**
 **/
GokKeyboardDirection 
gok_keyboard_parse_direction (const gchar *string)
{
	if (!strcmp (string, "east"))
		return GOK_DIRECTION_E;
	else if (!strcmp (string, "northeast"))
		return GOK_DIRECTION_NE;
	else if (!strcmp (string, "north"))
		return GOK_DIRECTION_N;
	else if (!strcmp (string, "northwest"))
		return GOK_DIRECTION_NW;
	else if (!strcmp (string, "west"))
		return GOK_DIRECTION_W;
	else if (!strcmp (string, "southwest"))
		return GOK_DIRECTION_SW;
	else if (!strcmp (string, "south"))
		return GOK_DIRECTION_S;
	else if (!strcmp (string, "southeast"))
		return GOK_DIRECTION_SE;
	else if (!strcmp (string, "fillwidth"))
		return GOK_DIRECTION_FILL_EW;
	else if (!strcmp (string, "narrower"))
		return GOK_RESIZE_NARROWER;
	else if (!strcmp (string, "wider"))
		return GOK_RESIZE_WIDER;
	else if (!strcmp (string, "shorter"))
		return GOK_RESIZE_SHORTER;
	else if (!strcmp (string, "taller"))
		return GOK_RESIZE_TALLER;
	return GOK_DIRECTION_NONE;
}

/* TODO: extend the list to allow command predictions as well */
gint
gok_keyboard_set_predictions (GokKeyboard *pKeyboard, gchar **list, gchar *add_word)
{
	GokKey* pKey;
	gint count;
	gint length;
	gboolean add_word_pending = add_word && (g_utf8_strlen (add_word, -1) > 2);
	int i;

	/* clear the current word predictions */
	gok_keyboard_clear_completion_keys (pKeyboard);

	/* fill in the word completion keys */
	count = 0;
	pKey = pKeyboard->pKeyFirst;
	
	for (i = 0; list && list[i]; ++i)
	{
	    /* make sure we're not going over our maximum predictions */
	    count++;
	    if (count > gok_data_get_num_predictions())
	    {
		break;
	    }
		
	    /* get the next word completion key on the keyboard */
	    while (pKey != NULL)
	    {
		if (pKey->Type == KEYTYPE_WORDCOMPLETE)
		{
		    gok_key_set_button_label (pKey, list[i]);
		    gok_key_add_label (pKey, list[i], 0, 0, NULL);
		    
			pKey = pKey->pKeyNext;
			break;
		}
		else if (add_word && add_word_pending && pKey->Type == KEYTYPE_ADDWORD)
		{
		    gchar *label = g_strconcat ("Add \'", add_word, "\'", NULL);
		    gok_key_set_button_label (pKey, label);
		    gok_key_add_label (pKey, label, 0, 0, NULL);
		    pKey->Target = g_strdup (add_word);
		    g_free (label);
		    add_word_pending = FALSE;
		}
		pKey = pKey->pKeyNext;
	    }
	    if (pKey == NULL)
	    {
		break;
	    }
	}

	if (add_word && add_word_pending)
	{
	    while (pKey && add_word_pending)
	    {
		if (pKey->Type == KEYTYPE_ADDWORD)
		{
		    gchar *label = g_strconcat ("Add \'", add_word, "\'", NULL);
		    gok_key_set_button_label (pKey, label);
		    gok_key_add_label (pKey, label, 0, 0, NULL);
		    pKey->Target = g_strdup (add_word);
		    g_free (label);
		    add_word_pending = FALSE;
		}
		pKey = pKey->pKeyNext;
	    }
	}
	
	/* change the font size for the word completion keys */
	if (count > 0)
	{
		gok_keyboard_calculate_font_size_group (pKeyboard, FONT_SIZE_GROUP_WORDCOMPLETE, TRUE);
	}
	
	return count;
}
