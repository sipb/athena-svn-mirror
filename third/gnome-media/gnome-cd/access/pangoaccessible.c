/*
 * Copyright 2002 Sun Microsystems Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <sys/types.h>
#include <libgail-util/gailmisc.h>
#include <pango/pango-layout.h>
#include <atk/atkobject.h>

#include "../cdrom.h"
#include "../display.h"
#include "pangoaccessible.h"
#include "../gnome-cd.h"

static void pango_accessible_class_init       (PangoAccessibleClass *klass);

static gint pango_accessible_get_n_children   (AtkObject       *obj);
static gint pango_accessible_get_index_in_parent   (AtkObject       *obj);

static void pango_accessible_real_initialize  (AtkObject *obj, gpointer data);
static void pango_accessible_finalize         (GObject        *object);

static void atk_text_interface_init (AtkTextIface *iface);
static void atk_component_interface_init (AtkComponentIface *iface);

/* AtkText Interfaces */
static gchar* pango_accessible_get_text (AtkText *text,
                     			 gint    start_pos,
                     			 gint    end_pos);
static gchar* pango_accessible_get_text_before_offset(AtkText *text,
						      gint offset,
						      AtkTextBoundary bound_type,
						      gint *start_offset,
						      gint *end_offset);
static gchar* pango_accessible_get_text_after_offset (AtkText *text,
						      gint offset,
						      AtkTextBoundary bound_type,
						      gint *start_offset,
						      gint *end_offset);
static gchar* pango_accessible_get_text_at_offset (AtkText *text,
						   gint offset,
						   AtkTextBoundary boundary_type,
						   gint *start_offset,
						   gint *end_offset);
static gint pango_accessible_get_character_count (AtkText *text);
static gunichar pango_accessible_get_character_at_offset (AtkText *text,
							  gint offset);
static AtkAttributeSet* pango_accessible_get_run_attributes (AtkText *text,
							     gint offset,
							     gint *start_offset,
							     gint *end_offset);
static void pango_accessible_get_character_extents (AtkText	*text,
						    gint         offset,
						    gint         *x,
						    gint         *y,
						    gint         *width,
						    gint         *height,
						    AtkCoordType coords);
static gint pango_accessible_get_offset_at_point (AtkText      *text,
						  gint         x,
						  gint         y,
						  AtkCoordType coords);
static AtkAttributeSet* pango_accessible_get_default_attributes (AtkText *text);
static gint pango_accessible_get_caret_offset (AtkText *text);


/* AtkComponent Interfaces */
static gboolean pango_accessible_contains (AtkComponent   *component,
					   gint           x,
					   gint           y,
					   AtkCoordType coords);
static void pango_accessible_get_position (AtkComponent   *component,
					   gint           *x,
					   gint           *y,
					   AtkCoordType coords);
static void pango_accessible_get_size (AtkComponent   *component,
				       gint           *width,
				       gint           *height);
static void pango_accessible_get_extents (AtkComponent *component,
					  gint *x, gint *y, gint *width,
					  gint *height, AtkCoordType coords);

static gpointer parent_class = NULL;
extern AtkObject *pango_accessible[];

