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
 */

#include <config.h>
#include <string.h>
#include <gmodule.h>

#include <libgnomeprint/gnome-print-i18n.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gp-gc-private.h>
#include <libgnomeprint/gnome-print-transport.h>
#include <libgnomeprint/gnome-print-ps2.h>
#include <libgnomeprint/gnome-print-pdf.h>
#include <libgnomeprint/gnome-print-frgba.h>

/* For the buffer stuff, remove when the buffer stuff is moved out here */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
/* Endif */

static void gnome_print_context_class_init (GnomePrintContextClass *klass);
static void gnome_print_context_init (GnomePrintContext *pc);
static void gnome_print_context_finalize (GObject *object);

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
	pc->pages = 0;
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
	GnomePrintReturnCode retval = GNOME_PRINT_OK;
	
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (config != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	g_return_val_if_fail (pc->config == NULL, GNOME_PRINT_ERROR_UNKNOWN);

	pc->config = gnome_print_config_ref (config);

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->construct)
		retval = GNOME_PRINT_CONTEXT_GET_CLASS (pc)->construct (pc);
	
	return retval;
}

gint
gnome_print_context_create_transport (GnomePrintContext *pc)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (pc->config != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (pc->transport == NULL, GNOME_PRINT_ERROR_UNKNOWN);

	pc->transport = gnome_print_transport_new (pc->config);
	
	if (pc->transport == NULL) {
		g_warning ("Could not create transport inside gnome_print_context_create_transport");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return GNOME_PRINT_OK;
}

/* Direct class-method frontends */

/**
 * gnome_print_beginpage:
 * @pc: A #GnomePrintContext
 * @name: Name of the page, NULL if you just want to use the page number of the page
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
	guchar *real_name;
	
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (!pc->haspage, GNOME_PRINT_ERROR_NOMATCH);

	pc->pages++;
	if (name == NULL) {
		real_name = g_strdup_printf ("%d", pc->pages);
	} else {
		real_name = (guchar *) name;
	}
	
	gp_gc_reset (pc->gc);
	pc->haspage = TRUE;

	if (GNOME_PRINT_CONTEXT_GET_CLASS (pc)->beginpage)
		return GNOME_PRINT_CONTEXT_GET_CLASS (pc)->beginpage (pc, real_name);

	if (name == NULL) {
		g_free (real_name);
	}

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
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

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
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

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
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

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

	if (ret != GNOME_PRINT_OK) {
		g_warning ("Could not close transport inside gnome_print_context_close");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	if (pc->transport) {
		g_warning ("file %s: line %d: Closing Context should clear transport", __FILE__, __LINE__);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return ret;
}

/**
 * gnome_print_context_new:
 * @config: GnomePrintConfig object to query print settings from
 *
 * Create new printing context from config. You have to have set
 * all the options/settings beforehand, as changing the config of
 * an existing context has undefined results.
 *
 * Also, if creating the context by hand, it completely ignores layout and
 * orientation value. If you need those, use GnomePrintJob. The
 * latter also can create output context for you, so in most cases
 * you may want to ignore gnome_print_context_new at all.
 *
 * Returns: The new GnomePrintContext or NULL on error
 */
GnomePrintContext *
gnome_print_context_new (GnomePrintConfig *config)
{
	GnomePrintContext *pc = NULL;
	guchar *drivername;

	g_return_val_if_fail (config != NULL, NULL);

	drivername = gnome_print_config_get (config, "Settings.Engine.Backend.Driver");
	if (drivername == NULL) {
		drivername = g_strdup ("gnome-print-ps");
	}

	if (strcmp (drivername, "gnome-print-ps") == 0) {
		GnomePrintContext *ps;
		ps = gnome_print_ps2_new (config);
		if (ps == NULL)
			return NULL;
		pc = gnome_print_frgba_new (ps);
		if (pc == NULL)
			return NULL;
		g_object_unref (G_OBJECT (ps));
		g_free (drivername);
		return pc;
	}
	
	if (strcmp (drivername, "gnome-print-pdf") == 0) {
		pc = gnome_print_pdf_new (config);
		if (pc == NULL)
			return NULL;
		g_free (drivername);
		return pc;
	}

	g_free (drivername);

	return pc;
}


void
gnome_print_buffer_munmap (GnomePrintBuffer *b)
{
	if (b->buf)
		munmap (b->buf, b->buf_size);
	b->buf = NULL;
	b->buf_size = 0;
}

gint
gnome_print_buffer_mmap (GnomePrintBuffer *b,
			 const guchar *file_name)
{
	struct stat s;
	gint fh;

	b->buf = NULL;
	b->buf_size = 0;

	fh = open (file_name, O_RDONLY);
	if (fh < 0) {
		g_warning ("Can't open \"%s\"", file_name);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	if (fstat (fh, &s) != 0) {
		g_warning ("Can't stat \"%s\"", file_name);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	
	b->buf = mmap (NULL, s.st_size, PROT_READ, MAP_SHARED, fh, 0);
	b->buf_size = s.st_size;
	
	close (fh);
	if ((b->buf == NULL) || (b->buf == (void *) -1)) {
		g_warning ("Can't mmap file %s", file_name);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return GNOME_PRINT_OK;
}
