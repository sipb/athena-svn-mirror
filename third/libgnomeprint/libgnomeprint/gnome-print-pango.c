/* 
 * gnome-print-pango.c
 *
 * Copyright (C) 2003 Jean Brefort <jean.brefort@ac-dijon.fr>
 * Copyright (C) 2004 Red Hat, Inc.
 *
 * Developed by Jean Brefort <jean.brefort@ac-dijon.fr>
 * Rewritten by Owen Taylor <otaylor@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#define PANGO_ENABLE_BACKEND	/* Needed to access PangoFcFont.font_pattern */

#include <config.h>

#include <libgnomeprint/gnome-print-pango.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gp-gc-private.h>

#include <pango/pangofc-font.h>
#include <pango/pangoft2.h>

/**
 * is_gnome_print_key :
 * 
 * We want to make sure that the Pango objects that get used
 * are the ones we create or there can be subtle bugs, so we
 * use a flag in gnome-print data to track this. It might be
 * cleaner to just derive a fontmap type, but there is
 * currently no pango_context_get_fontmap().
 **/
static GQuark
is_gnome_print_key (void)
{
	static GQuark gnome_print_key = 0;

	if (!gnome_print_key)
		gnome_print_key = g_quark_from_static_string ("gnome-print-pango");

	return gnome_print_key;
}

static void
set_is_gnome_print_object (GObject *gobject)
{
	g_object_set_qdata (gobject, is_gnome_print_key (),
			    GUINT_TO_POINTER (TRUE));
}

static gboolean
is_gnome_print_object (GObject *gobject)
{
	return g_object_get_qdata (gobject, is_gnome_print_key ()) != NULL;
}

static gboolean
is_gnome_print_layout (PangoLayout *layout)
{
	PangoContext *context = pango_layout_get_context (layout);

	return is_gnome_print_object (G_OBJECT (context));
}

/* This function gets called to convert a matched pattern into what
 * we'll use to actually load the font. We turn off hinting since we
 * want metrics that are independent of scale.
 */
static void
substitute_func (FcPattern *pattern, gpointer   data)
{
	FcPatternDel (pattern, FC_HINTING);
	FcPatternAddBool (pattern, FC_HINTING, FALSE);
}

/**
 * gnome_print_pango_font_map_new:
 * 
 * Creates a new #PangoFontMap object suitable for use with
 * gnome-print. In most cases, you probably want to use
 * gnome_print_pango_get_default_font_map () instead.
 * 
 * Return value: a newly created #PangoFontMap object
 **/
PangoFontMap *
gnome_print_pango_font_map_new (void)
{
	PangoFontMap *fontmap = NULL;
	PangoFT2FontMap *ft2fontmap;
		
	fontmap = pango_ft2_font_map_new ();
	ft2fontmap = PANGO_FT2_FONT_MAP (fontmap);
	
	pango_ft2_font_map_set_resolution (ft2fontmap, 72, 72);
	pango_ft2_font_map_set_default_substitute (ft2fontmap, substitute_func, NULL, NULL);

	set_is_gnome_print_object (G_OBJECT (fontmap));

	return fontmap;
}

/**
 * gnome_print_pango_get_default_font_map:
 * 
 * Gets a singleton #PangoFontMap object suitable for
 * use with gnome-print.
 * 
 * Return value: the default #PangoFontMap object for gnome-print. The
 * returned object is owned by gnome-print and should not be modified.
 * (If you need to set custom options, create a new font map with
 * gnome_print_pango_font_map_new().)  The reference count is
 * <emphasis>not</emphasis> increased.
 **/
PangoFontMap *
gnome_print_pango_get_default_font_map (void)
{
	static PangoFontMap *fontmap = NULL;

	if (!fontmap)
		fontmap = gnome_print_pango_font_map_new ();

	return fontmap;
}

/**
 * gnome_print_pango_create_context:
 * @fontmap: a #PangoFontMap from gnome_print_pango_get_default_font_map()
 *  or gnome_print_pango_create_font_map().
 * 
 * Creates a new #PangoContext object for the specified fontmap.
 * 
 * Return value: a newly created #PangoContext object
 **/
