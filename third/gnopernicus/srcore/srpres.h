/* srpres.h
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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

#ifndef _SRPRES_H_
#define _SRPRES_H_

#include "SRObject.h"
#include "glib.h"

gboolean src_presentation_init 	    (gchar *fname);
gboolean src_presentation_terminate ();

gchar* src_presentation_sro_get_pres_for_speech    (SRObject *obj, const gchar *reason);
gchar* src_presentation_sro_get_pres_for_magnifier (SRObject *obj, const gchar *reason);
gchar* src_presentation_sro_get_pres_for_braille   (SRObject *obj, const gchar *reason);
gchar* src_presentation_sro_get_pres_for_device    (SRObject *obj, const gchar *reason,
						     const gchar *device);

G_CONST_RETURN gchar* src_pres_get_role_name_for_speech (const gchar *role);

#endif /* _SRPRES_H_ */
