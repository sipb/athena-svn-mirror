#ifndef GNOME_VFS_MODULE_SHARED_H
#define GNOME_VFS_MODULE_SHARED_H

const gchar *gnome_vfs_mime_type_from_mode (mode_t mode);

void         gnome_vfs_stat_to_file_info   (GnomeVFSFileInfo *file_info,
					    const struct stat *statptr);

GnomeVFSResult gnome_vfs_set_meta          (GnomeVFSFileInfo *info,
					    const gchar *file_name,
					    const gchar *meta_key);
GnomeVFSResult gnome_vfs_set_meta_for_list (GnomeVFSFileInfo *info,
					    const gchar *file_name,
					    const GList *meta_keys);
	
const char    *gnome_vfs_get_special_mime_type (GnomeVFSURI *uri);

#endif
