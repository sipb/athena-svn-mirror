/*
 * GNOME-MAG Magnification service for GNOME
 *
 * Copyright 2004 Sun Microsystems Inc.
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
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#ifdef HAVE_DAMAGE
#include <X11/extensions/Xdamage.h>
#endif
#ifdef HAVE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif
#include "magnifier.h"
#include "magnifier-private.h"

Display *damage_client_connection = NULL;
guint   damage_client_gsource = 0;

#ifdef HAVE_DAMAGE
Damage _magnifier_client_damage;
#endif
int damage_event_base, damage_error_base;

#ifdef HAVE_XFIXES
XserverRegion region;
#endif

gboolean
magnifier_damage_reset (gpointer data)
{
#ifdef HAVE_DAMAGE
#ifdef HAVE_XFIXES
  XDamageSubtract (damage_client_connection, _magnifier_client_damage, None, region);
  XFlush (damage_client_connection);
#endif
#endif
    return FALSE;
}

gboolean
magnifier_damage_handler (GIOChannel *source, GIOCondition condition, gpointer data)
{
#ifdef HAVE_DAMAGE
  XEvent ev;
  Magnifier *magnifier = (Magnifier *) data;
#ifdef HAVE_XFIXES
  XRectangle *rectlist;

  do
  {
      XNextEvent(damage_client_connection, &ev);
      if (ev.type == damage_event_base + XDamageNotify) 
      {
	  XDamageNotifyEvent  *dev = (XDamageNotifyEvent *) &ev;
#ifdef DEBUG_DAMAGE
	  g_message ("Damage %3d, %3d x %3d, %3d\n",
		     (int) dev->area.x, (int) dev->area.x + dev->area.width,
		     (int) dev->area.y, (int) dev->area.y + dev->area.height);
#endif
      }
  } while (XPending (damage_client_connection));
  XDamageSubtract (damage_client_connection, _magnifier_client_damage, None, region);
  
  if (magnifier)
  {
      int i, howmany;
      /* TODO: maintain this list on the client instead, to avoid the roundtrip below */
      rectlist = XFixesFetchRegion (damage_client_connection, region, &howmany);
      if (rectlist == NULL) /* no reply from fetch */
	  return TRUE;
      for (i=0; i < howmany; ++i) {
	  magnifier_notify_damage (magnifier, &rectlist[i]);
      }
      XFree (rectlist);
  }

  XFlush (damage_client_connection);
#endif

  return TRUE;
#else
  return FALSE;
#endif
}

gboolean
magnifier_source_has_damage_extension (Magnifier *magnifier)
{
	int event_base, error_base;
	Display *dpy;
	g_assert (magnifier);
#ifdef HAVE_DAMAGE
	dpy = GDK_DISPLAY_XDISPLAY (magnifier->source_display);
	if (g_getenv ("MAGNIFIER_IGNORE_DAMAGE"))
	        return FALSE;
	if (XDamageQueryExtension (dpy, &event_base, &error_base))
		return TRUE;
#endif
	return FALSE;
}

gboolean
magnifier_damage_client_init (Magnifier *magnifier)
{
#ifdef HAVE_DAMAGE
    GIOChannel *ioc;
    int fd;
    Display *dpy;
    Window rootwin;

    if (damage_client_connection)
    {
        /* remove the old watch */
	if (damage_client_gsource) 
	  g_source_remove (damage_client_gsource);
	XCloseDisplay (damage_client_connection);
    }

    if (magnifier)
    {
	/* we need our own connection here to keep from gumming up the works */
	damage_client_connection = 
	  XOpenDisplay (magnifier->source_display_name); 
	rootwin = GDK_WINDOW_XWINDOW (magnifier->priv->root);
    }
    else 
    {
	dpy = GDK_DISPLAY ();
	damage_client_connection = XOpenDisplay (NULL);
	rootwin = RootWindow (damage_client_connection, DefaultScreen (damage_client_connection));
	g_message ("warning - using DefaultScreen for DAMAGE connection.");
    }

    if (!XDamageQueryExtension (damage_client_connection, 
				&damage_event_base, &damage_error_base))
    {
	g_warning ("Damage extension not currently active.\n");
	return FALSE;
    }
    else if (g_getenv ("MAGNIFIER_IGNORE_DAMAGE"))
    {
	g_warning ("Damage extension being ignored at user request.");
	return FALSE;
    }
    else
    {
	_magnifier_client_damage = XDamageCreate (damage_client_connection, 
						  rootwin,
						  XDamageReportDeltaRectangles);
#ifdef HAVE_XFIXES
	region = XFixesCreateRegion (damage_client_connection, 
				     0, 0);
#else
	return FALSE;
#endif
	fd = ConnectionNumber (damage_client_connection);
	ioc = g_io_channel_unix_new (fd);
	damage_client_gsource = 
	  g_io_add_watch (ioc, G_IO_IN | G_IO_HUP | G_IO_PRI | G_IO_ERR, magnifier_damage_handler, 
			  magnifier);
	g_io_channel_unref (ioc); 
	g_message ("added event source to damage connection");
	g_idle_add (magnifier_damage_reset, NULL);
    }
    return TRUE;
#else
    g_warning ("this copy of gnome-mag was built without damage extension support.\n");
    return FALSE;
#endif
}

