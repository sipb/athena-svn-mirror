/*
 * bonobo-property_frame.c:
 *
 * Authors:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include <gtk/gtksignal.h>

#include <bonobo/bonobo-widget.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-control.h>
#include <bonobo/bonobo-moniker-util.h>

#include "bonobo-property-frame.h"

static GtkFrameClass *bonobo_property_frame_parent_class;

#define PARENT_TYPE GTK_TYPE_FRAME

static void
bonobo_property_frame_destroy (GtkObject *object)
{
	BonoboPropertyFrame *pf;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_FRAME (object));

	pf = BONOBO_PROPERTY_FRAME (object);

	if (pf->moniker)
		g_free (pf->moniker);

	pf->moniker = NULL;

	if (pf->proxy != NULL)
		bonobo_object_unref (BONOBO_OBJECT (pf->proxy));

	pf->proxy = NULL;

	GTK_OBJECT_CLASS(bonobo_property_frame_parent_class)->destroy (object);
}

static void
bonobo_property_frame_class_init (BonoboPropertyFrame *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *)klass;

	bonobo_property_frame_parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_property_frame_destroy;
}

static void
bonobo_property_frame_init (BonoboPropertyFrame *property_frame)
{
	/* nothing to do */
}

GtkType
bonobo_property_frame_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboPropertyFrame",
			sizeof (BonoboPropertyFrame),
			sizeof (BonoboPropertyFrameClass),
			(GtkClassInitFunc)  bonobo_property_frame_class_init,
			(GtkObjectInitFunc) bonobo_property_frame_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};
		
		type = gtk_type_unique (PARENT_TYPE, &info);
	}

	return type;
}

static void
apply_cb (GnomePropertyBox *propertybox,
	  gint              arg1,
	  gpointer          user_data)
{
	BonoboPropertyFrame *pf = BONOBO_PROPERTY_FRAME (user_data);

	bonobo_pbproxy_update (pf->proxy);
}

static void
parent_set_cb (GtkWidget *widget,
	       GtkObject *old_parent,
	       gpointer   user_data)
{
	BonoboPropertyFrame *pf = BONOBO_PROPERTY_FRAME (user_data);
	GtkWidget *p;

	p = widget->parent;
	while (p) {
		if (GNOME_IS_PROPERTY_BOX (p)) {
			gtk_signal_connect (GTK_OBJECT (p), "apply",  
					    GTK_SIGNAL_FUNC (apply_cb), 
					    pf);
			break;
		}
		p = p->parent;
	}
}

static void
modified_cb (GtkWidget *widget,
	     gpointer   user_data)
{
	BonoboPropertyFrame *pf = BONOBO_PROPERTY_FRAME (user_data);
	GtkWidget *p;

	p = GTK_WIDGET (pf)->parent;
	while (p) {
		if (GNOME_IS_PROPERTY_BOX (p)) {
			gnome_property_box_changed 
				(GNOME_PROPERTY_BOX (p));
			break;
		}
		p = p->parent;
	}
}

GtkWidget *
bonobo_property_frame_new (char *label, char *moniker)
{
	BonoboPropertyFrame *pf;
	
	if (!(pf = gtk_type_new (bonobo_property_frame_get_type ())))
		return NULL;

	if (label)
		gtk_frame_set_label (GTK_FRAME (pf), label);
	else
		gtk_frame_set_shadow_type (GTK_FRAME (pf), GTK_SHADOW_NONE);

	pf->proxy = bonobo_pbproxy_new ();

	gtk_signal_connect (GTK_OBJECT (pf), "parent-set",  
			    GTK_SIGNAL_FUNC (parent_set_cb), pf);
	
	gtk_signal_connect (GTK_OBJECT (pf->proxy), "modified",  
			    GTK_SIGNAL_FUNC (modified_cb), pf);
	
	if (moniker)
		bonobo_property_frame_set_moniker (pf, moniker);

	return GTK_WIDGET (pf);
}

void
bonobo_property_frame_set_moniker (BonoboPropertyFrame *pf,
				   char                *moniker)
{
	Bonobo_PropertyBag pb;
	CORBA_Environment ev;

	g_return_if_fail (pf != NULL);

	if (pf->moniker)
		g_free (pf->moniker);

	pf->moniker = moniker ? g_strdup (moniker) : NULL;

	if (!moniker) {
		bonobo_pbproxy_set_bag (pf->proxy, NULL);
		return;
	}

	CORBA_exception_init (&ev);

	pb = bonobo_get_object (moniker, "IDL:Bonobo/PropertyBag:1.0", &ev);

	if (BONOBO_EX (&ev) || pb == CORBA_OBJECT_NIL) {
		bonobo_pbproxy_set_bag (pf->proxy, NULL);
		CORBA_exception_free (&ev);
		return;
	}

	bonobo_pbproxy_set_bag (pf->proxy, pb);

	bonobo_object_release_unref (pb, NULL);

	CORBA_exception_free (&ev);
}
