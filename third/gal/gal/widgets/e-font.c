/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-font.c - Temporary wrappers around GdkFonts to get unicode displaying
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#define _E_FONT_C_

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <iconv.h>
#include "gal/util/e-cache.h"
#include "e-font.h"
#include "e-unicode.h"
#include "gal/util/e-iconv.h"

#ifdef HAVE_CODESET
#include <langinfo.h>
#endif

static int e_font_verbose = 0;

#define E_FONT_VERBOSE
#define E_FONT_CACHE_SIZE 32

enum {
	E_XF_FOUNDRY,
	E_XF_FAMILY,
	E_XF_WEIGHT,
	E_XF_SLANT,
	E_XF_SET_WIDTH,
	E_XF_ADD_STYLE,
	E_XF_PIXEL_SIZE,
	E_XF_POINT_SIZE,
	E_XF_RESOLUTION_X,
	E_XF_RESOLUTION_Y,
	E_XF_SPACING,
	E_XF_AVERAGE_WIDTH,
	E_XF_CHARSET
};

struct _EFont {
	gint refcount;
	GdkFont *font;
	GdkFont *bold;
	gboolean twobyte;
	gboolean nbsp_zero_width;
	iconv_t to;
	iconv_t from;
};

static EFont * e_font_from_gdk_fontset (GdkFont *gdkfont);
static gchar * get_font_name (const GdkFont * font);
static void split_name (gchar * c[], gchar * name);
static gboolean find_variants (gchar **namelist, gint length,
			       gchar *base_weight, gchar **light,
			       gchar **bold);
static gint         e_font_to_native   (EFont *font, gchar *native, const gchar *utf, gint bytes);
#ifdef E_FONT_VERBOSE
static void e_font_print_gdk_font_name (const GdkFont * font);
#endif

/*
 * Creates EFont, given GdkFont, just like gdk_font(set)_load would do
 */

EFont *
e_font_from_gdk_name (const gchar *name)
{
	EFont * font;
	GdkFont *gdkfont;
	gboolean need_fontset = FALSE;
	gchar n[1024];

	g_return_val_if_fail (name != NULL, NULL);

#ifdef E_FONT_VERBOSE
	if (e_font_verbose) {
		g_print ("(e_font_from_gdk_name) Requested: %s\n", name);
	}
#endif

	/* Check whether we need font or fontset here */
	if (strchr (name, ',')) {
		/* Name is already fontset name */
		need_fontset = TRUE;
		g_snprintf (n, 1024, name);
	} else {
#if 0
		gchar *c[14], *nn;
		gint i;
		/* Split name into pieces */
		for (i = 0; i < 14; i++) c[i] = NULL;
		nn = g_strdup (name);
		split_name (c, nn);
		if (c[E_XF_CHARSET] != NULL) {
			gint len;
			/* Font name has charset */
			/* Determine length of base encoding part */
			if (strchr (c[E_XF_CHARSET], '.')) {
				len = strchr (c[E_XF_CHARSET], '.') - c[E_XF_CHARSET];
			} else {
				len = strlen (c[E_XF_CHARSET]);
			}
			/* Test well-known cases */
			/* fixme: Whoever actually uses CJK locale should check/change that */
			if (!strncasecmp ("eucjp", c[E_XF_CHARSET], len)) need_fontset = TRUE;
			else if (!strncasecmp ("ujis", c[E_XF_CHARSET], len)) need_fontset = TRUE;
			else if (!strncasecmp ("jis", c[E_XF_CHARSET], 3)) need_fontset = TRUE;
			else if (!strncasecmp ("gb2312", c[E_XF_CHARSET], len)) need_fontset = TRUE;
			else if (!strncasecmp ("ksc5601", c[E_XF_CHARSET], len)) need_fontset = TRUE;
		}
		g_free (nn);
#else
		if (MB_CUR_MAX > 1) need_fontset = TRUE;
#endif
		if (need_fontset) {
			/* We do not have ',', so we have to extend name */
			g_snprintf (n, 1024, "%s,*", name);
		} else {
			/* Simply duplicate name */
			g_snprintf (n, 1024, name);
		}
	}

	/*
	 * Here we are:
	 * n is actual name to be loaded
	 * need_fontset determines, whether we need fontset or not
	 */

#ifdef E_FONT_VERBOSE
	if (e_font_verbose) {
		g_print ("(e_font_from_gdk_name) Actual: %s\n", n);
	}
#endif

	if (need_fontset) {
		gdkfont = gdk_fontset_load (n);
	} else {
		gdkfont = gdk_font_load (n);
	}
	if (!gdkfont) gdkfont = gdk_font_load ("fixed");
	g_return_val_if_fail (gdkfont != NULL, NULL);
	font = e_font_from_gdk_font (gdkfont);
	gdk_font_unref (gdkfont);

	return font;
}

