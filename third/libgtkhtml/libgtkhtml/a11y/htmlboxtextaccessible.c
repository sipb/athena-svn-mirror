/*
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "htmlboxtextaccessible.h"
#include "layout/htmlboxtext.h"
#include "layout/htmlboxinline.h"
#include "htmlviewaccessible.h"
#include "htmllinkaccessible.h"
#include <libgail-util/gail-util.h>

static void           html_box_text_accessible_class_init          (HtmlBoxTextAccessibleClass *klass);
static void           html_box_text_accessible_finalize            (GObject             *object);
static void           html_box_text_accessible_real_initialize     (AtkObject           *object,
                                                                    gpointer            data);

static void           html_box_text_accessible_text_interface_init (AtkTextIface        *iface);
static gchar*         html_box_text_accessible_get_text            (AtkText             *text,
                                                              gint                start_offset,
                                                              gint                end_offset);
static gchar*         html_box_text_accessible_get_text_after_offset 
                                                             (AtkText             *text,
                                                              gint                offset,
                                                              AtkTextBoundary     boundary_type,
                                                              gint                *start_offset,
                                                              gint                *end_offset);
static gchar*         html_box_text_accessible_get_text_at_offset  (AtkText             *text,
                                                              gint                offset,
                                                              AtkTextBoundary     boundary_type,
                                                              gint                *start_offset,
                                                              gint                *end_offset);
static gchar*         html_box_text_accessible_get_text_before_offset 
                                                             (AtkText             *text,
                                                              gint                offset,
                                                              AtkTextBoundary     boundary_type,
                                                              gint                *start_offset,
                                                              gint                *end_offset);
static gunichar       html_box_text_accessible_get_character_at_offset 
                                                              (AtkText            *text,
                                                               gint               offset);
static gint           html_box_text_accessible_get_character_count  (AtkText            *text);
static gint           html_box_text_accessible_get_caret_offset     (AtkText            *text);
static gboolean       html_box_text_accessible_set_caret_offset     (AtkText            *text,
                                                               gint               offset);
static gint           html_box_text_accessible_get_offset_at_point  (AtkText            *text,
                                                               gint               x,
                                                               gint               y,
                                                               AtkCoordType       coords);
static void           html_box_text_accessible_get_character_extents (AtkText           *text,
                                                                gint              offset,
                                                                gint              *x,
                                                                gint              *y,
                                                                gint              *width,
                                                                gint              *height,
                                                                AtkCoordType      coords);
static AtkAttributeSet* 
                      html_box_text_accessible_get_run_attributes    (AtkText           *text,
                                                                gint              offset,
                                                                gint              *start_offset,
                                                                gint              *end_offset);
static AtkAttributeSet* 
                      html_box_text_accessible_get_default_attributes (AtkText          *text);
static gint           html_box_text_accessible_get_n_selections      (AtkText           *text);
static gchar*         html_box_text_accessible_get_selection         (AtkText           *text,
                                                                gint              selection_num,
                                                                gint              *start_pos,
                                                                gint              *end_pos);
static gboolean       html_box_text_accessible_add_selection         (AtkText           *text,
                                                                gint              start_pos,
                                                                gint              end_pos);
static gboolean       html_box_text_accessible_remove_selection      (AtkText           *text,
                                                                gint              selection_num);
static gboolean       html_box_text_accessible_set_selection         (AtkText           *text,
                                                                gint              selection_num,
                                                                gint              start_pos,
                                                                gint              end_pos);
static gchar*         get_text_near_offset                     (AtkText           *text,
                                                                GailOffsetType    function,
                                                                AtkTextBoundary   boundary_type,
                                                                gint              offset,
                                                                gint              *start_offset,
                                                                gint              *end_offset);

static void           html_box_text_accessible_hypertext_interface_init (AtkHypertextIface        *iface);

static AtkHyperlink*  html_box_text_accessible_get_link        (AtkHypertext      *hypertext,
                                                                gint              link_index);
static gint           html_box_text_accessible_get_n_links     (AtkHypertext      *hypertext);
static gint           html_box_text_accessible_get_link_index  (AtkHypertext      *hypertext,
                                                                gint              char_index);
static gboolean       has_link                                 (AtkHypertext      *hypertext);

static gpointer parent_class = NULL;

struct _HtmlBoxTextAccessiblePrivate
{
	GailTextUtil *textutil;
	gpointer links;
};

GType
html_box_text_accessible_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo tinfo = {
			sizeof (HtmlBoxTextAccessibleClass),
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) html_box_text_accessible_class_init,
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			sizeof (HtmlBoxTextAccessible),
			0, /* nb preallocs */
			(GInstanceInitFunc) NULL, /* instance init */
			NULL /* value table */
		};

		static const GInterfaceInfo atk_text_info = {
			(GInterfaceInitFunc) html_box_text_accessible_text_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		static const GInterfaceInfo atk_hypertext_info = {
			(GInterfaceInitFunc) html_box_text_accessible_hypertext_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};
		type = g_type_register_static (HTML_TYPE_BOX_ACCESSIBLE, "HtmlBoxTextAccessible", &tinfo, 0);
		g_type_add_interface_static (type, ATK_TYPE_TEXT, &atk_text_info);      
		g_type_add_interface_static (type, ATK_TYPE_HYPERTEXT, &atk_hypertext_info);      
	}

	return type;
}

