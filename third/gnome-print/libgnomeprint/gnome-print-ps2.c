/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/*
    gnome-print-ps2.c: A Postscript driver for GnomePrint based	in
		gnome-print-pdf which was based on the PS driver by Raph Levien.

    Copyright 2000 Jose M Celorio

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Authors:
     Chema Celorio <chema@celorio.com>
		 Lauris Kaplinski <lauris@helixcode.com>
*/

/* __FUNCTION__ is not defined in Irix according to David Kaelbling <drk@sgi.com>*/
#ifndef __GNUC__
  #define __FUNCTION__   ""
#endif
#define debug(section,str) if (FALSE) printf ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str); 
	
#include "config.h"
#include <time.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_misc.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-print-ps2.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-print-encode.h>

#include <libgnomeprint/gnome-pgl-private.h>

#define EOL "\n"
#define GNOME_PRINT_PS2_BUFFER_SIZE 1024
#define GNOME_PRINT_PS2_NUMBER_OF_ELEMENTS 3
#define GNOME_PRINT_PS2_NUMBER_OF_ELEMENTS_GROW 2
#define GNOME_PRINT_PS2_FONT_UNDEFINED 9999

static GnomePrintContextClass *parent_class = NULL;

typedef struct _GnomePrintDash            GnomePrintDash;
typedef struct _GnomePrintPs2Page         GnomePrintPs2Page;
typedef struct _GnomePrintPs2Font         GnomePrintPs2Font;
typedef struct _GnomePrintPs2Gsave        GnomePrintPs2Gsave;
typedef struct _GnomePrintPs2GraphicState GnomePrintPs2GraphicState;

typedef enum {
	PS2_COLOR_MODE_DEVICEGRAY,
	PS2_COLOR_MODE_DEVICERGB,
	PS2_COLOR_MODE_DEVICECMYK,
	PS2_COLOR_MODE_UNDEFINED
}Ps2ColorModes;

struct _GnomePrintDash {
  gint   number_values;
  double phase;
  double *values;
};

struct _GnomePrintPs2Font {
	GnomeFont *gnome_font;
	gint       font_number;
	gchar     *font_name;
	gboolean   reencoded;
	gboolean   external;
};

struct _GnomePrintPs2Gsave {
	gint graphics_mode;
	GnomePrintPs2GraphicState *graphic_state;
	GnomePrintPs2GraphicState *graphic_state_set;
};

struct _GnomePrintPs2Page{
	gint showpaged : 1;

	/* Page number */
	guint  page_number;
	gchar *page_name;
};

struct _GnomePrintPs2GraphicState{
	gboolean dirty        : 1;
	gboolean writen      : 1;

	/* CTM */
	double ctm[6];
	
	/* Current Path */
	GPPath * current_path;
	gint path_loaded;

	/* Color */
	gint color_mode;
	double color [4];
	
	/* Line stuff */
	gint linejoin;     /* 0, 1 or 2 */
	gint linecap;      /* 0, 1 or 2 */
	gint flat;         /* 0 <-> 100 */
	double miterlimit; /* greater or equal than 1 */
	double linewidth;  /* 0 or greater */
	
	/* Line dash */
	GnomePrintDash dash;

  /* Font */
	gint   ps2_font_number;
  double font_size;
	double font_character_spacing;
	double font_word_spacing;
	gint   text_flag : 1;

	/* Stuff for gnome-text lines */
	GnomeTextFontHandle text_font_handle;
	gint text_font_size;
};

struct _GnomePrintPs2 {
	
	GnomePrintContext pc;

	/* Graphic states */
	GnomePrintPs2GraphicState *graphic_state;
	GnomePrintPs2GraphicState *graphic_state_set;
	gint  graphics_mode;

	/* Offset in the output file */
	guint offset;
	
	/* Page stuff */
	GList             *pages;
	gint               current_page_number;
	GnomePrintPs2Page *current_page;

  /* Fonts */
  gint  fonts_internal_number;
  gchar **fonts_internal;

  gint  fonts_external_number;
  gint  fonts_external_max;
  gchar **fonts_external;

	gint fonts_max;
	gint fonts_number;
	GnomePrintPs2Font* fonts;

	/* gsave/grestore */
	gint gsave_level_number;
	gint gsave_level_max;
	GnomePrintPs2Gsave *gsave_stack;
};

struct _GnomePrintPs2Class
{
  GnomePrintContextClass parent_class;
};

static void gnome_print_ps2_class_init    (GnomePrintPs2Class *klass);
static void gnome_print_ps2_init          (GnomePrintPs2 *PS2);
static void gnome_print_ps2_destroy       (GtkObject *object);
static gint gnome_print_ps2_dictionary    (GnomePrintContext *pc);

/* -------------------------- PUBLIC FUNCTIONS ------------------------ */
static gchar*
gnome_print_ps2_get_date (void)
{
	time_t clock;
	struct tm *now;
	gchar *date;

#ifdef ADD_TIMEZONE_STAMP
  extern char * tzname[];
	/* TODO : Add :
		 "[+-]"
		 "HH'" Offset from gmt in hours
		 "OO'" Offset from gmt in minutes
	   we need to use tz_time. but I don't
	   know how protable this is. Chema */
	gprint ("Timezone %s\n", tzname[0]);
	gprint ("Timezone *%s*%s*%li*\n", tzname[1], timezone);
#endif	

	debug (FALSE, "");

	clock = time (NULL);
	now = localtime (&clock);

	date = g_strdup_printf ("D:%04d%02d%02d%02d%02d%02d",
													now->tm_year + 1900,
													now->tm_mon + 1,
													now->tm_mday,
													now->tm_hour,
													now->tm_min,
													now->tm_sec);

	return date;
}

static gchar *
gnome_print_ps2_print_ctm (const double *ctm)
{
	return g_strdup_printf ("- CTM.[%g,%g,%g,%g]",
													ctm [0],
													ctm [1],
													ctm [2],
													ctm [3]);
													
}

GnomePrintPs2 *
gnome_print_ps2_new (GnomePrinter *printer)
{
	GnomePrintPs2 *ps2;
	GnomePrintContext *pc;
	gchar *date;
	gint ret = 0;

	g_return_val_if_fail (printer != NULL, NULL);
	
	ps2 = gtk_type_new (gnome_print_ps2_get_type ());

	if (!gnome_print_context_open_file (GNOME_PRINT_CONTEXT (ps2), printer->filename))
		goto failure;

	pc = GNOME_PRINT_CONTEXT (ps2);

	date = gnome_print_ps2_get_date ();

	ret = gnome_print_context_fprintf (pc,
																		 "%%!PS-Adobe-2.0\n"
																		 "%%%% Creator: Gnome Print Version %s\n"
																		 "%%%% DocumentName: %s\n"
																		 "%%%% Author: %s\n"
																		 "%%%% Pages: (atend)\n"
																		 "%%%% Date: %s\n"
																		 "%%%% driver : gnome-print-ps2\n"
																		 "%%%% EndComments\n\n\n",
																		 VERSION,
																		 "Document Name Goes Here",
																		 "Author Goes Here",
																		 date);

	g_free (date);
	
	if ( ret < 0)
		goto failure;

	ret = gnome_print_ps2_dictionary (pc);
	
	if ( ret < 0)
		goto failure;

	return ps2;

	failure:
	
	g_warning ("gnome_print_ps2_new: ps2 new failure ..\n");
	gtk_object_unref (GTK_OBJECT (ps2));
	return NULL;
}


GtkType
gnome_print_ps2_get_type (void)
{
  static GtkType ps2_type = 0;

  if (!ps2_type)
    {
      GtkTypeInfo ps2_info =
      {
				"GnomePrintps2",
				sizeof (GnomePrintPs2),
				sizeof (GnomePrintPs2Class),
				(GtkClassInitFunc)  gnome_print_ps2_class_init,
				(GtkObjectInitFunc) gnome_print_ps2_init,
				/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      ps2_type = gtk_type_unique (gnome_print_context_get_type (), &ps2_info);
    }

  return ps2_type;
}

/* -------------------------- END: PUBLIC FUNCTIONS ------------------------ */
static gint
gnome_print_ps2_error (gint fatal, const char *format, ...)
{
	va_list arguments;
	gchar *text;

	debug (FALSE, "");

	va_start (arguments, format);
	text = g_strdup_vprintf (format, arguments);
	va_end (arguments);
	
	g_warning ("Offending command [[%s]]", text);

	g_free (text);
	
	return -1;
}


static GnomePrintPs2GraphicState *
gnome_print_ps2_graphic_state_current (GnomePrintPs2 *ps2, gint dirtyfy)
{
	GnomePrintPs2GraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_PS2(ps2), NULL);

	gs = ps2->graphic_state;

	if (dirtyfy)
		gs->dirty = TRUE;
	
	return gs;
}
	
