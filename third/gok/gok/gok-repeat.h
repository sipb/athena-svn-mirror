/* gok-repeat.h
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

#ifndef __GOKREPEAT_H__
#define __GOKREPEAT_H__

#include <gnome.h>
#include "gok-key.h"

gboolean gok_repeat_getArmed(void);
gboolean gok_repeat_getStarted(void);
void gok_repeat_arm(GokKey *pKey);
void gok_repeat_stop(void);
void gok_repeat_disarm(void);
gboolean gok_repeat_toggle_armed (GokKey *key);
gboolean gok_repeat_key(GokKey* pKey);
void gok_repeat_drop_refs (GokKey *key);

#endif /*__GOKREPEAT_H__*/
