/* GStreamer Mixer
 * Copyright (C) 2003 Ronald Bultje <rbultje@ronald.bitfreak.net>
 *
 * mixer.c: sample mixer application
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#undef USE_OPTION_WIDGET

#include <string.h>
#include <glib.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gst/gst.h>
#include <gst/mixer/mixer.h>
#include <gst/propertyprobe/propertyprobe.h>

typedef struct _MyMixerControls {
  GstMixer *mixer;
  GstMixerTrack *track;
  GList *adjustments;
  GtkWidget *lock, *mute, *record;
  gboolean locked;
} MyMixerControls;

/* private stock icons */
#define GST_MIXER_STOCK_PHONE "gst-mixer-phone"
#define GST_MIXER_STOCK_VIDEO "gst-mixer-video"
#define GST_MIXER_STOCK_TONE  "gst-mixer-tone"
#define GST_MIXER_STOCK_MIXER "gst-mixer-mixer"

static const struct {
  gchar *label,
	*pixmap;
} pix[] = {
  { "cd",         GTK_STOCK_CDROM       },
  { "line",       GNOME_STOCK_LINE_IN   },
  { "mic",        GNOME_STOCK_MIC       },
  { "mix",        GST_MIXER_STOCK_MIXER },
  { "pcm",        GST_MIXER_STOCK_TONE  },
  { "headphone",  NULL                  },
  { "phone",      GST_MIXER_STOCK_PHONE },
  { "speaker",    GNOME_STOCK_VOLUME    },
  { "video",      GST_MIXER_STOCK_VIDEO },
  { "volume",     GST_MIXER_STOCK_TONE  },
  { "master",     GST_MIXER_STOCK_TONE  },
  { NULL, NULL }
};

static void
cb_volume_changed (GtkAdjustment *adj,
		   gpointer       data)
{
  MyMixerControls *ctrl = (MyMixerControls *) data;
  gint *volumes, i = 0;
  GList *adjustments = ctrl->adjustments;

  if (ctrl->locked)
    return;
  ctrl->locked = TRUE;
  volumes = g_malloc (sizeof (gint) * g_list_length (adjustments));

  for ( ; adjustments != NULL; adjustments = adjustments->next) {
    GtkAdjustment *adj2 = (GtkAdjustment *) adjustments->data;

    if (ctrl->lock != NULL &&
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ctrl->lock))) {
      gtk_adjustment_set_value (adj2, gtk_adjustment_get_value (adj));
      volumes[i++] = gtk_adjustment_get_value (adj);
    } else {
      volumes[i++] = gtk_adjustment_get_value (adj2);
    }
  }

  gst_mixer_set_volume (ctrl->mixer, ctrl->track, volumes);

  g_free (volumes);
  ctrl->locked = FALSE;
}

static void
cb_lock_toggled (GtkWidget *button,
		 gpointer   data)
{
  MyMixerControls *ctrl = (MyMixerControls *) data;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
    /* get the mean value, and set it on the first adjustment.
     * the cb_olume_changed () callback will take care of the
     * rest */
    gint volume = 0, num = 0;
    GList *adjustments = ctrl->adjustments;

    for ( ; adjustments != NULL; adjustments = adjustments->next) {
      GtkAdjustment *adj = (GtkAdjustment *) adjustments->data;

      num++;
      volume += gtk_adjustment_get_value (adj);
    }

    /* safety check */
    if (ctrl->adjustments != NULL) {
      gtk_adjustment_set_value ((GtkAdjustment *) ctrl->adjustments->data,
				volume / num);
    }
  }
}

static void
cb_mute_toggled (GtkWidget *button,
		 gpointer   data)
{
  MyMixerControls *ctrl = (MyMixerControls *) data;
  gst_mixer_set_mute (ctrl->mixer, ctrl->track,
		      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)));
}

