/* gok-key.c
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

#include <string.h>
#include <gnome.h>
#include "gok-key.h"
#include "gok-keyboard.h"
#include "gok-mousecontrol.h"
#include "gok-repeat.h"
#include "gok-composer.h"
#include "gok-log.h"
#include "gok-modifier.h"
#include "gok-scanner.h"
#include "gok-feedback.h"
#include "gok-word-complete.h"
#include "gok-data.h"
#include "gok-branchback-stack.h"
#include "gok-modifier-keymasks.h"
#include "gok-gconf-keys.h"
#include "main.h"

#define XML_STRING_SIZE_STOCK ((const xmlChar *) "stock")
#define XML_STRING_SIZE_FIXED ((const xmlChar *) "fixed")
#define XML_STRING_SIZE_FIT ((const xmlChar *) "fit")
#define XML_STRING_SIZE_FILL ((const xmlChar *) "fill")

#define xmlStrPrefix(a, b) (!xmlStrncmp((a), (b), xmlStrlen(b)))

long keysym2ucs(KeySym keysym); /* from keysym2ucs.c */

gchar m_TextEmpty[]="";

gchar m_label[XkbKeyNameLength + 1];

gint m_Group = 0; /* FIXME: statics are bad */

#undef GOK_UTF8_DEBUG

static void 
gok_key_print_utf8_debug (gchar *buf)
{
      gchar *cp = buf;
      gunichar next_char;
      while ((next_char = g_utf8_get_char_validated (cp, -1)) >= 0) 
      {
	  fprintf (stderr, "[%x]", (unsigned long) next_char);
	  if (next_char == 0) 
	  {
	      fprintf (stderr, "\t");
	      break;
	  }
	  cp = g_utf8_next_char (cp);
      }
      cp = buf;
      while (*cp)
      {
	  gchar c = *cp;
	  fprintf (stderr, "0x%x ", (unsigned) c);
	  ++cp;
      }
      fprintf (stderr, "\n");
}

/* returned string must be freed */
static gchar *
gok_key_lookup_shorter_label (gchar *str)
{
	gint i;
	gchar *xstr = NULL;

	/* handle deadkeys gracefully */

	if (!strcmp (str, "Delete")) {
		xstr = "Del";
	}
	else if (!strcmp (str, "Super")) {
		xstr = "Fn";
	}
	else if (!strcmp (str, "Insert")) {
		xstr = "Ins";
	}
	else if (!strcmp (str, "Control")) {
		xstr = "Ctrl";
	}
	else if (!strcmp (str, "Escape")) {
		xstr = "Esc";
	}
	else if (!strcmp (str, "SunAudioRaiseVolume")) {
		xstr = "Vol\n+";
	}
	else if (!strcmp (str, "SunAudioLowerVolume")) {
		xstr = "Vol\n-";
	}
	else if (!strcmp (str, "SunAudioMute")) {
		xstr = _("Mute");
	}
	else if (!strcmp (str, "Pointer\nEnableKeys")) {
		xstr = _("Mouse\nKeys");
	}
	else if (!strcmp (str, "ISO_Left\nTab")) {
		xstr = _("Left\nTab");
	}
	else if (!strcmp (str, "Sterling")) {
		xstr = "£";
	}
	else if (!strncmp (str, "dead\n", 5)) {
		gchar *cp = (gchar *) &str[5];
		if (!strcmp (cp, "grave")) {
			xstr = "[\140]";
		}
		else if (!strcmp (cp, "acute")) {
			xstr = "[\302\264]";
		}
		else if (!strcmp (cp, "circumflex")) {
			xstr = "[^]";
		}
		else if (!strcmp (cp, "tilde")) {
			xstr = "[~]";
		}
		else if (!strcmp (cp, "macron")) {
			xstr = "[\302\257]";
		}
		else if (!strcmp (cp, "breve")) {
			xstr = "[\313\230]";
		}
		else if (!strcmp (cp, "abovedot")) {
			xstr = "[\313\231]";
		}
		else if (!strcmp (cp, "diaeresis")) {
			xstr = "[\302\250]";
		}
		else if (!strcmp (cp, "abovering")) {
			xstr = "[\313\232]";
		}
		else if (!strcmp (cp, "doubleacute")) {
			xstr = "[\313\235]";
		}
		else if (!strcmp (cp, "caron")) {
			xstr = "[\313\207]";
		}
		else if (!strcmp (cp, "cedilla")) {
			xstr = "[\302\270]";
		}
		else if (!strcmp (cp, "ogonek")) {
			xstr = "[\313\233]";
		}
		else if (!strcmp (cp, "iota")) {
			xstr = "[\315\205]";
		}
		else if (!strcmp (cp, "voiced")) { /* not sure */
			xstr = "[\313\254]";
		}
		else if (!strncmp (cp, "semivoiced", 10)) { /* not sure */
			xstr = "[\312\261]";
		}
		else if (!strcmp (cp, "belowdot")) {
			xstr = "[\314\243]";
		}
		else if (!strcmp (cp, "hook")) {
			xstr = "[\313\236]";
		}
		else if (!strcmp (cp, "horn")) { /* not sure */
			xstr = "[\314\233]";
		}
	}

	if (xstr != NULL) {
		g_free (str);
		str = g_strdup(xstr);
	}
	else {
		gint len;
		gboolean prev_is_lower, is_upper;
		gchar *s1, *st2, *st1;
		
		if (!strncmp (str, "Sun", 3)) {
			s1 = g_strdup (str + 3);
			g_free (str);
			str = s1;
		}
		len = strlen (str);
		/* fear not, non-ascii strings won't get changed */
		i = 1;
		prev_is_lower = g_ascii_islower (*str);
		while (i < len) {
		        is_upper = g_ascii_isupper (* (str + i));
			if (is_upper && prev_is_lower) {
				st1 = g_strdup (str + i);
				*(str + i) = '\0';
				st2 = g_strconcat (str, "\n", st1, NULL);
				g_free (st1);
				g_free (str);
				str = st2;
			}			
			prev_is_lower = g_ascii_islower (* (str + i));
			i++;
		}
	}
	return str;
}

static gchar *	
gok_key_label_from_keysym_string (const gchar *str)
{
	gchar *cp;
	size_t strl;

	if (!str) return g_strdup ("<nil>");
	strl = strlen(str);

	if (!strcmp (str, "KP_Divide")) {
		cp = _("Divide");
	}
	else if (!strcmp (str, "KP_Multiply")) {
		cp = _("Multiply");
	}
	else if (!strcmp (str, "KP_Subtract")) {
		cp = _("Subtract");
	}
	else if (!strcmp (str, "KP_Add")) {
		cp = _("Addition");
	}
	else if (!strcmp (str, "KP_Prior") || !strcmp (str, "Prior")) {
		cp = _("Prior");
	}
	else if (!strcmp (str, "KP_Next") || !strcmp (str, "Next")) {
		cp = _("Next");
	}
	else if (!strcmp (str, "KP_Home")) {
		cp = _("Home");
	}
	else if (!strcmp (str, "KP_End")) {
		cp = _("End");
	}
	else if (!strcmp (str, "KP_Up")) {
		cp = _("Up");
	}
	else if (!strcmp (str, "KP_Down")) {
		cp = _("Down");
	}
	else if (!strcmp (str, "KP_Left")) {
		cp = _("Left");
	}
	else if (!strcmp (str, "KP_Right")) {
		cp = _("Right");
	}
	else if (!strcmp (str, "KP_Begin")) {
		cp = _("Begin");
	}
	else if (!strcmp (str, "KP_Decimal")) {
		cp = _("Decimal");
	}
	else if (!strcmp (str, "Meta_L") || !strcmp (str, "Meta_R")) {
		cp = _("Meta");
	}
	else if (!strcmp (str, "Multi_key")) {
		cp = _("Multi\nkey");
	}
	else if (!strcmp (str, "Eisu_toggle")) {
		cp = _("Eisu\ntoggle");
	}
	else if (!strcmp (str, "Henkan_Mode")) {
		cp = _("Henkan\nMode");
	}
	else if (!strcmp (str, "Muhenkan")) {
		cp = _("Muhenkan");
	}
	else if (!strcmp (str, "Mode_switch")) {
		cp = _("Mode\nswitch");
	}
/*      REMOVE COMMENTS POST-BRANCH: freeze applies currently */
 	else if (!strcmp (str, "Hiragana_Katakana")) {
	        cp = "Hiragana\nKatakana";
/*
	        cp = _("Hiragana\nKatakana");
 */
	}
	else if (strncmp(str, "KP_", 3) == 0) {
 	        cp = g_strdup (str + 3);
 	}
	else if ((strl >= 2) && (str[strl-1] == 'L') && (str[strl-2] == '_')) {
		cp = g_strndup (str, strl - 2);
	}
	else if ((strl >= 2) && (str[strl-1] == 'R') && (str[strl-2] == '_')) {
		cp = g_strndup (str, strl - 2);
	}
	else {
	        cp = g_strdup (str);
	}
	if ((strl > 1) && g_strrstr (cp, "_")) {
		*g_strrstr (cp, "_") = '\n';
	}
	if (strlen (cp) > 4)
		cp = gok_key_lookup_shorter_label (cp);

	return cp;
}

static unsigned int _numlock_mask = 0xFFFF;

unsigned int
gok_key_get_numlock_mask (Display *display)
{
	struct gok_modifier_keymask *m;

	if (_numlock_mask == 0xFFFF) {
		_numlock_mask = XkbKeysymToModifiers (display, XK_Num_Lock);
	}
	return _numlock_mask;
}


int
gok_key_get_xkb_type_index (XkbDescPtr xkb, KeyCode keycode, guint group)
{
	int num_groups = XkbKeyNumGroups (xkb, keycode);
	int index;
	if (!xkb) 
	        return 1;
	if (group >= num_groups) 
		group = XkbOutOfRangeGroupNumber (XkbKeyGroupInfo (xkb, keycode));
	return xkb->map->key_sym_map[keycode].kt_index[group];
}

