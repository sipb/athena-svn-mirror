/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-svg.c: the SVG backend
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useoful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *          Dom Lachowicz <cinamod@hotmail.com>
 *
 *  References:
 *          http://www.w3.org/TR/SVG12/
 *          http://www.w3.org/TR/SVGPrint/
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <png.h>

#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_misc.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gp-gc-private.h>
#include <libgnomeprint/gnome-print-transport.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/gnome-pgl-private.h>
#include <libgnomeprint/gnome-print-svg.h>

/*************************************************************/
/*************************************************************/

typedef struct _GnomePrintSvgClass GnomePrintSvgClass;
typedef struct _GnomePrintSvg GnomePrintSvg;

struct _GnomePrintSvgClass {
	GnomePrintContextClass parent_class;
};

struct _GnomePrintSvg {
	GnomePrintContext pc;

	gdouble page_ctm[6];
	gdouble page_width;
	gdouble page_height;

	gsize nb_clips; /* # of clipping paths */
};

typedef enum {
	SVG_COLOR_GROUP_FILL,
	SVG_COLOR_GROUP_STROKE,
	SVG_COLOR_GROUP_BOTH
} SvgColorGroup;

/*************************************************************/
/*************************************************************/

static gint gnome_print_svg_construct (GnomePrintContext *pc);

static gint gnome_print_svg_beginpage (GnomePrintContext *pc, const guchar *name);
static gint gnome_print_svg_showpage  (GnomePrintContext *pc);
static gint gnome_print_svg_close     (GnomePrintContext *pc);

static gint gnome_print_svg_gsave     (GnomePrintContext *pc);
static gint gnome_print_svg_grestore  (GnomePrintContext *pc);

static gint gnome_print_svg_clip      (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule);
static gint gnome_print_svg_fill      (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule);
static gint gnome_print_svg_stroke    (GnomePrintContext *pc, const ArtBpath *bpath);

static gint gnome_print_svg_image     (GnomePrintContext *pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch);
static gint gnome_print_svg_glyphlist (GnomePrintContext *pc, const gdouble *affine, GnomeGlyphList *gl);

static gint gnome_print_svg_set_color (GnomePrintSvg *svg, SvgColorGroup which);
static gint gnome_print_svg_set_line (GnomePrintSvg *svg);
static gint gnome_print_svg_set_dash (GnomePrintSvg *svg);
static gint gnome_print_svg_set_font (GnomePrintSvg *svg, const GnomeFont *font);

static void gnome_print_svg_class_init (GnomePrintSvgClass *klass);
static void gnome_print_svg_init (GnomePrintSvg *svg);
static void gnome_print_svg_finalize (GObject *object);

static gint gnome_print_svg_fprintf_double (GnomePrintSvg *svg, 
					    const gchar *format, gdouble x);
static gint gnome_print_svg_fprintf (GnomePrintSvg *svg, const char *format, ...);

/*************************************************************/
/*************************************************************/

static GnomePrintContextClass *parent_class;