static void
cb_record_toggled (GtkWidget *button,
		   gpointer   data)
{
  MyMixerControls *ctrl = (MyMixerControls *) data;
  gst_mixer_set_record (ctrl->mixer, ctrl->track,
		        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)));
}

#if USE_OPTION_WIDGET
static void
cb_opt_changed (GtkComboBox *box,
		gpointer     data)
{
  MyMixerControls *ctrl = (MyMixerControls *) data;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *opt;

  if (gtk_combo_box_get_active_iter (box, &iter)) {
    model = gtk_combo_box_get_model (box);
    gtk_tree_model_get (model, &iter, 0, &opt, -1);
    gst_mixer_set_option (ctrl->mixer, GST_MIXER_OPTIONS (ctrl->track), opt);
  }
}
#endif

static MyMixerControls *
create_track_header (GstMixer      *mixer,
		     GtkWidget     *table,
		     gint           column_pos,
		     GstMixerTrack *track)
{
  GtkWidget *label, *image;
  gint i, c = track->num_channels ? track->num_channels : 1;
  MyMixerControls *ctrl = g_new0 (MyMixerControls, 1);
  gchar *str = NULL;
  gboolean found = FALSE;

  ctrl->mixer = mixer;
  ctrl->track = track;
  ctrl->locked = FALSE;

  /* image (optional) */
  for (i = 0; !found && pix[i].label != NULL; i++) {
    /* we dup the string to make the comparison case-insensitive */
    gchar *label_l = g_strdup (track->label),
	  *needle_l = g_strdup (pix[i].label);
    gint pos;

    /* make case insensitive */
    for (pos = 0; label_l[pos] != '\0'; pos++)
      label_l[pos] = g_ascii_tolower (label_l[pos]);
    for (pos = 0; needle_l[pos] != '\0'; pos++)
      needle_l[pos] = g_ascii_tolower (needle_l[pos]);

    if (g_strrstr (label_l, needle_l) != NULL) {
      str = pix[i].pixmap;
      found = TRUE;
    }

    g_free (label_l);
    g_free (needle_l);
  }

  if (str != NULL) {
    if ((image = gtk_image_new_from_stock (str, GTK_ICON_SIZE_MENU)) != NULL) {
      gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.5);
      gtk_table_attach (GTK_TABLE (table), image,
			column_pos, column_pos + c,
			0, 1, 5, 0, 0, 0);
    }
  }

  /* label */
  label = gtk_label_new (track->label);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_table_attach (GTK_TABLE (table), label,
		    column_pos, column_pos + c,
		    1, 2, 5, 0, 0, 0);

  return ctrl;
}

