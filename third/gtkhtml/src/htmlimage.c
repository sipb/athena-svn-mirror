/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
   Copyright (C) 1999 Red Hat Software
   Copyright (C) 1999, 2000 Helix Code, Inc.
   
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

   TODO:

     - implement proper animation loading (now it loops thru loaded frames
       and does not stop when loading is in progress and we are out of frames)
     - look at gdk-pixbuf to make gdk_pixbuf_compose work (look also
       on gk_pixbuf_render_to_drawable_alpha)
     - take care about all the frame->action values

*/

#include <config.h>
#include <glib.h>
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include "gtkhtml.h"
#include "gtkhtml-properties.h"
#include "gtkhtml-stream.h"

#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmldrawqueue.h"
#include "htmlengine.h"
#include "htmlengine-save.h"
#include "htmlenumutils.h"
#include "htmlimage.h"
#include "htmlobject.h"
#include "htmlmap.h"
#include "htmlprinter.h"
#include "htmlgdkpainter.h"

/* HTMLImageFactory stuff.  */

struct _HTMLImageFactory {
	HTMLEngine *engine;
	GHashTable *loaded_images;
};


#define DEFAULT_SIZE 48
#define ANIMATIONS(i) GTK_HTML_CLASS (GTK_OBJECT (i->image_ptr->factory->engine->widget)->klass)->properties->animations
#define STRDUP_HELPER(i,j) if (i != j) {char *tmp = g_strdup (j); g_free(i); i = tmp;}

HTMLImageClass html_image_class;
static HTMLObjectClass *parent_class = NULL;

static HTMLImageAnimation *html_image_animation_new     (HTMLImage *image);
static void                html_image_animation_destroy (HTMLImageAnimation *anim);
static HTMLImagePointer   *html_image_pointer_new       (const char *filename, HTMLImageFactory *factory);
static void                html_image_pointer_ref       (HTMLImagePointer *ip);
static void                html_image_pointer_unref     (HTMLImagePointer *ip);
static gboolean            html_image_pointer_timeout   (HTMLImagePointer *ip);
static void                render_cur_frame             (HTMLImage *image, gint nx, gint ny, const GdkColor *highlight_color);
static guint               get_actual_height            (HTMLImage *image, HTMLPainter *painter);


static guint
get_actual_width (HTMLImage *image,
		  HTMLPainter *painter)
{
	GdkPixbuf *pixbuf = image->image_ptr->pixbuf;
	GdkPixbufAnimation *anim = image->image_ptr->animation;
	gint width;

	if (image->percent_width) {
		/* The cast to `gdouble' is to avoid overflow (eg. when
                   printing).  */
		width = ((gdouble) html_engine_get_view_width (image->image_ptr->factory->engine)
			 * image->specified_width) / 100;
	} else if (image->specified_width > 0) {
		width = image->specified_width * html_painter_get_pixel_size (painter);
	} else if (image->image_ptr == NULL || pixbuf == NULL) {
		width = DEFAULT_SIZE * html_painter_get_pixel_size (painter);
	} else {
		width = (((anim) ? gdk_pixbuf_animation_get_width (anim) : gdk_pixbuf_get_width (pixbuf))
			  * html_painter_get_pixel_size (painter));

		if (image->specified_height > 0 || image->percent_height) {
			double scale;

			scale =  ((double) get_actual_height (image, painter)) 
				/ (((anim) ? gdk_pixbuf_animation_get_height (anim) : gdk_pixbuf_get_height (pixbuf))
				   * html_painter_get_pixel_size (painter));
			
			width *= scale;
		}

	}

	return width;
}


static guint
get_actual_height (HTMLImage *image,
		   HTMLPainter *painter)
{
	GdkPixbuf *pixbuf = image->image_ptr->pixbuf;
	GdkPixbufAnimation *anim = image->image_ptr->animation;
	gint height;
		
	if (image->percent_height) {
		/* The cast to `gdouble' is to avoid overflow (eg. when
                   printing).  */
		height = ((gdouble) html_engine_get_view_height (image->image_ptr->factory->engine)
			  * image->specified_height) / 100;
	} else if (image->specified_height > 0) {
		height = image->specified_height * html_painter_get_pixel_size (painter);
	} else if (image->image_ptr == NULL || pixbuf == NULL) {
		height = DEFAULT_SIZE * html_painter_get_pixel_size (painter);
	} else {
		height = (((anim) ? gdk_pixbuf_animation_get_height (anim) : gdk_pixbuf_get_height (pixbuf))
			  * html_painter_get_pixel_size (painter));

		if (image->specified_width > 0 || image->percent_width) {
			double scale;
			
			scale = ((double) get_actual_width (image, painter))
				/ (((anim) ? gdk_pixbuf_animation_get_width (anim) : gdk_pixbuf_get_width (pixbuf))
				   * html_painter_get_pixel_size (painter));
			
			height *= scale;
		} 
	}

	return height;
}


/* HTMLObject methods.  */

/* FIXME: We should close the stream here, too.  But in practice we cannot
   because the stream pointer might be invalid at this point, and there is no
   way to set it to NULL when the stream is closed.  This clearly sucks and
   must be fixed.  */
