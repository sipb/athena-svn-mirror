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
#include <stdarg.h>

#include <libart_lgpl/art_affine.h>

#include <bonobo/bonobo-exception.h>

#include <bonobo/bonobo-stream-client.h>
#include <libgnomeprint/gnome-print-bonobo-client.h>

#undef PRINT_DEBUG

struct _GnomePrintBonoboDimensions {
	Bonobo_PrintDimensions pd;
	Bonobo_PrintScissor    sc;
};

struct _GnomePrintBonoboData {
	GnomePrintBonoboDimensions dims;

	guchar                    *data;
	guint                      length;
};

/**
 * gnome_print_bonobo_dimensions_free:
 * @dims: the dimensions set
 * 
 * release the resources associated with a print dimension set.
 **/
void
gnome_print_bonobo_dimensions_free (GnomePrintBonoboDimensions *dims)
{
	g_free (dims);
}

/**
 * gnome_print_bonobo_data_free:
 * @pd: the print data
 * 
 * release the resources associated with a remotely printed
 * resource.
 **/
void
gnome_print_bonobo_data_free (GnomePrintBonoboData *pd)
{
	if (pd) {
		g_free (pd->data);
		pd->data = NULL;

		g_free (pd);
	}
}

/**
 * gnome_print_bonobo_data_get_meta:
 * @pd: the print data
 * 
 * Return value: the meta_data from @pd
 **/
GnomePrintMeta *
gnome_print_bonobo_data_get_meta (GnomePrintBonoboData *pd)
{
	GnomePrintContext *meta;

	g_return_val_if_fail (pd != NULL, NULL);
	g_return_val_if_fail (pd->data != NULL, NULL);
	
	meta = gnome_print_meta_new_local ();

	gnome_print_meta_render_data (meta, pd->data, pd->length);

	return GNOME_PRINT_META (meta);
}

/**
 * gnome_print_bonobo_dimensions_new_full:
 * @width: the width in pts of the component to render
 * @height: the height in pts of the component to render
 * @width_first_page: the clear width available on the first page
 * @width_per_page: the width available on subsequent pages
 * @height_first_page: the clear height available on the first page
 * @height_per_page: the height available on subsequent pages
 * 
 * This initializes a GnomePrintBonoboDimensions to contain the above
 * parameters so that it can be used by #gnome_print_bonobo_client_remote_render
 * 
 * Return value: a new GnomePrintBonoboDimensions structure.
 **/
GnomePrintBonoboDimensions *
gnome_print_bonobo_dimensions_new_full (double width,
					double height,
					double width_first_page,
					double width_per_page,
					double height_first_page,
					double height_per_page)
{
	GnomePrintBonoboDimensions *dims = g_new (
		GnomePrintBonoboDimensions, 1);

	dims->pd.width  = width;
	dims->pd.height = height;

	dims->sc.width_first_page  = width_first_page;
	dims->sc.width_per_page    = width_per_page;
	dims->sc.height_first_page = height_first_page;
	dims->sc.height_per_page   = height_per_page;

	return dims;
}

/**
 * gnome_print_bonobo_dimensions_new:
 * @width: the width in pts of the component to render
 * @height: the height in pts of the component to render
 * 
 * This constructs a GnomePrintBonoboDimensions with default
 * scissor data. see #gnome_print_bonobo_dimensions_new_full
 * 
 * Return value: a new GnomePrintBonoboDimensions structure.
 **/
GnomePrintBonoboDimensions *
gnome_print_bonobo_dimensions_new (double width,
			     double height)
{
	return gnome_print_bonobo_dimensions_new_full (
		width, height, width, 0.0, height, 0.0);
}

/**
 * gnome_print_bonobo_client_remote_render:
 * @print: the print client.
 * @dims: the dimensions to print the data into
 * @opt_ev: an optional CORBA_Environment
 * 
 * This routine is used to encourage a remote print client
 * to print itself. The GnomePrintBonoboDimensions specifies the size
 * information for the remote client to render itself to.
 * After render the GnomePrintBonoboData contains the meta data
 * for the rendered page. This interface is baroque.
 *
 * Returns: a new GnomePrintBonoboData structure.
 **/
