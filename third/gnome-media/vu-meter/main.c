/*
 * vu-meter -- A volume units meter for esd
 * Copyright (C) 1998 Gregory McLean
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Cambridge, MA 
 * 02139, USA.
 *
 * Modified April 9 1999, by Dave J. Andruczyk to fix time sync problems
 * now it should be pretty much perfectly in sync with the audio.  Adjust
 * the "lag" variable below to taste if you don't like it...  Utilized
 * code from "extace" to get the desired effects..
 * 
 * Small additions April 14th 1999, by Dave J. Andruczyk to fix the missing 
 * peaks problem that happened with quick transients not showin on the vu-meter.
 * Now it will catch pretty much all of them.
 * 
 * Eye candy!
 */
#include <config.h>
#include <gnome.h>
#include <esd.h>
#include <gtkledbar.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

static int sound = -1;
#define RATE   44100

typedef struct _vumeter {
	double      l_level;
	double      r_level;
	int         esd_stat;
	int         meter_stat;
} vumeter;

/* A fairly good size buffer to keep resource (cpu) down */
#define NSAMP  2048
#define BUFS  8

static short aubuf[BUFS][NSAMP];
static GtkWidget *dial[2];
static GtkWidget *window;
static char *esd_host = NULL;
static int curbuf = 0;
static int lag = 2;
static int locount = 0;
static int plevel_l = 0;
static int plevel_r = 0;

/* function prototypes to make gcc happy: */
static void update (void);
static char *itoa (int i);
static void open_sound (gint record);
static void update_levels (gpointer data);
static gint update_display (gpointer data);
static void handle_read (gpointer data, 
			 int source, 
			 GdkInputCondition condition);

static void 
sig_alarm (int sig)
{
	led_bar_light_percent (dial[0], (0.0));
	led_bar_light_percent (dial[1], (0.0));
}

static char *
itoa (int i)
{
	static char ret[ 30 ];
	sprintf (ret, "%d", i);
	return ret;
}

static int
save_state (GnomeClient *client, 
	    int phase, 
	    GnomeRestartStyle save_style,
	    int shutdown, 
	    GnomeInteractStyle inter_style, 
	    int fast,
	    gpointer client_data)
{
	char *argv[] = { NULL };
	
	argv[0]= (char *) client_data;
	gnome_client_set_clone_command (client, 1, argv);
	gnome_client_set_restart_command (client, 1, argv);
	
	return TRUE;
}

static void 
open_sound (int record)
{
	if (record) {
		sound = esd_record_stream (ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_RECORD,
					   RATE, esd_host, "recording_level_meter");
	} else {
		sound = esd_monitor_stream (ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_PLAY,
					    RATE, esd_host, "volume_meter");
	}
	
	if (sound < 0)
	{ /* TPG: Make a friendly error dialog if we can't open sound */
		GtkWidget *box;
		box = gnome_error_dialog(_("Cannot connect to sound daemon.\nPlease run 'esd' at a command prompt."));
		gnome_dialog_run(GNOME_DIALOG(box));
		exit(1);
	}
}

static void
update_levels(gpointer data)
{
	vumeter  *meter;
	int buf;
	register int i;
	register short val_l, val_r;
	static unsigned short bigl, bigr;

	meter = (vumeter *)data;
	meter->meter_stat = FALSE;

	curbuf++;

	if(curbuf >= BUFS) {
		curbuf = 0;
	}

	buf = ((BUFS*2)+curbuf-lag)%BUFS;

	if((curbuf%2)>0) {
		return; 
	}
    
	bigl = bigr = 0;
	for (i = 0; i < NSAMP/2;i++) {
		val_l = abs (aubuf[curbuf][i]);
		i++;
		val_r = abs (aubuf[curbuf][i]);
		bigl = (val_l > bigl) ? val_l : bigl;
		bigr = (val_r > bigr) ? val_r : bigr;
	}

	bigl /= (NSAMP/8);
	bigr /= (NSAMP/8);
	meter->l_level = bigl / 100.0;
	meter->r_level = bigr / 100.0;
	led_bar_light_percent (dial[0], meter->l_level);
	led_bar_light_percent (dial[1], meter->r_level);
	meter->meter_stat = TRUE;

	/* updates display RIGHT AWAY if a new
	   peak has arrived.  Fixes the "lost
	   peaks" problem that happens with fast
	   transient music. Also reduced the MAIN
	   update rate to lower cpu use. Makes it 
	   work a bit better too.. */

	if(plevel_l != meter->l_level) {
		update_display(meter);
		goto done;
	}

	if(plevel_r != meter->r_level) {
		update_display(meter);
	}
 done:

	plevel_l = meter->l_level;
	plevel_r = meter->r_level;
}

