/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-hal-mounts.c - read and monitor volumes using freedesktop HAL

   Copyright (C) 2004 David Zeuthen

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: David Zeuthen <david@fubar.dk>
*/

#include <config.h>

#ifdef USE_HAL

#include <fstab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>

#include <glib.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-i18n.h>

#include <libhal.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>


#include "gnome-vfs-hal-mounts.h"
#include "gnome-vfs-volume-monitor-daemon.h"
#include "gnome-vfs-volume-monitor-private.h"

/** Use HAL to retrieve information about drives and volumes.
 *
 *  TODO/QUESTIONS/ISSUES
 *
 *  - Fix HAL so hal_initialize() fails when we cannot connect to the HAL
 *    daemon; When this works GNOME VFS falls back to mtab/fstab monitoring.
 *    Which is nice.
 *
 *  - When a recordable disc (CD-R, CD-RW, DVD-R, DVD+RW etc.) is closed
 *    should we map down to the non-cdrecordable type? E.g. 
 *
 *                                 CD-R, CD-RW   |->   CD-ROM
 *       DVD-R, DVD-RW, DVD+R, DVD+RW, DVD-RAM   |->   DVD-ROM
 *
 *    Currently we don't do this. One reason for doing this is that consumers
 *    might download e.g. a Linux distrobution and burn the ISO's, close the
 *    disc and then it's a CD-ROM since they can't burn to the disc more
 *    anyway? Maybe we should do this, but not for RW discs, e.g. only
 *
 *                        CD-R   |->   CD-ROM
 *                DVD-R, DVD+R   |->   DVD-ROM
 *
 *    when closed; this I think would make sense. Comments?
 *
 *  - When libgphoto2 is completely halificated, get the GNOME VFS backend
 *    to work and add functionality (in this file) to show a camera icon that
 *    links to the appropriate GNOME VFS URI. (when the camera is not 
 *    usb-storage based of course)
 *
 *  - Do the same for MP3 players when GNOME VFS backends for these emerge.
 *
 *  NOTE
 *
 *  - For this patch you need a recent version of GNOME VFS that doesn't
 *    crash when a name contains a '/' character
 */


/* If this is defined then only mounted volumes are shown which gives
 *  a Mac OS X like experience. Otherwise unmounted volumes are shown
 *  as a drive icon. Obviously, this requires an automounter like
 *  gnome-volume-manager.
 */
/*#define HAL_ONLY_SHOW_MOUNTED_VOLUMES*/

/* If this is defined use the standard icons available. Otherwise new
 *  a new, more fine grained icon set is used. See below..
 *
 */
/*#define HAL_USE_STOCK_ICONS*/


#ifdef HAL_USE_STOCK_ICONS

#define HAL_ICON_DRIVE_REMOVABLE          "gnome-dev-removable"
#define HAL_ICON_DRIVE_REMOVABLE_USB      "gnome-dev-removable"
#define HAL_ICON_DRIVE_REMOVABLE_IEEE1394 "gnome-dev-removable"
#define HAL_ICON_DISC_CDROM               "gnome-dev-cdrom"
#define HAL_ICON_DISC_CDR                 "gnome-dev-cdrom"
#define HAL_ICON_DISC_CDRW                "gnome-dev-cdrom"
#define HAL_ICON_DISC_DVDROM              "gnome-dev-cdrom"
#define HAL_ICON_DISC_DVDRAM              "gnome-dev-cdrom"
#define HAL_ICON_DISC_DVDR                "gnome-dev-cdrom"
#define HAL_ICON_DISC_DVDRW               "gnome-dev-cdrom"
#define HAL_ICON_DISC_DVDRW_PLUS          "gnome-dev-cdrom"
#define HAL_ICON_DISC_DVDR_PLUS           "gnome-dev-cdrom"
#define HAL_ICON_MEDIA_COMPACT_FLASH      "gnome-dev-harddisk"
#define HAL_ICON_MEDIA_MEMORY_STICK       "gnome-dev-harddisk"
#define HAL_ICON_MEDIA_SMART_MEDIA        "gnome-dev-harddisk"
#define HAL_ICON_MEDIA_SD_MMC             "gnome-dev-harddisk"
#define HAL_ICON_MEDIA_FLOPPY             "gnome-dev-floppy"
#define HAL_ICON_MEDIA_HARDDISK           "gnome-dev-harddisk"
#define HAL_ICON_MEDIA_HARDDISK_USB       "gnome-dev-harddisk"
#define HAL_ICON_MEDIA_HARDDISK_IEEE1394  "gnome-dev-harddisk"
#define HAL_ICON_PORTABLE_AUDIO_PLAYER    "gnome-dev-harddisk"
#define HAL_ICON_CAMERA                   "gnome-dev-harddisk"

#else

#define HAL_ICON_DRIVE_REMOVABLE          "gnome-dev-removable"
#define HAL_ICON_DRIVE_REMOVABLE_USB      "gnome-dev-removable-usb"
#define HAL_ICON_DRIVE_REMOVABLE_IEEE1394 "gnome-dev-removable-ieee1394"
#define HAL_ICON_DISC_CDROM               "gnome-dev-cdrom"
#define HAL_ICON_DISC_CDR                 "gnome-dev-disc-cdr"
#define HAL_ICON_DISC_CDRW                "gnome-dev-disc-cdrw"
#define HAL_ICON_DISC_DVDROM              "gnome-dev-disc-dvdrom"
#define HAL_ICON_DISC_DVDRAM              "gnome-dev-disc-dvdram"
#define HAL_ICON_DISC_DVDR                "gnome-dev-disc-dvdr"
#define HAL_ICON_DISC_DVDRW               "gnome-dev-disc-dvdrw"
#define HAL_ICON_DISC_DVDRW_PLUS          "gnome-dev-disc-dvdrw-plus"
#define HAL_ICON_DISC_DVDR_PLUS           "gnome-dev-disc-dvdr-plus"
#define HAL_ICON_MEDIA_COMPACT_FLASH      "gnome-dev-media-cf"
#define HAL_ICON_MEDIA_MEMORY_STICK       "gnome-dev-media-ms"
#define HAL_ICON_MEDIA_SMART_MEDIA        "gnome-dev-media-sm"
#define HAL_ICON_MEDIA_SD_MMC             "gnome-dev-media-sdmmc"
#define HAL_ICON_MEDIA_FLOPPY             "gnome-dev-floppy"
#define HAL_ICON_MEDIA_HARDDISK           "gnome-dev-harddisk"
#define HAL_ICON_MEDIA_HARDDISK_USB       "gnome-dev-harddisk-usb"
#define HAL_ICON_MEDIA_HARDDISK_IEEE1394  "gnome-dev-harddisk-ieee1394"
#define HAL_ICON_PORTABLE_AUDIO_PLAYER    "gnome-dev-portable-audio-player"
#define HAL_ICON_CAMERA                   "gnome-dev-camera"

