/* gok-modifier-keymasks.h
 *
 * Copyright 2003 Sun Microsystems, Inc.,
 * Copyright 2003 University Of Toronto
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

#ifndef __GOK_MODIFIER_KEYMASKS_H__
#define __GOK_MODIFIER_KEYMASKS_H__

#include <gnome.h>

struct gok_modifier_keymask {
	gchar *name;
	int keymask;
};

/* A list of (modifier name, libspi keymask) pairs terminated by an
 * entry (NULL, 0) that relates the modifier names used in gok
 * keyboard XML to libspi keymasks.  The contents of
 * gok_modifier_keymasks is defined in gok-modifier-keymasks.c
 */

extern const struct gok_modifier_keymask gok_modifier_keymasks[];

#endif /* #ifndef __GOK_MODIFIER_KEYMASKS_H__ */