PangoContext *
gnome_print_pango_create_context (PangoFontMap *fontmap)
{
	PangoContext *context;

	g_return_val_if_fail (PANGO_FT2_IS_FONT_MAP (fontmap), NULL);
	g_return_val_if_fail (is_gnome_print_object (G_OBJECT (fontmap)), NULL);

	context = pango_ft2_font_map_create_context (PANGO_FT2_FONT_MAP (fontmap));
	set_is_gnome_print_object (G_OBJECT (context));

	return context;
}

/**
 * gnome_print_pango_update_context:
 * @context: a #PangoContext from gnome_print_pango_create_context().
 * @gpc: a #GnomePrintContext
 * 
 * Update a context so that layout done with it reflects the
 * current state of @gpc. In general, every time you use a
 * #PangoContext with a different #GnomePrintContext, or
 * you change the transformation matrix of the #GnomePrintContext
 * (other than pure translations) you should call this function.
 * You also need to call pango_layout_context_changed() for any
 * #PangoLayout objects that exit for the #PangoContext.
 *
 * This function currently does nothing and that isn't expected
 * to change for gnome-print. The main benefit of calling it
 * is that your code will be properly prepared for conversion
 * to use with future rendering systems such as Cairo where
 * the corresponding operation will actually do something.
 */ 
void
gnome_print_pango_update_context (PangoContext      *context,
				  GnomePrintContext *gpc)
				  
{
	g_return_if_fail (PANGO_IS_CONTEXT (context));
	g_return_if_fail (is_gnome_print_object (G_OBJECT (context)));
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (gpc));
	
	/* nothing */
}

/**
 * gnome_print_pango_create_layout:
 * @gpc: a #GnomePrintContext
 * 
 * Convenience function that creates a new #PangoContext, updates
 * it for rendering to @gpc, and then creates a #PangoLayout for
 * that #PangoContext. Generally this function suffices for
 * most usage of gnome-print with Pango and you don't need to
 * deal with the #PangoContext directly.
 * 
 * Return value: the newly created #PangoLayout. Free with
 *  g_object_unref() when you are done with it.
 **/
PangoLayout *
gnome_print_pango_create_layout (GnomePrintContext *gpc)
{
	PangoFontMap *fontmap;
	PangoContext *context;
	PangoLayout *layout;

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (gpc), NULL);

	fontmap = gnome_print_pango_get_default_font_map ();
	context = gnome_print_pango_create_context (fontmap);
	/* gnome_print_pango_update_context (gpc, context); */
	
	layout = pango_layout_new (context);

	g_object_unref (context);

	return layout;
}

/* Finds the #GnomeFont corresponding to a #PangoFont. This differs
 * from gnome_font_face_find_closest_from_pango_font() because it
 * requires a PangoFcFont not an arbitrary font and works off the
 * filename rather than making guesses about name correspondences.
 */
static GnomeFont *
font_from_pango_font (PangoFont *font)
{
	PangoFcFont *fcfont;
	FcChar8 *filename;
	gint id;
	gdouble size;
	
	if (!PANGO_IS_FC_FONT (font))
		return NULL;

	fcfont = PANGO_FC_FONT (font);

	if (FcPatternGetString (fcfont->font_pattern, FC_FILE, 0, &filename) != FcResultMatch)
		return NULL;
      
	if (FcPatternGetInteger (fcfont->font_pattern, FC_INDEX, 0, &id) != FcResultMatch)
		return NULL;
	
	if (FcPatternGetDouble (fcfont->font_pattern, FC_SIZE, 0, &size) != FcResultMatch)
		return NULL;

	return gnome_font_find_from_filename (filename, id, size);
}

/**
 * gnome_print_pango_glyph_string:
 * @gpc: a #GnomePrintContext
 * @font: the #PangoFont that the glyphs in @glyphs are from
 * @glyphs: a #PangoGlyphString
 * 
 * Draws the glyphs in @glyphs into the specified #GnomePrintContext.
 * Positioning information in @glyphs is transformed by the current
 * transformation matrix, the glyphs are drawn in the current color,
 * and the glyphs are positioned so that the left edge of the baseline
 * is at the current point.
 **/