GType
pango_accessible_get_type (void)
{
	static GType type = 0;

	if (!type)
	{
		static const GTypeInfo tinfo =
		{
			sizeof (PangoAccessibleClass), /* class size */
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) pango_accessible_class_init, /* class init */
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			sizeof (PangoAccessible), /* instance size */
			0, /* nb preallocs */
			(GInstanceInitFunc)NULL, /* instance init */
			NULL /* value table */
		};

		static const GInterfaceInfo atk_text_info =
		{
			(GInterfaceInitFunc) atk_text_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		static const GInterfaceInfo atk_component_info =
		{
			(GInterfaceInitFunc) atk_component_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		type = g_type_register_static (ATK_TYPE_OBJECT,
                                   	       "PangoAccessible", &tinfo, 0);
		g_type_add_interface_static(type, ATK_TYPE_TEXT,
					    &atk_text_info);
		g_type_add_interface_static(type, ATK_TYPE_COMPONENT,
					    &atk_component_info);

	}

	return type;
}

static void
pango_accessible_class_init (PangoAccessibleClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

	g_return_if_fail(class != NULL);
	parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize = pango_accessible_finalize;

	class->get_n_children = pango_accessible_get_n_children;
	class->get_index_in_parent = pango_accessible_get_index_in_parent;
	class->initialize = pango_accessible_real_initialize;
}

static void
pango_accessible_real_initialize (AtkObject *obj,
				  gpointer  data)
{
	PangoAccessible *pango_accessible;
	PangoLayout *playout;

	pango_accessible = PANGO_ACCESSIBLE(obj);
	g_return_if_fail(pango_accessible != NULL);
	pango_accessible->textutil = gail_text_util_new();

	playout = PANGO_LAYOUT(data);
	g_return_if_fail(playout != NULL);
	gail_text_util_text_setup(pango_accessible->textutil,
				  pango_layout_get_text(playout));
}

AtkObject *
pango_accessible_new (PangoLayout *obj)
{
	GObject *object;
	AtkObject *accessible;
	PangoAccessible *cp;

	object = g_object_new (PANGO_TYPE_ACCESSIBLE, NULL);
	g_return_val_if_fail(object != NULL, NULL);

	accessible = ATK_OBJECT (object);
	atk_object_initialize (accessible, obj);

	cp = PANGO_ACCESSIBLE(object);
	cp->playout = obj;

	accessible->role = ATK_ROLE_DRAWING_AREA;

	return accessible;
}

/*
 * Report the number of children as 0
 */
static gint
pango_accessible_get_n_children (AtkObject* obj)
{
	return 0;
}

static gint
pango_accessible_get_index_in_parent (AtkObject* obj)
{
	gint i;

	for (i = 0; i < CD_DISPLAY_END; i++) {
		if (pango_accessible [i] == obj)
			return i;
	}
	return -1;
}

static void
pango_accessible_finalize (GObject *object)
{
	PangoAccessible *cp = PANGO_ACCESSIBLE(object);

	g_object_unref (cp->textutil);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gchar*
pango_accessible_get_text (AtkText *text,
			   gint    start_pos,
			   gint    end_pos)
{
	PangoAccessible *pango_accessible;
	PangoLayout *playout;

	pango_accessible = PANGO_ACCESSIBLE(text);
	playout = pango_accessible->playout;
	g_return_val_if_fail(playout != NULL, NULL);

	gail_text_util_text_setup(pango_accessible->textutil,
				  pango_layout_get_text(playout));

	return gail_text_util_get_substring (PANGO_ACCESSIBLE(text)->textutil,
					     start_pos, end_pos);
}

static gchar*
pango_accessible_get_text_before_offset(AtkText         *text,
					gint            offset,
					AtkTextBoundary boundary_type,
					gint            *start_offset,
					gint            *end_offset)
{
	PangoLayout *playout;

	playout = PANGO_ACCESSIBLE(text)->playout;
	g_return_val_if_fail(playout != NULL, NULL);
	return gail_text_util_get_text (PANGO_ACCESSIBLE (text)->textutil,
					playout , GAIL_BEFORE_OFFSET,
					boundary_type, offset,
					start_offset, end_offset);
}

static gchar*
pango_accessible_get_text_after_offset (AtkText         *text,
					gint            offset,
					AtkTextBoundary boundary_type,
					gint            *start_offset,
					gint            *end_offset)
{
	PangoLayout *playout;

	playout = PANGO_ACCESSIBLE (text)->playout;
	g_return_val_if_fail(playout != NULL, NULL);
	return gail_text_util_get_text (PANGO_ACCESSIBLE (text)->textutil,
					playout, GAIL_AFTER_OFFSET,
					boundary_type, offset,
					start_offset, end_offset);
}

static gchar*
pango_accessible_get_text_at_offset (AtkText         *text,
				     gint            offset,
				     AtkTextBoundary boundary_type,
				     gint            *start_offset,
				     gint            *end_offset)
{
	PangoLayout *playout;

	playout = PANGO_ACCESSIBLE (text)->playout;
	g_return_val_if_fail(playout != NULL, NULL);
	return gail_text_util_get_text (PANGO_ACCESSIBLE (text)->textutil,
					playout, GAIL_AT_OFFSET,
					boundary_type, offset, start_offset,
					end_offset);
}

static gint
pango_accessible_get_character_count (AtkText *text)
{
	PangoLayout *playout;

	playout = PANGO_ACCESSIBLE (text)->playout;
	g_return_val_if_fail(playout != NULL, 0);
	return g_utf8_strlen (pango_layout_get_text(playout), -1);
}

static gunichar
pango_accessible_get_character_at_offset (AtkText              *text,
					  gint                 offset)
{
	PangoLayout *playout;
	const gchar *string;
	gchar *index;

	playout = PANGO_ACCESSIBLE (text)->playout;
	string = (gchar *)pango_layout_get_text(playout);
	index = g_utf8_offset_to_pointer (string, offset);

	return g_utf8_get_char (index);
}

static AtkAttributeSet*
pango_accessible_get_run_attributes (AtkText *text,
				     gint    offset,
				     gint    *start_offset,
				     gint    *end_offset)
{
	AtkAttributeSet *at_set = NULL;
	PangoLayout *pl;
	PangoContext *context;
	gint dir;
	gchar *ptr;

	pl = PANGO_ACCESSIBLE(text)->playout;
	g_return_val_if_fail(pl != NULL, NULL);
	context = pango_layout_get_context(pl);
	ptr = (gchar *)pango_layout_get_text(pl);

	/*
	 * In atk the direction in defined as enum { none, ltr, rtl }, whereas
	 * in pango, it is defined as {PANGO_DIRECTION_LTR, PANGO_DIRECTION_RTL,
	 * PANGO_DIRECTION_TTB_LTR, PANGO_DIRECTION_TTB_RTL }. So add 1 to the
	 * direction obtained from Pango.
	 */
	dir = pango_context_get_base_dir(context) + 1;
	at_set = gail_misc_add_attribute (at_set, ATK_TEXT_ATTR_DIRECTION,
	g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_DIRECTION, dir)));

	at_set = gail_misc_layout_get_run_attributes (at_set, pl, ptr,
						      offset,
						      start_offset, end_offset);
	return at_set;
}