#endif /* HAL_USE_STOCK_ICONS */

typedef struct {
	char *udi;

	int device_major;
	int device_minor;
	char *device_file;

	char *bus;                /* ide, scsi, usb, ieee1394, ... */
	char *vendor;             /* may be NULL, is never "" */
	char *model;              /* may be NULL, is never "" */
	gboolean is_hotpluggable;
	gboolean is_removable;
	gboolean no_partitions;
	char *type;               /* disk, cdrom, floppy, compact_flash, 
				   * memory_stick, smart_media, sd_mmc, ... */

	char *physical_device;  /* UDI of physical device, e.g. the 
				 * IDE, USB, IEEE1394 device */

} GnomeVFSHalDrive;

typedef struct {
	char *udi;

	int device_major;
	int device_minor;
	char *device_file;
	char *volume_label; /* may be NULL, is never "" */
	gboolean is_mounted;
	char *mount_point;  /* NULL iff !is_mounted */
	char *fstype;       /* NULL iff !is_mounted or unknown */

	gboolean is_disc;
	char *disc_type;    /* NULL iff !is_disc */
	gboolean disc_has_audio;
	gboolean disc_has_data;
	gboolean disc_is_appendable;
	gboolean disc_is_blank;
	gboolean disc_is_rewritable;
} GnomeVFSHalVolume;

static void
_hal_free_drive (GnomeVFSHalDrive *drive)
{
	if (drive == NULL )
		return;

	free (drive->udi);
	hal_free_string (drive->device_file);
	hal_free_string (drive->bus);
	hal_free_string (drive->vendor);
	hal_free_string (drive->model);
	hal_free_string (drive->type);
}


static void
_hal_free_vol (GnomeVFSHalVolume *vol)
{
	if (vol == NULL )
		return;

	free (vol->udi);
	hal_free_string (vol->device_file);
	hal_free_string (vol->volume_label);
	hal_free_string (vol->fstype);
	hal_free_string (vol->mount_point);
	hal_free_string (vol->disc_type);
}


/* Given a UDI for a HAL device of capability 'storage', this
 *  function retrieves all the relevant properties into a convenient
 *  structure.  Returns NULL if UDI is invalid or device is not of
 *  capability 'storage'.
 *
 *  Free with _hal_free_drive().
 */
static GnomeVFSHalDrive *
_hal_get_drive (const char *udi, LibHalContext *hal_ctx)
{
	GnomeVFSHalDrive *drive;

	drive = g_new0 (GnomeVFSHalDrive, 1);
	if (drive == NULL)
		goto error;

	drive->udi = strdup (udi);
	if (drive->udi == NULL)
		goto error;

	drive->device_file = hal_device_get_property_string (
		hal_ctx, udi, "block.device");
	if (drive->device_file == NULL)
		goto error;

	drive->bus = hal_device_get_property_string (
		hal_ctx, udi, "storage.bus");
	if (drive->bus == NULL)
		goto error;

	drive->vendor = hal_device_get_property_string (
		hal_ctx, udi, "storage.vendor");
	if (drive->vendor == NULL)
		goto error;

	drive->model = hal_device_get_property_string (
		hal_ctx, udi, "storage.model");
	if (drive->model == NULL)
		goto error;

	drive->type = hal_device_get_property_string (
		hal_ctx, udi, "storage.drive_type");
	if (drive->type == NULL)
		goto error;
	
	drive->is_hotpluggable = hal_device_get_property_bool (
		hal_ctx, udi, "storage.hotpluggable");

	drive->is_removable = hal_device_get_property_bool (
		hal_ctx, udi, "storage.removable");

	drive->no_partitions = hal_device_get_property_bool (
		hal_ctx, udi, "block.no_partitions");

	drive->physical_device = hal_device_get_property_string (
		hal_ctx, udi, "storage.physical_device");
	if (drive->physical_device == NULL)
		goto error;

	return drive;
error:
	_hal_free_drive (drive);
	return NULL;
}

/* Given a UDI for a HAL device of capability 'volume', this function
 *  retrieves all the relevant properties into a convenient structure.
 *  Returns NULL if UDI is invalid or device is not of capability
 *  'volume'.
 *
 *  Free with _hal_free_vol().
 */
