#ifndef _ELOADER_MONIKER_H_
#define _ELOADER_MONIKER_H_

/*  This file is part of the GtkHTML library.

    Copyright (C) 2001 Ximian, Inc.

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

    Author: Radek Doulik  <rodo@ximian.com>
*/

#include <bonobo.h>
#include "eloader.h"

BEGIN_GNOME_DECLS

#define ELOADER_MONIKER_TYPE (eloader_moniker_get_type ())
#define ELOADER_MONIKER(o) (GTK_CHECK_CAST ((o), ELOADER_MONIKER_TYPE, ELoaderMoniker))
#define IS_LOADER_MONIKER(o) (GTK_CHECK_TYPE ((o), ELOADER_MONIKER_TYPE))

typedef struct _ELoaderMoniker ELoaderMoniker;
typedef struct _ELoaderMonikerClass ELoaderMonikerClass;

struct _ELoaderMoniker {
	ELoader loader;

	Bonobo_Unknown stream;
	guint idle_id;

	gchar *url;
};

struct _ELoaderMonikerClass {
	ELoaderClass parent_class;
};

GtkType eloader_moniker_get_type (void);

ELoader * eloader_moniker_new (EBrowser * ebr, const gchar * path, GtkHTMLStream * stream);

END_GNOME_DECLS

#endif
