/*
 * bonobo-property-editor-default.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <bonobo.h>

#include "bonobo-property-editor.h"
#include "bonobo-config-utils.h"

static void
set_value_cb (BonoboPEditor     *editor,
	      BonoboArg         *value,
	      CORBA_Environment *ev)
{
	GtkWidget *e;
	char *s;

	e = bonobo_peditor_get_widget (editor);
	s = bonobo_config_any_to_string (value);
	
	gtk_entry_set_text (GTK_ENTRY (e), s);

	g_free (s);
}

GtkObject *
bonobo_peditor_default_new ()
{
	GtkWidget *e;

	e = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (e), "no value set");
	gtk_entry_set_editable (GTK_ENTRY (e), FALSE);

	return bonobo_peditor_default_construct (e);
}

GtkObject *
bonobo_peditor_default_construct (GtkWidget *widget) 
{
	BonoboPEditor *ed;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GTK_IS_ENTRY (widget), NULL);

	ed = bonobo_peditor_construct (widget, set_value_cb, NULL);

	return GTK_OBJECT (ed);
}
