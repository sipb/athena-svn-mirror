/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc. 2001, 2002 Ximian Inc.

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

    Author: Ettore Perazzoli <ettore@helixcode.com>
            Radek Doulik <rodo@ximian.com>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <bonobo/bonobo-shlib-factory.h>
#include <libgnome/gnome-i18n.h>

#include "editor-control-factory.h"

static void
editor_shlib_init (void)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		initialized = TRUE;

		/* Initialize the i18n support */
		bindtextdomain (GNOME_EXPLICIT_TRANSLATION_DOMAIN, GNOMELOCALEDIR);
		bind_textdomain_codeset (GNOME_EXPLICIT_TRANSLATION_DOMAIN, "UTF-8");
	}
}

BonoboObject *
editor_control_shlib_factory (BonoboGenericFactory *factory, const gchar *component_id, gpointer closure)
{
	editor_shlib_init ();

	return editor_control_factory (factory, component_id, closure);
}

BONOBO_ACTIVATION_SHLIB_FACTORY (CONTROL_FACTORY_ID, "GNOME HTML Editor factory", editor_control_shlib_factory, NULL);