AtkObject*
html_box_text_accessible_new (GObject *obj)
{
	gpointer object;
	AtkObject *atk_object;

	g_return_val_if_fail (HTML_IS_BOX_TEXT (obj), NULL);
	object = g_object_new (HTML_TYPE_BOX_TEXT_ACCESSIBLE, NULL);
	atk_object = ATK_OBJECT (object);

	atk_object_initialize (atk_object, obj);

	atk_object->role =  ATK_ROLE_TEXT;
	return atk_object;
}

static void
html_box_text_accessible_real_initialize (AtkObject *object,
                                          gpointer   data)
{
	HtmlBoxTextAccessible *text;
	HtmlBoxText *box_text;
	GtkTextBuffer *text_buffer;
	gchar *contents;
	gint len;

	ATK_OBJECT_CLASS (parent_class)->initialize (object, data);

	text = HTML_BOX_TEXT_ACCESSIBLE (object);
        text->priv = g_new0 (HtmlBoxTextAccessiblePrivate, 1);
	box_text = HTML_BOX_TEXT (data);
	contents  = html_box_text_get_text (box_text, &len);
	text_buffer = gtk_text_buffer_new (NULL);
	if (contents)
		gtk_text_buffer_set_text (text_buffer, contents, len);
	text->priv->textutil = gail_text_util_new ();
	gail_text_util_buffer_setup (text->priv->textutil, text_buffer);
	g_object_unref (text_buffer);

}

static void
html_box_text_accessible_class_init (HtmlBoxTextAccessibleClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize = html_box_text_accessible_finalize;

	class->initialize = html_box_text_accessible_real_initialize;
}

static void
html_box_text_accessible_finalize (GObject *object)
{
	HtmlBoxTextAccessible *text = HTML_BOX_TEXT_ACCESSIBLE (object);

	if (text->priv->links)
		g_object_unref (text->priv->links);

	g_object_unref (text->priv->textutil);
	g_free (text->priv);
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
html_box_text_accessible_text_interface_init (AtkTextIface *iface)
{
	g_return_if_fail (iface != NULL);

	iface->get_text = html_box_text_accessible_get_text;
	iface->get_text_after_offset = html_box_text_accessible_get_text_after_offset;
	iface->get_text_at_offset = html_box_text_accessible_get_text_at_offset;
	iface->get_text_before_offset = html_box_text_accessible_get_text_before_offset;
	iface->get_character_at_offset = html_box_text_accessible_get_character_at_offset;
	iface->get_character_count = html_box_text_accessible_get_character_count;
	iface->get_caret_offset = html_box_text_accessible_get_caret_offset;
	iface->set_caret_offset = html_box_text_accessible_set_caret_offset;
	iface->get_offset_at_point = html_box_text_accessible_get_offset_at_point;
	iface->get_character_extents = html_box_text_accessible_get_character_extents;
	iface->get_n_selections = html_box_text_accessible_get_n_selections;
	iface->get_selection = html_box_text_accessible_get_selection;
	iface->add_selection = html_box_text_accessible_add_selection;
	iface->remove_selection = html_box_text_accessible_remove_selection;
	iface->set_selection = html_box_text_accessible_set_selection;
	iface->get_run_attributes = html_box_text_accessible_get_run_attributes;
	iface->get_default_attributes = html_box_text_accessible_get_default_attributes;
}

static gchar*
html_box_text_accessible_get_text (AtkText *text,
                                   gint    start_offset,
                                   gint    end_offset)
{
	HtmlBoxTextAccessible *text_accessible;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	g_return_val_if_fail (HTML_IS_BOX_TEXT_ACCESSIBLE (text), NULL);
	text_accessible = HTML_BOX_TEXT_ACCESSIBLE (text);
	g_return_val_if_fail (text_accessible->priv->textutil, NULL);

	buffer = text_accessible->priv->textutil->buffer;
	gtk_text_buffer_get_iter_at_offset (buffer, &start, start_offset);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, end_offset);

	return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
}