static void
destroy (HTMLObject *o)
{
	HTMLImage *image = HTML_IMAGE (o);

	html_image_factory_unregister (image->image_ptr->factory,
				       image->image_ptr, HTML_IMAGE (image));

	if (image->animation)
		html_image_animation_destroy (image->animation);

	g_free (image->url);
	g_free (image->target);
	g_free (image->alt);
	g_free (image->usemap);
	g_free (image->final_url);

	if (image->color)
		html_color_unref (image->color);

	HTML_OBJECT_CLASS (parent_class)->destroy (o);
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	HTMLImage *dimg = HTML_IMAGE (dest);
	HTMLImage *simg = HTML_IMAGE (self);

	/* FIXME not sure this is all correct.  */

	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	dimg->image_ptr = simg->image_ptr;
	dimg->animation = NULL;          /* don't bother with animation copying now. TODO */
	dimg->color = simg->color;
	if (simg->color)
		html_color_ref (dimg->color);

	dimg->have_color = simg->have_color;

	dimg->border = simg->border;

	dimg->specified_width = simg->specified_width;
	dimg->specified_height = simg->specified_height;
	dimg->percent_width = simg->percent_width;
	dimg->percent_height = simg->percent_height;
	dimg->ismap = simg->ismap;

	dimg->hspace = simg->hspace;
	dimg->vspace = simg->vspace;

	dimg->valign = simg->valign;

	dimg->url = g_strdup (simg->url);
	dimg->target = g_strdup (simg->target);
	dimg->alt = g_strdup (simg->alt);
	dimg->usemap = g_strdup (simg->usemap);
	dimg->final_url = NULL;

	/* add dest to image_ptr interests */
	dimg->image_ptr->interests = g_slist_prepend (dimg->image_ptr->interests, dimg);
	html_image_pointer_ref (dimg->image_ptr);
}

static void 
image_update_url (HTMLImage *image, gint x, gint y)
{
	HTMLMap *map;
	HTMLObject *o = HTML_OBJECT (image);
	char *url = NULL;

	/* 
	 * FIXME this is a huge hack waiting until we implement events for now we write
	 * over the image->url for every point since we always call point before get_url
	 * it is sick, I know.
	 */
	if (image->usemap != NULL) {
		map = (HTMLMap *)html_engine_get_object_by_id (image->image_ptr->factory->engine, 
							       image->usemap + 1);
		
		if (map) {
			url = html_map_calc_point (map, x - o->x , y - (o->y - o->ascent));
			
			if (url)
				url = g_strdup (url);
		}
	} else if (image->ismap) {
		if (image->url)
			url = g_strdup_printf ("%s?%d,%d", image->url, x - o->x, y - (o->y - o->ascent));
	} else {
		return;
	}
	
	g_free (image->final_url);
	image->final_url = url;
}

static HTMLObject *
check_point (HTMLObject *self,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	if ((x >= self->x)
	    && (x < (self->x + self->width))
	    && (y >= (self->y - self->ascent))
	    && (y < (self->y + self->descent))) {
		if (offset_return != NULL)
			*offset_return = x - self->x < self->width / 2 ? 0 : 1;
		
		image_update_url (HTML_IMAGE (self), x, y);
		return self;
	}
	
	return NULL;
}

static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
	HTMLImage *image = HTML_IMAGE (o);
	guint pixel_size;
	guint min_width;

	pixel_size = html_painter_get_pixel_size (painter);

	if (image->percent_width || image->percent_height)
		min_width = pixel_size;
	else
		min_width = get_actual_width (HTML_IMAGE (o), painter);

	min_width += (image->border * 2 + 2 * image->hspace) * pixel_size;

	return min_width;
}

static gint
calc_preferred_width (HTMLObject *o,
		      HTMLPainter *painter)
{
	HTMLImage *image = HTML_IMAGE (o);
	guint width;

	width = get_actual_width (HTML_IMAGE (o), painter)
		+ (image->border * 2 + 2 * image->hspace) * html_painter_get_pixel_size (painter);

	return width;
}

static gboolean
calc_size (HTMLObject *o, HTMLPainter *painter, GList **changed_objs)
{
	HTMLImage *image;
	guint pixel_size;
	guint width, height;
	gint old_width, old_ascent, old_descent;

	old_width = o->width;
	old_ascent = o->ascent;
	old_descent = o->descent;

	image = HTML_IMAGE (o);

	pixel_size = html_painter_get_pixel_size (painter);

	width = get_actual_width (image, painter);
	height = get_actual_height (image, painter);

	o->width  = width + (image->border + image->hspace) * 2 * pixel_size;
	o->ascent = height + (image->border + image->vspace) * 2 * pixel_size;
	o->descent = 0;

	if (o->descent != old_descent
	    || o->ascent != old_ascent
	    || o->width != old_width)
		return TRUE;

	return FALSE;
}

