/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

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

    Author: Ettore Perazzoli <ettore@helixcode.com>
*/

/* FIXME: Should use BonoboUIHandler.  */

#include <config.h>
#include <libgnome/gnome-i18n.h>
#include <gnome.h>
#include <bonobo.h>

#include "gi-color-combo.h"
#include "toolbar.h"
#include "utils.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlengine-edit-fontstyle.h"
#include "htmlsettings.h"

#define EDITOR_TOOLBAR_PATH "/HTMLEditor"


/* Paragraph style option menu.  */

static struct {
	GtkHTMLParagraphStyle style;
	const gchar *description;
} paragraph_style_items[] = {
	{ GTK_HTML_PARAGRAPH_STYLE_NORMAL, N_("Normal") },
	{ GTK_HTML_PARAGRAPH_STYLE_PRE, N_("Preformat") },
	{ GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED, N_("Bulleted List") },
	{ GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT, N_("Numbered List") },
	{ GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN, N_("Roman List") },
	{ GTK_HTML_PARAGRAPH_STYLE_ITEMALPHA, N_("Alphabetical List") },
	{ GTK_HTML_PARAGRAPH_STYLE_H1, N_("Header 1") },
	{ GTK_HTML_PARAGRAPH_STYLE_H2, N_("Header 2") },
	{ GTK_HTML_PARAGRAPH_STYLE_H3, N_("Header 3") },
	{ GTK_HTML_PARAGRAPH_STYLE_H4, N_("Header 4") },
	{ GTK_HTML_PARAGRAPH_STYLE_H5, N_("Header 5") },
	{ GTK_HTML_PARAGRAPH_STYLE_H6, N_("Header 6") },
	{ GTK_HTML_PARAGRAPH_STYLE_ADDRESS, N_("Address") },
	{ GTK_HTML_PARAGRAPH_STYLE_NORMAL, NULL },
};

static void
paragraph_style_changed_cb (GtkHTML *html,
			    GtkHTMLParagraphStyle style,
			    gpointer data)
{
	GtkOptionMenu *option_menu;
	guint i;

	option_menu = GTK_OPTION_MENU (data);

	for (i = 0; paragraph_style_items[i].description != NULL; i++) {
		if (paragraph_style_items[i].style == style) {
			gtk_option_menu_set_history (option_menu, i);
			return;
		}
	}

	g_warning ("Editor component toolbar: unknown paragraph style %d", style);
}

static void
paragraph_style_menu_item_activated_cb (GtkWidget *widget,
					gpointer data)
{
	GtkHTMLParagraphStyle style;
	GtkHTML *html;

	html = GTK_HTML (data);
	style = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "paragraph_style_value"));

	/* g_warning ("Setting paragraph style to %d.", style); */

	gtk_html_set_paragraph_style (html, style);
}

static void
paragraph_style_menu_item_update (GtkWidget *widget, gpointer format_html)
{
	GtkHTMLParagraphStyle style;
	gint sensitive;

	style = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "paragraph_style_value"));
	
	sensitive = (format_html
		     || style == GTK_HTML_PARAGRAPH_STYLE_NORMAL
		     || style == GTK_HTML_PARAGRAPH_STYLE_PRE
		     || style == GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED
		     || style == GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN
		     || style == GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT
		     || style == GTK_HTML_PARAGRAPH_STYLE_ITEMALPHA
		     );

	gtk_widget_set_sensitive (widget, sensitive);	
}

static void
paragraph_style_option_menu_set_mode (GtkWidget *option_menu, gboolean format_html)
{
	GtkWidget *menu;
	
	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (option_menu));
	gtk_container_forall (GTK_CONTAINER (menu), 
			      paragraph_style_menu_item_update, 
			      GINT_TO_POINTER (format_html));
}

static GtkWidget *
setup_paragraph_style_option_menu (GtkHTML *html)
{
	GtkWidget *option_menu;
	GtkWidget *menu;
	guint i;

	option_menu = gtk_option_menu_new ();
	menu = gtk_menu_new ();

	for (i = 0; paragraph_style_items[i].description != NULL; i++) {
		GtkWidget *menu_item;

		menu_item = gtk_menu_item_new_with_label (_(paragraph_style_items[i].description));
		gtk_widget_show (menu_item);

		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		g_object_set_data (G_OBJECT (menu_item), "paragraph_style_value",
				     GINT_TO_POINTER (paragraph_style_items[i].style));
		g_signal_connect (menu_item, "activate", G_CALLBACK (paragraph_style_menu_item_activated_cb), html);
	}

	gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
	g_signal_connect (html, "current_paragraph_style_changed", G_CALLBACK (paragraph_style_changed_cb), option_menu);
	gtk_widget_show (option_menu);

	return option_menu;
}