static void
create_track_widget (GstMixer      *mixer,
                     GtkWidget     *table,
                     gint           column_pos,
                     GstMixerTrack *track)
{
  GtkWidget *slider, *button;
  gint i, *volumes;
  GList *adjlist = NULL;
  GtkObject *adj;
  AtkObject *accessible;
  gchar *accessible_name;
  MyMixerControls *ctrl;

  /* header */
  ctrl = create_track_header (mixer, table, column_pos, track);

  /* now sliders for each of the tracks */
  volumes = g_malloc (sizeof (gint) * track->num_channels);
  gst_mixer_get_volume (mixer, track, volumes);
  for (i = 0; i < track->num_channels; i++) {
    adj = gtk_adjustment_new (volumes[i],
			      track->min_volume, track->max_volume,
			      1.0, 1.0, 0.0);
    adjlist = g_list_append (adjlist, (gpointer) adj);
    g_signal_connect (G_OBJECT (adj), "value_changed",
		      G_CALLBACK (cb_volume_changed), (gpointer) ctrl);
    slider = gtk_vscale_new (GTK_ADJUSTMENT (adj));
    gtk_scale_set_draw_value (GTK_SCALE (slider), FALSE);
    gtk_range_set_inverted (GTK_RANGE (slider), TRUE);
    gtk_widget_set_size_request (slider, -1, 100);
    gtk_table_attach_defaults (GTK_TABLE (table), slider,
			       column_pos + i, column_pos + i + 1,
			       2, 3);
    accessible = gtk_widget_get_accessible (slider);
    if (GTK_IS_ACCESSIBLE (accessible)) {
       if (track->num_channels == 1)
          accessible_name = g_strdup_printf (_("%s Slider"), ctrl->track->label);
       else {
          gchar *accessible_desc = g_strdup_printf (_("Channel %d of %s Slider"), i+1, ctrl->track->label);
          accessible_name = g_strdup_printf (_("%s Slider %d"), ctrl->track->label, i+1);
          atk_object_set_description (accessible, accessible_desc); 
          g_free (accessible_desc);
       }
       atk_object_set_name (accessible, accessible_name);
       g_free (accessible_name);
    }
  }

  ctrl->adjustments = adjlist;

  /* buttons (lock, mute, rec) */
  if (track->num_channels > 1) {
    button = gtk_check_button_new_with_label (_("Lock"));
    gtk_table_attach (GTK_TABLE (table), button,
		      column_pos, column_pos + track->num_channels,
		      3, 4, 5, 0, 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    ctrl->lock = button;
    g_signal_connect (G_OBJECT (button), "toggled",
		      G_CALLBACK (cb_lock_toggled), (gpointer) ctrl);
    accessible = gtk_widget_get_accessible (ctrl->lock);
    if (GTK_IS_ACCESSIBLE (accessible)) {
      accessible_name = g_strdup_printf (_("%s Lock"), ctrl->track->label);
      atk_object_set_name (accessible, accessible_name);
      g_free (accessible_name);
    }
  }

  button = gtk_check_button_new_with_label (_("Mute"));
  gtk_table_attach (GTK_TABLE (table), button,
		    column_pos, column_pos + track->num_channels,
		    4, 5, 5, 0, 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				GST_MIXER_TRACK_HAS_FLAG (track,
						GST_MIXER_TRACK_MUTE));
  g_signal_connect (G_OBJECT (button), "toggled",
		    G_CALLBACK (cb_mute_toggled), (gpointer) ctrl);
  ctrl->mute = button;
  accessible = gtk_widget_get_accessible (ctrl->mute);
  if (GTK_IS_ACCESSIBLE (accessible)) {
    accessible_name = g_strdup_printf (_("%s Mute"), ctrl->track->label);
    atk_object_set_name (accessible, accessible_name);
    g_free (accessible_name);
  }

  if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_INPUT)) {
    button = gtk_check_button_new_with_label (_("Rec."));
    gtk_table_attach (GTK_TABLE (table), button,
		      column_pos, column_pos + track->num_channels,
		      5, 6, 5, 0, 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				  GST_MIXER_TRACK_HAS_FLAG (track,
						GST_MIXER_TRACK_RECORD));
    g_signal_connect (G_OBJECT (button), "toggled",
		      G_CALLBACK (cb_record_toggled), (gpointer) ctrl);
    ctrl->record = button;
    accessible = gtk_widget_get_accessible (ctrl->record);
    if (GTK_IS_ACCESSIBLE (accessible)) {
      accessible_name = g_strdup_printf (_("%s Record"), ctrl->track->label);
      atk_object_set_name (accessible, accessible_name);
      g_free (accessible_name);
    }
  }
  g_free (volumes);
}