static GnomeVFSHalVolume *
_hal_get_vol (const char *udi, LibHalContext *hal_ctx)
{
	GnomeVFSHalVolume *vol;

	vol = g_new0 (GnomeVFSHalVolume, 1);
	if (vol == NULL)
		goto error;

	/* only care about capability volume */
	if (!hal_device_query_capability (
		    hal_ctx,
		    udi, "volume"))
		goto error;

	vol->udi = strdup (udi);
	if (vol->udi == NULL)
		goto error;

	vol->device_major = hal_device_get_property_int (
		hal_ctx, udi, "block.major");

	vol->device_minor = hal_device_get_property_int (
		hal_ctx, udi, "block.minor");

	vol->device_file = hal_device_get_property_string (
		hal_ctx,
		udi, "block.device");
	if (vol->device_file == NULL)
		goto error;

	if (hal_device_property_exists (
		    hal_ctx,
		    udi, "volume.label")) {
		vol->volume_label = hal_device_get_property_string (
			hal_ctx,
			udi, "volume.label");
		if (strlen (vol->volume_label) == 0) {
			hal_free_string (vol->volume_label);
			vol->volume_label = NULL;
		}
	}


	vol->is_mounted = hal_device_get_property_bool (
		hal_ctx,
		udi, "volume.is_mounted");
			    
	if (vol->is_mounted) {
		vol->mount_point = hal_device_get_property_string (
			hal_ctx,
			udi, "volume.mount_point");
		if (vol->mount_point == NULL || strlen (vol->mount_point) == 0)
			goto error;
		
	}

	vol->fstype = hal_device_get_property_string (
		hal_ctx,
		udi, "volume.fstype");
	if (vol->fstype == NULL)
		goto error;
	if (strlen(vol->fstype) == 0) {
		hal_free_string (vol->fstype);
		vol->fstype = NULL;
	}

	/* optical disc properties */
	if (hal_device_get_property_bool (hal_ctx,
					  udi, "volume.is_disc")) {
		vol->is_disc = TRUE;

		vol->disc_type = hal_device_get_property_string (
			hal_ctx,
			udi, "volume.disc.type");
		if (vol->disc_type == NULL || strlen (vol->disc_type) == 0)
			goto error;

		vol->disc_has_audio = hal_device_get_property_bool (
			hal_ctx,
			udi, "volume.disc.has_audio");
		vol->disc_has_data = hal_device_get_property_bool (
			hal_ctx,
			udi, "volume.disc.has_data");
		vol->disc_is_appendable = hal_device_get_property_bool (
			hal_ctx,
			udi, "volume.disc.is_appendable");
		vol->disc_is_blank = hal_device_get_property_bool (
			hal_ctx,
			udi, "volume.disc.is_blank");
		vol->disc_is_rewritable = hal_device_get_property_bool (
			hal_ctx,
			udi, "volume.disc.is_rewritable");
	}



	return vol;

error:
	_hal_free_vol (vol);
	return NULL;
}

/** Get the GnomeVFSHalDrive from a GnomeVFSHalVolume. Returns NULL on error.
 *
 *  Free with _hal_free_drive().
 */
static GnomeVFSHalDrive *
_hal_get_drive_from_vol (GnomeVFSHalVolume *vol, LibHalContext *hal_ctx)
{
	const char *storage_udi;

	storage_udi = hal_device_get_property_string (
		hal_ctx, vol->udi, "block.storage_device");	
	if (storage_udi == NULL)
		goto error;

	return _hal_get_drive (storage_udi, hal_ctx);
error:
	return NULL;
}



/***********************************************************************/

/** If you don't have a recent GNOME VFS you need to uncomment the code
 *  below
 *
 *  OLD ERROR: Fixup the name given to GnomeVFS, it may not contain
 *  '/'-characters.  Substitute these with '-'.
 */
static void
fixup_name (char *name)
{
	;
	/* 
	int i;

	for (i=0; name[i]!='\0'; i++) {
		if (name[i] == '/')
			name[i] = '-';
	}
	*/
}

/***********************************************************************/

static char *_hal_get_vol_name (GnomeVFSHalVolume *vol, 
				GnomeVFSHalDrive *drive, 
				LibHalContext *hal_ctx);
static char *_hal_get_vol_icon (GnomeVFSHalVolume *vol, 
				GnomeVFSHalDrive *drive, 
				LibHalContext *hal_ctx);
static int _hal_get_vol_type (GnomeVFSHalVolume *vol, 
			      GnomeVFSHalDrive *drive, 
			      LibHalContext *hal_ctx);



/* vol may be NULL */
static char *
_hal_get_drive_name (GnomeVFSHalVolume *vol, GnomeVFSHalDrive *drive, 
		     LibHalContext *hal_ctx)
{
	char *name;


#ifdef HAL_ONLY_SHOW_MOUNTED_VOLUMES
	/* this is safe, as drives and volumes are 1:1 */
	if (vol != NULL)
		return _hal_get_vol_name (vol, drive, hal_ctx);
#else
	/* Bork, storage.removable is a bit broken on HAL (I fear only
	 * .fdi files will fix it), so it's not sane to rely on this..
	 * Which is bad, because without computer:/// looks ugly, e.g.
	 * "<drive_name> : <volume_name>" even for storage that is
	 * not removable. Bork.
	 */

        /*
	if (vol != NULL && !drive->is_removable)
		return _hal_get_vol_name (vol, drive, hal_ctx);
	*/
#endif

	if (strcmp (drive->type, "cdrom") == 0) {
		gboolean cdr;
		gboolean cdrw;
		gboolean dvd;
		gboolean dvdplusr;
		gboolean dvdplusrw;
		gboolean dvdr;
		gboolean dvdram;
		char *first;
		char *second;

		/* use the capabilities of the optical device */

		cdr = hal_device_get_property_bool (
			hal_ctx, drive->udi, "storage.cdrom.cdr");
		cdrw = hal_device_get_property_bool (
			hal_ctx, drive->udi, "storage.cdrom.cdrw");
		dvd = hal_device_get_property_bool (
			hal_ctx, drive->udi, "storage.cdrom.dvd");
		dvdplusr = hal_device_get_property_bool (
			hal_ctx, drive->udi, "storage.cdrom.dvdplusr");
		dvdplusrw = hal_device_get_property_bool (
			hal_ctx, drive->udi, "storage.cdrom.dvdplusrw");
		dvdr = hal_device_get_property_bool (
			hal_ctx, drive->udi, "storage.cdrom.dvdr");
		dvdram = hal_device_get_property_bool (
			hal_ctx, drive->udi, "storage.cdrom.dvdram");

		first = "CD-ROM";
		if( cdr )
			first = "CD-R";
		if( cdrw )
			first = "CD-RW";

		/* Hmm, '/' is not an allowed character, so use '\' instead 
		 *
		 * TODO: Fix this
		 */
		second = "";
		if( dvd )
			second = "/DVD-ROM";
		if( dvdplusr )
			second = "/DVD+R";
		if( dvdplusrw )
			second = "/DVD+RW";
		if( dvdr )
			second = "/DVD-R";
		if( dvdram )
			second = "/DVD-RAM";
		if( dvdplusr && dvdr )
			second = "/DVD±R";
		if( dvdplusr && dvdplusrw && dvdr )
			second = "/DVD±RW";

		name = g_strdup_printf ("%s%s", first, second);
	} else if (strcmp (drive->type, "floppy") == 0) {
		/* TODO: Check for LS120 or Zip drive etc. */
		name = g_strdup ("Floppy Drive");
	} else {
		name = g_strdup (drive->model);
	}

	fixup_name (name);

	return name;
}

