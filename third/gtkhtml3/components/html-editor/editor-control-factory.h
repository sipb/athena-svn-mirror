/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2002 Ximian, Inc.

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

    Author: Radek Doulik

*/

#ifndef EDITOR_CONTROL_FACTORY_H_
#define EDITOR_CONTROL_FACTORY_H_

#include <config.h>
#include <bonobo.h>

#define CONTROL_FACTORY_ID "OAFIID:GNOME_GtkHTML_Editor_Factory:" EDITOR_API_VERSION
#define CONTROL_ID         "OAFIID:GNOME_GtkHTML_Editor:" EDITOR_API_VERSION

BonoboObject *editor_control_factory (BonoboGenericFactory *factory, const gchar *component_id, gpointer closure);

#endif
