/*
 * AT-SPI - Assistive Technology Service Provider Interface
 * (Gnome Accessibility Project; http://developer.gnome.org/projects/gap)
 *
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <popt.h>
#include <gdk/gdkwindow.h>
#include <libbonobo.h>
#include <login-helper/login-helper.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xatom.h>
#ifdef HAVE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif
#include "magnifier.h"
#include "magnifier-private.h"
#include "zoom-region.h"
#include "zoom-region-private.h"
#include "GNOME_Magnifier.h"

typedef struct
{
    LoginHelper parent;
    Magnifier *mag;
} MagLoginHelper;

typedef struct 
{
    LoginHelperClass parent_class;
} MagLoginHelperClass;

static GObjectClass *parent_class = NULL;

enum {
	STRUT_LEFT = 0,
	STRUT_RIGHT = 1,
	STRUT_TOP = 2,
	STRUT_BOTTOM = 3,
	STRUT_LEFT_START = 4,
	STRUT_LEFT_END = 5,
	STRUT_RIGHT_START = 6,
	STRUT_RIGHT_END = 7,
	STRUT_TOP_START = 8,
	STRUT_TOP_END = 9,
	STRUT_BOTTOM_START = 10,
	STRUT_BOTTOM_END = 11
};

enum {
	MAGNIFIER_SOURCE_DISPLAY_PROP,
	MAGNIFIER_TARGET_DISPLAY_PROP,
	MAGNIFIER_SOURCE_SIZE_PROP,
	MAGNIFIER_TARGET_SIZE_PROP,
	MAGNIFIER_CURSOR_SET_PROP,
	MAGNIFIER_CURSOR_SIZE_PROP,
	MAGNIFIER_CURSOR_ZOOM_PROP,
	MAGNIFIER_CURSOR_COLOR_PROP,
	MAGNIFIER_CURSOR_HOTSPOT_PROP,
	MAGNIFIER_CURSOR_DEFAULT_SIZE_PROP,
	MAGNIFIER_CROSSWIRE_SIZE_PROP,
	MAGNIFIER_CROSSWIRE_CLIP_PROP,
	MAGNIFIER_CROSSWIRE_COLOR_PROP
} PropIdx;

static int _x_error = 0;
static int fixes_event_base = 0, fixes_error_base;
static Display *cursor_client_connection;
static Magnifier *_this_magnifier = NULL;

static void magnifier_transform_cursor (Magnifier *magnifier);
static void magnifier_init_cursor_set (Magnifier *magnifier, gchar *cursor_set);
static gboolean magnifier_check_set_struts (Magnifier *magnifier);
static gboolean magnifier_reset_struts_at_idle (gpointer data);
static gboolean _is_override_redirect = FALSE;

static Window*
mag_login_helper_get_raise_windows (LoginHelper *helper)
{
    Window *mainwin = NULL;
    MagLoginHelper *mag_helper = (MagLoginHelper *) helper;
    Magnifier *magnifier = MAGNIFIER (mag_helper->mag);

    if (magnifier && magnifier->priv && magnifier->priv->w)
    {
	mainwin = g_new0 (Window, 2);
	mainwin[0] = GDK_WINDOW_XWINDOW (magnifier->priv->w->window);
	mainwin[1] = None;
    }
    return mainwin;
}

static LoginHelperDeviceReqFlags
mag_login_helper_get_device_reqs (LoginHelper *helper)
{
    /* means "don't grab the xserver or core pointer", 
       and "we need to raise windows" */

    return LOGIN_HELPER_GUI_EVENTS | 
	LOGIN_HELPER_POST_WINDOWS | 
	LOGIN_HELPER_CORE_POINTER;
}

static gboolean
mag_login_helper_set_safe (LoginHelper *helper, gboolean ignored)
{
    return TRUE;
}

static void
mag_login_helper_class_init (MagLoginHelperClass *klass)
{
	LoginHelperClass *login_helper_class = LOGIN_HELPER_CLASS(klass);
	login_helper_class->get_raise_windows = mag_login_helper_get_raise_windows;
	login_helper_class->get_device_reqs = mag_login_helper_get_device_reqs;
	login_helper_class->set_safe = mag_login_helper_set_safe;
}

static void
mag_login_helper_init (MagLoginHelper *helper)
{
    helper->mag = NULL; /* we set this with mag_login_helper_set_magnifier */
}

static void
mag_login_helper_set_magnifier (MagLoginHelper *helper, Magnifier *mag)
{
    if (helper) 
	helper->mag = mag;
}

BONOBO_TYPE_FUNC (MagLoginHelper, 
		  LOGIN_HELPER_TYPE,
		  mag_login_helper)

gboolean
magnifier_error_check (void)
{
	if (_x_error) {
		_x_error = 0;
		return TRUE;
	}
	return FALSE;
}

static int
magnifier_x_error_handler (Display	 *display,
			   XErrorEvent *error)
{
	if (error->error_code == BadAlloc) {
		_x_error = error->error_code;
	}
	else {
		return -1;
	}
	return 0;
}

static void
magnifier_warp_cursor_to_screen (Magnifier *magnifier)
{
	int x, y, unused_x, unused_y;
	unsigned int mask;
	Window root_return, child_return;

	if (!XQueryPointer (GDK_DISPLAY_XDISPLAY (magnifier->source_display), 
			    GDK_WINDOW_XWINDOW (magnifier->priv->root), 
			    &root_return,
			    &child_return,
			    &x, &y,
			    &unused_x, &unused_y,
			    &mask))
	{
		XWarpPointer (GDK_DISPLAY_XDISPLAY (magnifier->source_display),
			      None,
			      GDK_WINDOW_XWINDOW (magnifier->priv->root),
			      0, 0, 0, 0,
			      x, y);
		XSync (GDK_DISPLAY_XDISPLAY (magnifier->source_display), FALSE);
	}
}

static void
magnifier_zoom_regions_mark_dirty (Magnifier *magnifier, GNOME_Magnifier_RectBounds rect_bounds)
{
	GList *list;

	g_assert (magnifier);
	list = magnifier->zoom_regions;
	while (list) 
	{
		/* propagate the expose events to the zoom regions */
		GNOME_Magnifier_ZoomRegion zoom_region;
		CORBA_Environment ev;
		zoom_region = list->data;
		CORBA_exception_init (&ev);
		if (zoom_region)
			GNOME_Magnifier_ZoomRegion_markDirty (CORBA_Object_duplicate (zoom_region, &ev),
							      &rect_bounds,
							      &ev);
		list = g_list_next (list);
	}
}

void
magnifier_set_cursor_from_pixbuf (Magnifier *magnifier, GdkPixbuf *cursor_pixbuf)
{
	GdkPixmap *pixmap, *mask;
	gint width, height;
	GdkGC *gc;
	GdkDrawable *drawable = magnifier->priv->w->window;

	if (magnifier->priv->cursor) {
		g_object_unref (magnifier->priv->cursor);
		magnifier->priv->cursor = NULL;
	}
	if (drawable && cursor_pixbuf)
	{
		const gchar *xhot_string = NULL, *yhot_string = NULL;
		width = gdk_pixbuf_get_width (cursor_pixbuf);
		height = gdk_pixbuf_get_height (cursor_pixbuf);
		pixmap = gdk_pixmap_new (drawable, width, height, -1);
		gc = gdk_gc_new (pixmap);
		gdk_draw_pixbuf (pixmap, gc, cursor_pixbuf, 0, 0, 0, 0, 
				 width, height,
				 GDK_RGB_DITHER_NONE, 0, 0);
		mask = gdk_pixmap_new (drawable, width, height, 1);
		gdk_pixbuf_render_threshold_alpha (cursor_pixbuf, mask, 0, 0, 0, 0, 
						   width, height,
						   200);
		gdk_gc_unref (gc);
		magnifier->priv->cursor = pixmap;
		magnifier->priv->cursor_mask = mask;
		xhot_string = gdk_pixbuf_get_option (cursor_pixbuf,"x_hot");
		yhot_string = gdk_pixbuf_get_option (cursor_pixbuf,"y_hot");
		if (xhot_string) magnifier->cursor_hotspot.x = atoi (xhot_string);
		if (yhot_string) magnifier->cursor_hotspot.y = atoi (yhot_string);

		if (pixmap) {
			gdk_drawable_get_size (pixmap,
					       &magnifier->priv->cursor_default_size_x,
					       &magnifier->priv->cursor_default_size_y);
			magnifier->priv->cursor_hotspot_x = magnifier->cursor_hotspot.x;
			magnifier->priv->cursor_hotspot_y = magnifier->cursor_hotspot.y;
		}
	}
}


