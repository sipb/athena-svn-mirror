/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  bonobo-print-client.c: a print client interface for compound documents.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Michael Meeks <mmeeks@gnu.org>
 *
 *  Copyright (C) 1999-2002 Ximian, Inc. and authors
 *
 */

#include <config.h>

#include <bonobo/bonobo-types.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-stream-memory.h>

#include <libgnomeprint/libgnomeprint-marshal.h>
#include <libgnomeprint/gnome-print-bonobo.h>

#undef PRINT_DEBUG

#define CLASS(o) GNOME_PRINT_BONOBO_CLASS(G_OBJECT_GET_CLASS (o))

/* Parent object class */
static GObjectClass *gnome_print_bonobo_parent_class;

static Bonobo_Stream
impl_render (PortableServer_Servant        servant,
	     const Bonobo_PrintDimensions *pd,
	     const Bonobo_PrintScissor    *scissor,
	     CORBA_Environment            *ev)
{
	GnomePrintBonobo  *print;
	BonoboStream      *stream;
	const guchar      *buffer;
	int                buf_len;
	GnomePrintContext *ctx;

#ifdef PRINT_DEBUG
	g_warning ("Rendering");
#endif
	print = GNOME_PRINT_BONOBO (bonobo_object (servant));
	g_return_val_if_fail (print != NULL, CORBA_OBJECT_NIL);

	g_return_val_if_fail (pd != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

	ctx = gnome_print_meta_new ();
		
#ifdef PRINT_DEBUG
	g_warning ("Render %g %g", pd->width, pd->height);
#endif

	gnome_print_beginpage (ctx, NULL);
	gnome_print_gsave (ctx);

	if (print->render) {
		bonobo_closure_invoke (
			print->render, G_TYPE_NONE, NULL,
			GNOME_PRINT_BONOBO_TYPE, print,
			GNOME_TYPE_PRINT_META, ctx,
			G_TYPE_DOUBLE, pd->width,
			G_TYPE_DOUBLE, pd->height,
			G_TYPE_POINTER, scissor, 0);

	} else if (CLASS (print)->render)
		CLASS (print)->render (
			print, ctx, pd->width, pd->height, scissor);

	else
		g_warning ("No render method on print object");

	gnome_print_grestore (ctx);
	gnome_print_showpage (ctx);

	gnome_print_context_close (ctx);

	buffer  = gnome_print_meta_get_buffer (GNOME_PRINT_META (ctx));
	buf_len = gnome_print_meta_get_length (GNOME_PRINT_META (ctx));

	/*
	 * FIXME: this does an expensive mem-copy that we could
	 * avoid easily with a custom stream.
	 */
	stream = bonobo_stream_mem_create (
		buffer, buf_len, TRUE, FALSE);

	g_object_unref (G_OBJECT (ctx));

	return CORBA_Object_duplicate (BONOBO_OBJREF (stream), ev);
}

static void
gnome_print_bonobo_finalize (GObject *object)
{
	GnomePrintBonobo *pb = (GnomePrintBonobo *) object;
       
	if (pb->render)
		g_closure_unref (pb->render);
	pb->render = NULL;

	gnome_print_bonobo_parent_class->finalize (object);
}

/**
 * gnome_print_bonobo_construct:
 * @p: the print object
 * @render: the render method
 * 
 * Construct @p setting its @render and @user_data pointers
 * 
 * Return value: a constructed GnomePrintBonobo object
 **/
GnomePrintBonobo *
gnome_print_bonobo_construct (GnomePrintBonobo *p,
			      GClosure         *render)
{
	p->render = bonobo_closure_store (render,
		libgnomeprint_marshal_VOID__OBJECT_DOUBLE_DOUBLE_POINTER);

	return p;
}

/**
 * gnome_print_bonobo_new_closure:
 * @render: a render closure
 * 
 * Create a new bonobo-print implementing BonoboObject
 * interface.
 *
 * This interface is called to ask a component to
 * @render itself to a print context with the specified
 * width and height, and scissoring data.
 * 
 * Return value: a new GnomePrintBonobo interface
 **/
GnomePrintBonobo *
gnome_print_bonobo_new_closure (GClosure *render)
{
	GnomePrintBonobo *p;

	p = g_object_new (gnome_print_bonobo_get_type (), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	return gnome_print_bonobo_construct (p, render);
}

/**
 * gnome_print_bonobo_new:
 * @render: the render function
 * @user_data: the render's user data function
 * 
 * Create a new bonobo-print implementing BonoboObject
 * interface.
 *
 * This interface is called to ask a component to
 * @render itself to a print context with the specified
 * width and height, and scissoring data.
 * 
 * Return value: a new GnomePrintBonobo interface
 **/
GnomePrintBonobo *
gnome_print_bonobo_new (GnomePrintBonoboRenderFn *render,
			gpointer             user_data)
{
	return gnome_print_bonobo_new_closure (
		g_cclosure_new (G_CALLBACK (render), user_data, NULL));
}

static void
gnome_print_bonobo_class_init (GnomePrintBonoboClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass *) klass;

	POA_Bonobo_Print__epv *epv = &klass->epv;

	klass->render = NULL;

	epv->render = impl_render;

	gobject_class->finalize = gnome_print_bonobo_finalize;

	gnome_print_bonobo_parent_class = 
		g_type_class_peek_parent (klass);
}

static void
gnome_print_bonobo_init (GObject *object)
{
	/* nothing */
}

BONOBO_TYPE_FUNC_FULL (GnomePrintBonobo, 
		       Bonobo_Print,
		       BONOBO_OBJECT_TYPE,
		       gnome_print_bonobo);

