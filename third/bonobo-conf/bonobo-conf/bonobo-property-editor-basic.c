/*
 * bonobo-property-editor-basic.c:
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

static gboolean
check_type (CORBA_TypeCode tc1, CORBA_TypeCode tc2)
{

	return bonobo_arg_type_is_equal (tc1, tc2, NULL);
}

static void        
changed_cb (GtkEntry *entry,
	    gpointer user_data)
{
	BonoboPEditor *editor = BONOBO_PEDITOR (user_data);
	CORBA_Environment ev;
	DynamicAny_DynAny dyn = NULL;
	CORBA_TypeCode tc;
	BonoboArg *arg;
	CORBA_ORB orb;
	char *text;

	if (!editor->tc)
		return;

	CORBA_exception_init (&ev);

	orb = bonobo_orb ();

	text = gtk_entry_get_text (entry);

	if (editor->tc->kind == CORBA_tk_alias)
		tc = editor->tc->subtypes [0];
	else
		tc = editor->tc;

	if (check_type (tc, TC_ushort)) {
		CORBA_unsigned_short v;

		dyn = CORBA_ORB_create_basic_dyn_any (orb, TC_ushort, &ev);
		v = strtoul (text, NULL, 0);
		DynamicAny_DynAny_insert_ushort (dyn, v, &ev);

	} else if (check_type (tc, TC_short)) {
		CORBA_short v;
		
		dyn = CORBA_ORB_create_basic_dyn_any (orb, TC_short, &ev);
		v = strtol (text, NULL, 0);
		DynamicAny_DynAny_insert_short (dyn, v, &ev);
	
	} else if (check_type (tc, TC_ulong)) {
		CORBA_unsigned_long v;

		dyn = CORBA_ORB_create_basic_dyn_any (orb, TC_ulong, &ev);
		v = strtoul (text, NULL, 0);
		DynamicAny_DynAny_insert_ulong (dyn, v, &ev);

	} else if (check_type (tc, TC_long)) {
		CORBA_long v;

		dyn = CORBA_ORB_create_basic_dyn_any (orb, TC_long, &ev);
		v = strtol (text, NULL, 0);
		DynamicAny_DynAny_insert_long (dyn, v, &ev);
	
	} else if (check_type (tc, TC_float)) {
		CORBA_float v;
		
		dyn = CORBA_ORB_create_basic_dyn_any (orb, TC_float, &ev);
		v = strtod (text, NULL);
		DynamicAny_DynAny_insert_float (dyn, v, &ev);

	} else if (check_type (tc, TC_double)) {
		CORBA_float v;
		
		dyn = CORBA_ORB_create_basic_dyn_any (orb, TC_double, &ev);
		v = strtod (text, NULL);
		DynamicAny_DynAny_insert_double (dyn, v, &ev);

	} else if (check_type (tc, TC_string)) {

		dyn = CORBA_ORB_create_basic_dyn_any (orb, TC_string, &ev);
		DynamicAny_DynAny_insert_string (dyn, text, &ev);
	}

	if (BONOBO_EX (&ev) || dyn == NULL)
		return;

	arg = DynamicAny_DynAny_to_any (dyn, &ev);

	bonobo_peditor_set_value (editor, arg, &ev);

	bonobo_arg_release (arg);

	CORBA_Object_release ((CORBA_Object) dyn, &ev);

	CORBA_exception_free (&ev);
}

static void
set_value_cb (BonoboPEditor     *editor,
	      BonoboArg         *value,
	      CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn;
	GtkEntry *entry;
	char *text;

	entry = GTK_ENTRY (bonobo_peditor_get_widget (editor));

	dyn = CORBA_ORB_create_dyn_any (bonobo_orb (), value, ev);

	if (BONOBO_EX (ev) || dyn == NULL)
		return;

	if (check_type (value->_type, TC_ushort)) {
		CORBA_unsigned_short v;
		
		v = DynamicAny_DynAny_get_ushort (dyn, ev); 
		text =  g_strdup_printf ("%u", v);
	
	} else if (check_type (value->_type, TC_short)) {
		CORBA_short v;
		
		v = DynamicAny_DynAny_get_short (dyn, ev); 
		text =  g_strdup_printf ("%d", v);

	} else if (check_type (value->_type, TC_ulong)) {
		CORBA_unsigned_long v;
		
		v = DynamicAny_DynAny_get_ulong (dyn, ev); 
		text =  g_strdup_printf ("%u", v);
	
	} else if (check_type (value->_type, TC_long)) {
		CORBA_long v;
		
		v = DynamicAny_DynAny_get_long (dyn, ev); 
		text =  g_strdup_printf ("%d", v);

	} else if (check_type (value->_type, TC_float)) {
		CORBA_float v;
		
		v = DynamicAny_DynAny_get_float (dyn, ev); 
		text =  g_strdup_printf ("%f", v);

	} else if (check_type (value->_type, TC_double)) {
		CORBA_double v;
		
		v = DynamicAny_DynAny_get_double (dyn, ev); 
		text =  g_strdup_printf ("%g", v);

	} else if (check_type (value->_type, TC_string)) {
		CORBA_char *v;

		v = DynamicAny_DynAny_get_string (dyn, ev); 
		text =  g_strdup (v);
		CORBA_free (v);

	} else {
		text = g_strdup ("(unknown type code)");
	}


	CORBA_Object_release ((CORBA_Object) dyn, ev);

	gtk_signal_handler_block_by_func (GTK_OBJECT (entry), changed_cb, 
					  editor);

	if (strcmp (gtk_entry_get_text (entry), text)) {
		gtk_entry_set_editable (entry, TRUE);
		gtk_entry_set_text (entry, text);
	}

	gtk_signal_handler_unblock_by_func (GTK_OBJECT (entry), changed_cb, 
					    editor);

	g_free (text);
}

static void
spin_set_value_cb (BonoboPEditor     *editor,
		   BonoboArg         *value,
		   CORBA_Environment *ev)
{
	GtkEntry *entry;
	guint32 v;

	entry = GTK_ENTRY (bonobo_peditor_get_widget (editor));

	if (!check_type (value->_type, TC_long))
		return;
	
	v = BONOBO_ARG_GET_LONG (value);

	gtk_signal_handler_block_by_func (GTK_OBJECT (entry), changed_cb, 
					  editor);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (entry), (float) v);

	gtk_signal_handler_unblock_by_func (GTK_OBJECT (entry), changed_cb, 
					    editor);
}

static GtkObject *
bonobo_peditor_basic_construct (GtkWidget *widget, CORBA_TypeCode tc)
{
	BonoboPEditor *editor;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GTK_IS_ENTRY (widget), NULL);

	editor = bonobo_peditor_construct (widget, set_value_cb, tc);

	gtk_signal_connect (GTK_OBJECT (widget), "changed",
			    (GtkSignalFunc) changed_cb, editor);

	return GTK_OBJECT (editor);
}

static GtkObject *
bonobo_peditor_basic_new (CORBA_TypeCode tc)
{
	GtkWidget *entry;
	
	entry = gtk_entry_new ();
	gtk_widget_show (entry);

	return bonobo_peditor_basic_construct (entry, tc);
}

GtkObject *
bonobo_peditor_int_range_construct (GtkWidget *widget)
{
	BonoboPEditor *editor;
	
	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GTK_IS_SPIN_BUTTON (widget), NULL);

	editor = bonobo_peditor_construct (widget, spin_set_value_cb, TC_long);

	gtk_signal_connect (GTK_OBJECT (widget), "changed",
			    (GtkSignalFunc) changed_cb, editor);

	return GTK_OBJECT (editor);
}

GtkObject *
bonobo_peditor_int_range_new (gint32 lower, gint32 upper, gint32 incr)
{
	GtkObject            *adj;
	GtkWidget            *entry;
	
	adj = gtk_adjustment_new ((float) lower, (float) lower, (float) upper,
				  (float) incr, (float) incr, 1.0); 

	entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1.0, 0);
 
	gtk_widget_show (entry);

	return bonobo_peditor_int_range_construct (entry);
}

GtkObject *
bonobo_peditor_short_new ()
{
	return bonobo_peditor_basic_new (TC_short);
}

GtkObject *
bonobo_peditor_ushort_new ()
{
	return bonobo_peditor_basic_new (TC_ushort);
}

GtkObject *
bonobo_peditor_long_new ()
{
	return bonobo_peditor_basic_new (TC_long);
}

GtkObject *
bonobo_peditor_ulong_new ()
{
	return bonobo_peditor_basic_new (TC_ulong);
}

GtkObject *
bonobo_peditor_float_new ()
{
	return bonobo_peditor_basic_new (TC_float);
}

GtkObject *
bonobo_peditor_double_new ()
{
	return bonobo_peditor_basic_new (TC_double);
}

GtkObject *
bonobo_peditor_string_new ()
{
	return bonobo_peditor_basic_new (TC_string);
}

GtkObject *
bonobo_peditor_short_construct (GtkWidget *widget)
{
	return bonobo_peditor_basic_construct (widget, TC_short);
}

GtkObject *
bonobo_peditor_ushort_construct (GtkWidget *widget)
{
	return bonobo_peditor_basic_construct (widget, TC_ushort);
}

GtkObject *
bonobo_peditor_long_construct (GtkWidget *widget)
{
	return bonobo_peditor_basic_construct (widget, TC_long);
}

GtkObject *
bonobo_peditor_ulong_construct (GtkWidget *widget)
{
	return bonobo_peditor_basic_construct (widget, TC_ulong);
}

GtkObject *
bonobo_peditor_float_construct (GtkWidget *widget)
{
	return bonobo_peditor_basic_construct (widget, TC_float);
}

GtkObject *
bonobo_peditor_double_construct (GtkWidget *widget)
{
	return bonobo_peditor_basic_construct (widget, TC_double);
}

GtkObject *
bonobo_peditor_string_construct (GtkWidget *widget)
{
	return bonobo_peditor_basic_construct (widget, TC_string);
}
