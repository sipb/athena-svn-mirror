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

#include <config.h>
#include <bonobo.h>

#include "gtkhtml.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmltext.h"
#include "htmltype.h"
#include "htmlundo.h"

#include "engine.h"

static BonoboObjectClass *engine_parent_class;
static POA_GNOME_GtkHTML_Editor_Engine__vepv engine_vepv;

inline static EditorEngine *
html_editor_engine_from_servant (PortableServer_Servant servant)
{
	return EDITOR_ENGINE (bonobo_object_from_servant (servant));
}

static CORBA_char *
impl_get_paragraph_data (PortableServer_Servant servant, const CORBA_char * key, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);
	CORBA_char * value = CORBA_OBJECT_NIL;

	if (e->html->engine->cursor->object && e->html->engine->cursor->object->parent
	    && HTML_IS_CLUEFLOW (e->html->engine->cursor->object->parent))
		value = html_object_get_data (e->html->engine->cursor->object->parent, key);

	/* printf ("get paragraph data\n"); */

	return CORBA_string_dup (value ? value : "");
}

static void
impl_set_paragraph_data (PortableServer_Servant servant,
			 const CORBA_char * key, const CORBA_char * value,
			 CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	if (e->html->engine->cursor->object && e->html->engine->cursor->object->parent
	    && HTML_OBJECT_TYPE (e->html->engine->cursor->object->parent) == HTML_TYPE_CLUEFLOW)
		html_object_set_data (HTML_OBJECT (e->html->engine->cursor->object->parent), key, value);
}

static void
impl_set_object_data_by_type (PortableServer_Servant servant,
			 const CORBA_char * type_name, const CORBA_char * key, const CORBA_char * value,
			 CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	/* printf ("set data by type\n"); */

	/* FIXME should use bonobo_arg_to_gtk, but this seems to be broken */
	html_engine_set_data_by_type (e->html->engine, html_type_from_name (type_name), key, value);
}

static void
unref_listener (EditorEngine *e)
{
	if (e->listener_client != CORBA_OBJECT_NIL)
		bonobo_object_unref (BONOBO_OBJECT (e->listener_client));
}

static void
impl_set_listener (PortableServer_Servant servant, const GNOME_GtkHTML_Editor_Listener value, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	/* printf ("set listener\n"); */

	unref_listener (e);
	e->listener_client = bonobo_object_client_from_corba (value);
	e->listener        = bonobo_object_client_query_interface (e->listener_client,
								   "IDL:GNOME/GtkHTML/Editor/Listener:1.0", ev);
}

static GNOME_GtkHTML_Editor_Listener
impl_get_listener (PortableServer_Servant servant, CORBA_Environment * ev)
{
	return html_editor_engine_from_servant (servant)->listener;
}

static CORBA_boolean
impl_run_command (PortableServer_Servant servant, const CORBA_char * command, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	/* printf ("command: %s\n", command); */

	return gtk_html_command (e->html, command);
}

static CORBA_boolean
impl_is_paragraph_empty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	if (e->html->engine->cursor->object
	    && e->html->engine->cursor->object->parent
	    && HTML_OBJECT_TYPE (e->html->engine->cursor->object->parent) == HTML_TYPE_CLUEFLOW) {
		return html_clueflow_is_empty (HTML_CLUEFLOW (e->html->engine->cursor->object->parent));
	}
	return CORBA_FALSE;
}

static CORBA_boolean
impl_is_previous_paragraph_empty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	if (e->html->engine->cursor->object
	    && e->html->engine->cursor->object->parent
	    && e->html->engine->cursor->object->parent->prev
	    && HTML_IS_CLUEFLOW (e->html->engine->cursor->object->parent->prev))
		return html_clueflow_is_empty (HTML_CLUEFLOW (e->html->engine->cursor->object->parent->prev));

	return CORBA_FALSE;
}

static void
impl_insert_html (PortableServer_Servant servant, const CORBA_char * html, CORBA_Environment * ev)
{
	/* printf ("impl_insert_html\n"); */
	gtk_html_insert_html (html_editor_engine_from_servant (servant)->html, html);
}

static CORBA_boolean
impl_search_by_data (PortableServer_Servant servant, const CORBA_long level, const CORBA_char * klass,
		     const CORBA_char * key, const CORBA_char * value, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);
	HTMLObject *o, *lco = NULL;
	gchar *o_value;

	do {
		if (e->html->engine->cursor->object != lco) {
			o = html_object_nth_parent (e->html->engine->cursor->object, level);
			if (o) {
				o_value = html_object_get_data (o, key);
				if (o_value && !strcmp (o_value, value))
					return TRUE;
			}
		}
		lco = e->html->engine->cursor->object;
	} while (html_cursor_forward (e->html->engine->cursor, e->html->engine));
	return FALSE;
}

