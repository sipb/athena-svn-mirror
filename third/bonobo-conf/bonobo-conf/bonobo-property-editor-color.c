/*
 * bonobo-property-editor-color.c:
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

enum {
	ARG_TITLE,
};

static void
color_set_cb (GnomeColorPicker *cp, guint r, guint g, guint b, guint a, 
	      gpointer user_data)
{
	BonoboPEditor *editor = BONOBO_PEDITOR (user_data);
	CORBA_Environment ev;
	DynamicAny_DynAny dyn;
	BonoboArg *arg;
	double dr, dg, db, da;
	
	CORBA_exception_init (&ev);
	
	dyn =  CORBA_ORB_create_dyn_struct (bonobo_orb (), 
					    TC_Bonobo_Config_Color, &ev);

	gnome_color_picker_get_d (cp, &dr, &dg, &db, &da);

	DynamicAny_DynAny_insert_double (dyn, dr, &ev);
	DynamicAny_DynAny_next (dyn, &ev);
	DynamicAny_DynAny_insert_double (dyn, dg, &ev);
	DynamicAny_DynAny_next (dyn, &ev);
	DynamicAny_DynAny_insert_double (dyn, db, &ev);
	DynamicAny_DynAny_next (dyn, &ev);
	DynamicAny_DynAny_insert_double (dyn, da, &ev);

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
	GtkWidget *widget;
	Bonobo_Config_Color *color;

	if (!bonobo_arg_type_is_equal (value->_type, TC_Bonobo_Config_Color, 
				       NULL))
		return;

	widget = bonobo_peditor_get_widget (editor);

	gtk_signal_handler_block_by_func (GTK_OBJECT (widget), color_set_cb, 
					  editor);

	color = (Bonobo_Config_Color *)value->_value;

	gnome_color_picker_set_d (GNOME_COLOR_PICKER (widget), color->r, 
				  color->g,  color->b,  color->a);

	gtk_signal_handler_unblock_by_func (GTK_OBJECT (widget), color_set_cb, 
					    editor);
}

GtkObject *
bonobo_peditor_color_new ()
{
	GtkWidget *cp;

	cp = gnome_color_picker_new ();
	return bonobo_peditor_color_construct (cp);
}

GtkObject *
bonobo_peditor_color_construct (GtkWidget *widget)
{
	BonoboPEditor *ed;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_COLOR_PICKER (widget), NULL);

	ed = bonobo_peditor_construct (widget, set_value_cb, 
				       TC_Bonobo_Config_Color);

	gtk_signal_connect (GTK_OBJECT (widget), "color-set", 
			    (GtkSignalFunc) color_set_cb, ed);
	
	return GTK_OBJECT (ed);
}