static void
set_font_size (GtkWidget *w, GtkHTMLControlData *cd)
{
	GtkHTMLFontStyle style = GTK_HTML_FONT_STYLE_SIZE_1 + GPOINTER_TO_INT (g_object_get_data (G_OBJECT (w),
												    "size"));

	if (!cd->block_font_style_change)
		gtk_html_set_font_style (cd->html, GTK_HTML_FONT_STYLE_MAX & ~GTK_HTML_FONT_STYLE_SIZE_MASK, style);
}

static void
font_size_changed (GtkWidget *w, GtkHTMLParagraphStyle style, GtkHTMLControlData *cd)
{
	if (style == GTK_HTML_FONT_STYLE_DEFAULT)
		style = GTK_HTML_FONT_STYLE_SIZE_3;
	cd->block_font_style_change++;
	gtk_option_menu_set_history (GTK_OPTION_MENU (cd->font_size_menu),
				     (style & GTK_HTML_FONT_STYLE_SIZE_MASK) - GTK_HTML_FONT_STYLE_SIZE_1);
	cd->block_font_style_change--;
}

static GtkWidget *
setup_font_size_option_menu (GtkHTMLControlData *cd)
{
	GtkWidget *option_menu;
	GtkWidget *menu;
	guint i;
	gchar size [3];

	cd->font_size_menu = option_menu = gtk_option_menu_new ();

	menu = gtk_menu_new ();
	size [2] = 0;

	for (i = 0; i < GTK_HTML_FONT_STYLE_SIZE_MAX; i++) {
		GtkWidget *menu_item;

		size [0] = (i>1) ? '+' : '-';
		size [1] = '0' + ((i>1) ? i - 2 : 2 - i);

		menu_item = gtk_menu_item_new_with_label (size);
		gtk_widget_show (menu_item);

		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		g_object_set_data (G_OBJECT (menu_item), "size",
				     GINT_TO_POINTER (i));
		g_signal_connect (menu_item, "activate", G_CALLBACK (set_font_size), cd);
	}

	gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), 2);

	g_signal_connect (cd->html, "insertion_font_style_changed", G_CALLBACK (font_size_changed), cd);

	gtk_widget_show (option_menu);

	return option_menu;
}

static void
apply_color (GdkColor *gdk_color, GtkHTMLControlData *cd)
{
	HTMLColor *color;
	
	color = gdk_color
		&& gdk_color != &html_colorset_get_color (cd->html->engine->settings->color_set, HTMLTextColor)->color
		? html_color_new_from_gdk_color (gdk_color) : NULL;

	gtk_html_set_color (cd->html, color);
	if (color)
		html_color_unref (color);
}

void
toolbar_apply_color (GtkHTMLControlData *cd)
{
	GdkColor *color;
	gboolean default_color;

	color = color_combo_get_color (COLOR_COMBO (cd->combo), &default_color);
	apply_color (color, cd);
	if (color)
		gdk_color_free (color);
}

static void
color_changed (GtkWidget *w, GdkColor *gdk_color, gboolean custom, gboolean by_user, gboolean is_default,
	       GtkHTMLControlData *cd)
{
	/* If the color was changed programatically there's not need to set things */
	if (!by_user)
		return;
	apply_color (gdk_color, cd);
}

static void
unset_focus (GtkWidget *w, gpointer data)
{
	GTK_WIDGET_UNSET_FLAGS (w, GTK_CAN_FOCUS);
	if (GTK_IS_CONTAINER (w))
		gtk_container_forall (GTK_CONTAINER (w), unset_focus, NULL);
}

static void
set_color_combo (GtkHTML *html, GtkHTMLControlData *cd)
{
	color_combo_set_color (COLOR_COMBO (cd->combo),
			       &html_colorset_get_color_allocated (html->engine->settings->color_set,
								   html->engine->painter, HTMLTextColor)->color);
}

