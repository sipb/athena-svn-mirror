/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print.c: Abstract base class of gnome-print drivers
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
 *    Raph Levien <raph@acm.org>
 *    Miguel de Icaza <miguel@kernel.org>
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio <chema@celorio.com>
 *
 *  Copyright (C) 1999-2001 Ximian Inc. and authors
 *
 */

#define __GNOME_PRINT_C__

#include <string.h>
#include <gmodule.h>
#include <bonobo/bonobo-types.h>

#include "gnome-print-i18n.h"
#include "gnome-print-private.h"
#include "gp-gc-private.h"
#include "gnome-print-transport.h"
#include "gnome-print-ps2.h"
#include "gnome-print-frgba.h"
#include "libgnomeprint-marshal.h"

static void gnome_print_context_class_init (GnomePrintContextClass *klass);
static void gnome_print_context_init (GnomePrintContext *pc);

static void gnome_print_context_finalize (GObject *object);

static GnomePrintContext *gnome_print_context_create (gpointer get_type, GnomePrintConfig *config);

static GObjectClass *parent_class = NULL;

GType
gnome_print_context_get_type (void)
{
	static GType pc_type = 0;
	if (!pc_type) {
		static const GTypeInfo pc_info = {
			sizeof (GnomePrintContextClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_context_class_init,
			NULL, NULL,
			sizeof (GnomePrintContext),
			0,
			(GInstanceInitFunc) gnome_print_context_init
		};
		pc_type = g_type_register_static (G_TYPE_OBJECT, "GnomePrintContext", &pc_info, 0);
	}
	return pc_type;
}

static void
gnome_print_context_class_init (GnomePrintContextClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass*) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_print_context_finalize;
}

static void
gnome_print_context_init (GnomePrintContext *pc)
{
	pc->config = NULL;
	pc->transport = NULL;

	pc->gc = gp_gc_new ();
	pc->haspage = FALSE;
}

static void
gnome_print_context_finalize (GObject *object)
{
	GnomePrintContext *pc;

	pc = GNOME_PRINT_CONTEXT (object);

	if (pc->transport) {
		g_warning ("file %s: line %d: Destorying Context with open transport", __FILE__, __LINE__);
		g_object_unref (G_OBJECT (pc->transport));
		pc->transport = NULL;
	}

	if (pc->config) {
		pc->config = gnome_print_config_unref (pc->config);
	}

	gp_gc_unref (pc->gc);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

gint
gnome_print_context_construct (GnomePrintContext *pc, GnomePrintConfig *config)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (config != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	g_return_val_if_fail (pc->config == NULL, GNOME_PRINT_ERROR_UNKNOWN);

	pc->config = gnome_print_config_ref (config);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->construct)
		GNOME_PRINT_CONTEXT_GET_CLASS (pc)->construct (pc);

	return GNOME_PRINT_OK;
}

