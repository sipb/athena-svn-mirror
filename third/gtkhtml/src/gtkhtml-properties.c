/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2000 Helix Code, Inc.
   Authors:           Radek Doulik (rodo@helixcode.com)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHcANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <string.h>
#include <gdk/gdkx.h>
#include "gtkhtml.h"
#include "gtkhtml-properties.h"
#include "htmlfontmanager.h"

#define DEFAULT_FONT_SIZE   12
#define DEFAULT_FONT_SIZE_PRINT   10

#define STRINGIZE(x) #x

static void get_default_fonts (gchar **var_name, gchar **fix_name);

GtkHTMLClassProperties *
gtk_html_class_properties_new (void)
{
	GtkHTMLClassProperties *p = g_new0 (GtkHTMLClassProperties, 1);
	gchar *var_name, *fix_name;

	get_default_fonts (&var_name, &fix_name);

	/* default values */
	p->magic_links             = TRUE;
	p->magic_smileys           = TRUE;
	p->keybindings_theme       = g_strdup ("ms");
	p->font_var                = var_name;
	p->font_fix                = fix_name;
	p->font_var_size           = DEFAULT_FONT_SIZE;
	p->font_fix_size           = DEFAULT_FONT_SIZE;
	p->font_var_points         = FALSE;
	p->font_fix_points         = FALSE;
	p->font_var_print          = g_strdup ("-*-helvetica-*-*-*-*-10-*-*-*-*-*-*-*");
	p->font_fix_print          = g_strdup ("-*-courier-*-*-*-*-10-*-*-*-*-*-*-*");
	p->font_var_size_print     = DEFAULT_FONT_SIZE_PRINT;
	p->font_fix_size_print     = DEFAULT_FONT_SIZE_PRINT;
	p->font_var_print_points   = FALSE;
	p->font_fix_print_points   = FALSE;
	p->animations              = TRUE;
	p->link_color              = g_strdup ("#0000ff");
	p->alink_color             = g_strdup ("#0000ff");
	p->vlink_color             = g_strdup ("#0000ff");

	p->live_spell_check        = TRUE;
	p->spell_error_color.red   = 0xffff;
	p->spell_error_color.green = 0;
	p->spell_error_color.blue  = 0;

	p->language                = g_strdup ("en");

	return p;
}

void
gtk_html_class_properties_destroy (GtkHTMLClassProperties *p)
{
	g_free (p->keybindings_theme);
	g_free (p);
}