static void
draw (HTMLObject *o,
      HTMLPainter *painter,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLImage *image;
	GdkPixbuf *pixbuf;
	gint base_x, base_y;
	gint scale_width, scale_height;
	GdkColor *highlight_color;
	guint pixel_size;
	ArtIRect paint;

	/* printf ("Image::draw\n"); */

	html_object_calc_intersection (o, &paint, x, y, width, height);
	if (art_irect_empty (&paint))
		return;

	image = HTML_IMAGE (o);

	pixbuf = image->image_ptr->pixbuf;
	pixel_size = html_painter_get_pixel_size (painter);

	if (o->selected) {
		highlight_color = &html_colorset_get_color_allocated
			(painter, painter->focus ? HTMLHighlightColor : HTMLHighlightNFColor)->color;
	} else
		highlight_color = NULL;

	if (pixbuf == NULL) {
		gint vspace, hspace;

		hspace = image->hspace * pixel_size;
		vspace = image->vspace * pixel_size;

		if (image->image_ptr->loader && !image->image_ptr->stall) 
			return;

		if (o->selected) {
			html_painter_set_pen (painter, highlight_color);
			html_painter_fill_rect (painter, 
						o->x + tx + hspace,
						o->y + ty - o->ascent + vspace,
						o->width - 2 * hspace,
						o->ascent + o->descent - 2 * vspace);
		}
		html_painter_draw_panel (painter,
					 &((html_colorset_get_color (painter->color_set, HTMLBgColor))->color),
					 o->x + tx + hspace,
					 o->y + ty - o->ascent + vspace,
					 o->width - 2 * hspace,
					 o->ascent + o->descent - 2 * vspace,
					 GTK_HTML_ETCH_IN, 1);
		return;
	}
	base_x = o->x + tx + (image->border + image->hspace) * pixel_size;
	base_y = o->y + ty + (image->border + image->vspace) * pixel_size - o->ascent;

	scale_width = get_actual_width (image, painter);
	scale_height = get_actual_height (image, painter);

	if (image->border) {
		if (image->have_color) {
			html_color_alloc (image->color, painter);
			html_painter_set_pen (painter, &image->color->color);
		}
		
		html_painter_draw_panel (painter,
					 &((html_colorset_get_color (painter->color_set, HTMLBgColor))->color),
					 base_x - image->border * pixel_size,
					 base_y - image->border * pixel_size,
					 scale_width + (2 * image->border) * pixel_size,
					 scale_height + (2 * image->border) * pixel_size,
					 GTK_HTML_ETCH_NONE, image->border);
		
	}
	if (ANIMATIONS (image) &&  image->animation && ! HTML_IS_PRINTER (painter)) {
		image->animation->active = TRUE;
		image->animation->x = base_x;
		image->animation->y = base_y;
		image->animation->ex = image->image_ptr->factory->engine->x_offset;
		image->animation->ey = image->image_ptr->factory->engine->y_offset;

		render_cur_frame (image, base_x, base_y, highlight_color);
	} else {
		html_painter_draw_pixmap (painter, pixbuf,
					  base_x, base_y,
					  scale_width, scale_height,
					  highlight_color);
	}
}

gchar *
html_image_resolve_image_url (GtkHTML *html, gchar *image_url)
{
	gchar *url = NULL;

	/* printf ("html_image_resolve_image_url %p\n", html->editor_api); */
	if (html->editor_api) {
		GtkArg *args [2];

		args [0] = gtk_arg_new (GTK_TYPE_STRING);
		GTK_VALUE_STRING (*args [0]) = image_url;

		args [1] = (* html->editor_api->event) (html, GTK_HTML_EDITOR_EVENT_IMAGE_URL, args, html->editor_data);
		gtk_arg_free (args [0], FALSE);

		if (args [1]) {
			if (args [1]->type == GTK_TYPE_STRING)
				url = GTK_VALUE_STRING (*args [1]);
			gtk_arg_free (args [1], FALSE);
		}
	}
	if (!url)
		url = g_strdup (image_url);
	/* printf ("image URL resolved to: %s (from: %s)\n", url, image_url); */

	return url;
}

static gboolean
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	HTMLImage *image;
	gchar *url;
	gboolean result;

	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (state != NULL, FALSE);

	image  = HTML_IMAGE (self);

	url    = html_image_resolve_image_url (state->engine->widget, image->image_ptr->url);
	result = html_engine_save_output_string (state, "<IMG SRC=\"%s\"", url);
	g_free (url);
	if (!result)
		return FALSE;	

	if (image->percent_width) {
		if (!html_engine_save_output_string (state, " WIDTH=\"%d\%\"", image->specified_width))
			return FALSE;
	} else if (image->specified_width > 0) {
		if (!html_engine_save_output_string (state, " WIDTH=\"%d\"", image->specified_width))
			return FALSE;
	}

	if (image->percent_height) {
		if (!html_engine_save_output_string (state, " HEIGHT=\"%d\%\"", image->specified_height))
			return FALSE;
	} else if (image->specified_height > 0) {
		if (!html_engine_save_output_string (state, " HEIGHT=\"%d\"", image->specified_height))
			return FALSE;
	}

	if (image->vspace) {
		if (!html_engine_save_output_string (state, " VSPACE=\"%d\"", image->vspace))
			return FALSE;
	}

	if (image->hspace) {
		if (!html_engine_save_output_string (state, " HSPACE=\"%d\"", image->hspace))
			return FALSE;
	}

	if (image->vspace) {
		if (!html_engine_save_output_string (state, " VSPACE=\"%d\"", image->vspace))
			return FALSE;
	}

	if (image->valign != HTML_VALIGN_NONE) {
		if (!html_engine_save_output_string (state, " ALIGN=\"%s\"", html_valign_name (image->valign)))
			return FALSE;
	}

	if (image->alt) {
		if (!html_engine_save_output_string (state, " ALT=\"%s\"", image->alt))
			return FALSE;
	}

	/* FIXME this is the default set in htmlengine.c but there is no real way to tell
	 * if the usr specified it directly
	 */
	if (image->border != 2) {
		if (!html_engine_save_output_string (state, " BORDER=\"%d\"", image->border))
			return FALSE;
	}

	if (!html_engine_save_output_string (state, ">"))
		return FALSE;
	
	return TRUE;
}