int
gok_key_level_for_type (Display *display, XkbDescRec *kbd,
			int type, unsigned int *modmask)
{
	int i, level = 0;
	gboolean level_set = FALSE;
	XkbKeyTypeRec *key_type = &kbd->map->types[type];

	/* compare against each map entry in the XkbKeyType */
	for (i = 0; i < key_type->map_count && !level_set; ++i)
	{
		unsigned int vmods_equiv = 0, level_mods = 0;
		unsigned int mods_mask;
		unsigned int mods = *modmask;
		XkbVirtualModsToReal (kbd, key_type->map[i].mods.vmods,
				      &vmods_equiv);
		XkbVirtualModsToReal (kbd, key_type->mods.vmods,
				      &mods_mask);
		mods_mask = key_type->mods.mask;
		level_mods = key_type->map[i].mods.real_mods | vmods_equiv;
#ifdef GOK_DEBUG
		fprintf (stderr, "key type %d [map %d], level %d, preserve %x; mods_mask %x; level_mods %x; modmask %x\n",
					type, i, key_type->map[i].level,
					key_type->preserve ? key_type->preserve[i].mask : 0,
					mods_mask,
			                level_mods,
			                *modmask);
#endif
		if ((key_type->map[i].active) && 
		    ((mods & mods_mask) == level_mods))
		{	
			level = key_type->map[i].level;
			level_set = TRUE;
			/** 
			 *  Special hack for alpha types: see section 15.2.1 of XKBlib spec. 
			 *  We'd restrict this to the canonical types, but XFree's maps tend to use 
			 *  non-canonical types for the primary alphanumeric keycodes, in order
			 *  to support more shift levels (AltGr, Shift-AltGr, etc.)
			 *  If we reported the Xkb 'level' here, the keycap strings would not get
			 *  'shifted' in our compose keyboard when CapsLock is active, since the 
			 *  capitalization in that case is deferred to the Xlib lookup stage.
			 **/
			if (key_type->preserve && (key_type->preserve[i].mask == LockMask))
			{
			    /* Lock is preserved for Xlib, but we want to report 'equivalent' level */
			    if ((mods & LockMask) && !(mods & ShiftMask))
			    {
				int j;
				/* find the map matching the 'Shift' equivalent */
				guint equiv_mods = (mods & ~LockMask) | ShiftMask; 
				for (j = 0; j < key_type->map_count; ++j)
				{
				    if (key_type->map[j].active && 
					(key_type->map[j].mods.real_mods == equiv_mods)) 
				    {
					level = key_type->map[j].level;
					level_set = TRUE;
					break;
				    }
				}
			    }
			}

			/* preserve what needs preserving */
			if (key_type->preserve) 
				*modmask = *modmask & key_type->preserve[i].mask;
			else
				*modmask = 0;
			break;
		}
	}
	if (i == key_type->map_count) /* no explicit match */
		*modmask = 0;

	return level;
}

static gchar *
gok_key_label_from_keycode (KeyCode keycode, Display *display, guint level, guint group)
{
  char buf[20];
  int extra_rtn, nbytes, state = 0;
  KeySym keysym = 0;
  XkbStateRec xkb_state;
  long ucs;
  gunichar unichar;

  keysym = XkbKeycodeToKeysym (display, keycode, group, level);
  if ((ucs = keysym2ucs (keysym)) <= 0) 
  {
      gint reported_nbytes = XkbTranslateKeySym (display, &keysym, 0, buf, 19, &extra_rtn);
      gchar *s = NULL;
      /* workaround; XSun gets nbytes wrong, so we truncate to the largest legal value */  
      nbytes = MIN (reported_nbytes, 19);
      buf[nbytes] = '\0';
      if (keysym) s = XKeysymToString (keysym);
      return gok_key_label_from_keysym_string (s);
  }
  else
  {
      gchar cbuf[10];
      unichar = (gunichar) ucs;
      cbuf[g_unichar_to_utf8 (unichar, cbuf)] = '\0';

      return gok_key_label_from_keysym_string (cbuf);
  }
}

static int
gok_key_keycode_from_xkb_key (XkbKeyPtr keyp, XkbDescPtr kbd)
{
  int k;
  gchar *name = keyp->name.name;
  if (kbd) {
    for (k = kbd->min_key_code; k < kbd->max_key_code; ++k) 
      {
	if (!strncmp (name, kbd->names->keys[k].name, XkbKeyNameLength))
	  {
	    return k;
	  }
      }
  }
  return 0;
}

/* HACK copies gtk+ */
#define CHILD_SPACING 1 
#define BEVEL_WIDTH 1 

gint
gok_key_get_default_border_width (GokKey *pKey)
{
	if (pKey->pButton)
		return GTK_CONTAINER (pKey->pButton)->border_width + 
			CHILD_SPACING +
			GTK_WIDGET (pKey->pButton)->style->xthickness;
	else
		return 0;
}

static gint
gok_key_get_default_font_size (GokKey *pKey)
{
        PangoFontDescription *font_desc = 
	        GTK_WIDGET (pKey->pButton)->style->font_desc;
        return PANGO_PIXELS (pango_font_description_get_size (font_desc));
}

static gint
gok_key_get_default_border_height (GokKey *pKey)
{
        return GTK_CONTAINER (pKey->pButton)->border_width + 
                CHILD_SPACING + 
                GTK_WIDGET (pKey->pButton)->style->ythickness;
}

/**
 **/
gchar *
gok_key_modifier_for_keycode (Display *display, XkbDescPtr xkb, int keycode)
{
	guint group = gok_key_get_effective_group ();
	gint type = gok_key_get_xkb_type_index (xkb, keycode, group);
	guint modmask = 0;	
	GokModifier *modifier;
        /* FIXME: assumes modifier bindings don't depend on current modmask! */
	gint level = gok_key_level_for_type (display, gok_keyboard_get_xkb_desc (), type, &modmask);
	KeySym keysym = XkbKeycodeToKeysym (display, (KeyCode) keycode, 
					    group, level);
	modmask = XkbKeysymToModifiers (display, keysym);
	return gok_modifier_first_name_from_mask (modmask);
}

gint
gok_key_get_effective_group ()
{
	/* FIXME : globals are bad. Also we need to init properly */
	return m_Group;
}

void
gok_key_set_effective_group (gint group)
{
	/* FIXME: globals are bad */
	m_Group = group;
	gok_keyboard_update_labels ();
}

void
gok_keyimage_set_size_from_spec (GokKeyImage *keyimage, gchar *sizespec, gchar *align)
{
	if (xmlStrPrefix ((const unsigned char*)sizespec, XML_STRING_SIZE_STOCK)) {
		keyimage->type = IMAGE_TYPE_STOCK;
	}
	else if (xmlStrPrefix ((const unsigned char*)sizespec, XML_STRING_SIZE_FIXED)) {
		int w, h;
		keyimage->type = IMAGE_TYPE_FIXED;
		if (sscanf (sizespec, "%*6c%d,%d", &w, &h) == 2) {
			keyimage->w = w;
			keyimage->h = h;
		}
	}
	else if (xmlStrPrefix ((const unsigned char*)sizespec, XML_STRING_SIZE_FIT))
		keyimage->type = IMAGE_TYPE_FIT;
	else if (xmlStrPrefix ((const unsigned char*)sizespec, XML_STRING_SIZE_FILL))
		keyimage->type = IMAGE_TYPE_FILL;

	if (!xmlStrcmp ((const unsigned char*)align, (const xmlChar *) "right")) {
		keyimage->placement_policy = IMAGE_PLACEMENT_RIGHT;
	}
	else { /* TODO: that's all we recognize for now, implement other options */
		keyimage->placement_policy = IMAGE_PLACEMENT_LEFT;
	}
}

GokKey *
gok_key_from_xkb_key (GokKey *prevKey, GokKeyboard *pKeyboard, Display *display, XkbGeometryPtr pGeom, XkbRowPtr pRow, XkbSectionPtr pXkbSection, XkbKeyPtr keyp, int section, int row, int col)
{
  GokKey *pKey;
  gchar *label;
  gchar name[XkbKeyNameLength + 1];
  gchar keycode_name[8];
  int keycode;
  XkbBoundsRec *pBounds;
  int width, height, len;
  gboolean is_modifier = FALSE;
  unsigned int numlock_mask;
  int group, level;

  pKey = gok_key_new (prevKey, NULL, pKeyboard);
  is_modifier = FALSE;
  pKey->has_text = TRUE; /* TODO: handle non-textual keyboard key caps! */
  pKey->Top = row;
  pBounds = &pGeom->shapes[keyp->shape_ndx].bounds;
  pKey->Bottom = row + MAX (1, (pBounds->y2 - pBounds->y1)/(pBounds->x2 - pBounds->x1));
  pKey->Left = col + pRow->left/(pBounds->y2 - pBounds->y1);
  pKey->Right = pKey->Left + MAX (1, (pBounds->x2 - pBounds->x1)/(pBounds->y2 - pBounds->y1));
  pKey->Section = section;
  keycode = gok_key_keycode_from_xkb_key (keyp, gok_keyboard_get_xkb_desc ());
  pKey->ModifierName = gok_key_modifier_for_keycode (display, 
						     gok_keyboard_get_xkb_desc (),
						     keycode);
  if (pKey->ModifierName != NULL) {
	  is_modifier = TRUE;
	  gok_modifier_add (pKey->ModifierName);
  }
  if (is_modifier) {
    pKey->Type = KEYTYPE_MODIFIER;
    pKey->pImage = gok_keyimage_new (pKey, NULL);
    pKey->pImage->type = IMAGE_TYPE_INDICATOR;
  }
  else {
    pKey->Type = KEYTYPE_NORMAL;
	  pKey->is_repeatable = TRUE;
  }
  pKey->Style = KEYSTYLE_NORMAL;

  pKey->FontSize = -1;

  /* TODO: figure out how to specify smaller modmasks */
  for (group = 0; group < 4; ++group) 
  {
	  for (level = 0; level < 4; ++level) 
	  {
		  /* this API is a little weird, gok_keylabel_new adds the label to the key */
		  label = gok_key_label_from_keycode (keycode, display, level, group);
		  if (!level && !group) /* bogus but we only have one group per key, not per label */
		  {
			  len = g_utf8_strlen (label, -1);
			  if (len > 1) {
				  if (len < 4) 
					  pKey->FontSizeGroup = 3;
				  else
					  pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;  
			  }
			  else {
				  pKey->FontSizeGroup = FONT_SIZE_GROUP_GLYPH;
			  }
		  }
		  gok_keylabel_new (pKey, label, level, group, NULL);
	  }
  }
  /* XXX do we need to g_free (label)  ?? */
  snprintf (keycode_name, 8, "%d", keycode);
  pKey->pOutput = gok_output_new (OUTPUT_KEYCODE, keycode_name, SPI_KEY_PRESSRELEASE);
  return pKey;
}

