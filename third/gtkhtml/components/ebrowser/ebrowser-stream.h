#ifndef _EBROWSER_STREAM_H_
#define _EBROWSER_STREAM_H_

/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Lauris Kaplinski  <lauris@helixcode.com>
*/

#include <bonobo.h>

BEGIN_GNOME_DECLS

void ebrowser_ps_load (BonoboPersistStream * ps,
		       Bonobo_Stream stream,
		       Bonobo_Persist_ContentType type,
		       gpointer data,
		       CORBA_Environment * ev);
Bonobo_Persist_ContentTypeList * ebrowser_ps_types (BonoboPersistStream * ps,
						    gpointer data,
						    CORBA_Environment * ev);

END_GNOME_DECLS

#endif