static gint
gnome_print_ps2_page_start (GnomePrintContext *pc)
{
	GnomePrintPs2Page *page;
	GnomePrintPs2 *ps2;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	g_return_val_if_fail (ps2->current_page == NULL, -1);
	
	page = g_new (GnomePrintPs2Page, 1);
	ps2->current_page = page;

	page->page_name = NULL;
	page->showpaged = FALSE;
	page->page_number = ps2->current_page_number++;

	return 0;
}

static gint
gnome_print_ps2_dictionary  (GnomePrintContext *pc)
{
	gint ret = 0;
	
	ret += gnome_print_context_fprintf (pc,
																			"/|/def load def/,/load load" EOL
																			"|/m/moveto , |/l/lineto , |/c/curveto , |/q/gsave ," EOL
																			"|/Q/grestore , |/rg/setrgbcolor , |/J/setlinecap ," EOL
																			"|/j/setlinejoin , |/w/setlinewidth , |/M/setmiterlimit ," EOL
																			"|/d/setdash , |/i/pop , |/W/clip , |/W*/eoclip , |/n/newpath ," EOL
																			"|/S/stroke , |/f/fill , |/f*/eofill , |/Tj/show , |/Tm/moveto ," EOL
																			"|/FF/findfont ," EOL
																			"|/h/closepath , |/cm/concat , |/rm/rmoveto , |/sp/strokepath ," EOL
																			"|/SP/showpage , |/p/pop , |/EX/exch , |/DF/definefont , |" EOL
																			/* Define the procedure for reencoding fonts */
																			"/RE {dup length dict begin {1 index /FID ne {def} {p p} ifelse}forall" EOL
																			"/Encoding ISOLatin1Encoding def currentdict end} |" EOL
																			"/F {scalefont setfont} def" EOL);
	return ret;
}
																			
static gint
gnome_print_ps2_page_end (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2Page *page;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2  != NULL, -1);
	g_return_val_if_fail (ps2->current_page != NULL, -1);

	page = ps2->current_page;
	
	ps2->pages = g_list_prepend (ps2->pages, page);
	ps2->current_page = NULL;
	ps2->graphic_state->writen = FALSE;

	ret += gnome_print_context_fprintf (pc,"SP" EOL);

	return ret;
}

static void
gnome_print_ps2_graphic_state_free (GnomePrintPs2GraphicState *gs)
{
	debug (FALSE, "");

	gp_path_unref (gs->current_path);
	g_free (gs);
}

static gboolean
gnome_print_dash_init (GnomePrintDash *dash, gint undefined)
{
	debug (FALSE, "");
	
	dash->number_values = undefined;
	dash->phase = undefined;
	dash->values = NULL;
	
	return TRUE;
}


static GnomePrintPs2GraphicState *
gnome_print_ps2_graphic_state_new (gint undefined)
{
	GnomePrintPs2GraphicState * state;
	gint def;

	debug (FALSE, "");

	if (undefined)
		def = 1;
	else
		def = 0;
	
	state = g_new (GnomePrintPs2GraphicState, 1);

	state->dirty   = TRUE;
	state->writen  = FALSE;

	state->current_path = gp_path_new();
	state->path_loaded = FALSE;

	/* CTM */
	art_affine_identity (state->ctm);
		
	/* Color stuff */
	if (undefined)
		state->color_mode = PS2_COLOR_MODE_UNDEFINED;
	else
		state->color_mode = PS2_COLOR_MODE_DEVICERGB;
	state->color [0] = def;
	state->color [1] = def;
	state->color [2] = def;
	state->color [3] = def;

	/* Line stuff */
	state->linejoin   = def;
	state->linecap    = def;
	if (undefined) {
		state->miterlimit = 9;
		state->linewidth  = 2;
	}	else {
		state->miterlimit = 10;
		state->linewidth  = 1;
	}
	if (undefined)
		gnome_print_dash_init (&state->dash, def);
	else
		gnome_print_dash_init (&state->dash, def);

	/* Font stuff */
  state->font_size = def;
	state->font_character_spacing = def;
	state->font_word_spacing = def;

  state->ps2_font_number = GNOME_PRINT_PS2_FONT_UNDEFINED;
	state->text_flag = FALSE;

	/* Text font stuff */
	state->text_font_handle = def;
	state->text_font_size = def;

	return state;
}

static gint
gnome_print_ps2_showpage (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	g_return_val_if_fail (ps2->current_page != NULL, -1);

	if (ps2->current_page->showpaged) {
		gnome_print_ps2_error (TRUE , "showpage, showpaged used twice for the same page");
		return 0;
	}
	
	if (ps2->gsave_level_number > 0)
		gnome_print_ps2_error (FALSE, "showpage, with graphic state stack NOT empty");

	ps2->current_page->showpaged = TRUE;

	gnome_print_ps2_page_end   (pc);

	gnome_print_ps2_graphic_state_free (ps2->graphic_state);
	gnome_print_ps2_graphic_state_free (ps2->graphic_state_set);
	ps2->graphic_state     = gnome_print_ps2_graphic_state_new (FALSE);
	ps2->graphic_state_set = gnome_print_ps2_graphic_state_new (TRUE);
	
	gnome_print_ps2_page_start (pc);

	return 0;
}

static gint
gnome_print_ps2_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
	GnomePrintPs2 *ps2;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (name_of_this_page != NULL, -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	g_return_val_if_fail (ps2->current_page != NULL, -1);
	g_return_val_if_fail (ps2->current_page->page_name == NULL, -1);
	
	ps2->current_page->page_name = g_strdup (name_of_this_page);

	return gnome_print_context_fprintf (pc,
                                      "%%%%Page: %s\n",
																			name_of_this_page);
}


static gint
gnome_print_ps2_close (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	if (ps2->gsave_level_number > 0)
		gnome_print_ps2_error (FALSE, "context close, with graphic state stack NOT empty. [Unbalanced gsaves&gretores]");

	ret += gnome_print_context_fprintf (pc,
																			"%c%cEOF" EOL,
																			'%',
																			'%');
	gnome_print_context_close_file (pc);
	
	return ret;
}

static gint
gnome_print_ps2_moveto (GnomePrintContext *pc, double x, double y)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	ArtPoint point;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_current (ps2, FALSE);

	point.x = x;
	point.y = y;

	art_affine_point (&point, &point, gs->ctm);	
	gp_path_moveto (gs->current_path, point.x, point.y);

	return 0;
}

static gint
gnome_print_ps2_setopacity (GnomePrintContext *pc, double opacity)
{
  static gboolean warned = FALSE;

	debug (FALSE, "");

	if (!warned) {
    g_warning ("Unimplemented setopacity");
    warned = TRUE;
  }
  return 0;
}

static gint
gnome_print_ps2_setrgbcolor (GnomePrintContext *pc,
														 double r, double g, double b)
{
  GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_current (ps2, TRUE);

	gs->color_mode = PS2_COLOR_MODE_DEVICERGB;
	gs->color [0]  = r;
	gs->color [1]  = g;
	gs->color [2]  = b;
	
	return 0;
}

static gint
gnome_print_ps2_setlinewidth (GnomePrintContext *pc, double width)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	if (width < 0)
		return gnome_print_ps2_error (FALSE, "setlinewidth, invalid parameter range %g", width);
	
	gs = gnome_print_ps2_graphic_state_current (ps2, TRUE);

#if 0	
	gs->linewidth = ((gs->ctm[0] + gs->ctm[3]) / 2) * linewidth;
#else
/* This is an ugly hack, but it should work ok for 90 deg.
	 angles */
	gs->linewidth = (fabs (width * gs->ctm[0]) +
									 fabs (width * gs->ctm[1]) +
									 fabs (width * gs->ctm[2]) +
									 fabs (width * gs->ctm[3])) / 2;
	
#endif	

	return 0;
}

static gint
gnome_print_ps2_setmiterlimit (GnomePrintContext *pc, double miterlimit)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	if (miterlimit < 1)
		return gnome_print_ps2_error (FALSE, "setmiterlimit, invalid parameter range %g", miterlimit);
	
	gs = gnome_print_ps2_graphic_state_current (ps2, TRUE);

	gs->miterlimit = miterlimit;

	return 0;
}

static gint
gnome_print_ps2_setlinecap (GnomePrintContext *pc, gint linecap)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	if (linecap < 0 || linecap > 2)
		return gnome_print_ps2_error (FALSE, "setlinecap, invalid parameter range %i", linecap);

	gs = gnome_print_ps2_graphic_state_current (ps2, TRUE);

	gs->linecap = linecap;
	
	return 0;
}

