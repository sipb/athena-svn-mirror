/* gok-modifier.h
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

#ifndef __GOKMODIFIER_H__
#define __GOKMODIFIER_H__

#include <gnome.h>
#include "gok-output.h"
#include "gok-key.h"
#include "gok-keyboard.h"

/* modifier name */
#define MODIFIER_NORMAL ""

/* modifier state */
typedef enum {
MODIFIER_STATE_OFF,
MODIFIER_STATE_ON,
MODIFIER_STATE_LOCKED
} ModifierStates;

/* modifier types */
typedef enum {
/* a normal modifier like shift or CTRL */
MODIFIER_TYPE_NORMAL,
/* a toggle type of modifier like Capslock */
MODIFIER_TYPE_TOGGLE
} ModifierTypes;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* a modifier */
/* if you add data members to this structure, initialize them in gok_modifier_add */
typedef struct GokModifier{
	gint State;
	gchar* Name;
	int Type;
	GokOutput* WrapperPre;
	GokOutput* WrapperPost;
	struct GokModifier* pModifierNext;	
} GokModifier;

void gok_modifier_open (void);
void gok_modifier_close (void);
gboolean gok_modifier_add (gchar* Name);
gboolean gok_modifier_set_pre (gchar* Name, GokOutput* pOutput);
gboolean gok_modifier_set_post (gchar* Name, GokOutput* pOutput);
void gok_modifier_set_type (gchar* Name, gint Type);
void gok_modifier_output_pre (void);
void gok_modifier_output_post (void);
void gok_modifier_all_off (void);
int gok_modifier_get_state (gchar* pNameModifier);
int gok_modifier_get_type (gchar* pNameModifier);
gboolean gok_modifier_get_normal (void);
void gok_modifier_update_modifier_keys (GokKeyboard* pKeyboard);
guint gok_modifier_mask_for_name (Display *display, char *modifier_name);
gchar* gok_modifier_first_name_from_mask (guint modmask);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKMODIFIER_H__ */