static void
pango_accessible_get_character_extents (AtkText      *text,
					gint         offset,
					gint         *x,
					gint         *y,
					gint         *width,
					gint         *height,
					AtkCoordType coords)
{
	PangoLayout *pl;
	gint index, i, j;
	GtkWidget *widget;
	AtkObject *parent;
	gchar *ptr;
	PangoRectangle char_rect;
	CDDisplay *display;
	GnomeCDText *layout;

	pl = PANGO_ACCESSIBLE(text)->playout;
	g_return_if_fail(pl != NULL);
	parent = atk_object_get_parent(ATK_OBJECT(text));
	widget = GTK_ACCESSIBLE(parent)->widget;
	g_return_if_fail(widget != NULL);

	atk_component_get_extents(ATK_COMPONENT(parent), x, y, width,
				  height, coords);
	ptr = (gchar *)pango_layout_get_text(pl);
	index = g_utf8_offset_to_pointer (ptr, offset) - ptr;
	pango_layout_index_to_pos (pl, index, &char_rect);
	gail_misc_get_extents_from_pango_rectangle (widget, &char_rect,
						    0, 0, x, y,
						    width, height, coords);
	display = CD_DISPLAY (widget);

	/*
	* Check which pango layout we are referring to.
	*/
	for (i = 0; i < CD_DISPLAY_END; i++)
		if (pango_accessible[i] == ATK_OBJECT(text))
			break;

	if (i == CD_DISPLAY_END)
		/* 
		 * Accessible object not found !!! Error
		 */
		return;

	for (j = 1; j <= i; j++) {
		/* 
		 * Add up the height depending on which pango layout it is.
		 */
		layout = (GnomeCDText *)cd_display_get_layout(display, j - 1);
		*y = *y + layout->height;
	}

}

