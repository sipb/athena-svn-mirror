/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of gnome-spell bonobo component
    copied from echo.c written by Miguel de Icaza and updated for Spell.idl needs

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

#include <config.h>
#include <gnome.h>
#include <bonobo.h>
#include "Spell.h"

int
main (int argc, char *argv [])
{
	GNOME_Spell_Dictionary en;
	GNOME_Spell_StringSeq *seq;
	CORBA_Environment   ev;
	CORBA_sequence_GNOME_Spell_Language *language_seq;
	gint i;

	if (!bonobo_ui_init ("test-spell", VERSION, &argc, argv))
		g_error (_("I could not initialize Bonobo"));
	bonobo_activate ();

	en = bonobo_get_object ("OAFIID:GNOME_Spell_Dictionary:" API_VERSION, "GNOME/Spell/Dictionary", 0);

	if (en == CORBA_OBJECT_NIL) {
		g_error ("Could not create an instance of the spell component");
		return 1;
	}

	CORBA_exception_init (&ev);

	/*
	 * test dictionary
	 */

	GNOME_Spell_Dictionary_getLanguages (en, &ev);
	GNOME_Spell_Dictionary_setLanguage (en, "en", &ev);

	printf ("check: %s --> %d\n",
		"helo",
		GNOME_Spell_Dictionary_checkWord (en, "helo", &ev));
	printf ("check: %s --> %d\n",
		"hello",
		GNOME_Spell_Dictionary_checkWord (en, "hello", &ev));

	seq = GNOME_Spell_Dictionary_getSuggestions (en, "helo", &ev);
	printf ("suggestions for helo\n");
	for (i=0; i<seq->_length; i++)
		printf ("\t%s\n", seq->_buffer [i]);
	CORBA_free (seq);
	printf ("add helo to session\n");
	GNOME_Spell_Dictionary_addWordToSession (en, "helo", &ev);
	printf ("check: %s --> %d\n",
		"helo",
		GNOME_Spell_Dictionary_checkWord (en, "helo", &ev));
	language_seq = GNOME_Spell_Dictionary_getLanguages (en, &ev);
	if (language_seq) {
		printf ("Languages:\n");
		for (i = 0; i < language_seq->_length; i++)
			printf ("\t%s\n", language_seq->_buffer[i].name);
	}

	CORBA_exception_free (&ev);

	bonobo_object_release_unref (en, NULL);

	return 0;
}