/* vol may be NULL */
static char *
_hal_get_drive_icon (GnomeVFSHalVolume *vol, GnomeVFSHalDrive *drive, 
		     LibHalContext *hal_ctx)
{
	char *name;

	if (strcmp (drive->bus, "usb") == 0) {
		name = g_strdup (HAL_ICON_DRIVE_REMOVABLE_USB);
	} else if (strcmp (drive->bus, "ieee1394") == 0) {
		name = g_strdup (HAL_ICON_DRIVE_REMOVABLE_IEEE1394);
	} else {
		name = g_strdup (HAL_ICON_DRIVE_REMOVABLE);
	}

	return name;
}

/* vol may be NULL */
static int
_hal_get_drive_type (GnomeVFSHalVolume *vol, GnomeVFSHalDrive *drive, 
		     LibHalContext *hal_ctx)
{
	int type;

	type = GNOME_VFS_DEVICE_TYPE_HARDDRIVE;

	if (strcmp (drive->type, "cdrom") == 0)
		type = GNOME_VFS_DEVICE_TYPE_CDROM;
	if (strcmp (drive->type, "floppy") == 0)
		type = GNOME_VFS_DEVICE_TYPE_FLOPPY;
	else if (strcmp (drive->type, "compact_flash") == 0 ||
		 strcmp (drive->type, "memory_stick") == 0 ||
		 strcmp (drive->type, "smart_media") == 0 ||
		 strcmp (drive->type, "sd_mmc") == 0)
		type = GNOME_VFS_DEVICE_TYPE_MEMORY_STICK;

	return type;
}



/***********************************************************************/

static char *
_hal_get_vol_name (GnomeVFSHalVolume *vol, GnomeVFSHalDrive *drive, 
		   LibHalContext *hal_ctx)
{
	char *name;

	name = NULL;
	if (vol->volume_label != NULL ) {

		/* Using the label is the best thing */
		name = g_strdup (vol->volume_label);

	} else if (strcmp (drive->type, "cdrom") == 0) {

		/* If it's a optical disc, use the disc type */
		if (strcmp (vol->disc_type, "cd_rom") == 0) {
			name = g_strdup (_("CD-ROM Disc"));
		} else if (strcmp (vol->disc_type, "cd_r") == 0) {
			if (vol->disc_is_blank)
				name = g_strdup (_("Blank CD-R Disc"));
			else
				name = g_strdup (_("CD-R Disc"));
		} else if (strcmp (vol->disc_type, "cd_rw") == 0) {
			if (vol->disc_is_blank)
				name = g_strdup (_("Blank CD-RW Disc"));
			else
				name = g_strdup (_("CD-RW Disc"));
		} else if (strcmp (vol->disc_type, "dvd_rom") == 0) {
					name = g_strdup (_("DVD-ROM Disc"));
		} else if (strcmp (vol->disc_type, "dvd_r") == 0) {
			if (vol->disc_is_blank)
				name = g_strdup (_("Blank DVD-R Disc"));
			else
				name = g_strdup (_("DVD-R Disc"));
		} else if (strcmp (vol->disc_type, "dvd_ram") == 0) {
			if (vol->disc_is_blank)
				name = g_strdup (_("Blank DVD-RAM Disc"));
			else
				name = g_strdup (_("DVD-RAM Disc"));
		} else if ((strcmp (vol->disc_type, "dvd_rw_restricted_overwrite") == 0) || (strcmp (vol->disc_type, "dvd_rw") == 0)) {
			if (vol->disc_is_blank)
				name = g_strdup (_("Blank DVD-RW Disc"));
			else
				name = g_strdup (_("DVD-RW Disc"));
		} else if (strcmp (vol->disc_type, "dvd_plus_rw") == 0) {
			if (vol->disc_is_blank)
				name = g_strdup (_("Blank DVD+RW Disc"));
			else
				name = g_strdup (_("DVD+RW Disc"));
		} else if (strcmp (vol->disc_type, "dvdplusr") == 0) {
			if (vol->disc_is_blank)
				name = g_strdup (_("Blank DVD+R Disc"));
			else
				name = g_strdup (_("DVD+R Disc"));
		} else {
			name = g_strdup (_("Optical Disc"));
		}

		/* Special case for pure audio disc */
		if (vol->disc_has_audio && !vol->disc_has_data) {
			g_free (name);
			name = g_strdup (_("Audio Disc"));
		}

	} else if (strcmp (drive->type, "compact_flash") == 0) {
		name = g_strdup (_("Compact Flash Media"));
	} else if (strcmp (drive->type, "memory_stick") == 0) {
		name = g_strdup (_("Memory Stick Media"));
	} else if (strcmp (drive->type, "smart_media") == 0) {
		name = g_strdup (_("Smart Media Media"));
	} else if (strcmp (drive->type, "sd_mmc") == 0) {
		name = g_strdup (_("SD/MMC Media"));
	} else if (strcmp (drive->type, "floppy") == 0) {
		name = g_strdup (_("Floppy Disk"));
	} else if (strcmp (drive->type, "disk") == 0) {

		/* Look at the filesystem type, if applicable */
		if (vol->fstype != NULL) {

			    if (strcmp (vol->fstype, "hfs") == 0) {
				    name = g_strdup (_("Mac OS disk"));
			    } else if (strcmp (vol->fstype, "hfsplus") == 0) {
				    name = g_strdup (_("Mac OS X disk"));
			    } else if (strcmp (vol->fstype, "vfat") == 0 ||
				strcmp (vol->fstype, "fat") == 0 ||
				strcmp (vol->fstype, "msdos") == 0 ||
				strcmp (vol->fstype, "msdosfs") == 0 ) {
				    /* Many cameras and mp3 players use
				     * FAT and we don't want the volume
				     * icon to say "DOS Disk"; we'd rather
				     * want the name of the device as fallback;
				     * see below..
				     */
				    ;
				    /*name = g_strdup (_("DOS Disk"));*/
			    } else if (strcmp (vol->fstype, "ntfs") == 0) {
				    name = g_strdup (_("Windows Disk"));
			    } else if (strcmp (vol->fstype, "ext2") == 0 ||
				       strcmp (vol->fstype, "ext3") == 0 ||
				       strcmp (vol->fstype, "jfs") == 0 ||
				       strcmp (vol->fstype, "xfs") == 0 ||
				       strcmp (vol->fstype, "reiser") == 0) {
				    name = g_strdup (_("Linux Disk"));
			    }
		}
	}

	/* fallback; use the same name as the drive */
	if (name == NULL)
		name = _hal_get_drive_name (NULL, drive, hal_ctx);

	fixup_name (name);

	return name;
}

