#include <math.h>
#include <libart_lgpl/art_affine.h>
#include <gnome.h>

#include "libgnomeprint/gp-unicode.h"
#include "libgnomeprint/gnome-printer.h"
#include "libgnomeprint/gnome-printer-dialog.h"

#include "libgnomeprint/gnome-print-rbuf.h"
#include "libgnomeprint/gnome-print-frgba.h"
#include "libgnomeprint/gnome-print-preview.h"

#include "libgnomeprint/gnome-font-dialog.h"

#define P_WIDTH (21 * 72)
#define P_HEIGHT (30 * 72)

static int preview_gdk;
static int preview;
static int rbuf;
static int rbuf_alpha;
static int rbuf_frgba;

static struct poptOption options [] = {
	{ "preview", 0, POPT_ARG_NONE, &preview },
	{ "preview-gdk", 0, POPT_ARG_NONE, &preview_gdk },
	{ "rbuf", 0, POPT_ARG_NONE, &rbuf },
	{ "rbuf-alpha", 0, POPT_ARG_NONE, &rbuf_alpha },
	{ "rbuf-frgba", 0, POPT_ARG_NONE, &rbuf_frgba },
	{NULL}
};

static void do_image (GnomePrintContext * pc, gdouble size, gint alpha);
static gint latin_to_utf8 (guchar * text, guchar * utext, gint ulength);

static void
do_image (GnomePrintContext * pc, gdouble size, gboolean alpha)
{
	guchar * image;
	gint x, y;
	guchar * p;
	gint save_level = 300;
	gint i;

	image = g_new (guchar, 256 * 256 * 4);
	for (y = 0; y < 256; y++) {
		p = image + 256 * 4 * y;
		for (x = 0; x < 256; x++) {
			*p++ = x;
			*p++ = y;
			*p++ = 0;
			*p++ = alpha ? x : 0xff;
		}
	}

	for (i=0;i<save_level;i++)
		gnome_print_gsave (pc);
		
	gnome_print_translate (pc, 0.0, size);
	gnome_print_scale (pc, size, -size);
	gnome_print_rgbaimage (pc, image, 256, 256, 4 * 256);

	for (i=0;i<save_level;i++)
		gnome_print_grestore (pc);
	
	g_free (image);
}

static void
do_star (GnomePrintContext * pc, gdouble size) {
	gdouble angle, ra, x, y;

	gnome_print_moveto (pc, 0.5 * size, 0.0);

#define ANGLE (M_PI / 3.0)
#define HANGLE (M_PI / 6.0)

	for (angle = 0.0; angle < 359.0; angle += 60.0) {
		ra = angle * M_PI / 180.0;
		x = size * cos (ra + HANGLE);
		y = size * sin (ra + HANGLE);
		gnome_print_lineto (pc, x, y);
		x = 0.5 * size * cos (ra + ANGLE);
		y = 0.5 * size * sin (ra + ANGLE);
		gnome_print_lineto (pc, x, y);
	}

	gnome_print_closepath (pc);
}

static void
do_stars (GnomePrintContext * pc, gdouble size, gint num)
{
	gdouble step, i;

	step = size / ((double) num - 0.99);

	for (i = 0; i < size; i += step) {
		do_star (pc, size);
		gnome_print_translate (pc, step, step / 4);
	}
}

static void
do_rosette (GnomePrintContext * pc, GnomeFont * font)
{
	static guchar sym[] = {0xce, 0x93, 0xce, 0xb5,
			       0xce, 0xb9, 0xce, 0xac,
			       0x20, 0xcf, 0x83, 0xce,
			       0xb1, 0xcf, 0x82, 0x00};
	GnomeDisplayFont * df;
	GnomeFont * tf, * cf;
	GnomeGlyphList * gl;
	gdouble angle;
	gdouble m[6];
	gchar u[256];
	gint len;

	tf = gnome_font_new ("Times-Bold", 19.0);
	g_assert (df = gnome_get_display_font ("ITC Zapf Chancery", GNOME_FONT_BOOK, FALSE, 19.0, 1.0));
	g_assert (gnome_display_font_get_gdk_font (df));
	cf = gnome_font_new ("Symbol", 10.0);

	gl = gnome_glyphlist_from_text_dumb (font, 0xff0000ff, 1.0, 0.0, "");

	len = latin_to_utf8 ("ÕÄÖÜ", u, 256);
	if (len > 0) gnome_glyphlist_text_sized_dumb (gl, u, len);
	gnome_glyphlist_rmoveto (gl, 12.0, 0.0);
	gnome_glyphlist_color (gl, 0x00ff00ff);
	gnome_glyphlist_font (gl, tf);
	len = latin_to_utf8 ("õäöü", u, 256);
	if (len > 0) gnome_glyphlist_text_sized_dumb (gl, u, len);
	gnome_glyphlist_color (gl, 0x0000ffff);
	gnome_glyphlist_text_dumb (gl, "THIS is ");
	gnome_glyphlist_color (gl, 0x000000ff);
	gnome_glyphlist_font (gl, cf);
	gnome_glyphlist_text_dumb (gl, sym);
#if 0
	gnome_glyphlist_text_dumb (gl, "glyphlist");
#endif

	for (angle = 0.0; angle < 360.0; angle += 78.0) {
		art_affine_rotate (m, angle);
		gnome_print_gsave (pc);
		gnome_print_concat (pc, m);
		gnome_print_moveto (pc, 0.0, 0.0);
#if 1
		gnome_print_glyphlist (pc, gl);
#else
		gnome_print_setfont (pc, cf);
		gnome_print_show (pc, sym);
#endif
		gnome_print_grestore (pc);
	}
}