static void
realize_engine (GtkHTML *html, GtkHTMLControlData *cd)
{
	set_color_combo (html, cd);
	g_signal_handlers_disconnect_matched (html, G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
					      0, 0, NULL, G_CALLBACK (realize_engine), cd);
}

static void
load_done (GtkHTML *html, GtkHTMLControlData *cd)
{
	if (GTK_WIDGET_REALIZED (cd->html))
		set_color_combo (html, cd);
	else
		g_signal_connect (cd->html, "realize", G_CALLBACK (realize_engine), cd);
}

static GtkWidget *
setup_color_combo (GtkHTMLControlData *cd)
{
	HTMLColor *color;

	color = html_colorset_get_color (cd->html->engine->settings->color_set, HTMLTextColor);
	if (GTK_WIDGET_REALIZED (cd->html))
		html_color_alloc (color, cd->html->engine->painter);
	else
		g_signal_connect (cd->html, "realize", G_CALLBACK (realize_engine), cd);
        g_signal_connect (cd->html, "load_done", G_CALLBACK (load_done), cd);

	cd->combo = color_combo_new (NULL, _("Automatic"), &color->color, color_group_fetch ("toolbar_text", cd));
        g_signal_connect (cd->combo, "color_changed", G_CALLBACK (color_changed), cd);

	gtk_widget_show_all (cd->combo);
	return cd->combo;
}


/* Font style group.  */

static void
editor_toolbar_tt_cb (GtkWidget *widget, GtkHTMLControlData *cd)
{
	if (!cd->block_font_style_change) {
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
			gtk_html_set_font_style (GTK_HTML (cd->html),
						 GTK_HTML_FONT_STYLE_MAX,
						 GTK_HTML_FONT_STYLE_FIXED);
		else
			gtk_html_set_font_style (GTK_HTML (cd->html), ~GTK_HTML_FONT_STYLE_FIXED, 0);
	}
}

static void
editor_toolbar_bold_cb (GtkWidget *widget, GtkHTMLControlData *cd)
{
	if (!cd->block_font_style_change) {
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
			gtk_html_set_font_style (GTK_HTML (cd->html),
						 GTK_HTML_FONT_STYLE_MAX,
						 GTK_HTML_FONT_STYLE_BOLD);
		else
			gtk_html_set_font_style (GTK_HTML (cd->html), ~GTK_HTML_FONT_STYLE_BOLD, 0);
	}
}

static void
editor_toolbar_italic_cb (GtkWidget *widget, GtkHTMLControlData *cd)
{
	if (!cd->block_font_style_change) {
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
			gtk_html_set_font_style (GTK_HTML (cd->html),
						 GTK_HTML_FONT_STYLE_MAX,
						 GTK_HTML_FONT_STYLE_ITALIC);
		else
			gtk_html_set_font_style (GTK_HTML (cd->html), ~GTK_HTML_FONT_STYLE_ITALIC, 0);
	}
}

static void
editor_toolbar_underline_cb (GtkWidget *widget, GtkHTMLControlData *cd)
{
	if (!cd->block_font_style_change) {
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
			gtk_html_set_font_style (GTK_HTML (cd->html),
						 GTK_HTML_FONT_STYLE_MAX,
						 GTK_HTML_FONT_STYLE_UNDERLINE);
		else
			gtk_html_set_font_style (GTK_HTML (cd->html), ~GTK_HTML_FONT_STYLE_UNDERLINE, 0);
	}
}

static void
editor_toolbar_strikeout_cb (GtkWidget *widget, GtkHTMLControlData *cd)
{
	if (!cd->block_font_style_change) {
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
			gtk_html_set_font_style (GTK_HTML (cd->html),
						 GTK_HTML_FONT_STYLE_MAX,
						 GTK_HTML_FONT_STYLE_STRIKEOUT);
		else
			gtk_html_set_font_style (GTK_HTML (cd->html), ~GTK_HTML_FONT_STYLE_STRIKEOUT, 0);
	}
}

static void
insertion_font_style_changed_cb (GtkHTML *widget, GtkHTMLFontStyle font_style, GtkHTMLControlData *cd)
{
	cd->block_font_style_change++;

	if (font_style & GTK_HTML_FONT_STYLE_FIXED)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->tt_button), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->tt_button), FALSE);

	if (font_style & GTK_HTML_FONT_STYLE_BOLD)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->bold_button), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->bold_button), FALSE);

	if (font_style & GTK_HTML_FONT_STYLE_ITALIC)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->italic_button), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->italic_button), FALSE);

	if (font_style & GTK_HTML_FONT_STYLE_UNDERLINE)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->underline_button), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->underline_button), FALSE);

	if (font_style & GTK_HTML_FONT_STYLE_STRIKEOUT)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->strikeout_button), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cd->strikeout_button), FALSE);

	cd->block_font_style_change--;
}


