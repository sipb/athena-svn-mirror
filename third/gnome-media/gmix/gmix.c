/*
 * GMIX 3.0
 *
 * A total rewrite of gmix 2.5...
 *
 * This version includes most features of gnome-hello, except DND and SM. That
 * will follow.
 *
 * Supported sound-drivers:
 * - OSS (only lite checked)
 * - ALSA driver >0.4.1f, lib >0.4.1d
 *
 * TODO:
 * - /dev/audio (if the admins install GTK and GNOME on our Sparcs...)
 * - other sound systems
 * - you name it...
 * - Animated ramping of mixer sliders to preset values
 * - Load Settings button, several for presets ?
 * - more configuration
 * - move the sliders in accordance to the information received from
 *   the ALSA driver when another application adjusts a mixer setting
 *   (yes ALSA does this, without having to poll the mixer!!)
 *
 * Copyright (C) 1998 Jens Ch. Restemeier <jchrr@hrz.uni-bielefeld.de>
 * Config dialog added by Matt Martin <Matt.Martin@ieee.org>, Sept 1999
 * ALSA driver by Brian J. Murrell <gnome-alsa@interlinx.bc.ca> Dec 1999
 *
 * Copyright (C) 2001 - 2002 Iain Holmes <iain@ximian.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#ifdef ALSA
#include <sys/asoundlib.h>
#else
#ifdef HAVE_LINUX_SOUNDCARD_H 
#include <linux/soundcard.h>
#else 
#include <machine/soundcard.h>
#endif
#endif

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include <gconf/gconf-client.h>

#include "gmix.h"
#include "prefs.h"

static void config_cb (GtkWidget *widget, gpointer data);

static device_info *open_device(int num);
static void scan_devices(void);
static void free_one_device(gpointer a, gpointer b);
static void free_devices(void);
static void init_one_device(gpointer a, gpointer b);
static void init_devices(void);
static void get_one_device_config(gpointer a, gpointer b);
static void get_device_config(void);
static void put_one_device_config(gpointer a, gpointer b);
static void put_device_config(void);
static GtkWidget *make_slider_mixer(channel_info *ci);
static void open_dialog(void);

static void scan_devices(void);
static void free_devices(void);
static void init_devices(void);

static void quit_cb (GtkWidget *widget, gpointer data);
static void lock_cb (GtkWidget *widget, channel_info *data);
static void mute_cb (GtkWidget *widget, channel_info *data);
static void rec_cb (GtkWidget *widget, channel_info *data);
static void adj_left_cb (GtkAdjustment *widget, channel_info *data);
static void adj_right_cb (GtkAdjustment *widget, channel_info *data);
static void about_cb (GtkWidget *widget, gpointer data);

static void help_cb(GtkWidget *widget, gpointer data);
static void fill_in_device_guis(GtkWidget *notebook);
static void gmix_build_slidernotebook(void);
static void gmix_hide_last_separator (void);

/*
 * Gnome info:
 */
GtkWidget *app;
GtkWidget  *slidernotebook;