static gboolean
save_plain (HTMLObject *self,
	    HTMLEngineSaveState *state,
	    gint requested_width)
{
	HTMLImage *image;
	gboolean rv = TRUE;

	image = HTML_IMAGE (self);
	
	if (image->alt)
		rv = html_engine_save_output_string (state, "%s", image->alt);

	return rv;
}

static const gchar *
get_url (HTMLObject *o)
{
	HTMLImage *image;

	image = HTML_IMAGE (o);
	return image->final_url ? image->final_url : image->url;
}

static const gchar *
get_target (HTMLObject *o)
{
	HTMLImage *image;

	image = HTML_IMAGE (o);
	return image->target;
}

static const gchar *
get_src (HTMLObject *o)
{
	HTMLImage *image;
	
	image = HTML_IMAGE (o);
	return image->image_ptr->url;
}

static HTMLObject *
set_link (HTMLObject *self, HTMLColor *color, const gchar *url, const gchar *target)
{
	HTMLImage *image = HTML_IMAGE (self);

	STRDUP_HELPER (image->url, url);
	STRDUP_HELPER (image->target, target);

	return NULL;
}

static gboolean
accepts_cursor (HTMLObject *o)
{
	return TRUE;
}

static HTMLVAlignType
get_valign (HTMLObject *self)
{
	HTMLImage *image;

	image = HTML_IMAGE (self);

	return image->valign;
}

static gboolean
select_range (HTMLObject *self,
	      HTMLEngine *engine,
	      guint offset,
	      gint length,
	      gboolean queue_draw)
{
	if ((*parent_class->select_range) (self, engine, offset, length, queue_draw)) {
		if (queue_draw)
			html_engine_queue_draw (engine, self);
		return TRUE;
	} else
		return FALSE;
}


void
html_image_type_init (void)
{
	html_image_class_init (&html_image_class, HTML_TYPE_IMAGE, sizeof (HTMLImage));
}

void
html_image_class_init (HTMLImageClass *image_class,
		       HTMLType type,
		       guint size)
{
	HTMLObjectClass *object_class;

	object_class = HTML_OBJECT_CLASS (image_class);

	html_object_class_init (object_class, type, size);

	object_class->copy = copy;
	object_class->draw = draw;	
	object_class->destroy = destroy;
	object_class->calc_min_width = calc_min_width;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->calc_size = calc_size;
	object_class->check_point = check_point;
	object_class->get_url = get_url;
	object_class->get_target = get_target;
	object_class->get_src = get_src;
	object_class->set_link = set_link;
	object_class->accepts_cursor = accepts_cursor;
	object_class->get_valign = get_valign;
	object_class->save = save;
	object_class->save_plain = save_plain;
	object_class->select_range = select_range;

	parent_class = &html_object_class;
}

void
html_image_init (HTMLImage *image,
		 HTMLImageClass *klass,
		 HTMLImageFactory *imf,
		 const gchar *filename,
		 const gchar *url,
		 const gchar *target,
		 gint16 width, gint16 height,
		 gboolean percent_width, gboolean percent_height,
		 gint8 border,
		 HTMLColor *color,
		 HTMLVAlignType valign,
		 gboolean reload)
{
	HTMLObject *object;

	g_assert (filename);

	object = HTML_OBJECT (image);

	html_object_init (object, HTML_OBJECT_CLASS (klass));

	image->url = g_strdup (url);
	image->target = g_strdup (target);
	image->usemap = NULL;
	image->final_url = NULL;
	image->ismap = FALSE;

	image->specified_width  = width;
	image->specified_height = height;
	image->percent_width    = percent_width;
	image->percent_height   = percent_height;
	image->border           = border;

	if (color) {
		image->color = color;
		image->have_color = TRUE;
		html_color_ref (color);
	} else {
		image->color = NULL;
		image->have_color = FALSE;
	}

	image->animation = NULL;
	image->alt = NULL;

	image->hspace = 0;
	image->vspace = 0;

	if (valign == HTML_VALIGN_NONE)
		valign = HTML_VALIGN_BOTTOM;
	image->valign = valign;

	image->image_ptr = html_image_factory_register (imf, image, filename, reload);
}

HTMLObject *
html_image_new (HTMLImageFactory *imf,
		const gchar *filename,
		const gchar *url,
		const gchar *target,
		gint16 width, gint16 height,
		gboolean percent_width, gboolean percent_height,
		gint8 border,	
		HTMLColor *color,
		HTMLVAlignType valign,
		gboolean reload)
{
	HTMLImage *image;

	image = g_new(HTMLImage, 1);

	html_image_init (image, &html_image_class,
			 imf,
			 filename,
			 url,
			 target,
			 width, height,
			 percent_width, percent_height,
			 border,
			 color,
			 valign,
			 reload);

	return HTML_OBJECT (image);
}

void
html_image_set_spacing (HTMLImage *image, gint hspace, gint vspace)
{
	gboolean changed = FALSE;

	if (image->hspace != hspace) {
		image->hspace = hspace;
		changed = TRUE;
	}

	if (image->vspace != vspace) {
		image->vspace = vspace;
		changed = TRUE;
	}

	if (changed) {
		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_engine_schedule_update (image->image_ptr->factory->engine);
	}
}

