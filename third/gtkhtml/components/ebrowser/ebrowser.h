#ifndef _EBROWSER_H_
#define _EBROWSER_H_

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

#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

#define EBROWSER_FACTORY_OAFIID "OAFIID:GNOME_GtkHTML_EBrowserFactory"
#define EBROWSER_OAFIID "OAFIID:GNOME_GtkHTML_EBrowser"

void ebrowser_factory_init (void);

END_GNOME_DECLS

#endif