/* Alignment group.  */

static void
editor_toolbar_left_align_cb (GtkWidget *widget,
			      GtkHTMLControlData *cd)
{
	/* If the button is not active at this point, it means that the user clicked on
           some other button in the radio group.  */
	if (! gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		return;

	gtk_html_set_paragraph_alignment (GTK_HTML (cd->html),
					  GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT);
}

static void
editor_toolbar_center_cb (GtkWidget *widget,
			  GtkHTMLControlData *cd)
{
	/* If the button is not active at this point, it means that the user clicked on
           some other button in the radio group.  */
	if (! gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		return;

	gtk_html_set_paragraph_alignment (GTK_HTML (cd->html),
					  GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER);
}

static void
editor_toolbar_right_align_cb (GtkWidget *widget,
			       GtkHTMLControlData *cd)
{
	/* If the button is not active at this point, it means that the user clicked on
           some other button in the radio group.  */
	if (! gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		return;

	gtk_html_set_paragraph_alignment (GTK_HTML (cd->html),
					  GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT);
}

static void
safe_set_active (GtkWidget *widget,
		 gpointer data)
{
	GtkObject *object;
	GtkToggleButton *toggle_button;

	object = GTK_OBJECT (widget);
	toggle_button = GTK_TOGGLE_BUTTON (widget);

	g_signal_handlers_block_matched (object, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, data);
	gtk_toggle_button_set_active (toggle_button, TRUE);
	g_signal_handlers_unblock_matched (object, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, data);
}

static void
paragraph_alignment_changed_cb (GtkHTML *widget,
				GtkHTMLParagraphAlignment alignment,
				GtkHTMLControlData *cd)
{
	switch (alignment) {
	case GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT:
		safe_set_active (cd->left_align_button, cd);
		break;
	case GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER:
		safe_set_active (cd->center_button, cd);
		break;
	case GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT:
		safe_set_active (cd->right_align_button, cd);
		break;
	default:
		g_warning ("Unknown GtkHTMLParagraphAlignment %d.", alignment);
	}
}


/* Indentation group.  */

static void

editor_toolbar_indent_cb (GtkWidget *widget,
			  GtkHTMLControlData *cd)
{
	gtk_html_indent_push_level (GTK_HTML (cd->html), HTML_LIST_TYPE_BLOCKQUOTE);
}

static void
editor_toolbar_unindent_cb (GtkWidget *widget,
			    GtkHTMLControlData *cd)
{
	gtk_html_indent_pop_level (GTK_HTML (cd->html));
}


/* Editor toolbar.  */

enum EditorAlignmentButtons {
	EDITOR_ALIGNMENT_LEFT,
	EDITOR_ALIGNMENT_CENTER,
	EDITOR_ALIGNMENT_RIGHT
};
static GnomeUIInfo editor_toolbar_alignment_group[] = {
	{ GNOME_APP_UI_ITEM, N_("Left align"), N_("Left justifies the paragraphs"),
	  editor_toolbar_left_align_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },
	{ GNOME_APP_UI_ITEM, N_("Center"), N_("Center justifies the paragraphs"),
	  editor_toolbar_center_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },
	{ GNOME_APP_UI_ITEM, N_("Right align"), N_("Right justifies the paragraphs"),
	  editor_toolbar_right_align_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },
	GNOMEUIINFO_END
};

enum EditorToolbarButtons {
	EDITOR_TOOLBAR_TT,
	EDITOR_TOOLBAR_BOLD,
	EDITOR_TOOLBAR_ITALIC,
	EDITOR_TOOLBAR_UNDERLINE,
	EDITOR_TOOLBAR_STRIKEOUT,
	EDITOR_TOOLBAR_SEP1,
	EDITOR_TOOLBAR_ALIGNMENT,
	EDITOR_TOOLBAR_SEP2,
	EDITOR_TOOLBAR_UNINDENT,
	EDITOR_TOOLBAR_INDENT
};

static GnomeUIInfo editor_toolbar_style_uiinfo[] = {

	{ GNOME_APP_UI_TOGGLEITEM, N_("Typewriter"), N_("Toggle typewriter font style"),
	  editor_toolbar_tt_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },
	{ GNOME_APP_UI_TOGGLEITEM, N_("Bold"), N_("Makes the text bold"),
	  editor_toolbar_bold_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },
	{ GNOME_APP_UI_TOGGLEITEM, N_("Italic"), N_("Makes the text italic"),
	  editor_toolbar_italic_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },
	{ GNOME_APP_UI_TOGGLEITEM, N_("Underline"), N_("Underlines the text"),
	  editor_toolbar_underline_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },
	{ GNOME_APP_UI_TOGGLEITEM, N_("Strikeout"), N_("Strikes out the text"),
	  editor_toolbar_strikeout_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_RADIOLIST (editor_toolbar_alignment_group),

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, N_("Unindent"), N_("Indents the paragraphs less"),
	  editor_toolbar_unindent_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },
	{ GNOME_APP_UI_ITEM, N_("Indent"), N_("Indents the paragraphs more"),
	  editor_toolbar_indent_cb, NULL, NULL, GNOME_APP_PIXMAP_FILENAME },

	GNOMEUIINFO_END
};

/* static void
toolbar_destroy_cb (GtkObject *object,
		    GtkHTMLControlData *cd)
{
	if (cd->html)
		gtk_signal_disconnect (GTK_OBJECT (cd->html),
				       cd->font_style_changed_connection_id);
}

static void
html_destroy_cb (GtkObject *object,
		 GtkHTMLControlData *cd)
{
	cd->html = NULL;
} */

static void
indentation_changed (GtkWidget *w, guint level, GtkHTMLControlData *cd)
{
	gtk_widget_set_sensitive (cd->unindent_button, level != 0);
}

static GtkWidget *
create_style_toolbar (GtkHTMLControlData *cd)
{
	GtkWidget *hbox;
	gchar *domain;
	
	hbox = gtk_hbox_new (FALSE, 0);

	cd->toolbar_style = gtk_toolbar_new ();
	gtk_box_pack_start (GTK_BOX (hbox), cd->toolbar_style, TRUE, TRUE, 0);

	cd->paragraph_option = setup_paragraph_style_option_menu (cd->html);
	gtk_toolbar_prepend_space (GTK_TOOLBAR (cd->toolbar_style));
	gtk_toolbar_prepend_widget (GTK_TOOLBAR (cd->toolbar_style),
				    cd->paragraph_option,
				    NULL, NULL);

	gtk_toolbar_prepend_space (GTK_TOOLBAR (cd->toolbar_style));
	gtk_toolbar_prepend_widget (GTK_TOOLBAR (cd->toolbar_style),
				    setup_font_size_option_menu (cd),
				    NULL, NULL);

	/* 
	 * FIXME: steal textdomain temporarily from main-process,  and set it to 
	 * GETTEXT_PACKAGE, after create the widgets, restore it.
	 */
	domain = g_strdup (textdomain (NULL));
	textdomain (GETTEXT_PACKAGE);
	
	editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_TT].pixmap_info = GTKHTML_DATADIR "/icons/font-tt-24.png";
	editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_BOLD].pixmap_info = gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text_bold", 24, NULL, NULL);
	editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_ITALIC].pixmap_info = gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text_italic", 24, NULL, NULL);
	editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_UNDERLINE].pixmap_info = gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text_underlined", 24, NULL, NULL);
	editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_STRIKEOUT].pixmap_info = gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text-strikethrough", 24, NULL, NULL);
	editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_UNINDENT].pixmap_info = gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text_unindent", 24, NULL, NULL);
	editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_INDENT].pixmap_info = gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text_indent", 24, NULL, NULL);

	((GnomeUIInfo *) editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_ALIGNMENT].moreinfo) [EDITOR_ALIGNMENT_LEFT].pixmap_info
		= gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text_left", 24, NULL, NULL);
	((GnomeUIInfo *) editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_ALIGNMENT].moreinfo) [EDITOR_ALIGNMENT_CENTER].pixmap_info
		= gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text_center", 24, NULL, NULL);
	((GnomeUIInfo *) editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_ALIGNMENT].moreinfo) [EDITOR_ALIGNMENT_RIGHT].pixmap_info
		= gnome_icon_theme_lookup_icon (cd->icon_theme, "stock_text_right", 24, NULL, NULL);

	gnome_app_fill_toolbar_with_data (GTK_TOOLBAR (cd->toolbar_style), editor_toolbar_style_uiinfo, NULL, cd);

	/* restore the stolen domain */
	textdomain (domain);
	g_free (domain);

	gtk_toolbar_append_widget (GTK_TOOLBAR (cd->toolbar_style),
				   setup_color_combo (cd),
				   NULL, NULL);

	cd->font_style_changed_connection_id
		= g_signal_connect (GTK_OBJECT (cd->html), "insertion_font_style_changed",
				      G_CALLBACK (insertion_font_style_changed_cb), cd);

	/* The following SUCKS!  */
	cd->tt_button        = editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_TT].widget;
	cd->bold_button      = editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_BOLD].widget;
	cd->italic_button    = editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_ITALIC].widget;
	cd->underline_button = editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_UNDERLINE].widget;
	cd->strikeout_button = editor_toolbar_style_uiinfo [EDITOR_TOOLBAR_STRIKEOUT].widget;

	cd->left_align_button = editor_toolbar_alignment_group[0].widget;
	cd->center_button = editor_toolbar_alignment_group[1].widget;
	cd->right_align_button = editor_toolbar_alignment_group[2].widget;

	cd->unindent_button  = editor_toolbar_style_uiinfo [8].widget;
	gtk_widget_set_sensitive (cd->unindent_button, gtk_html_get_paragraph_indentation (cd->html) != 0);
	g_signal_connect (cd->html, "current_paragraph_indentation_changed",
			  G_CALLBACK (indentation_changed), cd);

	cd->indent_button    = editor_toolbar_style_uiinfo [9].widget;

	/* g_signal_connect (GTK_OBJECT (cd->html), "destroy",
	   G_CALLBACK (html_destroy_cb), cd);

	   g_signal_connect (GTK_OBJECT (cd->toolbar_style), "destroy",
	   G_CALLBACK (toolbar_destroy_cb), cd); */

	g_signal_connect (cd->html, "current_paragraph_alignment_changed",
			  G_CALLBACK (paragraph_alignment_changed_cb), cd);

	gtk_toolbar_set_style (GTK_TOOLBAR (cd->toolbar_style), GTK_TOOLBAR_ICONS);
	gtk_widget_show_all (hbox);

	toolbar_update_format (cd);
	GTK_WIDGET_UNSET_FLAGS (cd->toolbar_style, GTK_CAN_FOCUS);
	gtk_container_forall (GTK_CONTAINER (cd->toolbar_style), unset_focus, NULL);

	return hbox;
}

