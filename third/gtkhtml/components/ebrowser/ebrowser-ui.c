#define _EBROWSER_C_

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

    Authors: Lauris Kaplinski  <lauris@helixcode.com>
             Miguel de Icaza   <miguel@helixcode.com>
	     
*/
#include <config.h>
#include <gnome.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-control.h>
#include "ebrowser.h"
#include "ebrowser-ui.h"
#include "ebrowser-widget.h"

static void
verb_stop_loading (BonoboUIComponent *component, gpointer user_data, const char *cname)
{
	g_print ("Stop loading\n");
	ebrowser_stop (EBROWSER (user_data));
}

static void
verb_history_back (BonoboUIComponent *component, gpointer user_data, const char *cname)
{
	EBrowser *browser = EBROWSER (user_data);

	if (!browser->history)
		return;
}

static void
verb_history_forward (BonoboUIComponent *component, gpointer user_data, const char *cname)
{
	EBrowser *browser = EBROWSER (user_data);

	if (!browser->history)
		return;
}

static BonoboUIVerb verbs [] = {
	BONOBO_UI_VERB ("Stop", verb_stop_loading),
	BONOBO_UI_VERB ("HistoryBack", verb_history_back),
	BONOBO_UI_VERB ("HistoryForward", verb_history_forward),
	
	BONOBO_UI_VERB_END
};

static gchar * ui = 
"<Root>"
"  <commands>"
"    <cmd name=\"Stop\" _label=\"Stop\" _tip=\"Stop loading\" pixtype=\"stock\" pixname=\"Stop\"/>"
"  </commands>"
"  <menu>"
"    <submenu name=\"File\" _label=\"File\">"
"      <menuitem name=\"Stop\" verb=\"\"/>"
"    </submenu>"
"  </menu>"
"  <status>"
"    <item name=\"main\"/>"
"  </status>"
"</Root>";


void
ebrowser_control_activate_cb (BonoboControl * control, gboolean activate, gpointer data)
{
	BonoboUIComponent * component;

	component = bonobo_control_get_ui_component (control);
	g_assert (component != NULL);

	if (activate) {
#if 1
		bonobo_ui_component_add_verb_list_with_data (component, verbs, data);
#endif
#if 1
		bonobo_ui_component_set_translate (component, "/", ui, NULL);
#endif
	}

	g_print ("Activate: %d\n", activate);
}

