/* spgscb.h
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
#ifndef _SPGSCB_H_
#define _SPGSCB_H_


#include <gnome-speech/gnome-speech.h>

typedef void (*SRSGSMarkersCallback) (GNOME_Speech_speech_callback_type type, 
					CORBA_long text_id, CORBA_long offset);
gboolean srs_gs_cb_register_callback (GNOME_Speech_Speaker speaker,
					    SRSGSMarkersCallback callback);

#endif  /* _SPGSCB_H_ */