static char *
_hal_get_vol_icon (GnomeVFSHalVolume *vol, GnomeVFSHalDrive *drive, 
		   LibHalContext *hal_ctx)
{
	char *icon;
	char *category;


	category = hal_device_get_property_string (hal_ctx, 
						   drive->physical_device,
						   "info.category");
	if (category != NULL) {
		if (strcmp (category, "portable_audio_player") == 0) {
			icon = g_strdup (HAL_ICON_PORTABLE_AUDIO_PLAYER);
			return icon;
		} else if (strcmp (category, "camera") == 0) {
			icon = g_strdup (HAL_ICON_CAMERA);
			return icon;
		}
	}


	if (strcmp (drive->type, "cdrom") == 0) {

		/* If it's a optical disc, use the disc type */
		if (strcmp (vol->disc_type, "cd_rom") == 0) {
			icon = g_strdup (HAL_ICON_DISC_CDROM);
		} else if (strcmp (vol->disc_type, "cd_r") == 0) {
			icon = g_strdup (HAL_ICON_DISC_CDR);
		} else if (strcmp (vol->disc_type, "cd_rw") == 0) {
			icon = g_strdup (HAL_ICON_DISC_CDRW);
		} else if (strcmp (vol->disc_type, "dvd_rom") == 0) {
			icon = g_strdup (HAL_ICON_DISC_DVDROM);
		} else if (strcmp (vol->disc_type, "dvd_r") == 0) {
			icon = g_strdup (HAL_ICON_DISC_DVDR);
		} else if (strcmp (vol->disc_type, "dvd_ram") == 0) {
			icon = g_strdup (HAL_ICON_DISC_DVDRAM);
		} else if ((strcmp (vol->disc_type, "dvd_rw_restricted_overwrite") == 0) || (strcmp (vol->disc_type, "dvd_rw") == 0)) {
			icon = g_strdup (HAL_ICON_DISC_DVDRW);
		} else if (strcmp (vol->disc_type, "dvd_plus_rw") == 0) {
			icon = g_strdup (HAL_ICON_DISC_DVDRW_PLUS);
		} else if (strcmp (vol->disc_type, "dvdplusr") == 0) {
			icon = g_strdup (HAL_ICON_DISC_DVDR_PLUS);
		} else {
			icon = g_strdup (HAL_ICON_DISC_CDROM);
		}

	} else if (strcmp (drive->type, "floppy") == 0) {
		icon = g_strdup (HAL_ICON_MEDIA_FLOPPY);
	} else if (strcmp (drive->type, "compact_flash") == 0) {
		icon = g_strdup (HAL_ICON_MEDIA_COMPACT_FLASH);
	} else if (strcmp (drive->type, "memory_stick") == 0) {
		icon = g_strdup (HAL_ICON_MEDIA_MEMORY_STICK);
	} else if (strcmp (drive->type, "smart_media") == 0) {
		icon = g_strdup (HAL_ICON_MEDIA_SMART_MEDIA);
	} else if (strcmp (drive->type, "sd_mmc") == 0) {
		icon = g_strdup (HAL_ICON_MEDIA_SD_MMC);
	} else {

		if (strcmp (drive->bus, "usb") == 0) {
			icon = g_strdup (HAL_ICON_MEDIA_HARDDISK_USB);
		} else if (strcmp (drive->bus, "ieee1394") == 0) {
			icon = g_strdup (HAL_ICON_MEDIA_HARDDISK_IEEE1394);
		} else {
			icon = g_strdup (HAL_ICON_MEDIA_HARDDISK);
		}
	}
	
	return icon;
}

static int
_hal_get_vol_type (GnomeVFSHalVolume *vol, GnomeVFSHalDrive *drive, 
		   LibHalContext *hal_ctx)
{
	return _hal_get_drive_type (NULL, drive, hal_ctx);
}

/***********************************************************************/

/** This function is used to skip certain volumes/drives we don't
 *  want to expose in GnomeVFS.
 *
 */
static gboolean
_hal_old_school_mount_point (GnomeVFSHalDrive *hal_drive, 
			     GnomeVFSHalVolume *hal_vol,  /* may be NULL */
			     char *mount_point)
{
	/* Skip standard UNIX-like mount points */
	if (strcmp (mount_point, "/var") == 0 ||
	    strcmp (mount_point, "/usr") == 0 ||
	    strcmp (mount_point, "/bin") == 0 ||
	    strcmp (mount_point, "/sbin") == 0 ||
	    strcmp (mount_point, "/boot") == 0 ||
	    strcmp (mount_point, "/tmp") == 0 ||
	    strcmp (mount_point, "/opt") == 0 ||
	    strcmp (mount_point, "/home") == 0 ||
	    strcmp (mount_point, "/") == 0)
		return TRUE;

	return FALSE;
}

/***********************************************************************/


/* Add a drive where the media is not partition based.
 *
 *  It's safe to call this function multiple times for the same HAL 
 *  UDI (Unique Device Identifier).
 */