static gchar*
html_box_text_accessible_get_text_after_offset (AtkText         *text,
                                                gint            offset,
                                                AtkTextBoundary boundary_type,
                                                gint            *start_offset,
                                                gint            *end_offset)
{
	return get_text_near_offset (text, GAIL_AFTER_OFFSET,
				     boundary_type, offset, 
				     start_offset, end_offset);
}

static gchar*
html_box_text_accessible_get_text_at_offset (AtkText         *text,
                                             gint            offset,
                                             AtkTextBoundary boundary_type,
                                             gint            *start_offset,
                                             gint            *end_offset)
{
	return get_text_near_offset (text, GAIL_AT_OFFSET,
				     boundary_type, offset, 
				     start_offset, end_offset);
}

static gchar*
html_box_text_accessible_get_text_before_offset (AtkText         *text,
                                                 gint            offset,
                                                 AtkTextBoundary boundary_type,
                                                 gint            *start_offset,
                                                 gint            *end_offset)
{
	return get_text_near_offset (text, GAIL_BEFORE_OFFSET,
				     boundary_type, offset, 
				     start_offset, end_offset);
}

static gunichar
html_box_text_accessible_get_character_at_offset (AtkText *text,
                                                  gint    offset)
{
	HtmlBoxTextAccessible *text_accessible;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	gchar *string;
	gchar *index;
	gunichar unichar;

	g_return_val_if_fail (HTML_IS_BOX_TEXT_ACCESSIBLE (text), '\0');
	text_accessible = HTML_BOX_TEXT_ACCESSIBLE (text);
	buffer = text_accessible->priv->textutil->buffer;
	if (offset >= gtk_text_buffer_get_char_count (buffer))
		return '\0';

	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);
	string = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	index = g_utf8_offset_to_pointer (string, offset);

	unichar = g_utf8_get_char (index);
	g_free (string);
	return unichar;
}

static gint
html_box_text_accessible_get_character_count (AtkText *text)
{
	GtkTextBuffer *buffer;
	HtmlBoxTextAccessible *text_accessible;

	g_return_val_if_fail (HTML_IS_BOX_TEXT_ACCESSIBLE (text), 0);
	text_accessible = HTML_BOX_TEXT_ACCESSIBLE (text);
	g_return_val_if_fail (text_accessible->priv->textutil, 0);
	buffer = text_accessible->priv->textutil->buffer;
	return gtk_text_buffer_get_char_count (buffer);
}

static gint
html_box_text_accessible_get_caret_offset (AtkText *text)
{
	HtmlBoxTextAccessible *text_accessible;
	GtkTextBuffer *buffer;
	GtkTextMark *cursor_mark;
	GtkTextIter cursor_itr;

	g_return_val_if_fail (HTML_IS_BOX_TEXT_ACCESSIBLE (text), 0);
	text_accessible = HTML_BOX_TEXT_ACCESSIBLE (text);
	g_return_val_if_fail (text_accessible->priv->textutil, 0);
	buffer = text_accessible->priv->textutil->buffer;
	cursor_mark = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_get_iter_at_mark (buffer, &cursor_itr, cursor_mark);
	return gtk_text_iter_get_offset (&cursor_itr);
}