GType
gnome_print_svg_get_type (void)
{
	static GType svg_type = 0;
	if (!svg_type) {
		static const GTypeInfo svg_info = {
			sizeof (GnomePrintSvgClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_svg_class_init,
			NULL, NULL,
			sizeof (GnomePrintSvg),
			0,	
			(GInstanceInitFunc) gnome_print_svg_init
		};
		svg_type = g_type_register_static (GNOME_TYPE_PRINT_CONTEXT, "GnomePrintSvg", &svg_info, 0);
	}
	return svg_type;
}

GnomePrintContext *
gnome_print_svg_new (GnomePrintConfig *config)
{
	GnomePrintContext *ctx;
	gint ret;

	g_return_val_if_fail (config != NULL, NULL);

	ctx = g_object_new (GNOME_TYPE_PRINT_SVG, NULL);

	ret = gnome_print_context_construct (ctx, config);

	if (ret != GNOME_PRINT_OK) {
		g_object_unref (G_OBJECT (ctx));
		ctx = NULL;
	}
	
	return ctx;
}

static void
gnome_print_svg_class_init (GnomePrintSvgClass *klass)
{
	GnomePrintContextClass *pc_class;
	GnomePrintSvgClass *svg_class;
	GObjectClass *object_class;

	object_class = (GObjectClass *) klass;
	pc_class = (GnomePrintContextClass *)klass;
	svg_class = (GnomePrintSvgClass *)klass;

	parent_class = g_type_class_peek_parent (klass);
	
	object_class->finalize = gnome_print_svg_finalize;

	pc_class->construct = gnome_print_svg_construct;
	pc_class->beginpage = gnome_print_svg_beginpage;
	pc_class->showpage = gnome_print_svg_showpage;
	pc_class->gsave = gnome_print_svg_gsave;
	pc_class->grestore = gnome_print_svg_grestore;
	pc_class->clip = gnome_print_svg_clip;
	pc_class->fill = gnome_print_svg_fill;
	pc_class->stroke = gnome_print_svg_stroke;
	pc_class->image = gnome_print_svg_image;
	pc_class->glyphlist = gnome_print_svg_glyphlist;
	pc_class->close = gnome_print_svg_close;
}

static void
gnome_print_svg_init (GnomePrintSvg *svg)
{
	art_affine_identity (svg->page_ctm);
	svg->page_width = svg->page_height = 0.;
}

static void
gnome_print_svg_finalize (GObject *object)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gnome_print_svg_construct (GnomePrintContext *ctx)
{
	GnomePrintSvg *svg;
	gint ret = GNOME_PRINT_OK;

	svg = GNOME_PRINT_SVG (ctx);

	ret += gnome_print_context_create_transport (ctx);
	ret += gnome_print_transport_open (ctx->transport);
	g_return_val_if_fail (ret >= 0, ret);
	
	if (ctx->config) {
		gdouble ctm[6];

		gnome_print_config_get_length (ctx->config, GNOME_PRINT_KEY_PAPER_WIDTH,  &svg->page_width, NULL);
		gnome_print_config_get_length (ctx->config, GNOME_PRINT_KEY_PAPER_HEIGHT, &svg->page_height, NULL);		

		svg->page_ctm[3] = -1;
		svg->page_ctm[5] = svg->page_height;

		gnome_print_config_get_transform (ctx->config, GNOME_PRINT_KEY_PAPER_ORIENTATION_MATRIX, ctm);

		art_affine_multiply(svg->page_ctm, ctm, svg->page_ctm);
	}

	gnome_print_svg_fprintf (svg, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	gnome_print_svg_fprintf (svg, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"); 
	gnome_print_svg_fprintf (svg, "\t\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
	gnome_print_svg_fprintf (svg, "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" ");

	if (svg->page_width > 0.) {
		gnome_print_svg_fprintf (svg, " width=\"");
		gnome_print_svg_fprintf_double (svg, "%g", svg->page_width);
		gnome_print_svg_fprintf (svg, "\"");
	}

	if (svg->page_height > 0.) {
		gnome_print_svg_fprintf (svg, " height=\"");
		gnome_print_svg_fprintf_double (svg, "%g", svg->page_height);
		gnome_print_svg_fprintf (svg, "\"");
	}

	gnome_print_svg_fprintf (svg, " version=\"1.2\" streamable=\"true\">\n");
	gnome_print_svg_fprintf (svg, "<!-- This SVG 1.2 file was created by GnomePrint: http://www.gnome.org -->\n");
	gnome_print_svg_fprintf (svg, "<!-- http://www.w3.org/TR/SVG12/ | http://www.w3.org/TR/SVGPrint/ -->\n");
	gnome_print_svg_fprintf (svg, "<pageSet>\n");

	return ret;
}

static gint
gnome_print_svg_gsave (GnomePrintContext *ctx)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG (ctx);

	gnome_print_svg_fprintf (svg, "<g>\n");

	return GNOME_PRINT_OK;
}

static gint
gnome_print_svg_grestore (GnomePrintContext *ctx)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG (ctx);

	gnome_print_svg_fprintf (svg, "</g>\n");
	
	return GNOME_PRINT_OK;
}

static gint
gnome_print_svg_print_bpath (GnomePrintSvg *svg, const ArtBpath *bpath)
{
	gboolean started, closed;
	
	started = closed = FALSE;

	gnome_print_svg_fprintf (svg, " d=\"");

	while (bpath->code != ART_END) {
		switch (bpath->code) {
		case ART_MOVETO_OPEN:
			if (started && closed)
				gnome_print_svg_fprintf (svg, " Z");

			started = closed = FALSE;

			gnome_print_svg_fprintf (svg, " M");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->x3);
			gnome_print_svg_fprintf (svg, ",");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->y3);
			break;
		case ART_MOVETO:
			if (started && closed)
				gnome_print_svg_fprintf (svg, " Z");

			started = closed = TRUE;

			gnome_print_svg_fprintf (svg, " M");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->x3);
			gnome_print_svg_fprintf (svg, ",");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->y3);
			break;
		case ART_LINETO:
			gnome_print_svg_fprintf (svg, " L");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->x3);
			gnome_print_svg_fprintf (svg, ",");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->y3);
			break;
		case ART_CURVETO:
			gnome_print_svg_fprintf (svg, " C");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->x1);
			gnome_print_svg_fprintf (svg, ",");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->y1);
			gnome_print_svg_fprintf (svg, " ");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->x2);
			gnome_print_svg_fprintf (svg, ",");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->y2);
			gnome_print_svg_fprintf (svg, " ");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->x3);
			gnome_print_svg_fprintf (svg, ",");
			gnome_print_svg_fprintf_double (svg, "%g", bpath->y3);
			break;
		default:
			g_warning ("Path structure is corrupted");
			return -1;
		}
		bpath += 1;
	}

	if(started && closed)
		gnome_print_svg_fprintf (svg, " Z");

	gnome_print_svg_fprintf (svg, "\"");
	
	return 0;
}