static gint
gnome_print_ps2_setlinejoin (GnomePrintContext *pc, gint linejoin)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	if (linejoin < 0 || linejoin > 2)
		return gnome_print_ps2_error (FALSE, "setlinejoin, invalid parameter range %i", linejoin);

	gs = gnome_print_ps2_graphic_state_current (ps2, TRUE);

	gs->linejoin  = linejoin;

	return 0;
}

static gboolean
gnome_print_dash_compare (GnomePrintDash *dash1, GnomePrintDash *dash2)
{
	gint n;
	
	if (dash1->number_values != dash2->number_values)
		return FALSE;
	
	if (dash1->phase != dash2->phase)
		return FALSE;

	for (n=0; n < dash1->number_values; n++)
		if (dash1->values[n] != dash2->values[n])
			return FALSE;

	return TRUE;
}

static void
gnome_print_dash_copy (GnomePrintDash *dash_from, GnomePrintDash *dash_to)
{
	dash_to->number_values = dash_from->number_values;
	dash_to->phase         = dash_from->phase;

	if (dash_to->values == NULL)
		dash_to->values = g_new (gdouble, dash_from->number_values);
	
	memcpy (dash_to->values,
					dash_from->values,
					dash_to->number_values * sizeof (gdouble));
}

static gint
gnome_print_ps2_setdash (GnomePrintContext *pc, gint number_values, const double *values, double offset)
{
	GnomePrintPs2 *ps2;
	GnomePrintDash *dash;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	dash = &ps2->graphic_state->dash;

	g_free (dash->values);

	dash->phase = offset;
	dash->values = g_new (gdouble, number_values);
	dash->number_values = number_values;

	memcpy (dash->values, values,
					number_values * sizeof (gdouble));

	return 0;
}

static gint
gnome_print_ps2_graphic_state_set_color (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	GnomePrintPs2GraphicState *gs_set;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	
	gs     = ps2->graphic_state;
	gs_set = ps2->graphic_state_set;

	if (gs->color_mode != gs_set->color_mode ||
	 (gs->color [0] != gs_set->color [0] ||
		gs->color [1] != gs_set->color [1] ||
		gs->color [2] != gs_set->color [2] ||
		gs->color [3] != gs_set->color [3] ))
		switch (gs->color_mode){
		case PS2_COLOR_MODE_DEVICEGRAY:
			g_warning ("Implement color mode devicegray !!!\n");
#if 0	
			gnome_print_context_fprintf (pc, "%.3g G" EOL,
																	 gs->color [0]);
#endif	
			break;
		case PS2_COLOR_MODE_DEVICERGB:
			gnome_print_context_fprintf (pc, "%.3g %.3g %.3g rg" EOL,
																	 gs->color [0],
																	 gs->color [1],
																	 gs->color [2]);
			break;
		case PS2_COLOR_MODE_DEVICECMYK:
			g_warning ("Implement CMYK color mode \n");
#if 0	
			gnome_print_context_fprintf (pc, "%.3g %.3g %.3g %.3g K" EOL,
																	 gs->color [0],
																	 gs->color [1],
																	 gs->color [2],
																	 gs->color [3]);
#endif	
			break;
		}
	gs_set->color_mode = gs->color_mode;
	gs_set->color [0] = gs->color [0];
	gs_set->color [1] = gs->color [1];
	gs_set->color [2] = gs->color [2];
	gs_set->color [3] = gs->color [3];

	return 0;
}

static GnomePrintPs2GraphicState *
gnome_print_ps2_graphic_state_text_set (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	GnomePrintPs2GraphicState *gs_set;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), NULL);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, NULL);
	
	gs     = ps2->graphic_state;
	gs_set = ps2->graphic_state_set;

#ifdef FIX_ME_FIX_ME_FIX_ME
	if (!gs->dirty)
		return 0;
#endif

	gnome_print_ps2_graphic_state_set_color (pc);
	
	return gs;
}

#define gpcf gnome_print_context_fprintf
static gboolean
gnome_print_ps2_reencode_font (GnomePrintContext *pc, GnomePrintPs2Font *ps2font)
{
	GnomeFont * font;
	GnomeFontFace * face;
	gint nglyphs, nfonts;
	gint i, j;

	debug (FALSE, "");
	
	/* Bitch 'o' bitches (Lauris) ! */

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), FALSE);
	font = ps2font->gnome_font;
	g_return_val_if_fail (GNOME_IS_FONT (font), FALSE);
	face = (GnomeFontFace *) gnome_font_get_face (font);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), FALSE);
		
	nglyphs = gnome_font_face_get_num_glyphs (face);
	nfonts = (nglyphs + 255) >> 8;

	gpcf (pc, "32 dict begin" EOL);

	/* Common entries */

	gpcf (pc, "/FontType 0 def" EOL);
	gpcf (pc, "/FontMatrix [1 0 0 1 0 0] def" EOL);
	gpcf (pc, "/FontName /%s-Unicode-Composite def" EOL, gnome_font_face_get_ps_name (face));
	gpcf (pc, "/LanguageLevel 2 def" EOL);

	/* Type 0 entries */

	gpcf (pc, "/FMapType 2 def" EOL);

	/* Bitch 'o' bitches */

	gpcf (pc, "/FDepVector [" EOL);

	for (i = 0; i < nfonts; i++) {

		gpcf (pc, "/%s FF" EOL, gnome_font_face_get_ps_name (face));
		gpcf (pc, "dup length dict begin {1 index /FID ne {def} {pop pop} ifelse} forall" EOL);
		gpcf (pc, "/Encoding [" EOL);

		for (j = 0; j < 256; j++) {
			gint glyph;

			glyph = 256 * i + j;
			if (glyph >= nglyphs)
				glyph = 0;

			gpcf (pc, ((j & 0x0f) == 0x0f) ? "/%s" EOL : "/%s ", gnome_font_face_get_glyph_ps_name (face, glyph));
		}

		gpcf (pc, "] def" EOL);
		gpcf (pc, "currentdict end /%s-Unicode-Page-%d exch definefont" EOL,
					gnome_font_face_get_ps_name (face), i);
	}

	gpcf (pc, "] def" EOL);

	gpcf (pc, "/Encoding [" EOL);

	for (i = 0; i < 256; i++) {
		gint fn;

		fn = (i < nfonts) ? i : 0;

		gpcf (pc, ((i & 0x0f) == 0x0f) ? "%d" EOL : "%d  ", fn);
	}

	gpcf (pc, "] def" EOL);

	gpcf (pc, "currentdict end" EOL);
	gpcf (pc, "/Uni-%s EX DF p" EOL, gnome_font_face_get_ps_name (face));

	ps2font->reencoded = TRUE;

	return TRUE;
}

#undef gpcf

static void
gnome_print_ps2_download_font (GnomePrintContext *pc, GnomePrintPs2Font *font)
{
	gchar *pfa;

	debug (FALSE, "");
		
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (pc));
	g_return_if_fail (font != NULL);
	g_return_if_fail (font->reencoded == FALSE);
	g_return_if_fail (font->external == TRUE);

	pfa = gnome_font_get_pfa (font->gnome_font);

	if (pfa == NULL) {
		g_warning ("Could not get pfa");
		return;
	}

	gnome_print_context_fprintf (pc, "%s", pfa);
}

static gint
gnome_print_ps2_graphic_state_setfont (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	GnomePrintPs2GraphicState *gs_set;
	GnomePrintPs2Font *ps2_font;
	GnomePrintPs2Font *ps2_font_set;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs     = ps2->graphic_state;
	gs_set = ps2->graphic_state_set;

	g_return_val_if_fail (ps2->fonts[gs->ps2_font_number].font_name!= NULL, -1);
	g_return_val_if_fail (ps2->graphic_state->ps2_font_number !=
												GNOME_PRINT_PS2_FONT_UNDEFINED, -1);

#if 0	
	g_print ("Font number in state %i, font number set %i\n",
					 gs->ps2_font_number, gs_set->ps2_font_number);
#endif	
					 
	ps2_font     = &ps2->fonts [gs->ps2_font_number];
	ps2_font_set = &ps2->fonts [gs_set->ps2_font_number];

#if 0	
	g_print ("Address of font is %i,%i (num:%i)\n",
					 GPOINTER_TO_INT (ps2_font),
					 GPOINTER_TO_INT (ps2_font->gnome_font),
					 gs->ps2_font_number);
#endif	
					 
	g_return_val_if_fail (GNOME_IS_FONT (ps2_font->gnome_font),-1);
	
	if ((ps2_font == ps2_font_set) &&
			(gs->font_size == gs_set->font_size))
		return ret;
	
	/* we use the reencoded flag to know if the font has been
		 downloaded or not */
	if (ps2_font->external && !ps2_font->reencoded)
		gnome_print_ps2_download_font (pc, ps2_font);
	if (!ps2_font->reencoded)
		gnome_print_ps2_reencode_font (pc, ps2_font);
	
	ret += gnome_print_context_fprintf (pc,
																			"/Uni-%s FF %g F" EOL,
																			ps2->fonts[gs->ps2_font_number].font_name,
																			gs->font_size);
	gs_set->font_size = gs->font_size;
	gs_set->ps2_font_number = gs->ps2_font_number;

	return ret;
}

