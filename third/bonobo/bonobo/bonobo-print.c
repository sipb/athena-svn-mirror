/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-print.c: Remote printing support, client side.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-print.h>

#include <libgnomeprint/gnome-print.h>

#undef PRINT_DEBUG

static BonoboObjectClass *bonobo_print_parent_class;
static BonoboPrintClass  *bonobo_print_class;

POA_Bonobo_Print__vepv bonobo_print_vepv;

#define CLASS(o) BONOBO_PRINT_CLASS(GTK_OBJECT(o)->klass)

static inline BonoboPrint *
bonobo_print_from_servant (PortableServer_Servant _servant)
{
	if (!BONOBO_IS_PRINT (bonobo_object_from_servant (_servant)))
		return NULL;
	else
		return BONOBO_PRINT (bonobo_object_from_servant (_servant));
}

static Bonobo_PrintData *
impl_render (PortableServer_Servant        _servant,
	     const Bonobo_PrintDimensions *pd,
	     const Bonobo_PrintScissor    *scissor,
	     CORBA_Environment            *ev)
{
	GnomePrintMeta    *meta_context;
	BonoboPrint       *print;
	Bonobo_PrintData  *data;
	void              *buffer;
	int                buf_len;
	GnomePrintContext *ctx;

#ifdef PRINT_DEBUG
	g_warning ("Rendering");
#endif
	print = bonobo_print_from_servant (_servant);
	g_return_val_if_fail (print != NULL, CORBA_OBJECT_NIL);

	g_return_val_if_fail (pd != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

	meta_context = gnome_print_meta_new ();
		
#ifdef PRINT_DEBUG
	g_warning ("Render %g %g", pd->width, pd->height);
#endif

	ctx = GNOME_PRINT_CONTEXT (meta_context);

	gnome_print_gsave (ctx);

	if (print->render)
		print->render (ctx, pd->width, pd->height,
			       scissor, print->user_data);
	else
		bonobo_print_class->render (ctx, pd->width, pd->height,
					    scissor, print->user_data);

	gnome_print_grestore (ctx);
	gnome_print_context_close (ctx);

	data = Bonobo_PrintData__alloc ();

	gnome_print_meta_access_buffer (meta_context,
					&buffer, &buf_len);

	/* FIXME: we could kill this copy by nailing gnome-print-meta.c */
	data->_buffer = g_malloc (buf_len);
	memcpy (data->_buffer, buffer, buf_len);
	data->_length = buf_len;
	data->_maximum = buf_len;

	gtk_object_unref (GTK_OBJECT (meta_context));

	return data;
}

/**
 * bonobo_print_get_epv:
 *
 * Returns: The EPV for the default BonoboPrint implementation.  
 */
POA_Bonobo_Print__epv *
bonobo_print_get_epv (void)
{
	POA_Bonobo_Print__epv *epv;

	epv = g_new0 (POA_Bonobo_Print__epv, 1);

	epv->render = impl_render;

	return epv;
}

static void
init_print_corba_class (void)
{
	/* The VEPV */
	bonobo_print_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_print_vepv.Bonobo_Print_epv   = bonobo_print_get_epv ();
}

static void
bonobo_print_class_init (BonoboPrintClass *klass)
{
	bonobo_print_parent_class = gtk_type_class (bonobo_object_get_type ());
	bonobo_print_class = klass;

	klass->render = NULL;

	init_print_corba_class ();
}

/**
 * bonobo_print_get_type:
 *
 * Returns: the GtkType for a BonoboPrint object.
 */
GtkType
bonobo_print_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboPrint",
			sizeof (BonoboPrint),
			sizeof (BonoboPrintClass),
			(GtkClassInitFunc) bonobo_print_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

Bonobo_Print
bonobo_print_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_Print *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Print *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_print_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Print__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);
	return (Bonobo_Print) bonobo_object_activate_servant (object, servant);
}

BonoboPrint *
bonobo_print_construct (BonoboPrint         *p,
			Bonobo_Print         corba_p,
			BonoboPrintRenderFn *render,
			gpointer             user_data)
{
	static gboolean warned = FALSE;

	if (!warned) {
/* Still true, but a pain */
/*		g_warning ("The print interface is horribly immature, use at your own risk");*/
		warned = TRUE;
	}

	p->render       = render;
	p->user_data    = user_data;
	
	return BONOBO_PRINT (
		bonobo_object_construct (BONOBO_OBJECT (p), corba_p));
}

/**
 * bonobo_print_new:
 * @render: 
 * @user_data: 
 * 
 * Create a new bonobo-print implementing BonoboObject
 * interface.
 * 
 * Return value: 
 **/
BonoboPrint *
bonobo_print_new (BonoboPrintRenderFn *render,
		  gpointer             user_data)
{
	BonoboPrint  *p;
	Bonobo_Print  corba_p;

	p = gtk_type_new (bonobo_print_get_type ());
	g_return_val_if_fail (p != NULL, NULL);

	corba_p = bonobo_print_corba_object_create (BONOBO_OBJECT (p));
	if (corba_p == CORBA_OBJECT_NIL){
		bonobo_object_unref (BONOBO_OBJECT (p));
		return NULL;
	}

	return bonobo_print_construct (p, corba_p, render, user_data);
}