/* Menus */
static GnomeUIInfo help_menu[] = {
  	GNOMEUIINFO_ITEM_STOCK(N_("Help"), NULL, help_cb, GNOME_STOCK_PIXMAP_HELP),
	GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo settings_menu[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM (config_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_MENU_EXIT_ITEM(quit_cb, NULL),
	GNOMEUIINFO_END
};      

static GnomeUIInfo main_menu[] = {
        GNOMEUIINFO_MENU_FILE_TREE (file_menu),
	GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu),
        GNOMEUIINFO_MENU_HELP_TREE (help_menu),
        GNOMEUIINFO_END
};
/* End of menus */ 

GList *devices;
GHashTable *device_by_name = NULL;

int mode=0, mode_norestore=0, mode_initonly=0, mode_nosave=0, num_mixers, mode_restore=0;
#define M_NORESTORE	1
#define M_INITONLY 	2
#define M_NOSAVE	4

static const struct poptOption options[] = {
	{"norestore", 'r', POPT_ARG_NONE, &mode_norestore, 0, N_("don't restore mixer-settings from configuration"), NULL},
	{"restore", 'R', POPT_ARG_NONE, &mode_restore, 0, N_("restore mixer-settings from configuration"), NULL},
	{"initonly", 'i', POPT_ARG_NONE, &mode_initonly, 0, N_("initialise the mixer(s) from stored configuration and exit"), NULL},
	{"nosave", 's', POPT_ARG_NONE, &mode_nosave, 0, N_("don't save (modified) mixer-settings into configuration"), NULL},
	{NULL, '\0', 0, NULL, 0}
};

/*
 * Names for the mixer-channels. device_labels are the initial labels for the
 * sliders, device_names are used in the setup-file.
 *
 * i18n note: These names are defined in the soundcard.h file, but they are
 * only used in the initial configuration.
 * Don't translate the "device_names", because they're used for configuration.
 */
#ifdef ALSA
#ifndef GNOME_STOCK_PIXMAP_BLANK
#define GNOME_STOCK_PIXMAP_BLANK NULL
#endif
struct pixmap device_pixmap[] = {
	"Input Gain", GNOME_STOCK_PIXMAP_BLANK,
	"PC Speaker", GNOME_STOCK_VOLUME,
	"MIC", GNOME_STOCK_PIXMAP_BLANK,
	"Line", GNOME_STOCK_LINE_IN,
	"CD", GTK_STOCK_CDROM,
	"Synth", GNOME_STOCK_PIXMAP_BLANK,
	"PCM", GNOME_STOCK_PIXMAP_BLANK,
	"Output Gain", GNOME_STOCK_PIXMAP_BLANK,
	"Treble", GNOME_STOCK_PIXMAP_BLANK,
	"Bass", GNOME_STOCK_PIXMAP_BLANK,
	"Master", GNOME_STOCK_VOLUME,
	"default", GNOME_STOCK_PIXMAP_BLANK,
};
#else
const char *device_labels[] = SOUND_DEVICE_LABELS;
const char *device_names[]  = SOUND_DEVICE_NAMES;
#ifndef GNOME_STOCK_PIXMAP_BLANK
#define GNOME_STOCK_PIXMAP_BLANK NULL
#endif
const char *device_pixmap[] = {
	GNOME_STOCK_PIXMAP_VOLUME,               /* Master Volume */
	GNOME_STOCK_PIXMAP_BLANK,                /* Bass */
	GNOME_STOCK_PIXMAP_BLANK,                /* Treble */
	GNOME_STOCK_PIXMAP_BLANK,                /* Synth */
	GNOME_STOCK_PIXMAP_BLANK,                /* PCM */
	GNOME_STOCK_PIXMAP_VOLUME,               /* Speaker */
	GNOME_STOCK_PIXMAP_LINE_IN,              /* Line In */
	GNOME_STOCK_PIXMAP_MIC,                  /* Microphone */
	GNOME_STOCK_PIXMAP_CDROM,                /* CD-Rom */
	GNOME_STOCK_PIXMAP_BLANK,                /* Recording monitor ? */
	GNOME_STOCK_PIXMAP_BLANK,                /* ALT PCM */
	GNOME_STOCK_PIXMAP_BLANK,                /* Rec Level? */
	GNOME_STOCK_PIXMAP_BLANK,                /* In Gain */
	GNOME_STOCK_PIXMAP_BLANK,                /* Out Gain */
	GNOME_STOCK_PIXMAP_LINE_IN,              /* Aux 1 */
	GNOME_STOCK_PIXMAP_LINE_IN,              /* Aux 2 */
	GNOME_STOCK_PIXMAP_LINE_IN,              /* Line */
	GNOME_STOCK_PIXMAP_BLANK,                /* Digital 1 ? */
	GNOME_STOCK_PIXMAP_BLANK,                /* Digital 2 ? */
	GNOME_STOCK_PIXMAP_BLANK,                /* Digital 3 ? */
	GNOME_STOCK_PIXMAP_BLANK,                /* Phone in */
	GNOME_STOCK_PIXMAP_BLANK,                /* Phone Out */
	GNOME_STOCK_PIXMAP_BLANK,                /* Video */
	GNOME_STOCK_PIXMAP_BLANK,                /* Radio */
	GNOME_STOCK_PIXMAP_BLANK,                /* Monitor (usually mic) vol */
	GNOME_STOCK_PIXMAP_BLANK,                /* 3d Depth/space param */
	GNOME_STOCK_PIXMAP_BLANK,                /* 3d center param */
	GNOME_STOCK_PIXMAP_BLANK                 /* Midi */
};

#endif

#ifdef ALSA
snd_mixer_callbacks_t read_cbs;

static void rebuild_cb(void *data) {
	fprintf(stderr, "gmix rebuild_cb()\n");
}

static void element_cb(void *data, int cmd, snd_mixer_eid_t *eid) {
	device_info *device=(device_info *)data;
	int err;
	snd_mixer_element_info_t info;
	snd_mixer_element_t element;

	if(cmd != SND_MIXER_READ_ELEMENT_VALUE)
		return;

	if (snd_mixer_element_has_info(eid) != 1) {
/*		fprintf(stderr, "no element info\n"); */
		return;
	}
	memset(&info, 0, sizeof(info));
	info.eid = *eid;
	if ((err = snd_mixer_element_info_build(device->handle, &info)) < 0) {
		fprintf(stderr, "element info build error: %s\n", snd_strerror(err));
		return;
	}
	memset(&element, 0, sizeof(element));
	element.eid = *eid;
	if ((err = snd_mixer_element_build(device->handle, &element)) < 0) {
		fprintf(stderr, "element build error: %s\n", snd_strerror(err));
		return;
	}
	if (element.eid.type == SND_MIXER_ETYPE_VOLUME1) {
#if 0
		/* XXX This code should make the slider actually change to the
		 value in element.data.volume1.pvoices[0] (left) and
		 element.data.volume1.pvoices[1] (right).  This way the slider
		 reflects the value of the item changed in whatever other
		 applications are manipulating the (hardware) mixer */
		int i;
		GList *channels=g_list_first(device->channels);
		for (i=0; i<element.data.volume1.voices_size;
			 i++, channels = g_list_next(channels)) {
			channel_info *ci=(channel_info *)channels->data;

			fprintf(stderr, "%d(%d%%) ", element.data.volume1.pvoices[i],
				element.data.volume1.pvoices[i] * 100 /
				(info.data.volume1.prange[i].max -
				 info.data.volume1.prange[i].min));
			/* This did not seem to work.
			 * I don't know how "sliders" and "adjustments" and all that goo
			 * works in GTK+.
			 */
			gtk_adjustment_set_value(GTK_ADJUSTMENT(ci->left), (gfloat)(-(
				element.data.volume1.pvoices[i] * 100 /
				(info.data.volume1.prange[i].max -
				 info.data.volume1.prange[i].min))));
		}
		fprintf(stderr, "\n");
#endif
	} else /* XXX ignore silently */
		fprintf(stderr, "unsupported element.eid.type=%d for %s\n",
				element.eid.type, element.eid.name);
	snd_mixer_element_free(&element);

}

static void group_cb(void *data, int cmd, snd_mixer_gid_t *gid) {
	fprintf(stderr, "gmix group_cb()\n");
}

#if 0
void read_mixer (gpointer a, gpointer b)
#else
void read_mixer(gpointer a, gint source, GdkInputCondition condition) 
#endif
{
	device_info *info = (device_info *)a;
	int err;

	if ((err=snd_mixer_read(info->handle, &read_cbs))<0) {
		fprintf(stderr, "error reading group: %s\n", snd_strerror(err));
		exit (1);
	}
}

#if 0
gboolean read_mixers ()
{
	g_list_foreach(devices, read_mixer, NULL);
	return TRUE;
}
#endif
#endif

static void error_close_cb(void)
{
	exit(1);
}

void config_cb(GtkWidget *widget, gpointer data)
{
	prefs_make_window (app);
};

int
main(int argc,
     char *argv[]) 
{
	mode = 0;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	gnome_program_init("gnome-volume-control", VERSION,
			   LIBGNOMEUI_MODULE, argc, argv,
			   GNOME_PARAM_POPT_TABLE, options,
			   GNOME_PARAM_POPT_FLAGS, 0,
			   GNOME_PARAM_APP_DATADIR, DATADIR, NULL);		

	if (g_file_exists (GNOME_ICONDIR"/gnome-volume.png"))
		gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-volume.png");
	else
		gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-mixer.png");

	if (mode_nosave) {
		mode |= M_NOSAVE;
	}
	
	if (mode_initonly) {
		mode |= M_INITONLY;
	}

	scan_devices();
	if (devices) {
		if (~mode & M_INITONLY) {
		        get_gui_config();
			/* Beware boolean bastardization */
			if (!prefs.set_mixer_on_start)
				mode |= M_NORESTORE;
			  
		}
		/* Command line always overrides */
		if(mode_norestore) mode |= M_NORESTORE;
		if(mode_restore) mode &= ~M_NORESTORE;

/*  		if (~mode & M_NORESTORE) { */
		get_device_config();
		init_devices();
/*  		}  */

		if (~mode & M_INITONLY) {
			open_dialog();
		}

/*  		if (~mode & M_NOSAVE) { */
		put_device_config();
/*  		} */

		free_devices();
	} else {
		GtkWidget *box;

		box = gnome_error_dialog(_("I was not able to open your audio device.\n"
					   "Please check that you have permission to open /dev/mixer\n"
					   "and make sure you have sound support compiled into your kernel."));

		gtk_signal_connect(GTK_OBJECT(box), "close",
				   GTK_SIGNAL_FUNC(error_close_cb), NULL);
		gtk_main();
	}
	return 0;
}

/* Utility functions to make OSS related crack */
static int
recsrc_from_channels (device_info *device)
{
	GList *c;
	int recsrc = 0;
	
	for (c = device->channels; c; c = c->next) {
		channel_info *channel = c->data;

		if (channel->is_record_source == TRUE) {
			recsrc |= (1 << channel->channel);
		}
	}

	return recsrc;
}

static void
set_channels_from_recsrc (device_info *device,
			  int recsrc)
{
	GList *c;

	for (c = device->channels; c; c = c->next) {
		channel_info *channel = c->data;

		channel->is_record_source = (recsrc & (1 << channel->channel));
	}
}

/*
 * Open the device and query configuration
 */
device_info *
open_device (int num)
{
	device_info *new_device;
#ifdef ALSA
	int device = 0, err; 
	snd_mixer_info_t info; 
	snd_mixer_groups_t groups;
	snd_mixer_gid_t *group;
	int cnt, chn;
	char *card_name;

	/*
	 * create new device configureation
	 */
	new_device = g_new0 (device_info, 1);
	/*
	 * open the mixer-device
	 */
	if ((err = snd_mixer_open (&new_device->handle, num, device)) < 0) { 
		g_free (new_device);
		return NULL;
	} 
	if ((err = snd_mixer_info (new_device->handle, &info)) < 0) { 
		fprintf (stderr, "info failed: %s\n", snd_strerror (err));  
		snd_mixer_close (new_device->handle); 
		g_free (new_device);
		return NULL; 
	} 
	/*
	 * mixer-name
	 */
	if (snd_card_get_name(num, &card_name) == 0) {
		strcpy (new_device->info.name, card_name);
	} else {
		strcpy (new_device->info.name, info.name);
	}

	/* This is the name used thoughout all the config stuff
	   Numerical part will probably change if you start adding more cards,
	   screwing things up, but there's probably no way we can stop that. */
	new_device->card_name = g_strdup_printf ("alsa-%s-%d", new_device->info.name, num + 1);

	if (!g_ascii_isalpha (new_device->info.name[0])) {
		g_snprintf (new_device->info.name, 31, "Card %d", num+1);
	}
	
	/* and id */
	strcpy (new_device->info.id, info.id);
	
	/* 
	 * several bitmasks describing the mixer
	 */
	memset (&groups, 0, sizeof (groups));
	if ((err= snd_mixer_groups (new_device->handle, &groups)) < 0) {
		g_free (new_device->card_name);
		g_free (new_device);
		return NULL;
	}

	groups.pgroups = g_new (snd_mixer_gid_t, 1);
	
	groups.groups_size = groups.groups_over;
	groups.groups_over = groups.groups = 0;
	if ((err = snd_mixer_groups (new_device->handle, &groups)) < 0) {
		g_free (new_device);
		g_free (groups.pgroups);
		return NULL;
	}
	
	for (cnt = 0; cnt < groups.groups; cnt++) {
		channel_info *new_channel;
		snd_mixer_group_t *g;
		int j;
		
		group = &groups.pgroups[cnt];

		g = g_new0 (snd_mixer_group_t, 1);

		new_channel = g_new (channel_info, 1);
		new_channel->mixer_group = g;
		new_channel->device = new_device;
		new_channel->channel = cnt;

		/* Find the pixmap for this group */
		for (j = 0; strcmp (device_pixmap[j].name, "default") != 0 &&
			     strcmp (device_pixmap[j].name, g->gid.name) != 0; j++)
			;

		new_channel->pixmap = g_strdup (device_pixmap[j].pixmap);

		/* Chomp it to remove any yucky spaces */
		new_channel->title = g_strchomp (groups.pgroups[cnt].name);
		new_channel->user_title = g_strdup (new_channel->title);
		new_channel->passive = 0;
		
		g->gid = *group;
		err = snd_mixer_group_read (new_device->handle, &g);
		
		new_channel->is_muted = g->mute;
		new_channel->is_record_source = g->capture;
		new_channel->record_source = g->caps & SND_MIXER_GRPCAP_CAPTURE;
		new_channel->is_stereo = g->channels == SND_MIXER_CHN_MASK_STEREO;
		new_channel->is_locked = new_channel->is_stereo;
		new_channel->visible = TRUE; /* All visible */
		
		/*
		 * get current volume
		 */
		for (chn = 0; chn <= SND_MIXER_CHN_LAST; chn++) {
			if (!(g->channels & (1 << chn))) {
				continue;
			}
			
			if ((1 << chn) & SND_MIXER_CHN_MASK_FRONT_LEFT) {
				new_channel->volume_left = (g->volume.values[chn] * 100 / (g->max - g->min));
			} else if ((1 << chn) & SND_MIXER_CHN_MASK_FRONT_RIGHT) {
				new_channel->volume_right = (g->volume.values[chn] * 100 / (g->max - g->min));
			} else {
				fprintf(stderr, "Unhandled channel on soundcard: %s\n",
					snd_mixer_channel_name(chn));
			}
		}

		new_device->channels = g_list_append (new_device->channels, new_channel);
	}
	
	g_free (groups.pgroups);
	
	memset (&read_cbs, 0, sizeof(read_cbs));
	read_cbs.private_data = new_device;
	read_cbs.rebuild = rebuild_cb;
	read_cbs.element = element_cb;
	read_cbs.group = group_cb;
	gdk_input_add (snd_mixer_file_descriptor(new_device->handle), GDK_INPUT_READ,
		       read_mixer, new_device);
#else
	char device_name[255];
	int res, ver, cnt;
	/* Masks for the channel data - OSS blows compared to ALSA */
	int recmask, recsrc, stereodee;

	/*
	 * This list is borrowed from GCONF_SOURCE/gconf/gconf.c
	 */
	gchar gconf_key_invalid_chars[] = " \t\r\n\"$&<>,+=#!()'|{}[]?~`;%\\";

	
	/*
	 * create new device configureation
	 */
	new_device = g_new0 (device_info, 1);
	/*
	 * open the mixer-device
	 */
	if (num == 0) {
		sprintf (device_name, "/dev/mixer");
	} else {
		sprintf (device_name, "/dev/mixer%i", num);
	}

	new_device->fd = open (device_name, O_RDWR, 0);
	
	if (new_device->fd < 0) {
		g_free (new_device);
		return NULL;
	}
	
	/*
	 * check driver-version
	 */
#ifdef OSS_GETVERSION
	res = ioctl(new_device->fd, OSS_GETVERSION, &ver);
	if ((res == EINVAL) || (ver != SOUND_VERSION)) {
		if (res == EINVAL) {
			fprintf (stderr, 
				 _("Warning: This version of the Gnome Volume Control was compiled with\n"
				   "OSS version %d.%d.%d, and your system is running\n"
				   "a version prior to 3.6.0.\n"), 
				 SOUND_VERSION >> 16, 
				 (SOUND_VERSION >> 8) & 0xff, 
				 SOUND_VERSION & 0xff);
		} else {
			fprintf (stderr, 
				 _("Warning: This version of the Gnome Volume Control was compiled with\n"
				   "OSS version %d.%d.%d, and your system is running\n"
				   "version %d.%d.%d.\n"), 
				 SOUND_VERSION >> 16, 
				 (SOUND_VERSION >> 8) & 0xff, 
				 SOUND_VERSION & 0xff,
				 ver >> 16, (ver >> 8) & 0xff, ver & 0xff);
		}			
	}
#endif
	/*
	 * mixer-name
	 */
#ifndef __FreeBSD__
	res = ioctl (new_device->fd, SOUND_MIXER_INFO, &new_device->info);
	if (res != 0) {
		g_free (new_device);
		return NULL;
	}
	
	g_strdelimit (new_device->info.name, gconf_key_invalid_chars, '_');
	new_device->card_name = g_strdup_printf ("OSS-%s-%d", new_device->info.name, num + 1);
	
	if (!g_ascii_isalpha (new_device->info.name[0])) {
		g_snprintf (new_device->info.name, 31, "Card %d", num+1);
	} 
#else
	new_device->card_name = g_strdup_printf ("OSS-%d-%d", num + 1, num + 1);
	g_snprintf (new_device->info.name, 31, "Card %d", num+1);
#endif
	/* 
	 * several bitmasks describing the mixer
	 */
	res = ioctl (new_device->fd, SOUND_MIXER_READ_DEVMASK, &new_device->devmask);
        res |= ioctl (new_device->fd, SOUND_MIXER_READ_RECMASK, &recmask);
        res |= ioctl (new_device->fd, SOUND_MIXER_READ_RECSRC, &recsrc);
        res |= ioctl (new_device->fd, SOUND_MIXER_READ_STEREODEVS, &stereodee);
        res |= ioctl (new_device->fd, SOUND_MIXER_READ_CAPS, &new_device->caps);
	if (res != 0) {
		g_free (new_device->card_name);
		g_free (new_device);
		return NULL;
	}

	/* Make the channels */
	for (cnt = 0; cnt < SOUND_MIXER_NRDEVICES; cnt++) {

		if (new_device->devmask & (1 << cnt)) {
			channel_info *new_channel;
			unsigned long vol; /* l: vol&0xff, r:(vol&0xff00)>>8 */

			new_channel = g_new0 (channel_info, 1);
			new_channel->device = new_device;
			new_channel->channel = cnt;
			new_channel->pixmap = g_strdup (device_pixmap[cnt]);
			new_channel->title = g_strdup (_(device_names[cnt]));
			new_channel->user_title = g_strdup (new_channel->title);
			new_channel->passive = 0;

			new_channel->is_muted = FALSE;
			new_channel->record_source = recmask & (1 << cnt) ? TRUE : FALSE;
			new_channel->is_record_source = recsrc & (1 << cnt) ? TRUE : FALSE;
			new_channel->is_stereo = stereodee & (1 << cnt) ? TRUE : FALSE;
			new_channel->is_locked = new_channel->is_stereo;
			new_channel->visible = TRUE;
			
			res = ioctl(new_device->fd, MIXER_READ (cnt), &vol);

			new_channel->volume_left = vol & 0xff;
			if (new_channel->is_stereo == TRUE) {
				new_channel->volume_right = (vol & 0xff00) >> 8;
			} else {
				new_channel->volume_right = vol & 0xff;
			}

			new_device->channels = g_list_append (new_device->channels,
							      new_channel);
		}
	}
#endif
	return new_device;
}

static void
scan_devices (void)
{
	int cnt;
	device_info *new_device;
	
	cnt = 0;
	devices = NULL;
	device_by_name = g_hash_table_new (g_str_hash, g_str_equal);
		
	do {
		new_device = open_device (cnt++);
		if (new_device) {
			devices = g_list_append (devices, new_device);
			g_hash_table_insert (device_by_name, new_device->info.name, new_device);
		}
	} while (new_device);
	
	num_mixers = cnt - 1;
}

device_info *
device_from_name (const char *name)
{
	return g_hash_table_lookup (device_by_name, name);
}

static void
free_one_channel (gpointer a, gpointer b)
{
	channel_info *chan = (channel_info *) a;
#ifdef ALSA
	g_free (chan->mixer_group);
#endif
	g_free (chan);
}

static void
free_one_device (gpointer a, gpointer b)
{
	device_info *info = (device_info *)a;
#ifdef ALSA
	snd_mixer_close (info->handle);
#else
	close (info->fd);
#endif

	g_list_foreach (info->channels, free_one_channel, NULL);
	g_list_free (info->channels);
}

static void
free_devices (void)
{
	g_list_foreach(devices, free_one_device, NULL);
	g_list_free(devices);
}

void
init_one_device (gpointer a,
		 gpointer b)
{
	device_info *info = (device_info *)a;
	GList *p;
	unsigned long vol;
	
#ifdef ALSA
	for (p = info->channels; p; p = p->next) {
		channel_info *channel = (channel_info *)p->data;
		snd_mixer_group_t *g = channel->mixer_group;
		int err;
		
		if (channel->is_muted) {
			g->mute = SND_MIXER_CHN_MASK_FRONT_LEFT +
				SND_MIXER_CHN_MASK_FRONT_RIGHT;
		} else {
			g->mute = 0;
		}
		
		while ((err = snd_mixer_group_write (info->handle, g)) < 0 && err == -EBUSY) {
			if ((err = snd_mixer_read (info->handle, &read_cbs)) < 0) {
				fprintf(stderr, "error reading group: %s\n",
					snd_strerror(err));
				exit (1);
			}
		}
		
		if (err < 0) {
			fprintf(stderr, "error writing group: %s\n", snd_strerror(err));
			exit (1);
		}
	}
#else
	int recsrc;

	recsrc = recsrc_from_channels (info);
	ioctl(info->fd, SOUND_MIXER_WRITE_RECSRC, &recsrc);
	
	for (p = info->channels; p; p = p->next) {
		channel_info *channel = p->data;
		
		if (channel->is_muted) {
			vol = 0;
		} else {
			vol = channel->volume_left;
			vol |= channel->volume_right << 8;
		}

#ifdef WE_WANT_VOLUME_RESET
		ioctl (info->fd, MIXER_WRITE (channel->channel), &vol);
#endif
	}
	
#endif
}
	
static void
init_devices(void)
{
	g_list_foreach (devices, init_one_device, NULL);
}
 
static gboolean
get_bool_with_default (const char *key,
		       gboolean default_result)
{
	GConfClient *client;
	GConfValue *value = NULL;
	
	client = gconf_client_get_default ();
	value = gconf_client_get (client, key, NULL);
	g_object_unref (G_OBJECT (client));
	
	if (value != NULL) {
		gboolean result;
		
		result = gconf_value_get_bool (value);
		gconf_value_free (value);
		return result;
	} else {
		return default_result;
	}
}

static int
get_int_with_default (const char *key,
		      int default_result)
{
	GConfClient *client;
	GConfValue *value = NULL;

	client = gconf_client_get_default ();
	value = gconf_client_get (client, key, NULL);
	g_object_unref (G_OBJECT (client));

	if (value != NULL) {
		int result;

		result = gconf_value_get_int (value);
		gconf_value_free (value);
		
		return result;
	} else {
		return default_result;
	}
}

static char *
get_string_with_default (const char *key,
			 const char *default_result)
{
	GConfClient *client;
	GConfValue *value = NULL;
	
	client = gconf_client_get_default ();
	value = gconf_client_get (client, key, NULL);
	g_object_unref (G_OBJECT (client));

	if (value != NULL) {
		char *result;

		result = g_strdup (gconf_value_get_string (value));
		gconf_value_free (value);
		return result;
	} else {
		return g_strdup (default_result);
	}
}

static void
get_one_device_config (gpointer a,
		       gpointer b)
{
	device_info *info = (device_info *)a;
	char *key, *key_base;
	GList *p;

	for (p = info->channels; p; p = p->next) {
		GConfClient *client;
		channel_info *channel = (channel_info *) p->data;

		key_base = g_strdup_printf ("/apps/gnome-volume-control/%s/%s",
					    info->card_name, channel->title);

		/* Check if the key base directory exists */
		client = gconf_client_get_default ();
		if (gconf_client_dir_exists (client, key_base, NULL) == FALSE) {
			g_free (key_base);
			g_object_unref (G_OBJECT (client));
			continue;
		}
		
		g_object_unref (G_OBJECT (client));
		
		key = g_strdup_printf ("%s/title", key_base);

		if (channel->user_title != NULL) {
			g_free (channel->user_title);
		}
		
		channel->user_title = get_string_with_default (key, channel->title);
		g_free (key);
		
		key = g_strdup_printf ("%s/mute", key_base);
		channel->is_muted = get_bool_with_default (key, channel->is_muted);
		g_free (key);

		key = g_strdup_printf ("%s/visible", key_base);
		channel->visible = get_bool_with_default (key, TRUE);
		g_free (key);

#ifdef WE_WANT_VOLUME_RESET		
		if (channel->is_stereo) {
			key = g_strdup_printf ("%s/lock", key_base);
			channel->is_locked = get_bool_with_default (key, channel->is_locked);
			g_free (key);
			
			key = g_strdup_printf ("%s/volume_left", key_base);
			channel->volume_left = get_int_with_default (key,
								     channel->volume_left);
			g_free (key);
			
			key = g_strdup_printf ("%s/volume_right", key_base);
			channel->volume_right = get_int_with_default (key,
								      channel->volume_right);
			g_free (key);
		} else {
			key = g_strdup_printf ("%s/volume", key_base);
			channel->volume_left = get_int_with_default (key,
								     channel->volume_left);
			g_free (key);
		}
#endif		
		if (channel->record_source == TRUE) {
			key = g_strdup_printf ("%s/recsrc", key_base);
			channel->is_record_source = get_bool_with_default (key,
									   channel->is_record_source);
			g_free (key);
		}

		
		g_free (key_base);
	}
}

static void
get_device_config (void)
{
	g_list_foreach (devices, get_one_device_config, NULL);
}

static void
put_one_device_config (gpointer a,
		       gpointer b)
{
	GConfClient *client;
	GError *err = NULL;
	device_info *info = (device_info *)a;
	GList *p;
	char *key, *key_base;

	client = gconf_client_get_default ();
	
	for (p = info->channels; p; p = p->next) {
		channel_info *channel = (channel_info *)p->data;

		key_base = g_strdup_printf ("/apps/gnome-volume-control/%s/%s",
					    info->card_name, channel->title);

		key = g_strdup_printf ("%s/title", key_base);
		gconf_client_set_string (client, key, channel->user_title, &err);
		if (err != NULL) {
			g_warning ("Error says %s\n", err->message);
			g_error_free (err);
		}
		g_free (key);
		
		key = g_strdup_printf("%s/mute", key_base);
		gconf_client_set_bool (client, key, channel->is_muted, NULL);
		
		if (channel->is_stereo == TRUE) {
			key = g_strdup_printf ("%s/lock", key_base);
			gconf_client_set_bool(client, key, channel->is_locked, NULL);
			g_free (key);
			
			key = g_strdup_printf ("%s/volume_left", key_base);
			gconf_client_set_int (client, key, channel->volume_left, NULL);
			g_free (key);

			
			key = g_strdup_printf ("%s/volume_right", key_base);
			gconf_client_set_int(client, key, channel->volume_right, NULL);
			g_free (key);
		} else {
			key = g_strdup_printf ("%s/volume", key_base);
			gconf_client_set_int (client, key, channel->volume_left, NULL);
			g_free (key);
		}

		if (channel->record_source == TRUE) {
			key = g_strdup_printf ("%s/recsrc", key_base);
			gconf_client_set_bool (client, key, channel->is_record_source, NULL);
			g_free (key);
		}

		key = g_strdup_printf ("%s/visible", key_base);
		gconf_client_set_bool (client, key, channel->visible, NULL);
		g_free (key);

		g_free (key_base);
	}

	g_object_unref (G_OBJECT (client));
}

static void
put_device_config (void)
{
	g_list_foreach(devices, put_one_device_config, NULL);
}

/*
 * dialogs:
 */
GtkWidget *make_slider_mixer(channel_info *ci)
{
	AtkObject *accessible;
	gchar *accessible_name;
	GtkWidget *hbox, *scale;
	device_info *di;
	
	di = ci->device;

	hbox = gtk_hbox_new (FALSE, 1);

#ifdef ALSA
	ci->left = gtk_adjustment_new (-ci->volume_left, -100.0, 0.0, 5.0, 2.0, 0.0);
#else
	ci->left = gtk_adjustment_new (-ci->volume_left, -101.0, 0.0, 5.0, 2.0, 0.0);
#endif
	
	g_signal_connect (G_OBJECT (ci->left), "value_changed",
			  G_CALLBACK (adj_left_cb), ci);
	
	scale = gtk_vscale_new (GTK_ADJUSTMENT(ci->left));
	gtk_range_set_update_policy (GTK_RANGE(scale), GTK_UPDATE_CONTINUOUS);
	gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
	
	gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
	gtk_widget_show (scale);
	
	accessible_name = g_strdup_printf (ci->is_stereo ? _("left %s") : "%s",
					   ci->user_title);
	
	accessible = gtk_widget_get_accessible (scale);
	atk_object_set_name (accessible, accessible_name);
	g_free (accessible_name);
	
	if (ci->is_stereo == TRUE) {
		/* Right channel, display only if we have stereo */
#ifdef ALSA
		ci->right = gtk_adjustment_new (-ci->volume_right,
						-100.0, 0.0, 1.0, 1.0, 0.0);
#else
		ci->right = gtk_adjustment_new (-ci->volume_right,
						-101.0, 0.0, 1.0, 1.0, 0.0);
#endif
		
		g_signal_connect (G_OBJECT (ci->right), "value_changed",
				  G_CALLBACK (adj_right_cb), ci);
		
		scale = gtk_vscale_new (GTK_ADJUSTMENT (ci->right));
		gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
		gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
		gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
		gtk_widget_show (scale);
		
		accessible_name = g_strdup_printf (_("Right %s"), ci->user_title);
		accessible = gtk_widget_get_accessible(scale);
		atk_object_set_name (accessible, accessible_name);
		g_free (accessible_name);
	}

	return hbox;
}

static void
fill_in_device_guis (GtkWidget *notebook)
{
	GList *d, *c;
	GtkWidget *table, *spixmap;
	int j;
	
	if (num_mixers == 1) {
		gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
	}
	
	for (d = devices; d; d = d->next) {
		device_info *di;
		GList *table_focus_chain = NULL;
		int i;
		
		di = d->data;
		j = 0;

		/* Count the number of columes the table needs */
		for (c = di->channels; c; c = c->next) {
			j += 2;
		}

		table = gtk_table_new (j * 2, 5, FALSE);
		gtk_notebook_append_page (GTK_NOTEBOOK(notebook),
					  table, 
					  gtk_label_new(di->info.name));
		gtk_table_set_row_spacings (GTK_TABLE (table), 0);
		gtk_table_set_col_spacings (GTK_TABLE (table), 0);
		gtk_container_border_width (GTK_CONTAINER (table), 0);

		for (c = di->channels, i = 0; c; c = c->next, i += 2) {
			GtkWidget *label, *mixer, *separator;
			GtkWidget *hbox;
			channel_info *ci;
			char *s;

			ci = c->data;

			hbox = gtk_hbox_new (FALSE, 3);
			gtk_widget_show (hbox);
			
			gtk_table_attach (GTK_TABLE (table), hbox, i, i+1,
					  0, 1, 0,
					  GTK_FILL, 0, 3);
			
			if (ci->pixmap) {
				spixmap = gtk_image_new_from_stock (ci->pixmap,
								    GTK_ICON_SIZE_BUTTON);
				gtk_box_pack_start (GTK_BOX (hbox), spixmap, FALSE, FALSE, 0);
				gtk_widget_show (spixmap);
				ci->icon = spixmap;
			} else {
				ci->icon = NULL;
			}
			
			s = g_strdup_printf ("<span size=\"larger\">%s</span>",
					     ci->user_title);
			label = gtk_label_new (s);
			gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
			g_free (s);
			gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
			gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
			gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
			
			gtk_widget_show (label);
			ci->label = label;

			ci->mixer = make_slider_mixer(ci);
			gtk_table_attach (GTK_TABLE (table), ci->mixer,
					  i, i+1,
					  1, 2, GTK_EXPAND | GTK_FILL,
					  GTK_EXPAND | GTK_FILL, 0, 0);
  			gtk_widget_set_size_request (ci->mixer, -1, 100); 
			gtk_widget_show (ci->mixer);
			table_focus_chain = g_list_prepend (table_focus_chain,
							    ci->mixer);
			
			if (ci->is_stereo == TRUE) {
				/* lock-button, only useful for stereo */
				AtkObject *accessible;
				gchar *accessible_name;

				ci->lock = gtk_check_button_new_with_label (_("Lock"));
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ci->lock),
							      ci->is_locked);
				
				g_signal_connect (G_OBJECT (ci->lock), "toggled",
						  G_CALLBACK (lock_cb), ci);
				
				gtk_table_attach (GTK_TABLE (table), ci->lock,
						  i, i+1, 2, 3,
						  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
				gtk_widget_show (ci->lock);
				
				table_focus_chain = g_list_prepend (table_focus_chain, ci->lock);
				accessible_name = g_strdup_printf (_("%s Lock"),
								   ci->user_title);
				accessible = gtk_widget_get_accessible (ci->lock);
				atk_object_set_name (accessible, accessible_name);
				g_free (accessible_name);
			}

			{
				AtkObject *accessible;
				gchar *accessible_name;
				
				ci->mute = gtk_check_button_new_with_label (_("Mute"));
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ci->mute),
							      ci->is_muted);
				
				g_signal_connect (G_OBJECT (ci->mute), "toggled",
						  G_CALLBACK (mute_cb), ci);
				
				gtk_table_attach (GTK_TABLE (table), ci->mute,
						  i, i+1, 4, 5,
						  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
				gtk_widget_show (ci->mute);
				
				table_focus_chain = g_list_prepend (table_focus_chain, ci->mute);
				accessible_name = g_strdup_printf (_("%s Mute"),
								   ci->user_title);
				accessible = gtk_widget_get_accessible (ci->mute);
				atk_object_set_name (accessible, accessible_name);
				g_free (accessible_name);
			}

			/*
			 * recording sources
			 */
			if (ci->record_source == TRUE) {
				AtkObject *accessible;
				gchar *accessible_name;
				
				ci->rec = gtk_check_button_new_with_label (_("Rec."));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ci->rec),
							     ci->is_record_source);
				g_signal_connect (G_OBJECT (ci->rec), "toggled",
						  G_CALLBACK (rec_cb), ci);
				
				gtk_table_attach (GTK_TABLE (table), ci->rec,
						  i, i+1, 5, 6,
						  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
				gtk_widget_show (ci->rec);
				
				table_focus_chain = g_list_prepend (table_focus_chain, ci->rec);
				accessible_name = g_strdup_printf (_("%s Record"),
								   ci->user_title);
				accessible = gtk_widget_get_accessible (ci->rec);
				atk_object_set_name (accessible, accessible_name);
				g_free (accessible_name);
			} else { /* 
				  * David: need to init it to null
				  * otherwise we get a segfault when
				  * trying to toggle
				  * the buttons 
				  */
				ci->rec = NULL;
			}
	
			ci->separator = gtk_vseparator_new ();
			gtk_table_attach (GTK_TABLE (table), ci->separator,
					  i+1, i+2, 0, 6,
					  GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 0);
			gtk_widget_show (ci->separator);

			if (ci->visible == FALSE) {
				/* Hide everything */
				gtk_widget_hide (ci->separator);
				gtk_widget_hide (ci->mixer);
				if (ci->lock != NULL) {
					gtk_widget_hide (ci->lock);
				}
				
				if (ci->rec != NULL) {
					gtk_widget_hide (ci->rec);
				}
				gtk_widget_hide (ci->mute);
				if (ci->icon != NULL) {
					gtk_widget_hide (ci->icon);
				}
				gtk_widget_hide (ci->label);
			}
		}
		
		gtk_widget_show (table);
		/* Set the new focus chain for the table */

		table_focus_chain = g_list_reverse (table_focus_chain);
		gtk_container_set_focus_chain (GTK_CONTAINER(table), table_focus_chain);
		g_list_free (table_focus_chain);
	}

	gmix_hide_last_separator ();
}

