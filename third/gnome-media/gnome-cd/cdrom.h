/*
 * cdrom.h
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifndef __CDROM_H__
#define __CDROM_H__

#include <glib/gerror.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GNOME_CDROM_TYPE (gnome_cdrom_get_type ())
#define GNOME_CDROM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_CDROM_TYPE, GnomeCDRom))
#define GNOME_CDROM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GNOME_CDROM_TYPE, GnomeCDRomClass))
#define GNOME_IS_CDROM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_CDROM_TYPE))
#define GNOME_IS_CDROM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOME_CDROM_TYPE))
#define GNOME_CDROM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GNOME_CDROM_TYPE, GnomeCDRomClass))

#define GNOME_CDROM_ERROR gnome_cdrom_error_quark ()

#define ASSIGN_MSF(dest, src) \
{ \
  (dest).minute = (src).minute; \
  (dest).second = (src).second; \
  (dest).frame = (src).frame; \
}

typedef enum {
	GNOME_CDROM_DEVICE_STATIC,
	GNOME_CDROM_DEVICE_TRANSIENT
} GnomeCDRomDeviceLifetime;

typedef enum {
	GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
	GNOME_CDROM_ERROR_NOT_OPENED,
	GNOME_CDROM_ERROR_SYSTEM_ERROR,
	GNOME_CDROM_ERROR_IO,
	GNOME_CDROM_ERROR_NOT_READY
} GnomeCDRomError;

typedef struct _GnomeCDRom GnomeCDRom;
typedef struct _GnomeCDRomPrivate GnomeCDRomPrivate;
typedef struct _GnomeCDRomClass GnomeCDRomClass;

typedef enum _GnomeCDRomDriveStatus {
	GNOME_CDROM_STATUS_NOTHING,
	GNOME_CDROM_STATUS_OK,
	GNOME_CDROM_STATUS_NO_DISC,
	GNOME_CDROM_STATUS_TRAY_OPEN,
	GNOME_CDROM_STATUS_DRIVE_NOT_READY,
	GNOME_CDROM_STATUS_DATA_CD,
	GNOME_CDROM_STATUS_NO_CDROM
} GnomeCDRomDriveStatus;

typedef enum _GnomeCDRomAudioStatus {
	GNOME_CDROM_AUDIO_NOTHING,
	GNOME_CDROM_AUDIO_PLAY,
	GNOME_CDROM_AUDIO_PAUSE,
	GNOME_CDROM_AUDIO_COMPLETE,
	GNOME_CDROM_AUDIO_STOP,
	GNOME_CDROM_AUDIO_ERROR
} GnomeCDRomAudioStatus;

typedef enum _GnomeCDRomUpdate {
	GNOME_CDROM_UPDATE_NEVER,
	GNOME_CDROM_UPDATE_WHEN_CHANGED,
	GNOME_CDROM_UPDATE_CONTINOUS
} GnomeCDRomUpdate;

typedef enum _GnomeCDRomMode {
	GNOME_CDROM_WHOLE_CD,
	GNOME_CDROM_SINGLE_TRACK,
	GNOME_CDROM_LOOP,
	GNOME_CDROM_PLAY_ONCE
} GnomeCDRomMode;

typedef struct _GnomeCDRomMSF {
	int minute;
	int second;
	int frame;
} GnomeCDRomMSF;

typedef struct _GnomeCDRomStatus {
	GnomeCDRomDriveStatus cd;
	GnomeCDRomAudioStatus audio;
	int track;
	GnomeCDRomMSF relative;
	GnomeCDRomMSF absolute;
	int volume;
} GnomeCDRomStatus;

typedef struct _GnomeCDRomCDDBData {
	int discid;
	int ntrks;
	int *offsets;
	int nsecs;
} GnomeCDRomCDDBData;

struct _GnomeCDRom {
	GObject object;

	GnomeCDRomMode playmode, loopmode;

	int fd;

	GnomeCDRomDeviceLifetime lifetime;
	GnomeCDRomPrivate *priv;
};

struct _GnomeCDRomClass {
	GObjectClass klass;

	gboolean (*eject) (GnomeCDRom *cdrom,
			   GError **error);
	gboolean (*next) (GnomeCDRom *cdrom,
			  GError **error);
	gboolean (*ffwd) (GnomeCDRom *cdrom,
			  GError **error);
	gboolean (*play) (GnomeCDRom *cdrom,
			  int start_track,
			  GnomeCDRomMSF *start,
			  int finish_track,
			  GnomeCDRomMSF *finish,
			  GError **error);
	gboolean (*pause) (GnomeCDRom *cdrom,
			   GError **error);
	gboolean (*stop) (GnomeCDRom *cdrom,
			  GError **error);
	gboolean (*rewind) (GnomeCDRom *cdrom,
			    GError **error);
	gboolean (*back) (GnomeCDRom *cdrom,
			  GError **error);
	gboolean (*get_status) (GnomeCDRom *cdrom,
				GnomeCDRomStatus **status,
				GError **error);
	gboolean (*close_tray) (GnomeCDRom *cdrom,
				GError **error);

	gboolean (*set_volume) (GnomeCDRom *cdrom,
				int volume,
				GError **error);
	gboolean (*is_cdrom_device) (GnomeCDRom *cdrom,
				     const char *device,
				     GError **error);
	
	/* For CDDB */
	gboolean (*get_cddb_data) (GnomeCDRom *cdrom,
				   GnomeCDRomCDDBData **data,
				   GError **error);

	/* Configuration */
	gboolean (*set_device) (GnomeCDRom *cdrom,
				const char *device,
				GError **error);
	
	/* Signals */
	void (*status_changed) (GnomeCDRom *cdrom,
				GnomeCDRomStatus *status);

	/* Internal methods */
	gboolean (*open_dev)   (GnomeCDRom *cdrom,
				GError    **error);
	void     (*close_dev)  (GnomeCDRom *cdrom,
				gboolean    force_close);
	void     (*update_cd)  (GnomeCDRom *cdrom);
};