/**
* gok_key_new
* @pKeyPrevious: Pointer to the previous key in the list of keys.
* @pKeyNext: Pointer to the next key in the list
* @pKeyboard: Pointer to the keyboard.
*
* Allocates memory for a new key and initializes the GokKey structure.
* Returns a pointer to the new key, NULL if it can't be created.
* Call gok_key_delete on this when done with it.
*
* returns: A pointer to the new key, NULL if it wasn't created.
**/
GokKey* gok_key_new (GokKey* pKeyPrevious, GokKey* pKeyNext, GokKeyboard* pKeyboard)
{
	GokKey* pgok_key_new;

	g_assert (pKeyboard != NULL);

	/* allocate memory for the new key structure */
	pgok_key_new= (GokKey*) g_malloc(sizeof(GokKey));

	/* add the new key into the next/previous key list */
	if (pKeyboard->pKeyFirst == NULL)
	{
		pKeyboard->pKeyFirst = pgok_key_new;
	}

	if (pKeyPrevious != NULL)
	{
		pKeyPrevious->pKeyNext = pgok_key_new;
	}

	if (pKeyNext != NULL)
	{
		pKeyNext->pKeyPrevious = pgok_key_new;
	}

	/* initialize the data members of the structure */
	pgok_key_new->has_image = FALSE;
	pgok_key_new->has_text = TRUE;
	pgok_key_new->pImage = NULL;
	pgok_key_new->Type = KEYTYPE_NORMAL;
	pgok_key_new->Style = KEYTYPE_NORMAL;
	pgok_key_new->pLabel = NULL;
	pgok_key_new->Target = NULL;
	pgok_key_new->pOutput = NULL;
	pgok_key_new->pOutputWrapperPre = NULL;
	pgok_key_new->pOutputWrapperPost = NULL;
	pgok_key_new->pButton = NULL;
	pgok_key_new->ModifierName = NULL;
	pgok_key_new->pButton = NULL;
	pgok_key_new->Top = 0;
	pgok_key_new->Bottom = 0;
	pgok_key_new->Left = 0;
	pgok_key_new->Right = 0;
	pgok_key_new->TopWin = 0;
	pgok_key_new->BottomWin = 0;
	pgok_key_new->LeftWin = 0;
	pgok_key_new->RightWin = 0;
	pgok_key_new->pMimicKey = NULL;
	pgok_key_new->pKeyNext = pKeyNext;
	pgok_key_new->pKeyPrevious = pKeyPrevious;
	pgok_key_new->accessible_node = NULL;
	pgok_key_new->FontSizeGroup = FONT_SIZE_GROUP_UNDEFINED;
	pgok_key_new->FontSize = -1; /* not set yet */
	pgok_key_new->ComponentState.active = 0;
	pgok_key_new->ComponentState.radio = 0;
	pgok_key_new->ComponentState.latched = 0;
	pgok_key_new->ComponentState.locked = 0;
	pgok_key_new->State = GTK_STATE_NORMAL;
	pgok_key_new->StateWhenNotFlashed = GTK_STATE_NORMAL;
	pgok_key_new->pGeneral = NULL;
	pgok_key_new->action_ndx = 0;
	pgok_key_new->is_repeatable = FALSE;
	
	return pgok_key_new;
}

/**
* gok_key_delete
* @pKey: Pointer to the key that gets deleted.
* @pKeyboard: Pointer to the keyboard that contains the key (can be NULL). If
* pKeyboard is not NULL then the key is unhooked from the keyboard.
* @bDeleteButton: Flag that determines if the GTK button associated with
* the key should also be deleted. This should be set to TRUE if the key is
* deleted while the program is running. At the end of the program, when the
* GOK window is destroyed and the GTK buttons are destroyed, this should
* be set to FALSE.
* 
* Deletes the given key. This must be called on every key that has been created.
* Don't use the given key after calling this.
* This unhooks the key from the next/previous list of keys.
**/
void gok_key_delete (GokKey* pKey, GokKeyboard* pKeyboard, gboolean bDeleteButton)
{
	/* Don't call gok_spy_free on the NodeAccessible since this will be done by the keyboard. */
	GokKeyLabel* pLabel;
	GokKeyLabel* pLabelTemp;
	GokKeyImage* pImage;
	GokKeyImage* pImageTemp;
	
	if (pKey == NULL)
	{
		return;
	}

	/* delete all the key's images */
	pImage = pKey->pImage;
	while (pImage != NULL)
	{
		pImageTemp = pImage;
		pImage = pImage->pImageNext;
		gok_keyimage_delete (pImageTemp);
	}

	/* if this key is flashing then turn off the flashing */
	if (gok_feedback_get_key_flashing() == pKey)
	{
		gok_feedback_timer_stop_key_flash();
	}

	/* unhook the key from the keyboard */
	if ((pKeyboard != NULL) &&
		(pKeyboard->pKeyFirst == pKey))
	{
		pKeyboard->pKeyFirst = pKey->pKeyNext;
	}
	
	if (pKey->pKeyPrevious != NULL)
	{
		pKey->pKeyPrevious->pKeyNext = pKey->pKeyNext;
	}

	if (pKey->pKeyNext != NULL)
	{
		pKey->pKeyNext->pKeyPrevious = pKey->pKeyPrevious;
	}

	/* delete all the key's labels */
	pLabel = pKey->pLabel;
	if (pLabel) gok_log ("deleting key %s %x (%x)\n", pLabel->Text ? pLabel->Text : "<empty>", pKey,
			     (pKey->accessible_node) ? pKey->accessible_node->paccessible : (gpointer) 0xFFFF);
	while (pLabel != NULL)
	{
		pLabelTemp = pLabel;
		pLabel = pLabel->pLabelNext;
		gok_keylabel_delete (pLabelTemp);
	}

	/* delete the branch target string */
	if (pKey->Target != NULL)
	{
		g_free (pKey->Target);
	}

	/* delete the GTK button associated with the key */
	if ((pKey->pButton != NULL) &&
		(bDeleteButton == TRUE))
	{
/*		g_free(GOK_BUTTON(pKey->pButton)->indicator_type); */
		gtk_widget_destroy (pKey->pButton);
		pKey->pButton = NULL;
	}
	
	/* delete all the outputs associated with the key */
	gok_output_delete_all (pKey->pOutput);
	gok_output_delete_all (pKey->pOutputWrapperPre);
	gok_output_delete_all (pKey->pOutputWrapperPost);

	if (pKey->accessible_node && pKey->accessible_node->paccessible) 
	{
		gok_log ("unreffing source");
		gok_spy_accessible_unref (pKey->accessible_node->paccessible);
	} 

	gok_feedback_drop_refs (pKey);
	gok_scanner_drop_refs (pKey);
	gok_repeat_drop_refs (pKey);

	if (pKey->Type != KEYTYPE_WINDOW) g_free (pKey->pGeneral);

	g_free (pKey);
}