void
gmix_build_slidernotebook (void)
{
        /* This is a sloppy way to re-draw the mixers */
        GList *d;

	if (slidernotebook) {
		gtk_widget_hide(slidernotebook);
		/* Assumes that the number of devices is static... */
		for (d = devices; d; d = d->next) 
			gtk_notebook_remove_page(GTK_NOTEBOOK(slidernotebook),0);
	} else {
		slidernotebook = gtk_notebook_new();
	}
	
	gtk_widget_show (slidernotebook);
	fill_in_device_guis (slidernotebook);
}

static void
open_dialog (void)
{
	app = gnome_app_new ("gnome-volume-control", _("Volume Control") );
	gtk_widget_realize (app);
	gtk_window_set_resizable (GTK_WINDOW (app), FALSE);
	g_signal_connect (G_OBJECT (app), "delete_event",
			  G_CALLBACK (quit_cb), NULL);


/*  	gmix_restore_window_size (); */

	/*
	 * Build main menu
	 */
	gnome_app_create_menus (GNOME_APP (app), main_menu);

	/*
	 * Build table with sliders
	 */	
	gmix_build_slidernotebook ();
	gnome_app_set_contents (GNOME_APP (app), slidernotebook);

	gtk_widget_show (app);

	/*
	 * Go into gtk event-loop
	 */
	gtk_main ();
}