static GnomePrintPs2GraphicState *
gnome_print_ps2_graphic_state_set (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	GnomePrintPs2GraphicState *gs_set;
	gint changed = FALSE;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), NULL);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL,  NULL);

	gs     = ps2->graphic_state;
	gs_set = ps2->graphic_state_set;

	if (!gs->dirty)
		return 0;

	gnome_print_ps2_graphic_state_set_color (pc);
	
	/* Linecap */
	if (gs->linecap != gs_set->linecap || !gs->writen) {
		gnome_print_context_fprintf (pc, "%i J ", gs->linecap);
		gs_set->linecap    = gs->linecap;
		changed = TRUE;
	}

	/* Line join */
	if (gs->linejoin != gs_set->linejoin || !gs->writen)	{
		gnome_print_context_fprintf (pc, "%i j ", gs->linejoin);
		gs_set->linejoin = gs->linejoin;
		changed = TRUE;
	}

	/* Line width */
	if (gs->linewidth != gs_set->linewidth || !gs->writen) {
		gnome_print_context_fprintf (pc, "%g w ", gs->linewidth);
		gs_set->linewidth  = gs->linewidth;
		changed = TRUE;
	}
	
	/* Miterlimit */
	if (gs->miterlimit != gs_set->miterlimit || !gs->writen)	{
		gnome_print_context_fprintf (pc, "%g M ", gs->miterlimit);
		gs_set->miterlimit = gs->miterlimit;
		changed = TRUE;
	}

	/* Dash */
	if (!gnome_print_dash_compare (&gs->dash, &gs_set->dash) || !gs->writen)	{
		gint n;
		gnome_print_context_fprintf (pc, "[");
		for (n = 0; n < gs->dash.number_values; n++)
			gnome_print_context_fprintf (pc, " %g", gs->dash.values[n]);
		gnome_print_context_fprintf (pc, "]%g d", gs->dash.phase);
		gnome_print_dash_copy (&gs->dash, &gs_set->dash);
		changed = TRUE;
	}

	if (changed) 
		gnome_print_context_fprintf (pc, EOL);

	if (!gs->writen)
		gnome_print_context_fprintf (pc, "1 i " EOL);
	
	gs->writen = TRUE;

	return gs;
}

#ifdef NOT_USED_KILL_COMPILE_WARNING
static gboolean
path_is_rectangle (ArtBpath *path_incoming)
{
	ArtBpath *path;
	gint n = 0;

	path = path_incoming;
	if (path->code != ART_MOVETO &&
			path->code != ART_MOVETO_OPEN)
		return FALSE;

	path++;
	
	for (;path->code != ART_END; path++) {
		if (path->code != ART_LINETO)
			return FALSE;
		n++;
		if (n==5)
			return FALSE;
	}

	if (n!=4)
		return FALSE;
	
	path = path_incoming;

	if ( ( ((path+1)->x3 == (path+4)->x3) &&
				 ((path+2)->x3 == (path+3)->x3) &&
				 ((path+1)->y3 == (path+2)->y3) &&
				 ((path+3)->y3 == (path+4)->y3) ) ||
			 ( ((path+1)->y3 == (path+4)->y3) &&
				 ((path+2)->y3 == (path+3)->y3) &&
				 ((path+1)->x3 == (path+2)->x3) &&
				 ((path+3)->x3 == (path+4)->x3) ))
		/*
		return TRUE;
		*/;

	return FALSE;
}
#endif
	
	
static gint
gnome_print_ps2_path_print (GnomePrintContext *pc, GPPath *path_incoming)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	ArtBpath *path;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	
	gs        = gnome_print_ps2_graphic_state_current (ps2, FALSE);

	/*
	if (path_is_rectangle (path)) {
			gnome_print_context_fprintf (pc, "%g %g %g %g rectangle" EOL,
																	 path->x3,
																	 path->y3);
	}
	*/

	path = gp_path_bpath (path_incoming);
	
	for (; path->code != ART_END; path++)
		switch (path->code) {
		case ART_MOVETO_OPEN:
			gnome_print_context_fprintf (pc, "%g %g m" EOL, path->x3, path->y3);
			break;
		case ART_MOVETO:
			gnome_print_context_fprintf (pc, "%g %g m" EOL, path->x3, path->y3);
			break;
		case ART_LINETO:
			gnome_print_context_fprintf (pc, "%g %g l" EOL, path->x3, path->y3);
			break;
		case ART_CURVETO:
			gnome_print_context_fprintf (pc, "%g %g %g %g %g %g c" EOL,
																	 path->x1, path->y1,
																	 path->x2, path->y2,
																	 path->x3, path->y3);
			break;
		default:
			gnome_print_ps2_error (TRUE, "the path contains an unknown type point");
			return -1;
		}
	
	gp_path_reset (path_incoming);
	
	return 0;
}

#if 0
static gint
gnome_print_ps2_path_dump (GnomePrintContext *pc, GPPath *path_incoming)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	ArtBpath *real_path, *free_me, *temp;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	
	gs        = gnome_print_ps2_graphic_state_current (ps2, FALSE);
	temp      = gp_path_bpath (path_incoming);
	real_path = art_bpath_affine_transform (gp_path_bpath (path_incoming), gs->ctm);
	free_me   = real_path;
	
	for (; real_path->code != ART_END; real_path++)
		switch (real_path->code) {
		case ART_MOVETO_OPEN:
			g_print ("%g %g m" EOL, real_path->x3, real_path->y3);
			break;
		case ART_MOVETO:
			g_print ("%g %g m" EOL, real_path->x3, real_path->y3);
			break;
		case ART_LINETO:
			g_print ("%g %g l" EOL, real_path->x3, real_path->y3);
			break;
		case ART_CURVETO:
			g_print ("%g %g %g %g %g %g c" EOL,
							 real_path->x1, real_path->y1,
							 real_path->x2, real_path->y2,
							 real_path->x3, real_path->y3);
			break;
		default:
			gnome_print_ps2_error (TRUE, "the path contains an unknown type point");
			return -1;
		}
	
	art_free (free_me);
	
	return 0;
}
#endif

#ifdef NOT_USED_KILL_WARNING
static gboolean
gnome_print_ps2_ctm_compare (double ctm1[6], double ctm2[6])
{
	return (ctm1 [0] == ctm2 [0] &&
					ctm1 [1] == ctm2 [1] &&
					ctm1 [2] == ctm2 [2] &&
					ctm1 [3] == ctm2 [3] &&
					ctm1 [4] == ctm2 [4] &&
					ctm1 [5] == ctm2 [5]);
}
#endif

static GnomePrintPs2GraphicState *
gnome_print_ps2_graphic_state_duplicate (GnomePrintPs2GraphicState *gs_in, GnomePrintPs2 *ps2)
{
	GnomePrintPs2GraphicState *gs_out;

	debug (FALSE, "");
	
	gs_out = g_new (GnomePrintPs2GraphicState, 1);
	
	memcpy (gs_out,
					gs_in,
					sizeof (GnomePrintPs2GraphicState));

	gs_out->current_path = gp_path_duplicate (gs_in->current_path);

#if 0	
	g_print ("Duplicate, CTM_in :%i [%s] CTM_out:%i %s\n",
					 GPOINTER_TO_INT (gs_in->ctm),
					 gnome_print_ps2_print_ctm (gs_in->ctm),
					 GPOINTER_TO_INT (gs_out->ctm),
					 gnome_print_ps2_print_ctm (gs_out->ctm));
#endif	

	return gs_out;
}


static gint
gnome_print_ps2_gsave (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs_push_me;
	GnomePrintPs2GraphicState *gs_push_me_set;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gnome_print_ps2_graphic_state_set (pc);
	
	gs_push_me     = gnome_print_ps2_graphic_state_duplicate (ps2->graphic_state, ps2);
	gs_push_me_set = gnome_print_ps2_graphic_state_duplicate (ps2->graphic_state_set, ps2);

	ps2->gsave_stack [ps2->gsave_level_number].graphic_state     = gs_push_me;
	ps2->gsave_stack [ps2->gsave_level_number].graphic_state_set = gs_push_me_set;

	ps2->gsave_level_number++;

	if (ps2->gsave_level_number == ps2->gsave_level_max) {
		ps2->gsave_stack = g_realloc (ps2->gsave_stack, sizeof (GnomePrintPs2Gsave) *
																	(ps2->gsave_level_max += GNOME_PRINT_PS2_NUMBER_OF_ELEMENTS_GROW));
	}
			
	ret += gnome_print_context_fprintf (pc, "q" EOL);

	return ret;
}

	