/**
* gok_key_initialize
* @pKey: Pointer to the key that's getting initialized.
* @pNode: Pointer to the XML node that contains the key data.
*
* returns: TRUE if the key was initialized, FALSE if not.
**/ 
gboolean gok_key_initialize (GokKey* pKey, xmlNode* pNode)
{
	xmlChar* pStringAttributeValue, *pStringSubAttributeValue;
	xmlNode* pNodeKeyChild;
	xmlNode* pNodeWrapperChild;
	GokOutput* pNewOutput;
	GokOutput* pOutputTemp;
	gboolean settings_locked;
	guint modmask =  
		SPI_KEYMASK_SHIFT | SPI_KEYMASK_SHIFTLOCK | SPI_KEYMASK_ALT | 
		SPI_KEYMASK_CONTROL | SPI_KEYMASK_MOD1 | SPI_KEYMASK_MOD2 | SPI_KEYMASK_MOD3 | SPI_KEYMASK_MOD4 |
		SPI_KEYMASK_MOD5; /* ASSUMPTION: this includes numlock mask */
	
	g_assert (pKey != NULL);
	g_assert (pNode != NULL);

	pKey->has_text = FALSE;
	pKey->is_repeatable = FALSE;

	/* TODO: make more styles in gok.rc */
	
	/* type of key (normal, branch etc.) */
	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "type");
	if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"normal") == 0)
	{
		pKey->Type = KEYTYPE_NORMAL;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->is_repeatable = TRUE;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "modifier") == 0)
	{
		pKey->Type = KEYTYPE_MODIFIER;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->pImage = gok_keyimage_new (pKey, NULL);
		pKey->pImage->type = IMAGE_TYPE_INDICATOR;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "branch") == 0)
	{
		pKey->Type = KEYTYPE_BRANCH;
		pKey->Style = KEYSTYLE_BRANCH;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "branchModal") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHMODAL;
		pKey->Style = KEYSTYLE_BRANCHMODAL;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"branchBack") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHBACK;
		pKey->Style = KEYSTYLE_BRANCHBACK;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "branchCompose") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHCOMPOSE;
		pKey->Style = KEYSTYLE_BRANCH;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"branchAlphabet") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHALPHABET;
		pKey->Style = KEYSTYLE_BRANCHALPHABET;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"branchToolbars") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHTOOLBARS;
		pKey->Style = KEYSTYLE_BRANCHTOOLBARS;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"branchMenus") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHMENUS;
		pKey->Style = KEYSTYLE_BRANCHMENUS;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"branchGUI") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHGUI;
		pKey->Style = KEYSTYLE_BRANCHGUI;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"branchEditText") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHEDIT;
		pKey->Style = KEYSTYLE_GENERALDYNAMIC;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"branchSettings") == 0)
	{
		pKey->Type = KEYTYPE_BRANCH;
		pKey->Style = KEYSTYLE_SETTINGS;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"settings") == 0)
	{
		pKey->Type = KEYTYPE_SETTINGS;
		if (gok_main_get_login ())
		{
				pKey->Style = KEYSTYLE_INSENSITIVE;
		}
		else {
			pKey->Style = KEYSTYLE_SETTINGS;
		}
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"branchWindows") == 0)
	{
		pKey->Type = KEYTYPE_BRANCHWINDOWS;
		pKey->Style = KEYSTYLE_BRANCH;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"pointer") == 0)
	{
		pKey->Type = KEYTYPE_POINTERCONTROL;
		pKey->Style = KEYSTYLE_POINTERCONTROL;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"dock") == 0)
	{
		pKey->Type = KEYTYPE_DOCK;
		pKey->Style = KEYSTYLE_DOCK;
		pKey->pGeneral = g_new0 (GokKeyboardDirection, 1);
		pStringSubAttributeValue = 
			xmlGetProp (pNode, (const xmlChar *) "dir");
		if (pStringSubAttributeValue != NULL) 
		{
			* (GokKeyboardDirection *) pKey->pGeneral = 
				gok_keyboard_parse_direction ((const char *)pStringSubAttributeValue);
			xmlFree (pStringSubAttributeValue);
		}
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"move-resize") == 0)
	{
		pKey->Type = KEYTYPE_MOVERESIZE;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->pGeneral = g_new0 (GokKeyboardDirection, 1);
		pStringSubAttributeValue = 
			xmlGetProp (pNode, (const xmlChar *) "dir");
		if (pStringSubAttributeValue != NULL) 
		{
			* (GokKeyboardDirection *) pKey->pGeneral = 
				gok_keyboard_parse_direction ((const char *)pStringSubAttributeValue);
			xmlFree (pStringSubAttributeValue);
		}
		pKey->is_repeatable = TRUE;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"help") == 0)
	{
		pKey->Type = KEYTYPE_HELP;
		pKey->Style = KEYSTYLE_HELP;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"about") == 0)
	{
		pKey->Type = KEYTYPE_ABOUT;
		pKey->Style = KEYSTYLE_ABOUT;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"navigate") == 0)
	{
		pKey->Type = KEYTYPE_TEXTNAV;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->is_repeatable = TRUE;
		gok_compose_key_init (pKey, pNode);
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"edit") == 0)
	{
		pKey->Type = KEYTYPE_EDIT;
		pKey->Style = KEYSTYLE_NORMAL;
		gok_compose_key_init (pKey, pNode);
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"valuator") == 0)
	{
		pKey->Type = KEYTYPE_VALUATOR;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->is_repeatable = TRUE;
		pKey->pGeneral = g_new0 (GokKeyboardValueOp, 1);
		pStringSubAttributeValue = 
			xmlGetProp (pNode, (const xmlChar *) "command");
		if (pStringSubAttributeValue != NULL) 
		{
			* (GokKeyboardValueOp *) pKey->pGeneral = 
				gok_keyboard_parse_value_op ((const char *)pStringSubAttributeValue);
			xmlFree (pStringSubAttributeValue);
		}
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"select") == 0)
	{
		pKey->Type = KEYTYPE_SELECT;
		pKey->Style = KEYSTYLE_SELECT;
		gok_compose_key_init (pKey, pNode);
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"select-toggle") == 0)
	{
		pKey->Type = KEYTYPE_TOGGLESELECT;
		pKey->Style = KEYSTYLE_SELECT;
		pKey->pImage = gok_keyimage_new (pKey, NULL);
		pKey->pImage->type = IMAGE_TYPE_INDICATOR;
		pKey->has_image = TRUE;
		pKey->ComponentState.active = FALSE;
		gok_compose_key_init (pKey, pNode);
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"repeat-next") == 0)
	{
		pKey->Type = KEYTYPE_REPEATNEXT;
		pKey->Style = KEYSTYLE_REPEATNEXT;
		pKey->has_image = TRUE;
		pKey->pImage = gok_keyimage_new (pKey, NULL);
		pKey->pImage->type = IMAGE_TYPE_INDICATOR;
		pKey->ComponentState.active = FALSE;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"mouse") == 0)
	{
		pKey->Type = KEYTYPE_MOUSE;
		pKey->Style = KEYSTYLE_NORMAL;
		pKey->is_repeatable = TRUE;
		gok_mouse_control_init (pKey, pNode);
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"mousebutton") == 0)
	{
		pKey->Type = KEYTYPE_MOUSEBUTTON;
		if (!gok_scanner_current_state_uses_corepointer ()) 
			pKey->Style = KEYSTYLE_MOUSEBUTTON;
		else
			pKey->Style = KEYSTYLE_INSENSITIVE;
		gok_mouse_control_init (pKey, pNode);
	}
	xmlFree (pStringAttributeValue);

	/* branch target */
	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "target");
	{
		if (pStringAttributeValue != NULL)
		{
			pKey->Target = (char *) g_strdup ((const char*)pStringAttributeValue);
		}
	}
	xmlFree (pStringAttributeValue);

	/* font size group*/
	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "fontsizegroup");
	if (pStringAttributeValue != NULL)
	{
		if (!strcmp ((char *)pStringAttributeValue, "glyph"))
			pKey->FontSizeGroup = FONT_SIZE_GROUP_GLYPH;
		else if (!strcmp ((char *)pStringAttributeValue, "unique"))
			pKey->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
		else
			pKey->FontSizeGroup = atoi ((char *)pStringAttributeValue);
	}
	xmlFree (pStringAttributeValue);

	/* location of key (top, bottom, left & right) */
	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "top");
	if (pStringAttributeValue != NULL)
	{
		pKey->Top = atoi ((char *)pStringAttributeValue);
	}
	else
	{
		gok_log_x ("Can't find 'top' attribute in gok_key_initialize!\n");
		return FALSE;
	}
	xmlFree (pStringAttributeValue);

	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "bottom");
	if (pStringAttributeValue != NULL)
	{
		pKey->Bottom = atoi ((char *)pStringAttributeValue);
	}
	else
	{
		gok_log_x ("Warning: Can't find 'bottom' attribute in gok_key_initialize!\n");
		return FALSE;
	}
	xmlFree (pStringAttributeValue);

	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "left");
	if (pStringAttributeValue != NULL)
	{
		pKey->Left = atoi ((char *)pStringAttributeValue);
	}
	else
	{
		gok_log_x ("Warning: Can't find 'left' attribute in gok_key_initialize!\n");
		return FALSE;
	}
	xmlFree (pStringAttributeValue);

	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "right");
	if (pStringAttributeValue != NULL)
	{
		pKey->Right = atoi ((char *)pStringAttributeValue);
	}
	else
	{
		gok_log_x ("Warning: Can't find 'right' attribute in gok_key_initialize!\n");
		return FALSE;
	}
	xmlFree (pStringAttributeValue);

	/* is this a 'modifier' key? */
	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "modifier");
	if (pStringAttributeValue != NULL)
	{
		/* add the modifier to the list of modifiers */
		gok_modifier_add ((char *)g_strdup ((const char *)pStringAttributeValue));
		
		/* store the name of the modifier on the key */
		pKey->ModifierName = (gchar*)g_malloc (strlen ((char *)pStringAttributeValue) + 1);
		strcpy (pKey->ModifierName, (char *)pStringAttributeValue);

		/* what type of modifier is this? */
		pStringSubAttributeValue = xmlGetProp (pNode, (const xmlChar *) "modifiertype");
		if (pStringSubAttributeValue != NULL)
		{
		    if (strcmp ((char *)pStringSubAttributeValue, "toggle") == 0)
		    {
			gok_modifier_set_type (pKey->ModifierName, MODIFIER_TYPE_TOGGLE);
		    }
		    xmlFree (pStringSubAttributeValue);
		}
	}
	xmlFree (pStringAttributeValue);

	/* get child elements of the key */
	pNodeKeyChild = pNode->xmlChildrenNode;
	while (pNodeKeyChild != NULL)
	{
		/* key label */
		if (xmlStrcmp (pNodeKeyChild->name, (const xmlChar *)"label") == 0)
		{
			guint level = 0, group = 0;
			gchar *levelname = (gchar *) xmlGetProp (pNodeKeyChild, (const xmlChar *) "level");
			gchar *groupname = (gchar *) xmlGetProp (pNodeKeyChild, (const xmlChar *) "group");
			gchar *vmodname = (gchar *) xmlGetProp (pNodeKeyChild, (const xmlChar *) "modifier");
			gchar *content = (gchar *) xmlNodeGetContent (pNodeKeyChild);

			if (levelname) 
			{
			    level = atoi ((char *) levelname);
			}
			if (groupname) 
			{
			    group = atoi ((char *) groupname);
			}
			
			gok_keylabel_new (pKey, g_strstrip (content),
					  level, group, vmodname);
			g_free (content);
			pKey->has_text = TRUE;
		}

		/* key image */
		if (xmlStrcmp (pNodeKeyChild->name, (const xmlChar *)"image") == 0)
		{
			/* TODO: support href as well as local files ? */
			gchar *filename = (gchar *)xmlGetProp (pNodeKeyChild, (const xmlChar *) "source");
			gchar *sizespec = (gchar *)xmlGetProp (pNodeKeyChild, (const xmlChar *) "type");
			gchar *align = (gchar *)xmlGetProp (pNodeKeyChild, (const xmlChar *) "align");
			gok_keyimage_new (pKey, filename);
			if (sizespec != NULL) 
				gok_keyimage_set_size_from_spec (pKey->pImage, sizespec, align);
			pKey->has_image = TRUE;
		}

		/* output */
		else if (xmlStrcmp (pNodeKeyChild->name, (const xmlChar *)"output") == 0)
		{
			pNewOutput = gok_output_new_from_xml (pNodeKeyChild);
			if (pNewOutput != NULL)
			{
				if (pKey->pOutput == NULL)
				{
					pKey->pOutput = pNewOutput;
				}
				else
				{
					pOutputTemp = pKey->pOutput;
					while (pOutputTemp->pOutputNext != NULL)
					{
						pOutputTemp = pOutputTemp->pOutputNext;
					}
					pOutputTemp->pOutputNext = pNewOutput;
				}
			}
		}
		
		/* wrapper */
		else if (xmlStrcmp (pNodeKeyChild->name, (const xmlChar *)"wrapper") == 0)
		{
			/* is this key a modifier? */
			if (pKey->ModifierName == NULL)
			{
				gok_log_x ("Key '%s' is not modifier. Can't add wrapper!\n", gok_key_get_label (pKey));
			}
			else
			{
				/* get output of the wrapper */
				pNodeWrapperChild = pNodeKeyChild->xmlChildrenNode;
				while (pNodeWrapperChild != NULL)
				{
					if (xmlStrcmp (pNodeWrapperChild->name, (const xmlChar *)"output") == 0)
					{
						pNewOutput = gok_output_new_from_xml (pNodeWrapperChild);
						if (pNewOutput != NULL)
						{
							/* is this a 'pre' or 'post' wrapper? */
							pStringAttributeValue = xmlGetProp (pNodeKeyChild, (const xmlChar *) "type");
							if (pStringAttributeValue == NULL)
							{
								gok_log_x ("Wrapper does not have a type!\n");
							}
							else
							{
								if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"pre") == 0)
								{
									pKey->pOutputWrapperPre = pNewOutput;
								}
								else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"post") == 0)
								{
									pKey->pOutputWrapperPost = pNewOutput;
								}
								else
								{
									gok_log_x ("Wrapper type '%s' is invalid!\n", pStringAttributeValue);
								}
							}
						}
					}
					pNodeWrapperChild = pNodeWrapperChild->next;
				}
			}
		}
		
		pNodeKeyChild = pNodeKeyChild->next;
	}

	return TRUE;
}