static gboolean
toolbar_item_represents (GtkWidget *item, GtkWidget *widget)
{
	GtkWidget *parent;

	if (item == widget)
		return TRUE;

	parent = gtk_widget_get_parent (widget);
	while (parent) {
		if (parent == item)
			return TRUE;
		parent = gtk_widget_get_parent (parent);
	}

	return FALSE;
}

static void
toolbar_item_update_sensitivity (GtkWidget *widget, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *)data;
	gboolean sensitive;

	if (toolbar_item_represents (widget, cd->unindent_button))
		return;

	sensitive = (cd->format_html
		     || toolbar_item_represents (widget, cd->paragraph_option)
		     || toolbar_item_represents (widget, cd->indent_button)
		     || toolbar_item_represents (widget, cd->left_align_button)
		     || toolbar_item_represents (widget, cd->center_button)
		     || toolbar_item_represents (widget, cd->right_align_button));

	gtk_widget_set_sensitive (widget, sensitive);
}

void
toolbar_update_format (GtkHTMLControlData *cd)
{
	if (cd->toolbar_style)
		gtk_container_foreach (GTK_CONTAINER (cd->toolbar_style), 
		toolbar_item_update_sensitivity, cd);

	if (cd->paragraph_option)
		paragraph_style_option_menu_set_mode (cd->paragraph_option, 
						      cd->format_html);
}


GtkWidget *
toolbar_style (GtkHTMLControlData *cd)
{
	g_return_val_if_fail (cd->html != NULL, NULL);
	g_return_val_if_fail (GTK_IS_HTML (cd->html), NULL);

	return create_style_toolbar (cd);
}