static gint
gnome_print_ps2_grestore (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	ps2->gsave_level_number--;

	if (ps2->gsave_level_number < 0) {
		gnome_print_ps2_error (TRUE, "grestore, graphic state stack empty");
		return 0;
	}

	gnome_print_ps2_graphic_state_free (ps2->graphic_state_set);
	gnome_print_ps2_graphic_state_free (ps2->graphic_state);
	ps2->graphic_state_set = ps2->gsave_stack [ps2->gsave_level_number].graphic_state_set;
	ps2->graphic_state     = ps2->gsave_stack [ps2->gsave_level_number].graphic_state;

	ret += gnome_print_context_fprintf (pc, "Q" EOL);

	return ret;
}


static gint
gnome_print_ps2_ctm_is_identity (double matrix [6])
{
	double identity [6];

	art_affine_identity (identity);

	if ( (identity [0] != matrix [0]) ||
			 (identity [1] != matrix [1]) ||
			 (identity [2] != matrix [2]) ||
			 (identity [3] != matrix [3]) ||
			 (identity [4] != matrix [4]) ||
			 (identity [5] != matrix [5]))
		return FALSE;
	
	return TRUE;
}

static gint
gnome_print_ps2_show_matrix_set (GnomePrintPs2 *ps2)
{
	GnomePrintContext *pc;
	GnomePrintPs2GraphicState *gs;
	GnomePrintPs2GraphicState *gs_set;
	gint ret = 0;

	pc = GNOME_PRINT_CONTEXT (ps2);

	gs     = ps2->graphic_state;
	gs_set = ps2->graphic_state_set;

#if 0	/* I am not sure about this ... */
	if (gnome_print_ps2_ctm_is_identity (gs->ctm))
		return 0;
#endif	
	
	ret += gnome_print_ps2_gsave (pc);
	ret += gnome_print_context_fprintf (pc,
                                      "[ %g %g %g %g %g %g ] cm" EOL,
                                      gs->ctm [0],
                                      gs->ctm [1],
                                      gs->ctm [2],
                                      gs->ctm [3],
                                      gs->ctm [4],
                                      gs->ctm [5]);

	gs->text_flag = TRUE;

	return ret;
}

static gint
gnome_print_ps2_show_matrix_restore (GnomePrintPs2 *ps2)
{
	GnomePrintContext *pc;
	gint ret = 0;

	if (!ps2->graphic_state->text_flag)
		return 0;

	ps2->graphic_state->text_flag = FALSE;
		
	pc = GNOME_PRINT_CONTEXT (ps2);

	gnome_print_ps2_grestore (pc);

	return ret;
}

static gint
gnome_print_ps2_get_font_number (GnomePrintContext *pc, GnomeFont *gnome_font,
																 gboolean external)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2Font *font;
	gint n;
	gchar *font_name;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (GNOME_IS_FONT (gnome_font), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	
	font_name = g_strdup (gnome_font_get_ps_name (gnome_font));

	for (n = 0; n < ps2->fonts_number; n++)
		if (!strcmp (font_name, ps2->fonts[n].font_name))
			break;

	if (n != ps2->fonts_number) {
		g_free (font_name);
		return n;
	}

	if (ps2->fonts_number == ps2->fonts_max) {
		ps2->fonts = g_realloc (ps2->fonts, sizeof (GnomePrintPs2Font) *
														(ps2->fonts_max += GNOME_PRINT_PS2_NUMBER_OF_ELEMENTS_GROW));
	}

	font = &ps2->fonts[ps2->fonts_number++];
	font->font_number   = ps2->fonts_number*2;
	gnome_font_ref (gnome_font);
	font->gnome_font    = gnome_font;
	
	font->font_name     = font_name;
	font->reencoded     = FALSE;
	font->external      = external;

	return ps2->fonts_number-1;
}

static gint
gnome_print_ps2_setfont_raw (GnomePrintContext *pc, GnomeFontFace *face,
														 double size)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
  const char *fontname;
	gint n;

	debug (FALSE, "");

	g_warning ("Returnin from set font raw \n");
	return 0;

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
  ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

  if (face == NULL)
    return -1;

  fontname = gnome_font_face_get_ps_name (face);

  for (n = 0; n < ps2->fonts_internal_number; n++)
    if (!strcmp (fontname, ps2->fonts_internal[n]))
      break;

	if (n == ps2->fonts_internal_number) {
		static gint warned = FALSE;
		if (!warned) {
			g_warning ("External fonts not implemented. (Yet !)");
			warned = TRUE;
		}
		return 0;
	}

	gs = ps2->graphic_state;
	gs->font_size = size;
	gs->ps2_font_number = GNOME_PRINT_PS2_FONT_UNDEFINED;

	/* Invalidate the current settings for gnome-text */
	gs->text_font_handle = 0;
	gs->text_font_size = 0;
	
	return 0;
}

static gint
gnome_print_ps2_setfont (GnomePrintContext *pc, GnomeFont *font)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
  const char *fontname;
	gint n;
	gboolean external = FALSE;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
  ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT (font), -1);

  fontname = gnome_font_get_ps_name (font);

#if 0	
	g_print ("Setting font [%s,%i]\n", fontname, GPOINTER_TO_INT (font));
#endif	

  for (n = 0; n < ps2->fonts_internal_number; n++)
    if (!strcmp (fontname, ps2->fonts_internal[n]))
      break;

	if (n == ps2->fonts_internal_number) {
    fontname = gnome_font_get_glyph_name (font);
		for (n = 0; n < ps2->fonts_external_number; n++)
			if (!strcmp (fontname, ps2->fonts_external[n]))
				break;

		if (n == ps2->fonts_external_number)
		{
			gchar *pfa = NULL;
			/* We need to make sure the PFA will be available */
			pfa = gnome_font_get_pfa (font);
			if (pfa == NULL) {
				g_warning ("Could not get the PFA for font %s\n", fontname);
				return -1;
			}
			g_free (pfa);
		}
		external = TRUE;
	}

	gs = ps2->graphic_state;
	gs->font_size = gnome_font_get_size (font);
	gs->ps2_font_number = gnome_print_ps2_get_font_number (pc, font, external);

	/* Invalidate the current settings for gnome-text */
	gs->text_font_handle = 0;
	gs->text_font_size = 0;
	
	return 0;
}

static gint
gnome_print_ps2_show_sized (GnomePrintContext *pc, const char *text, int bytes)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	ArtPoint point;
	gint ret = 0;
	const char *p;
	const GnomeFontFace * face;

	debug (FALSE, "");

	ps2 = (GnomePrintPs2 *) pc;

	g_return_val_if_fail (GNOME_IS_PRINT_PS2 (ps2), -1);
	g_return_val_if_fail (ps2->fonts != NULL, -1);

	gs = ps2->graphic_state;
	
	if (!gp_path_has_currentpoint (gs->current_path)) {
		gnome_print_ps2_error (FALSE, "show, currentpoint not defined.");
		return -1;
	}

	if (gs->ps2_font_number == GNOME_PRINT_PS2_FONT_UNDEFINED ||
			gs->font_size == 0) {
		gnome_print_ps2_error (FALSE, "show, fontname or fontsize not defined.");
		return -1;
	}
	
	gp_path_currentpoint (gs->current_path, &point);

	ret += gnome_print_ps2_graphic_state_setfont (pc);

	g_return_val_if_fail (GNOME_IS_FONT (ps2->fonts[gs->ps2_font_number].gnome_font), -1);

	face = gnome_font_get_face (ps2->fonts[gs->ps2_font_number].gnome_font);

	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);

	ret += gnome_print_context_fprintf (pc,
																			"%g %g Tm" EOL,
																			point.x,
																			point.y);

	gnome_print_ps2_graphic_state_text_set (pc);
	gnome_print_ps2_show_matrix_set (ps2);

	/* I don't like the fact that we are writing one letter at time */
  if (gnome_print_context_fprintf (pc, "(") < 0)
    return -1;

  for (p = text; p && p < (text + bytes); p = g_utf8_next_char (p))
	{
		gunichar u;
		gint g, glyph, page;

    u = g_utf8_get_char (p);
		g = gnome_font_face_lookup_default (face, u);
		glyph = g & 0xff;
		page = (g >> 8) & 0xff;

		if (gnome_print_context_fprintf (pc,"\\%03o\\%03o", page, glyph) < 0)
			return -1;
	}

	ret += gnome_print_context_fprintf (pc, ") Tj" EOL);
	ret += gnome_print_ps2_show_matrix_restore (ps2);

  return ret;
}