static void
set_nbsp_zero_width_flag (EFont *efont)
{
	guchar *nbsp = "\xc2\xa0";
	guchar native_nbsp [8];

	efont->nbsp_zero_width = FALSE;
	efont->nbsp_zero_width = gdk_text_width (efont->font, native_nbsp, e_font_to_native (efont, native_nbsp, nbsp, 2))
		? FALSE : TRUE;
}

/*
 * Creates EFont from existing GdkFont.
 * 1. Use cache
 * 2. Analyze, whether we need font or fontset. For fontset use e_font_from_gdk_fontset
 * 3. Try, if we can simply use iso-10646 variant of font
 * 4. Determine bold or light variant
 */

EFont *
e_font_from_gdk_font (GdkFont *gdkfont)
{
	static ECache * cache = NULL;
	EFont *font;
	GdkFont *keyfont, *boldfont, *lightfont;
	gchar * name;
	XFontStruct *xfs;

#ifdef E_FONT_VERBOSE
	if (e_font_verbose) {
		g_print ("(e_font_from_gdk_font): Initial font:\n");
		e_font_print_gdk_font_name (gdkfont);
	}
#endif

	g_return_val_if_fail (gdkfont != NULL, NULL);

	/* Try cache */
	if (!cache) cache = e_cache_new (NULL, /* Key hash */
					 NULL, /* Key compare */
					 (ECacheDupFunc) gdk_font_ref, /* Key dup func */
					 (ECacheFreeFunc) gdk_font_unref, /* Key free func */
					 (ECacheFreeFunc) e_font_unref, /* Object free func */
					 E_FONT_CACHE_SIZE, /* Soft limit (fake) */
					 E_FONT_CACHE_SIZE); /* Hard limit */

	if ((font = e_cache_lookup (cache, gdkfont))) {
		/* Use cached font, increase refcount and return */
		e_font_ref (font);
		return font;
	} else {
		/* Save original font to be used as cache key */
		keyfont = gdkfont;
	}

	if (gdkfont->type == GDK_FONT_FONTSET) {
#if 0
		gchar *xlocale[] = {	"ja_JP.euc", "ja_JP.ujis",
					"zh_CN.GB2312", "zh_TW.Big5",
					"ko_KR.eucKR", NULL};
		char * fslocale;
		int i;

		fslocale = XLocaleOfFontSet (GDK_FONT_XFONT (gdkfont));
		/* fixme: Whoever uses CJK locales should check that */
		for (i = 0; xlocale[i] != NULL; i++) {
			if ( g_strncasecmp (fslocale, xlocale[i], strlen (xlocale[i])) == 0) {
				/* We need fontset */
				font = e_font_from_gdk_fontset (gdkfont);
				/* Insert into cache */
				if (e_cache_insert (cache, keyfont, font, 1)) {
					/* Inserted, so add cache ref manually */
					e_font_ref (font);
				}
				return font;
			}
		}
#else
		if (MB_CUR_MAX > 1) {
			/* We need fontset */
			font = e_font_from_gdk_fontset (gdkfont);
			/* Insert into cache */
			if (e_cache_insert (cache, keyfont, font, 1)) {
				/* Inserted, so add cache ref manually */
				e_font_ref (font);
			}
			return font;
		}
#endif
	}

	/* We can do with simple font */
	/* Fill out emergency values */
	lightfont = gdkfont;
	boldfont = NULL;

	/* ref in case we cannot find any better variants */
	gdk_font_ref (gdkfont);

	/* Get full-qualified font name */
	name = get_font_name (gdkfont);

	if (name) {
		gchar p[1024];
		gchar *c[14];
		const gchar * encoding;
		gchar *boldname, *lightname;
		gchar **namelist;
		GdkFont *newfont;
		gint numfonts;

		/* Split name into components */
		split_name (c, name);

		/* Try to find iso-10646-1 encoded font with same name */
		/* Compose name for unicode encoding */
		/* fixme: We require same pixel size, I do not know, whether that is correct */
		encoding = "iso10646-1";
		g_snprintf (p, 1024, "-*-%s-%s-%s-%s-*-%s-*-*-*-*-*-%s",
			    c[E_XF_FAMILY],
			    c[E_XF_WEIGHT],
			    c[E_XF_SLANT],
			    c[E_XF_SET_WIDTH],
			    c[E_XF_PIXEL_SIZE],
			    encoding);
		/* Try to load unicode font */
#ifdef E_FONT_VERBOSE
		if (e_font_verbose) {
			g_print ("Trying unicode font: %s\n", p);
		}
#endif
		newfont = gdk_font_load (p);
		if (newfont) {
#ifdef E_FONT_VERBOSE
			if (e_font_verbose) {
				e_font_print_gdk_font_name (newfont);
			}
#endif
			/* OK, use found iso-10646 font */
			/* Unref original font */
			gdk_font_unref (gdkfont);
			gdkfont = newfont;
		} else {
			encoding = c[E_XF_CHARSET];
		}

		/*
		 * Here we are:
		 * gdkfont - font to be used, with right refcount
		 * encoding - X encoding string for that font
		 */

		/* Read all weight variants of given font */
		g_snprintf (p, 1024, "-*-%s-*-%s-%s-*-%s-*-*-*-*-*-%s",
			    c[E_XF_FAMILY],
			    c[E_XF_SLANT],
			    c[E_XF_SET_WIDTH],
			    c[E_XF_PIXEL_SIZE],
			    encoding);
		namelist = XListFonts (GDK_FONT_XDISPLAY (gdkfont), p, 32, &numfonts);

		lightname = boldname = NULL;
		if (namelist &&
		    numfonts &&
		    find_variants (namelist, numfonts, c[E_XF_WEIGHT], &lightname, &boldname) &&
		    lightname &&
		    boldname) {
			/* There are usable weight variants - their weights are returned in lightname and boldname */
			lightfont = NULL;
			boldfont  = NULL;
			if (!g_strcasecmp (c[E_XF_WEIGHT], lightname)) {
				/* Our name maps to light font */
				lightfont = gdkfont;
			} else if (!g_strcasecmp (c[E_XF_WEIGHT], boldname)) {
				/* Our name maps to bold font */
				boldfont = gdkfont;
			} else {
				/* Our name maps neither to light nor to bold font */
				gdk_font_unref (gdkfont);
				gdkfont = NULL;
			}

			if (!lightfont) {
				/* We have to load lighter variant */
				g_snprintf (p, 1024, "-*-%s-%s-%s-%s-*-%s-*-*-*-*-*-%s",
					    c[E_XF_FAMILY],
					    lightname,
					    c[E_XF_SLANT],
					    c[E_XF_SET_WIDTH],
					    c[E_XF_PIXEL_SIZE],
					    encoding);
#ifdef E_FONT_VERBOSE
				if (e_font_verbose) {
					g_print ("Trying light: %s\n", p);
				}
#endif
				lightfont = gdk_font_load (p);
#ifdef E_FONT_VERBOSE
				if (e_font_verbose) {
					e_font_print_gdk_font_name (lightfont);
				}
#endif
			}
			if (!boldfont) {
				/* We have to load bolder variant */
				g_snprintf (p, 1024, "-*-%s-%s-%s-%s-*-%s-*-*-*-*-*-%s",
					    c[E_XF_FAMILY],
					    boldname,
					    c[E_XF_SLANT],
					    c[E_XF_SET_WIDTH],
					    c[E_XF_PIXEL_SIZE],
					    encoding);
#ifdef E_FONT_VERBOSE
				if (e_font_verbose) {
					g_print ("Trying bold: %s\n", p);
				}
#endif
				boldfont = gdk_font_load (p);
#ifdef E_FONT_VERBOSE
				if (e_font_verbose) {
					e_font_print_gdk_font_name (boldfont);
				}
#endif
			}
			/* I am not sure, whether that can happen, but let's be cautious */
			if (!lightfont) {
				lightfont = keyfont;
				gdk_font_ref (lightfont);
			}
		} else {
			/* We fall back to double drawing */
			lightfont = gdkfont;
			boldfont = NULL;
		}

		XFreeFontNames (namelist);

		g_free (name);
	}

#ifdef E_FONT_VERBOSE
	if (e_font_verbose) {
		g_print ("******** Creating EFont with following fonts ********\n");
		e_font_print_gdk_font_name (lightfont);
		e_font_print_gdk_font_name (boldfont);
	}
#endif

	/* Here we are:
	 * lightfont is light GdkFont with correct refcount
	 * boldfont is either bold GdkFont or NULL
	 */

	font = g_new (EFont, 1);

	xfs = GDK_FONT_XFONT (lightfont);

	font->refcount = 1;
	font->font = lightfont;
	font->bold = boldfont;
	font->twobyte = (lightfont->type == GDK_FONT_FONTSET || ((xfs->min_byte1 != 0) || (xfs->max_byte1 != 0)));
	font->to = e_iconv_to_gdk_font (font->font);
	font->from = e_iconv_from_gdk_font (font->font);
	set_nbsp_zero_width_flag (font);

	/* Insert into cache */
	if (e_cache_insert (cache, keyfont, font, 1)) {
		/* Inserted, so add cache ref manually */
		e_font_ref (font);
	}

	return font;

}