static void
do_print (GnomePrintContext * pc, gdouble scale)
{
	static double dash[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
	static gint n_dash = 6;
	GnomeFont * font;

	font = gnome_font_new ("Helvetica", 12.0);

	gnome_print_beginpage (pc, "Test");

	gnome_print_scale (pc, scale, scale);

	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 0.0, 0.0);
	gnome_print_lineto (pc, 0.0, P_HEIGHT);
	gnome_print_lineto (pc, P_WIDTH, P_HEIGHT);
	gnome_print_lineto (pc, P_WIDTH, 0.0);
	gnome_print_closepath (pc);
	gnome_print_gsave (pc);
	gnome_print_setopacity (pc, 1.0);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_setlinewidth (pc, 6.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);
	gnome_print_setopacity (pc, 1.0);
	gnome_print_setrgbcolor (pc, 1.0, 1.0, 1.0);
#if 1
	gnome_print_eofill (pc);
#endif

	gnome_print_gsave (pc);
	gnome_print_translate (pc, 100.0, 100.0);
	gnome_print_newpath (pc);
	do_stars (pc, 100.0, 5);
	gnome_print_setrgbcolor (pc, 1.0, 0.5, 0.0);
	gnome_print_setopacity (pc, 0.7);
	gnome_print_eofill (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave (pc);
	gnome_print_translate (pc, 300.0, 100.0);
	gnome_print_newpath (pc);
	do_stars (pc, 100.0, 5);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_setopacity (pc, 1.0);
	gnome_print_setlinewidth (pc, 4.0);
	gnome_print_setdash (pc, n_dash, dash, 0.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_translate (pc, 200, 400);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_scale (pc, 3.0, 3.0);
	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 20.0, 20.0);
/*	gnome_print_show (pc, "Hello");*/
	gnome_print_grestore (pc);

	gnome_print_newpath (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.5, 0.0);
	gnome_print_setopacity (pc, 0.6);
	gnome_print_translate (pc, 0.0, 30.0);
	gnome_print_scale (pc, 2.0, 2.0);
	do_rosette (pc, font);
	gnome_print_grestore (pc);

	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 50.0, 50.0);
	gnome_print_lineto (pc, 50.0, 250.0);
	gnome_print_lineto (pc, 250.0, 250.0);
	gnome_print_lineto (pc, 250.0, 50.0);
	gnome_print_closepath (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave (pc);
#if 1
	gnome_print_eoclip (pc);
#endif
	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 1.0, 0.5, 0.0);
	gnome_print_setopacity (pc, 0.6);
	gnome_print_translate (pc, 130.0, 130.0);
	gnome_print_scale (pc, 2.0, 2.0);
	do_rosette (pc, font);
	gnome_print_grestore (pc);

	gnome_print_translate (pc, 100.0, 100.0);
	gnome_print_scale (pc, 0.5, 0.5);

	gnome_print_newpath (pc);
	do_star (pc, 200.0);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_setlinewidth (pc, 4.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave (pc);
	gnome_print_eoclip (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.5, 1.0);
	gnome_print_setopacity (pc, 0.6);
	gnome_print_translate (pc, 0.0, 0.0);
	gnome_print_scale (pc, 2.0, 2.0);
	do_rosette (pc, font);
	gnome_print_grestore (pc);

	do_image (pc, 150, TRUE);

	gnome_print_translate (pc, 45.0, 45.0);
	gnome_print_scale (pc, 0.5, 0.5);

	gnome_print_newpath (pc);
	do_star (pc, 100.0);

	gnome_print_gsave (pc);
	gnome_print_setlinewidth (pc, 36.0);
	gnome_print_strokepath (pc);

	gnome_print_gsave (pc);
	gnome_print_setlinewidth (pc, 2.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 1.0, 0.5, 0.5);
	gnome_print_eofill (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave (pc);
	gnome_print_eoclip (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.2, 0.0);
	gnome_print_setopacity (pc, 0.9);
	gnome_print_translate (pc, 130.0, 130.0);
	gnome_print_scale (pc, 8.0, 8.0);
	do_rosette (pc, font);
	gnome_print_grestore (pc);

	gnome_print_rotate (pc, 30.0);
	do_image (pc, 100, FALSE);
}

static gint
delete_event (GtkWidget * widget)
{
	gtk_main_quit ();
	return FALSE;
}

static gint
do_dialog (void)
{
     GnomePrinter *printer;
     GnomePrintContext *pc;

     printer = gnome_printer_dialog_new_modal ();
	
     if (!printer)
	  return 0;

     pc = gnome_print_context_new_with_paper_size (printer, "US-Letter");

     do_print (pc, 1.0);

     return 0;
}


static void
do_preview (gboolean aa)
{
	GtkWidget * w, * sw, * c;
	GnomePrintContext * pc;
	GnomeFont * font;

	font = gnome_font_new ("Helvetica-Oblique", 18.0);
	w = gnome_font_selection_dialog_new ("Test");
	gtk_widget_show (w);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_usize (w, 512, 512);

	gtk_signal_connect (GTK_OBJECT (w), "delete_event",
		GTK_SIGNAL_FUNC (delete_event), NULL);

	sw = gtk_scrolled_window_new (NULL, NULL);

	if (aa) {
		gtk_widget_push_colormap (gdk_rgb_get_cmap ());
		gtk_widget_push_visual (gdk_rgb_get_visual ());

		c = gnome_canvas_new_aa ();

		gtk_widget_pop_visual ();
		gtk_widget_pop_colormap ();

	} else {
		gtk_widget_push_colormap (gdk_rgb_get_cmap ());
		gtk_widget_push_visual (gdk_rgb_get_visual ());

		c = gnome_canvas_new ();

		gtk_widget_pop_visual ();
		gtk_widget_pop_colormap ();

	}

	gnome_canvas_set_scroll_region ((GnomeCanvas *) c, 0, 0, P_WIDTH, P_HEIGHT);

	gtk_container_add (GTK_CONTAINER (sw), c);
	gtk_container_add (GTK_CONTAINER (w), sw);

	gtk_widget_show_all (w);

	pc = gnome_print_preview_new (GNOME_CANVAS (c), "A4");

	do_print (pc, 1.0);

	gnome_print_context_close (pc);
}

#define PMSCALE 0.5
#define PMW (PMSCALE * P_WIDTH)
#define PMH (PMSCALE * P_HEIGHT)

static void
do_rbuf (gboolean alpha, gboolean frgba)
{
	GtkWidget * w, * sw, * p;
	gint bpp;
	guchar * buf;
	gdouble p2b[6];
	GnomePrintContext * pc;
	GdkPixbuf * pb;
	GdkPixmap * pm;
	GdkBitmap * bm;

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_usize (w, 512, 512);

	gtk_signal_connect (GTK_OBJECT (w), "delete_event",
		GTK_SIGNAL_FUNC (delete_event), NULL);

	sw = gtk_scrolled_window_new (NULL, NULL);

	bpp = (alpha) ? 4 : 3;
	art_affine_scale (p2b, PMSCALE, -PMSCALE);
	p2b[5] = PMH;

	buf = g_new (guchar, PMW * PMH * bpp);

	pc = gnome_print_rbuf_new (buf, PMW, PMH, bpp * PMW, p2b, alpha);

	if (frgba) {
		pc = gnome_print_frgba_new (pc);
	}

	do_print (pc, 2.0);

	gnome_print_context_close (pc);

	pb = gdk_pixbuf_new_from_data (buf, GDK_COLORSPACE_RGB, alpha,
		8, PMW, PMH, bpp * PMW, NULL, NULL);

	gdk_pixbuf_render_pixmap_and_mask (pb, &pm, &bm, 128);

	gdk_pixbuf_unref (pb);
	g_free (buf);

	p = gtk_pixmap_new (pm, bm);

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), p);
	gtk_container_add (GTK_CONTAINER (w), sw);

	gtk_widget_show_all (w);
}

int
main (int argc, char ** argv)
{
	gnome_init_with_popt_table ("TestPrint", "0.1", argc, argv, options, 0, NULL);

	if (preview) {
		do_preview (TRUE);
		gtk_main ();
	} else if (preview_gdk) {
		do_preview (FALSE);
		gtk_main ();
	} else if (rbuf) {
		do_rbuf (FALSE, FALSE);
		gtk_main ();
	} else 	if (rbuf_alpha) {
		do_rbuf (TRUE, FALSE);
		gtk_main ();
	} else if (rbuf_frgba) {
		do_rbuf (TRUE, TRUE);
		gtk_main ();
	} else {
	     do_dialog ();
	}


	return 0;
}

static gint
latin_to_utf8 (guchar * text, guchar * utext, gint ulength)
{
	guchar * i, * o;

	o = utext;

	for (i = text; *i; i++) {
		o += g_unichar_to_utf8 (*i, o);
	}

	return o - utext;
}