static gint
gnome_print_ps2_glyphlist (GnomePrintContext *pc, GnomeGlyphList *gl)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	GnomePosGlyphList *pgl;
	gdouble affine[6];
	gdouble ictm[6];
	ArtPoint p;
	GSList * l;

	ps2 = (GnomePrintPs2 *) pc;

	gs = ps2->graphic_state;

	if (!gp_path_has_currentpoint (gs->current_path)) {
		gnome_print_ps2_error (FALSE, "glyphlist, currentpoint not defined.");
		return -1;
	}

	gnome_print_ps2_graphic_state_text_set (pc);
	gnome_print_ps2_show_matrix_set (ps2);

	gp_path_currentpoint (gs->current_path, &p);
	art_affine_invert (ictm, gs->ctm);
	art_affine_point (&p, &p, ictm);

	art_affine_identity (affine);
	affine[4] = p.x;
	affine[5] = p.y;

	pgl = gnome_pgl_from_gl (gl, affine, GNOME_PGL_RENDER_DEFAULT);

	for (l = pgl->strings; l != NULL; l = l->next) {
		GnomePosString * string;
		GnomeFont * font;
		gint i;

		string = (GnomePosString *) l->data;

		if (string->length < 1) continue;

		font = (GnomeFont *) gnome_rfont_get_font (string->rfont);

		gnome_print_ps2_setfont (pc, font);
		gnome_print_ps2_graphic_state_setfont (pc);

		/* Moveto */
		if (gnome_print_context_fprintf (pc, "%g %g Tm ", string->glyphs->x, string->glyphs->y) < 0) return -1;

		/* Build string */
		if (gnome_print_context_fprintf (pc, "(") < 0) return -1;
		for (i = 0; i < string->length; i++) {
			gint glyph, page;
			glyph = string->glyphs[i].glyph & 0xff;
			page = (string->glyphs[i].glyph >> 8) & 0xff;
			if (gnome_print_context_fprintf (pc, "\\%o\\%o", page, glyph) < 0) return -1;
		}
		if (gnome_print_context_fprintf (pc, ") ") < 0) return -1;

		/* Build array */
		if (gnome_print_context_fprintf (pc, "[") < 0) return -1;
		for (i = 1; i < string->length; i++) {
			if (gnome_print_context_fprintf (pc, "%g %g ",
																			 string->glyphs[i].x - string->glyphs[i-1].x,
																			 string->glyphs[i].y - string->glyphs[i-1].y) < 0) return -1;
		}
		if (gnome_print_context_fprintf (pc, "0 0 ] ") < 0) return -1;

		/* xyshow */
		if (gnome_print_context_fprintf (pc, "xyshow" EOL) < 0) return -1;
	}

	gnome_print_ps2_show_matrix_restore (ps2);

	gnome_pgl_destroy (pgl);

	return 1;
}

static gint
gnome_print_ps2_clip (GnomePrintContext *pc, ArtWindRule rule)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	gint ret = 0;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_set (pc);

#ifdef FIND_A_WAY_TO_CHECK_FOR_THIS		
	if (gp_path_length (gs->current_path) < 2) {
		gnome_print_ps2_error (FALSE, "Trying to clip with an empty path. (length:%i)",
													 gp_path_length (gs->current_path));
		gp_path_reset (gs->current_path);
		return -1;
	}
#endif	

	ret += gnome_print_ps2_path_print  (pc, gs->current_path);
	if (rule == ART_WIND_RULE_NONZERO) {
		ret += gnome_print_context_fprintf (pc, "W n" EOL);
	} else {
		ret += gnome_print_context_fprintf (pc, "W n" EOL);
	}

	gp_path_reset (gs->current_path);
		
  return ret;
}


static gint
gnome_print_ps2_stroke (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	gint ret = 0;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_set (pc);

#ifdef FIND_A_WAY_TO_CHECK_FOR_THIS		
	if (gp_path_length (gs->current_path) < 2) {
		gnome_print_ps2_error (FALSE, "Trying to stroke an empty path");
#if 0	
		gp_path_reset (gs->current_path);
		return -1;
#endif	
	}
#endif
	
	ret += gnome_print_ps2_path_print  (pc, gs->current_path);
	ret += gnome_print_context_fprintf (pc, "S" EOL);

  return ret;
}

static gint
gnome_print_ps2_fill (GnomePrintContext *pc, ArtWindRule rule)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_set (pc);

#ifdef FIND_A_WAY_TO_CHECK_FOR_THIS		
	if (gp_path_length (gs->current_path) < 2) {
		gnome_print_ps2_error (FALSE, "Trying to fill an empty path");
#if 0	
		gp_path_reset (gs->current_path);
		return -1;
#endif	
	}
#endif
	
	ret += gnome_print_ps2_path_print  (pc, gs->current_path);
	if (rule == ART_WIND_RULE_NONZERO) {
		ret += gnome_print_context_fprintf (pc, "f" EOL);
	} else {
		ret += gnome_print_context_fprintf (pc, "f*" EOL);
	}

  return ret;
}

static gint
gnome_print_ps2_lineto (GnomePrintContext *pc, double x, double y)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	ArtPoint point;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_current (ps2, FALSE);

	point.x = x;
	point.y = y;
	art_affine_point (&point, &point, gs->ctm);

	gp_path_lineto (gs->current_path, point.x, point.y);

	return 0;
}

static gint
gnome_print_ps2_curveto (GnomePrintContext *pc,
												 double x0, double y0,
												 double x1, double y1,
												 double x2, double y2)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	ArtPoint point_0;
	ArtPoint point_1;
	ArtPoint point_2;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_current (ps2, FALSE);

	/* IS there a libart function for this ??? Chema */
	point_0.x = x0;
	point_0.y = y0;
	point_1.x = x1;
	point_1.y = y1;
	point_2.x = x2;
	point_2.y = y2;

	art_affine_point (&point_0, &point_0, gs->ctm);
	art_affine_point (&point_1, &point_1, gs->ctm);
	art_affine_point (&point_2, &point_2, gs->ctm);

	gp_path_curveto (gs->current_path,
									 point_0.x, point_0.y,
									 point_1.x, point_1.y,
									 point_2.x, point_2.y); 
	
	return 0;
}

static gint
gnome_print_ps2_newpath (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_current (ps2, FALSE);

	if (gp_path_length (gs->current_path) > 1) 
		g_warning ("Path was disposed without using it [newpath]\n");
	
	gp_path_reset (gs->current_path);

  return 0;
}

static gint
gnome_print_ps2_strokepath (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_set (pc);

#ifdef FIND_A_WAY_TO_CHECK_FOR_THIS		
	if (gp_path_length (gs->current_path) < 2) {
		gnome_print_ps2_error (FALSE, "Trying to fill an empty path");
#if 0	
		gp_path_reset (gs->current_path);
		return -1;
#endif	
	}
#endif
	
	ret += gnome_print_ps2_path_print  (pc, gs->current_path);
	
	ret += gnome_print_context_fprintf (pc, "sp" EOL);

  return ret;
}

#if 0
static gint
gnome_print_ps2_textline2 (GnomePrintContext *pc, GnomeTextLine *line)
{
	static gint warned = FALSE;

	debug (FALSE, "");

	if (warned)
		return 0;

	g_warning ("ps2_textline not supported.\n");

	warned = TRUE;
	
	return 0;
}
#endif