void
magnifier_free_cursor_pixels (guchar *pixels, gpointer data)
{
    /* XFree (data); FIXME why doesn't this work properly? */
}

GdkPixbuf *
magnifier_get_source_pixbuf (Magnifier *magnifier)
{
#ifdef HAVE_XFIXES
	XFixesCursorImage *cursor_image = XFixesGetCursorImage (cursor_client_connection);
        GdkPixbuf *cursor_pixbuf = NULL;
	gchar s[6];
	if (cursor_image)
	{
		cursor_pixbuf = gdk_pixbuf_new_from_data ((guchar *) cursor_image->pixels, 
							  GDK_COLORSPACE_RGB, 
							  TRUE, 8,  
							  cursor_image->width, cursor_image->height,
							  cursor_image->width * 4, 
							  magnifier_free_cursor_pixels, 
							  cursor_image );
		gdk_pixbuf_set_option (cursor_pixbuf, "x_hot", 
				       g_ascii_dtostr (s, 6, (gdouble) cursor_image->xhot));
		gdk_pixbuf_set_option (cursor_pixbuf, "y_hot", 
				       g_ascii_dtostr (s, 6, (gdouble) cursor_image->yhot));
	}
	return cursor_pixbuf;
#else
	return NULL;
#endif
}

GdkPixbuf *
magnifier_get_pixbuf_for_name (Magnifier *magnifier, const gchar *cursor_name)
{
    GdkPixbuf *retval = NULL;
    g_message ("cursor %s", cursor_name);
    if (magnifier->priv->cursorlist) 
	    retval = g_hash_table_lookup (magnifier->priv->cursorlist, cursor_name);
    if (retval) 
	    g_object_ref (retval);
    return retval;
}

void
magnifier_set_cursor_pixmap_by_name (Magnifier *magnifier, const gchar *cursor_name, 
				     gboolean source_fallback)
{
	GdkPixbuf *pixbuf;
	/* search local table; if not found, use source screen's cursor if source_fallback is TRUE */
	if ((pixbuf = magnifier_get_pixbuf_for_name (magnifier, cursor_name)) == NULL) {
#ifndef HAVE_XFIXES
		source_fallback = FALSE;
#endif
		if (source_fallback == TRUE)
		{
			pixbuf = magnifier_get_source_pixbuf (magnifier);
		}
		else
		{
			pixbuf = magnifier_get_pixbuf_for_name (magnifier, "default");
		}
	}
	magnifier_set_cursor_from_pixbuf (magnifier, pixbuf);
	if (pixbuf) gdk_pixbuf_unref (pixbuf);
}

gboolean
magnifier_cursor_notify (GIOChannel *source, GIOCondition condition, gpointer data)
{
#ifdef HAVE_XFIXES
  XEvent ev;
  Magnifier *magnifier = (Magnifier *) data;
  XFixesCursorNotifyEvent  *cev;

  /* TODO: don't ask for name unless xfixes version >= 2 available */
  do
  {
      XNextEvent(cursor_client_connection, &ev);
      if (ev.type == fixes_event_base + XFixesCursorNotify) 
      {
	  cev = (XFixesCursorNotifyEvent *) &ev;
      }
  } while (XPending (cursor_client_connection));

  if (magnifier->priv->use_source_cursor) 
  {
      GdkPixbuf *cursor_pixbuf = magnifier_get_source_pixbuf (magnifier);
      magnifier_set_cursor_from_pixbuf (magnifier, cursor_pixbuf);
      if (cursor_pixbuf) gdk_pixbuf_unref (cursor_pixbuf);
  }
  else
  {
      magnifier_set_cursor_pixmap_by_name (magnifier, 
					   gdk_x11_get_xatom_name (cev->cursor_name),
					   TRUE);
  }

  magnifier_transform_cursor (magnifier);
#ifdef DEBUG_CURSOR
  g_message ("cursor changed: subtype=%d, cursor_serial=%lu, name=[%x] %s\n",
	     (int) cev->subtype, cev->cursor_serial, (int) cev->cursor_name,
	     gdk_x11_get_xatom_name (cev->cursor_name));
#endif

  return TRUE;
#else
  return FALSE;
#endif
}

gboolean
magnifier_cursor_notification_init (Magnifier *magnifier)
{
#ifdef HAVE_XFIXES
    GIOChannel *ioc;
    int fd;
    Display *dpy;
    Window rootwin;

    dpy = GDK_DISPLAY_XDISPLAY (magnifier->source_display);
    cursor_client_connection = XOpenDisplay (0);
    rootwin = GDK_WINDOW_XWINDOW (magnifier->priv->root);

    if (!XFixesQueryExtension (cursor_client_connection, &fixes_event_base, &fixes_error_base))
    {
	g_warning ("XFixes extension not currently active.\n");
	return FALSE;
    }
    else
    {
	XFixesSelectCursorInput (cursor_client_connection, rootwin, XFixesDisplayCursorNotifyMask);
	fd = ConnectionNumber (cursor_client_connection);
	ioc = g_io_channel_unix_new (fd);
	g_io_add_watch (ioc, G_IO_IN | G_IO_HUP | G_IO_PRI | G_IO_ERR, magnifier_cursor_notify, 
			magnifier);
	g_io_channel_unref (ioc); 
	g_message ("added event source to xfixes cursor-notify connection");
 	XFlush (cursor_client_connection); 
   }
    return TRUE;
#else
    g_warning ("this copy of gnome-mag was built without xfixes extension support.\n");
    return FALSE;
#endif
}

void
magnifier_notify_damage (Magnifier *magnifier, XRectangle *rect)
{
	GNOME_Magnifier_RectBounds rect_bounds;
	rect_bounds.x1 = rect->x;
	rect_bounds.y1 = rect->y;
	rect_bounds.x2 = rect->x + rect->width;
	rect_bounds.y2 = rect->y + rect->height;
#ifdef DEBUG_DAMAGE
	g_message ("damage");
#endif
	magnifier_zoom_regions_mark_dirty (magnifier, rect_bounds);
}

GdkFilterReturn
magnifier_expose_filter (GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
	Magnifier *magnifier = data;
	GList *list;

	if (event->any.type == GDK_EXPOSE) 
	{
		GNOME_Magnifier_RectBounds rect_bounds;
		GdkRectangle rect;
		gdk_region_get_clipbox (event->expose.region, &rect);
		rect_bounds.x1 = rect.x;
		rect_bounds.y1 = rect.y;
		rect_bounds.x2 = rect.x + rect.width;
		rect_bounds.y2 = rect.y + rect.height;
		magnifier_zoom_regions_mark_dirty (magnifier, rect_bounds);

		return GDK_FILTER_TRANSLATE;
	}
	return GDK_FILTER_CONTINUE;
}
static void
magnifier_set_extension_listeners (Magnifier *magnifier, GdkWindow *root)
{
	g_message ("adding expose listener on root");
	/* TODO: remove listener from 'old' root if one was already present */	
	gdk_window_add_filter (root, magnifier_expose_filter, NULL);
	magnifier_damage_client_init (magnifier);
	magnifier_cursor_notification_init (magnifier);
}