static gboolean
html_box_text_accessible_set_caret_offset (AtkText *text,
                                           gint    offset)
{
	HtmlBoxTextAccessible *text_accessible;
	GtkTextBuffer *buffer;
	GtkTextIter pos_itr;

	g_return_val_if_fail (HTML_IS_BOX_TEXT_ACCESSIBLE (text), FALSE);
	text_accessible = HTML_BOX_TEXT_ACCESSIBLE (text);
	g_return_val_if_fail (text_accessible->priv->textutil, FALSE);
	buffer = text_accessible->priv->textutil->buffer;
	gtk_text_buffer_get_iter_at_offset (buffer,  &pos_itr, offset);
	gtk_text_buffer_move_mark_by_name (buffer, "insert", &pos_itr);
	return TRUE;
}

static gint
html_box_text_accessible_get_offset_at_point (AtkText      *text,
                                              gint         x,
                                              gint         y,
                                              AtkCoordType coords)
{
	gint real_x, real_y, real_width, real_height;
	AtkGObjectAccessible *atk_gobj;
	GObject *g_obj;
	HtmlBoxText *box_text;
	HtmlBox *box;

	atk_component_get_extents (ATK_COMPONENT (text), &real_x, &real_y,
				   &real_width, &real_height, coords);
	if (y < real_y || y >= real_y + real_height)
		return -1;
	if (x < real_x || x >= real_x + real_width)
		return -1;

	atk_gobj = ATK_GOBJECT_ACCESSIBLE (text);
	g_obj = atk_gobject_accessible_get_object (atk_gobj);
	if (g_obj == NULL)
		return -1;

	box_text = HTML_BOX_TEXT (g_obj);
	box = HTML_BOX (g_obj);

	if (box->prev == NULL) {
		while (HTML_IS_BOX_INLINE (box->parent)) {
			x -= html_box_left_border_width (box->parent);
			box = box->parent;
		}
	}
	return html_box_text_get_index (box_text, x - real_x);
}

static void
html_box_text_accessible_get_character_extents (AtkText      *text,
                                                gint         offset,
                                                gint         *x,
                                                gint         *y,
                                                gint         *width,
                                                gint         *height,
                                                AtkCoordType coords)
{
	gint real_x, real_y;
	AtkGObjectAccessible *atk_gobj;
	GObject *g_obj;
	HtmlBoxText *box_text;
	HtmlBox *box;
	GdkRectangle rect;

	atk_component_get_position (ATK_COMPONENT (text), &real_x, &real_y,
				    coords);

	atk_gobj = ATK_GOBJECT_ACCESSIBLE (text);
	g_obj = atk_gobject_accessible_get_object (atk_gobj);
	if (g_obj == NULL)
		return;

	box_text = HTML_BOX_TEXT (g_obj);
	box = HTML_BOX (g_obj);

	html_box_text_get_character_extents (box_text, offset, &rect);
        *x = real_x + rect.x;
	if (box->prev == NULL) {
		while (HTML_IS_BOX_INLINE (box->parent)) {
			*x += html_box_left_border_width (box->parent);
			box = box->parent;
		}
	}
	
        *y = real_y;
        *width = rect.width;
	*height = rect.height;
}

static AtkAttributeSet*
html_box_text_accessible_get_run_attributes (AtkText *text,
                                             gint    offset,
                                             gint    *start_offset,
                                             gint    *end_offset)
{
	return NULL;
}