GnomePrintBonoboData *
gnome_print_bonobo_client_remote_render (Bonobo_Print                      print,
					 const GnomePrintBonoboDimensions *dims,
					 CORBA_Environment                *opt_ev)
{
	Bonobo_Stream      stream;
	CORBA_Environment *ev, temp_ev;
	GnomePrintBonoboData   *pd;

	g_return_val_if_fail (dims != NULL, NULL);
	g_return_val_if_fail (print != CORBA_OBJECT_NIL, NULL);
       
	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	pd = g_new0 (GnomePrintBonoboData, 1);
	pd->dims = *dims;

	stream = Bonobo_Print_render (print, &dims->pd, &dims->sc, ev);

#ifdef PRINT_DEBUG
	{
		int i;

		printf ("Read data %d %d\n", data->_length, data->_maximum);
		for (i = 0; i < data->_length; i++)
			printf ("0x%x ", data->_buffer [i]);
		printf ("\n");
	}
#endif

	if (BONOBO_EX (ev)) {
		if (!opt_ev)
			g_warning ("Component print exception");
		return NULL;
	}
	if (stream == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			g_warning ("Component print returns no data");
		g_free (pd);
		return NULL;
	}

	pd->data = bonobo_stream_client_read (stream, -1, &pd->length, ev);
	if (BONOBO_EX (ev) || !pd->data)
		if (!opt_ev)
			g_warning ("Failed to read print data from stream");

	bonobo_object_release_unref (stream, ev);

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);

	return pd;
}

/**
 * gnome_print_bonobo_data_re_render:
 * @ctx: the context
 * @x: the tlc bbox x
 * @y: the ltc bbox y
 * @pd: the print data to render
 * @meta_x: the offset into the print data x
 * @meta_y: the offset into the print data y
 * 
 * This is used to render the print data in @pd
 * onto a GnomePrintContext in @ctx.
 **/
void
gnome_print_bonobo_data_re_render (GnomePrintContext    *ctx,
				   double                x,
				   double                y,
				   GnomePrintBonoboData *pd,
				   double                meta_x,
				   double                meta_y)
{
	double matrix[6];
	double w, h;
	GnomePrintBonoboDimensions *dims;

	g_return_if_fail (pd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (ctx));

	if (!pd->data)
		return;

	dims = &pd->dims;

	if (meta_x == 0.0)
		w = dims->sc.width_first_page;
	else
		w = dims->sc.width_per_page;

	if (meta_y == 0.0)
		h = dims->sc.height_first_page;
	else
		h = dims->sc.height_per_page;

	gnome_print_gsave (ctx);
	/* FIXME: we need a clip region & a different translation ! */
	gnome_print_moveto (ctx, x, y);
	gnome_print_lineto (ctx, x + w, y);
	gnome_print_lineto (ctx, x + w, y + h);
	gnome_print_lineto (ctx, x, y + h);
	gnome_print_lineto (ctx, x, y);
	gnome_print_clip   (ctx);

	art_affine_translate (matrix, x - meta_x, y - meta_y);
	gnome_print_concat (ctx, matrix);

	if (gnome_print_meta_render_data_page (ctx, pd->data, pd->length, 0, FALSE) != GNOME_PRINT_OK)
		g_warning ("Failed to meta render");

	gnome_print_grestore (ctx);

#ifdef PRINT_DEBUG
	{
		gnome_print_gsave (ctx);
		gnome_print_setlinewidth (ctx, 0);
		gnome_print_setrgbcolor (ctx, 1.0, 0.0, 0.0);

		gnome_print_moveto (ctx, x, y);
		gnome_print_lineto (ctx, x + w, y);
		gnome_print_lineto (ctx, x + w, y + h);
		gnome_print_lineto (ctx, x, y + h);
		gnome_print_lineto (ctx, x, y);
		gnome_print_stroke (ctx);

		gnome_print_grestore (ctx);
	}
#endif
}