void
gnome_print_pango_glyph_string (GnomePrintContext *gpc, PangoFont *font, PangoGlyphString  *glyphs)
{
	GnomeGlyphList *glyph_list;
	GnomeFont *gnome_font;
	gint x_off = 0;
	gint i;

	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (gpc));
	g_return_if_fail (PANGO_IS_FONT (font));
	g_return_if_fail (glyphs != NULL);
	
	gnome_font = font_from_pango_font (font);
	if (!gnome_font)
		return;

	glyph_list = gnome_glyphlist_new ();

	gnome_glyphlist_font (glyph_list, gnome_font);
	g_object_unref (gnome_font);
	
	gnome_glyphlist_color (glyph_list, gp_gc_get_rgba (gpc->gc));
	
	for (i = 0; i < glyphs->num_glyphs; i++) {
		PangoGlyphInfo *gi = &glyphs->glyphs[i];

		if (gi->glyph) {
			int x = x_off + gi->geometry.x_offset;
			int y = gi->geometry.y_offset;
			
			gnome_glyphlist_moveto (glyph_list,
						(gdouble) x / PANGO_SCALE, (gdouble) y / PANGO_SCALE);
			gnome_glyphlist_glyph (glyph_list, gi->glyph);
		}
			
		x_off += glyphs->glyphs[i].geometry.width;
	}

	gnome_print_glyphlist (gpc, glyph_list);
	gnome_glyphlist_unref (glyph_list);
}

typedef struct
{
	PangoUnderline  uline;
	gboolean        strikethrough;
	PangoColor     *fg_color;
	PangoColor     *bg_color;
	PangoRectangle *shape_ink_rect;
	PangoRectangle *shape_logical_rect;
	gint            rise;
} ItemProperties;

/* Retrieves the properties we care about for a single run in an
 * easily accessible structure rather than a list of attributes
 */
static void
get_item_properties (PangoItem *item, ItemProperties *properties)
{
	GSList *tmp_list = item->analysis.extra_attrs;

	properties->uline = PANGO_UNDERLINE_NONE;
	properties->strikethrough = FALSE;
	properties->fg_color = NULL;
	properties->bg_color = NULL;
	properties->rise = 0;
	properties->shape_ink_rect = NULL;
	properties->shape_logical_rect = NULL;

	while (tmp_list) {
		PangoAttribute *attr = tmp_list->data;
	  
		switch (attr->klass->type) {
		case PANGO_ATTR_UNDERLINE:
			properties->uline = ((PangoAttrInt *)attr)->value;
			break;
		  
		case PANGO_ATTR_STRIKETHROUGH:
			properties->strikethrough = ((PangoAttrInt *)attr)->value;
			break;
		  
		case PANGO_ATTR_FOREGROUND:
			properties->fg_color = &((PangoAttrColor *)attr)->color;
			break;
		  
		case PANGO_ATTR_BACKGROUND:
			properties->bg_color = &((PangoAttrColor *)attr)->color;
			break;
		  
		case PANGO_ATTR_SHAPE:
			properties->shape_logical_rect = &((PangoAttrShape *)attr)->logical_rect;
			properties->shape_ink_rect = &((PangoAttrShape *)attr)->ink_rect;
			break;
		
		case PANGO_ATTR_RISE:
			properties->rise = ((PangoAttrInt *)attr)->value;
			break;
		  
		default:
			break;
		}
		tmp_list = tmp_list->next;
	}
}

/*
 * Couple of helper functions to work in Pango units rather than doubles
 */
static void
rect_filled (GnomePrintContext *gpc, gint x, gint y, gint width, gint height)
{
	gnome_print_rect_filled (gpc,
				 (gdouble) x / PANGO_SCALE,     (gdouble) y / PANGO_SCALE,
				 (gdouble) width / PANGO_SCALE, (gdouble) height / PANGO_SCALE);
}

static void
translate (GnomePrintContext *gpc, gint x, gint y)
{
	gnome_print_translate (gpc,
			       (gdouble) x / PANGO_SCALE, (gdouble) y / PANGO_SCALE);
}