static void 
handle_read (gpointer data, 
	     int source, 
	     GdkInputCondition condition)
{
	static int pos = 0;
	static int to_get = NSAMP*2;
	static int count;

	count = read(source, aubuf[curbuf] + pos, to_get);
	if (count <0) {
		exit(1);		/* no data */
	} else {
		pos += count;
		to_get -= count;
	}

	if (to_get == 0) {
		to_get = NSAMP*2;
		pos = 0;
		update_levels(data);
	}
}

static int
update_display (gpointer data)
{
	vumeter *meter;
	meter = (vumeter *)data;
	
	if(!meter->meter_stat) {
		meter->l_level = (meter->l_level >= 0.0) ? meter->l_level - 1.0 : 0.0;
		meter->r_level = (meter->r_level >= 0.0) ? meter->r_level - 1.0 : 0.0;
		led_bar_light_percent (dial[0], meter->l_level);
		led_bar_light_percent (dial[1], meter->r_level);
	}

	return TRUE;
}

int 
main (int argc, 
      char *argv[])
{
	GnomeClient *client;
	GtkWidget *hbox;
	GtkWidget *frame;
	vumeter *meter;
	int time_id;
	int i;
	gint          session_xpos = -1;
	gint          session_ypos = -1;
	gint          orient = 0;
	gint          record = 0;

	struct poptOption options[] = 
	{
		{ NULL, 'x', POPT_ARG_INT, NULL, 0, 
		  N_("Specify the X position of the meter."), 
		  N_("X-Position") },
		{ NULL, 'y', POPT_ARG_INT, NULL, 0, 
		  N_("Specify the Y position of the meter."), 
		  N_("Y-Position") },
		{ NULL, 's', POPT_ARG_STRING, NULL, 0, 
		  N_("Connect to the esd server on this host."), 
		  N_("ESD Server Host") },
		{ NULL, 'v', POPT_ARG_NONE, NULL, 0, 
		  N_("Open a vertical version of the meter."), NULL },
		{ NULL, 'r', POPT_ARG_NONE, NULL, 0, 
		  N_("Act as recording level meter."), NULL },
		{ NULL, '\0', 0, NULL, 0 }
	};

	options[0].arg = &session_xpos;
	options[1].arg = &session_ypos;
	options[2].arg = &esd_host;
	options[3].arg = &orient;
	options[4].arg = &record;
	
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	gnome_init_with_popt_table ("Volume Meter", VERSION, argc, argv, options, 
				    0, NULL);
#ifdef DEBUG
	if (esd_host) {
		g_print (_("Host is %s\n"), esd_host);
	}
#endif

	meter = g_new0 (vumeter, 1);
	client = gnome_master_client ();
	gtk_object_ref (GTK_OBJECT (client));
	gtk_object_sink (GTK_OBJECT (client));
	g_signal_connect (G_OBJECT (client), "save_yourself",
			  G_CALLBACK (save_state), argv[0]);
	g_signal_connect (G_OBJECT (client), "die",
			  G_CALLBACK (gtk_main_quit), argv[0]);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gnome_window_icon_set_from_file (GTK_WINDOW (window),
					 GNOME_ICONDIR"/gnome-vumeter.png");
	gtk_window_set_title (GTK_WINDOW (window),
			      (record ? _("Recording level") : _("Volume Meter") ));
	if (session_xpos >=0 && session_ypos >= 0) {
		gtk_widget_set_uposition (window, session_xpos, session_ypos);
	}

	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect (G_OBJECT (window), "delete_event",
			  G_CALLBACK (gtk_main_quit), NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_border_width (GTK_CONTAINER (frame), 4);
	gtk_container_add (GTK_CONTAINER (window), frame);
	
	if ( !orient ) {
		hbox = gtk_vbox_new (FALSE, 5);
	} else {
		hbox = gtk_hbox_new (FALSE, 5);
	}
	
	gtk_container_border_width (GTK_CONTAINER (hbox), 5);
	gtk_container_add (GTK_CONTAINER (frame), hbox);
	
	for (i = 0; i < 2; i++) {
		dial[i] = led_bar_new (25, orient);
		gtk_box_pack_start (GTK_BOX (hbox), dial[i], FALSE, FALSE, 0);
	}
	
	gtk_widget_show_all (window);
	open_sound(record);
	fcntl(sound, F_SETFL, O_NONBLOCK);
	
	if(sound > 0) { /* TPG: Make sure we have a valid fd... */
		gdk_input_add (sound, GDK_INPUT_READ, handle_read, meter);
	}
	
	/* time_id = gtk_timeout_add (50000, (GtkFunction)update_display, meter); */
	
	gtk_main ();
	
	return 0;
}