static gint
pango_accessible_get_offset_at_point (AtkText      *text,
				      gint         x,
				      gint         y,
				      AtkCoordType coords)
{
	PangoLayout *pl;
	PangoRectangle ink_rect, logical_rect;
	AtkObject *parent;
	GtkWidget *widget;
	gchar *ptext;
	gint index;

	pl = PANGO_ACCESSIBLE(text)->playout;
	parent = atk_object_get_parent (ATK_OBJECT(text));
	widget = GTK_ACCESSIBLE (parent)->widget;

	pango_layout_get_extents(pl, &ink_rect, &logical_rect);

	index = gail_misc_get_index_at_point_in_layout (widget, pl,
							logical_rect.x,
							logical_rect.y,
							x, y, coords);
	ptext = (gchar *)pango_layout_get_text(pl);

	if (index == -1)
		return index;
	else
		return g_utf8_pointer_to_offset(ptext, ptext + index);
}

static AtkAttributeSet*
pango_accessible_get_default_attributes (AtkText *text)
{
	AtkAttributeSet *at_set = NULL;
	PangoLayout *pl;
	GtkWidget *widget;
	gint align, wrap;
	AtkObject *parent;

	pl = PANGO_ACCESSIBLE(text)->playout;
	g_return_val_if_fail(pl != NULL, NULL);

	parent = atk_object_get_parent(ATK_OBJECT(text));
	widget = GTK_ACCESSIBLE (parent)->widget;
	g_return_val_if_fail(widget != NULL, NULL);

	align = pango_layout_get_alignment(pl);
	/*
	 * In atk, align is defined as { left, right, center } whereas
	 * pango layout it is defined as { PANGO_ALIGN_LEFT, PANGO_ALIGN_CENTER,
	 * PANGO_ALIGN_RIGHT }. So change it accordingly.
	 */
	if (align == PANGO_ALIGN_CENTER)
		align = 2;
	else if (align == PANGO_ALIGN_RIGHT)
		align = 1;

	at_set = gail_misc_add_attribute (at_set, ATK_TEXT_ATTR_JUSTIFICATION,
	      g_strdup(atk_text_attribute_get_value(ATK_TEXT_ATTR_JUSTIFICATION,
						    align)));
	/*
	 * In atk, wrap is defined as { none, char, word }
	 * in pango it is defined as { PANGO_WRAP_WORD, PANGO_WRAP_CHAR }
	 */
	wrap = pango_layout_get_wrap(pl);
	if (wrap == PANGO_WRAP_WORD)
		wrap = 2;

	at_set = gail_misc_add_attribute (at_set, ATK_TEXT_ATTR_WRAP_MODE,
		g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_WRAP_MODE,
							wrap)));

	at_set = gail_misc_get_default_attributes (at_set, pl, widget);

	return at_set;
}

static gint
pango_accessible_get_caret_offset (AtkText *text)
{
	return 0;
}

/*
 * selection and caret related functions are not needed for PangoLayout.
 */
static void
atk_text_interface_init (AtkTextIface *iface)
{
	g_return_if_fail (iface != NULL);

	iface->get_text = pango_accessible_get_text;
	iface->get_text_before_offset = pango_accessible_get_text_before_offset;
	iface->get_text_after_offset = pango_accessible_get_text_after_offset;
	iface->get_text_at_offset = pango_accessible_get_text_at_offset;
	iface->get_character_count = pango_accessible_get_character_count;
	iface->get_character_at_offset =
				pango_accessible_get_character_at_offset;
	iface->get_run_attributes = pango_accessible_get_run_attributes;
	iface->get_default_attributes = pango_accessible_get_default_attributes;
	iface->get_offset_at_point = pango_accessible_get_offset_at_point;
	iface->get_character_extents = pango_accessible_get_character_extents;
	iface->get_caret_offset = pango_accessible_get_caret_offset;
}