static void
magnifier_size_allocate (GtkWidget *widget)
{
	magnifier_check_set_struts (_this_magnifier);
}

static void
magnifier_realize (GtkWidget *widget)
{
	XWMHints wm_hints;
	Atom wm_window_protocols[2];
	Atom wm_type_atoms[1];
	Atom net_wm_window_type;
	GdkDisplay *target_display = gdk_drawable_get_display (widget->window);
	
	static gboolean initialized = FALSE;

#ifndef MAG_WINDOW_OVERRIDE_REDIRECT	
	if (!initialized) {
		wm_window_protocols[0] = gdk_x11_get_xatom_by_name_for_display (target_display,
										"WM_DELETE_WINDOW");
		wm_window_protocols[1] = gdk_x11_get_xatom_by_name_for_display (target_display,
										"_NET_WM_PING");
		/* use DOCK until Metacity RFE for new window type goes in */
                wm_type_atoms[0] = gdk_x11_get_xatom_by_name_for_display (target_display,
									  "_NET_WM_WINDOW_TYPE_DOCK");
	}
  
	wm_hints.flags = InputHint;
	wm_hints.input = False;
	
	XSetWMHints (GDK_WINDOW_XDISPLAY (widget->window),
		     GDK_WINDOW_XWINDOW (widget->window), &wm_hints);
	
	XSetWMProtocols (GDK_WINDOW_XDISPLAY (widget->window),
			 GDK_WINDOW_XWINDOW (widget->window), wm_window_protocols, 2);

	net_wm_window_type = gdk_x11_get_xatom_by_name_for_display 
		(target_display, "_NET_WM_WINDOW_TYPE");

	if (net_wm_window_type && wm_type_atoms[0])
		XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window),
				 GDK_WINDOW_XWINDOW (widget->window),
				 net_wm_window_type,
				 XA_ATOM, 32, PropModeReplace,
				 (guchar *)wm_type_atoms,
				 1);
#else
#endif
	/* TODO: make sure this works/is reset if the DISPLAY 
	 * (as well as the SCREEN) changes.
	 */

	XSetErrorHandler (magnifier_x_error_handler);
}

GdkWindow*
magnifier_get_root (Magnifier *magnifier)
{
	if (!magnifier->priv->root) {
		magnifier->priv->root = gdk_screen_get_root_window (
			gdk_display_get_screen (magnifier->source_display,
						magnifier->source_screen_num));
	}
	return magnifier->priv->root;
}

static gint
magnifier_parse_display_name (Magnifier *magnifier, gchar *full_display_string,
			      gchar **display_name)
{
	gchar *screen_ptr;
	gchar **strings;
	
	if (display_name != NULL) {
		strings = g_strsplit (full_display_string, ":", 2);
		*display_name = strings [0];
		if (strings [1] != NULL)
			g_free (strings [1]);
	}

	screen_ptr = rindex (full_display_string, '.');
	if (screen_ptr != NULL) {
		return (gint) strtol (++screen_ptr, NULL, 10);
	}
	return 0;
}

static void
magnifier_get_display_rect_bounds (Magnifier *magnifier, GNOME_Magnifier_RectBounds *rect_bounds, gboolean is_target)
{
    if (is_target)
    {
	rect_bounds->x1 = 0;
	rect_bounds->x2 = gdk_screen_get_width (
	    gdk_display_get_screen (magnifier->target_display,
				    magnifier->target_screen_num));
	rect_bounds->y1 = 0;
	rect_bounds->y2 = gdk_screen_get_height (
	    gdk_display_get_screen (magnifier->target_display,
				    magnifier->target_screen_num));

    }
    else 
    {
	rect_bounds->x1 = 0;
	rect_bounds->x2 = gdk_screen_get_width (
	    gdk_display_get_screen (magnifier->source_display,
				    magnifier->source_screen_num));
	rect_bounds->y1 = 0;
	rect_bounds->y2 = gdk_screen_get_height (
	    gdk_display_get_screen (magnifier->source_display,
				    magnifier->source_screen_num));

    }
}

static void
magnifier_unref_zoom_region (gpointer data, gpointer user_data)
{
/*	Magnifier *magnifier = user_data; NOT USED */
	CORBA_Environment ev;
	GNOME_Magnifier_ZoomRegion zoom_region = data;
	CORBA_exception_init (&ev);
	GNOME_Magnifier_ZoomRegion_dispose (zoom_region, &ev);
}

static void
magnifier_reparent_zoom_regions (Magnifier *magnifier)
{
    GList *list;
    
    g_assert (magnifier);
    list = magnifier->zoom_regions;
    while (list) 
    {
	GNOME_Magnifier_ZoomRegion zoom_region;
	CORBA_Environment ev;
	zoom_region = list->data;
	CORBA_exception_init (&ev);
	if (zoom_region)
	{
	    GNOME_Magnifier_ZoomRegion new_region;
	    Bonobo_PropertyBag properties, new_properties;
	    GNOME_Magnifier_RectBounds rectbounds, viewport;
	    CORBA_any *value;
	    rectbounds = GNOME_Magnifier_ZoomRegion_getROI (zoom_region, &ev);
	    properties = GNOME_Magnifier_ZoomRegion_getProperties (zoom_region, &ev);
	    value = bonobo_pbclient_get_value (properties, "viewport", TC_GNOME_Magnifier_RectBounds, &ev);
	    memcpy (&viewport, value->_value, sizeof (GNOME_Magnifier_RectBounds));
	    fprintf (stderr, "reparenting zoomer with viewport %d,%d - %d,%d\n", viewport.x1, viewport.y1, viewport.x2, viewport.y2);
	    CORBA_free (value);
	    new_region = GNOME_Magnifier_Magnifier_createZoomRegion (BONOBO_OBJREF (magnifier), 0, 0, &rectbounds, &viewport, &ev);
	    new_properties = GNOME_Magnifier_ZoomRegion_getProperties (new_region, &ev);
	    bonobo_pbclient_set_boolean (new_properties, "is-managed", 
					 bonobo_pbclient_get_boolean (properties, "is-managed", NULL), NULL);
	    bonobo_pbclient_set_long (new_properties, "smooth-scroll-policy", 
				       bonobo_pbclient_get_long (properties, "smooth-scroll-policy", NULL), NULL);
	    bonobo_pbclient_set_float (new_properties, "contrast", 
					 bonobo_pbclient_get_float (properties, "contrast", NULL), NULL);
	    bonobo_pbclient_set_float (new_properties, "mag-factor-x", 
					 bonobo_pbclient_get_float (properties, "mag-factor-x", NULL), NULL);
	    bonobo_pbclient_set_float (new_properties, "mag-factor-y", 
					 bonobo_pbclient_get_float (properties, "mag-factor-y", NULL), NULL);
	    bonobo_pbclient_set_long (new_properties, "x-alignment", 
					 bonobo_pbclient_get_long (properties, "x-alignment", NULL), NULL);
	    bonobo_pbclient_set_long (new_properties, "y-alignment", 
					 bonobo_pbclient_get_long (properties, "y-alignment", NULL), NULL);
	    bonobo_pbclient_set_long (new_properties, "border-color", 
				      bonobo_pbclient_get_long (properties, "border-color", NULL), NULL); 
	    bonobo_pbclient_set_long (new_properties, "border-size", 
				      bonobo_pbclient_get_long (properties, "border-size", NULL), NULL);
	    bonobo_pbclient_set_string (new_properties, "smoothing-type", 
					bonobo_pbclient_get_string (properties, "smoothing-type", NULL), NULL); 
	    bonobo_pbclient_set_boolean (new_properties, "inverse-video", 
					 bonobo_pbclient_get_boolean (properties, "inverse-video", NULL), NULL); 
	    bonobo_object_release_unref (properties, &ev);
	    bonobo_object_release_unref (new_properties, &ev);
	    list->data = new_region;
	    magnifier_unref_zoom_region ((gpointer) zoom_region, NULL);
	}
	list = g_list_next (list);
    }   
}