static gint
gnome_print_svg_close (GnomePrintContext *pc)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG (pc);

	g_return_val_if_fail (pc->transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	gnome_print_svg_fprintf (svg, "</pageSet>\n");
	gnome_print_svg_fprintf (svg, "</svg>\n");

 	gnome_print_transport_close (pc->transport);
	g_object_unref (G_OBJECT (pc->transport));
	pc->transport = NULL;
 	
	return GNOME_PRINT_OK;
}

static gint
gnome_print_svg_matrix (GnomePrintSvg *svg, const gdouble * ctm)
{
	gnome_print_svg_fprintf (svg, " transform=\"matrix(");
	gnome_print_svg_fprintf_double (svg, "%g", ctm[0]);
	gnome_print_svg_fprintf (svg, ", ");
	gnome_print_svg_fprintf_double (svg, "%g", ctm[1]);
	gnome_print_svg_fprintf (svg, ", ");
	gnome_print_svg_fprintf_double (svg, "%g", ctm[2]);
	gnome_print_svg_fprintf (svg, ", ");
	gnome_print_svg_fprintf_double (svg, "%g", ctm[3]);
	gnome_print_svg_fprintf (svg, ", ");
	gnome_print_svg_fprintf_double (svg, "%g", ctm[4]);
	gnome_print_svg_fprintf (svg, ", ");
	gnome_print_svg_fprintf_double (svg, "%g", ctm[5]);
	gnome_print_svg_fprintf (svg, ")\"");

	return GNOME_PRINT_OK;
}

static gint
gnome_print_svg_beginpage (GnomePrintContext *pc, const guchar *name)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG (pc);

	gnome_print_svg_fprintf (svg, "<page>\n");
	gnome_print_svg_fprintf (svg, "<g");
	gnome_print_svg_matrix (svg, svg->page_ctm);
	gnome_print_svg_fprintf (svg, ">\n");

	return GNOME_PRINT_OK;
}

static gint
gnome_print_svg_showpage (GnomePrintContext *pc)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG (pc);

	g_return_val_if_fail (svg != NULL, GNOME_PRINT_ERROR_BADCONTEXT);

	gnome_print_svg_fprintf (svg, "</g>\n");
	gnome_print_svg_fprintf (svg, "</page>\n");
	
	return GNOME_PRINT_OK;
}

/* Note "format" should be locale independent, so it should not use %g */
/* and friends */
static gint
gnome_print_svg_fprintf (GnomePrintSvg *svg, const char *format, ...)
{
 	va_list arguments;
	gint len;
 	gchar *text;

	/* Compose the text */
 	va_start (arguments, format);
 	text = g_strdup_vprintf (format, arguments);
 	va_end (arguments);
	
	/* Write it */
	len = gnome_print_transport_write (GNOME_PRINT_CONTEXT(svg)->transport, text, strlen(text));

	g_free (text);
	
	return len;
}