#if 0
static void gmix_restore_window_size (void)
{
	gint width = 0, height = 0;

	width = gnome_config_get_int ("/gmix/gui/width=0");
	height = gnome_config_get_int ("/gmix/gui/height=0");

	if (width!=0 && height!=0)
		gtk_window_set_default_size (GTK_WINDOW (app), width, height);
}

static void gmix_save_window_size (void)
{
	gint width, height;

	gdk_window_get_size (app->window, &width, &height);

	gnome_config_set_int ("/gmix/gui/width", width);
	gnome_config_set_int ("/gmix/gui/height", height);
}
#endif

/*
 * GTK Callbacks:
 */
static void
quit_cb (GtkWidget *widget,
	 gpointer data)
{
/*  	gmix_save_window_size (); */
	gtk_main_quit();
}

static void
lock_cb (GtkWidget *widget,
	 channel_info *data)
{
	if (data == NULL)
		return;

	data->is_locked = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->lock));
	
	if (data->is_locked == TRUE) {
		gdouble value;

		value = (GTK_ADJUSTMENT (data->right)->value +
			 GTK_ADJUSTMENT (data->left)->value) / 2.0;
		gtk_adjustment_set_value (GTK_ADJUSTMENT (data->left), value);
		gtk_adjustment_set_value (GTK_ADJUSTMENT (data->right), value);
	}
}

