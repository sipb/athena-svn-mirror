/* gok-modifier-keymasks.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gok-modifier-keymasks.h"
#include <libspi/keymasks.h>

const struct gok_modifier_keymask gok_modifier_keymasks[] = {
	{ "shift",    SPI_KEYMASK_SHIFT },
	{ "capslock", SPI_KEYMASK_SHIFTLOCK },
	{ "ctrl",     SPI_KEYMASK_CONTROL },
	{ "alt",      SPI_KEYMASK_ALT },
	{ "mod2",     SPI_KEYMASK_MOD2 },
	{ "mod3",     SPI_KEYMASK_MOD3 },
	{ "mod4",     SPI_KEYMASK_MOD4 },
	{ "mod5",     SPI_KEYMASK_MOD5 },
	{ NULL,       0 }
};