/* Allowed conversion specifiers are 'e', 'E', 'f', 'F', 'g' and 'G'. */
static gint   
gnome_print_svg_fprintf_double (GnomePrintSvg *svg, 
				const gchar *format, gdouble x)
{
 	gchar *text;
	gint len;

	text = g_new (gchar, G_ASCII_DTOSTR_BUF_SIZE);
	g_ascii_formatd (text, G_ASCII_DTOSTR_BUF_SIZE, format, x);

	len = gnome_print_transport_write (GNOME_PRINT_CONTEXT(svg)->transport, text, strlen(text));
	g_free (text);

	return len;
}

#define SVG_COLOR_NORM(c) (gint)((c)*255.)

static gint
gnome_print_svg_set_color_real (GnomePrintSvg *svg, SvgColorGroup color_group,
				gdouble r, gdouble g, gdouble b)
{
	if(color_group == SVG_COLOR_GROUP_FILL || color_group == SVG_COLOR_GROUP_BOTH)
		gnome_print_svg_fprintf (svg, " fill=\"#%02x%02x%02x\"",
					 SVG_COLOR_NORM(r), SVG_COLOR_NORM(g),
					 SVG_COLOR_NORM(b));
	else
		gnome_print_svg_fprintf (svg, " fill=\"none\"");

	if(color_group == SVG_COLOR_GROUP_STROKE || color_group == SVG_COLOR_GROUP_BOTH)
		gnome_print_svg_fprintf (svg, " stroke=\"#%02x%02x%02x\"",
					 SVG_COLOR_NORM(r), SVG_COLOR_NORM(g),
					 SVG_COLOR_NORM(b));
	else
		gnome_print_svg_fprintf (svg, " stroke=\"none\"");	

	return GNOME_PRINT_OK;
}

static gint
gnome_print_svg_set_color (GnomePrintSvg *svg, SvgColorGroup color_group)
{
	GnomePrintContext *ctx;
	gint ret = GNOME_PRINT_OK;

	ctx = GNOME_PRINT_CONTEXT (svg);

	gnome_print_svg_set_color_real (svg,
					color_group,
					gp_gc_get_red   (ctx->gc),
					gp_gc_get_green (ctx->gc),
					gp_gc_get_blue  (ctx->gc));
	
	return ret;
}

static const char *
art_linecap_to_svg(ArtPathStrokeCapType type)
{
	switch(type)
		{
		case ART_PATH_STROKE_CAP_ROUND:
			return "round";
		case ART_PATH_STROKE_CAP_SQUARE:
			return "square";
		default:
			return "butt";
		}

	return "";
}

static const char *
art_linejoin_to_svg(ArtPathStrokeJoinType type)
{
	switch(type)
		{
		case ART_PATH_STROKE_JOIN_ROUND:
			return "round";
		case ART_PATH_STROKE_JOIN_BEVEL:
			return "bevel";
		default:			
			return "miter";
		}
	
	return "";
}

static const char *
art_windrule_to_svg(ArtWindRule rule)
{
	switch(rule)
		{
		case ART_WIND_RULE_NONZERO:
			return "nonzero";
		default:
			return "evenodd";			
		}

	return "";
}

static gint
gnome_print_svg_set_line (GnomePrintSvg *svg)
{
	GnomePrintContext *ctx;

	ctx = GNOME_PRINT_CONTEXT (svg);

	gnome_print_svg_fprintf (svg, " stroke-width=\"");
	gnome_print_svg_fprintf_double (svg, "%g", 
					   gp_gc_get_linewidth (ctx->gc));
	gnome_print_svg_fprintf (svg, "\"");

	gnome_print_svg_fprintf (svg, " stroke-linecap=\"%s\"", art_linecap_to_svg(gp_gc_get_linecap (ctx->gc)));
	gnome_print_svg_fprintf (svg, " stroke-linejoin=\"%s\"", art_linejoin_to_svg(gp_gc_get_linejoin (ctx->gc)));

	gnome_print_svg_fprintf (svg, " stroke-miterlimit=\"");
	gnome_print_svg_fprintf_double (svg, "%g", 
					   gp_gc_get_miterlimit (ctx->gc));
	gnome_print_svg_fprintf (svg, "\"");

	return GNOME_PRINT_OK;
}
static gint
gnome_print_svg_set_dash (GnomePrintSvg *svg)
{
	GnomePrintContext *ctx;
	const ArtVpathDash *dash;
	gint i;

	ctx = GNOME_PRINT_CONTEXT (svg);

	dash = gp_gc_get_dash (ctx->gc);

	if(dash->n_dash > 0) {
		gnome_print_svg_fprintf (svg, " stroke-dasharray=\"");
		for (i = 0; i < dash->n_dash; i++) {
			gnome_print_svg_fprintf (svg, ",");
			gnome_print_svg_fprintf_double (svg, "%g", dash->dash[i]);
		}
		gnome_print_svg_fprintf (svg, "\"");
	}

	gnome_print_svg_fprintf (svg, " stroke-dashoffset=\"");
	gnome_print_svg_fprintf_double (svg, "%g", dash->n_dash > 0 ? dash->offset : 0.0);
	gnome_print_svg_fprintf (svg, "\"");

	return GNOME_PRINT_OK;
}