GQuark gnome_cdrom_error_quark (void);

GType gnome_cdrom_get_type (void) G_GNUC_CONST;
gboolean gnome_cdrom_eject (GnomeCDRom *cdrom,
			    GError **error);
gboolean gnome_cdrom_next (GnomeCDRom *cdrom,
			   GError **error);
gboolean gnome_cdrom_fast_forward (GnomeCDRom *cdrom,
				   GError **error);
gboolean gnome_cdrom_play (GnomeCDRom *cdrom,
			   int start_track,
			   GnomeCDRomMSF *start,
			   int finish_track,
			   GnomeCDRomMSF *finish,
			   GError **error);
gboolean gnome_cdrom_pause (GnomeCDRom *cdrom,
			    GError **error);
gboolean gnome_cdrom_pause (GnomeCDRom *cdrom,
			    GError **error);
gboolean gnome_cdrom_stop (GnomeCDRom *cdrom,
			   GError **error);
gboolean gnome_cdrom_rewind (GnomeCDRom *cdrom,
			     GError **error);
gboolean gnome_cdrom_back (GnomeCDRom *cdrom,
			   GError **error);
gboolean gnome_cdrom_get_status (GnomeCDRom *cdrom,
				 GnomeCDRomStatus **status,
				 GError **error);
gboolean gnome_cdrom_close_tray (GnomeCDRom *cdrom,
				 GError **error);
gboolean gnome_cdrom_set_volume (GnomeCDRom *cdrom,
				 int volume,
				 GError **error);
gboolean gnome_cdrom_is_cdrom_device (GnomeCDRom *cdrom,
				      const char *device,
				      GError **error);
gboolean gnome_cdrom_get_cddb_data (GnomeCDRom *cdrom,
				    GnomeCDRomCDDBData **data,
				    GError **error);
void gnome_cdrom_free_cddb_data (GnomeCDRomCDDBData *data);

/* This function is not defined in cdrom.c.
   It is defined in each of the architecture specific files. It is defined here
   so that the main program does not need to know about the architecture
   specific headers */
GnomeCDRom *gnome_cdrom_new (const char *cdrom_device,
			     GnomeCDRomUpdate update,
			     GError **error);


void gnome_cdrom_status_changed (GnomeCDRom *cdrom,
				 GnomeCDRomStatus *new_status);
gboolean gnome_cdrom_status_equal (GnomeCDRomStatus *status1,
				   GnomeCDRomStatus *status2);
GnomeCDRomStatus *gnome_cdrom_copy_status (GnomeCDRomStatus *original);

gboolean gnome_cdrom_set_device (GnomeCDRom *cdrom,
				 const char *device,
				 GError **error);

/* Methods for device implementors */

GnomeCDRom *gnome_cdrom_construct  (GnomeCDRom      *cdrom,
				    const char      *device,
				    GnomeCDRomUpdate update,
				    GnomeCDRomDeviceLifetime lifetime,
				    GError         **error);
gboolean    gnome_cdrom_open_dev   (GnomeCDRom      *cdrom,
				    GError         **error);
void        gnome_cdrom_close_dev  (GnomeCDRom      *cdrom,
				    gboolean         force_close);
void        gnome_cdrom_force_status_rescan (GnomeCDRom *cdrom);

G_END_DECLS

#endif