GdkFont *
e_font_to_gdk_font (EFont *font, EFontStyle style)
{
	if (style & E_FONT_BOLD) {
		gdk_font_ref(font->bold);
		return font->bold;
	} else {
		gdk_font_ref(font->font);
		return font->font;
	}
}

/*
 * Creates EFont from GdkFontset, if locale needs fontset
 */

static EFont *
e_font_from_gdk_fontset (GdkFont *gdkfont)
{
	EFont *font;
	GdkFont *boldfont, *lightfont;
	gchar * name;
	XFontStruct *xfs;

#ifdef E_FONT_VERBOSE
	if (e_font_verbose) {
		g_print ("(e_font_from_gdk_fontset): Initial font:\n");
		e_font_print_gdk_font_name (gdkfont);
	}
#endif

	lightfont = gdkfont;
	boldfont = NULL;

	/* fixme: why we ref here? */
	gdk_font_ref (gdkfont);

	/* Get full-qualified font name */
	name = get_font_name (gdkfont);

	if (name) {
		gchar *c[14];
		gchar p[1024];
		const gchar * encoding;
		gchar *boldname, *lightname;
		gchar **namelist;
		gint numfonts;

		/* Split name into components */
		split_name (c, name);

		encoding = c[E_XF_CHARSET];

		/* Here we are:
		 * gdkfont points to fontset to be used, with right refcount
		 * encoding is the X encoding string for that font
		 */

		/* Read all weight variants of base font */
		g_snprintf (p, 1024, "-*-%s-*-%s-%s-*-%s-*-*-*-*-*-%s",
			    c[E_XF_FAMILY],
			    c[E_XF_SLANT],
			    c[E_XF_SET_WIDTH],
			    c[E_XF_PIXEL_SIZE],
			    encoding);
		namelist = XListFonts (GDK_FONT_XDISPLAY (gdkfont), p, 32, &numfonts);

		if (namelist &&
		    numfonts &&
		    find_variants (namelist, numfonts, c[E_XF_WEIGHT], &lightname, &boldname) &&
		    lightname &&
		    boldname) {
			/* There are usable variants - their weights are returned in lightname and boldname */
			lightfont = NULL;
			boldfont  = NULL;
			if (!g_strcasecmp (c[E_XF_WEIGHT], lightname)) {
				/* Our name maps to light font */
				lightfont = gdkfont;
				gdk_font_ref (gdkfont);
			} else if (!g_strcasecmp (c[E_XF_WEIGHT], boldname)) {
				/* Our name maps to bold font */
				boldfont = gdkfont;
				gdk_font_ref (gdkfont);
			}

			if (!lightfont) {
				/* We have to load lighter variant */
				g_snprintf (p, 1024, "-*-%s-%s-%s-%s-*-%s-*-*-*-*-*-%s,*",
					    c[E_XF_FAMILY],
					    lightname,
					    c[E_XF_SLANT],
					    c[E_XF_SET_WIDTH],
					    c[E_XF_PIXEL_SIZE],
					    encoding);
#ifdef E_FONT_VERBOSE
				if (e_font_verbose) {
					g_print ("Trying light: %s\n", p);
				}
#endif
				lightfont = gdk_fontset_load (p);
#ifdef E_FONT_VERBOSE
				if (e_font_verbose) {
					e_font_print_gdk_font_name (lightfont);
				}
#endif
			}
			if (!boldfont) {
				/* We have to load lighter variant */
				g_snprintf (p, 1024, "-*-%s-%s-%s-%s-*-%s-*-*-*-*-*-%s,*",
					    c[E_XF_FAMILY],
					    boldname,
					    c[E_XF_SLANT],
					    c[E_XF_SET_WIDTH],
					    c[E_XF_PIXEL_SIZE],
					    encoding);
#ifdef E_FONT_VERBOSE
				if (e_font_verbose) {
					g_print ("Trying bold: %s\n", p);
				}
#endif
				boldfont = gdk_fontset_load (p);
#ifdef E_FONT_VERBOSE
				if (e_font_verbose) {
					e_font_print_gdk_font_name (boldfont);
				}
#endif
			}
			if (!lightfont) {
				lightfont = gdkfont;
				gdk_font_ref (lightfont);
			} else {
				gdk_font_unref (gdkfont);
			}
		} else {
			/* We fall back to double drawing */
			lightfont = gdkfont;
			boldfont = NULL;
		}

		XFreeFontNames (namelist);

		g_free (name);
	}

#ifdef E_FONT_VERBOSE
	if (e_font_verbose) {
		g_print ("******** Creating EFont with following fonts ********\n");
		e_font_print_gdk_font_name (lightfont);
		e_font_print_gdk_font_name (boldfont);
	}
#endif

	/* Here we are:
	 * lightfont is light GdkFont with correct refcount
	 * boldfont is either bold GdkFont or NULL
	 */

	font = g_new (EFont, 1);

	xfs = GDK_FONT_XFONT (lightfont);

	font->refcount = 1;
	font->font = lightfont;
	font->bold = boldfont;
	font->twobyte = (lightfont->type == GDK_FONT_FONTSET || ((xfs->min_byte1 != 0) || (xfs->max_byte1 != 0)));
	font->to = e_iconv_to_gdk_font (font->font);
	font->from = e_iconv_from_gdk_font (font->font);
	set_nbsp_zero_width_flag (font);

	return font;

}