void
html_image_edit_set_url (HTMLImage *image, const gchar *url)
{
	if (url) {
		HTMLImageFactory *imf = image->image_ptr->factory;

		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_image_factory_unregister (imf, image->image_ptr, HTML_IMAGE (image));
		image->image_ptr = html_image_factory_register (imf, image, url, TRUE);
		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_engine_schedule_update (imf->engine);
	}
}

void
html_image_set_url (HTMLImage *image, const gchar *url)
{
	if (url && strcmp (image->image_ptr->url, url)) {
		HTMLImageFactory *imf = image->image_ptr->factory;

		html_image_factory_unregister (imf, image->image_ptr, HTML_IMAGE (image));
		image->image_ptr = html_image_factory_register (imf, image, url, FALSE);
	}
}

void
html_image_set_valign (HTMLImage *image, HTMLVAlignType valign)
{
	if (image->valign != valign) {
		image->valign = valign;
		html_engine_schedule_update (image->image_ptr->factory->engine);
	}
}

void
html_image_set_border (HTMLImage *image, gint border)
{
	if (image->border != border) {
		image->border = border;
		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_engine_schedule_update (image->image_ptr->factory->engine);
	}
}

void
html_image_set_alt (HTMLImage *image, gchar *alt)
{
	g_free (image->alt);
	image->alt = g_strdup (alt);
}

void
html_image_set_map (HTMLImage *image, gchar *usemap, gboolean ismap)
{
	char *url = NULL;

	g_free (image->usemap);

	if (usemap != NULL) {
		image->ismap = FALSE;
		url = g_strdup (usemap);
	} else {
		image->ismap = ismap;
	}
	image->usemap = url;
}

void
html_image_set_size (HTMLImage *image, gint w, gint h, gboolean pw, gboolean ph)
{
	gboolean changed = FALSE;

	if (pw != image->percent_width) {
		image->percent_width = pw;
		changed = TRUE;
	}

	if (ph != image->percent_height) {
		image->percent_height = ph;
		changed = TRUE;
	}

	if (w != image->specified_width) {
		image->specified_width = w;
		changed = TRUE;
	}

	if (h != image->specified_height) {
		image->specified_height = h;
		changed = TRUE;
	}

	if (changed) {
		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_engine_schedule_update (image->image_ptr->factory->engine);
	}
}

char *image_content_types[] = {"image/*", NULL};

static char **
html_image_factory_types (GtkHTMLStream *stream,
			  gpointer user_data)
{
	/* FIXME: this should use the not currently existant function
	 * in gdk-pixbuf that tells us what mime types are supported 
	 * by the modules it has loaded.
	 */
	return image_content_types;
}

static void
update_or_redraw (HTMLImagePointer *ip)
{
	GSList *list;
	gboolean update = FALSE;

	for (list = ip->interests; list; list = list->next) {
		if (list->data == NULL)
			update = TRUE;
		else {
			HTMLImage *image = HTML_IMAGE (list->data);
			gint pixel_size = html_painter_get_pixel_size (ip->factory->engine->painter);
			gint w, h;

			w = get_actual_width (image, ip->factory->engine->painter)
				+ (image->border * 2 + 2 * image->hspace) * pixel_size;
			h = get_actual_height (image, ip->factory->engine->painter)
				+ (image->border * 2 + 2 * image->vspace) * pixel_size;

			/* printf ("%dx%d  <-->  %dx%d\n", w, h, HTML_OBJECT (list->data)->width,
			   HTML_OBJECT (list->data)->ascent + HTML_OBJECT (list->data)->descent); */

			if (w != HTML_OBJECT (list->data)->width
			    || h != HTML_OBJECT (list->data)->ascent + HTML_OBJECT (list->data)->descent) {
				html_object_change_set (HTML_OBJECT (list->data), HTML_CHANGE_ALL_CALC);
				update = TRUE;
			}
		}
	}

	if (!update) {
		/* printf ("REDRAW\n"); */
		for (list = ip->interests; list; list = list->next)
			if (list->data) // && html_object_is_visible (HTML_OBJECT (list->data)))
				html_engine_queue_draw (ip->factory->engine, HTML_OBJECT (list->data));
		if (ip->interests)
			html_engine_flush_draw_queue (ip->factory->engine);
	} else {
		/* printf ("UPDATE\n"); */
		html_engine_schedule_update (ip->factory->engine);
	}
}

static void
html_image_factory_end_pixbuf (GtkHTMLStream *stream,
			       GtkHTMLStreamStatus status,
			       gpointer user_data)
{
	HTMLImagePointer *ip = user_data;

	gdk_pixbuf_loader_close (ip->loader);
	if (!ip->animation && !ip->pixbuf) {
		ip->pixbuf = gdk_pixbuf_loader_get_pixbuf (ip->loader);

		if (ip->pixbuf)
			gdk_pixbuf_ref (ip->pixbuf);
	}

	update_or_redraw (ip);
	
	html_image_pointer_unref (ip);
}

static void
html_image_factory_write_pixbuf (GtkHTMLStream *stream,
				 const gchar *buffer,
				 size_t size,
				 gpointer user_data)
{
	HTMLImagePointer *p = user_data;

	/* FIXME ! Check return value */
	gdk_pixbuf_loader_write (p->loader, buffer, size);
}

static void
html_image_factory_area_updated (GdkPixbufLoader *loader, guint x, guint y, guint width, guint height)
{

}

