/* gok-branchback-stack.h
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2001 University Of Toronto
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

#ifndef __BRANCHBACKSTACK_H__
#define __BRANCHBACKSTACK_H__

#include "gok-keyboard.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* number of keyboards in the branch back stack */
#define MAX_BRANCHBACKSTACK 50

void gok_branchbackstack_initialize (void);
void gok_branchbackstack_push (GokKeyboard* pKeyboard);
GokKeyboard* gok_branchbackstack_pop (void);
gboolean gok_branchbackstack_is_empty (void);
gint gok_branchbackstack_pushes_minus_pops (void);
gboolean gok_branchbackstack_contains (GokKeyboard* pKeyboard);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __BRANCHBACKSTACK_H__ */