static AtkAttributeSet*
html_box_text_accessible_get_default_attributes (AtkText *text)
{
	AtkGObjectAccessible *atk_gobj;
	AtkAttributeSet *attrib_set = NULL;
	GObject *g_obj;
	GtkWidget *view;
	HtmlBox *box;
	HtmlFontSpecification *font_spec;
	PangoAttrFontDesc *pango_font_desc;
	PangoFontDescription *font;
	PangoFontMask mask;
	PangoAttrList *attrs;
	PangoAttrIterator *iter;
	PangoAttrInt *pango_int;
	HtmlColor *color;
	HtmlStyle *style;
	HtmlTextAlignType text_align;
	gint len;
	gint int_value;
	gchar *value;
	GSList *attr;

	atk_gobj = ATK_GOBJECT_ACCESSIBLE (text);
	g_obj = atk_gobject_accessible_get_object (atk_gobj);
	if (g_obj == NULL)
		return NULL;

	box = HTML_BOX (g_obj);
	
	view = html_box_accessible_get_view_widget (box);
	style = HTML_BOX_GET_STYLE (box);
	font_spec = style->inherited->font_spec;
	attrs = pango_attr_list_new ();
	html_font_specification_get_all_attributes (font_spec, attrs, 0, len,
						    HTML_VIEW (view)->magnification);
	iter = pango_attr_list_get_iterator (attrs);

        int_value = html_box_get_bidi_level (box);
	if (int_value > 1)
		int_value = 1;
	/*
         * int_value + 1 is to allow for skip "none" value for
	 * ATK_TEXT_ATTR_DIRECTION
	 */
	value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_DIRECTION, int_value + 1));
	attrib_set = gail_misc_add_attribute (attrib_set,
                                              ATK_TEXT_ATTR_DIRECTION,
                                              value);
	/*
	 * Currently unable to get language; see bug 297 in 
	 * bugzilla.codefactory.se
	 */
	if ((pango_font_desc  = (PangoAttrFontDesc*) pango_attr_iterator_get (iter, PANGO_ATTR_FONT_DESC)) != NULL) {
		font = pango_font_desc->desc;
		mask = pango_font_description_get_set_fields (font);
		if (mask & PANGO_FONT_MASK_STYLE) {
			value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_STYLE, pango_font_description_get_style (font)));
			attrib_set = gail_misc_add_attribute (attrib_set,
							      ATK_TEXT_ATTR_STYLE,
							      value);
		}
		if (mask & PANGO_FONT_MASK_VARIANT) {
			value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_VARIANT, pango_font_description_get_variant (font)));
			attrib_set = gail_misc_add_attribute (attrib_set,
							      ATK_TEXT_ATTR_VARIANT,
							      value);
		}
		if (mask & PANGO_FONT_MASK_STRETCH) {
			value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_STRETCH, pango_font_description_get_variant (font)));
			attrib_set = gail_misc_add_attribute (attrib_set,
							      ATK_TEXT_ATTR_STRETCH,
							      value);
		}
		if (mask & PANGO_FONT_MASK_FAMILY) {
			value = g_strdup (pango_font_description_get_family (font));
			attrib_set = gail_misc_add_attribute (attrib_set,
							      ATK_TEXT_ATTR_FAMILY_NAME,
							      value);
		}
		if (mask & PANGO_FONT_MASK_WEIGHT) {
			value = g_strdup_printf ("%i", pango_font_description_get_weight (font));
			attrib_set = gail_misc_add_attribute (attrib_set,
							      ATK_TEXT_ATTR_WEIGHT,
							      value);
		}
		if (mask & PANGO_FONT_MASK_SIZE) {
			value = g_strdup_printf ("%i", pango_font_description_get_size (font) / PANGO_SCALE);
			attrib_set = gail_misc_add_attribute (attrib_set,
							      ATK_TEXT_ATTR_SIZE,
							      value);
		}
	}

	text_align = style->inherited->text_align;
	if (text_align == HTML_TEXT_ALIGN_RIGHT)
		int_value = 1;
	else if (text_align == HTML_TEXT_ALIGN_CENTER)
		int_value = 2;
	else if (text_align == HTML_TEXT_ALIGN_JUSTIFY)
		int_value = 3;
	else
		int_value = 0;
	value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_JUSTIFICATION, int_value));
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_JUSTIFICATION,
					      value);

	/* Guess wrap word */
	value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_WRAP_MODE, 2));
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_WRAP_MODE,
					      value);

	color = &style->background->color;
	value = g_strdup_printf ("%u,%u,%u",
				 color->red, color->green, color->blue);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_BG_COLOR,
					      value);

	color = style->inherited->color;
	if (color) {
		value = g_strdup_printf ("%u,%u,%u",
					 color->red, color->green, color->blue);
		attrib_set = gail_misc_add_attribute (attrib_set,
						      ATK_TEXT_ATTR_FG_COLOR,
						      value);
	}

	value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_FG_STIPPLE, 0));
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_FG_STIPPLE,
					      value);
	value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_BG_STIPPLE, 0));
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_BG_STIPPLE,
					      value);
	if ((pango_int  = (PangoAttrInt*) pango_attr_iterator_get (iter, PANGO_ATTR_UNDERLINE)) != NULL) 
		int_value = pango_int->value;
	else
		int_value = 0;
	value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_UNDERLINE, int_value));
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_UNDERLINE,
					      value);
	if ((pango_int  = (PangoAttrInt*) pango_attr_iterator_get (iter, PANGO_ATTR_STRIKETHROUGH)) != NULL)
		int_value = pango_int->value;
	else
		int_value = 0;
	value = g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_STRIKETHROUGH, int_value));
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_STRIKETHROUGH,
					      value);
	value = g_strdup_printf ("%i", 0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_RISE,
					      value);
	value = g_strdup_printf ("%g", 1.0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_SCALE,
					      value);
	value = g_strdup_printf ("%i", 0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_BG_FULL_HEIGHT,
					      value);
	value = g_strdup_printf ("%i", 0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_PIXELS_INSIDE_WRAP,
					      value);
	value = g_strdup_printf ("%i", 0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_PIXELS_BELOW_LINES,
					      value);
	value = g_strdup_printf ("%i", 0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_PIXELS_ABOVE_LINES,
					      value);
	value = g_strdup_printf (atk_text_attribute_get_value (ATK_TEXT_ATTR_EDITABLE, 0));
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_EDITABLE,
					      value);
	if (style->visibility == HTML_VISIBILITY_VISIBLE)
		int_value = 0;
	else
		int_value = 1;
	value = g_strdup_printf (atk_text_attribute_get_value (ATK_TEXT_ATTR_INVISIBLE, int_value));
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_INVISIBLE,
					      value);
	value = g_strdup_printf ("%i", 0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_INDENT,
					      value);
	value = g_strdup_printf ("%i", 0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_RIGHT_MARGIN,
					      value);
	value = g_strdup_printf ("%i", 0);
	attrib_set = gail_misc_add_attribute (attrib_set,
					      ATK_TEXT_ATTR_LEFT_MARGIN,
					      value);

	pango_attr_iterator_destroy (iter);
	pango_attr_list_unref (attrs);
	return attrib_set;
}