static int
gnome_print_ps2_textline (GnomePrintContext *pc, GnomeTextLine *line)
{
	/* new */
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;

	GnomeTextFontHandle font_handle, font_handle_last;
	gint font_size, font_size_last;

	gint current_x_scale;
	gboolean open;

	ArtPoint *point;

	gint ret = 0;
	/* new */


	int i;
  int attr_idx;
  double scale_factor;
  GnomeTextGlyphAttrEl *attrs = line->attrs;
  int x;
  int glyph;

	/* new */
	debug (FALSE, "");

	g_warning ("using textline\n");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = ps2->graphic_state;

	if (!gp_path_has_currentpoint (gs->current_path)) {
		gnome_print_ps2_error (FALSE, "textline, currentpoint not defined.");
		return -1;
	}

	/* Set the initial point */
	point = art_new (ArtPoint, 1);
	point = gp_path_currentpoint (gs->current_path, point);
  art_affine_point (point, point, gs->ctm);
	/* CRASH CRASH CRASH CRASH 
	ret += gnome_print_ps2_graphic_state_setfont (pc);
	*/
		
	ret += gnome_print_context_fprintf (pc,
																			"%g %g Tm" EOL,
																			point->x,
																			point->y);
	/* new */

	font_handle = gs->text_font_handle;
	font_handle_last = font_handle;
	font_size   = gs->text_font_size;
	font_size_last = font_size;

	current_x_scale = 1000;
  scale_factor = font_size * current_x_scale * 1e-9 * GNOME_TEXT_SCALE;

  open = 0;
  x = 0;
  attr_idx = 0;
  for (i = 0; i < line->n_glyphs; i++) {
		while (attrs[attr_idx].glyph_pos == i) {
			switch (attrs[attr_idx].attr) {
	    case GNOME_TEXT_GLYPH_FONT:
	      font_handle = attrs[attr_idx].attr_val;
	      break;
	    case GNOME_TEXT_GLYPH_SIZE:
	      font_size = attrs[attr_idx].attr_val;
	      scale_factor = font_size * current_x_scale * 1e-9 * GNOME_TEXT_SCALE;
	      break;
	    default:
	      break;
	    }
			attr_idx++;
		}

		if (font_size != font_size_last ||
				font_handle != font_handle_last)
		{
#ifdef VERBOSE
			g_print ("cur_size = %d, expands to %g\n",
							 cur_size, ps->current_font_size);
#endif
			if (open)
				gnome_print_context_fprintf (pc, ") Tj" EOL);
			gnome_print_ps2_setfont_raw (pc, gnome_text_get_font (font_handle),
																	 font_size * 0.001);
			open = 0;
			font_size_last = font_size;
			font_handle_last = font_handle;
		}
#ifdef VERBOSE
		g_print ("x = %d, glyph x = %d\n",
						 x, line->glyphs[i].x);
#endif
		if (abs (line->glyphs[i].x - x) > 1)
		{
			gnome_print_context_fprintf (pc, "%s%g 0 rm" EOL,
																	 open ? ") Tj " : "",
																	 ((line->glyphs[i].x - x) * 1.0 /
																		GNOME_TEXT_SCALE));
			open = 0;
			x = line->glyphs[i].x;
		}
		glyph = line->glyphs[i].glyph_num;
		if (!open)
			gnome_print_context_fprintf (pc, "(");
		if (glyph >= ' ' && glyph < 0x7f)
			if (glyph == '(' || glyph == ')' || glyph == '\\')
				gnome_print_context_fprintf (pc, "\\%c", glyph);
			else
				gnome_print_context_fprintf (pc, "%c", glyph);
		else
			gnome_print_context_fprintf (pc, "\\%03o", glyph);
		open = 1;
		x += floor (gnome_text_get_width (font_handle, glyph) * scale_factor + 0.5);
	}

	if (open)
    gnome_print_context_fprintf (pc, ") Tj" EOL);

	gs->text_font_handle = font_handle;
  gs->text_font_size   = font_size;

	return 0;
}

#define G_P_VERY_SMALL (double) 1e-9
static void
gnome_print_ps2_matrix_clean (double matrix[6])
{
	gint n;

	for ( n=0; n < 6; n++)
	{
		double evaluate;
		evaluate = matrix[n];
		if (evaluate < 0)
			evaluate = evaluate * -1;
		if ( evaluate < G_P_VERY_SMALL)
			matrix [n] = 0;
	}
	
}


static gint
gnome_print_ps2_concat (GnomePrintContext *pc, const double matrix[6])
{
	GnomePrintPs2 * ps2;
	GnomePrintPs2GraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_current (ps2, FALSE);

#if 0
	g_print ("Concat %s with %s\n",
					 gnome_print_ps2_print_ctm (gs->ctm),
					 gnome_print_ps2_print_ctm (matrix));
#endif	
	art_affine_multiply (gs->ctm, matrix, gs->ctm);
#if 0	
	g_print ("AfterConcat %s\n",
					 gnome_print_ps2_print_ctm (gs->ctm));
#endif	
	gnome_print_ps2_matrix_clean (gs->ctm);

	return 0;
}

#if 0
static gint
gnome_print_ps2_concat_optimize (GnomePrintContext *pc, const double incoming_matrix[6])
{
	gint ret = 0;
	double matrix[6];

	matrix [0] = incoming_matrix [0];
	matrix [1] = incoming_matrix [1];
	matrix [2] = incoming_matrix [2];
	matrix [3] = incoming_matrix [3];
	matrix [4] = incoming_matrix [4];
	matrix [5] = incoming_matrix [5];
	
	gnome_print_ps2_matrix_clean (matrix);
	ret += gnome_print_context_fprintf (pc,
                                      "[ %g %g %g %g %g %g ] cm" EOL,
                                      matrix [0], matrix [1],
                                      matrix [2], matrix [3],
                                      matrix [4], matrix [5]);
	return 0;
}
#endif

static gint
gnome_print_ps2_closepath (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	gint ret = 0;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_current (ps2, FALSE);
	
	ret += gnome_print_ps2_path_print (pc, gs->current_path);
	ret += gnome_print_context_fprintf (pc, "h" EOL);

  return 0;
}

static int
gnome_print_ps2_image (GnomePrintContext *pc, const char *data, int width, int height,
											 int rowstride, int bytes_per_pixel)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	gchar *hex_data;
  gint status;
  gint bytes_per_line;
	gint data_size, data_size_real;
	gchar str[128];

	/*
  int x, y;
  int pos;
  int startline, ix;
  unsigned char b;
	*/

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);

	gs = gnome_print_ps2_graphic_state_current (ps2, FALSE);

#ifdef FIND_A_WAY_TO_CHECK_FOR_THIS		
	if (!gp_path_length(gs->current_path)) {
		gnome_print_ps2_error (FALSE, "show, currentpoint not defined.");
		return -1;
	}
#endif
	
	/* Dirty HACK ! but we need to FIX the API !, not this hack */
  art_affine_to_string (str, gs->ctm);
  gnome_print_context_fprintf (pc,"%s\n", str);
  gnome_print_context_fprintf (pc,"0 0 m\n");
	/* End of dirty hack */
	
  bytes_per_line = width * bytes_per_pixel;

	/* Image commands */
  status = gnome_print_context_fprintf (pc,
																				"/buf %d string def" EOL
																				"%d %d 8" EOL,
																				bytes_per_line,
																				width,
																				height);
  if (status < 0)
    return status;
  status = gnome_print_context_fprintf (pc, "[ %d 0 0 %d 0 %d ]\n",
																				width,
																				-height,
																				height);
  if (status < 0)
    return status;
  status = gnome_print_context_fprintf (pc, "{ currentfile buf readhexstring pop }\n");
  if (status < 0)
    return status;
  if (bytes_per_pixel == 1)
    status = gnome_print_context_fprintf (pc, "image\n");
  else if (bytes_per_pixel == 3)
    status = gnome_print_context_fprintf (pc, "false %d colorimage\n",
																					bytes_per_pixel);
  if (status < 0)
    return status;
	/* End: image commands */



	data_size = width * height * bytes_per_pixel;
	hex_data = g_new (gchar, gnome_print_encode_hex_wcs (data_size));
	data_size_real = gnome_print_encode_hex (data, hex_data, data_size);
		
	gnome_print_context_write_file (pc, hex_data, data_size_real);
	gnome_print_context_fprintf (pc, EOL);
	
  return 0;
}

