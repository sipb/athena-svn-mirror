/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of gnome-spell bonobo component

    Copyright (C) 1999, 2000 Helix Code, Inc.
    Authors:                 Radek Doulik <rodo@helixcode.com>

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

#ifndef SPELL_DICTIONARY_H_
#define SPELL_DICTIONARY_H_

G_BEGIN_DECLS

#include <bonobo/bonobo-object.h>
#include <aspell.h>

#define GNOME_SPELL_DICTIONARY_TYPE        (gnome_spell_dictionary_get_type ())
#define GNOME_SPELL_DICTIONARY(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), \
									GNOME_SPELL_DICTIONARY_TYPE, GNOMESpellDictionary))
#define GNOME_SPELL_DICTIONARY_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), \
								    GNOME_SPELL_DICTIONARY_TYPE, GNOMESpellDictionaryClass))
#define IS_GNOME_SPELL_DICTIONARY(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_SPELL_DICTIONARY_TYPE))
#define IS_GNOME_SPELL_DICTIONARY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GNOME_SPELL_DICTIONARY_TYPE))

typedef struct {
	AspellConfig  *config;
	AspellSpeller *speller;
	gboolean       changed;
} SpellEngine;

typedef struct {
	BonoboObject parent;

	gboolean changed;
	GSList *engines;
	GHashTable *languages;
	GHashTable *engines_ht;
} GNOMESpellDictionary;

typedef struct {
	BonoboObjectClass parent_class;
	POA_GNOME_Spell_Dictionary__epv epv;
} GNOMESpellDictionaryClass;

GtkType                          gnome_spell_dictionary_get_type   (void);
GNOMESpellDictionary            *gnome_spell_dictionary_construct  (GNOMESpellDictionary   *dictionary,
								    GNOME_Spell_Dictionary  corba_dictionary);
BonoboObject                    *gnome_spell_dictionary_new        (void);
POA_GNOME_Spell_Dictionary__epv *gnome_spell_dictionary_get_epv    (void);

G_END_DECLS

#endif