static gint
html_box_text_accessible_get_n_selections (AtkText *text)
{
	AtkGObjectAccessible *atk_gobj;
	GObject *g_obj;
	HtmlBoxText *box_text;
        gint n_selections;

	atk_gobj = ATK_GOBJECT_ACCESSIBLE (text);
	g_obj = atk_gobject_accessible_get_object (atk_gobj);
	if (g_obj == NULL)
		return 0;

	box_text = HTML_BOX_TEXT (g_obj);
	switch (box_text->selection) {
	case HTML_BOX_TEXT_SELECTION_NONE:
		n_selections = 0;
		break;
	default:
		n_selections = 1;
		break;
	}
	return n_selections;
}

static gchar*
html_box_text_accessible_get_selection (AtkText *text,
                                        gint    selection_num,
                                        gint    *start_pos,
                                        gint    *end_pos)
{
	AtkGObjectAccessible *atk_gobj;
	GObject *g_obj;
	HtmlBoxText *box_text;

        if (selection_num)
		return NULL;

	atk_gobj = ATK_GOBJECT_ACCESSIBLE (text);
	g_obj = atk_gobject_accessible_get_object (atk_gobj);
	if (g_obj == NULL)
		return NULL;

	box_text = HTML_BOX_TEXT (g_obj);
	if  (box_text->selection != HTML_BOX_TEXT_SELECTION_NONE) {
		*start_pos = box_text->sel_start_index;
		*end_pos = box_text->sel_end_index;
		return html_box_text_accessible_get_text (text, *start_pos, *end_pos);
	} else
		return NULL;
}

static gboolean
html_box_text_accessible_add_selection (AtkText *text,
                                        gint    start_pos,
                                        gint    end_pos)
{
	AtkGObjectAccessible *atk_gobj;
	GObject *g_obj;
	HtmlBoxText *box_text;

	atk_gobj = ATK_GOBJECT_ACCESSIBLE (text);
	g_obj = atk_gobject_accessible_get_object (atk_gobj);
	if (g_obj == NULL)
		return FALSE;

	box_text = HTML_BOX_TEXT (g_obj);
	if  (box_text->selection != HTML_BOX_TEXT_SELECTION_NONE)
		return FALSE;
	else {
		gint len = html_box_text_get_len (box_text);
		HtmlBoxTextSelection selection;
		GtkWidget *view;

		if (start_pos < 0 || 
		    end_pos < 0 || 
		    len < start_pos ||
		    len < end_pos ||
		    start_pos == end_pos)
			return FALSE;

		selection = HTML_BOX_TEXT_SELECTION_BOTH;

		html_box_text_set_selection (box_text, selection, start_pos, end_pos);
		view = html_box_accessible_get_view_widget (HTML_BOX (box_text));
		gtk_widget_queue_draw (view);

		return TRUE;
	}
}