static void
moveto (GnomePrintContext *gpc, gint x, gint y)
{
	gnome_print_moveto (gpc,
			    (gdouble) x / PANGO_SCALE, (gdouble) y / PANGO_SCALE);
}

/* It's easiest for us to work in coordinates where the current point (which
 * determines the origin of drawing for the #PangoGlyphString, #PangoLayout,
 * or #PangoLayoutLine) is at the origin. This function changes the CTM to
 * that coordinate system.
 */
static void
current_point_to_origin (GnomePrintContext *gpc)
{
	const gdouble *ctm;
	const ArtPoint *cp;
	gdouble affine[6];

	ctm = gp_gc_get_ctm (gpc->gc);
	cp = gp_gc_get_currentpoint (gpc->gc);

	affine[0] = ctm[0];
	affine[1] = ctm[1];
	affine[2] = ctm[2];
	affine[3] = ctm[3];
	affine[4] = cp->x;
	affine[5] = cp->y;

	gp_gc_setmatrix (gpc->gc, affine);
}

/* Draws an error underline that looks like one of:
 *              H       E                H
 *     /\      /\      /\        /\      /\               -
 *   A/  \    /  \    /  \     A/  \    /  \              |
 *    \   \  /    \  /   /D     \   \  /    \             |
 *     \   \/  C   \/   /        \   \/   C  \            | height = HEIGHT_SQUARES * square
 *      \      /\  F   /          \  F   /\   \           | 
 *       \    /  \    /            \    /  \   \G         |
 *        \  /    \  /              \  /    \  /          |
 *         \/      \/                \/      \/           -
 *         B                         B       
 * |----|
 *   unit_width = (HEIGHT_SQUARES - 1) * square
 *
 * The x, y, width, height passed in give the desired bounding box;
 * x/width are adjusted to make the underline a integer number of units
 * wide.
 */
#define HEIGHT_SQUARES 2.5

static void
draw_error_underline (GnomePrintContext *gpc, gdouble x, gdouble y, gdouble width, gdouble height)
{
	gdouble square = height / HEIGHT_SQUARES;
	gdouble unit_width = (HEIGHT_SQUARES - 1) * square;
	gint width_units = (width + unit_width / 2) / unit_width;
	gdouble y_top, y_bottom;
	gint i;

	x += (width - width_units * unit_width);
	width = width_units * unit_width;

	gnome_print_newpath (gpc);

	y_top = y + height;
	y_bottom = y;

	/* Bottom of squiggle */
	gnome_print_moveto (gpc, x - square / 2, y_top - square / 2); /* A */
	for (i = 0; i < width_units; i += 2) {
		gdouble x_middle = x + (i + 1) * unit_width;
		gdouble x_right = x + (i + 2) * unit_width;
		
		gnome_print_lineto (gpc, x_middle, y_bottom); /* B */

		if (i + 1 == width_units)
			/* Nothing */;
		else if (i + 2 == width_units)
			gnome_print_lineto (gpc, x_right + square / 2, y_top - square / 2); /* D */
		else
			gnome_print_lineto (gpc, x_right, y_top - square); /* C */
	}

	/* Top of squiggle */
	for (i -= 2; i >= 0; i -= 2) {
		gdouble x_left = x + i * unit_width;
		gdouble x_middle = x + (i + 1) * unit_width;
		gdouble x_right = x + (i + 2) * unit_width;
		
		if (i + 1 == width_units)
			gnome_print_lineto (gpc, x_middle + square / 2, y_bottom + square / 2); /* G */
		else {
			if (i + 2 == width_units)
				gnome_print_lineto (gpc, x_right, y_top); /* E */
			gnome_print_lineto (gpc, x_middle, y_bottom + square); /* F */
		}
		
		gnome_print_lineto (gpc, x_left, y_top);   /* H */
	}

	gnome_print_closepath (gpc);
	gnome_print_fill (gpc);
}

/* Helper function to draw the underline for one layout run; the descent field gives
 * the descent of the ink for the actual text in the layout run.
 */