gchar *
e_font_get_name (EFont *font)
{
	return get_font_name (font->font);
}

void
e_font_ref (EFont *font)
{
	font->refcount++;
}

void
e_font_unref (EFont *font)
{
	font->refcount--;

	if (font->refcount < 1) {
		e_iconv_close(font->to);
		e_iconv_close(font->from);
		gdk_font_unref (font->font);
		if (font->bold) gdk_font_unref (font->bold);
		g_free (font);
	}
}

gint
e_font_ascent (EFont * font)
{
	return font->font->ascent;
}

gint
e_font_descent (EFont * font)
{
	return font->font->descent;
}

static gint
no_conv_wrapper (EFont *font, gchar *native, const gchar *utf, gint bytes)
{
	gint len;
	const gchar *u;
	gunichar uc;

	u   = utf;
	len = 0;

	while (u && u - utf < bytes) {
		u = e_unicode_get_utf8 (u, &uc);
		if (font->twobyte) {
			native [len] = (uc & 0xff00) >> 8; len++;
		}
		native [len] = uc & 0xff; len++;
	}

	return len;
}

static guchar *
replace_nbsp_with_spaces (const guchar *orig, gint *bytes)
{
	guchar *rv, *p;
	gboolean nbsp1;
	gint remains = *bytes;

	if (!orig)
		return NULL;

	/* printf ("bytes: %d orig\n%s\n", *bytes, orig); */

	p = rv = (guchar *) g_malloc (strlen (orig) + 1);
	nbsp1  = FALSE;

	while (remains) {
		if (*orig == 0xc2)
			nbsp1 = TRUE;
		else if (nbsp1) {
			if (*orig == 0xa0) {
				*p = ' ';   p ++;
				(*bytes) --;
			} else {
				*p = 0xc2;  p ++;
				*p = *orig; p ++;
			}
			nbsp1 = FALSE;
		} else {
			*p = *orig;
			p ++;
		}
		orig ++;
		remains --;
	}

	if (nbsp1) {
		*p = 0xc2; p ++;
	}
	*p = 0;

	/* printf ("rv: %s\n", rv); */

	return rv;
}

