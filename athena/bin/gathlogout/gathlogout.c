/* gathlogout.c - gnome-session logout substitute for Athena
   Written by Greg Hudson <ghudson@mit.edu>
   Copyright (C) MIT

   User interface code based heavily on gnome-session's logout.c
   Written by Owen Taylor <otaylor@redhat.com>
   Copyright (C) Red Hat

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <gtk/gtkinvisible.h>
#include <gdk/gdkx.h>

typedef struct {
  GdkScreen    *screen;
  GdkRectangle  area;
  int           rowstride;
  GdkWindow    *root_window;
  GdkWindow    *draw_window;
  GdkPixbuf    *start_pb, *end_pb, *frame;
  guchar       *start_p, *end_p, *frame_p;
  GTimeVal      start_time;
  GdkGC        *gc;
} FadeoutData;

static GList *fadeout_windows = NULL;

/* Go for five seconds */
#define FADE_DURATION 1500.0

static void
get_current_frame (FadeoutData *fadeout,
		   double    sat)
{
  guchar *sp, *ep, *fp;
  int i, j, width, offset;

  width = fadeout->area.width * 3;
  offset = 0;
  
  for (i = 0; i < fadeout->area.height; i++)
    {
      sp = fadeout->start_p + offset;
      ep = fadeout->end_p   + offset;
      fp = fadeout->frame_p + offset;

      for (j = 0; j < width; j += 3)
	{
	  guchar r = abs (*(sp++) - ep[0]);
	  guchar g = abs (*(sp++) - ep[1]);
	  guchar b = abs (*(sp++) - ep[2]);

	  *(fp++) = *(ep++) + r * sat;
	  *(fp++) = *(ep++) + g * sat;
	  *(fp++) = *(ep++) + b * sat;
	}

      offset += fadeout->rowstride;
    }
}

static void
darken_pixbuf (GdkPixbuf *pb)
{
  int width, height, rowstride;
  int i, j;
  guchar *p, *pixels;
  
  width     = gdk_pixbuf_get_width (pb) * 3;
  height    = gdk_pixbuf_get_height (pb);
  rowstride = gdk_pixbuf_get_rowstride (pb);
  pixels    = gdk_pixbuf_get_pixels (pb);
  
  for (i = 0; i < height; i++)
    {
      p = pixels + (i * rowstride);
      for (j = 0; j < width; j++)
	p [j] >>= 1;
    }
}

static gboolean
fadeout_callback (FadeoutData *fadeout)
{
  GTimeVal current_time;
  double elapsed, percent;

  g_get_current_time (&current_time);
  elapsed = ((((double)current_time.tv_sec - fadeout->start_time.tv_sec) * G_USEC_PER_SEC +
	      (current_time.tv_usec - fadeout->start_time.tv_usec))) / 1000.0;

  if (elapsed < 0)
    {
      g_warning ("System clock seemed to go backwards?");
      elapsed = G_MAXDOUBLE;
    }

  if (elapsed > FADE_DURATION)
    {
      gdk_draw_pixbuf (fadeout->draw_window,
		       fadeout->gc,
		       fadeout->end_pb,
		       0, 0,
		       0, 0,
		       fadeout->area.width,
		       fadeout->area.height,
		       GDK_RGB_DITHER_NONE,
		       0, 0);

      g_object_unref (fadeout->gc);
      g_object_unref (fadeout->start_pb);
      g_object_unref (fadeout->end_pb);
      g_object_unref (fadeout->frame);

      g_free (fadeout);
    
      return FALSE;
    }

  percent = elapsed / FADE_DURATION;

  get_current_frame (fadeout, 1.0 - percent);
  gdk_draw_pixbuf (fadeout->draw_window,
		   fadeout->gc,
		   fadeout->frame,
		   0, 0,
		   0, 0,
		   fadeout->area.width,
		   fadeout->area.height,
		   GDK_RGB_DITHER_NONE,
		   0, 0);

  gdk_flush ();
  
  return TRUE;
}
  