#if 0
static gint
gnome_print_ps2_image (GnomePrintContext *pc, const char *data, gint width, gint height,
											 gint rowstride, gint bytes_per_pixel)
{
	GnomePrintPs2 *ps2;
	GnomePrintPs2GraphicState *gs;
	gint ret = 0;
  gint bytes_per_line;

	gchar *image_stream;
	gint image_stream_size;
  gint x, y;
  gint pos;
  const char tohex[16] = "0123456789abcdef";
	
  gint startline, ix;
  unsigned char b;

	return 0;
	
	g_return_val_if_fail (GNOME_IS_PRINT_PS2(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	
	debug (FALSE, "");

	gs = ps2->graphic_state;
	ret += gnome_print_context_fprintf (pc, "%g %g %g %g %g %g cm" EOL,
																			gs->ctm[0], gs->ctm[1],
																			gs->ctm[2],	gs->ctm[3],
																			gs->ctm[4], gs->ctm[5]);
	image_stream_size = 1024;
	image_stream = g_new (gchar, image_stream_size);
	
  bytes_per_line = width * bytes_per_pixel;

  pos = 0;
  startline = 0;
  for (y = 0; y < height; y++)
	{
		ix = startline;
		for (x = 0; x < bytes_per_line; x++)
		{
			b = data[ix++];
			image_stream [pos++] = tohex[b >> 4];
			image_stream [pos++] = tohex[b & 15];
			if (pos + 32 > image_stream_size)
			{
				image_stream_size += 1024;
				image_stream = g_realloc (image_stream, image_stream_size);
			}

			if ((pos%72) == 0)
	      image_stream [pos++] = '\n';
		}
		startline += rowstride;
	}

  if (pos)
	{
		image_stream [pos++] = '\n';
		image_stream [pos++] = 0;
	}
	else
		return -1;

	ret += gnome_print_context_fprintf (pc, "/Im i Do" EOL);
	
  return 0;
}
#endif

static gint
gnome_print_ps2_grayimage (GnomePrintContext *pc, const char *data, gint width, gint height, gint rowstride)
{
	GnomePrintPs2 *ps2;
	gint ret = 0;

	g_return_val_if_fail (GNOME_IS_PRINT_PS2(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	
	debug (FALSE, "");

  ret += gnome_print_ps2_image (pc,
																data,
																width,
																height,
																rowstride,
																1);

	return ret;
}

static gint
gnome_print_ps2_rgbimage (GnomePrintContext *pc,
													const char *data,
													gint width,
													gint height,
													gint rowstride)
{
	GnomePrintPs2 *ps2;
	gint ret = 0;

	g_return_val_if_fail (GNOME_IS_PRINT_PS2(pc), -1);
	ps2 = GNOME_PRINT_PS2 (pc);
	g_return_val_if_fail (ps2 != NULL, -1);
	
	debug (FALSE, "");

	ret += gnome_print_ps2_image (pc,
																data,
																width,
																height,
																rowstride,
																3);
	
  return ret;
}

/* These are the PostScript 35, assumed to be in the printer. */
static char *gnome_print_ps2_internal_fonts[] = {
  "AvantGarde-Book",
  "AvantGarde-BookOblique",
  "AvantGarde-Demi",
  "AvantGarde-DemiOblique",
  "Bookman-Demi",
  "Bookman-DemiItalic",
  "Bookman-Light",
  "Bookman-LightItalic",
  "Courier",
  "Courier-Bold",
  "Courier-BoldOblique",
  "Courier-Oblique",
  "Helvetica",
  "Helvetica-Bold",
  "Helvetica-BoldOblique",
  "Helvetica-Narrow",
  "Helvetica-Narrow-Bold",
  "Helvetica-Narrow-BoldOblique",
  "Helvetica-Narrow-Oblique",
  "Helvetica-Oblique",
  "NewCenturySchlbk-Bold",
  "NewCenturySchlbk-BoldItalic",
  "NewCenturySchlbk-Italic",
  "NewCenturySchlbk-Roman",
  "Palatino-Bold",
  "Palatino-BoldItalic",
  "Palatino-Italic",
  "Palatino-Roman",
  "Symbol",
  "Times-Bold",
  "Times-BoldItalic",
  "Times-Italic",
  "Times-Roman",
  "ZapfChancery-MediumItalic",
  "ZapfDingbats"
};


static gint
gnome_print_ps2_page_free (GnomePrintPs2Page *page)
{
	g_return_val_if_fail (page != NULL, FALSE);

	if (page->page_name)
		g_free (page->page_name);
	g_free (page);

	return TRUE;
}

static void
gnome_print_ps2_destroy (GtkObject *object)
{
	GnomePrintPs2Font *font;
	GnomePrintPs2Page *page;
	GnomePrintPs2 *ps2;
	GList *list;
	gint showpaged_all;
	gint n;

	debug (FALSE, "");

  g_return_if_fail (object != NULL);
  g_return_if_fail (GNOME_IS_PRINT_PS2 (object));
  ps2 = GNOME_PRINT_PS2 (object);

	/* gsave stack & graphic States*/
	if (ps2->gsave_level_number != 0)
		g_warning ("gsave unmatched. Should end with an empty stack");
	g_free (ps2->gsave_stack);
	gnome_print_ps2_graphic_state_free (ps2->graphic_state);
	gnome_print_ps2_graphic_state_free (ps2->graphic_state_set);

	if (g_list_length (ps2->pages) == 0)
		showpaged_all = FALSE;
	else
		showpaged_all = TRUE;

	/* Free Pages */
	for (list = ps2->pages; list != NULL; list = list->next) {
		page = (GnomePrintPs2Page *) list->data;
		if (!page->showpaged)
			showpaged_all = FALSE;
		gnome_print_ps2_page_free (page);
	}
	gnome_print_ps2_page_free (ps2->current_page);
	g_list_free (ps2->pages);

	if (!showpaged_all && (g_list_length(ps2->pages)>0))
		g_warning ("The application didn't called \"showpage\" for\n"
							 "one or more pages. Please report this bug for the\n"
							 "program you are using to print. Some **CRITICAL**\n"
							 "messages are normal because of this bug.\n"
							 "This is not a gnome-print bug.\n\n");


	for (n=0; n < ps2->fonts_number; n++) {
		font = &ps2->fonts[n];
		gtk_object_unref (GTK_OBJECT (font->gnome_font));
		g_free (font->font_name);
	}
	
	g_free (ps2->fonts); 
	g_free (ps2->fonts_internal);
	if (ps2->fonts_external) g_free (ps2->fonts_external);

  if (* GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gnome_print_ps2_init (GnomePrintPs2 *ps2)
{
	GnomePrintContext *pc = GNOME_PRINT_CONTEXT (ps2);
	gint n;

	debug (FALSE, "");

	ps2->current_page = NULL;
	ps2->pages = NULL;
	ps2->current_page_number = 1;
	
	/* Fonts - Internal */
	ps2->fonts_internal_number = sizeof(gnome_print_ps2_internal_fonts) / sizeof(gchar *);
  ps2->fonts_internal = g_new (gchar *, ps2->fonts_internal_number);
	for (n = 0; n < sizeof(gnome_print_ps2_internal_fonts) / sizeof(gchar *); n++)
    ps2->fonts_internal [n] = gnome_print_ps2_internal_fonts[n];

	/* Fonts - External */
  ps2->fonts_external_max = GNOME_PRINT_PS2_NUMBER_OF_ELEMENTS;
	ps2->fonts_external_number = 0;
  ps2->fonts_external = g_new (gchar *, ps2->fonts_external_max);

	/* Fonts - Ps2Fonts */
  ps2->fonts_max = GNOME_PRINT_PS2_NUMBER_OF_ELEMENTS;
	ps2->fonts_number = 0;
  ps2->fonts = g_new (GnomePrintPs2Font, ps2->fonts_max);

	/* Start Page */
	gnome_print_ps2_page_start (pc);

	/* Allocate and set defaults for the current graphic state */
	ps2->graphic_state     = gnome_print_ps2_graphic_state_new (FALSE);
	ps2->graphic_state_set = gnome_print_ps2_graphic_state_new (TRUE);

	/* gsave/grestore */
	ps2->gsave_level_max = GNOME_PRINT_PS2_NUMBER_OF_ELEMENTS;
	ps2->gsave_level_number = 0;
	ps2->gsave_stack = g_new (GnomePrintPs2Gsave, ps2->gsave_level_max);

}

static void
gnome_print_ps2_class_init (GnomePrintPs2Class *class)
{
  GtkObjectClass *object_class;
  GnomePrintContextClass *pc_class;

  object_class = (GtkObjectClass *)class;
  pc_class     = (GnomePrintContextClass *)class;

	parent_class = gtk_type_class (gnome_print_context_get_type ());
	
  object_class->destroy = gnome_print_ps2_destroy;

  pc_class->newpath         = gnome_print_ps2_newpath;
  pc_class->moveto          = gnome_print_ps2_moveto;
  pc_class->lineto          = gnome_print_ps2_lineto;
  pc_class->curveto         = gnome_print_ps2_curveto;
  pc_class->closepath       = gnome_print_ps2_closepath;
  pc_class->setrgbcolor     = gnome_print_ps2_setrgbcolor;
  pc_class->fill            = gnome_print_ps2_fill;
  pc_class->setlinewidth    = gnome_print_ps2_setlinewidth;
  pc_class->setmiterlimit   = gnome_print_ps2_setmiterlimit;
  pc_class->setlinejoin     = gnome_print_ps2_setlinejoin;
  pc_class->setlinecap      = gnome_print_ps2_setlinecap;
  pc_class->setdash         = gnome_print_ps2_setdash;
  pc_class->strokepath      = gnome_print_ps2_strokepath;
  pc_class->stroke          = gnome_print_ps2_stroke;
  pc_class->setfont         = gnome_print_ps2_setfont;
  pc_class->show_sized      = gnome_print_ps2_show_sized;
  pc_class->concat          = gnome_print_ps2_concat;
  pc_class->gsave           = gnome_print_ps2_gsave;
  pc_class->grestore        = gnome_print_ps2_grestore;
  pc_class->clip            = gnome_print_ps2_clip;
  pc_class->grayimage       = gnome_print_ps2_grayimage;
  pc_class->rgbimage        = gnome_print_ps2_rgbimage;
  pc_class->textline        = gnome_print_ps2_textline;
  pc_class->showpage        = gnome_print_ps2_showpage;
  pc_class->beginpage       = gnome_print_ps2_beginpage;
  pc_class->setopacity      = gnome_print_ps2_setopacity;
  pc_class->glyphlist       = gnome_print_ps2_glyphlist;

  pc_class->close         = gnome_print_ps2_close;
}
