/* Copyright 2001 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

static const char rcsid[] = "$Id: panel-wrapper.c,v 1.1 2001-01-12 18:22:47 ghudson Exp $";

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <gtk/gtk.h>

static GtkWidget *create_dialog(void);
static void confirm_logout(GtkWidget *dialog);
static void logout(GtkWidget *widget, gpointer data);
static void cancel(GtkWidget *widget, gpointer data);

int main(int argc, char **argv)
{
  int c;
  const char *progname = "panel";
  pid_t pid, wpid;
  int status;
  GtkWidget *dialog;

  gtk_init(&argc, &argv);
  dialog = create_dialog();

  while ((c = getopt(argc, argv, "p:")) != -1)
    {
      switch (c)
	{
	case 'p':
	  progname = optarg;
	  break;
	default:
	  break;
	}
    }

  while (1)
    {
      pid = fork();
      if (pid == 0)
	{
	  /* In the child, execute the panel program. */
	  execlp(progname, progname, (char *) NULL);
	  perror(progname);
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
	confirm_logout(dialog);
    }
}

static GtkWidget *create_dialog()
{
  GtkWidget *msg, *buttons, *yes_button, *no_button, *dialog;

  msg = gtk_label_new("Really log out?");

  buttons = gtk_hbutton_box_new();
  yes_button = gtk_button_new_with_label("Yes");
  gtk_container_add(GTK_CONTAINER(buttons), yes_button);
  no_button = gtk_button_new_with_label("No");
  gtk_container_add(GTK_CONTAINER(buttons), no_button);

  dialog = gtk_dialog_new();
  gtk_object_set(GTK_OBJECT(dialog), "type", GTK_WINDOW_POPUP, NULL);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), msg, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), buttons,
		     FALSE, FALSE, 0);

  gtk_widget_show(msg);
  gtk_widget_show(yes_button);
  gtk_widget_show(no_button);
  gtk_widget_show(buttons);

  gtk_signal_connect(GTK_OBJECT(yes_button), "clicked",
		     GTK_SIGNAL_FUNC(logout), NULL);
  gtk_signal_connect(GTK_OBJECT(no_button), "clicked",
		     GTK_SIGNAL_FUNC(cancel), NULL);

  return dialog;
}

static void confirm_logout(GtkWidget *dialog)
{
  gtk_widget_show(dialog);
  gtk_main();
  gtk_widget_hide(dialog);
  gtk_main_iteration_do(FALSE);
}

static void logout(GtkWidget *widget, gpointer data)
{
  exit(0);
}

static void cancel(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}