gint
gnome_print_context_create_transport (GnomePrintContext *pc)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (pc->config != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (pc->transport == NULL, GNOME_PRINT_ERROR_UNKNOWN);

	pc->transport = gnome_print_transport_new (pc->config);
	g_return_val_if_fail (pc->transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	return GNOME_PRINT_OK;
}

/* Direct class-method frontends */

/**
 * gnome_print_beginpage:
 * @pc: A #GnomePrintContext
 * @name: Name of the page
 *
 * Starts new output page with @name. Naming is used for interactive
 * contexts like #GnomePrintPreview and Document Structuring Convention
 * conformant PostScript output.
 * This function has to be called before any drawing methods and immediately
 * after each #gnome_print_showpage albeit the last one. It also resets
 * graphic state values (transformation, color, line properties, font),
 * so one has to define these again at the beginning of each page.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_beginpage (GnomePrintContext *pc, const guchar *name)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (!pc->haspage, GNOME_PRINT_ERROR_NOMATCH);

	gp_gc_reset (pc->gc);
	pc->haspage = TRUE;

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->beginpage)
		return GNOME_PRINT_CONTEXT_GET_CLASS (pc)->beginpage (pc, name);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_showpage:
 * @pc: A #GnomePrintContext
 *
 * Finishes rendering of current page, and marks it as shown. All subsequent
 * drawing methods will fail, until new page is started with #gnome_print_newpage.
 * Printing contexts may process drawing methods differently - some do
 * rendering immediately (like #GnomePrintPreview), some accumulate all
 * operators to internal stack, and only after #gnome_print_showpage is
 * any output produced.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_showpage (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);

#ifdef ALLOW_BROKEN_PGL
	if (!pc->haspage) {
		g_warning ("file %s: line %d: Missing beginpage in print job", __FILE__, __LINE__);
		gnome_print_beginpage (pc, "Unnamed");
	}
#else
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
#endif

	ret = GNOME_PRINT_OK;

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->showpage)
		ret = GNOME_PRINT_CONTEXT_GET_CLASS (pc)->showpage (pc);

	pc->haspage = FALSE;

	return ret;
}

/**
 * gnome_print_gsave:
 * @pc: A #GnomePrintContext
 *
 * Saves current graphic state (transformation, color, line properties, font)
 * into stack (push). Values itself remain unchanged.
 * You can later restore saved values, using #gnome_print_grestore, but not
 * over page boundaries. Graphic state stack has to be cleared for each
 * #gnome_print_showpage, i.e. the number of #gnome_print_gsave has to
 * match the number of #gnome_print_grestore for each page.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_gsave (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);

#ifdef ALLOW_BROKEN_PGL
	if (!pc->haspage) {
		g_warning ("file %s: line %d: Missing beginpage in print job", __FILE__, __LINE__);
		gnome_print_beginpage (pc, "Unnamed");
	}
#else
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
#endif

	ret = GNOME_PRINT_OK;

	gp_gc_gsave (pc->gc);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->gsave)
		ret = GNOME_PRINT_CONTEXT_GET_CLASS (pc)->gsave (pc);

	return ret;
}

/**
 * gnome_print_grestore:
 * @pc: A #GnomePrintContext
 *
 * Retrieves last saved graphic state from stack (pop). Stack has to be
 * at least the size of one.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_grestore (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);

#ifdef ALLOW_BROKEN_PGL
	if (!pc->haspage) {
		g_warning ("file %s: line %d: Missing beginpage in print job", __FILE__, __LINE__);
		gnome_print_beginpage (pc, "Unnamed");
	}
#else
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
#endif

	ret = GNOME_PRINT_OK;

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->grestore)
		ret = GNOME_PRINT_CONTEXT_GET_CLASS (pc)->grestore (pc);

	gp_gc_grestore (pc->gc);

	return ret;
}

int
gnome_print_clip_bpath_rule (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail ((rule == ART_WIND_RULE_NONZERO) || (rule == ART_WIND_RULE_ODDEVEN), GNOME_PRINT_ERROR_BADVALUE);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->clip)
		return GNOME_PRINT_CONTEXT_GET_CLASS (pc)->clip (pc, bpath, rule);

	return GNOME_PRINT_OK;
}

int
gnome_print_fill_bpath_rule (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail ((rule == ART_WIND_RULE_NONZERO) || (rule == ART_WIND_RULE_ODDEVEN), GNOME_PRINT_ERROR_BADVALUE);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->fill)
		return GNOME_PRINT_CONTEXT_GET_CLASS (pc)->fill (pc, bpath, rule);

	return GNOME_PRINT_OK;
}

int
gnome_print_stroke_bpath (GnomePrintContext *pc, const ArtBpath *bpath)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (bpath != NULL, GNOME_PRINT_ERROR_BADVALUE);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->stroke)
		return GNOME_PRINT_CONTEXT_GET_CLASS (pc)->stroke (pc, bpath);

	return GNOME_PRINT_OK;
}

int
gnome_print_image_transform (GnomePrintContext *pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (affine != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (px != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (w > 0, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (h > 0, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (rowstride >= ch * w, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail ((ch == 1) || (ch == 3) || (ch == 4), GNOME_PRINT_ERROR_BADVALUE);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->image)
		return GNOME_PRINT_CONTEXT_GET_CLASS (pc)->image (pc, affine, px, w, h, rowstride, ch);

	return GNOME_PRINT_OK;
}

int
gnome_print_glyphlist_transform (GnomePrintContext *pc, const gdouble *affine, GnomeGlyphList *gl)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (affine != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (gl != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (GNOME_IS_GLYPHLIST (gl), GNOME_PRINT_ERROR_BADVALUE);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->glyphlist)
		return GNOME_PRINT_CONTEXT_GET_CLASS (pc)->glyphlist (pc, affine, gl);

	return GNOME_PRINT_OK;
}

gint
gnome_print_callback_closure_invoke (GnomePrintContext *pc,
				     gpointer           pagedata,
				     gpointer           docdata,
				     GClosure          *closure)
{
	guint  ret;

	bonobo_closure_invoke (
		closure, G_TYPE_UINT, &ret,
		GNOME_TYPE_PRINT_CONTEXT, pc,
		G_TYPE_POINTER, pagedata,
		G_TYPE_POINTER, docdata, 0);

	return ret;
}

gint
gnome_print_page_callback_closure (GnomePrintContext *pc,
				   const guchar      *name,
				   gpointer           pagedata,
				   gpointer           docdata,
				   GClosure          *closure)
{
	guint  ret;
	GValue ret_val = {0, };

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (!pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (closure != NULL, GNOME_PRINT_ERROR_BADVALUE);

	g_value_init (&ret_val, G_TYPE_INT);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->page)
		return GNOME_PRINT_CONTEXT_GET_CLASS (pc)->page (
			pc, name, pagedata, docdata, closure);

	ret = gnome_print_beginpage (pc, name);
	g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);

	ret = gnome_print_callback_closure_invoke (
		pc, pagedata, docdata, closure);
	g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);

	ret = gnome_print_showpage (pc);
	g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);

	return ret;
}


gint
gnome_print_page_callback (GnomePrintContext *pc, GnomePrintPageCallback callback,
			   const guchar *name, gpointer pagedata, gpointer docdata)
{
	GClosure *closure;
	gint      ret;

	closure = bonobo_closure_store (
		g_cclosure_new (G_CALLBACK (callback), NULL, NULL),
		libgnomeprint_marshal_INT__OBJECT_POINTER_POINTER);

	ret = gnome_print_page_callback_closure (
		pc, name, pagedata, docdata, closure);

	g_closure_unref (closure);

	return ret;
}

/**
 * gnome_print_context_close:
 * @pc: A #GnomePrintContext
 *
 * Informs given #GnomePrintContext that application has finished print
 * job. From that point on, @pc has to be considered illegal pointer,
 * and any further printing operation with it may kill application.
 * Some printing contexts may not start printing before context is
 * closed.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_context_close (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);

	ret = GNOME_PRINT_OK;

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->close)
		ret = GNOME_PRINT_CONTEXT_GET_CLASS (pc)->close (pc);

	g_return_val_if_fail (ret == GNOME_PRINT_OK, GNOME_PRINT_ERROR_UNKNOWN);

	if (pc->transport) {
		g_warning ("file %s: line %d: Closing Context did not clear transport", __FILE__, __LINE__);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return ret;
}

/**
 * gnome_print_context_new:
 * @config: GnomePrintConfig object to query print settings from
 *
 * This method gives you new #GnomePrintContext object, associated with given
 * #GnomePrinter. You should use the resulting object as 'black box', without
 * assuming anything about it's type, as depending on situation appropriate
 * wrapper context may be used instead of direct driver.
 *
 * Returns: The new #GnomePrintContext or %NULL, if there is an error
 */

GnomePrintContext *
gnome_print_context_new (GnomePrintConfig *config)
{
	GnomePrintContext *pc;
	guchar *drivername;

	g_return_val_if_fail (config != NULL, NULL);

	drivername = gnome_print_config_get (config, "Settings.Engine.Backend.Driver");
	g_return_val_if_fail (drivername != NULL, NULL);

	pc = NULL;

	if (!strcmp (drivername, "gnome-print-ps")) {
		GnomePrintContext *ps;
		ps = gnome_print_ps2_new (config);
		pc = gnome_print_frgba_new (ps);
		g_object_unref (G_OBJECT (ps));
	} else {
		guchar *modulename;
		modulename = gnome_print_config_get (config, "Settings.Engine.Backend.Driver.Module");
		if (modulename) {
			static GHashTable *modules = NULL;
			GModule *module;
			if (!modules) modules = g_hash_table_new (g_str_hash, g_str_equal);
			module = g_hash_table_lookup (modules, modulename);
			if (!module) {
				gchar *path;
				path = g_module_build_path (GNOME_PRINT_LIBDIR "/drivers", modulename);
				module = g_module_open (path, G_MODULE_BIND_LAZY);
				if (module) g_hash_table_insert (modules, g_strdup (modulename), module);
				g_free (path);
			}
			if (module) {
				gpointer get_type;
				if (g_module_symbol (module, "gnome_print__driver_get_type", &get_type)) {
					pc = gnome_print_context_create (get_type, config);
				} else {
					g_warning ("Missing gnome_print__driver_get_type in %s\n", modulename);
					g_module_close (module);
				}
			} else {
				g_warning ("Cannot open module: %s\n", modulename);
			}
			g_free (modulename);
		} else {
			g_warning ("Unknown driver: %s", drivername);
		}
	}

	g_free (drivername);

	return pc;
}

static GnomePrintContext *
gnome_print_context_create (gpointer get_type, GnomePrintConfig *config)
{
	GnomePrintContext *pc;
	GType (* driver_get_type) (void);
	GType type;

	driver_get_type = get_type;

	type = (* driver_get_type) ();
	g_return_val_if_fail (g_type_is_a (type, GNOME_TYPE_PRINT_CONTEXT), NULL);

	pc = g_object_new (type, NULL);
	gnome_print_context_construct (pc, config);

	return pc;
}

