/* -*- mode: c; style: linux -*- */
/* -*- c-basic-offset: 2 -*- */

/* pipeline-tests.c
 * Copyright (C) 2002 Jan Schmidt
 *
 * Written by: Jan Schmidt <thaytan@mad.scientist.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <locale.h>
#include <string.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/gconf/gconf.h>

#include "pipeline-tests.h"
#define WID(s) glade_xml_get_widget (interface_xml, s)
static gint timeout_tag;

static GstElement *gst_test_pipeline;
static GstClock *s_clock;

static void pipeline_error_dlg(GtkWindow * parent,
                GSTPPipelineDescription * pipeline_desc);
/* User responded in the dialog */
static void
user_test_pipeline_response(GtkDialog * widget, gint response_id,
			    GladeXML * dialog)
{
  /* Close the window causing the test to end */
  gtk_widget_hide(GTK_WIDGET(widget));
}

/* Timer timeout has been occurred */
static gint user_test_pipeline_timeout( gpointer data )
{
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(data));
  return TRUE;
}

/* Build the pipeline */
static gboolean
build_test_pipeline(GSTPPipelineDescription * pipeline_desc)
{
  gboolean return_val = FALSE;
  GError *error = NULL;
  gchar* test_pipeline_str = NULL;
  gchar* full_pipeline_str = NULL;

  switch (pipeline_desc->test_type) {
    case TEST_PIPE_AUDIOSINK:
      test_pipeline_str = gst_gconf_get_string ("default/audiosink");
      break;
    case TEST_PIPE_VIDEOSINK:
      test_pipeline_str = gst_gconf_get_string ("default/videosink");
      break;
    case TEST_PIPE_SUPPLIED:
      test_pipeline_str = pipeline_desc->test_pipe;
      break;
  }
  switch (pipeline_desc->type) {
    case PIPE_TYPE_AUDIOSINK:
    case PIPE_TYPE_VIDEOSINK:
      full_pipeline_str = g_strdup_printf("{ %s ! %s }", test_pipeline_str, pipeline_desc->pipeline);
      break;
    case PIPE_TYPE_AUDIOSRC:
    case PIPE_TYPE_VIDEOSRC:
      full_pipeline_str = g_strdup_printf("{ %s ! %s }", pipeline_desc->pipeline, test_pipeline_str);
      break;
  }
  if (full_pipeline_str) {
    gst_test_pipeline = (GstElement *)gst_parse_launch(full_pipeline_str, &error);
    if (!error) {
      return_val = TRUE;
    }
    else {
      /* FIXME display the error? */
      g_error_free(error);
    }
 
    g_free(full_pipeline_str);
  }
  return return_val;
}

static void
pipeline_error_dlg(GtkWindow * parent,
		   GSTPPipelineDescription * pipeline_desc)
{
  gchar *errstr = g_strdup_printf( _("Failed to construct test pipeline for '%s'"),
		  pipeline_desc->name);
    if (parent == NULL) {
	    g_print(errstr);
    }
    else {
      GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(parent,
							  GTK_DIALOG_DESTROY_WITH_PARENT,
							  GTK_MESSAGE_ERROR,
							  GTK_BUTTONS_CLOSE,
							  errstr));
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(GTK_WIDGET(dialog));
    }
    g_free(errstr);
}

/* Construct and iterate the pipeline. Use the indicated parent
   for any user interaction window.
*/
void
user_test_pipeline(GladeXML * interface_xml,
		   GtkWindow * parent,
		   GSTPPipelineDescription * pipeline_desc)
{
  GtkDialog *dialog = NULL;
  gst_test_pipeline = NULL;
  s_clock = NULL;

  /* Build the pipeline */
  if (!build_test_pipeline(pipeline_desc)) {
    /* Show the error pipeline */
    pipeline_error_dlg(parent, pipeline_desc);
    return;
  }

  /* Setup the 'click ok when done' dialog */
  if (parent) {
    dialog = GTK_DIALOG(WID("test_pipeline"));
    /* g_return_if_fail(dialog != NULL); */
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    g_signal_connect(G_OBJECT(dialog), "response",
         (GCallback) user_test_pipeline_response,
         interface_xml);
  }

  /* Start the pipeline */
  if (gst_element_set_state(gst_test_pipeline, GST_STATE_PLAYING) !=
    GST_STATE_SUCCESS) {
    pipeline_error_dlg(parent, pipeline_desc);
    return;
  }

  s_clock = gst_bin_get_clock(GST_BIN(gst_test_pipeline));
  /* Show the dialog */
  if (dialog) {
    gtk_window_present(GTK_WINDOW(dialog));
    timeout_tag = gtk_timeout_add(50, user_test_pipeline_timeout, WID("test_pipeline_progress"));    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_timeout_remove( timeout_tag );
    gtk_widget_hide(GTK_WIDGET(dialog));
  }
  else {
    gboolean busy;
    gint secs = 0, max_secs = 5; /* A bit hacky: No parent dialog, run in limited test mode */
    do {
      secs++;
      g_print(".");
      g_usleep(1000000); // 1 sec
      busy = (secs < max_secs);
    } while (busy);
  }

  if (gst_test_pipeline) {
    gst_element_set_state(gst_test_pipeline, GST_STATE_NULL);
    /* Free up the pipeline */
    gst_object_unref(GST_OBJECT(gst_test_pipeline));
    gst_test_pipeline = NULL;
  }
}