static void
magnifier_init_display (Magnifier *magnifier, gchar *display_name, gboolean is_target)
{
    if (is_target)
    {
	magnifier->target_screen_num =
	    magnifier_parse_display_name (magnifier,
					  display_name,
					  NULL);
	magnifier->target_display =
	    gdk_display_open (display_name);
	magnifier->priv->root =
	    gdk_screen_get_root_window (
		gdk_display_get_screen (
		    magnifier->target_display,
		    magnifier->target_screen_num));
    }
    else 
    {
	magnifier->source_screen_num =
	    magnifier_parse_display_name (magnifier,
					  display_name,
					  NULL);
	magnifier->source_display =
	    gdk_display_open (display_name);
	magnifier->priv->root =
	    gdk_screen_get_root_window (
		gdk_display_get_screen (
		    magnifier->source_display,
		    magnifier->source_screen_num));
    }
}

static void
magnifier_exit (GtkObject *object)
{
	gtk_main_quit ();
	exit (0);
}

#define GET_PIXEL(a,i,j,s,b) \
(*(guint32 *)(memcpy (b,(a) + ((j) * s + (i) * pixel_size_t), pixel_size_t)))

#define PUT_PIXEL(a,i,j,s,b) \
(memcpy (a + ((j) * s + (i) * pixel_size_t), &(b), pixel_size_t))

static void
magnifier_recolor_pixbuf (Magnifier *magnifier, GdkPixbuf *pixbuf)
{
	int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	int i, j;
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);
	guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
	guint32 pixval = 0, cursor_color = 0;
	size_t pixel_size_t = 3; /* FIXME: invalid assumption ? */

	cursor_color = ((magnifier->cursor_color & 0xFF0000) >> 16) +
		(magnifier->cursor_color & 0x00FF00) +
		((magnifier->cursor_color & 0x0000FF) << 16);
	for (j = 0; j < h; ++j) {
		for (i = 0; i < w; ++i) {
			pixval = GET_PIXEL (pixels, i, j, rowstride, &pixval);
			if ((pixval & 0x808080) == 0)
			{
				pixval = cursor_color;
				PUT_PIXEL (pixels, i, j, rowstride,
					   pixval);
			}
		}
	}
}

static void
magnifier_transform_cursor (Magnifier *magnifier)
{
	if (magnifier->priv->cursor) /* don't do this if cursor isn't intialized yet */
	{
		int width, height;
		int size_x, size_y;
		GdkPixbuf *scaled_cursor_pixbuf;
		GdkPixbuf *scaled_mask_pixbuf;
		GdkPixbuf *cursor_pixbuf;
		GdkPixbuf *mask_pixbuf;
		GdkPixmap *cursor_pixmap = magnifier->priv->cursor;
		GdkPixmap *mask_pixmap = magnifier->priv->cursor_mask;
		GdkGC *cgc;
		GdkGC *mgc;

		if (magnifier->cursor_size_x)
		{
			size_x = magnifier->cursor_size_x;
			size_y = magnifier->cursor_size_y;
		}
		else
		{
			size_x = magnifier->priv->cursor_default_size_x * 
			       magnifier->cursor_scale_factor;
			size_y = magnifier->priv->cursor_default_size_y * 
			       magnifier->cursor_scale_factor;
		}
		gdk_drawable_get_size (magnifier->priv->cursor, &width, &height);
		if ((size_x == width) && (size_y == height) 
		    && (magnifier->cursor_color == 0xFF000000)) {
			return; /* nothing changes */
		}
		cgc = gdk_gc_new (cursor_pixmap);
		mgc = gdk_gc_new (mask_pixmap);
		cursor_pixbuf = gdk_pixbuf_get_from_drawable (NULL, cursor_pixmap,
							      NULL, 0, 0, 0, 0,
							      width, height);
		if (magnifier->cursor_color != 0xFF000000)
			magnifier_recolor_pixbuf (magnifier, cursor_pixbuf);
		mask_pixbuf = gdk_pixbuf_get_from_drawable (NULL,
							    mask_pixmap,
							    NULL, 0, 0, 0, 0,
							    width, height);
		scaled_cursor_pixbuf = gdk_pixbuf_scale_simple (
			cursor_pixbuf, size_x, size_y, GDK_INTERP_NEAREST);
		
		magnifier->cursor_hotspot.x = magnifier->priv->cursor_hotspot_x * size_x 
					    / magnifier->priv->cursor_default_size_x;
		magnifier->cursor_hotspot.y = magnifier->priv->cursor_hotspot_y * size_y 
					    / magnifier->priv->cursor_default_size_y;
					    
		scaled_mask_pixbuf = gdk_pixbuf_scale_simple (
			mask_pixbuf, size_x, size_y, GDK_INTERP_NEAREST);
		g_object_unref (cursor_pixbuf);
		g_object_unref (mask_pixbuf);
		g_object_unref (cursor_pixmap);
		g_object_unref (mask_pixmap);
		magnifier->priv->cursor = gdk_pixmap_new (
			magnifier->priv->w->window,
			size_x, size_y,
			-1);
		if (!magnifier->priv->cursor) g_warning ("magnifier cursur pixmap is non-null");
		magnifier->priv->cursor_mask = gdk_pixmap_new (
			magnifier->priv->w->window,
			size_x, size_y,
			1);

		gdk_draw_pixbuf (magnifier->priv->cursor,
				 cgc,
				 scaled_cursor_pixbuf,
				 0, 0, 0, 0, size_x, size_y,
				 GDK_RGB_DITHER_NONE, 0, 0 );
		scaled_mask_pixbuf = gdk_pixbuf_add_alpha (
			scaled_mask_pixbuf, True, 0, 0, 0);
		gdk_pixbuf_render_threshold_alpha (scaled_mask_pixbuf,
						   magnifier->priv->cursor_mask,
						   0, 0, 0, 0, size_x, size_y,
						   0x80);
		g_object_unref (scaled_cursor_pixbuf);
		g_object_unref (scaled_mask_pixbuf);
		g_object_unref (mgc);
		g_object_unref (cgc);
	}	
}

