
#include <glib.h>
#include <string.h>
#include "cd-drive.h"

static char*
add_desc (char *string, const char *addition)
{
	char *new;

	if (strcmp (string, "") == 0) {
		new = g_strdup_printf ("%s", addition);
	} else {
		new = g_strdup_printf ("%s/%s", string, addition);
	}
	g_free (string);
	return new;
}

static char*
drive_type (int type)
{
	char *string;

	string = g_strdup ("");
	if (type & CDDRIVE_TYPE_FILE) {
		string = add_desc (string, "File");
	}
	if (type & CDDRIVE_TYPE_CD_RECORDER) {
		string = add_desc (string, "CD Recorder");
	}
	if (type & CDDRIVE_TYPE_DVD_RAM_RECORDER) {
		string = add_desc (string, "DVD-RAM");
	}
	if (type & CDDRIVE_TYPE_DVD_RW_RECORDER) {
		string = add_desc (string, "DVD-RW");
	}
	if (type & CDDRIVE_TYPE_DVD_PLUS_R_RECORDER) {
		string = add_desc (string, "DVD+R");
	}
	if (type & CDDRIVE_TYPE_DVD_PLUS_RW_RECORDER) {
		string = add_desc (string, "DVD+RW");
	}
	if (type & CDDRIVE_TYPE_CD_DRIVE) {
		string = add_desc (string, "CD Drive");
	}
	if (type & CDDRIVE_TYPE_DVD_DRIVE) {
		string = add_desc (string, "DVD Drive");
	}

	return string;
}

static const char*
media_type_get_string (int type)
{
	switch (type) {
	case CD_MEDIA_TYPE_BUSY:
		return "Unknown media, CD drive is busy";
	case CD_MEDIA_TYPE_ERROR:
		return "Couldn't open media";
	case CD_MEDIA_TYPE_UNKNOWN:
		return "Unknown Media";
	case CD_MEDIA_TYPE_CD:
		return "Commercial CD or Audio CD";
	case CD_MEDIA_TYPE_CDR:
		return "CD-R";
	case CD_MEDIA_TYPE_CDRW:
		return "CD-RW";
	case CD_MEDIA_TYPE_DVD:
		return "DVD";
	case CD_MEDIA_TYPE_DVDR:
		return "DVD-R, or DVD-RAM";
	case CD_MEDIA_TYPE_DVDRW:
		return "DVD-RW";
	case CD_MEDIA_TYPE_DVD_RAM:
		return "DVD-RAM";
	case CD_MEDIA_TYPE_DVD_PLUS_R:
		return "DVD+R";
	case CD_MEDIA_TYPE_DVD_PLUS_RW:
		return "DVD+RW";
	}

	return "Broken media type";
}

static void
list_cdroms (void)
{
	GList *cdroms, *l;
	CDDrive *cd;

	cdroms = scan_for_cdroms (FALSE, FALSE);

	for (l = cdroms; l != NULL; l = l->next)
	{
		char *type_str;
		const char *media;
		int media_type;
		gint64 size;

		cd = l->data;
		type_str = drive_type (cd->type);
		media_type = cd_drive_get_media_type (cd);
		media = media_type_get_string (media_type);

		g_print ("name: %s device: %s max_speed_read: %d\n",
				cd->display_name, cd->device,
				cd->max_speed_read);
		g_print ("type: %s\n", type_str);
		g_print ("media type: %s\n", media);
		size = cd_drive_get_media_size (cd);
		g_print ("media size: %0.2f megs", (float) size / 1024 / 1024);
		if (media_type == CD_MEDIA_TYPE_CDR || media_type == CD_MEDIA_TYPE_CDRW) {
			g_print ("approx. or %d mins %d secs)\n", SIZE_TO_TIME(size) / 60, SIZE_TO_TIME(size) % 60);
		} else {
			g_print ("\n");
		}
		g_print ("CD-Recorder/SCSI devices only: max_speed_write: %d"
				" id: %s\n", cd->max_speed_write,
				cd->cdrecord_id);
		g_print ("\n");
		g_free (type_str);
		cd_drive_free (cd);
	}
	g_list_free (cdroms);
}

int main (int argc, char **argv)
{
	list_cdroms ();

	return 0;
}