static void 
_hal_add_drive_no_partitions (
	GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon,
	const char *udi)
{
	GnomeVFSHalDrive *hal_drive = NULL;
	LibHalContext *hal_ctx = volume_monitor_daemon->hal_ctx;
	GnomeVFSDrive *drive;
	GnomeVFSVolumeMonitor *volume_monitor = 
		GNOME_VFS_VOLUME_MONITOR (volume_monitor_daemon);
	char *name = NULL;
	char *icon = NULL;
	GnomeVFSDeviceType device_type = GNOME_VFS_DEVICE_TYPE_HARDDRIVE;
	gboolean computer_visible = TRUE;
	struct fstab *fst;
	char *mount_point = NULL;

#ifdef HAL_ONLY_SHOW_MOUNTED_VOLUMES
	return;
#endif

	/*
	 * We create the GnomeVFSDrive for such beasts since we can
	 * actually do it in advance due to the fact that we already
	 * know the device file and the mount point.
	 */
	
	hal_drive = _hal_get_drive (udi, hal_ctx);
	if (hal_drive == NULL)
		goto out;

	/* get mount point from /etc/fstab */
	if (setfsent () == 1) {
		fst = getfsspec (hal_drive->device_file);
		if (fst != NULL )
			mount_point = strdup (fst->fs_file);
		endfsent ();
	}
	if (mount_point == NULL || mount_point[0] != '/')
		goto out;

	/* Include, but hide, drives with a hidden mount point */
	if (_hal_old_school_mount_point (hal_drive, NULL, mount_point)) {
		computer_visible = FALSE;
	}
	
	/* see if drive was already added */
	drive = _gnome_vfs_volume_monitor_find_drive_by_hal_udi (
		volume_monitor, udi);
	if (drive == NULL ) {

		/* nope, make one */
		
		name = _hal_get_drive_name (NULL, hal_drive, hal_ctx);
		icon = _hal_get_drive_icon (NULL, hal_drive, hal_ctx);
		device_type = _hal_get_drive_type (NULL, hal_drive, hal_ctx);
		
		drive = g_object_new (GNOME_VFS_TYPE_DRIVE, NULL);
		drive->priv->device_path = g_strdup (hal_drive->device_file);
		
		drive->priv->activation_uri = gnome_vfs_get_uri_from_local_path (mount_point);
		drive->priv->is_connected = TRUE;
		drive->priv->device_type = device_type;
		drive->priv->icon = g_strdup (icon);
		drive->priv->display_name = _gnome_vfs_volume_monitor_uniquify_drive_name (volume_monitor, name);
		drive->priv->is_user_visible = computer_visible;
		drive->priv->volumes = NULL;
		drive->priv->hal_udi = g_strdup (udi);
		
		_gnome_vfs_volume_monitor_connected (volume_monitor, drive);
		gnome_vfs_drive_unref (drive);
	}
	
out:
	_hal_free_drive (hal_drive);
	g_free (name);
	g_free (icon);
	free (mount_point);
}


/* Add a hal-volume, that is, a HalDevice of capability 'volume'. 
 *
 *  It's safe to call this function multiple times for the same HAL 
 *  UDI (Unique Device Identifier).
 */
static void 
_hal_add_volume (GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon,
		 const char *udi)
{
	GnomeVFSHalVolume *hal_vol = NULL;
	GnomeVFSHalDrive *hal_drive = NULL;
	LibHalContext *hal_ctx = volume_monitor_daemon->hal_ctx;

	GnomeVFSDrive *drive;
	GnomeVFSVolume *vol;
	GnomeVFSVolumeMonitor *volume_monitor = 
		GNOME_VFS_VOLUME_MONITOR (volume_monitor_daemon);
	char *name = NULL;
	char *icon = NULL;
	GnomeVFSDeviceType device_type = GNOME_VFS_DEVICE_TYPE_HARDDRIVE;
	gboolean computer_visible = TRUE;
	gboolean desktop_visible = FALSE;
	struct fstab *fst;
	char *mount_point = NULL;
	gboolean is_blank_disc = FALSE;
	gboolean is_audio_disc = FALSE;

	/* Only care about capability volume */
	if (!hal_device_query_capability (
		    volume_monitor_daemon->hal_ctx,
		    udi, "volume"))
	{

		/* And capability block where block.no_partition==TRUE and
		 * block.is_volume==FALSE
		 */
		if (hal_device_query_capability (
			    hal_ctx, udi, "block") &&
		    (hal_device_get_property_bool (
			    hal_ctx, udi, "block.no_partitions")==TRUE) &&
		    (hal_device_get_property_bool (
			    hal_ctx, udi, "block.is_volume")==FALSE)) {
			/* This represent top-level block devices with
			 * media not using partitions.. Such as floppy
			 * drives or optical drives..
			 */

			_hal_add_drive_no_partitions (
				volume_monitor_daemon, udi);
			goto out;

		} else {
			/* Nothing we can use... */
			goto out;
		}
	}


	/* get HAL drive and volume objects */
	hal_vol = _hal_get_vol (udi, hal_ctx);
	if (hal_vol == NULL)
		goto out;
	hal_drive = _hal_get_drive_from_vol (hal_vol, hal_ctx);
	if (hal_drive == NULL)
		goto out;

	/* See if we should have an icon on the desktop */
	if (hal_drive->is_hotpluggable || 
	    hal_vol->is_disc ||
	    hal_drive->is_removable)
		desktop_visible = TRUE;

	/* If we are mounted, use the mount point that HAL knows */
	if (hal_vol->is_mounted) {
		mount_point = strdup (hal_vol->mount_point);
	} else {
		/* Otherwise... Yuck.. 
		 *
		 * gnome-vfs expects a mount point for a not yet
		 * mounted volume which is kind of gross as it may be
		 * mounted anywhere by e.g. root in the
		 * future. Surely, this is not needed at all,
		 * gnome-vfs should just be able to do a 'mount
		 * /dev/sda1' or something.
		 *
		 * Now, HAL *could* monitor the /etc/fstab file and
		 * maintain volume.mount_point property while
		 * volume.is_mounted is FALSE, but this is unclean and
		 * we really want the volume.mount_point to be empty
		 * if, and only if, volume.is_mounted is FALSE.
		 *
		 * So, we just need to read the /etc/fstab here to
		 * find the mount point (which is probably created by
		 * a callout anyway). Oh well, piece of cake anyway..
		 */
		if (setfsent () == 1) {
			fst = getfsspec (hal_vol->device_file);
			if (fst != NULL )
				mount_point = strdup (fst->fs_file);
			endfsent ();
		}
	}

	/* And we do need a mount point, and now we tried so hard to find
	 * one.. bail out if we haven't got any.. 
	 *
	 * Further, getfspec above may return 'swap', so only accept a mount
	 * point that starts with /. Hmmm..
	 */
	if (mount_point == NULL || mount_point[0]!='/')
		goto out;

	/* bail out if fstype is swap */
	if (hal_vol->fstype != NULL && strcmp (hal_vol->fstype, "swap") == 0)
		goto out;

	/* see if we're a blank disc instead of being mounted */
	is_blank_disc = hal_vol->is_disc && hal_vol->disc_is_blank;

	/* see if we're a pure audio disc instead of being mounted */
	is_audio_disc = hal_vol->is_disc && hal_vol->disc_has_audio &&
		!hal_vol->disc_has_data;

	/* Include, but hide, volumes with a hidden mount point */
	if (_hal_old_school_mount_point (hal_drive, hal_vol, mount_point)) {
		desktop_visible = FALSE;
		computer_visible = FALSE;
	}

#ifdef HAL_ONLY_SHOW_MOUNTED_VOLUMES
	/* we should also show blank discs */
	if (!hal_vol->is_mounted && !is_blank_disc)
		goto out;
#endif

	/* see if it was already added */
	drive = _gnome_vfs_volume_monitor_find_drive_by_hal_udi (
		volume_monitor, 
#ifdef HAL_ONLY_SHOW_MOUNTED_VOLUMES
			udi);
#else
			hal_drive->no_partitions ? hal_drive->udi : udi);
