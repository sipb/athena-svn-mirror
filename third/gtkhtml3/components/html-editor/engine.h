/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of gnome-spell bonobo component

    Copyright (C) 2000 Helix Code, Inc.
    Authors:           Radek Doulik <rodo@helixcode.com>

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
*/

#ifndef ENGINE_H_
#define ENGINE_H_

G_BEGIN_DECLS

typedef struct _EditorEngine EditorEngine;

#include <gtk/gtktypeutils.h>
#include <bonobo/bonobo-object.h>
#include "Editor.h"
#include "control-data.h"
#include "gtkhtml.h"

#define EDITOR_ENGINE_TYPE        (editor_engine_get_type ())
#define EDITOR_ENGINE(o)          (GTK_CHECK_CAST ((o), EDITOR_ENGINE_TYPE, EditorEngine))
#define EDITOR_ENGINE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), EDITOR_ENGINE_TYPE, EditorEngineClass))
#define IS_EDITOR_ENGINE(o)       (GTK_CHECK_TYPE ((o), EDITOR_ENGINE_TYPE))
#define IS_EDITOR_ENGINE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), EDITOR_ENGINE_TYPE))

struct _EditorEngine {
	BonoboObject parent;

	GtkHTMLControlData *cd;

	GNOME_GtkHTML_Editor_Listener listener;
};

typedef struct {
	BonoboObjectClass parent_class;
	POA_GNOME_GtkHTML_Editor_Engine__epv epv;
} EditorEngineClass;

GtkType                               editor_engine_get_type   (void);
EditorEngine                         *editor_engine_new        (GtkHTMLControlData          *cd);
POA_GNOME_GtkHTML_Editor_Engine__epv *editor_engine_get_epv    (void);

G_END_DECLS

#endif /* ENGINE_H_ */