static void
magnifier_init_cursor_set (Magnifier *magnifier, gchar *cursor_set)
{
	GdkPixbuf	*cursor_pixbuf = NULL;
	/*
	 * we check the cursor-set property string here,
	 * and create/apply the appropriate cursor settings
	 */
	magnifier->cursor_set = cursor_set;
#ifdef HAVE_XFIXES	
	magnifier->priv->use_source_cursor = 
	    (!strcmp (cursor_set, "default") && 
	     (fixes_event_base != 0));
#else
	magnifier->priv->use_source_cursor = FALSE;
#endif
	if (magnifier->priv->use_source_cursor) return;

	if (!strcmp (magnifier->cursor_set, "none")) {
		magnifier->priv->cursor = NULL;
		return;
	}
	else 
	{
		GDir *cursor_dir;
		const gchar *filename;
		gchar *cursor_dirname;

		if (magnifier->priv->cursorlist)
		{
			g_hash_table_destroy (magnifier->priv->cursorlist);
		}
		magnifier->priv->cursorlist = g_hash_table_new_full (g_str_hash, g_str_equal,
								     g_free, g_object_unref);

		cursor_dirname = g_strconcat (CURSORSDIR, "/", magnifier->cursor_set, NULL);
		cursor_dir = g_dir_open (cursor_dirname, 0, NULL);
		/* assignment, not comparison, is intentional */
		g_message ("cursor_dir=%x", cursor_dir);
		while (cursor_dir && (filename = g_dir_read_name (cursor_dir)) != NULL) 
		{
			if (filename) 
			{
				gchar *path = g_strconcat (cursor_dirname, "/", filename, NULL);
				GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (path, NULL);
				if (pixbuf)
				{
					/* add this pixbuf and its name to our list */
					gchar **sv, *cname;
					cname = g_path_get_basename (filename);
					sv = g_strsplit (cname, ".", 2);
					g_hash_table_insert (magnifier->priv->cursorlist, 
							     g_strdup (sv[0]),
							     pixbuf);
					g_free (cname);
					g_strfreev (sv);
				}
				g_free (path);
			}
		} 
		g_free (cursor_dirname);
		if (cursor_dir) g_dir_close (cursor_dir);
	}
	/* don't fallover to source cursor here, we haven't initialized X yet */
	magnifier_set_cursor_pixmap_by_name (magnifier, "default", FALSE);
	magnifier_transform_cursor (magnifier);
}

static gboolean 
magnifier_reset_struts_at_idle (gpointer data)
{
	if (data)
	{
		Magnifier *magnifier = MAGNIFIER (data);
		if (magnifier->priv && GTK_WIDGET_REALIZED (magnifier->priv->w) && 
		    magnifier_check_set_struts (magnifier))
		{
			return FALSE;
		}
	}
	return TRUE;
}

static gboolean
magnifier_check_set_struts (Magnifier *magnifier)
{
	/* TODO: don't do this if we're using Composite */
	if (magnifier &&
	    magnifier->priv && magnifier->priv->w && GTK_WIDGET_REALIZED (magnifier->priv->w) &&
	    magnifier->priv->w->window) 
	{
		Atom atom_strut = gdk_x11_get_xatom_by_name ("_NET_WM_STRUT");
		Atom atom_strut_partial = gdk_x11_get_xatom_by_name ("_NET_WM_STRUT_PARTIAL");
		guint32 struts[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		GtkWidget *widget = magnifier->priv->w;
		gint width = gdk_screen_get_width (
			gdk_display_get_screen (magnifier->target_display,
						magnifier->target_screen_num));
		gint height = gdk_screen_get_height (
			gdk_display_get_screen (magnifier->target_display,
						magnifier->target_screen_num));

		gint right_margin, left_margin, top_margin, bottom_margin;
		gint wx, wy, ww, wh;

		gtk_window_get_position (GTK_WINDOW (magnifier->priv->w), &wx, &wy);
		gtk_window_get_size (GTK_WINDOW (magnifier->priv->w), &ww, &wh);

		left_margin = wx;
		right_margin = (width - ww) - wx;
		top_margin = wy;
		bottom_margin = (height - wh) - wy;

		/* set the WM_STRUT properties on the appropriate side */
		if (bottom_margin > top_margin && 
		    bottom_margin > left_margin &&
		    bottom_margin > right_margin)
		{
			struts[STRUT_TOP] = wh + wy;
			struts[STRUT_TOP_START] = wx;
			struts[STRUT_TOP_END] = wx + ww;
		} 
		else if (top_margin > bottom_margin && 
			 top_margin > left_margin &&
			 top_margin > right_margin)
		{
			struts[STRUT_BOTTOM] = height - wy;
			struts[STRUT_BOTTOM_START] = wx;
			struts[STRUT_BOTTOM_END] = wx + ww;
		}
		else if (right_margin > left_margin &&
			 right_margin > top_margin &&
			 right_margin > bottom_margin)
		{
			struts[STRUT_LEFT] = wx;
			struts[STRUT_LEFT_START] = wy;
			struts[STRUT_LEFT_END] = wh + wy;
		}
		else 
		{
			struts[STRUT_RIGHT] = width - wx;
			struts[STRUT_RIGHT_START] = wy;
			struts[STRUT_RIGHT_END] = wy + wh;
		}
		
		gdk_error_trap_push ();
		XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), 
				 GDK_WINDOW_XWINDOW (widget->window), 
				 atom_strut,
				 XA_CARDINAL, 32, PropModeReplace,
				 (guchar *) &struts, 4);
		XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), 
				 GDK_WINDOW_XWINDOW (widget->window), 
				 atom_strut_partial,
				 XA_CARDINAL, 32, PropModeReplace,
				 (guchar *) &struts, 12); 
		gdk_error_trap_pop ();

#ifdef DEBUG_STRUTS
		g_message ("struts TOP %d (%d - %d)", struts[STRUT_TOP], struts[STRUT_TOP_START], struts[STRUT_TOP_END]);
		g_message ("struts BOTTOM %d (%d - %d)", struts[STRUT_BOTTOM], struts[STRUT_BOTTOM_START], struts[STRUT_BOTTOM_END]);
		g_message ("struts LEFT %d (%d - %d)", struts[STRUT_LEFT], struts[STRUT_LEFT_START], struts[STRUT_LEFT_END]);
		g_message ("struts RIGHT %d (%d - %d)", struts[STRUT_RIGHT], struts[STRUT_RIGHT_START], struts[STRUT_RIGHT_END]);
#endif
		return TRUE;
	}
	return FALSE;
}

static void
magnifier_get_property (BonoboPropertyBag *bag,
			BonoboArg *arg,
			guint arg_id,
			CORBA_Environment *ev,
			gpointer user_data)
{
	Magnifier *magnifier = user_data;
	GNOME_Magnifier_RectBounds rect_bounds;
	int csize = 0;
	
	switch (arg_id) {
	case MAGNIFIER_SOURCE_SIZE_PROP:
		BONOBO_ARG_SET_GENERAL (arg, magnifier->source_bounds,
					TC_GNOME_Magnifier_RectBounds,
					GNOME_Magnifier_RectBounds, NULL);
		break;
	case MAGNIFIER_TARGET_SIZE_PROP:
	    	BONOBO_ARG_SET_GENERAL (arg, magnifier->target_bounds,
					TC_GNOME_Magnifier_RectBounds,
					GNOME_Magnifier_RectBounds, NULL);

		break;
	case MAGNIFIER_CURSOR_SET_PROP:
		BONOBO_ARG_SET_STRING (arg, magnifier->cursor_set);
		break;
	case MAGNIFIER_CURSOR_SIZE_PROP:
		BONOBO_ARG_SET_INT (arg, magnifier->cursor_size_x);
		BONOBO_ARG_SET_INT (arg, magnifier->cursor_size_y);
		break;
	case MAGNIFIER_CURSOR_ZOOM_PROP:
		BONOBO_ARG_SET_FLOAT (arg, magnifier->cursor_scale_factor);
		break;
	case MAGNIFIER_CURSOR_COLOR_PROP:
		BONOBO_ARG_SET_GENERAL (arg, magnifier->cursor_color,
					TC_CORBA_unsigned_long,
					CORBA_unsigned_long, NULL);
		break;
	case MAGNIFIER_CURSOR_HOTSPOT_PROP:
		BONOBO_ARG_SET_GENERAL (arg, magnifier->cursor_hotspot,
					TC_GNOME_Magnifier_Point,
					GNOME_Magnifier_Point, NULL);

		break;
	case MAGNIFIER_CURSOR_DEFAULT_SIZE_PROP:
		if (magnifier->priv->cursor)
			gdk_drawable_get_size (magnifier->priv->cursor,
					       &csize, &csize);
		BONOBO_ARG_SET_INT (arg, csize);
		break;
	case MAGNIFIER_CROSSWIRE_SIZE_PROP:
		BONOBO_ARG_SET_INT (arg, magnifier->crosswire_size);
		break;
	case MAGNIFIER_CROSSWIRE_CLIP_PROP:
		BONOBO_ARG_SET_BOOLEAN (arg, magnifier->crosswire_clip);
		break;
	case MAGNIFIER_CROSSWIRE_COLOR_PROP:
		BONOBO_ARG_SET_LONG (arg, magnifier->crosswire_color);
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
	};
}