#if USE_OPTION_WIDGET
static void
create_options_widget (GstMixer      *mixer,
		       GtkWidget     *table,
		       gint           column_pos,
		       GstMixerTrack *track)
{
  GtkWidget *box;
  GstMixerOptions *options = GST_MIXER_OPTIONS (track);
  MyMixerControls *ctrl;
  const GList *opt;
  AtkObject *accessible;
  gchar *accessible_name;
  gint i = 0;
  const gchar *active_opt;

  /* header */
  ctrl = create_track_header (mixer, table, column_pos, track);

  /* optionmenu */
  active_opt = gst_mixer_get_option (mixer, GST_MIXER_OPTIONS (track));
  box = gtk_combo_box_new_text ();
  for (opt = options->values; opt != NULL; opt = opt->next, i++) {
    gtk_combo_box_append_text (GTK_COMBO_BOX (box), opt->data);
    if (!strcmp (active_opt, opt->data)) {
      gtk_combo_box_set_active (GTK_COMBO_BOX (box), i);
    }
  }
  gtk_table_attach (GTK_TABLE (table), box, column_pos, column_pos + 1, 2, 3,
		    0, 0, 0, 0);
  accessible = gtk_widget_get_accessible (box);
  if (GTK_IS_ACCESSIBLE (accessible)) {
    /* once this code is un-#if 0'ed, please i18n'ize this */
    accessible_name = g_strdup_printf ("%s Option Selection",
				       ctrl->track->label);
    atk_object_set_name (accessible, accessible_name);
    g_free (accessible_name);
  }
  gtk_widget_show (box);
  g_signal_connect (box, "changed", G_CALLBACK (cb_opt_changed), ctrl);
}
#endif

static void
create_switch_widget (GstMixer      *mixer,
		      GtkWidget     *table,
		      gint           column_pos,
		      GstMixerTrack *track)
{
  GtkWidget *button;
  MyMixerControls *ctrl;
  AtkObject *accessible;
  gchar *accessible_name;

  /* header */
  ctrl = create_track_header (mixer, table, column_pos, track);

  /* mute button */
  button = gtk_check_button_new_with_label (_("Mute"));
  gtk_table_attach (GTK_TABLE (table), button,
                    column_pos, column_pos + 1,
                    4, 5, 5, 0, 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                GST_MIXER_TRACK_HAS_FLAG (track,
                                                GST_MIXER_TRACK_MUTE));
  g_signal_connect (G_OBJECT (button), "toggled",
                    G_CALLBACK (cb_mute_toggled), (gpointer) ctrl);
  ctrl->mute = button;
  accessible = gtk_widget_get_accessible (ctrl->mute);
  if (GTK_IS_ACCESSIBLE (accessible)) {
    accessible_name = g_strdup_printf (_("%s Mute"), ctrl->track->label);
    atk_object_set_name (accessible, accessible_name);
    g_free (accessible_name);
  }
}

static GtkWidget *
create_mixer_widget (GstMixer *mixer)
{
  GtkWidget *view;
  GtkWidget *table;
  GtkWidget *viewport;
  GtkAdjustment *hadjustment;
  GtkAdjustment *vadjustment;

  gint tablepos = 0;
  const GList *tracks;

  /* count number of tracks */
  tracks = gst_mixer_list_tracks (mixer);
  for ( ; tracks != NULL; tracks = tracks->next) {
    tablepos += ((GstMixerTrack *) tracks->data)->num_channels;
    if (tracks->next != NULL)
      tablepos++;
  }

  /* create table for all single tracks */
  table = gtk_table_new (6, tablepos, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);

  /* add each */
  tablepos = 0;
  tracks = gst_mixer_list_tracks (mixer);
  for ( ; tracks != NULL; tracks = tracks->next) {
    GstMixerTrack *track = GST_MIXER_TRACK (tracks->data);

    /* hack for bad API (because of API stability...) in GStreamer-0.8.x */
    if (track->num_channels == 0) {
      if (GST_IS_MIXER_OPTIONS (track)) {
#if USE_OPTION_WIDGET
        /* options - menu */
        create_options_widget (mixer, table, tablepos, track);
#else
        /* ignore */
        continue;
#endif
      } else {
        /* switch */
        create_switch_widget (mixer, table, tablepos, track);
      }
    } else {
      create_track_widget (mixer, table, tablepos, track);
    }

    tablepos += track->num_channels ? track->num_channels : 1;
    if (tracks->next != NULL) {
      GtkWidget *sep = gtk_vseparator_new ();
      gtk_table_attach_defaults (GTK_TABLE (table), sep,
				 tablepos, tablepos+1, 0, 6);
      tablepos++;
    }
  }

  /* put table in a scrollview for handling lots of tracks */
  view = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (view),
				  tablepos <= 30 ?
				  GTK_POLICY_NEVER : GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_NEVER);
  if (tablepos > 30)
    gtk_widget_set_size_request (view, 600, -1);

  hadjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW (view));
  vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW (view));

  viewport = gtk_viewport_new(GTK_ADJUSTMENT (hadjustment), GTK_ADJUSTMENT (vadjustment));
  gtk_viewport_set_shadow_type(GTK_VIEWPORT (viewport), GTK_SHADOW_NONE);

  gtk_container_add(GTK_CONTAINER (viewport), table);
  gtk_container_add(GTK_CONTAINER (view), viewport);


  return view;
}