static void
html_image_factory_area_prepared (GdkPixbufLoader *loader, HTMLImagePointer *ip)
{
	if (!ip->animation) {
		ip->pixbuf = gdk_pixbuf_loader_get_pixbuf (ip->loader);
		g_assert (ip->pixbuf);

		gdk_pixbuf_ref (ip->pixbuf);
		update_or_redraw (ip);
	}
}

static void
render_cur_frame (HTMLImage *image, gint nx, gint ny, const GdkColor *highlight_color)
{
	GdkPixbufFrame    *frame;
	HTMLPainter       *painter;
	HTMLImageAnimation *anim = image->animation;
	GdkPixbufAnimation *ganim = image->image_ptr->animation;
	GList *cur = gdk_pixbuf_animation_get_frames (ganim);
	gint w, h;
	gboolean saved_alpha = TRUE;

	painter = image->image_ptr->factory->engine->painter;

	frame = (GdkPixbufFrame *) anim->cur_frame->data;
	/* printf ("w: %d h: %d action: %d\n", w, h, frame->action); */

	/* FIXME this is hack to turn off alpha blending while rending 
	 * animations.  This breaks nothing since gdk-pixbuf doesn't support animations
	 * for anything but gif and it makes a huge difference if there are lots of
	 * frames.  We really should add a parameter to the html_painter_draw_pixmap
	 * call and track wether each image has full alpha or bilevel alpha, but that
	 * will wait for a bit.
	 */  
	if (HTML_IS_GDK_PAINTER (painter)) {
		saved_alpha = HTML_GDK_PAINTER (painter)->alpha;
		HTML_GDK_PAINTER (painter)->alpha = FALSE;
	}

	do {
		frame = (GdkPixbufFrame *) cur->data;
		w = gdk_pixbuf_get_width (gdk_pixbuf_frame_get_pixbuf (frame));
		h = gdk_pixbuf_get_height (gdk_pixbuf_frame_get_pixbuf (frame));
		if (anim->cur_frame == cur) {
			html_painter_draw_pixmap (painter, gdk_pixbuf_frame_get_pixbuf (frame),
						  nx + gdk_pixbuf_frame_get_x_offset (frame),
						  ny + gdk_pixbuf_frame_get_y_offset (frame),
						  w, h,
						  highlight_color);
			break;
		} else if (gdk_pixbuf_frame_get_action (frame) == GDK_PIXBUF_FRAME_RETAIN) {
			html_painter_draw_pixmap (painter, gdk_pixbuf_frame_get_pixbuf (frame),
						  nx + gdk_pixbuf_frame_get_x_offset (frame),
						  ny + gdk_pixbuf_frame_get_y_offset (frame),
						  w, h,
						  highlight_color);
		} 
		cur = cur->next;
	} while (1);

	if (HTML_IS_GDK_PAINTER (painter)) {
		HTML_GDK_PAINTER (painter)->alpha = saved_alpha;
	}

}

static gint
html_image_animation_timeout (HTMLImage *image)
{
	HTMLImageAnimation *anim = image->animation;
	GdkPixbufAnimation *ganim = image->image_ptr->animation;
	GdkPixbufFrame    *frame;
	HTMLEngine        *engine;
	gint nx, ny, nex, ney;

	/* printf ("animation_timeout\n"); */
	anim->cur_frame = anim->cur_frame->next;
	if (!anim->cur_frame)
		anim->cur_frame = gdk_pixbuf_animation_get_frames (image->image_ptr->animation);

	frame = (GdkPixbufFrame *) anim->cur_frame->data;
	
	/* draw only if animation is active - onscreen */

	engine = image->image_ptr->factory->engine;

	nex = engine->x_offset;
	ney = engine->y_offset;

	nx = anim->x - (nex - anim->ex);
	ny = anim->y - (ney - anim->ey);
	
	if (anim->active) {
		gint aw, ah;

		aw = gdk_pixbuf_animation_get_width (ganim);
		ah = gdk_pixbuf_animation_get_height (ganim);

		if (MAX(0, nx) < MIN(engine->width, nx+aw)
		    && MAX(0, ny) < MIN(engine->height, ny+ah)) {
			html_engine_draw (engine,
					  nx, ny,
					  aw, ah);
			/* html_engine_queue_draw (engine, HTML_OBJECT (image)); */
			
		}
		
	}
	/* printf ("timeout: %d\n", gdk_pixbuf_frame_get_delay_time (frame)); */
	anim->timeout = g_timeout_add (10 * (gdk_pixbuf_frame_get_delay_time (frame) > 0
					     ? gdk_pixbuf_frame_get_delay_time (frame) : 1),
				       (GtkFunction) html_image_animation_timeout, (gpointer) image);

	return FALSE;
}

static void
html_image_animation_start (HTMLImage *image)
{
	HTMLImageAnimation *anim = image->animation;

	if (anim && gdk_pixbuf_animation_get_num_frames (image->image_ptr->animation) > 1) {
		if (anim->timeout == 0) {
			GList *frames = gdk_pixbuf_animation_get_frames (image->image_ptr->animation);

			anim->cur_frame = frames->next;
			anim->cur_n = 1;
			anim->timeout = g_timeout_add (10 * gdk_pixbuf_frame_get_delay_time
						       ((GdkPixbufFrame *) frames->data),
						       (GtkFunction) html_image_animation_timeout, (gpointer) image);
		}
	}
}