static void
mute_cb (GtkWidget *widget,
	 channel_info *data)
{
#ifdef ALSA
	int err;
	snd_mixer_group_t *g=data->mixer_group;
#else
	unsigned long vol;
#endif
	
	if (data == NULL) {
		/* FIXME: Why might this happen? */
		return;
	}
	
	data->is_muted = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->mute));
	
#ifdef ALSA
	g->mute = data->is_muted ?
		SND_MIXER_CHN_MASK_FRONT_LEFT + SND_MIXER_CHN_MASK_FRONT_RIGHT : 0;
		
	while ((err = snd_mixer_group_write (data->device->handle, g)) < 0 &&
	       err == -EBUSY) {
		if ((err = snd_mixer_read(data->device->handle, &read_cbs)) < 0) {
			g_warning ("error reading group: %s\n", snd_strerror (err));
			exit (1);
		}
	}
	
	if (err < 0) {
		g_warning ("error writing group: %s\n", snd_strerror(err));
		exit (1);
	}
#else
	if (data->is_muted) {
		vol = 0;
	} else {
		vol = data->volume_left;
		vol |= data->volume_right << 8;
	}
	
	ioctl (data->device->fd, MIXER_WRITE(data->channel), &vol);
#endif
}

static void
turn_off_record_except (device_info *device,
		     channel_info *channel)
{
	GList *c;
	
	for (c = device->channels; c; c = c->next) {
		channel_info *info;
		
		info = c->data;
		
		if (info == channel) {
			continue;
		}
		
		/* check to see if channel can record */
		info->is_record_source = FALSE;
	}
}