static void
pango_accessible_get_extents (AtkComponent *component,
			      gint *x, gint *y, gint *width, gint *height,
			      AtkCoordType coords)
{
	PangoLayout *pl;
	CDDisplay *display;
	GnomeCDText *layout;
	AtkObject *parent;
	gint i, j;
	GtkWidget *widget;
	PangoRectangle ink_rect, logical_rect;

	pl = PANGO_ACCESSIBLE(component)->playout;
	g_return_if_fail(pl != NULL);
	parent = atk_object_get_parent(ATK_OBJECT(component));
	widget = GTK_ACCESSIBLE(parent)->widget;
	g_return_if_fail(widget != NULL);

	atk_component_get_extents(ATK_COMPONENT(parent), x, y, width, height,
				  coords);
	pango_layout_get_extents(pl, &ink_rect, &logical_rect);
	gail_misc_get_extents_from_pango_rectangle (widget, &logical_rect,
						    0, 0, x, y, width, height,
						    coords);
	display = CD_DISPLAY (widget);

	/*
	 * Check which pango layout is this one.
	 */
	for (i = 0; i < CD_DISPLAY_END; i++)
		if (pango_accessible[i] == ATK_OBJECT(component))
			break;

	if (i == CD_DISPLAY_END)
	/*
	 * Accessible object not found !!! Error
	 */
		return;

	for (j = 1; j <= i; j++) {
		/*
		 * Add up the height depending on which pango layout it is.
		 */
		layout = (GnomeCDText *)cd_display_get_layout(display, j - 1);
		*y = *y + layout->height;
	}
}

static void
pango_accessible_get_size (AtkComponent   *component,
			   gint           *width,
			   gint           *height)
{
	PangoLayout *playout;
	PangoRectangle ink_rect, logical_rect;

	playout = PANGO_ACCESSIBLE(component)->playout;
	g_return_if_fail(playout != NULL);

	pango_layout_get_extents(playout, &ink_rect, &logical_rect);
	*width = logical_rect.width / PANGO_SCALE;
	*height = logical_rect.height / PANGO_SCALE;
}

static void
pango_accessible_get_position (AtkComponent   *component,
			       gint           *x,
			       gint           *y,
			       AtkCoordType coords)
{
	PangoLayout *playout;
	int width, height;

	playout = PANGO_ACCESSIBLE(component)->playout;
	g_return_if_fail(playout != NULL);

	pango_accessible_get_extents(component, x, y, &width, &height, coords);
}

static gboolean
pango_accessible_contains (AtkComponent   *component,
			   gint           x,
			   gint           y,
			   AtkCoordType	  coords)
{
	PangoLayout *playout;
	gint lx, ly, width, height;

	playout = PANGO_ACCESSIBLE(component)->playout;

	pango_accessible_get_extents(component, &lx, &ly, &width, &height,
				     coords);

	/*
	 * See if the specified co-ordinates fall within the pango layout.
	 */
	if ((x >= lx) && (x <= (lx + width))  &&
	    (y >= ly) && (y <= (ly + height)))
		return TRUE;
	else
		return FALSE;
}

/*
 * set and focus related functions are not needed.
 */
static void
atk_component_interface_init(AtkComponentIface *iface)
{
	g_return_if_fail(iface != NULL);

	iface->get_extents = pango_accessible_get_extents;
	iface->get_size = pango_accessible_get_size;
	iface->get_position = pango_accessible_get_position;
	iface->contains = pango_accessible_contains;
}