static gint
gnome_print_svg_set_font (GnomePrintSvg *svg, const GnomeFont *gnome_font)
{
	gnome_print_svg_fprintf (svg, " font-size=\"");
	gnome_print_svg_fprintf_double (svg, "%g", gnome_font_get_size(gnome_font));
	gnome_print_svg_fprintf (svg, "pt\"");

	gnome_print_svg_fprintf (svg, " font-style=\"%s\"",
				      gnome_font_is_italic (gnome_font) ? "italic" : "normal");

	gnome_print_svg_fprintf (svg, " font-weight=\"%d\"",
				      gnome_font_get_weight_code(gnome_font));

	gnome_print_svg_fprintf (svg, " font-family=\"'%s', '%s', '%s'\"",
				 gnome_font_get_name(gnome_font),
				 gnome_font_get_ps_name(gnome_font),
				 gnome_font_get_family_name(gnome_font));
	
	return GNOME_PRINT_OK;
}

static gint gnome_print_svg_clip (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG(pc);

	gnome_print_svg_fprintf (svg, "<clipPath id=\"clip-path-%ld\" clip-rule=\"%s\">\n<path ", 
				 svg->nb_clips, art_windrule_to_svg(rule));
	gnome_print_svg_print_bpath (svg, bpath);
	gnome_print_svg_fprintf (svg, "/>\n</clipPath>\n");

	gnome_print_svg_fprintf (svg, "<path");
	gnome_print_svg_print_bpath (svg, bpath);
	gnome_print_svg_fprintf (svg, " />\n", svg->nb_clips);

	svg->nb_clips++;

	return GNOME_PRINT_OK;
}

static gint gnome_print_svg_fill (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG(pc);

	gnome_print_svg_fprintf (svg, "<path");
	gnome_print_svg_print_bpath (svg, bpath);
	gnome_print_svg_set_color (svg, SVG_COLOR_GROUP_FILL);
	gnome_print_svg_set_line (svg);
	gnome_print_svg_set_dash (svg);

	gnome_print_svg_fprintf (svg, " fill-rule=\"%s\"", art_windrule_to_svg(rule));
	gnome_print_svg_fprintf (svg, "/>\n");

	return GNOME_PRINT_OK;
}

static gint gnome_print_svg_stroke (GnomePrintContext *pc, const ArtBpath *bpath)
{
	GnomePrintSvg *svg;

	svg = GNOME_PRINT_SVG(pc);

	gnome_print_svg_fprintf (svg, "<path");
	gnome_print_svg_print_bpath (svg, bpath);
	gnome_print_svg_set_color (svg, SVG_COLOR_GROUP_STROKE);
	gnome_print_svg_set_line (svg);
	gnome_print_svg_set_dash (svg);
	gnome_print_svg_fprintf (svg, "/>\n");

	return GNOME_PRINT_OK;
}

static const char s_UTF8_B64Alphabet[64] = {
	0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, /* A-Z */
	0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, /* a-z */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, /* 0-9 */
	0x2b, /* + */
	0x2f  /* / */
};
static const char s_UTF8_B64Pad = 0x3d;