static void
rec_cb (GtkWidget *widget,
	channel_info *data)
{
	GList *c;
#ifdef ALSA
	snd_mixer_group_t *g=data->mixer_group;
	int err;
#else
	int recsrc;
#endif
	
	if (data == NULL) {
		/* FIXME: Why might this happen? */
		return;
	}
	
	if (data->passive) {
		return;
	}

	data->is_record_source = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->rec));
	if (GTK_TOGGLE_BUTTON (data->rec)->active) {
#ifdef ALSA
		if (g->caps & SND_MIXER_GRPCAP_EXCL_CAPTURE) {
			turn_off_record_except (data->device, data);
		}
#else
		if (data->device->caps & SOUND_CAP_EXCL_INPUT) {
			/* XXX not sure if this is how this works because I don't have a
			 *     mixer that has "exclusive capture" devices
			 */
			turn_off_record_except (data->device, data);
		}
#endif
	}
	
#ifdef ALSA
	if (data->is_record_source == TRUE) {
		g->capture = g->channels;
	} else {
		g->capture = 0;
	}
	
	while ((err = snd_mixer_group_write (data->device->handle, g)) < 0 &&
	       err == -EBUSY) {
		if ((err = snd_mixer_read (data->device->handle, &read_cbs)) < 0) {
			g_warning ("error reading group: %s\n", snd_strerror (err));
			exit (1);
		}
	}
	
	if (err < 0) {
		fprintf(stderr, "error writing group: %s\n", snd_strerror(err));
		exit (1);
	}