static void
magnifier_set_property (BonoboPropertyBag *bag,
			BonoboArg *arg,
			guint arg_id,
			CORBA_Environment *ev,
			gpointer user_data)
{
	Magnifier *magnifier = user_data;
	GNOME_Magnifier_RectBounds rect_bounds;
	gchar *full_display_string;
	
	switch (arg_id) {
	case MAGNIFIER_SOURCE_DISPLAY_PROP:
		full_display_string = BONOBO_ARG_GET_STRING (arg);
		magnifier->source_screen_num =
			magnifier_parse_display_name (magnifier,
						      full_display_string,
						      NULL);
		magnifier->source_display =
			gdk_display_open (full_display_string);
		magnifier->priv->root =
			gdk_screen_get_root_window (
				gdk_display_get_screen (
					magnifier->source_display,
					magnifier->source_screen_num));
		magnifier_warp_cursor_to_screen (magnifier);
		magnifier_get_display_rect_bounds (magnifier, &magnifier->source_bounds, FALSE);
		magnifier_check_set_struts (magnifier);
		break;
	case MAGNIFIER_TARGET_DISPLAY_PROP:
		full_display_string = BONOBO_ARG_GET_STRING (arg);
		magnifier->target_screen_num =
			magnifier_parse_display_name (magnifier,
						      full_display_string,
						      NULL);
		magnifier->target_display =
			gdk_display_open (full_display_string);
		if (GTK_IS_WINDOW (magnifier->priv->w)) 
			gtk_window_set_screen (GTK_WINDOW (magnifier->priv->w), 
					       gdk_display_get_screen (
						       magnifier->target_display,
						       magnifier->target_screen_num));
		magnifier_get_display_rect_bounds (magnifier, &magnifier->target_bounds, TRUE);
		magnifier_get_display_rect_bounds (magnifier, &magnifier->source_bounds, FALSE);
		magnifier_reparent_zoom_regions (magnifier);
		magnifier_init_cursor_set (magnifier, magnifier->cursor_set); /* needed to reset pixmaps */
		magnifier_check_set_struts (magnifier);
		break;
	case MAGNIFIER_SOURCE_SIZE_PROP:
	        magnifier->source_bounds = BONOBO_ARG_GET_GENERAL (arg,
								   TC_GNOME_Magnifier_RectBounds,
								   GNOME_Magnifier_RectBounds,
								   NULL);
		break;
	case MAGNIFIER_TARGET_SIZE_PROP:
	        magnifier->target_bounds = BONOBO_ARG_GET_GENERAL (arg,
								   TC_GNOME_Magnifier_RectBounds,
								   GNOME_Magnifier_RectBounds,
								   NULL);
		gtk_window_move (GTK_WINDOW (magnifier->priv->w),
				 magnifier->target_bounds.x1,
				 magnifier->target_bounds.y1);
		
		gtk_window_resize (GTK_WINDOW (magnifier->priv->w),
				   magnifier->target_bounds.x2 - magnifier->target_bounds.x1,
				   magnifier->target_bounds.y2 - magnifier->target_bounds.y1);
		magnifier_check_set_struts (magnifier);
		break;
	case MAGNIFIER_CURSOR_SET_PROP:
		magnifier_init_cursor_set (magnifier, g_strdup (BONOBO_ARG_GET_STRING (arg)));
		break;
	case MAGNIFIER_CURSOR_SIZE_PROP:
		magnifier->cursor_size_x = BONOBO_ARG_GET_INT (arg);
		magnifier->cursor_size_y = BONOBO_ARG_GET_INT (arg);
		magnifier_transform_cursor (magnifier);
		break;
	case MAGNIFIER_CURSOR_ZOOM_PROP:
		magnifier->cursor_scale_factor = BONOBO_ARG_GET_FLOAT (arg);
		magnifier_transform_cursor (magnifier);
		break;
	case MAGNIFIER_CURSOR_COLOR_PROP:
		magnifier->cursor_color = BONOBO_ARG_GET_GENERAL (arg,
								  TC_CORBA_unsigned_long, 
								  CORBA_unsigned_long, 
								  NULL);
		magnifier_transform_cursor (magnifier);
		break;
	case MAGNIFIER_CURSOR_HOTSPOT_PROP:
		magnifier->cursor_hotspot = BONOBO_ARG_GET_GENERAL (arg,
								    TC_GNOME_Magnifier_Point,
								    GNOME_Magnifier_Point,
								    NULL);
		/* TODO: notify zoomers */
                /* FIXME: don't call init_cursor, it overwrites this property! */
		magnifier_transform_cursor (magnifier); 
		break;
	case MAGNIFIER_CURSOR_DEFAULT_SIZE_PROP:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_ReadOnly);
		break;
	case MAGNIFIER_CROSSWIRE_SIZE_PROP:
		magnifier->crosswire_size = BONOBO_ARG_GET_INT (arg);
		/* TODO: notify zoomers */
		break;
	case MAGNIFIER_CROSSWIRE_CLIP_PROP:
		magnifier->crosswire_clip = BONOBO_ARG_GET_BOOLEAN (arg);
		break;
	case MAGNIFIER_CROSSWIRE_COLOR_PROP:
		magnifier->crosswire_color = BONOBO_ARG_GET_LONG (arg);
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
	};
}

static void
magnifier_do_dispose (Magnifier *magnifier)
{
	/* FIXME: this is dead ropey code structuring */
	bonobo_activation_active_server_unregister (
		MAGNIFIER_OAFIID, BONOBO_OBJREF (magnifier));

	if (magnifier->zoom_regions)
		g_list_free (magnifier->zoom_regions);
	magnifier->zoom_regions = NULL;
	
	bonobo_main_quit ();
}