static void
fadeout_screen (GdkScreen *screen)
{
  GdkWindowAttr attr;
  int attr_mask;
  GdkGCValues values;
  FadeoutData *fadeout;

  fadeout = g_new (FadeoutData, 1);

  fadeout->screen = screen;
  gdk_screen_get_monitor_geometry (screen, 0, &fadeout->area);

  fadeout->root_window = gdk_screen_get_root_window (screen);
  attr.window_type = GDK_WINDOW_CHILD;
  attr.x = fadeout->area.x;
  attr.y = fadeout->area.y;
  attr.width = fadeout->area.width;
  attr.height = fadeout->area.height;
  attr.wclass = GDK_INPUT_OUTPUT;
  attr.visual = gdk_screen_get_system_visual (fadeout->screen);
  attr.colormap = gdk_screen_get_default_colormap (fadeout->screen);
  attr.override_redirect = TRUE;
  attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP | GDK_WA_NOREDIR;

  fadeout->draw_window = gdk_window_new (fadeout->root_window, &attr, attr_mask);
  fadeout_windows = g_list_prepend (fadeout_windows, fadeout->draw_window);
  
  fadeout->start_pb = gdk_pixbuf_get_from_drawable (NULL,
						    fadeout->root_window,
						    NULL,
						    fadeout->area.x,
						    fadeout->area.y,
						    0, 0,
						    fadeout->area.width,
						    fadeout->area.height);
  
  fadeout->end_pb = gdk_pixbuf_copy (fadeout->start_pb);
  darken_pixbuf (fadeout->end_pb);
  
  fadeout->frame = gdk_pixbuf_copy (fadeout->start_pb);
  fadeout->rowstride = gdk_pixbuf_get_rowstride (fadeout->start_pb);

  fadeout->start_p = gdk_pixbuf_get_pixels (fadeout->start_pb);
  fadeout->end_p   = gdk_pixbuf_get_pixels (fadeout->end_pb);
  fadeout->frame_p = gdk_pixbuf_get_pixels (fadeout->frame);
  
  values.subwindow_mode = GDK_INCLUDE_INFERIORS;

  fadeout->gc = gdk_gc_new_with_values (fadeout->root_window, &values, GDK_GC_SUBWINDOW);

  gdk_window_set_back_pixmap (fadeout->draw_window, NULL, FALSE);
  gdk_window_show (fadeout->draw_window);
  gdk_draw_pixbuf (fadeout->draw_window,
		   fadeout->gc,
		   fadeout->frame,
		   0, 0,
		   0, 0,
		   fadeout->area.width,
		   fadeout->area.height,
		   GDK_RGB_DITHER_NONE,
		   0, 0);
  
  g_get_current_time (&fadeout->start_time);
  g_idle_add ((GSourceFunc) fadeout_callback, fadeout);
}

static void
hide_fadeout_windows (void)
{
  GList *l;

  for (l = fadeout_windows; l; l = l->next)
    {
      gdk_window_hide (GDK_WINDOW (l->data));
      g_object_unref (l->data);
    }

  g_list_free (fadeout_windows);
  fadeout_windows = NULL;
}

static GtkWidget *
make_title_label (const char *text)
{
  GtkWidget *label;
  char *full;

  full = g_strdup_printf ("<span weight=\"bold\">%s</span>", text);
  label = gtk_label_new (full);
  g_free (full);

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

  return label;
}