static gint
e_font_to_native (EFont *font, gchar *native, const gchar *utf, gint bytes)
{
	const char *ib;
	char *ob;
	size_t ibl, obl;
	gint rv;

	if (font->nbsp_zero_width)
		utf = replace_nbsp_with_spaces (utf, &bytes);

	if (font->to == (iconv_t) -1)
		rv = no_conv_wrapper (font, native, utf, bytes);
	else {
		ib = utf;
		ibl = bytes;
		ob = native;
		obl = bytes * 4;

		while (ibl > 0) {
			e_iconv (font->to, &ib, &ibl, &ob, &obl);
			if (ibl > 0) {
				ib = g_utf8_next_char (ib);

				ibl = bytes - (ib - utf);
				if (ibl > bytes) ibl = 0;
				if (!font->twobyte) {
					*ob++ = '_';
					obl--;
				} else {
					*((guint16 *) ob) = '_';
					ob += 2;
					obl -= 2;
				}
			}
		}
		rv = ob - native;
	}

	if (font->nbsp_zero_width)
		g_free ((void *) utf);

	return rv;
}

void
e_font_draw_utf8_text (GdkDrawable *drawable, EFont *font, EFontStyle style, GdkGC *gc, gint x, gint y, const gchar *text, gint numbytes)
{
	gchar *native;
	gint native_bytes;

	g_return_if_fail (drawable != NULL);
	g_return_if_fail (font != NULL);
	g_return_if_fail (gc != NULL);
	g_return_if_fail (text != NULL);

	if (numbytes < 1) return;

	native = alloca (numbytes * 4);

	native_bytes = e_font_to_native (font, native, text, numbytes);
	
	if ((style & E_FONT_BOLD) && (font->bold)) {
		gdk_draw_text (drawable, font->bold, gc, x, y, native, native_bytes);
	} else {
		gdk_draw_text (drawable, font->font, gc, x, y, native, native_bytes);
		if (style & E_FONT_BOLD)
			gdk_draw_text (drawable, font->font, gc, x + 1, y, native, native_bytes);
	}
}