static gboolean
utf8_base64_encode(char ** b64ptr, size_t * b64len, const char ** binptr, size_t * binlen)
{
	guint8 u1, u2, u3;

	while (((*binlen) >= 3) && ((*b64len) >= 4))
		{
			u1 = (guint8)(*(*binptr)++);
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u1>>2];
			u2 = (guint8)(*(*binptr)++);
			u1 = ((u1 & 0x03) << 4) | (u2 >> 4);
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u1];
			u2 = (u2 & 0x0f) << 2;
			u3 = (guint8)(*(*binptr)++);
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u2 | (u3 >> 6)];
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u3 & 0x3f];
			(*b64len) -= 4;
			(*binlen) -= 3;
		}

	if ((*binlen) >= 3) 
		return FALSE; /* huh? */
	if ((*binlen) == 0) 
		return TRUE;

	if ((*b64len) < 4) 
		return FALSE; /* huh? */

	if ((*binlen) == 2)
		{
			u1 = (guint8)(*(*binptr)++);
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u1>>2];
			u2 = (guint8)(*(*binptr)++);
			u1 = ((u1 & 0x03) << 4) | (u2 >> 4);
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u1];
			u2 = (u2 & 0x0f) << 2;
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u2];
			*(*b64ptr)++ = s_UTF8_B64Pad;
			(*b64len) -= 4;
			(*binlen) -= 2;
		}
	else /* if (binlen == 1) */
		{
			u1 = (guint8)(*(*binptr)++);
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u1>>2];
			u1 = (u1 & 0x03) << 4;
			*(*b64ptr)++ = s_UTF8_B64Alphabet[u1];
			*(*b64ptr)++ = s_UTF8_B64Pad;
			*(*b64ptr)++ = s_UTF8_B64Pad;
			(*b64len) -= 4;
			(*binlen) -= 1;
		}

	return TRUE;
}

static void _png_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
	GByteArray * pBB = (GByteArray *) png_get_io_ptr(png_ptr);

	g_byte_array_append(pBB, data, length);
}

static void _png_flush(png_structp png_ptr)
{
}

#define EPSILON 1e-9

static gint gnome_print_svg_image (GnomePrintContext *pc, const gdouble *a, 
				   const guchar *px, gint w, gint h, gint rowstride, gint ch)
{
	GnomePrintSvg * svg;

	png_structp png_ptr;
	png_infop info_ptr;
	GByteArray * pBB = NULL;
	gsize i;
	png_byte color_type;

	svg = GNOME_PRINT_SVG(pc);

	if(ch == 1)
		color_type = PNG_COLOR_TYPE_GRAY;
	else if(ch == 3)
		color_type = PNG_COLOR_TYPE_RGB;
	else if(ch == 4)
		color_type = PNG_COLOR_TYPE_RGBA;
	else {
		g_warning ("Unknown # of channels\n");
		return GNOME_PRINT_OK;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
					  (png_error_ptr)NULL, (png_error_ptr)NULL);
	
	info_ptr = png_create_info_struct(png_ptr);

	if (setjmp(png_ptr->jmpbuf))
	{
		/* If we get here, we had a problem reading the file */
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		if(pBB)
			g_byte_array_free(pBB, TRUE);
		
		g_warning("PNG encoding problems\n");

		return GNOME_PRINT_OK;
	}

	pBB = g_byte_array_new();
	png_set_write_fn(png_ptr, (void *)pBB, _png_write, _png_flush);

	png_set_IHDR(png_ptr,
		     info_ptr,
		     w,
		     h,
		     8,
		     color_type,
		     PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_BASE,
		     PNG_FILTER_TYPE_BASE);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	for (i = 0; i < h; i++)
	{
		guint8 *pRow = (guint8 *)(px + i * rowstride);
		
		png_write_rows(png_ptr, &pRow, 1);
	}

	/*
	  Wrap things up with libpng
	*/
	png_write_end(png_ptr, info_ptr);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

	{
		char buffer[75];
		char * bufptr = 0;
		size_t buflen;
		size_t imglen = pBB->len;
		const char * imgptr = pBB->data;
		gdouble dx, dy;
		gdouble ctm[6] = {1,0,0,-1,0,0};

		dx = a[4];
		dy = a[5];

		{
			gdouble Tm[6];
			art_affine_translate(Tm, dx, dy);
			art_affine_multiply(ctm, ctm, Tm);
		}

		buffer[0] = '\r';
		buffer[1] = '\n';

		gnome_print_svg_fprintf (svg, "<image width=\"");
		gnome_print_svg_fprintf_double (svg, "%g", w);
		gnome_print_svg_fprintf(svg, "\" height=\"");
		gnome_print_svg_fprintf_double (svg, "%g", h);
		gnome_print_svg_fprintf(svg, "\"");
		gnome_print_svg_matrix (svg, ctm);
		gnome_print_svg_fprintf (svg, " xlink:href=\"data:image/png;base64,");		

		while (imglen)
			{
				buflen = 72;
				bufptr = buffer + 2;
				
				utf8_base64_encode (&bufptr, &buflen, &imgptr, &imglen);

				*bufptr = 0;			       

				gnome_print_transport_write (pc->transport, buffer, sizeof(buffer)-1-buflen);	
			}

		gnome_print_svg_fprintf(svg, "\" />\n");
	}

	g_byte_array_free(pBB, TRUE);
	
	return GNOME_PRINT_OK;
}