static void
draw_underline (GnomePrintContext *gpc, PangoFontMetrics  *metrics, PangoUnderline uline, gint x, gint width, gint descent)
{
	gint underline_thickness = pango_font_metrics_get_underline_thickness (metrics);
	gint underline_position = pango_font_metrics_get_underline_position (metrics);
	gint y_off = 0;		/* Quiet GCC */
  
	switch (uline) {
	case PANGO_UNDERLINE_NONE:
		g_assert_not_reached();
		break;
	case PANGO_UNDERLINE_SINGLE:
		y_off = underline_position - underline_thickness;
		break;
	case PANGO_UNDERLINE_DOUBLE:
		y_off = underline_position - underline_thickness;
		break;
	case PANGO_UNDERLINE_LOW:
		y_off = - 2 * underline_thickness - descent;
		break;
	case PANGO_UNDERLINE_ERROR:
		draw_error_underline (gpc,
				      (gdouble) x / PANGO_SCALE,
				      (gdouble) (underline_position - 3 * underline_thickness) / PANGO_SCALE,
				      (gdouble) width / PANGO_SCALE,
				      (gdouble) (3 * underline_thickness) / PANGO_SCALE);
		return;
	}
	
	rect_filled (gpc,
		     x, y_off,
		     width, underline_thickness);
	
	if (uline == PANGO_UNDERLINE_DOUBLE)
		rect_filled (gpc,
			     x, y_off - 2 * underline_thickness,
			     width, underline_thickness);
}

/* Helper function to draw a strikethrough for one layout run
 */
static void
draw_strikethrough (GnomePrintContext *gpc, PangoFontMetrics  *metrics, gint x, gint width)
{
	gint strikethrough_thickness = pango_font_metrics_get_strikethrough_thickness (metrics);
	gint strikethrough_position = pango_font_metrics_get_strikethrough_position (metrics);

	rect_filled (gpc,
		     x, strikethrough_position - strikethrough_thickness,
		     width, strikethrough_thickness);
}

static void 
print_pango_layout_line (GnomePrintContext *gpc, PangoLayoutLine *line)
{
	GSList *tmp_list = line->runs;
	PangoRectangle overall_rect;
	PangoRectangle logical_rect;
	PangoRectangle ink_rect;
	gint x_off = 0;
	
	gnome_print_gsave (gpc);

	current_point_to_origin (gpc);

	pango_layout_line_get_extents (line, NULL, &overall_rect);
	
	while (tmp_list) {
		ItemProperties properties;
		PangoLayoutRun *run = tmp_list->data;
		
		tmp_list = tmp_list->next;
		
		get_item_properties (run->item, &properties);

		if (properties.shape_logical_rect) {
			x_off += properties.shape_logical_rect->width;
			continue;
		}

		gnome_print_gsave (gpc);

		translate (gpc, x_off, properties.rise);
		gnome_print_moveto (gpc, 0, 0);

		if (properties.uline == PANGO_UNDERLINE_NONE && !properties.strikethrough)
			pango_glyph_string_extents (run->glyphs, run->item->analysis.font,
						    NULL, &logical_rect);
		else
			pango_glyph_string_extents (run->glyphs, run->item->analysis.font,
						    &ink_rect, &logical_rect);
		
		if (properties.bg_color) {
			gnome_print_gsave (gpc);

			gnome_print_setrgbcolor (gpc,
						 (gdouble) properties.bg_color->red / 0xFFFF,
						 (gdouble) properties.bg_color->green / 0xFFFF,
						 (gdouble) properties.bg_color->blue / 0xFFFF);

			rect_filled (gpc,
				     logical_rect.x,    - overall_rect.y - overall_rect.height,
				     logical_rect.width,  overall_rect.height);

			gnome_print_grestore (gpc);
		}

		if (properties.fg_color) {
			gnome_print_setrgbcolor (gpc,
						 (gdouble) properties.fg_color->red / 0xFFFF,
						 (gdouble) properties.fg_color->green / 0xFFFF,
						 (gdouble) properties.fg_color->blue / 0xFFFF);
		}

		gnome_print_pango_glyph_string (gpc, run->item->analysis.font, run->glyphs);
		
		if (properties.uline != PANGO_UNDERLINE_NONE || properties.strikethrough) {
			PangoFontMetrics *metrics = pango_font_get_metrics (run->item->analysis.font,
									    run->item->analysis.language);
	      
			if (properties.uline != PANGO_UNDERLINE_NONE)
				draw_underline (gpc, metrics,
						properties.uline,
						ink_rect.x,
						ink_rect.width,
						ink_rect.y + ink_rect.height);
			
			if (properties.strikethrough)
				draw_strikethrough (gpc, metrics,
						    ink_rect.x,
						    ink_rect.width);

			pango_font_metrics_unref (metrics);
		}
		
		gnome_print_grestore (gpc);
	
		x_off += logical_rect.width;
	}
	
	gnome_print_grestore (gpc);
}