static GList *
create_mixer_collection (GtkWidget *notebook)
{
  GtkWidget *page, *label;
  const GList *elements;
  GList *collection = NULL;
  gint num = 0;

  /* go through all elements of a certain class and check whether
   * they implement a mixer. If so, add a page */
  elements = gst_registry_pool_feature_list (GST_TYPE_ELEMENT_FACTORY);
  for ( ; elements != NULL; elements = elements->next) {
    GstElementFactory *factory = GST_ELEMENT_FACTORY (elements->data);
    gchar *title = NULL, *name;
    const gchar *klass;
    GstElement *element = NULL;
    const GParamSpec *devspec;
    GstPropertyProbe *probe;
    GValueArray *array = NULL;
    gint n;

    /* check category */
    klass = gst_element_factory_get_klass (factory);
    if (strcmp (klass, "Generic/Audio"))
      goto next;

    /* create element */
    title = g_strdup_printf ("gst-mixer-%d", num);
    element = gst_element_factory_create (factory, title);
    if (!element)
      goto next;

    if (!GST_IS_PROPERTY_PROBE (element))
      goto next;

    probe = GST_PROPERTY_PROBE (element);
    if (!(devspec = gst_property_probe_get_property (probe, "device")))
      goto next;
    if (!(array = gst_property_probe_probe_and_get_values (probe, devspec)))
      goto next;

    /* set all devices and test for mixer */
    for (n = 0; n < array->n_values; n++) {
      GValue *device = g_value_array_get_nth (array, n);

      /* set this device */
      g_object_set_property (G_OBJECT (element), "device", device);
      if (gst_element_set_state (element,
				 GST_STATE_READY) == GST_STATE_FAILURE)
        continue;

      /* is this device a mixer? */
      if (!GST_IS_MIXER (element)) {
        gst_element_set_state (element, GST_STATE_NULL);
        continue;
      }

      /* create mixer UI object */
      page = create_mixer_widget (GST_MIXER (element));
      if (g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (element)),
					"device-name")) {
        gchar *devname;
        g_object_get (element, "device-name", &devname, NULL);
        name = g_strdup_printf ("%s [%s]", devname,
				gst_element_factory_get_longname (factory));
      } else {
        name = g_strdup_printf ("%s [%s]", title,
				gst_element_factory_get_longname (factory));
      }
      label = gtk_label_new (name);

      /* add new notebook page + keep track */
      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
      collection = g_list_append (collection, element);
      num++;

      /* and recreate this object, since we give it to the mixer */
      title = g_strdup_printf ("gst-mixer-%d", num);
      element = gst_element_factory_create (factory, title);
    }

next:
    if (element)
      gst_object_unref (GST_OBJECT (element));
    if (array)
      g_value_array_free (array);
    g_free (title);
  }

  return collection;
}