gint
e_font_utf8_text_width (EFont *font, EFontStyle style, const char *text, int numbytes)
{
	gchar *native;
	gint native_bytes;
	gint width;

	g_return_val_if_fail (font != NULL, 0);
	g_return_val_if_fail (text != NULL, 0);

	if (numbytes < 1) return 0;

	native = alloca (numbytes * 4);

	native_bytes = e_font_to_native (font, native, text, numbytes);

	if ((style & E_FONT_BOLD) && (font->bold)) {
		width = gdk_text_width (font->bold, native, native_bytes);
	} else {
		width = gdk_text_width (font->font, native, native_bytes);
	}

	return width;
}

gint
e_font_utf8_char_width (EFont *font, EFontStyle style, char *text)
{
	g_return_val_if_fail (font != NULL, 0);
	g_return_val_if_fail (text != NULL, 0);

	return e_font_utf8_text_width (font, style, text, g_utf8_skip[*(guchar *)text]);
}

const gchar *
e_gdk_font_encoding (GdkFont *font)
{
	static ECache * cache = NULL;
	Atom font_atom, atom;
	Bool status;
	char *name, *p;
	const gchar *encoding;
	gint i;

	if (!font) return NULL;

	if (!cache) cache = e_cache_new (NULL, /* Key hash (GdkFont) */
					 NULL, /* Key compare (GdkFont) */
					 (ECacheDupFunc) gdk_font_ref, /* Key dup func */
					 (ECacheFreeFunc) gdk_font_unref, /* Key free func */
					 NULL, /* Object free func (const string) */
					 E_FONT_CACHE_SIZE, /* Soft limit (fake) */
					 E_FONT_CACHE_SIZE); /* Hard limit */

	encoding = e_cache_lookup (cache, font);
	if (encoding) return encoding;

#ifdef E_FONT_VERBOSE
	if (e_font_verbose) {
		g_print ("Extracting X font info\n");
	}
#endif

	if (font->type == GDK_FONT_FONTSET) {
		/* We are fontset, so try to find right encoding from locale */
		encoding = e_iconv_charset_name(e_iconv_locale_charset());
		if (encoding)
			return encoding;
	}

	font_atom = gdk_atom_intern ("FONT", FALSE);

	if (font->type == GDK_FONT_FONTSET) {
		XFontStruct **font_structs;
		gint num_fonts;
		gchar **font_names;

		num_fonts = XFontsOfFontSet (GDK_FONT_XFONT (font),
					     &font_structs,
					     &font_names);
		status = XGetFontProperty (font_structs[0],
					   font_atom,
					   &atom);
	} else {
		status = XGetFontProperty (GDK_FONT_XFONT (font),
					   font_atom,
					   &atom);
	}

	if (!status) {
		/* Negative cache */
		e_cache_insert (cache, font, NULL, 1);
		return NULL;
	}

	name = p = gdk_atom_name (atom);

	for (i = 0; i < 13; i++) {
		/* Skip hyphen */
		while (*p && (*p != '-')) p++;
		if (*p) p++;
	}

	if (!*p) {
		/* Negative cache */
		e_cache_insert (cache, font, NULL, 1);
		return NULL;
	}

	encoding = e_iconv_charset_name(p);
	e_cache_insert (cache, font, (gpointer) encoding, 1);
	g_free (name);


	return encoding;
}