static void
magnifier_gobject_dispose (GObject *object)
{
	magnifier_do_dispose (MAGNIFIER (object));

	BONOBO_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
impl_magnifier_set_source_display (PortableServer_Servant servant,
				   const CORBA_char *display,
				   CORBA_Environment *ev)
{
	Magnifier *magnifier = MAGNIFIER (bonobo_object_from_servant (servant));
	BonoboArg *arg = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (arg, display);

	magnifier_set_property (magnifier->property_bag,
				arg,
				MAGNIFIER_SOURCE_DISPLAY_PROP,
				ev,
				magnifier);

	/* attach listeners for DAMAGE, "dirty region", XFIXES cursor changes */
	magnifier_set_extension_listeners (magnifier, magnifier_get_root (magnifier));

	bonobo_arg_release (arg);
}

static void
impl_magnifier_set_target_display (PortableServer_Servant servant,
				   const CORBA_char *display,
				   CORBA_Environment *ev)
{
	Magnifier *magnifier = MAGNIFIER (bonobo_object_from_servant (servant));
	BonoboArg *arg = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (arg, display);

	magnifier_set_property (magnifier->property_bag,
				arg,
				MAGNIFIER_TARGET_DISPLAY_PROP,
				ev,
				magnifier);

	bonobo_arg_release (arg);
}

static GNOME_Magnifier_ZoomRegion
impl_magnifier_create_zoom_region (PortableServer_Servant servant,
				   const CORBA_float zx,
				   const CORBA_float zy,
				   const GNOME_Magnifier_RectBounds *roi,
				   const GNOME_Magnifier_RectBounds *viewport,
				   CORBA_Environment *ev)
{
	Magnifier *magnifier = MAGNIFIER (bonobo_object_from_servant (servant));
	CORBA_any viewport_any;
	ZoomRegion *zoom_region = zoom_region_new ();
	Bonobo_PropertyBag properties;
	GNOME_Magnifier_ZoomRegion retval;

	/* FIXME:
	 * shouldn't do this here, since it causes the region to get
	 * mapped onto the parent, if if it's not explicitly added!
	 */
	zoom_region->priv->parent = magnifier;

	retval = BONOBO_OBJREF (zoom_region);
	/* XXX: should check ev after each call, below */
	CORBA_exception_init (ev);
	GNOME_Magnifier_ZoomRegion_setMagFactor (retval, zx, zy, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		fprintf (stderr, "EXCEPTION setMagFactor\n");

	CORBA_exception_init (ev);
	properties = GNOME_Magnifier_ZoomRegion_getProperties (retval, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		fprintf (stderr, "EXCEPTION getProperties\n");

	viewport_any._type = TC_GNOME_Magnifier_RectBounds;
	viewport_any._value = (gpointer) viewport;
	Bonobo_PropertyBag_setValue (
		properties, "viewport", &viewport_any, ev);

	GNOME_Magnifier_ZoomRegion_setROI (retval, roi, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		fprintf (stderr, "EXCEPTION setROI\n");

	CORBA_exception_init (ev);

	gtk_widget_set_size_request (magnifier->priv->canvas,
			   viewport->x2 - viewport->x1,
			   viewport->y2 - viewport->y1);
	gtk_widget_show (magnifier->priv->canvas);
	gtk_widget_show (magnifier->priv->w);

	bonobo_object_release_unref (properties, ev);
	
	return CORBA_Object_duplicate (retval, ev);
}

static
CORBA_boolean
impl_magnifier_add_zoom_region (PortableServer_Servant servant,
				const GNOME_Magnifier_ZoomRegion region,
				CORBA_Environment * ev)
{
	Magnifier *magnifier = MAGNIFIER (bonobo_object_from_servant (servant));

	if (magnifier->zoom_regions == NULL) 
	{
		magnifier_set_extension_listeners (magnifier, magnifier_get_root (magnifier));
	}

	/* FIXME: this needs proper lifecycle management */
	magnifier->zoom_regions = g_list_append (magnifier->zoom_regions, region);
	magnifier_check_set_struts (magnifier);

	return CORBA_TRUE;
}

static Bonobo_PropertyBag
impl_magnifier_get_properties (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	Magnifier *magnifier = MAGNIFIER (bonobo_object_from_servant (servant));

	return bonobo_object_dup_ref (
		BONOBO_OBJREF (magnifier->property_bag), ev);
}

GNOME_Magnifier_ZoomRegionList *
impl_magnifier_get_zoom_regions (PortableServer_Servant servant,
				 CORBA_Environment * ev)
{
	Magnifier *magnifier =
		MAGNIFIER (bonobo_object_from_servant (servant));

	GNOME_Magnifier_ZoomRegionList *list;
	CORBA_Object objref;
	int i, len;

	len = g_list_length (magnifier->zoom_regions);
	list = GNOME_Magnifier_ZoomRegionList__alloc ();
	list->_length = len;
	list->_buffer =
		GNOME_Magnifier_ZoomRegionList_allocbuf (list->_length);
	for (i = 0; i < len; ++i) {
		objref = g_list_nth_data (magnifier->zoom_regions, i);
		list->_buffer [i] =
			CORBA_Object_duplicate (objref, ev);
	}
	CORBA_sequence_set_release (list, CORBA_TRUE);
	
	return list; 
}

static void
impl_magnifier_clear_all_zoom_regions (PortableServer_Servant servant,
				       CORBA_Environment * ev)
{
	Magnifier *magnifier = MAGNIFIER (bonobo_object_from_servant (servant));

	fprintf (stderr, "Destroying all zoom regions!\n");
	g_list_foreach (magnifier->zoom_regions,
			magnifier_unref_zoom_region, magnifier);
	g_list_free (magnifier->zoom_regions);
	magnifier->zoom_regions = NULL;
}

static void
impl_magnifier_dispose (PortableServer_Servant servant,
			CORBA_Environment *ev)
{
	magnifier_do_dispose (
		MAGNIFIER (bonobo_object_from_servant (servant)));
}

static void
magnifier_class_init (MagnifierClass *klass)
{
        GObjectClass * object_class = (GObjectClass *) klass;
        POA_GNOME_Magnifier_Magnifier__epv *epv = &klass->epv;

	object_class->dispose = magnifier_gobject_dispose;

        epv->_set_SourceDisplay = impl_magnifier_set_source_display;
	epv->_set_TargetDisplay = impl_magnifier_set_target_display;
	epv->getProperties = impl_magnifier_get_properties;
	epv->getZoomRegions = impl_magnifier_get_zoom_regions;
	epv->createZoomRegion = impl_magnifier_create_zoom_region;
	epv->addZoomRegion = impl_magnifier_add_zoom_region;
	epv->clearAllZoomRegions = impl_magnifier_clear_all_zoom_regions;
	epv->dispose = impl_magnifier_dispose;
}

static void
magnifier_properties_init (Magnifier *magnifier)
{
	BonoboArg *def;
	GNOME_Magnifier_RectBounds rect_bounds;
	gchar *display_env;

	magnifier->property_bag =
		bonobo_property_bag_new_closure (
			g_cclosure_new_object (
				G_CALLBACK (magnifier_get_property),
				G_OBJECT (magnifier)),
			g_cclosure_new_object (
				G_CALLBACK (magnifier_set_property),
				G_OBJECT (magnifier)));

	/* Aggregate so magnifier implements Bonobo_PropertyBag */
	bonobo_object_add_interface (BONOBO_OBJECT (magnifier),
				     BONOBO_OBJECT (magnifier->property_bag));

	def = bonobo_arg_new (BONOBO_ARG_STRING);
	display_env = getenv ("DISPLAY");
	BONOBO_ARG_SET_STRING (def, display_env);

	bonobo_property_bag_add (magnifier->property_bag,
				 "source-display-screen",
				 MAGNIFIER_SOURCE_DISPLAY_PROP,
				 BONOBO_ARG_STRING,
				 def,
				 "source display screen",
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_property_bag_add (magnifier->property_bag,
				 "target-display-screen",
				 MAGNIFIER_TARGET_DISPLAY_PROP,
				 BONOBO_ARG_STRING,
				 def,
				 "target display screen",
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);

	magnifier_init_display (magnifier, display_env, TRUE);
	magnifier_init_display (magnifier, display_env, FALSE);

	magnifier_get_display_rect_bounds (magnifier, &rect_bounds, FALSE);
	def = bonobo_arg_new_from (TC_GNOME_Magnifier_RectBounds, &rect_bounds);
        
	bonobo_property_bag_add (magnifier->property_bag,
				 "source-display-bounds",
				 MAGNIFIER_SOURCE_SIZE_PROP,
				 TC_GNOME_Magnifier_RectBounds,
				 def,
				 "source display bounds/size",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	bonobo_arg_release (def);
	
	magnifier_get_display_rect_bounds (magnifier, &rect_bounds, TRUE);
	def = bonobo_arg_new_from (TC_GNOME_Magnifier_RectBounds, &rect_bounds);

	bonobo_property_bag_add (magnifier->property_bag,
				 "target-display-bounds",
				 MAGNIFIER_TARGET_SIZE_PROP,
				 TC_GNOME_Magnifier_RectBounds,
				 def,
				 "target display bounds/size",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	bonobo_arg_release (def);

	bonobo_property_bag_add (magnifier->property_bag,
				 "cursor-set",
				 MAGNIFIER_CURSOR_SET_PROP,
				 BONOBO_ARG_STRING,
				 NULL,
				 "name of cursor set",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	def = bonobo_arg_new (BONOBO_ARG_INT);
	BONOBO_ARG_SET_INT (def, 64);
	
	bonobo_property_bag_add (magnifier->property_bag,
				 "cursor-size",
				 MAGNIFIER_CURSOR_SIZE_PROP,
				 BONOBO_ARG_INT,
				 def,
				 "cursor size, in pixels",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	bonobo_arg_release (def);
	
	bonobo_property_bag_add (magnifier->property_bag,
				 "cursor-scale-factor",
				 MAGNIFIER_CURSOR_ZOOM_PROP,
				 BONOBO_ARG_FLOAT,
				 NULL,
				 "scale factor for cursors (overrides size)",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	
	bonobo_property_bag_add (magnifier->property_bag,
				 "cursor-color",
				 MAGNIFIER_CURSOR_COLOR_PROP,
				 TC_CORBA_unsigned_long,
				 NULL,
				 "foreground color for 1-bit cursors, as ARGB",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);	

	bonobo_property_bag_add (magnifier->property_bag,
				 "cursor-hotspot",
				 MAGNIFIER_CURSOR_HOTSPOT_PROP,
				 TC_GNOME_Magnifier_Point,
				 NULL,
				 "hotspot relative to cursor's upper-left-corner, at default resolition",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	
	bonobo_property_bag_add (magnifier->property_bag,
				 "cursor-default-size",
				 MAGNIFIER_CURSOR_DEFAULT_SIZE_PROP,
				 BONOBO_ARG_INT,
				 NULL,
				 "default size of current cursor set",
				 Bonobo_PROPERTY_READABLE);

	bonobo_property_bag_add (magnifier->property_bag,
				 "crosswire-size",
				 MAGNIFIER_CROSSWIRE_SIZE_PROP,
				 BONOBO_ARG_INT,
				 NULL,
				 "thickness of crosswire cursor, in target pixels",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	
	bonobo_property_bag_add (magnifier->property_bag,
				 "crosswire-color",
				 MAGNIFIER_CROSSWIRE_COLOR_PROP,
				 BONOBO_ARG_LONG,
				 NULL,
				 "color of crosswire, as A-RGB; note that alpha is required. (use 0 for XOR wire)",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_property_bag_add (magnifier->property_bag,
				 "crosswire-clip",
				 MAGNIFIER_CROSSWIRE_CLIP_PROP,
				 BONOBO_ARG_BOOLEAN,
				 NULL,
				 "whether to inset the cursor over the crosswire or not",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
}

static void
magnifier_init_window (Magnifier *magnifier)
{
	GtkWindowType mag_win_type = GTK_WINDOW_TOPLEVEL;
	if (_is_override_redirect) mag_win_type = GTK_WINDOW_POPUP;

	magnifier->priv->w =
		g_object_connect (gtk_widget_new (gtk_window_get_type (),
						  "user_data", NULL,
						  "can_focus", FALSE,
						  "type", mag_win_type,  
						  "title", "magnifier",
						  "allow_grow", TRUE,
						  "allow_shrink", TRUE,
						  "border_width", 0,
						  NULL),
				  "signal::realize", magnifier_realize, NULL,
				  "signal::size_allocate", magnifier_size_allocate, NULL,
				  "signal::destroy", magnifier_exit, NULL,
				  NULL);
	magnifier->priv->canvas = gtk_fixed_new ();
	gtk_container_add (GTK_CONTAINER (magnifier->priv->w),
			   magnifier->priv->canvas);
	magnifier->priv->root = NULL;
}

static void
magnifier_init (Magnifier *magnifier)
{
	magnifier->priv = g_new0 (MagnifierPrivate, 1);
	magnifier_properties_init (magnifier);
	magnifier->zoom_regions = NULL;
	magnifier->source_screen_num = 0;
	magnifier->target_screen_num = 0;
	magnifier->cursor_size_x = 0;
	magnifier->cursor_size_y = 0;
	magnifier->cursor_scale_factor = 1.0F;
	magnifier->cursor_color = 0xFF000000;
	magnifier->crosswire_size = 1;
	magnifier->crosswire_color = 0;
	magnifier->crosswire_clip = FALSE;
	magnifier->cursor_hotspot.x = 0;
	magnifier->cursor_hotspot.y = 0;
	magnifier->priv->cursor = NULL;
	magnifier->priv->w = NULL;
	magnifier->priv->use_source_cursor = TRUE;
	magnifier->priv->cursorlist = NULL;
	magnifier_init_window (magnifier);
	magnifier_init_cursor_set (magnifier, "default");
}

GdkDrawable *
magnifier_get_cursor (Magnifier *magnifier)
{
        if (magnifier->priv->cursor == NULL) {
	    if ((fixes_event_base == 0) && 
		strcmp (magnifier->cursor_set, "none")) 
	    {
		GdkPixbuf *pixbuf;
		gchar *default_cursor_filename = 
		    g_strconcat (CURSORSDIR, "/", "default-cursor.xpm", NULL);
		pixbuf = gdk_pixbuf_new_from_file (default_cursor_filename, NULL);
		if (pixbuf) 
		{
		    magnifier_set_cursor_from_pixbuf (magnifier, pixbuf);
		    g_object_unref (pixbuf);
		    magnifier_transform_cursor (magnifier);
		}
		g_free (default_cursor_filename);
	    }
        }
	return magnifier->priv->cursor;
}

Magnifier *
magnifier_new (gboolean override_redirect)
{
	Magnifier *mag;
	MagLoginHelper *helper;
	int ret;

	_is_override_redirect = override_redirect;

	mag = g_object_new (magnifier_get_type(), NULL);

	_this_magnifier = mag; /* FIXME what about multiple instances? */

	helper = g_object_new (mag_login_helper_get_type (), NULL);
	mag_login_helper_set_magnifier (helper, mag);

	bonobo_object_add_interface (bonobo_object (mag), 
				     BONOBO_OBJECT (helper));
	
	ret = bonobo_activation_active_server_register (
		MAGNIFIER_OAFIID, BONOBO_OBJREF (mag));
	if (ret != Bonobo_ACTIVATION_REG_SUCCESS)
	    g_error ("Error registering magnifier server.\n");
	else
	    g_message ("The magnifier server was successfully registred.\n");

	g_idle_add (magnifier_reset_struts_at_idle, mag);

	return mag;
}

BONOBO_TYPE_FUNC_FULL (Magnifier, 
		       GNOME_Magnifier_Magnifier,
		       BONOBO_TYPE_OBJECT,
		       magnifier)