static void
cb_about (GtkWidget *widget,
	  gpointer   data)
{
  GtkWidget *about;
  const gchar *authors[] = { "Ronald Bultje <rbultje@ronald.bitfreak.net>",
			     "Leif Johnson <leif@ambient.2y.net>",
			     NULL };
  const gchar *documentors[] = { "Sun Microsystems",
				 NULL};

  about = gnome_about_new (_("Volume Control"),
			   VERSION,
			   "(c) 2003-2004 Ronald Bultje",
			   _("A GNOME/GStreamer-based mixer application"),
			   authors, documentors, NULL,
			   NULL);

  gtk_widget_show (about);
}

static void
cb_destroy (GtkWidget *widget,
            gpointer   data)
{
  gtk_main_quit();
}

static GnomeUIInfo file_menu[] = {
  GNOMEUIINFO_MENU_EXIT_ITEM (cb_destroy, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[] = {
  GNOMEUIINFO_HELP ("gnome-volume-control"),
  GNOMEUIINFO_MENU_ABOUT_ITEM (cb_about, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo main_menu[] = {
  GNOMEUIINFO_MENU_FILE_TREE (file_menu),
  GNOMEUIINFO_MENU_HELP_TREE (help_menu),
  GNOMEUIINFO_END
};

static void
register_stock_icons (void)
{
  GtkIconFactory *icon_factory;
  struct {
    gchar *filename,
	  *stock_id;
  } list[] = {
    { "mixer.png", GST_MIXER_STOCK_MIXER },
    { "phone.png", GST_MIXER_STOCK_PHONE },
    { "tone.png",  GST_MIXER_STOCK_TONE  },
    { "video.png", GST_MIXER_STOCK_VIDEO },
    { NULL, NULL }
  };
  gint num;
 
  icon_factory = gtk_icon_factory_new ();
  gtk_icon_factory_add_default (icon_factory);

  for (num = 0; list[num].filename != NULL; num++) {
    gchar *filename = gnome_program_locate_file (NULL,
						 GNOME_FILE_DOMAIN_APP_PIXMAP,
						 list[num].filename, TRUE, NULL);
    if (filename) {
      GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
      GtkIconSet *icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
      gtk_icon_factory_add (icon_factory, list[num].stock_id, icon_set);
      g_free (filename);
    }
  }
}

gint
main (gint   argc,
      gchar *argv[])
{
  gchar *file;
  GtkWidget *window, *notebook;
  GList *mixers, *item;
  struct poptOption options[] = {
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, NULL, 0, "GStreamer", NULL},
    POPT_TABLEEND
  };

  /* yay! */
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* init gstreamer/gnome */
  options[0].arg = (void *) gst_init_get_popt_table();
  gnome_program_init ("gnome-volume-control", VERSION, LIBGNOMEUI_MODULE,
		      argc, argv,
		      GNOME_PARAM_POPT_TABLE, options,
		      GNOME_PARAM_APP_DATADIR, DATA_DIR,
		      NULL);
  register_stock_icons ();

  /* create main window + menus */
  window = gnome_app_new ("gnome-volume-control", _("Volume Control"));
  gnome_app_create_menus (GNOME_APP (window), main_menu);
 
  /* Set appicon image */ 
  if ((file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP,
					 PIX_DIR "/mixer.png", TRUE, NULL))) {
    gnome_window_icon_set_default_from_file (file);
    g_free (file);
  }

  /* create all mixers */
  notebook = gtk_notebook_new ();
  if (!(mixers = create_mixer_collection (notebook))) {
    GtkWidget *dialog =
	gtk_message_dialog_new (NULL, 0,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Sorry, no mixer elements and/or devices found"));
    gtk_widget_show (dialog);
    gtk_dialog_run (GTK_DIALOG (dialog));
    return 0;
  }
  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook)) == 1)
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);

  /* add to window */
  gnome_app_set_contents (GNOME_APP (window), notebook);
  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (cb_destroy), NULL);

  /* show off and run */
  gtk_widget_show_all (window);
  gtk_main ();

  /* unref */
  for (item = mixers; item != NULL; item = item->next) {
    GstElement *element = GST_ELEMENT (item->data);

    gst_element_set_state (element, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (element));
  }

  return 0;
}