#endif
	if (drive == NULL ) {

		/* nope, make one */

		name = _hal_get_drive_name (hal_vol, hal_drive, hal_ctx);
		icon = _hal_get_drive_icon (hal_vol, hal_drive, hal_ctx);
		device_type = _hal_get_drive_type (hal_vol, hal_drive, hal_ctx);

		drive = g_object_new (GNOME_VFS_TYPE_DRIVE, NULL);
		drive->priv->device_path = g_strdup (hal_vol->device_file);

		drive->priv->activation_uri = gnome_vfs_get_uri_from_local_path (mount_point);
		drive->priv->is_connected = TRUE;
		drive->priv->device_type = device_type;
		drive->priv->icon = g_strdup (icon);
		drive->priv->display_name = _gnome_vfs_volume_monitor_uniquify_drive_name (volume_monitor, name);
		drive->priv->is_user_visible = computer_visible;
		drive->priv->volumes = NULL;

		drive->priv->hal_udi = g_strdup (
#ifdef HAL_ONLY_SHOW_MOUNTED_VOLUMES
			udi
#else
			hal_drive->no_partitions ? hal_drive->udi : udi
#endif
		);
		_gnome_vfs_volume_monitor_connected (volume_monitor, drive);
		gnome_vfs_drive_unref (drive);
	} 

	g_free (name); name = NULL;
	g_free (icon); icon = NULL;

	/* drive is now a reference to GnomeVFSDrive */

	/* Only create a GnomeVFSVolume object if the volume is mounted 
	 * or a blank disc */
	if (hal_vol->is_mounted || is_blank_disc || is_audio_disc) {
		/* see if hal_vol was already added */
		vol = _gnome_vfs_volume_monitor_find_volume_by_hal_udi (
			volume_monitor, udi);
		if (vol == NULL ) {

			name = _hal_get_vol_name (hal_vol, hal_drive, hal_ctx);
			icon = _hal_get_vol_icon (hal_vol, hal_drive, hal_ctx);
			device_type = _hal_get_vol_type (hal_vol, hal_drive, hal_ctx);

			vol = g_object_new (GNOME_VFS_TYPE_VOLUME, NULL);
			vol->priv->hal_udi = g_strdup (udi);
			vol->priv->volume_type = GNOME_VFS_VOLUME_TYPE_MOUNTPOINT;
			
			if (is_blank_disc) {
				vol->priv->device_path = g_strdup (hal_vol->device_file);
				vol->priv->activation_uri = g_strdup ("burn:///");
				vol->priv->unix_device = makedev (hal_vol->device_major, 
								  hal_vol->device_minor);
				vol->priv->filesystem_type = g_strdup (hal_vol->fstype);

			} else if (is_audio_disc) {
				vol->priv->device_path = g_strdup (hal_vol->device_file);
				vol->priv->activation_uri = g_strdup_printf (
					"cddb://%s", hal_vol->device_file);
				vol->priv->unix_device = makedev (hal_vol->device_major, 
								  hal_vol->device_minor);
				vol->priv->filesystem_type = g_strdup (hal_vol->fstype);

			} else {
				vol->priv->device_path = g_strdup (hal_vol->device_file);
				vol->priv->activation_uri = gnome_vfs_get_uri_from_local_path (mount_point);
				vol->priv->unix_device = makedev (hal_vol->device_major, 
								  hal_vol->device_minor);
				vol->priv->filesystem_type = g_strdup (hal_vol->fstype);
			}

			vol->priv->is_read_only = FALSE;
			vol->priv->is_mounted = TRUE;
			
			vol->priv->device_type = device_type;
			
			vol->priv->display_name = _gnome_vfs_volume_monitor_uniquify_volume_name (volume_monitor, name);
			vol->priv->icon = g_strdup(icon);
			vol->priv->is_user_visible = desktop_visible;
			
			vol->priv->drive = drive;
			_gnome_vfs_drive_add_mounted_volume (drive, vol);
			
			_gnome_vfs_volume_monitor_mounted (volume_monitor, vol);			gnome_vfs_volume_unref (vol);

		}
	}

out:
	_hal_free_vol (hal_vol);
	_hal_free_drive (hal_drive);
	g_free (name);
	g_free (icon);
	free (mount_point);
}


/*  Remove a hal-volume, that is, a HalDevice of capability 'volume'. 
 *
 *  It's safe to call this function even though the hal-volume doesn't
 *  correspond to any (GnomeVFSDrive, GnomeVFSVolume) pair.
 */
static void 
_hal_remove_volume (GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon,
		    const char *udi)
{
	GnomeVFSDrive *drive;
	GnomeVFSVolume *vol;
	GnomeVFSVolumeMonitor *volume_monitor;

	volume_monitor = GNOME_VFS_VOLUME_MONITOR (volume_monitor_daemon);

	drive = _gnome_vfs_volume_monitor_find_drive_by_hal_udi (
		volume_monitor, udi);
	if (drive != NULL ) {
		_gnome_vfs_volume_monitor_disconnected (volume_monitor, drive);
	}

	vol = _gnome_vfs_volume_monitor_find_volume_by_hal_udi (
		volume_monitor, udi);
	if (vol != NULL ) {
		_gnome_vfs_volume_monitor_unmounted (volume_monitor, vol);
	}
}