/**
* gok_keylabel_new
* @pKey: Pointer to the key that gets the new label.
* @pLabelText: Text string for this label.
* @level: the level (see XKB spec) for which this label is valid.
* @group: the group (see XKB spec) for which this label is valid.
* @vmods: a delimited list of virtual modifier names which must be matched 
*         in order for this label to be valid, or NULL if no virtual 
*         modifiers are relevant to this label.
*
* Allocates memory for a new key label and initializes the GokKeyLabel structure.
* Returns a pointer to the new key label, NULL if it can't be created.
* Add this label to a key so it will be deleted when the key is deleted.
*
* returns: A pointer to the new key label, NULL if it wasn't created.
**/
GokKeyLabel* gok_keylabel_new (GokKey* pKey, gchar* pLabelText, guint level, guint group, const gchar *vmods)
{
	GokKeyLabel* pNewLabel;
	GokKeyLabel* pKeyLabel;
	
	/* allocate memory for the new label structure */
	pNewLabel = (GokKeyLabel*) g_malloc(sizeof(GokKeyLabel));
	
	pNewLabel->Text = m_TextEmpty;
	pNewLabel->level = level;
	pNewLabel->group = group;
	pNewLabel->vmods = g_strdup (vmods);
	pNewLabel->pLabelNext = NULL;

	if (pLabelText != NULL)
	{
	        int len = strlen (pLabelText);
		pNewLabel->Text = g_strdup (pLabelText);
	}
	
	if (pKey != NULL)
	{
		pKeyLabel = pKey->pLabel;
		if (pKeyLabel == NULL)
		{
			pKey->pLabel = pNewLabel;
		}
		else
		{
			while (pKeyLabel->pLabelNext != NULL)
			{
				pKeyLabel = pKeyLabel->pLabelNext;
			}
			pKeyLabel->pLabelNext = pNewLabel;
		}
	}
	
	return pNewLabel;
}

/**
* gok_keylabel_delete
* @pKeyLabel: Pointer to the key label that will be deleted.
**/ 
void gok_keylabel_delete (GokKeyLabel* pKeyLabel)
{
	if (pKeyLabel != NULL)
	{
		if (pKeyLabel->Text != m_TextEmpty)
		{
			g_free (pKeyLabel->Text);
		}
		if (pKeyLabel->vmods)
		{
			g_free (pKeyLabel->vmods);
		}
		g_free (pKeyLabel);
	}
}

/**
* gok_key_add_label
* @pKey: Pointer to the key that's gets the new label.
* @pLabel: Pointer to the label text.
* @pModifier: Pointer to the 'modifier' for the key's label.
*
* Adds a label to the key. This allocates memory for the label that will
* be freed in gok_key_delete.
*
* returns: TRUE if the key was initialized, FALSE if not.
**/ 
gboolean gok_key_add_label (GokKey* pKey, gchar* pLabelText, guint level, guint group, const gchar *vmods)
{
	GokKeyLabel* pKeyLabel;
		
	g_assert (pKey != NULL);
	g_assert (pLabelText != NULL);
	
	if (pKey->pLabel == NULL)
	{
		gok_keylabel_new (pKey, pLabelText, level, group, vmods);
	}
	else
	{
		/* if the key already has a label with the same modifier then change it */
		pKeyLabel = pKey->pLabel;
		while (pKeyLabel != NULL)
		{
			if ((pKeyLabel->level == level) && (pKeyLabel->group == group))
			{
				g_free (pKeyLabel->Text);
				pKeyLabel->Text = (gchar*)g_malloc (strlen(pLabelText) + 1);
				strcpy (pKeyLabel->Text, pLabelText);
				
				break;
			}
			pKeyLabel = pKeyLabel->pLabelNext;
		}
		
		if (pKeyLabel == NULL)
		{
			gok_keylabel_new (pKey, pLabelText, level, group, vmods);
		}
	}
		
	return TRUE;
}

/**
* gok_key_change_label
* @pKey: Pointer to the key that gets the new label.
* @LabelText: The new label text.
*
* Changes the label displayed on the gok key.
**/
void gok_key_change_label (GokKey* pKey, gchar* LabelText)
{
	g_assert (pKey != NULL);
	g_assert (LabelText != NULL);
	
	gok_key_add_label (pKey, LabelText, 0, 0, NULL);
	
	/* change the label text */
	gok_key_set_button_label (pKey, LabelText);

	/* set the label 'name' */
	gok_key_set_button_name (pKey);	
}

static
int gok_key_get_label_length (GokKey* pKey)
{
	PangoLayout* pPangoLayout;
	PangoRectangle rectInk;
	PangoRectangle rectLogical;
	GtkLabel* pLabel;

	if ((pKey->pButton == NULL) ||
	    (GOK_BUTTON (pKey->pButton)->pLabel == NULL) ||
		(strlen (gok_key_get_label (pKey)) == 0))
	{
		return 0;
	}
	
	pLabel = GTK_LABEL(((GokButton*)pKey->pButton)->pLabel);
	g_assert (pLabel != NULL);
	
	pPangoLayout = gtk_label_get_layout (pLabel);
	pango_layout_get_pixel_extents (pPangoLayout, &rectInk, &rectLogical);

	return (rectLogical.x + MAX (rectLogical.width, rectInk.width));
}

static
int gok_key_get_label_height (GokKey* pKey)
{
	PangoLayout* pPangoLayout;
	PangoRectangle rectInk;
	PangoRectangle rectLogical;
	GtkLabel* pLabel;

	if ((pKey->pButton == NULL) ||
		(strlen (gok_key_get_label (pKey)) == 0))
	{
		return 0;
	}
	
	pLabel = GTK_LABEL(((GokButton*)pKey->pButton)->pLabel);
	
	if (pLabel != NULL) {
		pPangoLayout = gtk_label_get_layout (pLabel);
		pango_layout_get_pixel_extents (pPangoLayout, &rectInk, &rectLogical);
		return rectLogical.y + MAX (rectLogical.height, rectInk.height); 
	}
	else
		return 0;
}

/**
* gok_key_update_label
* @pKey: Pointer to the key that gets an updated label.
*
* Changes the key's label if the modifier state has changed.
**/
void gok_key_update_label (GokKey* pKey)
{
	gchar* pNewLabelText;
	const gchar* pOldLabelText = NULL;
	GtkWidget* pButtonLabel;
	
	if (pKey->pButton == NULL)
	{
		return;
	}

	/* get the key's label text */
	pNewLabelText = gok_key_get_label (pKey);
	
	/* get the current text displayed on the key */
	pButtonLabel = ((GokButton*)pKey->pButton)->pLabel;
	if (pButtonLabel != NULL) {
		pOldLabelText = gtk_label_get_text (GTK_LABEL(pButtonLabel));
	}
	if (pOldLabelText == NULL)
	{
		return;
	}
	
	/* if the new text is different from the old text then change it */
	if (strcmp (pNewLabelText, pOldLabelText) != 0)
	{
#ifdef GOK_UTF8_DEBUG	    
	    if (!g_utf8_validate (pNewLabelText, -1, NULL))
		fprintf (stderr, "INVALID UTF8 : %s\n", pNewLabelText);
	    fprintf (stderr, "setting button label: %s", pNewLabelText);
	    gok_key_print_utf8_debug (pNewLabelText);
#endif
		gtk_label_set_text (GTK_LABEL(pButtonLabel), pNewLabelText);
	}

	/* make sure the label still fits on the key */
	if ((gok_key_get_label_length (pKey) > 
	     (gok_data_get_key_width () * 
	      (pKey->Right - pKey->Left) - 2 * 
	      gok_key_get_default_border_width (pKey) - 1)) ||
	     (gok_key_get_label_height (pKey) >
	      (gok_data_get_key_height () *
	       (pKey->Bottom - pKey->Top) - 2 *
	       gok_key_get_default_border_height (pKey) - 1)) )
	{
		/* Note that this shrinks the label size for all mod states */
#ifdef GOK_DEBUG
		gok_log (stderr, "oversize label %s; group %d, size%d\n", 
			 pNewLabelText,
			 pKey->FontSizeGroup,
			 pKey->FontSize);
#endif
		gok_key_set_font_size (pKey,
				       gok_key_calculate_font_size (pKey, 
								    TRUE, TRUE));
	}
}

/**
* gok_key_get_image
* @pKey: Pointer to the key that you want the image for.
*
* Returns: A pointer to the key's GokKeyImage, or NULL if the key has no images.
**/
GokKeyImage* 
gok_key_get_image (GokKey* pKey)
{
	return pKey->pImage;
}

