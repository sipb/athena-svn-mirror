/* Icon loading support for the Midnight Commander
 *
 * Copyright (C) 1998-1999 The Free Software Foundation
 *
 * Authors: Miguel de Icaza <miguel@nuclecu.unam.mx>
 *          Federico Mena <federico@nuclecu.unam.mx>
 */

#include "config.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <gnome.h>
#include <libgnomevfs/gnome-vfs-mime.h>
#include <libgnomevfs/gnome-vfs-mime-info.h>
#include "gicon.h"

/* What kinds of images can an icon set contain */
typedef enum {
	ICON_TYPE_PLAIN,
	ICON_TYPE_SYMLINK,
	ICON_TYPE_STALLED
} IconType;

/* Information for an icon set (plain icon plus overlayed versions) */
typedef struct {
	GdkImlibImage *plain;		/* Plain version */
	GdkImlibImage *symlink;		/* Symlink version */
	GdkImlibImage *stalled;		/* Stalled symlink version */
	char *filename;			/* Filename for the plain image */
} IconSet;

static int gicon_inited;		/* Has the icon system been inited? */

static GHashTable *name_hash;		/* Hash from filename -> IconSet */
static GHashTable *image_hash;		/* Hash from imlib image -> IconSet */

static GdkImlibImage *symlink_overlay;	/* The little symlink overlay */
static GdkImlibImage *stalled_overlay;	/* The little stalled symlink overlay */

/* Default icons, guaranteed to exist */
static IconSet *iset_directory;
static IconSet *iset_dirclosed;
static IconSet *iset_executable;
static IconSet *iset_regular;
static IconSet *iset_core;
static IconSet *iset_sock;
static IconSet *iset_fifo;
static IconSet *iset_chardev;
static IconSet *iset_blockdev;

/* Our UID and GID, needed to see if the user can access some files */
static uid_t our_uid;
static gid_t our_gid;


/* Whether we should always use (expensive) metadata lookups for file panels or not */
int we_can_afford_the_speed = 0;


/* Builds a composite of the plain image and the litle symlink icon */
static GdkImlibImage *
build_overlay (GdkImlibImage *plain, GdkImlibImage *overlay)
{
	int rowstride;
	int overlay_rowstride;
	guchar *src, *dest;
	int y;
	GdkImlibImage *im;

	im = gdk_imlib_clone_image (plain);

	rowstride = plain->rgb_width * 3;
	overlay_rowstride = overlay->rgb_width * 3;

	dest = im->rgb_data + ((plain->rgb_height - overlay->rgb_height) * rowstride
			       + (plain->rgb_width - overlay->rgb_width) * 3);

	src = overlay->rgb_data;

	for (y = 0; y < overlay->rgb_height; y++) {
		memcpy (dest, src, overlay_rowstride);

		dest += rowstride;
		src += overlay_rowstride;
	}

	gdk_imlib_changed_image (im);
	return im;
}

/* Ensures that the icon set has the requested image */
static void
ensure_icon_image (IconSet *iset, IconType type)
{
	g_assert (iset->plain != NULL);

	switch (type) {
	case ICON_TYPE_PLAIN:
		/* The plain type always exists, so do nothing */
		break;

	case ICON_TYPE_SYMLINK:
		iset->symlink = build_overlay (iset->plain, symlink_overlay);
		g_hash_table_insert (image_hash, iset->symlink, iset);
		break;

	case ICON_TYPE_STALLED:
		iset->stalled = build_overlay (iset->plain, stalled_overlay);
		g_hash_table_insert (image_hash, iset->stalled, iset);
		break;

	default:
		g_assert_not_reached ();
	}
}

/* Returns the icon set corresponding to the specified icon filename, or NULL if
 * the file could not be loaded.
 */
static IconSet *
get_icon_set (const char *filename)
{
	GdkImlibImage *im;
	IconSet *iset;

	iset = g_hash_table_lookup (name_hash, filename);
	if (iset)
		return iset;

	im = gdk_imlib_load_image ((char *) filename);
	if (!im)
		return NULL;

	iset = g_new (IconSet, 1);
	iset->plain = im;
	iset->symlink = NULL;
	iset->filename = g_strdup (filename);

	/* Insert the icon information into the hash tables */

	g_hash_table_insert (name_hash, iset->filename, iset);
	g_hash_table_insert (image_hash, iset->plain, iset);

	return iset;
}

