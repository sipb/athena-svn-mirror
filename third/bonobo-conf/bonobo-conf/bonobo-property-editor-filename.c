/*
 * bonobo-property-editor-filename.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>
#include <bonobo.h>

#include "bonobo-property-editor.h"

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

	dyn =  CORBA_ORB_create_basic_dyn_any (bonobo_orb (), 
					       TC_Bonobo_Config_FileName, &ev);

	DynamicAny_DynAny_insert_string (dyn, new_str, &ev);

	arg = DynamicAny_DynAny_to_any (dyn, &ev);

	bonobo_peditor_set_value (editor, arg, &ev);

	bonobo_arg_release (arg);

	CORBA_Object_release ((CORBA_Object)dyn, &ev);

	CORBA_exception_free (&ev);
}

static void
set_value_cb (BonoboPEditor     *editor,
	      BonoboArg         *value,
	      CORBA_Environment *ev)
{
	GtkWidget *widget, *entry;
	char *new_value, *old_value;

	if (!bonobo_arg_type_is_equal (value->_type, TC_string, NULL))
		return;

	widget = bonobo_peditor_get_widget (editor);

	entry = gnome_entry_gtk_entry 
		(GNOME_ENTRY (GNOME_FILE_ENTRY (widget)->gentry));

	old_value = gtk_entry_get_text (GTK_ENTRY (entry) );

	new_value = BONOBO_ARG_GET_STRING (value);

	gtk_signal_handler_block_by_func (GTK_OBJECT (entry), changed_cb, 
					  editor);

	if (strcmp (new_value, old_value))
		gtk_entry_set_text (GTK_ENTRY (entry), new_value);

	gtk_signal_handler_unblock_by_func (GTK_OBJECT (entry), changed_cb, 
					    editor);
}

GtkObject *
bonobo_peditor_filename_new ()
{
	GtkWidget *fe;

	fe = gnome_file_entry_new (NULL, NULL);
	return bonobo_peditor_filename_construct (fe);
}

GtkObject *
bonobo_peditor_filename_construct (GtkWidget *widget)
{
	GtkWidget *entry;
	BonoboPEditor *ed;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FILE_ENTRY (widget), NULL);

	ed = bonobo_peditor_construct (widget, set_value_cb, 
				       TC_Bonobo_Config_FileName);

	entry = gnome_entry_gtk_entry 
		(GNOME_ENTRY (GNOME_FILE_ENTRY (widget)->gentry));

	gtk_signal_connect (GTK_OBJECT (entry), "changed", 
			    (GtkSignalFunc) changed_cb, ed);
	
	return GTK_OBJECT (ed);
}