GtkWidget *
gok_key_status_image (GokKey *key) 
{
	GokUIState state = key->ComponentState;
	GtkWidget *image = NULL;
	gchar *icon_name, *file;

#if STATUS_IMAGES
	if (state.latched) {
		icon_name = "latched.png";
	}
	else if (state.locked) {
		icon_name = "locked.png";
	}
	else {
		icon_name = "empty.png";
	}
#else
	icon_name = "small-empty.png";
#endif
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_DATADIR,
					  icon_name,
					  FALSE, NULL);

	if (file) {
		image = gtk_image_new_from_file (file);
		g_free (file);
	}

	return image;
}
	

/**
* gok_key_create_image_widget
* @pKey: Pointer to the key that you want the image for.
*
* Returns: A GtkWidget pointer to a newly-created GtkImage, NULL if no image could be created.
**/
GtkWidget* 
gok_key_create_image_widget (GokKey* pKey)
{
	GtkWidget *image = NULL;
	gchar *file;
	if (pKey->pImage != NULL) {
		/* FIT and FILL aren't yet implemented, since they are dynamic */
		switch (pKey->pImage->type) {
		case IMAGE_TYPE_INDICATOR:
			image = gok_key_status_image (pKey);
			break;
		case IMAGE_TYPE_STOCK:
			image = gtk_image_new_from_stock (pKey->pImage->Filename,
							  GTK_ICON_SIZE_BUTTON /*pKey->pImage->stock_size*/);
			break;
		case IMAGE_TYPE_FIXED:
		default:
			if ((pKey->pImage->w == -1) && (pKey->pImage->h == -1)) {
				file = gok_key_get_image_filename (pKey);
				image = gtk_image_new_from_file (file);
				g_free (file);
			}
			else {
				GdkPixbuf *scaled = NULL, *pixbuf;
				file = gok_key_get_image_filename (pKey);
				pixbuf = gdk_pixbuf_new_from_file (file, NULL);
				g_free (file);
				if (pixbuf) {
					scaled = gdk_pixbuf_scale_simple (
						pixbuf, pKey->pImage->w,
						pKey->pImage->h, 
						GDK_INTERP_BILINEAR);
					g_object_unref (pixbuf);
				}
				if (scaled) {
					image = gtk_image_new_from_pixbuf (scaled);
					g_object_unref (scaled);
				}
			}
			break;
		}
	}
	return image;
}

/**
* gok_key_get_label
* @pKey: Pointer to the key that you want the label for.
*
* Returns: A pointer to the label's text string, NULL if no label.
**/
gchar* gok_key_get_label (GokKey* pKey)
{
	gchar* pTextReturned;
	gchar* pToken;
	gchar buffer[150];
	GokKeyLabel* pKeyLabel;
	gboolean bFoundLabel;
	int  type = XkbAlphabeticIndex, keycode;
	guint level;
	guint group = gok_key_get_effective_group ();
	guint mods = gok_spy_get_modmask ();
	XkbDescRec *xkb = gok_keyboard_get_xkb_desc ();
	GokOutputType output_type = OUTPUT_INVALID;

#ifdef GOK_DEBUG
	char *typename = XGetAtomName (gok_keyboard_get_display (), xkb->map->types[type].name);
#endif

	if (pKey->pOutput) output_type = pKey->pOutput->Type;
	switch (output_type)
	{
	    case OUTPUT_KEYCODE:
	    case OUTPUT_KEYSYM:
		keycode = gok_output_get_keycode (pKey->pOutput);
		if (keycode > 0)
		    type = gok_key_get_xkb_type_index (xkb, keycode, group);
#ifdef GOK_DEBUG
		fprintf (stderr, "keycode %d, group %d, type=%d [%s]\n", keycode, group, type, typename);
#endif
		level = gok_key_level_for_type (gok_keyboard_get_display (), 
						xkb,
						type, &mods);
#ifdef GOK_DEBUG
		if (pKey->pLabel) fprintf (stderr, "Key %s: type %d, level %d [mods %x, preserved %x]\n", 
					   pKey->pLabel->Text,
					   type, level, gok_spy_get_modmask (), mods);
#endif
		break;
	    case OUTPUT_KEYSTRING:
	    default:
		level = gok_key_level_for_type (gok_keyboard_get_display (), xkb, XkbAlphabeticIndex, &mods);
		break;
	}
		
	g_assert (pKey != NULL);
		
	pTextReturned = m_TextEmpty;
	
	/* get the 'normal' label */
	pKeyLabel = pKey->pLabel;
	while (pKeyLabel != NULL)
	{
		if ((pKeyLabel->level == 0) && (pKeyLabel->group == 0))
		{
			pTextReturned = pKeyLabel->Text;
			break;
		}
		pKeyLabel = pKeyLabel->pLabelNext;
	}

	/* are any modifiers turned on? */
	if ((gok_modifier_get_normal() == TRUE) && 
	    (gok_key_get_effective_group () == 0))
	{
		/* no modifiers, so just return the 'normal' label */
		return pTextReturned;
	}
	
	/* find the correct label text for the current level and group */
	bFoundLabel = FALSE;
	pKeyLabel = pKey->pLabel;
	while (pKeyLabel != NULL)
	{
		if (pKeyLabel->level == level && pKeyLabel->group == group) 
		{
			pTextReturned = pKeyLabel->Text;
			break;
		}
		pKeyLabel = pKeyLabel->pLabelNext;
	}
	
	return pTextReturned;
}

/**
* gok_key_set_output
* @pKey: Pointer to the key that's gets the new output.
* @Type: Type of output (e.g. keysym or keycode)
* @pName: Pointer to the name string.
* @Flag: Type of key synth output (if relevant)
*
* Sets the output for the key. This allocates memory for the output that will
* be freed in gok_key_delete.
**/ 
void gok_key_set_output (GokKey* pKey, gint Type, gchar* pName, AccessibleKeySynthType Flag)
{
	g_assert (pKey != NULL);
	
	gok_output_delete_all (pKey->pOutput);
	if (pName != NULL)
	{
		pKey->pOutput = gok_output_new (Type, pName, Flag);
	}
}

/**
* gok_key_add_output
* @pKey: Pointer to the key that's gets the new output.
* @Type: Type of output (e.g. keysym or keycode)
* @pName: Pointer to the name string.
* @Flag: Type of key synth output (if relevant)
*
* Adds output for the key. This allocates memory for the output that will
* be freed in gok_key_delete.
**/ 
void gok_key_add_output (GokKey* pKey, gint Type, gchar* pName, AccessibleKeySynthType Flag)
{
	GokOutput* pOutputPrevious;
	GokOutput* pOutputTemp = NULL;

	gok_log_enter();
	
	g_assert (pKey != NULL);
	
	if (pName != NULL)
	{
		pOutputPrevious = pOutputTemp;
		pOutputTemp = pKey->pOutput;
		while (pOutputTemp != NULL)
		{
			pOutputPrevious = pOutputTemp;
			pOutputTemp = pOutputTemp->pOutputNext;
		}
		
		pOutputTemp = gok_output_new (Type, pName, Flag);
		
		if (pOutputPrevious == NULL)
		{ 	
	    	pKey->pOutput = pOutputTemp; 
	    }
	    else
	    {
	    	pOutputPrevious->pOutputNext = pOutputTemp;
	    }
	}

	gok_log_leave();
}

/**
* gok_key_set_button_name
* @pKey: Pointer to the key that gets the new 'name'.
*
* Sets the 'name' of the key's label. The 'name' is used to determine
* the key/label colors from the .rc file. This must be called for every key
* after it's created and after the label name has been changed.
**/
void gok_key_set_button_name (GokKey* pKey)
{
	gchar styleButton[200];
	gchar styleText[200];
	
	switch (pKey->Style)
	{
	case KEYSTYLE_DOCK:
	case KEYSTYLE_NORMAL:
		strcpy (styleButton, "StyleButtonNormal");
		strcpy (styleText, "StyleTextNormal");
		break;
		
	case KEYSTYLE_BRANCH: 
		strcpy (styleButton, "StyleButtonBranch");
		strcpy (styleText, "StyleTextNormal");
		break;
	case KEYSTYLE_BRANCHMODAL:
		strcpy (styleButton, "StyleButtonBranch");
		strcpy (styleText, "StyleTextNormal");
		break;
	case KEYSTYLE_BRANCHBACK: 			
		/* disable the button if we can't branch back */
		if (gok_branchbackstack_is_empty() == TRUE)
		{
			strcpy (styleButton, "StyleButtonDisabled");
			strcpy (styleText, "StyleTextDisabled");
		}
		else
		{
			strcpy (styleButton, "StyleButtonBranchBack");
			strcpy (styleText, "StyleTextNormal");
		}
		break;

	case KEYSTYLE_GENERALDYNAMIC:
		strcpy (styleButton, "StyleButtonGeneralDynamic");
		strcpy (styleText, "StyleTextNormal");
		break;

	case KEYSTYLE_SELECT:
		strcpy (styleButton, "StyleButtonSelect");
		strcpy (styleText, "StyleTextNormal");
		break;
		
	case KEYSTYLE_BRANCHMENUS:
		strcpy (styleButton, "StyleButtonBranchMenus");
		strcpy (styleText, "StyleTextNormal");
		break;
		
	case KEYSTYLE_BRANCHMENUITEMS:
		strcpy (styleButton, "StyleButtonBranchMenuItems");
		strcpy (styleText, "StyleTextNormal");
		break;
		
	case KEYSTYLE_MENUITEM:
		strcpy (styleButton, "StyleButtonMenuItem");
		strcpy (styleText, "StyleTextNormal");
		break;

	case KEYSTYLE_BRANCHTOOLBARS:
		strcpy (styleButton, "StyleButtonBranchToolbars");
		strcpy (styleText, "StyleTextNormal");
		break;
		
	case KEYSTYLE_ABOUT: 
	case KEYSTYLE_BRANCHGUI:
		strcpy (styleButton, "StyleButtonBranchGUI");
		strcpy (styleText, "StyleTextNormal");
		break;

	case KEYSTYLE_PAGESELECTION:
		strcpy (styleButton, "StyleButtonBranchPageSelection");
		strcpy (styleText, "StyleTextNormal");
		break;

	case KEYSTYLE_BRANCHGUIACTIONS:
	case KEYSTYLE_EDIT:
	case KEYSTYLE_REPEATNEXT:
		strcpy (styleButton, "StyleButtonBranchGuiActions");
		strcpy (styleText, "StyleTextNormal");
		break;
		
	case KEYSTYLE_HYPERLINK:
		strcpy (styleButton, "StyleButtonHyperlink");
		strcpy (styleText, "StyleTextHyperlink");
		break;

	case KEYSTYLE_BRANCHHYPERTEXT:
		strcpy (styleButton, "StyleButtonHyperText");
		strcpy (styleText, "StyleTextNormal");
		break;

	case KEYSTYLE_HTMLACTION:
		strcpy (styleButton, "StyleButtonHtmlAction");
		strcpy (styleText, "StyleTextNormal");
		break;

	case KEYSTYLE_BRANCHALPHABET:
	case KEYSTYLE_BRANCHTEXT:
		strcpy (styleButton, "StyleButtonBranchAlphabet");
		strcpy (styleText, "StyleTextBranchAlphabet");
		break;
		
	case KEYSTYLE_SETTINGS:
		strcpy (styleButton, "StyleButtonSettings");
		strcpy (styleText, "StyleTextNormal");
		break;
					
	case KEYSTYLE_ADDWORD:
	case KEYSTYLE_HELP:
		strcpy (styleButton, "StyleButtonHelp");
		strcpy (styleText, "StyleTextHelp");
		break;
					
	case KEYSTYLE_POINTERCONTROL:
	case KEYSTYLE_WORDCOMPLETE:
	case KEYSTYLE_SPELL:
		strcpy (styleButton, "StyleButtonWordComplete");
		strcpy (styleText, "StyleTextWordComplete");
		break;
		
	case KEYSTYLE_INSENSITIVE:		
		strcpy (styleButton, "StyleButtonDisabled");
		strcpy (styleText, "StyleTextDisabled");
		break;
	default:
		gok_log_x ("Warning: default hit in gok_key_set_button_name! Key = %s\n", gok_key_get_label (pKey));
		strcpy (styleButton, "StyleButtonNormal");
		strcpy (styleText, "StyleTextNormal");
		break;
	}
	
	/* set the 'name' of the button */
	gtk_widget_set_name (pKey->pButton, styleButton); 

	/* set 'name' of the button text */
	if (((GokButton*)pKey->pButton)->pLabel != NULL)
		gtk_widget_set_name (((GokButton*)pKey->pButton)->pLabel, styleText);
}