iconv_t
e_iconv_from_gdk_font (GdkFont *font)
{
	const gchar *enc;

	enc = e_gdk_font_encoding (font);
	if (enc == NULL)
		return (iconv_t) -1;

	return e_iconv_open("utf-8", enc);
}

iconv_t
e_iconv_to_gdk_font (GdkFont *font)
{
	const gchar *enc;

	enc = e_gdk_font_encoding (font);
	if (enc == NULL)
		return (iconv_t) -1;

	return e_iconv_open(enc, "utf-8");
}

/*
 * Return newly allocated full name
 */

static gchar *
get_font_name (const GdkFont * font)
{
	Atom font_atom, atom;
	Bool status;

#ifdef E_FONT_VERBOSE
	if (e_font_verbose) {
		g_print ("Extracting X font info\n");
	}
#endif

	font_atom = gdk_atom_intern ("FONT", FALSE);

	if (font->type == GDK_FONT_FONTSET) {
		XFontStruct **font_structs;
		gint num_fonts;
		gchar **font_names;

		num_fonts = XFontsOfFontSet (GDK_FONT_XFONT (font), &font_structs, &font_names);
#ifdef E_FONT_VERBOSE
		if (e_font_verbose) {
			gint i;

			g_print ("Fonts of fontset:\n");
			for (i = 0; i < num_fonts; i++)
				g_print ("  %s\n", font_names[i]);
		}
#endif
		status = XGetFontProperty (font_structs[0], font_atom, &atom);
	} else {
		status = XGetFontProperty (GDK_FONT_XFONT (font), font_atom, &atom);
	}

	if (status) {
		return gdk_atom_name (atom);
	}

	return NULL;
}

/*
 * Splits full X font name into pieces, overwriting hyphens
 */

static void
split_name (gchar * c[], gchar * name)
{
	gchar *p;
	gint i;

	p = name;
	if (*p == '-') p++;

	for (i = 0; i < 12; i++) {
		c[i] = p;
		/* Skip text */
		while (*p && (*p != '-')) p++;
		/* Replace hyphen with '\0' */
		if (*p) *p++ = '\0';
	}

	c[i] = p;
}

