
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         gtklevelmeter.c  -  description
 *                         -------------------------------
 *   begin                : Sat Dec 23 2003
 *   copyright            : (C) 2003 by Stefan Brüns <lurch@gmx.li>
 *   description          : This file contains a GTK VU Meter.
 *
 */


#include "gtklevelmeter.h"



/* Local data */
static GtkWidgetClass *parent_class = NULL;


/* Forward declarations */
static void gtk_levelmeter_class_init (GtkLevelMeter *);

static void gtk_levelmeter_init (GtkLevelMeter *);

static void gtk_levelmeter_destroy (GtkObject *);

static gboolean gtk_levelmeter_expose (GtkWidget *,
				       GdkEventExpose *);

static void gtk_levelmeter_size_allocate (GtkWidget *,
                                          GtkAllocation *);

static void gtk_levelmeter_size_request (GtkWidget *,
					 GtkRequisition *);

static void gtk_levelmeter_realize (GtkWidget *);

static void gtk_levelmeter_paint (GtkLevelMeter *);

static void gtk_levelmeter_create_pixmap (GtkLevelMeter *);

static void gtk_levelmeter_rebuild_pixmap (GtkLevelMeter *);

static void gtk_levelmeter_allocate_colors (GArray *);

static void gtk_levelmeter_free_colors (GArray *);

static void gtk_levelmeter_set_defaultcolors (GArray *);


GType
gtk_levelmeter_get_type ()
{
  static GType levelmeter_type = 0;

  if (!levelmeter_type)
    {
      static const GTypeInfo levelmeter_info =
	{
	  sizeof (GtkLevelMeterClass),
	  NULL, /* base_init */
	  NULL, /* base_finalize */
	  (GClassInitFunc) gtk_levelmeter_class_init,
	  NULL, /* class_finalize */
	  NULL, /* class_data */
	  sizeof (GtkLevelMeter),
	  0, /* n_preallocs */
	  (GInstanceInitFunc) gtk_levelmeter_init
	};

      levelmeter_type =
	g_type_register_static (GTK_TYPE_WIDGET, "GtkLevelMeter",
				&levelmeter_info, 0);
    }

  return levelmeter_type;
}


static void
gtk_levelmeter_class_init (GtkLevelMeter *class)
{
  GObjectClass *gobject_class = NULL;
  GtkWidgetClass *widget_class = NULL;
  GtkLevelMeterClass *levelmeter_class = NULL;
  GtkObjectClass *gtkobject_class = NULL;

  gobject_class = G_OBJECT_CLASS (class);
  widget_class = (GtkWidgetClass *) class;
  levelmeter_class = (GtkLevelMeterClass *) class;
  gtkobject_class = GTK_OBJECT_CLASS (class);

  parent_class = gtk_type_class (gtk_widget_get_type ());

  gtkobject_class->destroy = gtk_levelmeter_destroy;

  widget_class->size_request = gtk_levelmeter_size_request;
  widget_class->size_allocate = gtk_levelmeter_size_allocate;
  widget_class->expose_event = gtk_levelmeter_expose;
  widget_class->realize = gtk_levelmeter_realize;
}


static void
gtk_levelmeter_init (GtkLevelMeter *lm)
{
  lm->orientation = GTK_METER_LEFT_TO_RIGHT;
  lm->showPeak = TRUE;
  lm->isSegmented = FALSE;
  lm->colorEntries = NULL;
  lm->offscreen_image = NULL;
  lm->offscreen_image_hl = NULL;
  lm->offscreen_image_dark = NULL;
  lm->level = .0;
  lm->peak = .0;
}


GtkWidget*
gtk_levelmeter_new ()
{
  GtkLevelMeter *lm = NULL;
  
  lm = gtk_type_new (gtk_levelmeter_get_type ());
    
  return GTK_WIDGET (lm);
}


static void
gtk_levelmeter_set_defaultcolors (GArray *colors)
{
  GtkLevelMeterColorEntry entry = { {0, 0, 65535, 30000}, 0.8 };

  g_array_append_val (colors, entry);
  entry.color.red = 65535; entry.stopvalue = .9;
  g_array_append_val (colors, entry);
  entry.color.green = 0; entry.stopvalue = 1.0;
  g_array_append_val (colors, entry);
}
 
 
static void
gtk_levelmeter_destroy (GtkObject *object)
{
  GtkLevelMeter *lm = NULL;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_LEVELMETER (object));

  lm = GTK_LEVELMETER (object);

  if (lm->colorEntries) {
    
    gtk_levelmeter_free_colors (lm->colorEntries);
    g_array_free (lm->colorEntries, TRUE);
    lm->colorEntries = NULL;
  }

  if (lm->offscreen_image) {

    g_object_unref (lm->offscreen_image);
    lm->offscreen_image = NULL;
  }

  if (lm->offscreen_image_hl) {

    g_object_unref (lm->offscreen_image_hl);
    lm->offscreen_image_hl = NULL;
  }

  if (lm->offscreen_image_dark) {

    g_object_unref (lm->offscreen_image_dark);
    lm->offscreen_image_dark = NULL;
  }


  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