static void
html_image_animation_stop (HTMLImageAnimation *anim)
{
	if (anim->timeout) {
		g_source_remove (anim->timeout);
		anim->timeout = 0;
	}
	anim->active  = 0;
}

static void
html_image_factory_frame_done (GdkPixbufLoader *loader, GdkPixbufFrame *frame, HTMLImagePointer *ip)
{
	/* printf ("frame done\n"); */

	if (!ip->animation) {
		ip->animation = gdk_pixbuf_loader_get_animation (loader);
		gdk_pixbuf_animation_ref (ip->animation);
	}
	g_assert (ip->animation);

	if (gdk_pixbuf_animation_get_num_frames (ip->animation) > 1) {
		GSList *cur = ip->interests;
		HTMLImage *image;

		while (cur) {
			if (cur->data) {
				image = HTML_IMAGE (cur->data);
				if (!image->animation) {
					image->animation = html_image_animation_new (image);
				}
				html_image_animation_start (image);
			}
			cur = cur->next;
		}
	}
}

static void
html_image_factory_animation_done (GdkPixbufLoader *loader, HTMLImagePointer *ip)
{
	g_print ("animation done\n");
}

HTMLImageFactory *
html_image_factory_new (HTMLEngine *e)
{
	HTMLImageFactory *retval;
	retval = g_new (HTMLImageFactory, 1);
	retval->engine = e;
	retval->loaded_images = g_hash_table_new (g_str_hash, g_str_equal);

	return retval;
}

static gboolean
cleanup_images (gpointer key, gpointer value, gpointer user_data)
{
	HTMLImagePointer *ptr;
	gboolean retval = FALSE;

	ptr = value;

	/* user data means: NULL only clean, non-NULL free */
	if (user_data){
		if (ptr->interests != NULL) {
			g_slist_free (ptr->interests);
			ptr->interests = NULL;
		}
	}

	/* clean only if this image is not used anymore */
	if (!ptr->interests){
		retval = TRUE;
		html_image_pointer_unref (ptr);
	}

	return retval;
}

void
html_image_factory_cleanup (HTMLImageFactory *factory)
{
	g_return_if_fail (factory);
	g_hash_table_foreach_remove (factory->loaded_images, cleanup_images, NULL);
}

void
html_image_factory_free (HTMLImageFactory *factory)
{
	g_return_if_fail (factory);

	g_hash_table_foreach_remove (factory->loaded_images, cleanup_images, factory);
	g_hash_table_destroy (factory->loaded_images);
	g_free (factory);
}

static HTMLImageAnimation *
html_image_animation_new (HTMLImage *image)
{
	HTMLImageAnimation *animation;

	animation = g_new (HTMLImageAnimation, 1);
	animation->cur_frame = gdk_pixbuf_animation_get_frames (image->image_ptr->animation);
	animation->cur_n = 0;
	animation->x = 0;
	animation->y = 0;
	animation->ex = 0;
	animation->ey = 0;
	animation->timeout = 0;
	animation->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
					    gdk_pixbuf_animation_get_width (image->image_ptr->animation),
					    gdk_pixbuf_animation_get_height (image->image_ptr->animation));
	animation->active = FALSE;

	return animation;
}

static void
html_image_animation_destroy (HTMLImageAnimation *anim)
{
	html_image_animation_stop (anim);
	gdk_pixbuf_unref (anim->pixbuf);
	g_free (anim);
}

#define STALL_INTERVAL 1000

static HTMLImagePointer *
html_image_pointer_new (const char *filename, HTMLImageFactory *factory)
{
	HTMLImagePointer *retval;

	retval = g_new (HTMLImagePointer, 1);
	retval->refcount = 1;
	retval->url = g_strdup (filename);
	retval->loader = gdk_pixbuf_loader_new ();
	retval->pixbuf = NULL;
	retval->animation = NULL;
	retval->interests = NULL;
	retval->factory = factory;
	retval->stall = FALSE;
	retval->stall_timeout = gtk_timeout_add (STALL_INTERVAL, 
						 (GtkFunction)html_image_pointer_timeout,
						 retval);
	return retval;
}

static gboolean
html_image_pointer_timeout (HTMLImagePointer *ip)
{
	GSList *list;
	HTMLImage *image;

	ip->stall = TRUE;

	list = ip->interests;
	/* 
	 * draw the frame now that we decided they've had enough time to
	 * load the image
	 */
	if (ip->pixbuf == NULL) {
		while (list) {
			image = (HTMLImage *)list->data;

			if (image)
				html_engine_queue_draw (ip->factory->engine,
							HTML_OBJECT (image));
			
			list = list->next;
		}
	}
	ip->stall_timeout = 0;
	return FALSE;
}

static void
html_image_pointer_ref (HTMLImagePointer *ip)
{
	ip->refcount++;
}

static void
free_image_ptr_data (HTMLImagePointer *ip)
{
	if (ip->loader) {
		gtk_object_unref (GTK_OBJECT (ip->loader));
		ip->loader = NULL;
	}
	if (ip->animation) {
		gdk_pixbuf_animation_unref (ip->animation);
		ip->animation = NULL;
	}
	if (ip->pixbuf) {
		gdk_pixbuf_unref (ip->pixbuf);
		ip->pixbuf = NULL;
	}

}