static void
impl_freeze (PortableServer_Servant servant, CORBA_Environment * ev)
{
	html_engine_freeze (html_editor_engine_from_servant (servant)->html->engine);
}

static void
impl_thaw (PortableServer_Servant servant, CORBA_Environment * ev)
{
	html_engine_thaw (html_editor_engine_from_servant (servant)->html->engine);
}

static void
impl_undo_begin (PortableServer_Servant servant, const CORBA_char * undo_name, const CORBA_char * redo_name,
		 CORBA_Environment * ev)
{
	html_undo_level_begin (html_editor_engine_from_servant (servant)->html->engine->undo, undo_name, redo_name);
}

static void
impl_undo_end (PortableServer_Servant servant, CORBA_Environment * ev)
{
	html_undo_level_end (html_editor_engine_from_servant (servant)->html->engine->undo);
}

POA_GNOME_GtkHTML_Editor_Engine__epv *
editor_engine_get_epv (void)
{
	POA_GNOME_GtkHTML_Editor_Engine__epv *epv;

	epv = g_new0 (POA_GNOME_GtkHTML_Editor_Engine__epv, 1);

	epv->_set_listener            = impl_set_listener;
	epv->_get_listener            = impl_get_listener;
	epv->setParagraphData         = impl_set_paragraph_data;
	epv->getParagraphData         = impl_get_paragraph_data;
	epv->setObjectDataByType      = impl_set_object_data_by_type;
	epv->runCommand               = impl_run_command;
	epv->isParagraphEmpty         = impl_is_paragraph_empty;
	epv->isPreviousParagraphEmpty = impl_is_previous_paragraph_empty;
	epv->searchByData             = impl_search_by_data;
	epv->insertHTML               = impl_insert_html;
	epv->freeze                   = impl_freeze;
	epv->thaw                     = impl_thaw;
	epv->undo_begin               = impl_undo_begin;
	epv->undo_end                 = impl_undo_end;

	return epv;
}

static void
init_engine_corba_class (void)
{
	engine_vepv.Bonobo_Unknown_epv    = bonobo_object_get_epv ();
	engine_vepv.GNOME_GtkHTML_Editor_Engine_epv = editor_engine_get_epv ();
}

static void
engine_object_init (GtkObject *object)
{
	EditorEngine *e = EDITOR_ENGINE (object);

	e->listener_client = CORBA_OBJECT_NIL;
}

static void
engine_object_destroy (GtkObject *object)
{
	EditorEngine *e = EDITOR_ENGINE (object);

	unref_listener (e);

	GTK_OBJECT_CLASS (engine_parent_class)->destroy (object);
}

static void
engine_class_init (EditorEngineClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	engine_parent_class   = gtk_type_class (bonobo_object_get_type ());
	object_class->destroy = engine_object_destroy;

	init_engine_corba_class ();
}

GtkType
editor_engine_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"EditorEngine",
			sizeof (EditorEngine),
			sizeof (EditorEngineClass),
			(GtkClassInitFunc) engine_class_init,
			(GtkObjectInitFunc) engine_object_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

EditorEngine *
editor_engine_construct (EditorEngine *engine, GNOME_GtkHTML_Editor_Engine corba_engine)
{
	g_return_val_if_fail (engine != NULL, NULL);
	g_return_val_if_fail (IS_EDITOR_ENGINE (engine), NULL);
	g_return_val_if_fail (corba_engine != CORBA_OBJECT_NIL, NULL);

	engine->listener_client = CORBA_OBJECT_NIL;

	if (!bonobo_object_construct (BONOBO_OBJECT (engine), (CORBA_Object) corba_engine))
		return NULL;

	return engine;
}

static GNOME_GtkHTML_Editor_Engine
create_engine (BonoboObject *engine)
{
	POA_GNOME_GtkHTML_Editor_Engine *servant;
	CORBA_Environment ev;

	servant = (POA_GNOME_GtkHTML_Editor_Engine *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &engine_vepv;

	CORBA_exception_init (&ev);
	POA_GNOME_GtkHTML_Editor_Engine__init ((PortableServer_Servant) servant, &ev);
	ORBIT_OBJECT_KEY(servant->_private)->object = NULL;

	if (ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return (GNOME_GtkHTML_Editor_Engine) bonobo_object_activate_servant (engine, servant);
}

EditorEngine *
editor_engine_new (GtkHTML *html)
{
	EditorEngine *engine;
	GNOME_GtkHTML_Editor_Engine corba_engine;

	engine = gtk_type_new (EDITOR_ENGINE_TYPE);
	engine->html = html;

	corba_engine = create_engine (BONOBO_OBJECT (engine));

	if (corba_engine == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (engine));
		return NULL;
	}
	
	return editor_engine_construct (engine, corba_engine);
}
