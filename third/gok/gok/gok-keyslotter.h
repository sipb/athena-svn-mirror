/* gok-keyslotter.h
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

#ifndef __GOKKEYSLOTTER_H__
#define __GOKKEYSLOTTER_H__

#include "gok-chunker.h"
#include "gok-keyboard.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

gboolean gok_keyslotter_on (gboolean bOnOff, int keytype);
void gok_keyslotter_change_number_predictions (int Number, int keytype);
gboolean gok_keyslotter_add_prediction_keys (GokKeyboard* pKeyboard, int keytype);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKKEYSLOTTER_H__ */