static gboolean
display_gui (void)
{
  GtkWidget *box;
  GtkWidget *title;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *image;
  GtkWidget *toggle_button = NULL;
  gint response;
  GtkWidget *halt = NULL;
  GtkWidget *reboot = NULL;
  GtkWidget *invisible;
  gboolean halt_supported = FALSE;
  gboolean reboot_supported = FALSE;
  gboolean retval = FALSE;
  gboolean save_active = FALSE;
  gboolean halt_active = FALSE;
  gboolean reboot_active = FALSE;
  gboolean a11y_enabled;
  GError *error = NULL;
  GdkScreen *screen;
  int monitor;

  /* It's really bad here if someone else has the pointer
   * grabbed, so we first grab the pointer and keyboard
   * to an offscreen window, and then once we have the
   * server grabbed, move that to our dialog.
   */
  gtk_rc_reparse_all ();

  screen = gdk_screen_get_default ();

  invisible = gtk_invisible_new_for_screen (screen);

  gtk_widget_show (invisible);

  a11y_enabled = GTK_IS_ACCESSIBLE (gtk_widget_get_accessible (invisible));

  /* Only create a managed window if a11y is enabled */
  if (!a11y_enabled)
    {
      while (1)
	{
	  if (gdk_pointer_grab (invisible->window, FALSE, 0,
				NULL, NULL, GDK_CURRENT_TIME) == Success)
	    {
	      if (gdk_keyboard_grab (invisible->window, FALSE, GDK_CURRENT_TIME)
		  == Success)
		break;
	      gdk_pointer_ungrab (GDK_CURRENT_TIME);
	    }
	  sleep (1);
	}

      box = g_object_new (GTK_TYPE_DIALOG,
			  "type", GTK_WINDOW_POPUP,
			  NULL);
    }
  else
    {
      box = gtk_dialog_new ();
      atk_object_set_role (gtk_widget_get_accessible (box), ATK_ROLE_ALERT);
      gtk_window_set_decorated (GTK_WINDOW (box), FALSE);
    }

  gtk_dialog_set_has_separator (GTK_DIALOG (box), FALSE);

  vbox = gtk_vbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (box)->vbox), vbox, FALSE, FALSE, 0);
  gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (box)->vbox), 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_widget_show (vbox);
  
  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  image = gtk_image_new_from_stock ("gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);
	
  title = make_title_label ("Are you sure you want to log out?");
  gtk_box_pack_start (GTK_BOX (hbox), title, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (title), 0, 0.5);
  gtk_widget_show (title);
  
  gtk_dialog_add_button (GTK_DIALOG (box), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button (GTK_DIALOG (box), GTK_STOCK_OK, GTK_RESPONSE_OK);

  gtk_dialog_set_default_response (GTK_DIALOG (box), GTK_RESPONSE_OK);
  gtk_window_set_screen (GTK_WINDOW (box), screen);
  gtk_window_set_policy (GTK_WINDOW (box), FALSE, FALSE, TRUE);

  gtk_container_set_border_width (GTK_CONTAINER (box), 5);

  gtk_window_set_resizable (GTK_WINDOW (box), FALSE);
  gtk_window_set_position (GTK_WINDOW (box), GTK_WIN_POS_CENTER);

  /* Grabbing the Xserver when accessibility is enabled will cause
   * a hang. See #93103 for details.
   */
  if (!a11y_enabled)
    {
      XGrabServer (GDK_DISPLAY ());
      fadeout_screen (screen);
    }

  gtk_widget_show_all (box);

  if (!a11y_enabled)
    {
      /* Move the grabs to our message box */
      gdk_pointer_grab (box->window, TRUE, 0,
			NULL, NULL, GDK_CURRENT_TIME);
      gdk_keyboard_grab (box->window, FALSE, GDK_CURRENT_TIME);
      XSetInputFocus (GDK_DISPLAY (),
		      GDK_WINDOW_XWINDOW (box->window),
		      RevertToParent,
		      CurrentTime);
    }

  response = gtk_dialog_run (GTK_DIALOG (box));

  if (halt)
    halt_active = GTK_TOGGLE_BUTTON (halt)->active;

  if (reboot)
    reboot_active = GTK_TOGGLE_BUTTON (reboot)->active;

  if (toggle_button)
    save_active = GTK_TOGGLE_BUTTON (toggle_button)->active;

  gtk_widget_destroy (box);
  gtk_widget_destroy (invisible);

  if (!a11y_enabled)
    {
      hide_fadeout_windows ();
      XUngrabServer (GDK_DISPLAY ());

      gdk_pointer_ungrab (GDK_CURRENT_TIME);
      gdk_keyboard_ungrab (GDK_CURRENT_TIME);

      gdk_flush ();
    }

  return (response == GTK_RESPONSE_OK);
}

int main (int argc, char **argv)
{
  const char *pidstr;

  gtk_init (&argc, &argv);
  if (display_gui ())
    {
      pidstr = getenv ("XSESSION");
      if (pidstr && isdigit (*pidstr))
	kill (atoi (pidstr), SIGHUP);
      else
	execlp ("/usr/athena/bin/end_session", "end_session", (char *) NULL);
      return 0;
    }

  return 1;
}
