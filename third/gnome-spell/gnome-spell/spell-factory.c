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

#include <config.h>
#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-shlib-factory.h>
#include <glade/glade.h>
#include "Spell.h"
#include "dictionary.h"
#include "control.h"

CORBA_Environment ev;
CORBA_ORB orb;

static Bonobo_Unknown
dictionary_factory (PortableServer_POA poa, const char *iid, gpointer impl_ptr, CORBA_Environment *ev)
{
	return bonobo_shlib_factory_std ("OAFIID:GNOME_Spell_DictionaryFactory:" API_VERSION, poa, impl_ptr, (BonoboFactoryCallback) gnome_spell_dictionary_new, NULL, ev);
}

static Bonobo_Unknown
control_factory (PortableServer_POA poa, const char *iid, gpointer impl_ptr, CORBA_Environment *ev)
{
	return bonobo_shlib_factory_std ("OAFIID:GNOME_Spell_ControlFactory:" API_VERSION, poa, impl_ptr, (BonoboFactoryCallback) gnome_spell_control_new, NULL, ev);
}

static BonoboActivationPluginObject plugin_list[] = {{"OAFIID:GNOME_Spell_DictionaryFactory:" API_VERSION, dictionary_factory},
						     {"OAFIID:GNOME_Spell_ControlFactory:" API_VERSION, control_factory},
						     { NULL } };
const  BonoboActivationPlugin Bonobo_Plugin_info = { plugin_list, "GNOME Spell factory" };