/*  Call when a HAL volume is unmounted.
 *
 *  It's safe to call this function even though the hal-volume doesn't
 *  correspond to GnomeVFSVolume object.
 */
static void 
_hal_volume_unmounted (GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon,
		       const char *udi)
{
	GnomeVFSVolume *vol;
	GnomeVFSVolumeMonitor *volume_monitor;

	volume_monitor = GNOME_VFS_VOLUME_MONITOR (volume_monitor_daemon);

	vol = _gnome_vfs_volume_monitor_find_volume_by_hal_udi (
		volume_monitor, udi);

	if (vol != NULL ) {
		_gnome_vfs_volume_monitor_unmounted (volume_monitor, vol);
	}

#ifdef HAL_ONLY_SHOW_MOUNTED_VOLUMES
	{
		GnomeVFSDrive *drive;
		drive = _gnome_vfs_volume_monitor_find_drive_by_hal_udi (
			volume_monitor, udi);
		if (drive != NULL ) {
			_gnome_vfs_volume_monitor_disconnected (volume_monitor, drive);
		}
	}
#endif

}


void 
_gnome_vfs_monitor_hal_get_volume_list (GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon)
{
	int i;
	int num_hal_volumes;
	char **hal_volumes;
	GnomeVFSVolumeMonitor *volume_monitor;


	volume_monitor = GNOME_VFS_VOLUME_MONITOR (volume_monitor_daemon);

	hal_volumes = hal_find_device_by_capability (
		volume_monitor_daemon->hal_ctx,
		"volume",
		&num_hal_volumes);
	for (i=0; i<num_hal_volumes; i++) {
		char *udi;

		udi = hal_volumes [i];

		_hal_add_volume (volume_monitor_daemon, udi);
	}
	hal_free_string_array (hal_volumes);

	hal_volumes = hal_find_device_by_capability (
		volume_monitor_daemon->hal_ctx,
		"block",
		&num_hal_volumes);
	for (i=0; i<num_hal_volumes; i++) {
		char *udi;

		udi = hal_volumes [i];

		_hal_add_volume (volume_monitor_daemon, udi);
	}
	hal_free_string_array (hal_volumes);
}

static void
_hal_mainloop_integration (LibHalContext *ctx, 
			   DBusConnection * dbus_connection)
{
        dbus_connection_setup_with_g_main (dbus_connection, NULL);
}

static void 
_hal_device_added (LibHalContext *ctx, 
		   const char *udi)
{
	GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon;
	volume_monitor_daemon = (GnomeVFSVolumeMonitorDaemon *)
		hal_ctx_get_user_data (ctx);
	_hal_add_volume (volume_monitor_daemon, udi);
}

static void 
_hal_device_removed (LibHalContext *ctx, 
		     const char *udi)
{
	GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon;
	volume_monitor_daemon = (GnomeVFSVolumeMonitorDaemon *)
		hal_ctx_get_user_data (ctx);
	_hal_remove_volume (volume_monitor_daemon, udi);
}

static void 
_hal_device_new_capability (LibHalContext *ctx, 
			    const char *udi, 
			    const char *capability)
{
	GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon;
	volume_monitor_daemon = (GnomeVFSVolumeMonitorDaemon *)
		hal_ctx_get_user_data (ctx);
	_hal_add_volume (volume_monitor_daemon, udi);
}

static void 
_hal_device_lost_capability (LibHalContext *ctx, 
			     const char *udi,
			     const char *capability)
{
}

static void 
_hal_device_property_modified (LibHalContext *ctx, 
			       const char *udi,
			       const char *key,
			       dbus_bool_t is_removed,
			       dbus_bool_t is_added)
{
	GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon;
	volume_monitor_daemon = (GnomeVFSVolumeMonitorDaemon *)
		hal_ctx_get_user_data (ctx);

	if (!is_removed && strcmp (key, "volume.is_mounted") == 0) {
		dbus_bool_t is_mounted;

		is_mounted = hal_device_get_property_bool(
			ctx, udi, "volume.is_mounted");

		if( is_mounted )
			_hal_add_volume (volume_monitor_daemon, udi);
		else
			_hal_volume_unmounted (volume_monitor_daemon, udi);
	}
}

static void 
_hal_device_condition (LibHalContext *ctx, 
		       const char *udi,
		       const char *condition_name,
		       DBusMessage *message)
{
	GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon;
	volume_monitor_daemon = (GnomeVFSVolumeMonitorDaemon *)
		hal_ctx_get_user_data (ctx);

	if (strcmp (condition_name, "BlockForcedUnmountPartition") == 0) {
		_hal_remove_volume (volume_monitor_daemon, udi);
	}
}

static LibHalFunctions
hal_functions = { _hal_mainloop_integration,
		  _hal_device_added,
		  _hal_device_removed,
		  _hal_device_new_capability,
		  _hal_device_lost_capability,
		  _hal_device_property_modified,
		  _hal_device_condition };

gboolean
_gnome_vfs_monitor_hal_mounts_init (GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon)
{
	/* initialise the connection to the hal daemon */
	if ((volume_monitor_daemon->hal_ctx = 
	     hal_initialize (&hal_functions, FALSE)) == NULL) {
		g_warning ("hal_initialize failed\n");
		return FALSE;
	}

	hal_ctx_set_user_data (volume_monitor_daemon->hal_ctx,
			       volume_monitor_daemon);

	hal_device_property_watch_all (volume_monitor_daemon->hal_ctx);

	return TRUE;
}

void
_gnome_vfs_monitor_hal_mounts_shutdown (GnomeVFSVolumeMonitorDaemon *volume_monitor_daemon)
{
	if (hal_shutdown (volume_monitor_daemon->hal_ctx) != 0) {
		g_warning ("hal_shutdown failed\n");
	}
}

#endif /* USE_HAL */