/* Convenience function to load one of the default icons and die if this fails */
static IconSet *
get_stock_icon (char *name)
{
	char *filename;
	IconSet *iset;

	filename = g_concat_dir_and_file (ICONDIR, name);
	iset = get_icon_set (filename);
	g_free (filename);

	if (!iset)
                /* FIXME bugzilla.eazel.com 1133: Do something useful here.  */
		/*  die_with_no_icons () */ ;

	return iset;
}

/* Convenience function to load one of the default overlays and die if this fails */
static GdkImlibImage *
get_stock_overlay (char *name)
{
	char *filename;
	GdkImlibImage *im;

	filename = g_concat_dir_and_file (ICONDIR, name);
	im = gdk_imlib_load_image (filename);
	g_free (filename);

	if (!im)                /* FIXME bugzilla.eazel.com 1133: Do something useful here. */
                /* die_with_no_icons () */ ;

	return im;
		
}

/**
 * gicon_init:
 * @void: 
 * 
 * Initializes the icon module.
 **/
void
gicon_init (void)
{
	if (gicon_inited)
		return;

	gicon_inited = TRUE;

	name_hash = g_hash_table_new (g_str_hash, g_str_equal);
	image_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	/* Load the default icons */

	iset_directory  = get_stock_icon ("i-directory.png");
	iset_dirclosed  = get_stock_icon ("i-dirclosed.png");
	iset_executable = get_stock_icon ("i-executable.png");
	iset_regular    = get_stock_icon ("i-regular.png");
	iset_core       = get_stock_icon ("i-core.png");
	iset_sock       = get_stock_icon ("i-sock.png");
	iset_fifo       = get_stock_icon ("i-fifo.png");
	iset_chardev    = get_stock_icon ("i-chardev.png");
	iset_blockdev   = get_stock_icon ("i-blockdev.png");

	/* Load the overlay icons */

	symlink_overlay = get_stock_overlay ("i-symlink.png");
	stalled_overlay = get_stock_overlay ("i-stalled.png");

	our_uid = getuid ();
	our_gid = getgid ();
}

/* Tries to get an icon from the file's metadata information */
static GdkImlibImage *
get_icon_from_metadata (const gchar *filename)
{
	int size;
	char *buf;
	GdkImlibImage *im;
	IconSet *iset;

	/* Try the inlined icon */

	if (gnome_metadata_get (filename, "icon-inline-png", &size, &buf) == 0) {
		im = gdk_imlib_inlined_png_to_image (buf, size);
		g_free (buf);

		if (im)
			return im;
	}

	/* Try the non-inlined icon */

	if (gnome_metadata_get (filename, "icon-filename", &size, &buf) == 0) {
		iset = get_icon_set (buf);
		g_free (buf);

		if (iset) {
			ensure_icon_image (iset, ICON_TYPE_PLAIN);
			return iset->plain;
		}
	}

	return NULL; /* nothing is available */
}

/* Returns whether we are in the specified group or not */
static int
we_are_in_group (gid_t gid)
{
	gid_t *groups;
	int ngroups, i;
	int retval;

	if (our_gid == gid)
		return TRUE;

	ngroups = getgroups (0, NULL);
	if (ngroups == -1 || ngroups == 0)
		return FALSE;

	groups = g_new (gid_t, ngroups);
	ngroups = getgroups (ngroups, groups);
	if (ngroups == -1) {
		g_free (groups);
		return FALSE;
	}

	retval = FALSE;

	for (i = 0; i < ngroups; i++)
		if (groups[i] == gid) 
			retval = TRUE;

	g_free (groups);
	return retval;
}

/* Returns whether we can access the contents of directory specified by 
   the stat buf.   */
static int
can_access_directory (const struct stat *stat_buf)
{
	mode_t test_mode;

	if (stat_buf->st_uid == our_uid)
		test_mode = S_IRUSR | S_IXUSR;
	else if (we_are_in_group (stat_buf->st_gid))
		test_mode = S_IRGRP | S_IXGRP;
	else
		test_mode = S_IROTH | S_IXOTH;

	return (stat_buf->st_mode & test_mode) == test_mode;
}

static int is_exe (mode_t mode)
{
    if ((S_IXUSR & mode) || (S_IXGRP & mode) || (S_IXOTH & mode))
	return 1;
    return 0;
}

/* This is the last resort for getting a file's icon.  It uses the file mode
 * bits or a hardcoded name.
 */