#define UCS2_UPPER_BOUND 65535

/* this is a HACK and will fall over on its face in lots of different situations.
   TODO: GnomePrint API change:
   http://www.w3.org/TR/SVG/text.html#CharactersAndGlyphs
*/
static gunichar gnome_print_unichar_for_glyph (GnomePrintSvg * svg, GnomeFont * font, gint glyph)
{
	FT_Face ft_face;
	gsize i;

	/* TODO: glyph->unichar map, based on font.ps_name */

	ft_face = gnome_font_get_face(font)->ft_face;
	if(ft_face) {
		/* do a dumb, brute-force reverse-lookup */
		for(i = 0; i < UCS2_UPPER_BOUND; i++) {
			if(FT_Get_Char_Index (ft_face, i) == glyph) {
				return (gunichar)i;
			}
		}
	}

	return (gunichar)0;
}

#undef UCS2_UPPER_BOUND

static gint gnome_print_svg_glyphlist (GnomePrintContext *pc, const gdouble *a, GnomeGlyphList *gl)
{
	GnomePrintSvg * svg;
	static const gdouble id[] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
	GnomePosGlyphList *pgl;
	gboolean identity;
	gdouble dx, dy;
	gint ret, s;

	svg = GNOME_PRINT_SVG(pc);

	identity = ((fabs (a[0] - 1.0) < EPSILON) && (fabs (a[1]) < EPSILON) && (fabs (a[2]) < EPSILON) && (fabs (a[3] - 1.0) < EPSILON));

	if (!identity) {
		dx = dy = 0.0;
	} else {
		dx = a[4];
		dy = a[5];
	}

	pgl = gnome_pgl_from_gl (gl, id, GNOME_PGL_RENDER_DEFAULT);

	for (s = 0; s < pgl->num_strings; s++) {
		GnomePosString *ps;
		GnomeFont *font;
		gint i, len;
		gdouble ctm[6] = {1,0,0,-1,0,0};

		ps = pgl->strings + s;
		gnome_print_svg_fprintf (svg, "<text");
		font = gnome_rfont_get_font (ps->rfont);
		gnome_print_svg_set_font(svg, font);
		
		ret = gnome_print_svg_set_color_real (svg,
						      SVG_COLOR_GROUP_FILL,
						      ((ps->color >> 24) & 0xff) / 255.0,
						      ((ps->color >> 16) & 0xff) / 255.0,
						      ((ps->color >>  8) & 0xff) / 255.0);

		if(!identity)
			art_affine_multiply(ctm, ctm, a);
		else {
			gdouble Tm[6];
			art_affine_translate(Tm, dx + pgl->glyphs[ps->start].x, dy + pgl->glyphs[ps->start].y);
			art_affine_multiply(ctm, ctm, Tm);
		}

		gnome_print_svg_matrix (svg, ctm);

		ret = gnome_print_svg_fprintf (svg, ">");

		for (i = ps->start; i < ps->start + ps->length; i++) {
			gunichar ch;
			gchar utf8 [7];

			ch = gnome_print_unichar_for_glyph (svg, font, pgl->glyphs[i].glyph);
			if(ch != 0) {
				len = g_unichar_to_utf8 (ch, utf8);
				utf8[len] = '\0';
				
				ret = gnome_print_svg_fprintf (svg, "%s", utf8);
				g_return_val_if_fail (ret >= 0, ret);
			}
		}
			
		ret = gnome_print_svg_fprintf (svg, "</text>\n");
	}

	gnome_pgl_destroy (pgl);

	return GNOME_PRINT_OK;
}