/*
 * Find light and bold variants of a font, ideally using the provided
 * weight for the light variant, and a weight 2 shades darker than it
 * for the bold variant. If there isn't something 2 shades darker, use
 * something 3 or more shades darker if it exists, or 1 shade darker
 * if that's all there is. If there is nothing darker than the provided
 * weight, but there are lighter fonts, then use the darker one for
 * bold and a lighter one for light.
 */

static gboolean
find_variants (gchar **namelist, gint length, gchar *weight,
	       gchar **lightname, gchar **boldname)
{
	static GHashTable *wh = NULL;
	/* Standard, Found, Bold, Light */
	gint sw, fw, bw, lw;
	gchar s[32];
	gchar *f, *b, *l;
	gchar *p;
	gint i;

	if (!wh) {
		wh = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (wh, "light", GINT_TO_POINTER (1));
		g_hash_table_insert (wh, "book", GINT_TO_POINTER (2));
		g_hash_table_insert (wh, "regular", GINT_TO_POINTER (2));
		g_hash_table_insert (wh, "medium", GINT_TO_POINTER (3));
		g_hash_table_insert (wh, "demibold", GINT_TO_POINTER (5));
		g_hash_table_insert (wh, "bold", GINT_TO_POINTER (6));
		g_hash_table_insert (wh, "black", GINT_TO_POINTER (8));
	}

	g_snprintf (s, 32, weight);
	g_strdown (s);
	sw = GPOINTER_TO_INT (g_hash_table_lookup (wh, s));
	if (sw == 0) return FALSE;

	fw = 0; lw = 0; bw = 32;
	f = NULL; l = NULL; b = NULL;
	*lightname = NULL; *boldname = NULL;

	for (i = 0; i < length; i++) {
		p = namelist[i];
		if (*p) p++;
		while (*p && (*p != '-')) p++;
		if (*p) p++;
		while (*p && (*p != '-')) p++;
		if (*p) p++;
		f = p;
		while (*p && (*p != '-')) p++;
		if (*p) *p = '\0';
		g_strdown (f);
		fw = GPOINTER_TO_INT (g_hash_table_lookup (wh, f));
		if (fw) {
			if (fw > sw) {
				if ((fw - 2 == sw) ||
				    ((fw > bw) && (bw == sw + 1)) ||
				    ((fw < bw) && (fw - 2 > sw))) {
					bw = fw;
					b = f;
				}
			} else if (fw < sw) {
				if ((fw + 2 == sw) ||
				    ((fw < lw) && (lw == sw - 1)) ||
				    ((fw > lw) && (fw + 2 < sw))) {
					lw = fw;
					l = f;
				}
			}
		}
	}

	if (b) {
		*lightname = weight;
		*boldname = b;
		return TRUE;
	} else if (l) {
		*lightname = l;
		*boldname = weight;
		return TRUE;
	}
	return FALSE;
}

#ifdef E_FONT_VERBOSE
/*
 * Return newly allocated full name
 */

static void
e_font_print_gdk_font_name (const GdkFont * font)
{
	Atom font_atom, atom;
	Bool status;

	font_atom = gdk_atom_intern ("FONT", FALSE);

	if (font == NULL) {
		g_print ("GdkFont is NULL\n");
	} else if (font->type == GDK_FONT_FONTSET) {
		XFontStruct **font_structs;
		gint num_fonts;
		gchar **font_names;
		gint i;

		num_fonts = XFontsOfFontSet (GDK_FONT_XFONT (font), &font_structs, &font_names);

		g_print ("Gdk Fontset, locale: %s\n", XLocaleOfFontSet (GDK_FONT_XFONT (font)));
		for (i = 0; i < num_fonts; i++) {
			g_print ("    %s\n", font_names[i]);
		}
	} else {
		gchar * name;
		status = XGetFontProperty (GDK_FONT_XFONT (font), font_atom, &atom);
		name = gdk_atom_name (atom);
		g_print ("GdkFont: %s\n", name);
		if (name) g_free (name);
	}
}
#endif