#else
	recsrc = recsrc_from_channels (data->device);
	ioctl (data->device->fd, SOUND_MIXER_WRITE_RECSRC, &recsrc);
	
	ioctl (data->device->fd, SOUND_MIXER_READ_RECSRC, &recsrc);
	set_channels_from_recsrc (data->device, recsrc);
#endif

	for (c = data->device->channels; c; c = c->next) {
		channel_info *info;
		
		info = c->data;
		
		if (info == data) {
			continue;
		}
		
		/* Should probably block signal instead of use this passive=1 hack */
		info->passive = 1;
		
		/* check to see if channel can record */
		info->is_record_source = FALSE;
		
		if (info->record_source == TRUE &&
		    info->rec != NULL) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (info->rec),
						      info->is_record_source);
		}
		
		info->passive = 0;
	}
}

static void
adj_left_cb (GtkAdjustment *adjustment,
	     channel_info *data)
{
	unsigned long vol;
#ifdef ALSA
	int err;
	snd_mixer_group_t *g = data->mixer_group;
#endif
	if (data == NULL) {
		return;
	}
	
	if (data->is_stereo == TRUE) {
		if (data->is_locked) {
			vol = -adjustment->value;

			data->volume_left = vol;
			data->volume_right = vol;

			if (GTK_ADJUSTMENT(data->left)->value !=
			    GTK_ADJUSTMENT(data->right)->value) {
				gtk_adjustment_set_value (GTK_ADJUSTMENT (data->right),
							  GTK_ADJUSTMENT (data->left)->value);
			}
		} else {
			data->volume_left = -GTK_ADJUSTMENT(data->left)->value;
			data->volume_right = -GTK_ADJUSTMENT(data->right)->value;
		}
	} else {
		data->volume_left = -GTK_ADJUSTMENT(data->left)->value;
	}
	
#ifdef ALSA
	vol = g->max * data->volume_left / 100;
	
	/* avoid writing if there is no change */
	if (g->volume.values[SND_MIXER_CHN_FRONT_LEFT] != vol) {
		g->volume.values[SND_MIXER_CHN_FRONT_LEFT] = vol;

		/* This should be in an alsa_write_group function */
		while ((err = snd_mixer_group_write(data->device->handle, g)) < 0 &&
		       err == -EBUSY) {
			if ((err = snd_mixer_read (data->device->handle, &read_cbs)) < 0) {
				g_warning ("error reading group: %s\n", snd_strerror(err));
				exit (1);
			}
		}

		if (err < 0) {
			g_warning ("error writing group: %s\n", snd_strerror(err));
			exit (1);
		}
	}
#else
	vol = data->volume_left;
	vol |= data->volume_right << 8;
	ioctl (data->device->fd, MIXER_WRITE (data->channel), &vol);
#endif

	if (GTK_TOGGLE_BUTTON (data->mute)->active) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->mute), FALSE);
	}
}