/**
 * gnome_print_pango_layout_line:
 * @gpc: a #GnomePrintContext
 * @line: a #PangoLayoutLine
 *
 * Draws the text in @line into the specified #GnomePrintContext.  The
 * text is drawn in the current color unless that has been overridden
 * by attributes set on the layout and the glyphs are positioned so
 * that the left edge of the baseline is at the current point.
 **/
void 
gnome_print_pango_layout_line (GnomePrintContext *gpc, PangoLayoutLine *line)
{
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (gpc));
	g_return_if_fail (line->layout != NULL);
	g_return_if_fail (PANGO_IS_LAYOUT (line->layout));
	g_return_if_fail (is_gnome_print_layout (line->layout));

	print_pango_layout_line (gpc, line);
}

static void 
print_pango_layout (GnomePrintContext *gpc, PangoLayout *layout)
{
	PangoLayoutIter *iter;
	
	gnome_print_gsave (gpc);
	
	current_point_to_origin (gpc);
	
	iter = pango_layout_get_iter (layout);
	
	do {
		PangoRectangle   logical_rect;
		PangoLayoutLine *line;
		int              baseline;
		
		line = pango_layout_iter_get_line (iter);
		
		pango_layout_iter_get_line_extents (iter, NULL, &logical_rect);
		baseline = pango_layout_iter_get_baseline (iter);
		
		moveto (gpc, logical_rect.x, - baseline);
		print_pango_layout_line (gpc, line);

	} while (pango_layout_iter_next_line (iter));

	pango_layout_iter_free (iter);
	
	gnome_print_grestore (gpc);
}

/**
 * gnome_print_pango_layout:
 * @gpc: a #GnomePrintContext
 * @layout: a #PangoLayout
 * 
 * Draws the text in @layout into the specified #GnomePrintContext.  The
 * text is drawn in the current color unless that has been overridden
 * by attributes set on the layout and the glyphs are positioned so
 * that the left edge of the baseline is at the current point.
 **/
void 
gnome_print_pango_layout (GnomePrintContext *gpc, PangoLayout *layout)
{
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (gpc));
	g_return_if_fail (PANGO_IS_LAYOUT (layout));
	g_return_if_fail (is_gnome_print_layout (layout));

	print_pango_layout (gpc, layout);
}

/**
 * gnome_print_pango_layout_print:
 * @gpc: a #GnomePrintContext
 * @pl: the #PangoLayout to print
 * 
 * Draws the text in @pl into the specified #GnomePrintContext.  The
 * text is drawn in the current color unless that has been overridden
 * by attributes set on the layout and the glyphs are positioned so
 * that the left edge of the baseline is at the point (0, 0). This function
 * is obsolete; use gnome_print_pango_layout() instead.
 **/
void
gnome_print_pango_layout_print (GnomePrintContext *gpc, PangoLayout* pl)
{
	/* For compatibility with old code we allow users to use
	 * this to print PangoLayout's from contexts that we
	 * didn't create ourself. The spacing may be very bad,
	 * because of hinting differences, but no worse than
	 * before.
	 */
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (gpc));
	g_return_if_fail (PANGO_IS_LAYOUT (pl));
	
	gnome_print_gsave (gpc);
	gnome_print_moveto (gpc, 0, 0);
	print_pango_layout (gpc, pl);
	gnome_print_grestore (gpc);
}
