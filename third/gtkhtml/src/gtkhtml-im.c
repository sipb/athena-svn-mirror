/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright 1999, 2000 Helix Code, Inc.

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
*/

#include <config.h>

#include <gtk/gtk.h>
#include "gtkhtml.h"
#include "gtkhtml-im.h"
#include "gtkhtml-private.h"

#ifdef GTK_HTML_USE_XIM
void 
gtk_html_im_focus_in (GtkHTML *html)
{
	if (html->priv->ic)
		gdk_im_begin (html->priv->ic, GTK_WIDGET (html)->window);
}

void
gtk_html_im_focus_out (GtkHTML *html)
{
	gdk_im_end ();
}

void
gtk_html_im_realize (GtkHTML *html)
{
	GtkWidget *widget = GTK_WIDGET (html);
	GdkICAttr *attr;
	GdkColormap *colormap;

	gint width, height;
	GdkEventMask mask;

	GdkICAttributesType attrmask = GDK_IC_ALL_REQ;
	GdkIMStyle style;
	GdkIMStyle supported_style =
		GDK_IM_PREEDIT_NONE |
		GDK_IM_PREEDIT_NOTHING |
		GDK_IM_PREEDIT_POSITION |
		GDK_IM_STATUS_NONE |
		GDK_IM_STATUS_NOTHING;

	if (!gdk_im_ready () || (attr = gdk_ic_attr_new ()) == NULL)
		return;
		
	if (widget->style &&
	    widget->style->font->type != GDK_FONT_FONTSET)
		supported_style &= ~GDK_IM_PREEDIT_POSITION;
	
	attr->style = style =
		gdk_im_decide_style (supported_style);
	attr->client_window = widget->window;
	
	if ((colormap = gtk_widget_get_colormap (widget)) !=
	    gtk_widget_get_default_colormap ()) {
		attrmask |= GDK_IC_PREEDIT_COLORMAP;
		attr->preedit_colormap = colormap;
	}

	attrmask |= GDK_IC_PREEDIT_FOREGROUND;
	attrmask |= GDK_IC_PREEDIT_BACKGROUND;
	attr->preedit_foreground = widget->style->fg[GTK_STATE_NORMAL];
	attr->preedit_background = widget->style->base[GTK_STATE_NORMAL];
	
	switch (style & GDK_IM_PREEDIT_MASK) {
	case GDK_IM_PREEDIT_POSITION:
		if (widget->style && widget->style->font->type
		    != GDK_FONT_FONTSET) {
			g_warning ("over-the-spot style requires fontset");
			break;
		}
		
		gdk_window_get_size (widget->window,
				     &width, &height);
		
		attrmask |= GDK_IC_PREEDIT_POSITION_REQ;
		attr->spot_location.x = 0;
		attr->spot_location.y = height;
		attr->preedit_area.x = 0;
		attr->preedit_area.y = 0;
		attr->preedit_area.width = width;
		attr->preedit_area.height = height;
		attr->preedit_fontset = widget->style->font;
		
		break;
	}
	html->priv->ic_attr = attr;
	html->priv->ic = gdk_ic_new (attr, attrmask);
	
	if (html->priv->ic == NULL)
		g_warning ("Can't create input context.");
	else {
		mask = gdk_window_get_events (widget->window);
		mask |= gdk_ic_get_events (html->priv->ic);
		gdk_window_set_events (widget->window, mask);
		
		if (GTK_WIDGET_HAS_FOCUS(widget))
			gdk_im_begin (html->priv->ic, widget->window);
	}
}

void
gtk_html_im_unrealize (GtkHTML *html) 
{
	if (html->priv->ic) {
		gdk_ic_destroy (html->priv->ic);
		html->priv->ic = NULL;
	}
	if (html->priv->ic_attr) {
		gdk_ic_attr_destroy (html->priv->ic_attr);
		html->priv->ic_attr = NULL;
	}
}

void
gtk_html_im_size_allocate (GtkHTML *html) 
{
	GtkWidget *widget = GTK_WIDGET (html);

	if (!GTK_WIDGET_REALIZED (widget))
		return;

	if (html->priv->ic == NULL)
		return;

	if (gdk_ic_get_style (html->priv->ic) & GDK_IM_PREEDIT_POSITION) {
		gint width, height;
		
		gdk_window_get_size (widget->window,
				     &width, &height);
		html->priv->ic_attr->preedit_area.width = width;
		html->priv->ic_attr->preedit_area.height = height;
		gdk_ic_set_attr (html->priv->ic, html->priv->ic_attr,
				 GDK_IC_PREEDIT_AREA);
	}
}


void 
gtk_html_im_style_set (GtkHTML *html)
{
	GtkWidget *widget = GTK_WIDGET (html);
	GdkICAttributesType mask = 0;
	
	if (!GTK_WIDGET_REALIZED (widget))
		return;

	if (html->priv->ic == NULL)
		return;

	gdk_ic_get_attr (html->priv->ic, html->priv->ic_attr,
			 GDK_IC_PREEDIT_FOREGROUND |
			 GDK_IC_PREEDIT_BACKGROUND |
			 GDK_IC_PREEDIT_FONTSET);
	
	if (html->priv->ic_attr->preedit_foreground.pixel != 
	    widget->style->fg[GTK_STATE_NORMAL].pixel) {
		mask |= GDK_IC_PREEDIT_FOREGROUND;
		html->priv->ic_attr->preedit_foreground
			= widget->style->fg[GTK_STATE_NORMAL];
	}
	if (html->priv->ic_attr->preedit_background.pixel != 
	    widget->style->base[GTK_STATE_NORMAL].pixel) {
		mask |= GDK_IC_PREEDIT_BACKGROUND;
		html->priv->ic_attr->preedit_background
			= widget->style->base[GTK_STATE_NORMAL];
	}
	if ((gdk_ic_get_style (html->priv->ic) & GDK_IM_PREEDIT_POSITION) && 
	    widget->style->font != NULL &&
	    widget->style->font->type == GDK_FONT_FONTSET &&
	    !gdk_font_equal (html->priv->ic_attr->preedit_fontset,
			     widget->style->font)) {
		mask |= GDK_IC_PREEDIT_FONTSET;
		html->priv->ic_attr->preedit_fontset = widget->style->font;
	}
	
	if (mask)
		gdk_ic_set_attr (html->priv->ic, html->priv->ic_attr, mask);
}


#endif