static void
html_image_pointer_remove_stall (HTMLImagePointer *ip)
{
	if (ip->stall_timeout) {
		gtk_timeout_remove (ip->stall_timeout);
		ip->stall_timeout = 0;
	}
}

static void
html_image_pointer_unref (HTMLImagePointer *ip)
{
	g_return_if_fail (ip != NULL);

	ip->refcount--;
	if (ip->refcount <= 0) {
		html_image_pointer_remove_stall (ip);
		g_free (ip->url);
		free_image_ptr_data (ip);
		g_free (ip);
	}
}

static void
html_image_pointer_load (HTMLImagePointer *ip)
{
	GtkHTMLStream *handle;

	html_image_pointer_ref (ip);

	handle = gtk_html_stream_new (GTK_HTML (ip->factory->engine->widget),
				      html_image_factory_types,
				      html_image_factory_write_pixbuf,
				      html_image_factory_end_pixbuf,
				      ip);

	/* This is a bit evil, I think.  But it's a lot better here
	   than in the HTMLImage object.  FIXME anyway -- ettore  */
	gtk_signal_emit_by_name (GTK_OBJECT (ip->factory->engine), "url_requested", ip->url, handle);
}

HTMLImagePointer *
html_image_factory_register (HTMLImageFactory *factory, HTMLImage *i, const char *filename, gboolean reload)
{
	HTMLImagePointer *retval;

	g_return_val_if_fail (factory, NULL);
	g_return_val_if_fail (filename, NULL);

	retval = g_hash_table_lookup (factory->loaded_images, filename);

	if (!retval){
		retval = html_image_pointer_new (filename, factory);
		if (*filename) {
			gtk_signal_connect (GTK_OBJECT (retval->loader), "area_prepared",
					    GTK_SIGNAL_FUNC (html_image_factory_area_prepared),
					    retval);

			gtk_signal_connect (GTK_OBJECT (retval->loader), "area_updated",
					    GTK_SIGNAL_FUNC (html_image_factory_area_updated),
					    retval);

			gtk_signal_connect (GTK_OBJECT (retval->loader), "frame_done",
					    GTK_SIGNAL_FUNC (html_image_factory_frame_done),
					    retval);

			gtk_signal_connect (GTK_OBJECT (retval->loader), "animation_done",
					    GTK_SIGNAL_FUNC (html_image_factory_animation_done),
					    retval);

			g_hash_table_insert (factory->loaded_images, retval->url, retval);
			html_image_pointer_load (retval);
		}
	} else if (reload) {
		free_image_ptr_data (retval);
		retval->loader = gdk_pixbuf_loader_new ();
		html_image_pointer_load (retval);
	}

	html_image_pointer_ref (retval);

	/* we add also NULL ptrs, as we dont want these to be cleaned out */
	retval->interests = g_slist_prepend (retval->interests, i);

	if (i) {
		i->image_ptr      = retval;

		if (retval->animation && gdk_pixbuf_animation_get_num_frames (retval->animation) > 1) {
			i->animation = html_image_animation_new (i);
			html_image_animation_start (i);
		}
	}

	return retval;
}

#if 0
HTMLEngine *
html_image_factory_get_engine (HTMLImageFactory *factory)
{
	return factory->engine;
}
#endif

void
html_image_factory_unregister (HTMLImageFactory *factory, HTMLImagePointer *pointer, HTMLImage *i)
{
	pointer->interests = g_slist_remove (pointer->interests, i);

	html_image_pointer_unref (pointer);
	if (pointer->refcount == 1) {
		g_assert (pointer->interests == NULL);
		g_hash_table_remove (factory->loaded_images, pointer->url);
		html_image_pointer_unref (pointer);
	}
}

static void
stop_anim (gpointer key, gpointer value, gpointer user_data)
{
	HTMLImagePointer *ip = value;
	GSList *cur = ip->interests;
	HTMLImage *image;

	html_image_pointer_remove_stall (ip);

	while (cur) {
		if (cur->data) {
			image = (HTMLImage *) cur->data;
			if (image->animation) {
				html_image_animation_stop (image->animation);
			}
		}
		cur = cur->next;
	}
}

void
html_image_factory_stop_animations (HTMLImageFactory *factory)
{
	g_hash_table_foreach (factory->loaded_images, stop_anim, NULL);
}

static gboolean
move_image_pointers (gpointer key, gpointer value, gpointer data)
{
	HTMLImageFactory *dst = HTML_IMAGE_FACTORY (data);
	HTMLImagePointer *ip  = HTML_IMAGE_POINTER (value);

	ip->factory = dst;
	g_hash_table_insert (dst->loaded_images, ip->url, ip);
	html_image_pointer_load (ip);

	return TRUE;
}

void
html_image_factory_move_images (HTMLImageFactory *dst, HTMLImageFactory *src)
{
	g_hash_table_foreach_remove (src->loaded_images, move_image_pointers, dst);
}

static void
deactivate_anim (gpointer key, gpointer value, gpointer user_data)
{
	HTMLImagePointer *ip = value;
	GSList *cur = ip->interests;
	HTMLImage *image;

	while (cur) {
		if (cur->data) {
			image = (HTMLImage *) cur->data;
			if (image->animation) {
				image->animation->active = 0;
			}
		}
		cur = cur->next;
	}
}

void
html_image_factory_deactivate_animations (HTMLImageFactory *factory)
{
	g_hash_table_foreach (factory->loaded_images, deactivate_anim, NULL);
}