#ifdef GTKHTML_HAVE_GCONF
#define GET(t,x,prop,f,c) \
        key = g_strconcat (GTK_HTML_GCONF_DIR, x, NULL); \
        val = gconf_client_get_without_default (client, key, NULL); \
        if (val) { f; p->prop = c (gconf_value_get_ ## t (val)); \
        gconf_value_free (val); } \
        g_free (key);

#define GNOME_SPELL_GCONF_DIR "/GNOME/Spell"
#define GETSP(t,x,prop,f,c) \
        key = g_strconcat (GNOME_SPELL_GCONF_DIR, x, NULL); \
        val = gconf_client_get_without_default (client, key, NULL); \
        if (val) { f; p->prop = c (gconf_value_get_ ## t (val)); \
        gconf_value_free (val); } \
        g_free (key);

void
gtk_html_class_properties_load (GtkHTMLClassProperties *p, GConfClient *client)
{
	GConfValue *val;
	gchar *key;

	g_assert (client);

	GET (bool, "/magic_links", magic_links,,);
	GET (bool, "/magic_smileys", magic_smileys,,);
	GET (bool, "/animations", animations,,);
	GET (string, "/keybindings_theme", keybindings_theme,
	     g_free (p->keybindings_theme), g_strdup);
	GET (string, "/font_variable", font_var,
	     g_free (p->font_var), g_strdup);
	GET (string, "/font_fixed", font_fix,
	     g_free (p->font_fix), g_strdup);
	GET (int, "/font_variable_size", font_var_size,,);
	GET (int, "/font_fixed_size", font_fix_size,,);
	GET (bool, "/font_variable_points", font_var_points,,);
	GET (bool, "/font_fixed_points", font_fix_points,,);
	GET (string, "/font_variable_print", font_var_print,
	     g_free (p->font_var_print), g_strdup);
	GET (string, "/font_fixed_print", font_fix_print,
	     g_free (p->font_fix_print), g_strdup);
	GET (int, "/font_variable_size_print", font_var_size_print,,);
	GET (int, "/font_fixed_size_print", font_fix_size_print,,);
	GET (bool, "/font_variable_print_points", font_var_print_points,,);
	GET (bool, "/font_fixed_print_points", font_fix_print_points,,);

	GET (bool, "/live_spell_check", live_spell_check,,);

	GET (string, "/link_color", link_color, g_free (p->link_color), g_strdup);
	GET (string, "/alink_color", alink_color, g_free (p->alink_color), g_strdup);
	GET (string, "/vlink_color", vlink_color, g_free (p->vlink_color), g_strdup);

	GETSP (int, "/spell_error_color_red",   spell_error_color.red,,);
	GETSP (int, "/spell_error_color_green", spell_error_color.green,,);
	GETSP (int, "/spell_error_color_blue",  spell_error_color.blue,,);

	GETSP (string, "/language", language,
	       g_free (p->language), g_strdup);
}

#define SET(t,x,prop) \
        { key = g_strconcat (GTK_HTML_GCONF_DIR, x, NULL); \
        gconf_client_set_ ## t (client, key, p->prop, NULL); \
        g_free (key); }


void
gtk_html_class_properties_update (GtkHTMLClassProperties *p, GConfClient *client, GtkHTMLClassProperties *old)
{
	gchar *key;

	if (p->animations != old->animations)
		SET (bool, "/animations", animations);
	if (p->magic_links != old->magic_links)
		SET (bool, "/magic_links", magic_links);
	if (p->magic_smileys != old->magic_smileys)
		SET (bool, "/magic_smileys", magic_smileys);
	SET (string, "/keybindings_theme", keybindings_theme);
	if (strcmp (p->font_var, old->font_var))
		SET (string, "/font_variable", font_var);
	if (strcmp (p->font_fix, old->font_fix))
		SET (string, "/font_fixed", font_fix);
	if (p->font_var_points != old->font_var_points)
		SET (bool, "/font_variable_points", font_var_points);
	if (p->font_fix_points != old->font_fix_points)
		SET (bool, "/font_fixed_points", font_fix_points);
	if (p->font_var_size != old->font_var_size || p->font_var_points != old->font_var_points)
		SET (int, "/font_variable_size", font_var_size);
	if (p->font_fix_size != old->font_fix_size || p->font_fix_points != old->font_fix_points)
		SET (int, "/font_fixed_size", font_fix_size);
	if (strcmp (p->font_var_print, old->font_var_print))
		SET (string, "/font_variable_print", font_var_print);
	if (strcmp (p->font_fix_print, old->font_fix_print))
		SET (string, "/font_fixed_print", font_fix_print);
	if (p->font_var_print_points != old->font_var_print_points)
		SET (bool, "/font_variable_print_points", font_var_print_points);
	if (p->font_fix_print_points != old->font_fix_print_points)
		SET (bool, "/font_fixed_print_points", font_fix_print_points);
	if (p->font_var_size_print != old->font_var_size_print || p->font_var_print_points != old->font_var_print_points)
		SET (int, "/font_variable_size_print", font_var_size_print);
	if (p->font_fix_size_print != old->font_fix_size_print || p->font_fix_print_points != old->font_fix_print_points)
		SET (int, "/font_fixed_size_print", font_fix_size_print);
	if (strcmp (p->link_color, old->link_color))
		SET (string, "/link_color", link_color);
	if (strcmp (p->alink_color, old->alink_color))
		SET (string, "/alink_color", alink_color);
	if (strcmp (p->vlink_color, old->vlink_color))
		SET (string, "/vlink_color", vlink_color);
	
	
	if (p->live_spell_check != old->live_spell_check)
		SET (bool, "/live_spell_check", live_spell_check);

	gconf_client_suggest_sync (client, NULL);
}

#else

#undef GET
#define GET(t,v,s) \
	p->v = gnome_config_get_ ## t (s)
#define GETS(v,s) \
        g_free (p->v); \
        GET(string,v,s)

void
gtk_html_class_properties_load (GtkHTMLClassProperties *p)
{
	gchar *s, *var_name, *fix_name, *var_default, *fix_default;

	get_default_fonts (&var_name, &fix_name);
	var_default = g_strdup_printf ("font_variable=%s", var_name);
	fix_default = g_strdup_printf ("font_fixed=%s", fix_name);
	g_free (var_name);
	g_free (fix_name);

	gnome_config_push_prefix (GTK_HTML_GNOME_CONFIG_PREFIX);
	GET  (bool, magic_links, "magic_links=true");
	GET  (bool, magic_smileys, "magic_smileys=true");
	GET  (bool, animations, "animations=true");
	GETS (keybindings_theme, "keybindings_theme=ms");
	GETS (font_var, var_default);
	GETS (font_fix, fix_default);
	GETS (font_var_print, "font_variable_print=-*-helvetica-*-*-*-*-10-*-*-*-*-*-*-*");
	GETS (font_fix_print, "font_fixed_print=-*-courier-*-*-*-*-10-*-*-*-*-*-*-*");

	g_free (var_default);
	g_free (fix_default);

	s = g_strdup_printf ("font_variable_size=%d", DEFAULT_FONT_SIZE);
	GET  (int, font_var_size, s);
	g_free (s);

	s = g_strdup_printf ("font_fixed_size=%d", DEFAULT_FONT_SIZE);
	GET  (int, font_fix_size, s);
	g_free (s);

	s = g_strdup_printf ("font_variable_size_print=%d", DEFAULT_FONT_SIZE_PRINT);
	GET  (int, font_var_size_print, s);
	g_free (s);

	s = g_strdup_printf ("font_fixed_size_print=%d", DEFAULT_FONT_SIZE_PRINT);
	GET  (int, font_fix_size_print, s);
	g_free (s);

	GET  (bool, font_var_points, "font_variable_points=false");
	GET  (bool, font_fix_points, "font_fixed_points=false");
	GET  (bool, font_var_print_points, "font_variable_print_points=false");
	GET  (bool, font_fix_print_points, "font_fixed_print_points=false");
	
	GETS (link_color, "link_color=#0000ff");
	GETS (alink_color, "alink_color=#0000ff");
	GETS (vlink_color, "vlink_color=#0000ff");

	GET  (bool, live_spell_check, "live_spell_check=true");

	GET  (int, spell_error_color.red,   "spell_error_color_red=65535");
	GET  (int, spell_error_color.green, "spell_error_color_green=0");
	GET  (int, spell_error_color.blue,  "spell_error_color_blue=0");

	GETS (language, "language=en");

	/* printf ("fonts:\n%s\n%s\n", p->font_var, p->font_fix); */

	gnome_config_pop_prefix ();
}

void
gtk_html_class_properties_save (GtkHTMLClassProperties *p)
{
	gnome_config_push_prefix (GTK_HTML_GNOME_CONFIG_PREFIX);
	gnome_config_set_bool ("magic_links", p->magic_links);
	gnome_config_set_bool ("magic_smileys", p->magic_smileys);
	gnome_config_set_bool ("animations", p->animations);
	gnome_config_set_string ("keybindings_theme", p->keybindings_theme);
	gnome_config_set_string ("font_variable", p->font_var);
	gnome_config_set_string ("font_fixed", p->font_fix);
	gnome_config_set_int ("font_variable_size", p->font_var_size);
	gnome_config_set_int ("font_fixed_size", p->font_fix_size);
	gnome_config_set_string ("font_variable_print", p->font_var_print);
	gnome_config_set_string ("font_fixed_print", p->font_fix_print);
	gnome_config_set_int ("font_variable_size_print", p->font_var_size_print);
	gnome_config_set_int ("font_fixed_size_print", p->font_fix_size_print);
	gnome_config_set_bool ("font_variable_points", p->font_var_points);
	gnome_config_set_bool ("font_fixed_points", p->font_fix_points);
	gnome_config_set_bool ("font_variable_print_points", p->font_var_print_points);
	gnome_config_set_bool ("font_fixed_print_points", p->font_fix_print_points);
	gnome_config_set_string ("link_color", p->link_color);
	gnome_config_set_string ("alink_color", p->alink_color);
	gnome_config_set_string ("vlink_color", p->vlink_color);

	gnome_config_set_bool ("live_spell_check", p->live_spell_check);

	gnome_config_set_int ("spell_error_color_red",   p->spell_error_color.red);
	gnome_config_set_int ("spell_error_color_green", p->spell_error_color.green);
	gnome_config_set_int ("spell_error_color_blue",  p->spell_error_color.blue);

	gnome_config_set_string ("language", p->language);

	gnome_config_pop_prefix ();
	gnome_config_sync ();
}
#endif

static gchar *
get_font_name (const GdkFont * font)
{
	Atom font_atom, atom;
	Bool status;

	font_atom = gdk_atom_intern ("FONT", FALSE);

	if (font->type == GDK_FONT_FONTSET) {
		XFontStruct **font_structs;
		gint num_fonts;
		gchar **font_names;

		num_fonts = XFontsOfFontSet (GDK_FONT_XFONT (font), &font_structs, &font_names);
		status = XGetFontProperty (font_structs[0], font_atom, &atom);
	} else {
		status = XGetFontProperty (GDK_FONT_XFONT (font), font_atom, &atom);
	}

	if (status) {
		return gdk_atom_name (atom);
	}

	return NULL;
}

static void
get_default_fonts (gchar **var_name, gchar **fix_name)
{
	GtkStyle *style;
	char *font_name = NULL;

	style = gtk_widget_get_default_style ();
	if (style->font) {
		font_name = get_font_name (style->font);
	}

	if (font_name) {
		gchar *enc1, *enc2;

		enc1 = html_font_manager_get_attr (font_name, 13);
		enc2 = html_font_manager_get_attr (font_name, 14);

		*var_name = g_strdup_printf ("-*-helvetica-*-*-*-*-12-*-*-*-*-*-%s-%s", enc1, enc2);
		*fix_name = g_strdup_printf ("-*-courier-*-*-*-*-12-*-*-*-*-*-%s-%s", enc1, enc2);

		/* printf ("default encoding %s-%s\n%s\n%s\n", enc1, enc2, *var_name, *fix_name); */
		g_free (font_name);
		g_free (enc1);
		g_free (enc2);
	} else {
		*var_name = g_strdup ("-*-helvetica-*-*-*-*-12-*-*-*-*-*-*-*");
		*fix_name = g_strdup ("-*-courier-*-*-*-*-12-*-*-*-*-*-*-*");
	}
}

#define COPYS(v) \
        g_free (p1->v); \
        p1->v = g_strdup (p2->v);
#define COPY(v) \
        p1->v = p2->v;

void
gtk_html_class_properties_copy (GtkHTMLClassProperties *p1,
				GtkHTMLClassProperties *p2)
{
	COPY  (animations)
	COPY  (magic_links);
	COPY  (magic_smileys);
	COPYS (keybindings_theme);
	COPYS (font_var);
	COPYS (font_fix);
	COPY  (font_var_size);
	COPY  (font_fix_size);
	COPYS (font_var_print);
	COPYS (font_fix_print);
	COPY  (font_var_size_print);
	COPY  (font_fix_size_print);
	COPY  (font_var_points);
	COPY  (font_fix_points);
	COPY  (font_var_print_points);
	COPY  (font_fix_print_points);
	COPYS  (link_color);
	COPYS  (alink_color);
	COPYS  (vlink_color);

	COPY  (live_spell_check);
	COPY  (spell_error_color);
	COPYS (language);
}

/* enums */

static GtkEnumValue _gtk_html_cursor_skip_values[] = {
  { GTK_HTML_CURSOR_SKIP_ONE,  "GTK_HTML_CURSOR_SKIP_ONE",  "one" },
  { GTK_HTML_CURSOR_SKIP_WORD, "GTK_HTML_CURSOR_SKIP_WORD", "word" },
  { GTK_HTML_CURSOR_SKIP_PAGE, "GTK_HTML_CURSOR_SKIP_WORD", "page" },
  { GTK_HTML_CURSOR_SKIP_ALL,  "GTK_HTML_CURSOR_SKIP_ALL",  "all" },
  { 0, NULL, NULL }
};

GtkType
gtk_html_cursor_skip_get_type ()
{
	static GtkType cursor_skip_type = 0;

	if (!cursor_skip_type)
		cursor_skip_type = gtk_type_register_enum ("GTK_HTML_CURSOR_SKIP", _gtk_html_cursor_skip_values);

	return cursor_skip_type;
}

static GtkEnumValue _gtk_html_command_values[] = {
  { GTK_HTML_COMMAND_UNDO,  "GTK_HTML_COMMAND_UNDO",  "undo" },
  { GTK_HTML_COMMAND_REDO,  "GTK_HTML_COMMAND_REDO",  "redo" },
  { GTK_HTML_COMMAND_COPY,  "GTK_HTML_COMMAND_COPY",  "copy" },
  { GTK_HTML_COMMAND_COPY_AND_DISABLE_SELECTION,  "GTK_HTML_COMMAND_COPY_AND_DISABLE_SELECTION",  "copy-and-disable-selection" },
  { GTK_HTML_COMMAND_CUT,   "GTK_HTML_COMMAND_CUT",   "cut" },
  { GTK_HTML_COMMAND_PASTE, "GTK_HTML_COMMAND_PASTE", "paste" },
  { GTK_HTML_COMMAND_CUT_LINE, "GTK_HTML_COMMAND_CUT_LINE", "cut-line" },

  { GTK_HTML_COMMAND_INSERT_RULE, "GTK_HTML_COMMAND_INSERT_RULE", "insert-rule" },
  { GTK_HTML_COMMAND_INSERT_PARAGRAPH, "GTK_HTML_COMMAND_INSERT_PARAGRAPH", "insert-paragraph" },
  { GTK_HTML_COMMAND_INSERT_TAB, "GTK_HTML_COMMAND_INSERT_TAB", "insert-tab" },
  { GTK_HTML_COMMAND_INSERT_TAB_OR_NEXT_CELL,
    "GTK_HTML_COMMAND_INSERT_TAB_OR_NEXT_CELL", "insert-tab-or-next-cell" },
  { GTK_HTML_COMMAND_DELETE, "GTK_HTML_COMMAND_DELETE", "delete" },
  { GTK_HTML_COMMAND_DELETE_BACK, "GTK_HTML_COMMAND_DELETE_BACK", "delete-back" },
  { GTK_HTML_COMMAND_DELETE_BACK_OR_INDENT_DEC, "GTK_HTML_COMMAND_DELETE_BACK_OR_INDENT_DEC", "delete-back-or-indent-dec" },
  { GTK_HTML_COMMAND_SELECTION_MODE, "GTK_HTML_COMMAND_SELECTION_MODE", "selection-mode" },
  { GTK_HTML_COMMAND_DISABLE_SELECTION, "GTK_HTML_COMMAND_DISABLE_SELECTION", "disable-selection" },
  { GTK_HTML_COMMAND_BOLD_ON, "GTK_HTML_COMMAND_BOLD_ON", "bold-on" },
  { GTK_HTML_COMMAND_BOLD_OFF, "GTK_HTML_COMMAND_BOLD_OFF", "bold-off" },
  { GTK_HTML_COMMAND_BOLD_TOGGLE, "GTK_HTML_COMMAND_BOLD_TOGGLE", "bold-toggle" },
  { GTK_HTML_COMMAND_ITALIC_ON, "GTK_HTML_COMMAND_ITALIC_ON", "italic-on" },
  { GTK_HTML_COMMAND_ITALIC_OFF, "GTK_HTML_COMMAND_ITALIC_OFF", "italic-off" },
  { GTK_HTML_COMMAND_ITALIC_TOGGLE, "GTK_HTML_COMMAND_ITALIC_TOGGLE", "italic-toggle" },
  { GTK_HTML_COMMAND_UNDERLINE_ON, "GTK_HTML_COMMAND_UNDERLINE_ON", "underline-on" },
  { GTK_HTML_COMMAND_UNDERLINE_OFF, "GTK_HTML_COMMAND_UNDERLINE_OFF", "underline-off" },
  { GTK_HTML_COMMAND_UNDERLINE_TOGGLE, "GTK_HTML_COMMAND_UNDERLINE_TOGGLE", "underline-toggle" },
  { GTK_HTML_COMMAND_STRIKEOUT_ON, "GTK_HTML_COMMAND_STRIKEOUT_ON", "strikeout-on" },
  { GTK_HTML_COMMAND_STRIKEOUT_OFF, "GTK_HTML_COMMAND_STRIKEOUT_OFF", "strikeout-off" },
  { GTK_HTML_COMMAND_STRIKEOUT_TOGGLE, "GTK_HTML_COMMAND_STRIKEOUT_TOGGLE", "strikeout-toggle" },
  { GTK_HTML_COMMAND_SIZE_MINUS_2, "GTK_HTML_COMMAND_SIZE_MINUS_2", "size-minus-2" },
  { GTK_HTML_COMMAND_SIZE_MINUS_1, "GTK_HTML_COMMAND_SIZE_MINUS_1", "size-minus-1" },
  { GTK_HTML_COMMAND_SIZE_PLUS_0, "GTK_HTML_COMMAND_SIZE_PLUS_0", "size-plus-0" },
  { GTK_HTML_COMMAND_SIZE_PLUS_1, "GTK_HTML_COMMAND_SIZE_PLUS_1", "size-plus-1" },
  { GTK_HTML_COMMAND_SIZE_PLUS_2, "GTK_HTML_COMMAND_SIZE_PLUS_2", "size-plus-2" },
  { GTK_HTML_COMMAND_SIZE_PLUS_3, "GTK_HTML_COMMAND_SIZE_PLUS_3", "size-plus-3" },
  { GTK_HTML_COMMAND_SIZE_PLUS_4, "GTK_HTML_COMMAND_SIZE_PLUS_4", "size-plus-4" },
  { GTK_HTML_COMMAND_SIZE_INCREASE, "GTK_HTML_COMMAND_SIZE_INCREASE", "size-inc" },
  { GTK_HTML_COMMAND_SIZE_DECREASE, "GTK_HTML_COMMAND_SIZE_DECREASE", "size-dec" },
  { GTK_HTML_COMMAND_ALIGN_LEFT, "GTK_HTML_COMMAND_ALIGN_LEFT", "align-left" },
  { GTK_HTML_COMMAND_ALIGN_CENTER, "GTK_HTML_COMMAND_ALIGN_CENTER", "align-center" },
  { GTK_HTML_COMMAND_ALIGN_RIGHT, "GTK_HTML_COMMAND_ALIGN_RIGHT", "align-right" },
  { GTK_HTML_COMMAND_INDENT_ZERO, "GTK_HTML_COMMAND_INDENT_ZERO", "indent-zero" },
  { GTK_HTML_COMMAND_INDENT_INC, "GTK_HTML_COMMAND_INDENT_INC", "indent-more" },
  { GTK_HTML_COMMAND_INDENT_INC_OR_NEXT_CELL, "GTK_HTML_COMMAND_INDENT_INC_OR_NEXT_CELL", "indent-more-or-next-cell" },
  { GTK_HTML_COMMAND_INDENT_DEC, "GTK_HTML_COMMAND_INDENT_DEC", "indent-less" },
  { GTK_HTML_COMMAND_PREV_CELL, "GTK_HTML_COMMAND_PREV_CELL", "prev-cell" },
  { GTK_HTML_COMMAND_INDENT_PARAGRAPH, "GTK_HTML_COMMAND_INDENT_PARAGRAPH", "indent-paragraph" },
  { GTK_HTML_COMMAND_BREAK_AND_FILL_LINE, "GTK_HTML_COMMAND_BREAK_AND_FILL_LINE", "break-and-fill" },
  { GTK_HTML_COMMAND_SPACE_AND_FILL_LINE, "GTK_HTML_COMMAND_SPACE_AND_FILL_LINE", "space-and-fill" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_NORMAL, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_NORMAL", "style-normal" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_H1, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_H1", "style-header1" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_H2, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_H2", "style-header2" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_H3, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_H3", "style-header3" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_H4, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_H4", "style-header4" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_H5, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_H5", "style-header5" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_H6, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_H6", "style-header6" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_ADDRESS, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_ADDRESS", "style-address" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_PRE, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_PRE", "style-pre" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMDOTTED, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMDOTTED", "style-itemdot" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMROMAN, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMROMAN", "style-itemroman" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMDIGIT, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMDIGIT", "style-itemdigit" },
  { GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMALPHA, "GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMALPHA", "style-itemalpha" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_UP, "GTK_HTML_COMMAND_MODIFY_SELECTION_UP", "selection-move-up" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_DOWN, "GTK_HTML_COMMAND_MODIFY_SELECTION_DOWN", "selection-move-down" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_LEFT, "GTK_HTML_COMMAND_MODIFY_SELECTION_LEFT", "selection-move-left" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_RIGHT, "GTK_HTML_COMMAND_MODIFY_SELECTION_RIGHT", "selection-move-right" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_BOL, "GTK_HTML_COMMAND_MODIFY_SELECTION_BOL", "selection-move-bol" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_EOL, "GTK_HTML_COMMAND_MODIFY_SELECTION_EOL", "selection-move-eol" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_BOD, "GTK_HTML_COMMAND_MODIFY_SELECTION_BOD", "selection-move-bod" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_EOD, "GTK_HTML_COMMAND_MODIFY_SELECTION_EOD", "selection-move-eod" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEUP, "GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEUP", "selection-move-pageup" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEDOWN, "GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEDOWN", "selection-move-pagedown" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_PREV_WORD, "GTK_HTML_COMMAND_MODIFY_SELECTION_PREV_WORD", "selection-move-prev-word" },
  { GTK_HTML_COMMAND_MODIFY_SELECTION_NEXT_WORD, "GTK_HTML_COMMAND_MODIFY_SELECTION_NEXT_WORD", "selection-move-next-word" },
  { GTK_HTML_COMMAND_SELECT_WORD, "GTK_HTML_COMMAND_SELECT_WORD", "select-word" },
  { GTK_HTML_COMMAND_SELECT_LINE, "GTK_HTML_COMMAND_SELECT_LINE", "select-line" },
  { GTK_HTML_COMMAND_SELECT_PARAGRAPH, "GTK_HTML_COMMAND_SELECT_PARAGRAPH", "select-paragraph" },
  { GTK_HTML_COMMAND_SELECT_PARAGRAPH_EXTENDED, "GTK_HTML_COMMAND_SELECT_PARAGRAPH_EXTENDED", "select-paragraph-extended" },
  { GTK_HTML_COMMAND_SELECT_ALL, "GTK_HTML_COMMAND_SELECT_ALL", "select-all" },
  { GTK_HTML_COMMAND_CURSOR_POSITION_SAVE, "GTK_HTML_COMMAND_CURSOR_POSITION_SAVE", "cursor-position-save" },
  { GTK_HTML_COMMAND_CURSOR_POSITION_RESTORE, "GTK_HTML_COMMAND_CURSOR_POSITION_RESTORE", "cursor-position-restore" },
  { GTK_HTML_COMMAND_CAPITALIZE_WORD, "GTK_HTML_COMMAND_CAPITALIZE_WORD", "capitalize-word" },
  { GTK_HTML_COMMAND_UPCASE_WORD, "GTK_HTML_COMMAND_UPCASE_WORD", "upcase-word" },
  { GTK_HTML_COMMAND_DOWNCASE_WORD, "GTK_HTML_COMMAND_DOWNCASE_WORD", "downcase-word" },
  { GTK_HTML_COMMAND_SPELL_SUGGEST, "GTK_HTML_COMMAND_SPELL_SUGGEST", "spell-suggest" },
  { GTK_HTML_COMMAND_SPELL_PERSONAL_DICTIONARY_ADD, "GTK_HTML_COMMAND_SPELL_PERSONAL_DICTIONARY_ADD", "spell-personal-add" },
  { GTK_HTML_COMMAND_SPELL_SESSION_DICTIONARY_ADD, "GTK_HTML_COMMAND_SPELL_SESSION_DICTIONARY_ADD", "spell-session-add" },
  { GTK_HTML_COMMAND_SEARCH_INCREMENTAL_FORWARD, "GTK_HTML_COMMAND_SEARCH_INCREMENTAL_FORWARD", "isearch-forward" },
  { GTK_HTML_COMMAND_SEARCH_INCREMENTAL_BACKWARD, "GTK_HTML_COMMAND_SEARCH_INCREMENTAL_BACKWARD", "isearch-backward" },
  { GTK_HTML_COMMAND_SEARCH, "GTK_HTML_COMMAND_SEARCH", "search" },
  { GTK_HTML_COMMAND_SEARCH_REGEX, "GTK_HTML_COMMAND_SEARCH_REGEX", "search-regex" },
  { GTK_HTML_COMMAND_FOCUS_FORWARD, "GTK_HTML_COMMAND_FOCUS_FORWARD", "focus-forward" },
  { GTK_HTML_COMMAND_FOCUS_BACKWARD, "GTK_HTML_COMMAND_FOCUS_BACKWARD", "focus-backward" },
  { GTK_HTML_COMMAND_POPUP_MENU, "GTK_HTML_COMMAND_POPUP_MENU", "popup-menu" },
  { GTK_HTML_COMMAND_PROPERTIES_DIALOG, "GTK_HTML_COMMAND_PROPERTIES_DIALOG", "property-dialog" },
  { GTK_HTML_COMMAND_CURSOR_FORWARD, "GTK_HTML_COMMAND_CURSOR_FORWARD", "cursor-forward" },
  { GTK_HTML_COMMAND_CURSOR_BACKWARD, "GTK_HTML_COMMAND_CURSOR_BACKWARD", "cursor-backward" },
  { GTK_HTML_COMMAND_INSERT_TABLE_1_1, "GTK_HTML_COMMAND_INSERT_TABLE_1_1", "insert-table-1-1" },
  { GTK_HTML_COMMAND_TABLE_INSERT_COL_AFTER, "GTK_HTML_COMMAND_TABLE_INSERT_COL_AFTER", "insert-col-after" },
  { GTK_HTML_COMMAND_TABLE_INSERT_COL_BEFORE, "GTK_HTML_COMMAND_TABLE_INSERT_COL_BEFORE", "insert-col-before" },
  { GTK_HTML_COMMAND_TABLE_INSERT_ROW_AFTER, "GTK_HTML_COMMAND_TABLE_INSERT_ROW_AFTER", "insert-row-after" },
  { GTK_HTML_COMMAND_TABLE_INSERT_ROW_BEFORE, "GTK_HTML_COMMAND_TABLE_INSERT_ROW_BEFORE", "insert-row-before" },
  { GTK_HTML_COMMAND_TABLE_DELETE_COL, "GTK_HTML_COMMAND_TABLE_DELETE_COL", "delete-col" },
  { GTK_HTML_COMMAND_TABLE_DELETE_ROW, "GTK_HTML_COMMAND_TABLE_DELETE_ROW", "delete-row" },
  { GTK_HTML_COMMAND_TABLE_CELL_INC_CSPAN, "GTK_HTML_COMMAND_TABLE_CELL_INC_CSPAN", "inc-cspan" },
  { GTK_HTML_COMMAND_TABLE_CELL_DEC_CSPAN, "GTK_HTML_COMMAND_TABLE_CELL_DEC_CSPAN", "dec-cspan" },
  { GTK_HTML_COMMAND_TABLE_CELL_INC_RSPAN, "GTK_HTML_COMMAND_TABLE_CELL_INC_RSPAN", "inc-rspan" },
  { GTK_HTML_COMMAND_TABLE_CELL_DEC_RSPAN, "GTK_HTML_COMMAND_TABLE_CELL_DEC_RSPAN", "dec-rspan" },
  { GTK_HTML_COMMAND_TABLE_CELL_JOIN_LEFT, "GTK_HTML_COMMAND_TABLE_CELL_JOIN_LEFT", "cell-join-left" },
  { GTK_HTML_COMMAND_TABLE_CELL_JOIN_RIGHT, "GTK_HTML_COMMAND_TABLE_CELL_JOIN_RIGHT", "cell-join-right" },
  { GTK_HTML_COMMAND_TABLE_CELL_JOIN_UP, "GTK_HTML_COMMAND_TABLE_CELL_JOIN_UP", "cell-join-up" },
  { GTK_HTML_COMMAND_TABLE_CELL_JOIN_DOWN, "GTK_HTML_COMMAND_TABLE_CELL_JOIN_DOWN", "cell-join-down" },
  { GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_INC, "GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_INC", "inc-border" },
  { GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_DEC, "GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_DEC", "dec-border" },
  { GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_ZERO, "GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_ZERO", "zero-border" },
  { GTK_HTML_COMMAND_TEXT_SET_DEFAULT_COLOR, "GTK_HTML_COMMAND_TEXT_SET_DEFAULT_COLOR", "text-default-color" },
  { GTK_HTML_COMMAND_CURSOR_BOD, "GTK_HTML_COMMAND_CURSOR_BOD", "cursor-bod" },
  { GTK_HTML_COMMAND_CURSOR_EOD, "GTK_HTML_COMMAND_CURSOR_EOD", "cursor-eod" },
  { GTK_HTML_COMMAND_BLOCK_REDRAW, "GTK_HTML_COMMAND_BLOCK_REDRAW", "block-redraw" },
  { GTK_HTML_COMMAND_UNBLOCK_REDRAW, "GTK_HTML_COMMAND_UNBLOCK_REDRAW", "unblock-redraw" },
  { GTK_HTML_COMMAND_ZOOM_IN, "GTK_HTML_COMMAND_ZOOM_IN", "zoom-in" },
  { GTK_HTML_COMMAND_ZOOM_OUT, "GTK_HTML_COMMAND_ZOOM_IN", "zoom-out" },
  { GTK_HTML_COMMAND_ZOOM_RESET, "GTK_HTML_COMMAND_ZOOM_RESET", "zoom-reset" },
  { GTK_HTML_COMMAND_TABLE_SPACING_INC, "GTK_HTML_COMMAND_TABLE_SPACING_INC", "inc-spacing" },
  { GTK_HTML_COMMAND_TABLE_SPACING_DEC, "GTK_HTML_COMMAND_TABLE_SPACING_DEC", "dec-spacing" },
  { GTK_HTML_COMMAND_TABLE_SPACING_ZERO, "GTK_HTML_COMMAND_TABLE_SPACING_ZERO", "zero-spacing" },
  { GTK_HTML_COMMAND_TABLE_PADDING_INC, "GTK_HTML_COMMAND_TABLE_PADDING_INC", "inc-padding" },
  { GTK_HTML_COMMAND_TABLE_PADDING_DEC, "GTK_HTML_COMMAND_TABLE_PADDING_DEC", "dec-padding" },
  { GTK_HTML_COMMAND_TABLE_PADDING_ZERO, "GTK_HTML_COMMAND_TABLE_PADDING_ZERO", "zero-padding" },
  { GTK_HTML_COMMAND_DELETE_TABLE, "GTK_HTML_COMMAND_DELETE_TABLE", "delete-table" },
  { GTK_HTML_COMMAND_DELETE_TABLE_ROW, "GTK_HTML_COMMAND_DELETE_TABLE_ROW", "delete-table-row" },
  { GTK_HTML_COMMAND_DELETE_TABLE_COLUMN, "GTK_HTML_COMMAND_DELETE_TABLE_COLUMN", "delete-table-column" },
  { GTK_HTML_COMMAND_DELETE_TABLE_CELL_CONTENTS, "GTK_HTML_COMMAND_DELETE_TABLE_CELL_CONTENTS", "delete-cell-contents" },
  { GTK_HTML_COMMAND_GRAB_FOCUS, "GTK_HTML_COMMAND_GRAB_FOCUS", "grab-focus" },
  { GTK_HTML_COMMAND_KILL_WORD, "GTK_HTML_COMMAND_KILL_WORD", "kill-word" },
  { GTK_HTML_COMMAND_KILL_WORD_BACKWARD, "GTK_HTML_COMMAND_KILL_WORD_BACKWARD", "backward-kill-word" },
  { GTK_HTML_COMMAND_TEXT_COLOR_APPLY, "GTK_HTML_COMMAND_TEXT_COLOR_APPLY", "text-color-apply" },
  { GTK_HTML_COMMAND_SAVE_DATA_ON, "GTK_HTML_COMMAND_SAVE_DATA_ON", "save-data-on" },
  { GTK_HTML_COMMAND_SAVE_DATA_OFF, "GTK_HTML_COMMAND_SAVE_DATA_OFF", "save-data-off" },
  { GTK_HTML_COMMAND_SAVED, "GTK_HTML_COMMAND_SAVED", "saved" },
  { GTK_HTML_COMMAND_IS_SAVED, "GTK_HTML_COMMAND_IS_SAVED", "is-saved" },
  { GTK_HTML_COMMAND_SCROLL_BOD, "GTK_HTML_COMMAND_SCROLL_BOD", "scroll-bod" },
  { GTK_HTML_COMMAND_SCROLL_EOD, "GTK_HTML_COMMAND_SCROLL_EOD", "scroll-eod" },
  { 0, NULL, NULL }
};

GtkType
gtk_html_command_get_type ()
{
	static GtkType command_type = 0;

	if (!command_type)
		command_type = gtk_type_register_enum ("GTK_HTML_COMMAND", _gtk_html_command_values);

	return command_type;
}