static gboolean
html_box_text_accessible_remove_selection (AtkText *text,
                                           gint    selection_num)
{
	AtkGObjectAccessible *atk_gobj;
	GObject *g_obj;
	HtmlBoxText *box_text;

	if (selection_num)
		return FALSE;

	atk_gobj = ATK_GOBJECT_ACCESSIBLE (text);
	g_obj = atk_gobject_accessible_get_object (atk_gobj);
	if (g_obj == NULL)
		return FALSE;

	box_text = HTML_BOX_TEXT (g_obj);
	if  (box_text->selection == HTML_BOX_TEXT_SELECTION_NONE)
		return FALSE;
	else {
		GtkWidget *view;

		html_box_text_set_selection (box_text, HTML_BOX_TEXT_SELECTION_NONE, -1, -1);
		view = html_box_accessible_get_view_widget (HTML_BOX (box_text));
		gtk_widget_queue_draw (view);
		return TRUE;
	}
}

static gboolean
html_box_text_accessible_set_selection (AtkText *text,
                                        gint    selection_num,
                                        gint    start_pos,
                                        gint    end_pos)
{
	if (selection_num)
		return FALSE;

	return html_box_text_accessible_add_selection (text, start_pos, end_pos);
}

static gchar*
get_text_near_offset (AtkText          *text,
                      GailOffsetType   function,
                      AtkTextBoundary  boundary_type,
                      gint             offset,
                      gint             *start_offset,
                      gint             *end_offset)
{
	return gail_text_util_get_text (HTML_BOX_TEXT_ACCESSIBLE (text)->priv->textutil, NULL,
					function, boundary_type, offset, 
					start_offset, end_offset);
}

static void
html_box_text_accessible_hypertext_interface_init (AtkHypertextIface *iface)
{
	g_return_if_fail (iface != NULL);

	iface->get_link = html_box_text_accessible_get_link;
	iface->get_n_links = html_box_text_accessible_get_n_links;
	iface->get_link_index = html_box_text_accessible_get_link_index;
}

static AtkHyperlink*
 html_box_text_accessible_get_link (AtkHypertext *hypertext,
                                    gint         link_index)
{
	HtmlBoxTextAccessible *text;

	text = HTML_BOX_TEXT_ACCESSIBLE (hypertext);
	if (has_link (hypertext)) {
		if (!text->priv->links) {
			text->priv->links =  html_link_accessible_new (ATK_OBJECT (hypertext));
		}
		return text->priv->links;
	}
	else
		return NULL;
}

static gint
html_box_text_accessible_get_n_links (AtkHypertext *hypertext)
{
	if (has_link (hypertext))
		return 1;
	else
		return 0;		
}

static gint
html_box_text_accessible_get_link_index (AtkHypertext *hypertext,
                                         gint         char_index)
{
	gint index = -1;

	if (has_link (hypertext)) {
		gint char_count;

		char_count = html_box_text_accessible_get_character_count (ATK_TEXT (hypertext));
		if (char_count > 0 && char_index >= 0 && char_index < char_count)
			index = 0;
	}
	return index;
}

static gboolean
has_link (AtkHypertext *hypertext)
{
	AtkGObjectAccessible *atk_gobj;
	GObject *g_obj;
	HtmlBox *box;
	gboolean has_link = FALSE;

	atk_gobj = ATK_GOBJECT_ACCESSIBLE (hypertext);
	g_obj = atk_gobject_accessible_get_object (atk_gobj);
	if (g_obj == NULL)
		return has_link;

	box = HTML_BOX (g_obj);
	if (HTML_IS_BOX_INLINE (box->parent)) {
		DomNode *node;

		node = box->parent->dom_node;

		if (node->xmlnode->name) {
			if (strcasecmp (node->xmlnode->name, "a") == 0 &&
			   (xmlHasProp (node->xmlnode, "href") != NULL)) {
				has_link = TRUE;
			}
		}
	}
	return has_link;
}