/**
 * gok_key_update_toggle_state:
 * @pKey: pointer to the key to update.
 *
 **/
void
gok_key_update_toggle_state (GokKey *pKey)
{
	gok_log_enter();
	if (pKey && pKey->pButton && GTK_IS_TOGGLE_BUTTON (pKey->pButton)) {
		gboolean is_active = FALSE; 
		if (pKey->ModifierName != NULL) {
			is_active = 
			(gok_modifier_get_state (pKey->ModifierName) != MODIFIER_STATE_OFF);
		}
		else {
			is_active = pKey->ComponentState.active;
		}
		if (pKey->ComponentState.radio) {
			GOK_BUTTON (pKey->pButton)->indicator_type = /*g_strdup( ?*/ "radiobutton" /*)*/;
		}
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (pKey->pButton),
					      is_active);
	}
	gok_log_leave();
}

/**
* gok_key_calculate_font_size
* @pKey: Pointer to the key that we're getting the font size for.
* @bWidth: If TRUE then calculate the font size needed for the width
* of the key's label.
* @bHeight: If TRUE then calculate the font size needed for the height
* of the key's label.
* Note: Both bWidth and bHeight can be TRUE.
*
* Calculates the font size needed for the key's label to fill the key.
*
* Returns: The font size, in 1000s of a point (e.g. 9 point is 9000).
**/
int gok_key_calculate_font_size (GokKey* pKey, gboolean bWidth, gboolean bHeight)
{
	/* TODO: account for any icons/images on this key! */
	gint sizeFontWidth;
	gint sizeFontHeight;
	gint sizeFontReturned;
	gint sizeTemp;
	gint widthCell;
	gint widthTrial;
	gint heightCell;
	gint heightTrial;
	gint increment;
	gint maxFontSize = 200000;

	/* at least one must be TRUE */
	g_assert ((bWidth == TRUE) || (bHeight == TRUE));

	/* store the current size of the key */
	sizeTemp = pKey->FontSize;

	/* start at a small font size and increase to a very large size */
	/* font sizes are expressed in 1000th of a point (1000 = 1 point) */
	/* start at 5 points and go up to 50 points in increments of 1 point */

	if (pKey->FontSizeGroup == FONT_SIZE_GROUP_GLYPH) 
	  maxFontSize = 1500 * gok_key_get_default_font_size (pKey);
	else
	  maxFontSize = 1000 * gok_key_get_default_font_size (pKey);

	/* first check cell width */
	if (bWidth == TRUE)
	{
		/* get the size of a key cell*/
	        widthCell = gok_keyboard_get_cell_width (gok_main_get_current_keyboard ()) * 
		        (pKey->Right - pKey->Left) - 2 * 
		        gok_key_get_default_border_width (pKey) - 1;
	
		increment = 500;

		for (sizeFontWidth = GOK_MIN_FONT_SIZE; sizeFontWidth < maxFontSize; sizeFontWidth += increment)
		{
			/* set the key label to the trial size */
			gok_key_set_font_size (pKey, sizeFontWidth);
			
			/* get the size of the label at the trial size */
			widthTrial = gok_key_get_label_length (pKey);
			
			/* stop if the label fits the key cell exactly */
			if (widthTrial == widthCell)
			{
				break;
			}
			
			/* is it bigger than the cell size? */
			else if (widthTrial > widthCell)
			{
				/* label is larger than a cell so decrease it a bit */
				sizeFontWidth -= increment;
				
				/* stop looking */
				break;
			}
			
			/* at very large font sizes, increase the increment we increase */
			if (sizeFontWidth > 9000)
			{
				increment = 1000;
			}
			else if (sizeFontWidth > 50000)
			{
				increment = 2000;
			}
			else if (sizeFontWidth > 100000)
			{
				increment = 5000;
			}
		}
	}
	
	sizeFontReturned = sizeFontWidth;
	
	/* now check the cell height */
	if (bHeight == TRUE)
	{
	        increment = 500;
		/* get the size of a key cell*/
		heightCell = gok_data_get_key_height() - 2 *
		        gok_key_get_default_border_height (pKey) - 1;
	
		for (sizeFontHeight = GOK_MIN_FONT_SIZE; sizeFontHeight < maxFontSize; sizeFontHeight += increment)
		{
			/* set the key label to the trial size */
			gok_key_set_font_size (pKey, sizeFontHeight);
			
			/* get the size of the label at the trial size */
			heightTrial = gok_key_get_label_height (pKey);
			
			/* stop if the label fits the key cell exactly */
			if (heightTrial == heightCell)
			{
				break;
			}
			
			/* is it bigger than the cell size? */
			else if (heightTrial > heightCell)
			{
				/* label is larger than a cell so decrease it a bit */
				sizeFontHeight -= increment;
				
				/* stop looking */
				break;
			}
			
			/* at very large font sizes, increase the increment we increase */
			if (sizeFontHeight > 9000)
			{
				increment = 1000;
			}
			else if (sizeFontHeight > 50000)
			{
				increment = 2000;
			}
			else if (sizeFontHeight > 100000)
			{
				increment = 5000;
			}
		}

		/* did we also get the font width? */
		if (bWidth == TRUE)
		{
			sizeFontReturned = (sizeFontHeight < sizeFontWidth) ? sizeFontHeight : sizeFontWidth;
		}
		else
		{
			sizeFontReturned = sizeFontHeight;
		}
	}
	
	/* restore key size to original size */
	if (sizeTemp != -1)
	{
		gok_key_set_font_size (pKey, sizeTemp);
	}
	else
	{
		pKey->FontSize = -1;
	}
	
	return sizeFontReturned;
}

/**
* gok_key_set_font_size
* @pKey: Pointer to the key that gets the new font size.
* @Size: Font size you want the key's text to be.
*
* Sets the font size for the key.
**/
void gok_key_set_font_size (GokKey* pKey, gint Size)
{
	PangoAttrList* pPangoAttrs;
	GokButton* pButton;
	gchar* pLabel;
	gchar SafeLabel[351]; /* will hold the HTML safe version of the label */
	gchar string [400];
	g_assert (pKey != NULL);
	g_assert (pKey->pButton != NULL);

	if (Size <= 0)
	{
		Size = 200;
	}
	
	if (pKey->pLabel == NULL)
	{
		return;
	}

	/* store the size on the key */
	pKey->FontSize = Size;
	
	if (strlen (gok_key_get_label (pKey)) > 300)
	{
		gok_log_x ("Warning: gok_key_set_font_size failed because key label too long.\n"); 
		return;
	}

	/* get an HTML save version of the label */
	pLabel = gok_key_get_label (pKey);
	gok_key_make_html_safe (pLabel, SafeLabel, 350);
	
	/* create a list of Pango attributes */
	sprintf (string, "<span size=\"%d\">%s</span>", Size, SafeLabel);
	if (pango_parse_markup (string, -1, 0, &pPangoAttrs, NULL, NULL, NULL) == TRUE)
	{
		/* add the Pango atrribute list to the key's label */
		pButton = (GokButton*)pKey->pButton;
		if (pButton->pLabel)
			gtk_label_set_attributes (GTK_LABEL(pButton->pLabel), pPangoAttrs);
	}
	else
	{
		gok_log_x ("Warning: pango_parse_markup failed in gok_key_set_font_size! String = %s\n", string);
	}
}

