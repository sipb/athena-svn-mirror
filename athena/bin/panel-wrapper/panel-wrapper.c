/* panel-wrapper.c - gnome-session substitute for Athena
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
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <gtk/gtk.h>
#include <gtk/gtkinvisible.h>
#include <gdk/gdkx.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

/* Some globals for rendering the iris.  */
static GdkRectangle iris_rect;
static gint iris_block = 0;
static GdkGC *iris_gc = NULL;

static gint
iris_callback(gpointer data)
{
  gint i;
  gint width_step;
  gint height_step;
  GdkPoint points[5];
  static gchar dash_list[2] = {1, 1};

  width_step = MIN(iris_rect.width / 2, iris_block);
  height_step = MIN(iris_rect.width / 2, iris_block);

  for (i = 0; i < MIN(width_step, height_step); i++)
    {
      gdk_gc_set_dashes(iris_gc, i % 1, dash_list, 2);

      points[0].x = iris_rect.x + i;
      points[0].y = iris_rect.y + i;
      points[1].x = iris_rect.x + iris_rect.width - i;
      points[1].y = iris_rect.y + i;
      points[2].x = iris_rect.x + iris_rect.width - i;
      points[2].y = iris_rect.y + iris_rect.height - i;
      points[3].x = iris_rect.x + i;
      points[3].y = iris_rect.y + iris_rect.height - i;
      points[4].x = iris_rect.x + i;
      points[4].y = iris_rect.y + i;

      gdk_draw_lines(GDK_ROOT_PARENT(), iris_gc, points, 5);
    }

  gdk_flush();

  iris_rect.x += width_step;
  iris_rect.y += width_step;
  iris_rect.width -= MIN(iris_rect.width, iris_block * 2);
  iris_rect.height -= MIN(iris_rect.height, iris_block * 2);

  if (iris_rect.width == 0 || iris_rect.height == 0)
    {
      gtk_main_quit();
      return FALSE;
    }
  else
    return TRUE;
}

static void
iris(void)
{
  iris_rect.x = 0;
  iris_rect.y = 0;
  iris_rect.width = gdk_screen_width();
  iris_rect.height = gdk_screen_height();

  if (!iris_gc)
    {
      GdkGCValues values;

      values.line_style = GDK_LINE_ON_OFF_DASH;
      values.subwindow_mode = GDK_INCLUDE_INFERIORS;

      iris_gc = gdk_gc_new_with_values(GDK_ROOT_PARENT(),
				       &values,
				       GDK_GC_LINE_STYLE | GDK_GC_SUBWINDOW);
    }

  /* Plan for a time of 0.5 seconds for effect */
  iris_block = iris_rect.height / (500 / 20);
  if (iris_block < 8)
    iris_block = 8;

  gtk_timeout_add(20, iris_callback, NULL);

  gtk_main();
}

static void
refresh_screen(void)
{
  GdkWindowAttr attributes;
  GdkWindow *window;

  attributes.x = 0;
  attributes.y = 0;
  attributes.width = gdk_screen_width();
  attributes.height = gdk_screen_height();
  attributes.window_type = GDK_WINDOW_TOPLEVEL;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.override_redirect = TRUE;
  attributes.event_mask = 0;
  
  window = gdk_window_new(NULL, &attributes,
			  GDK_WA_X | GDK_WA_Y | GDK_WA_NOREDIR);

  gdk_window_show(window);
  gdk_flush();
  gdk_window_hide(window);
}

static gboolean
display_gui(void)
{
  GtkWidget *box;
  GtkWidget *toggle_button = NULL;
  gint result;
  gchar *s;
  GtkWidget *halt = NULL;
  GtkWidget *reboot = NULL;
  GtkWidget *invisible;


  /* It's really bad here if someone else has the pointer
   * grabbed, so we first grab the pointer and keyboard
   * to an offscreen window, and then once we have the
   * server grabbed, move that to our dialog.
   */
  gtk_rc_reparse_all();

  invisible = gtk_invisible_new();
  gtk_widget_show(invisible);

  while (1)
    {
      if (gdk_pointer_grab(invisible->window, FALSE, 0,
			   NULL, NULL, GDK_CURRENT_TIME) == Success)
	{
	  if (gdk_keyboard_grab(invisible->window, FALSE,
				GDK_CURRENT_TIME) == Success)
	    break;
	  gdk_pointer_ungrab(GDK_CURRENT_TIME);
	}
      sleep (1);
    }

  XGrabServer(GDK_DISPLAY());
  iris();

  box = gnome_message_box_new("Really log out?",
			      GNOME_MESSAGE_BOX_QUESTION,
			      GNOME_STOCK_BUTTON_YES,
			      GNOME_STOCK_BUTTON_NO,
			      NULL);

  gtk_object_set(GTK_OBJECT(box), "type", GTK_WINDOW_POPUP, NULL);

  gnome_dialog_set_default(GNOME_DIALOG(box), 0);
  gtk_window_set_position(GTK_WINDOW(box), GTK_WIN_POS_CENTER);
  gtk_window_set_policy(GTK_WINDOW(box), FALSE, FALSE, TRUE);

  gnome_dialog_close_hides(GNOME_DIALOG(box), TRUE);

  gtk_container_set_border_width(GTK_CONTAINER(GNOME_DIALOG(box)->vbox),
				 GNOME_PAD);
  gtk_widget_show_all(box);

  /* Move the grabs to our message box */
  gdk_pointer_grab(box->window, TRUE, 0,
		   NULL, NULL, GDK_CURRENT_TIME);
  gdk_keyboard_grab(invisible->window, FALSE, GDK_CURRENT_TIME);

  /* Due to an apparent bug in gnome-dialog, even though the Yes is
   * the default, No somehow gets focus and thus temporarily grabs
   * default as well.
   */
  if (GTK_WINDOW(box)->default_widget != NULL)
    gtk_widget_grab_focus(GTK_WINDOW(box)->default_widget);

  result = gnome_dialog_run(GNOME_DIALOG(box));

  refresh_screen();
  XUngrabServer(GDK_DISPLAY());

  gdk_pointer_ungrab(GDK_CURRENT_TIME);
  gdk_keyboard_ungrab(GDK_CURRENT_TIME);

  gdk_flush();

  return (result == 0);
}

static void confirm_logout(void)
{
  if (display_gui())
    exit(0);
}

int main(int argc, char **argv)
{
  int c;
  pid_t pid, wpid;
  int status;

  gnome_init("panel-wrapper", "1.0", argc, argv);

  while (1)
    {
      pid = fork();
      if (pid == 0)
	{
	  /* In the child, execute the panel program. */
	  execlp("panel", "panel", (char *) NULL);
	  perror("panel");
	  _exit(1);
	}

      /* In the parent, wait for the child to exit. */
      while ((wpid = wait(&status)) == -1 && errno == EINTR)
	;
      if (wpid == -1)
	{
	  perror("panel-wrapper: wait");
	  exit(1);
	}
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
	confirm_logout();
    }
}
