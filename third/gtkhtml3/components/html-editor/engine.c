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
#include <libgnome/gnome-i18n.h>
#include <string.h>
#include <bonobo.h>

#include "gtkhtml.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmltext.h"
#include "htmltype.h"
#include "htmlundo.h"

#include "engine.h"
#include "spell.h"

static BonoboObjectClass *engine_parent_class;

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

	if (e->cd->html->engine->cursor->object && e->cd->html->engine->cursor->object->parent
	    && HTML_IS_CLUEFLOW (e->cd->html->engine->cursor->object->parent))
		value = html_object_get_data (e->cd->html->engine->cursor->object->parent, key);

	/* printf ("get paragraph data\n"); */

	return CORBA_string_dup (value ? value : "");
}

static void
impl_set_paragraph_data (PortableServer_Servant servant,
			 const CORBA_char * key, const CORBA_char * value,
			 CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	if (e->cd->html->engine->cursor->object && e->cd->html->engine->cursor->object->parent
	    && HTML_OBJECT_TYPE (e->cd->html->engine->cursor->object->parent) == HTML_TYPE_CLUEFLOW)
		html_object_set_data (HTML_OBJECT (e->cd->html->engine->cursor->object->parent), key, value);
}

static void
impl_set_object_data_by_type (PortableServer_Servant servant,
			 const CORBA_char * type_name, const CORBA_char * key, const CORBA_char * value,
			 CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	/* printf ("set data by type\n"); */

	/* FIXME should use bonobo_arg_to_gtk, but this seems to be broken */
	html_engine_set_data_by_type (e->cd->html->engine, html_type_from_name (type_name), key, value);
}

static void
impl_set_listener (PortableServer_Servant servant, const GNOME_GtkHTML_Editor_Listener value, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	/* printf ("set listener\n"); */

	bonobo_object_release_unref (e->listener, NULL);
	e->listener        = bonobo_object_dup_ref (value, NULL);
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

	return gtk_html_command (e->cd->html, command);
}

static CORBA_boolean
impl_is_paragraph_empty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	if (e->cd->html->engine->cursor->object
	    && e->cd->html->engine->cursor->object->parent
	    && HTML_OBJECT_TYPE (e->cd->html->engine->cursor->object->parent) == HTML_TYPE_CLUEFLOW) {
		return html_clueflow_is_empty (HTML_CLUEFLOW (e->cd->html->engine->cursor->object->parent));
	}
	return CORBA_FALSE;
}

static CORBA_boolean
impl_is_previous_paragraph_empty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	if (e->cd->html->engine->cursor->object
	    && e->cd->html->engine->cursor->object->parent
	    && e->cd->html->engine->cursor->object->parent->prev
	    && HTML_IS_CLUEFLOW (e->cd->html->engine->cursor->object->parent->prev))
		return html_clueflow_is_empty (HTML_CLUEFLOW (e->cd->html->engine->cursor->object->parent->prev));

	return CORBA_FALSE;
}

static void
impl_insert_html (PortableServer_Servant servant, const CORBA_char * html, CORBA_Environment * ev)
{
	/* printf ("impl_insert_html\n"); */
	gtk_html_insert_html (html_editor_engine_from_servant (servant)->cd->html, html);
}

static CORBA_boolean
impl_search_by_data (PortableServer_Servant servant, const CORBA_long level, const CORBA_char * klass,
		     const CORBA_char * key, const CORBA_char * value, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);
	HTMLObject *o, *lco = NULL;
	gchar *o_value;

	do {
		if (e->cd->html->engine->cursor->object != lco) {
			o = html_object_nth_parent (e->cd->html->engine->cursor->object, level);
			if (o) {
				o_value = html_object_get_data (o, key);
				if (o_value && !strcmp (o_value, value))
					return TRUE;
			}
		}
		lco = e->cd->html->engine->cursor->object;
	} while (html_cursor_forward (e->cd->html->engine->cursor, e->cd->html->engine));
	return FALSE;
}

static void
impl_freeze (PortableServer_Servant servant, CORBA_Environment * ev)
{
	html_engine_freeze (html_editor_engine_from_servant (servant)->cd->html->engine);
}

static void
impl_thaw (PortableServer_Servant servant, CORBA_Environment * ev)
{
	html_engine_thaw (html_editor_engine_from_servant (servant)->cd->html->engine);
}

static void
impl_undo_begin (PortableServer_Servant servant, const CORBA_char * undo_name, const CORBA_char * redo_name,
		 CORBA_Environment * ev)
{
	html_undo_level_begin (html_editor_engine_from_servant (servant)->cd->html->engine->undo, undo_name, redo_name);
}

static void
impl_undo_end (PortableServer_Servant servant, CORBA_Environment * ev)
{
	html_undo_level_end (html_editor_engine_from_servant (servant)->cd->html->engine->undo);
}

static void
impl_ignore_word (PortableServer_Servant servant, const CORBA_char * word, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	/* printf ("ignoreWord: %s\n", word); */

	spell_add_to_session (e->cd->html, word, e->cd);
}

static CORBA_boolean
impl_has_undo (PortableServer_Servant servant, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	/* printf ("hasUndo\n"); */

	return gtk_html_has_undo (e->cd->html);
}

static void
impl_drop_undo (PortableServer_Servant servant, CORBA_Environment * ev)
{
	EditorEngine *e = html_editor_engine_from_servant (servant);

	/* printf ("dropUndo\n"); */

	gtk_html_drop_undo (e->cd->html);
}

static void
engine_object_finalize (GObject *object)
{
	EditorEngine *e = EDITOR_ENGINE (object);

	bonobo_object_release_unref (e->listener, NULL);

	G_OBJECT_CLASS (engine_parent_class)->finalize (object);
}

static void
editor_engine_init (GObject *object)
{
	EditorEngine *e = EDITOR_ENGINE (object);

	e->listener = CORBA_OBJECT_NIL;
}

static void
editor_engine_class_init (EditorEngineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	POA_GNOME_GtkHTML_Editor_Engine__epv *epv = &klass->epv;

	engine_parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = engine_object_finalize;

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
	epv->undoBegin                = impl_undo_begin;
	epv->undoEnd                  = impl_undo_end;
	epv->ignoreWord               = impl_ignore_word;
	epv->hasUndo                  = impl_has_undo;
	epv->dropUndo                 = impl_drop_undo;
}

BONOBO_TYPE_FUNC_FULL (
	EditorEngine,                  /* Glib class name */
	GNOME_GtkHTML_Editor_Engine,   /* CORBA interface name */
	BONOBO_TYPE_OBJECT,            /* parent type */
	editor_engine);                /* local prefix ie. 'echo'_class_init */

EditorEngine *
editor_engine_new (GtkHTMLControlData *cd)
{
	EditorEngine *ee;

	ee = g_object_new (EDITOR_ENGINE_TYPE, NULL);

	ee->cd = cd;

	return ee;
}