static void
adj_right_cb (GtkAdjustment *adjustment,
	      channel_info *data)
{
	unsigned long vol;
#ifdef ALSA
	int err;
	snd_mixer_group_t *g=data->mixer_group;
#endif
	if (data == NULL) {
		return;
	}
	
	if (data->is_stereo == TRUE) {
		if (data->is_locked == TRUE) {
			vol = -adjustment->value;
			
			data->volume_left = vol;
			data->volume_right = vol;
			
			if (GTK_ADJUSTMENT(data->left)->value !=
			    GTK_ADJUSTMENT(data->right)->value) {
				gtk_adjustment_set_value (GTK_ADJUSTMENT (data->left),
							  GTK_ADJUSTMENT (data->right)->value);
			}
		} else {
			data->volume_left = -GTK_ADJUSTMENT(data->left)->value;
			data->volume_right = -GTK_ADJUSTMENT(data->right)->value;
		}
	} else {
		data->volume_left = -GTK_ADJUSTMENT(data->left)->value;
	}
	
#ifdef ALSA
	vol = g->max * data->volume_right / 100;
	
	/* avoid writing if there is no change */
	if (g->volume.values[SND_MIXER_CHN_FRONT_RIGHT] != vol) {
		g->volume.values[SND_MIXER_CHN_FRONT_RIGHT] = vol;
		while ((err = snd_mixer_group_write (data->device->handle, g)) < 0 &&
		       err == -EBUSY) {
			if ((err = snd_mixer_read (data->device->handle, &read_cbs)) < 0) {
				g_warning ("error reading group: %s\n", snd_strerror(err));
				exit (1);
			}
		}
		
		if (err < 0) {
			g_warning ("error writing group: %s\n", snd_strerror(err));
			exit (1);
		}
	}
#else
	vol = data->volume_left;
	vol |= data->volume_right << 8;
	ioctl (data->device->fd, MIXER_WRITE (data->channel), &vol);
#endif

	if (GTK_TOGGLE_BUTTON (data->mute)->active) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->lock), FALSE);
	}
}

void 
help_cb (GtkWidget *widget, 
	 gpointer data)
{
	GError *error = NULL;
	
	gnome_help_display ("gnome-volume-control", NULL, &error);

	if (error != NULL) {
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_MODAL,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 ("There was an error displaying help: \n%s"),
						 error->message);

		g_signal_connect (G_OBJECT (dialog), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);
		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_widget_show (dialog);
		g_error_free (error);
	}
}

static void
about_cb (GtkWidget *widget,
	  gpointer data)
{
	static GtkWidget *about = NULL;
	
	static const char *authors[] = {
		"Jens Ch. Restemeier",
		"Iain Holmes",
		NULL
	};
	
	if (about != NULL) {
		gdk_window_show (about->window);
		gdk_window_raise (about->window);
	} else {
		about = gnome_about_new ( _("Gnome Volume Control"), VERSION,
					  "(C) 1998 Jens Ch. Restemeier\n"
					  "(C) 2001-2002 Iain Holmes",
					  _("This is a mixer for sound devices"),
					  authors,
					  NULL, NULL,
					  NULL);
		
		g_signal_connect (G_OBJECT (about), "destroy",
				  G_CALLBACK (gtk_widget_destroyed), &about);
		gtk_widget_show (about);
	}
}

void
gmix_change_channel (channel_info *ci,
		     gboolean show)
{
	if (show == FALSE) {
		gtk_widget_hide (ci->separator);
		gtk_widget_hide (ci->mixer);
		if (ci->lock != NULL) {
			gtk_widget_hide (ci->lock);
		}

		if (ci->rec != NULL) {
			gtk_widget_hide (ci->rec);
		}
		
		gtk_widget_hide (ci->mute);
		
		if (ci->icon != NULL) {
			gtk_widget_hide (ci->icon);
		}
		gtk_widget_hide (ci->label);
	} else {
		gtk_widget_show (ci->separator);
		gtk_widget_show (ci->mixer);
		if (ci->lock != NULL) {
			gtk_widget_show (ci->lock);
		}

		if (ci->rec != NULL) {
			gtk_widget_show (ci->rec);
		}
		
		gtk_widget_show (ci->mute);
		if (ci->icon != NULL && prefs.show_icons == TRUE) {
			gtk_widget_show (ci->icon);
		}
		if (prefs.show_labels == TRUE) {
			gtk_widget_show (ci->label);
		}
	}

	ci->visible = show;

	gmix_hide_last_separator ();
}
		
void
gmix_change_icons (gboolean show)
{
	GList *p, *q;

	for (p = devices; p; p = p->next) {
		device_info *di = p->data;
		
		for (q = di->channels; q; q = q->next) {
			channel_info *ci = q->data;

			if (ci->visible == FALSE) {
				continue;
			}
			
			if (ci->icon == NULL) {
				continue;
			}
			
			if (show == TRUE) {
				gtk_widget_show (ci->icon);
			} else {
				gtk_widget_hide (ci->icon);
			}
		}
	}
}

void
gmix_change_labels (gboolean show)
{
	GList *p, *q;

	for (p = devices; p; p = p->next) {
		device_info *di = p->data;
		
		for (q = di->channels; q; q = q->next) {
			channel_info *ci = q->data;

			if (ci->visible == FALSE) {
				continue;
			}
			
			if (ci->label == NULL) {
				continue;
			}

			if (show == TRUE) {
				gtk_widget_show (ci->label);
			} else {
				gtk_widget_hide (ci->label);
			}
		}
	}
}

static void
gmix_hide_last_separator (void)
{
	GList *d, *c;
	gboolean hid = FALSE;
	
	for (d = devices; d; d = d->next) {
		device_info *di = d->data;

		for (c = g_list_last (di->channels); c; c = c->prev) {
			channel_info *ci = c->data;
		
			if (ci->visible && hid == FALSE) {
				gtk_widget_hide (ci->separator);
				hid = TRUE;
			} else if (ci->visible) {
				gtk_widget_show (ci->separator);
			}
		}
	}
}