static IconSet *
get_default_icon (const gchar *file_name, const struct stat *stat_buf)
{
	if (S_ISSOCK (stat_buf->st_mode))
		return iset_sock;

	if (S_ISCHR (stat_buf->st_mode))
		return iset_chardev;

	if (S_ISBLK (stat_buf->st_mode))
		return iset_blockdev;

	if (S_ISFIFO (stat_buf->st_mode))
		return iset_fifo;

	if (is_exe (stat_buf->st_mode))
		return iset_executable;

#if 0                           /* FIXME bugzilla.eazel.com 1131 */
	if (!strcmp (file_name, "core")
            || !strcmp (extension (file_name), "core"))
		return iset_core;
#endif

	return iset_regular; /* boooo */
}

static GdkImlibImage *
gicon_get_icon_for_file_2 (const gchar *full_name,
                           const struct stat *stat_buf,
                           gboolean do_quick)
{
	IconSet *iset;
	const char *mime_type;

        g_return_val_if_fail (full_name != NULL, NULL);
        g_return_val_if_fail (stat_buf != NULL, NULL);

	gicon_init ();

	/* 1. First try the user's settings */

	if (!do_quick || we_can_afford_the_speed) {
		GdkImlibImage *im;

		im = get_icon_from_metadata (full_name);
		if (im)
			return im;
	}

	/* 2. See if it is a directory */

	if (S_ISDIR (stat_buf->st_mode)) {
		if (can_access_directory (stat_buf))
			iset = iset_directory;
		else
			iset = iset_dirclosed;

		goto add_link;
	}

	/* 3. Try MIME-types */

	mime_type = gnome_vfs_mime_type_from_name (full_name);
	if (mime_type) {
		const char *icon_name;

		icon_name = gnome_vfs_mime_get_value (mime_type, "icon-filename");
		if (icon_name) {
			iset = get_icon_set (icon_name);
			if (iset)
				goto add_link;
		}
	}

	/* 4. Get an icon from the file mode */

	iset = get_default_icon (full_name, stat_buf);

 add_link:

	g_assert (iset != NULL);

#if 0
	if (S_ISLNK (stat_buf->st_mode)) {
		if (fe->f.link_to_dir)
			iset = iset_directory;

		if (fe->f.stalled_link) {
			ensure_icon_image (iset, ICON_TYPE_STALLED);
			return iset->stalled;
		} else {
			ensure_icon_image (iset, ICON_TYPE_SYMLINK);
			return iset->symlink;
		}
	} else {
		ensure_icon_image (iset, ICON_TYPE_PLAIN);
		return iset->plain;
	}
#else
        /* FIXME bugzilla.eazel.com 1131: We have to provide the
	 * link_to_dir, stalled_link and stuff.
	 */
	if (S_ISLNK (stat_buf->st_mode)) {
		ensure_icon_image (iset, ICON_TYPE_SYMLINK);
		return iset->symlink;
	} else {
		ensure_icon_image (iset, ICON_TYPE_PLAIN);
		return iset->plain;
	}
#endif
}

/**
 * gicon_get_icon_for_file:
 * @file_name: The file name.x
 * @do_quick: Whether the function should try to use (expensive) metadata information.
 * 
 * Returns the appropriate icon for the specified file.
 * 
 * Return value: The icon for the specified file.
 **/
GdkImlibImage *
gicon_get_icon_for_file (const gchar *full_name, gboolean do_quick)
{
        struct stat stat_buf;

	g_return_val_if_fail (iset_regular != NULL, NULL);

	return iset_regular->plain;

        if (stat (full_name, &stat_buf) < 0) {
                perror(full_name);
                return NULL;
        }

        return gicon_get_icon_for_file_2 (full_name, &stat_buf, do_quick);
}

/**
 * gicon_get_filename_for_icon:
 * @image: An icon image loaded by the icon module.
 * 
 * Queries the icon database for the icon filename that corresponds to the
 * specified image.
 * 
 * Return value: The filename that contains the icon for the specified image.
 **/
const char *
gicon_get_filename_for_icon (GdkImlibImage *image)
{
	IconSet *iset;

	g_return_val_if_fail (image != NULL, NULL);

	gicon_init ();

	iset = g_hash_table_lookup (image_hash, image);
	g_assert (iset != NULL);
	return iset->filename;
}

GdkImlibImage *
gicon_get_directory_icon (void)
{
        gicon_init ();

        if (iset_directory == NULL)
                return NULL;

        return iset_directory->plain;
}