/**
* gok_key_make_html_safe
* @pString: String that needs to be made HTML safe.
* @pSafeString: Pointer to a buffer that will hold the HTML safe string.
* @SafeStringLength: Length of the buffer that holds the HTML safe string.
*
* Converts a given string to an HTML safe string. This converts characters
* like '<' to &lt;
*
* Returns: TRUE if pSafeString is OK, FALSE if not.
**/
gboolean gok_key_make_html_safe (gchar* pString, gchar* pSafeString, gint SafeStringLength)
{
	gint i1, i2;
	
	for (i1 = 0, i2 = 0; i1 < strlen (pString); i1++)
	{
		if (pString[i1] == '<')
		{
			if (i2 > (SafeStringLength - 5))
			{
				gok_log_x ("pSafeString is too short to hold pString!");
				pSafeString[i2] = 0;
				return FALSE;
			}

			pSafeString[i2++] = '&';
			pSafeString[i2++] = 'l';
			pSafeString[i2++] = 't';
			pSafeString[i2++] = ';';
		}
		else if (pString[i1] == '>')
		{
			if (i2 > (SafeStringLength - 5))
			{
				gok_log_x ("pSafeString is too short to hold pString!");
				pSafeString[i2] = 0;
				return FALSE;
			}
			pSafeString[i2++] = '&';
			pSafeString[i2++] = 'g';
			pSafeString[i2++] = 't';
			pSafeString[i2++] = ';';
		}
		else if (pString[i1] == '&')
		{
			if (i2 > (SafeStringLength - 6))
			{
				gok_log_x ("pSafeString is too short to hold pString!");
				pSafeString[i2] = 0;
				return FALSE;
			}
			pSafeString[i2++] = '&';
			pSafeString[i2++] = 'a';
			pSafeString[i2++] = 'm';
			pSafeString[i2++] = 'p';
			pSafeString[i2++] = ';';
		}
		else if (pString[i1] == 34) /* double quote mark */
		{
			if (i2 > (SafeStringLength - 7))
			{
				gok_log_x ("pSafeString is too short to hold pString!");
				pSafeString[i2] = 0;
				return FALSE;
			}
			pSafeString[i2++] = '&';
			pSafeString[i2++] = 'q';
			pSafeString[i2++] = 'u';
			pSafeString[i2++] = 'o';
			pSafeString[i2++] = 't';
			pSafeString[i2++] = ';';
		}
		else
		{
			if (i2 > (SafeStringLength - 2))
			{
				gok_log_x ("pSafeString is too short to hold pString!");
				pSafeString[i2] = 0;
				return FALSE;
			}

			pSafeString[i2++] = pString[i1];
		}
	}
	
	pSafeString[i2] = 0;
	return TRUE;
}

/**
* gok_key_get_label_lengthpercell:
* @pKey: Pointer to the key you want the to find the label length per cell.
*
* Calculates the length of the key's label. Some keys span more
* than one cell so divide the label length into the number of cells.
*
* Returns: The length of the key's label per cell.
**/
int gok_key_get_label_lengthpercell (GokKey* pKey)
{
        gint numCells;

	if (pKey->pButton == NULL) 
		return 0;
	
	numCells = pKey->Right - pKey->Left;
	if (numCells <= 0) return 10;
	return gok_key_get_label_length (pKey) / numCells;
}

/**
* gok_key_get_label_heightpercell:
* @pKey: Pointer to the key you want to find the height.
*
* Calculates the height of the key's label. Some keys span more
* than one cell so divide the label height into the number of cells.
*
* Returns: The height of the key's label per cell.
**/
int gok_key_get_label_heightpercell (GokKey* pKey)
{
        gint numCells;

	if (pKey->pButton == NULL) 
		return 0;
	
	numCells = pKey->Bottom - pKey->Top;
	return gok_key_get_label_height (pKey) / numCells;
}

/**
* gok_key_set_button_label
* @pKey: Pointer to the key that will have it's button label changed.
* @LabelText: Text for the button.
*
* Changes the button label displayed on the key.
**/
void gok_key_set_button_label (GokKey* pKey, gchar* LabelText)
{
	GokButton* pButton;
	GtkWidget* pButtonLabel;

	g_assert (pKey != NULL);
	
	pButton = (GokButton*)pKey->pButton;
	if (pButton == NULL)
	{
		return;
	}

	pButtonLabel = pButton->pLabel;
	if (pButtonLabel == NULL)
	{
		return;
	}
	
	gtk_label_set_text (GTK_LABEL(pButtonLabel), LabelText);
}


/**
* gok_key_set_cells
* @pKey: Key that gets it's cells changed.
* @top: Top cell for the key.
* @bottom: Bottom cell for the key.
* @left: Left cell for the key.
* @right: Right cell for the key.
*
* Changes the cell coordinates (used by the editor).
**/
void gok_key_set_cells(GokKey* pKey, gint top, gint bottom, gint left, gint right)
{
	g_assert (pKey != NULL);
	g_assert (left < right);
	g_assert (top < bottom);
	pKey->Top = top;
	pKey->Bottom = bottom;
	pKey->Left = left;
	pKey->Right = right;
	
	/* update the button */
}

/**
* gok_key_duplicate
* @pKey: Pointer to the key that gets duplicated
*
* Not implemented yet.
*
* Returns: A pointer to the duplicate key, NULL if it was not created.
**/
GokKey* gok_key_duplicate(GokKey* pKey)
{
	return NULL;
}

/**
* gok_key_contains_point
* @pKey: Pointer to the key to test
* @x: x coordinate in keyboard window coordinates.
* @y: y coordinate in keyboard window coordinates.
*
* Checks to see if a key contains a given point.
*
* Returns: %TRUE if the key contains the point, %FALSE otherwise.
**/
gboolean gok_key_contains_point (GokKey* pKey, gint x, gint y)
{
	if (pKey) {
		if ((pKey->TopWin <= y) && (pKey->BottomWin >= y)
			&& (pKey->LeftWin <= x) && (pKey->RightWin >= x))
			return TRUE;
	}
	return FALSE;
}

/**
* gok_keyimage_new
* @pKey: Pointer to the key that gets the new image.
* @pFilename: Filename containing the image.
*
* Allocates memory for a new key image and initialises the
* GokKeyImage structure.  Returns a pointer to the new key image,
* NULL if it can't be created.  Add this image to a key so it will be
* deleted when the key is deleted.
*
* returns: A pointer to the new key image, NULL if it wasn't created.
**/
GokKeyImage*
gok_keyimage_new (GokKey* pKey, gchar* pFilename)
{
	GokKeyImage* pNewImage;
	GokKeyImage* pKeyImage;

	gok_log_enter();
	pNewImage = (GokKeyImage*) g_malloc (sizeof (GokKeyImage));

	pNewImage->Filename = NULL;
	pNewImage->pImageNext = NULL;
        /* "unspecified", i.e. scalable or context-dependent */
	pNewImage->w = pNewImage->h = -1; 
	
	if (pFilename != NULL)
	{
		pNewImage->Filename = (gchar*) g_malloc (strlen (pFilename) + 1);
		strcpy (pNewImage->Filename, pFilename);
	}
	
	if (pKey != NULL)
	{
		pKeyImage = pKey->pImage;
		if (pKeyImage == NULL)
		{
			pKey->pImage = pNewImage;
		}
		else
		{
			while (pKeyImage->pImageNext != NULL)
			{
				pKeyImage = pKeyImage->pImageNext;
			}
			pKeyImage->pImageNext = pNewImage;
		}
		pKey->has_image = TRUE;
	}
	
	gok_log_leave();
	return pNewImage;
}
	 
/**
* gok_keyimage_delete
* @pKeyImage: Pointer to the key image that will be deleted.
**/
void
gok_keyimage_delete (GokKeyImage* pKeyImage)
{
	if (pKeyImage != NULL)
	{
		if (pKeyImage->Filename != NULL)
		{
			g_free (pKeyImage->Filename);
		}
		g_free (pKeyImage);
	}
}

gchar*
gok_key_get_image_filename (GokKey* pKey)
{
	gchar *filename = NULL;
	g_assert (pKey != NULL);

	filename = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_DATADIR,
					      pKey->pImage->Filename,
					      FALSE, NULL);
	return filename;
}

/**
* gok_key_isRepeatable
* @pKey: Pointer to the key to assess..
*
* Call this to see if the key is repeatable.
*
* returns: gboolean
**/
gboolean 
gok_key_isRepeatable(GokKey* pKey)
{
	gboolean returncode = FALSE;
	gok_log_enter();
	if (pKey != NULL)
	{
		if ( pKey->is_repeatable ) {
			returncode = TRUE;
			gok_log("this key is repeatable");
		}		
	}
	gok_log_leave();
	return returncode;
}

/* 
 * the reason for this static: GokOutput is not a GObject, 
 * so we have no ref counting ability.
 * therefore we only delete a wordcomplete output when creating a new one. 
 * It would be better to do proper ref counting.
 */
static GokOutput *last_wordcomplete_output = NULL;

/**
* gok_key_wordcomplete_output
* 
* Gets the output for a word prediction key.
*
* @pKey: Pointer to the word completion key that will be output.
*
* returns: A pointer to the output.
**/
GokOutput* 
gok_key_wordcomplete_output (GokKey *pKey, GokWordComplete *complete)
{
	gchar* LabelText;
	gchar *word_part = gok_wordcomplete_get_word_part (complete);
	g_assert (pKey != NULL);

	/* make sure this is a word completion key */
	if (pKey->Type != KEYTYPE_WORDCOMPLETE)
	{
		gok_log_x ("Hey, this is not a word completion key!\n");
		return NULL;
	}
	
	/* subtract the part word from the output */
	LabelText = gok_key_get_label (pKey);
	if (LabelText == NULL)
	{
		return NULL;
	}

	if (last_wordcomplete_output) {
	    gok_output_delete_all (last_wordcomplete_output);
	    last_wordcomplete_output = NULL;
	}

	if (word_part && (g_utf8_strlen (word_part, -1) < g_utf8_strlen (LabelText, -1)))
	{
		gchar *cp, *word_part_canonical;

		word_part_canonical = g_utf8_normalize (word_part, -1, G_NORMALIZE_DEFAULT);
		cp = strstr (LabelText, word_part_canonical); 
		/* sanity check: is our word_part found in our label? */
		if (cp) {
			gint offset = strlen (word_part_canonical); /* in bytes, not chars */
			last_wordcomplete_output = gok_output_new (OUTPUT_KEYSTRING, cp + offset, SPI_KEY_STRING);
		}
		else {
			/* if not then we are doing a 'replace', handle that via EditableText. */
			g_warning ("Sorry, string replacement is not yet implemented.");
		}
		g_free (word_part_canonical);
	}

	return last_wordcomplete_output;
}