void
gtk_levelmeter_set_level (GtkLevelMeter* lm,
			  gfloat level,
			  gfloat peak)
{
  lm->level = level;
  lm->peak = peak;
  if (GTK_WIDGET_REALIZED (lm))
    gtk_levelmeter_paint (lm);
}


void
gtk_levelmeter_set_colors (GtkLevelMeter* lm,
			   GArray *colors)
{
  unsigned int i;

  if (lm->colorEntries) {
    
    /* free old colors, if they have been allocated; delete old array */
    if (GTK_WIDGET_REALIZED (lm)) 
      gtk_levelmeter_free_colors (lm->colorEntries);
    g_array_free (lm->colorEntries, TRUE);
  }

  lm->colorEntries =
    g_array_new (FALSE, FALSE, sizeof (GtkLevelMeterColorEntry));

  /* copy array */
  for (i = 0 ; i < colors->len ; i++)
  {	
    GtkLevelMeterColorEntry* entry = &g_array_index (colors, GtkLevelMeterColorEntry, i);
    g_array_append_val (lm->colorEntries, *entry);
  }

  if (GTK_WIDGET_REALIZED (lm)) {

    gtk_levelmeter_allocate_colors (lm->colorEntries);

    /* recalc */
    gtk_levelmeter_rebuild_pixmap (lm);
    gtk_levelmeter_paint (lm);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Frees the colors allocated for the meter
 * PRE          :  The array should be the same as the one previously
 *                 used with gtk_levelmeter_allocate_colors
 */
static void
gtk_levelmeter_free_colors (GArray *colors)
{
  GdkColor *light = NULL;
  GdkColor *dark = NULL;

  unsigned int i = 0;
  
  for (i = 0 ; i < colors->len ; i++) {
    
    light = &(g_array_index (colors, GtkLevelMeterColorEntry, i).color);
    dark = &(g_array_index (colors, GtkLevelMeterColorEntry, i).darkcolor);
    gdk_colormap_free_colors (gdk_colormap_get_system (), light, 1);
    gdk_colormap_free_colors (gdk_colormap_get_system (), dark, 1);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Allocate the colors according to the entries of the array
 * PRE          :  Only the light color is used, the dark one is set automatically
 */
static void
gtk_levelmeter_allocate_colors (GArray *colors)
{
  GdkColor *light = NULL;
  GdkColor *dark = NULL;

  unsigned int i = 0;

  if (colors->len == 0)
    gtk_levelmeter_set_defaultcolors (colors);

  for (i = 0 ; i < colors->len ; i++) {
    
    light = &(g_array_index (colors, GtkLevelMeterColorEntry, i).color);
    dark = &(g_array_index (colors, GtkLevelMeterColorEntry, i).darkcolor);
    dark->red = light->red * .4;
    dark->green = light->green * .4; 
    dark->blue = light->blue * .4; 
    gdk_colormap_alloc_color (gdk_colormap_get_system (), light, FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), dark, FALSE, TRUE);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Rebuilds the light and dark base images which are used
 *                 to compose the offscreen image
 * PRE          :  /
 */
static void
gtk_levelmeter_rebuild_pixmap (GtkLevelMeter* lm)
{
  GdkGC *gc = NULL;
  GtkWidget *widget = NULL;
  gint *borders = NULL;
  gint bar_length = 0;
  gint start_x = 0;
  gint start_y = 0;
  gint width_x = 0;
  gint width_y = 0;
  unsigned int i = 0;
  
  borders = (gint *) calloc (sizeof (gint), lm->colorEntries->len + 1);
  widget = GTK_WIDGET (lm);
  gc = gdk_gc_new (GTK_LEVELMETER (lm)->offscreen_image);

  gtk_paint_box (widget->style,
		     GTK_LEVELMETER (lm)->offscreen_image_dark,
		     GTK_STATE_PRELIGHT, GTK_SHADOW_IN,
		     NULL, widget, "bar",
		     0, 0,
		     widget->allocation.width, widget->allocation.height);

  switch (lm->orientation)
  {
  case GTK_METER_BOTTOM_TO_TOP:
    bar_length = widget->allocation.height - 2 * widget->style->ythickness;
    borders[0] = widget->style->ythickness;
    break;
  case GTK_METER_LEFT_TO_RIGHT:
  default:
    bar_length = widget->allocation.width - 2 * widget->style->xthickness;
    borders[0] = widget->style->xthickness;
  }

  for (i = 0 ; i < lm->colorEntries->len ; i++) {
    
    /* Calculate position of borders */
    borders[i+1] = borders[0] + bar_length *
      g_array_index (lm->colorEntries, GtkLevelMeterColorEntry, i).stopvalue;

    switch (lm->orientation)
    {
    case GTK_METER_BOTTOM_TO_TOP:
      start_x = widget->style->xthickness;
      width_x = widget->allocation.width - 2 * widget->style->xthickness;
      width_y = borders[i+1] - borders[i];
      start_y = widget->allocation.height - width_y - borders[i];
      break;
    case GTK_METER_LEFT_TO_RIGHT:
    default:
      start_x = borders[i];
      width_x = borders[i+1] - borders[i];
      start_y = widget->style->ythickness;
      width_y = widget->allocation.height - 2 * widget->style->ythickness;
    }

    gdk_gc_set_foreground (gc, &(g_array_index (lm->colorEntries, GtkLevelMeterColorEntry, i).color) );
    gdk_draw_rectangle (GTK_LEVELMETER (lm)->offscreen_image_hl,
			gc, TRUE,
			start_x, start_y,
			width_x, width_y);
    gdk_gc_set_foreground (gc, &(g_array_index (lm->colorEntries, GtkLevelMeterColorEntry, i).darkcolor) );
    gdk_draw_rectangle (GTK_LEVELMETER (lm)->offscreen_image_dark,
			gc,
			TRUE, /* filled */
			start_x, start_y,
			width_x, width_y);
  }

  gdk_gc_unref (gc);
  free (borders);
}


static void
gtk_levelmeter_realize (GtkWidget *widget)
{
  GtkLevelMeter *lm = NULL;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LEVELMETER (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  lm = GTK_LEVELMETER (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask =
    gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y;
  widget->window =
    gdk_window_new (widget->parent->window, &attributes, attributes_mask);
  
  widget->style = gtk_style_attach (widget->style, widget->window);
  gdk_window_set_user_data (widget->window, widget);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);

  gtk_levelmeter_create_pixmap (lm);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the pixmaps in which the light and dark base image
 *                 and the offscreen image are stored
 * PRE          :  /
 */
static void
gtk_levelmeter_create_pixmap (GtkLevelMeter* lm)
{
  GtkWidget *widget = NULL;

  g_return_if_fail (GTK_IS_LEVELMETER (lm));

  if (GTK_WIDGET_REALIZED (lm)) {
    widget = GTK_WIDGET (lm);

    if (lm->offscreen_image)
      g_object_unref (lm->offscreen_image);
    if (lm->offscreen_image_hl)
      g_object_unref (lm->offscreen_image_hl);
    if (lm->offscreen_image_dark)
      g_object_unref (lm->offscreen_image_dark);

    lm->offscreen_image = gdk_pixmap_new (widget->window,
					  widget->allocation.width,
					  widget->allocation.height,
					  -1);
    lm->offscreen_image_hl = gdk_pixmap_new (widget->window,
					     widget->allocation.width,
					     widget->allocation.height,
					     -1);
    lm->offscreen_image_dark = gdk_pixmap_new (widget->window,
					       widget->allocation.width,
					       widget->allocation.height,
					       -1);
    if (lm->colorEntries == NULL)
      lm->colorEntries =
        g_array_new (FALSE, FALSE, sizeof (GtkLevelMeterColorEntry));

    gtk_levelmeter_allocate_colors (lm->colorEntries);
    gtk_levelmeter_rebuild_pixmap (lm);
    gtk_levelmeter_paint(lm);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Composes the offscreen image from the base images
 *                 according to level and peak value
 * PRE          :  /
 */
static void
gtk_levelmeter_paint (GtkLevelMeter* lm)
{
  #define PEAKSTRENGTH 3

  GtkWidget *widget = NULL;

  gint inner_width = 0;
  gint inner_height = 0;
  gint peak_start_x = 0;
  gint peak_start_y = 0;
  gint peak_width = 0;
  gint peak_height = 0;
  gint hl_start_x = 0;
  gint hl_start_y = 0;
  gint hl_width = 0;
  gint hl_height = 0;
  
  widget = GTK_WIDGET (lm);

  /* widget size minus borders */
  inner_width = widget->allocation.width - 2 * widget->style->xthickness;
  inner_height = widget->allocation.height - 2 * widget->style->ythickness;

  if (lm->peak > 1.0 ) lm->peak = 1.0;
  if (lm->level < 0 ) lm->level = 0;

  switch (lm->orientation)
  {
  case GTK_METER_BOTTOM_TO_TOP:
    peak_width = inner_width;
    peak_height = PEAKSTRENGTH;
    peak_start_x = 0;
    hl_width = inner_width;
    hl_start_y =  ( (1.0 - lm->level) * inner_height );
    peak_start_y = ( (1.0 - lm->peak) * inner_height );

    if ( (peak_start_y + peak_height) > inner_height )
      peak_height = inner_height - peak_start_y;
    if ( ( hl_start_y-PEAKSTRENGTH) <= peak_start_y ) 
      hl_start_y = peak_start_y + (PEAKSTRENGTH+1);
    hl_height = inner_height - hl_start_y;
    if ( hl_height < 0 ) hl_height = 0;
    break;
  case GTK_METER_LEFT_TO_RIGHT:
  default:
    peak_width = PEAKSTRENGTH;
    peak_height = inner_height;
    peak_start_y = 0;
    hl_start_y = 0;
    hl_width = lm->level * inner_width;
    peak_start_x = (lm->peak * inner_width) - PEAKSTRENGTH;

    if ( peak_start_x < 0 )
    {
      peak_width += peak_start_x;
      peak_start_x = 0;
    }
    hl_height = inner_height;
    if ( hl_width >= peak_start_x ) 
      hl_width = peak_start_x-1;
    if ( hl_width < 0 ) hl_width = 0;
  }

  /* offset all values with x/ythickness */
  peak_start_x += widget->style->xthickness;
  peak_start_y += widget->style->ythickness;
  hl_start_x = widget->style->xthickness;
  hl_start_y += widget->style->ythickness;

  /* fill with dark and border */
  gdk_draw_drawable (GTK_LEVELMETER (widget)->offscreen_image,
		     widget->style->black_gc,
		     GTK_LEVELMETER (widget)->offscreen_image_dark,
		     0, 0,
		     0, 0,
		     widget->allocation.width, widget->allocation.height);
  /* paint level bar */
  gdk_draw_drawable (GTK_LEVELMETER (widget)->offscreen_image,
		     widget->style->black_gc,
		     GTK_LEVELMETER (widget)->offscreen_image_hl,
		     hl_start_x, hl_start_y, 
		     hl_start_x, hl_start_y,
		     hl_width, hl_height);
  /* paint peak */
  gdk_draw_drawable (GTK_LEVELMETER (widget)->offscreen_image,
		     widget->style->black_gc,
		     GTK_LEVELMETER (widget)->offscreen_image_hl,
		     peak_start_x, peak_start_y,
		     peak_start_x, peak_start_y,
		     peak_width, peak_height);

  /* repaint */
  if (GTK_WIDGET_DRAWABLE (widget))
    gdk_draw_drawable (widget->window,
		       widget->style->black_gc,
		       GTK_LEVELMETER (widget)->offscreen_image,
		       0, 0,
		       0, 0,
		       widget->allocation.width, widget->allocation.height);

  gtk_widget_queue_draw_area (widget,
                              0, 0,
                              widget->allocation.width, widget->allocation.height);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Sets the requisition to the minimun useful values depending
 *                 on the orientation and the border sizes
 * PRE          :  /
 */
static void 
gtk_levelmeter_size_request (GtkWidget *widget,
			     GtkRequisition *requisition)
{
  GtkLevelMeter *lm = NULL;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LEVELMETER (widget));

  lm = GTK_LEVELMETER (widget);

  switch (lm->orientation)
  {
  case GTK_METER_BOTTOM_TO_TOP:
    requisition->width = 4;
    requisition->height = 100;
    break;
  case GTK_METER_LEFT_TO_RIGHT:
  default:
    requisition->width = 100;
    requisition->height = 4;
  }
  requisition->width += 2 * widget->style->xthickness;
  requisition->height += 2 * widget->style->ythickness;
}


static void
gtk_levelmeter_size_allocate (GtkWidget *widget,
			      GtkAllocation *allocation)
{
  GtkLevelMeter* lm = NULL;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LEVELMETER (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget)) {

    lm = GTK_LEVELMETER (widget);

    gdk_window_move_resize (widget->window,
                            allocation->x, allocation->y,
			    allocation->width, allocation->height);

    gtk_levelmeter_create_pixmap (lm);
  }
}


/* DESCRIPTION  :  Get called when the widget has to be redrawn
 * BEHAVIOR     :  The widget gets redrawn from an offscreen image
 * PRE          :  /
 */
static gboolean
gtk_levelmeter_expose (GtkWidget *widget,
                       GdkEventExpose *event)
{
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_LEVELMETER (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->count > 0)
    return FALSE;

  /* repaint */
  if (GTK_WIDGET_DRAWABLE (widget))
    gdk_draw_drawable (widget->window,
		       widget->style->black_gc,
		       GTK_LEVELMETER (widget)->offscreen_image,
		       event->area.x, event->area.y,
		       event->area.x, event->area.y,
		       event->area.width,
		       event->area.height);

  return FALSE;
}




