/*
 * Copyright © 2002 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL RED HAT
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Owen Taylor, Red Hat, Inc.
 */

/* This file implements an X protocol for synchronizing multiple
 * CD player applications. The convention is that there is one CD
 * player application that is the "Master", which is identifies
 * by owning the _REDHAT_CD_PLAYER:<hostname>:<device> selection.
 * When a player reliquishes the "Master" role, it destroys the
 * owner window of the selection.
 */
#include <string.h>
#include <unistd.h>		/* For gethostname() */

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "cd-selection.h"

struct _CDSelection
{
  GdkAtom selection_atom;
  GdkWindow *owner_window;
  GtkWidget *invisible;
};

#define SELECTION_NAME "_REDHAT_CD_PLAYER"

static GdkFilterReturn cd_selection_filter     (GdkXEvent   *xevent,
					        GdkEvent    *event,
					        gpointer     data);
static void            cd_selection_negotiate  (CDSelection *cd_selection);

static void
cd_selection_reset (CDSelection *cd_selection)
{
  if (cd_selection->owner_window)
    {
      gdk_window_remove_filter (cd_selection->owner_window,
				cd_selection_filter, cd_selection);
      gdk_window_unref (cd_selection->owner_window);
      cd_selection->owner_window = NULL;
    }

  if (cd_selection->invisible)
    {
      gtk_widget_destroy (cd_selection->invisible);
      cd_selection->invisible = NULL;
    }
}

static void
cd_selection_clear (GtkWidget         *widget,
		    GdkEventSelection *event,
		    gpointer           user_data)
{
  CDSelection *cd_selection = user_data;

  cd_selection_reset (cd_selection);
  cd_selection_negotiate (cd_selection);
}

static gboolean
cd_selection_find_existing (CDSelection *cd_selection)
{
  Display *xdisplay = GDK_DISPLAY ();
  Window old;

  gdk_error_trap_push ();
  old = XGetSelectionOwner (xdisplay,
			    gdk_x11_atom_to_xatom (cd_selection->selection_atom));
  if (old)
    {
      XSelectInput (xdisplay, old, StructureNotifyMask);
      cd_selection->owner_window = gdk_window_foreign_new (old);
    }
  XSync (xdisplay, False);
  
  if (gdk_error_trap_pop () == 0 && cd_selection->owner_window)
    {
      gdk_window_add_filter (cd_selection->owner_window,
			     cd_selection_filter, cd_selection);
      
      return TRUE;
    }
  else
    {
      if (cd_selection->owner_window)
	{
	  gdk_window_unref (cd_selection->owner_window);
	  cd_selection->owner_window = NULL;
	}

      return FALSE;
    }
}

static gboolean
cd_selection_claim (CDSelection *cd_selection)
{
  cd_selection->invisible = gtk_invisible_new ();
  g_signal_connect (cd_selection->invisible, "selection-clear-event",
		    G_CALLBACK (cd_selection_clear), cd_selection);
  
  
  if (gtk_selection_owner_set (cd_selection->invisible,
			       cd_selection->selection_atom,
			       GDK_CURRENT_TIME))
    {
      return TRUE;
    }
  else
    {
      cd_selection_reset (cd_selection);
      return FALSE;
    }
}

static void
cd_selection_negotiate (CDSelection *cd_selection)
{
  Display *xdisplay = GDK_DISPLAY ();
  gboolean found = FALSE;

  /* We don't need both the XGrabServer() and the loop here;
   * the XGrabServer() should make sure that we only go through
   * the loop once. It also works if you remove the XGrabServer()
   * and just have the loop, but then the selection ownership
   * can get transfered a bunch of times before things
   * settle down.
   */
  while (!found)
    {
      XGrabServer (xdisplay);

      if (cd_selection_find_existing (cd_selection))
	found = TRUE;
      else if (cd_selection_claim (cd_selection))
	found = TRUE;

      XUngrabServer (xdisplay);
      gdk_flush ();
    }
}

static GdkFilterReturn
cd_selection_filter (GdkXEvent *xevent,
		     GdkEvent  *event,
		     gpointer   data)
{
  CDSelection *cd_selection = data;
  XEvent *xev = (XEvent *)xevent;

  if (xev->xany.type == DestroyNotify &&
      xev->xdestroywindow.window == xev->xdestroywindow.event)
    {
      cd_selection_reset (cd_selection);
      cd_selection_negotiate (cd_selection);

      return GDK_FILTER_REMOVE;
    }

  return GDK_FILTER_CONTINUE;
}

CDSelection *
cd_selection_start (const char *device)
{
  CDSelection *cd_selection = g_new (CDSelection, 1);
  gchar *selection_name;
  
  char hostname[256];

  memset (hostname, 0, 256);
  gethostname (hostname, 256);
  hostname[255] = 0;

  selection_name = g_strdup_printf (SELECTION_NAME ":%s:%s",
				    hostname, device);
  cd_selection->selection_atom = gdk_atom_intern (selection_name, FALSE);
  g_free (selection_name);
  
  cd_selection->owner_window = NULL;
  cd_selection->invisible = NULL;

  cd_selection_negotiate (cd_selection);

  return cd_selection;
}

void
cd_selection_stop (CDSelection *cd_selection)
{
  cd_selection_reset (cd_selection);
  g_free (cd_selection);
}

gboolean
cd_selection_is_master (CDSelection *cd_selection)
{
  return cd_selection->invisible != NULL;
}
