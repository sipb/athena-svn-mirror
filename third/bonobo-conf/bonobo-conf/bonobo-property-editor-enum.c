/*
 * bonobo-property-editor-enum.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>
#include <bonobo.h>

#include "bonobo-property-editor.h"

static void
destroy_cb (BonoboPEditor *editor,
	    gpointer       user_data) 
{
	GList *l;

	l = (GList *)editor->data;

	while (l) {
		g_free (l->data);
		l = l->next;
	}

	if (editor->data)
		g_list_free ((GList *)editor->data);
}

static void
changed_cb (GtkEditable *editable,
	    gpointer     user_data) 
{
	BonoboPEditor *editor = BONOBO_PEDITOR (user_data);
	CORBA_Environment ev;
	DynamicAny_DynAny dyn;
	BonoboArg *arg;
	char *new_str;

	CORBA_exception_init (&ev);

	new_str = gtk_entry_get_text (GTK_ENTRY (editable));

	dyn = CORBA_ORB_create_dyn_enum (bonobo_orb (), editor->tc, &ev);

	DynamicAny_DynEnum_set_as_string (dyn, new_str, &ev);

	arg = DynamicAny_DynAny_to_any (dyn, &ev);

	CORBA_Object_release ((CORBA_Object) dyn, &ev);

	bonobo_peditor_set_value (editor, arg, &ev);

	bonobo_arg_release (arg);

	CORBA_exception_free (&ev);
}

static void
set_value_cb (BonoboPEditor     *editor,
	      BonoboArg         *value,
	      CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn;
	GtkCombo *combo;
	GtkEntry *entry;
	char *str, *old_str;
	int i;

	if (value->_type->kind != CORBA_tk_enum)
		return;

	combo = GTK_COMBO (bonobo_peditor_get_widget (editor));
	entry = GTK_ENTRY (combo->entry);

	dyn = CORBA_ORB_create_dyn_any (bonobo_orb (), value, ev);

	if (!editor->data) {
		GList *l = NULL;

		for (i = 0; i < value->_type->sub_parts; i++) {
			l = g_list_append (l, 
			        g_strdup (value->_type->subnames [i]));
		}

		editor->data = l;
		gtk_combo_set_popdown_strings (combo, l);
	}

	old_str = gtk_entry_get_text (entry);

	str = DynamicAny_DynEnum_get_as_string (dyn, ev);
	
	gtk_entry_set_editable (entry, TRUE);

	if (str && strcmp (old_str, str))
		gtk_entry_set_text (entry, str);

	CORBA_free (str);
		
	CORBA_Object_release ((CORBA_Object) dyn, ev);
}

GtkObject *
bonobo_peditor_enum_new ()
{
	GtkWidget *combo;

	combo = gtk_combo_new ();
	gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (combo)->entry), FALSE);

	return bonobo_peditor_enum_construct (combo);
}

GtkObject *
bonobo_peditor_enum_construct (GtkWidget *widget)
{
	BonoboPEditor *ed;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GTK_IS_COMBO (widget), NULL);

	ed = bonobo_peditor_construct (widget, set_value_cb, NULL);

	gtk_signal_connect (GTK_OBJECT (GTK_COMBO (widget)->entry), "changed",
			    (GtkSignalFunc) changed_cb, ed);

	gtk_signal_connect (GTK_OBJECT (ed), "destroy",
			    (GtkSignalFunc) destroy_cb, NULL);

	return GTK_OBJECT (ed);
}
