
/* Generated data (by glib-mkenums) */

#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <glib-object.h>

/* enumerations from "gnome-vfs-directory.h" */
GType
gnome_vfs_directory_visit_options_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_DIRECTORY_VISIT_DEFAULT, "GNOME_VFS_DIRECTORY_VISIT_DEFAULT", "default" },
      { GNOME_VFS_DIRECTORY_VISIT_SAMEFS, "GNOME_VFS_DIRECTORY_VISIT_SAMEFS", "samefs" },
      { GNOME_VFS_DIRECTORY_VISIT_LOOPCHECK, "GNOME_VFS_DIRECTORY_VISIT_LOOPCHECK", "loopcheck" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSDirectoryVisitOptions", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-file-info.h" */
GType
gnome_vfs_file_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_FILE_FLAGS_NONE, "GNOME_VFS_FILE_FLAGS_NONE", "none" },
      { GNOME_VFS_FILE_FLAGS_SYMLINK, "GNOME_VFS_FILE_FLAGS_SYMLINK", "symlink" },
      { GNOME_VFS_FILE_FLAGS_LOCAL, "GNOME_VFS_FILE_FLAGS_LOCAL", "local" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSFileFlags", values);
  }
  return etype;
}

GType
gnome_vfs_file_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_FILE_TYPE_UNKNOWN, "GNOME_VFS_FILE_TYPE_UNKNOWN", "unknown" },
      { GNOME_VFS_FILE_TYPE_REGULAR, "GNOME_VFS_FILE_TYPE_REGULAR", "regular" },
      { GNOME_VFS_FILE_TYPE_DIRECTORY, "GNOME_VFS_FILE_TYPE_DIRECTORY", "directory" },
      { GNOME_VFS_FILE_TYPE_FIFO, "GNOME_VFS_FILE_TYPE_FIFO", "fifo" },
      { GNOME_VFS_FILE_TYPE_SOCKET, "GNOME_VFS_FILE_TYPE_SOCKET", "socket" },
      { GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE, "GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE", "character-device" },
      { GNOME_VFS_FILE_TYPE_BLOCK_DEVICE, "GNOME_VFS_FILE_TYPE_BLOCK_DEVICE", "block-device" },
      { GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK, "GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK", "symbolic-link" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSFileType", values);
  }
  return etype;
}

GType
gnome_vfs_file_info_fields_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_FILE_INFO_FIELDS_NONE, "GNOME_VFS_FILE_INFO_FIELDS_NONE", "none" },
      { GNOME_VFS_FILE_INFO_FIELDS_TYPE, "GNOME_VFS_FILE_INFO_FIELDS_TYPE", "type" },
      { GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS, "GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS", "permissions" },
      { GNOME_VFS_FILE_INFO_FIELDS_FLAGS, "GNOME_VFS_FILE_INFO_FIELDS_FLAGS", "flags" },
      { GNOME_VFS_FILE_INFO_FIELDS_DEVICE, "GNOME_VFS_FILE_INFO_FIELDS_DEVICE", "device" },
      { GNOME_VFS_FILE_INFO_FIELDS_INODE, "GNOME_VFS_FILE_INFO_FIELDS_INODE", "inode" },
      { GNOME_VFS_FILE_INFO_FIELDS_LINK_COUNT, "GNOME_VFS_FILE_INFO_FIELDS_LINK_COUNT", "link-count" },
      { GNOME_VFS_FILE_INFO_FIELDS_SIZE, "GNOME_VFS_FILE_INFO_FIELDS_SIZE", "size" },
      { GNOME_VFS_FILE_INFO_FIELDS_BLOCK_COUNT, "GNOME_VFS_FILE_INFO_FIELDS_BLOCK_COUNT", "block-count" },
      { GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE, "GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE", "io-block-size" },
      { GNOME_VFS_FILE_INFO_FIELDS_ATIME, "GNOME_VFS_FILE_INFO_FIELDS_ATIME", "atime" },
      { GNOME_VFS_FILE_INFO_FIELDS_MTIME, "GNOME_VFS_FILE_INFO_FIELDS_MTIME", "mtime" },
      { GNOME_VFS_FILE_INFO_FIELDS_CTIME, "GNOME_VFS_FILE_INFO_FIELDS_CTIME", "ctime" },
      { GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME, "GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME", "symlink-name" },
      { GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE, "GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE", "mime-type" },
      { GNOME_VFS_FILE_INFO_FIELDS_ACCESS, "GNOME_VFS_FILE_INFO_FIELDS_ACCESS", "access" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSFileInfoFields", values);
  }
  return etype;
}

GType
gnome_vfs_file_permissions_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_PERM_SUID, "GNOME_VFS_PERM_SUID", "suid" },
      { GNOME_VFS_PERM_SGID, "GNOME_VFS_PERM_SGID", "sgid" },
      { GNOME_VFS_PERM_STICKY, "GNOME_VFS_PERM_STICKY", "sticky" },
      { GNOME_VFS_PERM_USER_READ, "GNOME_VFS_PERM_USER_READ", "user-read" },
      { GNOME_VFS_PERM_USER_WRITE, "GNOME_VFS_PERM_USER_WRITE", "user-write" },
      { GNOME_VFS_PERM_USER_EXEC, "GNOME_VFS_PERM_USER_EXEC", "user-exec" },
      { GNOME_VFS_PERM_USER_ALL, "GNOME_VFS_PERM_USER_ALL", "user-all" },
      { GNOME_VFS_PERM_GROUP_READ, "GNOME_VFS_PERM_GROUP_READ", "group-read" },
      { GNOME_VFS_PERM_GROUP_WRITE, "GNOME_VFS_PERM_GROUP_WRITE", "group-write" },
      { GNOME_VFS_PERM_GROUP_EXEC, "GNOME_VFS_PERM_GROUP_EXEC", "group-exec" },
      { GNOME_VFS_PERM_GROUP_ALL, "GNOME_VFS_PERM_GROUP_ALL", "group-all" },
      { GNOME_VFS_PERM_OTHER_READ, "GNOME_VFS_PERM_OTHER_READ", "other-read" },
      { GNOME_VFS_PERM_OTHER_WRITE, "GNOME_VFS_PERM_OTHER_WRITE", "other-write" },
      { GNOME_VFS_PERM_OTHER_EXEC, "GNOME_VFS_PERM_OTHER_EXEC", "other-exec" },
      { GNOME_VFS_PERM_OTHER_ALL, "GNOME_VFS_PERM_OTHER_ALL", "other-all" },
      { GNOME_VFS_PERM_ACCESS_READABLE, "GNOME_VFS_PERM_ACCESS_READABLE", "access-readable" },
      { GNOME_VFS_PERM_ACCESS_WRITABLE, "GNOME_VFS_PERM_ACCESS_WRITABLE", "access-writable" },
      { GNOME_VFS_PERM_ACCESS_EXECUTABLE, "GNOME_VFS_PERM_ACCESS_EXECUTABLE", "access-executable" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSFilePermissions", values);
  }
  return etype;
}

GType
gnome_vfs_file_info_options_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_FILE_INFO_DEFAULT, "GNOME_VFS_FILE_INFO_DEFAULT", "default" },
      { GNOME_VFS_FILE_INFO_GET_MIME_TYPE, "GNOME_VFS_FILE_INFO_GET_MIME_TYPE", "get-mime-type" },
      { GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE, "GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE", "force-fast-mime-type" },
      { GNOME_VFS_FILE_INFO_FORCE_SLOW_MIME_TYPE, "GNOME_VFS_FILE_INFO_FORCE_SLOW_MIME_TYPE", "force-slow-mime-type" },
      { GNOME_VFS_FILE_INFO_FOLLOW_LINKS, "GNOME_VFS_FILE_INFO_FOLLOW_LINKS", "follow-links" },
      { GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS, "GNOME_VFS_FILE_INFO_GET_ACCESS_RIGHTS", "get-access-rights" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSFileInfoOptions", values);
  }
  return etype;
}

GType
gnome_vfs_set_file_info_mask_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_SET_FILE_INFO_NONE, "GNOME_VFS_SET_FILE_INFO_NONE", "none" },
      { GNOME_VFS_SET_FILE_INFO_NAME, "GNOME_VFS_SET_FILE_INFO_NAME", "name" },
      { GNOME_VFS_SET_FILE_INFO_PERMISSIONS, "GNOME_VFS_SET_FILE_INFO_PERMISSIONS", "permissions" },
      { GNOME_VFS_SET_FILE_INFO_OWNER, "GNOME_VFS_SET_FILE_INFO_OWNER", "owner" },
      { GNOME_VFS_SET_FILE_INFO_TIME, "GNOME_VFS_SET_FILE_INFO_TIME", "time" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSSetFileInfoMask", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-find-directory.h" */
GType
gnome_vfs_find_directory_kind_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_DIRECTORY_KIND_DESKTOP, "GNOME_VFS_DIRECTORY_KIND_DESKTOP", "desktop" },
      { GNOME_VFS_DIRECTORY_KIND_TRASH, "GNOME_VFS_DIRECTORY_KIND_TRASH", "trash" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSFindDirectoryKind", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-handle.h" */
GType
gnome_vfs_open_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_OPEN_NONE, "GNOME_VFS_OPEN_NONE", "none" },
      { GNOME_VFS_OPEN_READ, "GNOME_VFS_OPEN_READ", "read" },
      { GNOME_VFS_OPEN_WRITE, "GNOME_VFS_OPEN_WRITE", "write" },
      { GNOME_VFS_OPEN_RANDOM, "GNOME_VFS_OPEN_RANDOM", "random" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSOpenMode", values);
  }
  return etype;
}

GType
gnome_vfs_seek_position_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_SEEK_START, "GNOME_VFS_SEEK_START", "start" },
      { GNOME_VFS_SEEK_CURRENT, "GNOME_VFS_SEEK_CURRENT", "current" },
      { GNOME_VFS_SEEK_END, "GNOME_VFS_SEEK_END", "end" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSSeekPosition", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-mime-handlers.h" */
GType
gnome_vfs_mime_action_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_MIME_ACTION_TYPE_NONE, "GNOME_VFS_MIME_ACTION_TYPE_NONE", "none" },
      { GNOME_VFS_MIME_ACTION_TYPE_APPLICATION, "GNOME_VFS_MIME_ACTION_TYPE_APPLICATION", "application" },
      { GNOME_VFS_MIME_ACTION_TYPE_COMPONENT, "GNOME_VFS_MIME_ACTION_TYPE_COMPONENT", "component" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSMimeActionType", values);
  }
  return etype;
}

GType
gnome_vfs_mime_application_argument_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS, "GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS", "uris" },
      { GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS, "GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS", "paths" },
      { GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES, "GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES", "uris-for-non-files" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSMimeApplicationArgumentType", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-mime-utils.h" */
GType
gnome_vfs_mime_equivalence_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_MIME_UNRELATED, "GNOME_VFS_MIME_UNRELATED", "unrelated" },
      { GNOME_VFS_MIME_IDENTICAL, "GNOME_VFS_MIME_IDENTICAL", "identical" },
      { GNOME_VFS_MIME_PARENT, "GNOME_VFS_MIME_PARENT", "parent" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSMimeEquivalence", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-monitor.h" */
GType
gnome_vfs_monitor_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_MONITOR_FILE, "GNOME_VFS_MONITOR_FILE", "file" },
      { GNOME_VFS_MONITOR_DIRECTORY, "GNOME_VFS_MONITOR_DIRECTORY", "directory" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSMonitorType", values);
  }
  return etype;
}

GType
gnome_vfs_monitor_event_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_MONITOR_EVENT_CHANGED, "GNOME_VFS_MONITOR_EVENT_CHANGED", "changed" },
      { GNOME_VFS_MONITOR_EVENT_DELETED, "GNOME_VFS_MONITOR_EVENT_DELETED", "deleted" },
      { GNOME_VFS_MONITOR_EVENT_STARTEXECUTING, "GNOME_VFS_MONITOR_EVENT_STARTEXECUTING", "startexecuting" },
      { GNOME_VFS_MONITOR_EVENT_STOPEXECUTING, "GNOME_VFS_MONITOR_EVENT_STOPEXECUTING", "stopexecuting" },
      { GNOME_VFS_MONITOR_EVENT_CREATED, "GNOME_VFS_MONITOR_EVENT_CREATED", "created" },
      { GNOME_VFS_MONITOR_EVENT_METADATA_CHANGED, "GNOME_VFS_MONITOR_EVENT_METADATA_CHANGED", "metadata-changed" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSMonitorEventType", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-result.h" */
GType
gnome_vfs_result_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_OK, "GNOME_VFS_OK", "ok" },
      { GNOME_VFS_ERROR_NOT_FOUND, "GNOME_VFS_ERROR_NOT_FOUND", "error-not-found" },
      { GNOME_VFS_ERROR_GENERIC, "GNOME_VFS_ERROR_GENERIC", "error-generic" },
      { GNOME_VFS_ERROR_INTERNAL, "GNOME_VFS_ERROR_INTERNAL", "error-internal" },
      { GNOME_VFS_ERROR_BAD_PARAMETERS, "GNOME_VFS_ERROR_BAD_PARAMETERS", "error-bad-parameters" },
      { GNOME_VFS_ERROR_NOT_SUPPORTED, "GNOME_VFS_ERROR_NOT_SUPPORTED", "error-not-supported" },
      { GNOME_VFS_ERROR_IO, "GNOME_VFS_ERROR_IO", "error-io" },
      { GNOME_VFS_ERROR_CORRUPTED_DATA, "GNOME_VFS_ERROR_CORRUPTED_DATA", "error-corrupted-data" },
      { GNOME_VFS_ERROR_WRONG_FORMAT, "GNOME_VFS_ERROR_WRONG_FORMAT", "error-wrong-format" },
      { GNOME_VFS_ERROR_BAD_FILE, "GNOME_VFS_ERROR_BAD_FILE", "error-bad-file" },
      { GNOME_VFS_ERROR_TOO_BIG, "GNOME_VFS_ERROR_TOO_BIG", "error-too-big" },
      { GNOME_VFS_ERROR_NO_SPACE, "GNOME_VFS_ERROR_NO_SPACE", "error-no-space" },
      { GNOME_VFS_ERROR_READ_ONLY, "GNOME_VFS_ERROR_READ_ONLY", "error-read-only" },
      { GNOME_VFS_ERROR_INVALID_URI, "GNOME_VFS_ERROR_INVALID_URI", "error-invalid-uri" },
      { GNOME_VFS_ERROR_NOT_OPEN, "GNOME_VFS_ERROR_NOT_OPEN", "error-not-open" },
      { GNOME_VFS_ERROR_INVALID_OPEN_MODE, "GNOME_VFS_ERROR_INVALID_OPEN_MODE", "error-invalid-open-mode" },
      { GNOME_VFS_ERROR_ACCESS_DENIED, "GNOME_VFS_ERROR_ACCESS_DENIED", "error-access-denied" },
      { GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES, "GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES", "error-too-many-open-files" },
      { GNOME_VFS_ERROR_EOF, "GNOME_VFS_ERROR_EOF", "error-eof" },
      { GNOME_VFS_ERROR_NOT_A_DIRECTORY, "GNOME_VFS_ERROR_NOT_A_DIRECTORY", "error-not-a-directory" },
      { GNOME_VFS_ERROR_IN_PROGRESS, "GNOME_VFS_ERROR_IN_PROGRESS", "error-in-progress" },
      { GNOME_VFS_ERROR_INTERRUPTED, "GNOME_VFS_ERROR_INTERRUPTED", "error-interrupted" },
      { GNOME_VFS_ERROR_FILE_EXISTS, "GNOME_VFS_ERROR_FILE_EXISTS", "error-file-exists" },
      { GNOME_VFS_ERROR_LOOP, "GNOME_VFS_ERROR_LOOP", "error-loop" },
      { GNOME_VFS_ERROR_NOT_PERMITTED, "GNOME_VFS_ERROR_NOT_PERMITTED", "error-not-permitted" },
      { GNOME_VFS_ERROR_IS_DIRECTORY, "GNOME_VFS_ERROR_IS_DIRECTORY", "error-is-directory" },
      { GNOME_VFS_ERROR_NO_MEMORY, "GNOME_VFS_ERROR_NO_MEMORY", "error-no-memory" },
      { GNOME_VFS_ERROR_HOST_NOT_FOUND, "GNOME_VFS_ERROR_HOST_NOT_FOUND", "error-host-not-found" },
      { GNOME_VFS_ERROR_INVALID_HOST_NAME, "GNOME_VFS_ERROR_INVALID_HOST_NAME", "error-invalid-host-name" },
      { GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS, "GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS", "error-host-has-no-address" },
      { GNOME_VFS_ERROR_LOGIN_FAILED, "GNOME_VFS_ERROR_LOGIN_FAILED", "error-login-failed" },
      { GNOME_VFS_ERROR_CANCELLED, "GNOME_VFS_ERROR_CANCELLED", "error-cancelled" },
      { GNOME_VFS_ERROR_DIRECTORY_BUSY, "GNOME_VFS_ERROR_DIRECTORY_BUSY", "error-directory-busy" },
      { GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY, "GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY", "error-directory-not-empty" },
      { GNOME_VFS_ERROR_TOO_MANY_LINKS, "GNOME_VFS_ERROR_TOO_MANY_LINKS", "error-too-many-links" },
      { GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM, "GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM", "error-read-only-file-system" },
      { GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM, "GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM", "error-not-same-file-system" },
      { GNOME_VFS_ERROR_NAME_TOO_LONG, "GNOME_VFS_ERROR_NAME_TOO_LONG", "error-name-too-long" },
      { GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE, "GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE", "error-service-not-available" },
      { GNOME_VFS_ERROR_SERVICE_OBSOLETE, "GNOME_VFS_ERROR_SERVICE_OBSOLETE", "error-service-obsolete" },
      { GNOME_VFS_ERROR_PROTOCOL_ERROR, "GNOME_VFS_ERROR_PROTOCOL_ERROR", "error-protocol-error" },
      { GNOME_VFS_ERROR_NO_MASTER_BROWSER, "GNOME_VFS_ERROR_NO_MASTER_BROWSER", "error-no-master-browser" },
      { GNOME_VFS_ERROR_NO_DEFAULT, "GNOME_VFS_ERROR_NO_DEFAULT", "error-no-default" },
      { GNOME_VFS_ERROR_NO_HANDLER, "GNOME_VFS_ERROR_NO_HANDLER", "error-no-handler" },
      { GNOME_VFS_ERROR_PARSE, "GNOME_VFS_ERROR_PARSE", "error-parse" },
      { GNOME_VFS_ERROR_LAUNCH, "GNOME_VFS_ERROR_LAUNCH", "error-launch" },
      { GNOME_VFS_ERROR_TIMEOUT, "GNOME_VFS_ERROR_TIMEOUT", "error-timeout" },
      { GNOME_VFS_ERROR_NAMESERVER, "GNOME_VFS_ERROR_NAMESERVER", "error-nameserver" },
      { GNOME_VFS_ERROR_LOCKED, "GNOME_VFS_ERROR_LOCKED", "error-locked" },
      { GNOME_VFS_ERROR_DEPRECATED_FUNCTION, "GNOME_VFS_ERROR_DEPRECATED_FUNCTION", "error-deprecated-function" },
      { GNOME_VFS_NUM_ERRORS, "GNOME_VFS_NUM_ERRORS", "num-errors" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSResult", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-standard-callbacks.h" */
GType
gnome_vfs_module_callback_full_authentication_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_PREVIOUS_ATTEMPT_FAILED, "GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_PREVIOUS_ATTEMPT_FAILED", "previous-attempt-failed" },
      { GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_PASSWORD, "GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_PASSWORD", "need-password" },
      { GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_USERNAME, "GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_USERNAME", "need-username" },
      { GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_DOMAIN, "GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_DOMAIN", "need-domain" },
      { GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_SAVING_SUPPORTED, "GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_SAVING_SUPPORTED", "saving-supported" },
      { GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_ANON_SUPPORTED, "GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_ANON_SUPPORTED", "anon-supported" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSModuleCallbackFullAuthenticationFlags", values);
  }
  return etype;
}

GType
gnome_vfs_module_callback_full_authentication_out_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_OUT_ANON_SELECTED, "GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_OUT_ANON_SELECTED", "selected" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSModuleCallbackFullAuthenticationOutFlags", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-utils.h" */
GType
gnome_vfs_make_uri_dirs_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_MAKE_URI_DIR_NONE, "GNOME_VFS_MAKE_URI_DIR_NONE", "none" },
      { GNOME_VFS_MAKE_URI_DIR_HOMEDIR, "GNOME_VFS_MAKE_URI_DIR_HOMEDIR", "homedir" },
      { GNOME_VFS_MAKE_URI_DIR_CURRENT, "GNOME_VFS_MAKE_URI_DIR_CURRENT", "current" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSMakeURIDirs", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-volume.h" */
GType
gnome_vfs_device_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_DEVICE_TYPE_UNKNOWN, "GNOME_VFS_DEVICE_TYPE_UNKNOWN", "unknown" },
      { GNOME_VFS_DEVICE_TYPE_AUDIO_CD, "GNOME_VFS_DEVICE_TYPE_AUDIO_CD", "audio-cd" },
      { GNOME_VFS_DEVICE_TYPE_VIDEO_DVD, "GNOME_VFS_DEVICE_TYPE_VIDEO_DVD", "video-dvd" },
      { GNOME_VFS_DEVICE_TYPE_HARDDRIVE, "GNOME_VFS_DEVICE_TYPE_HARDDRIVE", "harddrive" },
      { GNOME_VFS_DEVICE_TYPE_CDROM, "GNOME_VFS_DEVICE_TYPE_CDROM", "cdrom" },
      { GNOME_VFS_DEVICE_TYPE_FLOPPY, "GNOME_VFS_DEVICE_TYPE_FLOPPY", "floppy" },
      { GNOME_VFS_DEVICE_TYPE_ZIP, "GNOME_VFS_DEVICE_TYPE_ZIP", "zip" },
      { GNOME_VFS_DEVICE_TYPE_JAZ, "GNOME_VFS_DEVICE_TYPE_JAZ", "jaz" },
      { GNOME_VFS_DEVICE_TYPE_NFS, "GNOME_VFS_DEVICE_TYPE_NFS", "nfs" },
      { GNOME_VFS_DEVICE_TYPE_AUTOFS, "GNOME_VFS_DEVICE_TYPE_AUTOFS", "autofs" },
      { GNOME_VFS_DEVICE_TYPE_CAMERA, "GNOME_VFS_DEVICE_TYPE_CAMERA", "camera" },
      { GNOME_VFS_DEVICE_TYPE_MEMORY_STICK, "GNOME_VFS_DEVICE_TYPE_MEMORY_STICK", "memory-stick" },
      { GNOME_VFS_DEVICE_TYPE_SMB, "GNOME_VFS_DEVICE_TYPE_SMB", "smb" },
      { GNOME_VFS_DEVICE_TYPE_APPLE, "GNOME_VFS_DEVICE_TYPE_APPLE", "apple" },
      { GNOME_VFS_DEVICE_TYPE_MUSIC_PLAYER, "GNOME_VFS_DEVICE_TYPE_MUSIC_PLAYER", "music-player" },
      { GNOME_VFS_DEVICE_TYPE_WINDOWS, "GNOME_VFS_DEVICE_TYPE_WINDOWS", "windows" },
      { GNOME_VFS_DEVICE_TYPE_LOOPBACK, "GNOME_VFS_DEVICE_TYPE_LOOPBACK", "loopback" },
      { GNOME_VFS_DEVICE_TYPE_NETWORK, "GNOME_VFS_DEVICE_TYPE_NETWORK", "network" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSDeviceType", values);
  }
  return etype;
}

GType
gnome_vfs_volume_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_VOLUME_TYPE_MOUNTPOINT, "GNOME_VFS_VOLUME_TYPE_MOUNTPOINT", "mountpoint" },
      { GNOME_VFS_VOLUME_TYPE_VFS_MOUNT, "GNOME_VFS_VOLUME_TYPE_VFS_MOUNT", "vfs-mount" },
      { GNOME_VFS_VOLUME_TYPE_CONNECTED_SERVER, "GNOME_VFS_VOLUME_TYPE_CONNECTED_SERVER", "connected-server" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSVolumeType", values);
  }
  return etype;
}


/* enumerations from "gnome-vfs-xfer.h" */
GType
gnome_vfs_xfer_options_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GNOME_VFS_XFER_DEFAULT, "GNOME_VFS_XFER_DEFAULT", "default" },
      { GNOME_VFS_XFER_UNUSED_1, "GNOME_VFS_XFER_UNUSED_1", "unused-1" },
      { GNOME_VFS_XFER_FOLLOW_LINKS, "GNOME_VFS_XFER_FOLLOW_LINKS", "follow-links" },
      { GNOME_VFS_XFER_UNUSED_2, "GNOME_VFS_XFER_UNUSED_2", "unused-2" },
      { GNOME_VFS_XFER_RECURSIVE, "GNOME_VFS_XFER_RECURSIVE", "recursive" },
      { GNOME_VFS_XFER_SAMEFS, "GNOME_VFS_XFER_SAMEFS", "samefs" },
      { GNOME_VFS_XFER_DELETE_ITEMS, "GNOME_VFS_XFER_DELETE_ITEMS", "delete-items" },
      { GNOME_VFS_XFER_EMPTY_DIRECTORIES, "GNOME_VFS_XFER_EMPTY_DIRECTORIES", "empty-directories" },
      { GNOME_VFS_XFER_NEW_UNIQUE_DIRECTORY, "GNOME_VFS_XFER_NEW_UNIQUE_DIRECTORY", "new-unique-directory" },
      { GNOME_VFS_XFER_REMOVESOURCE, "GNOME_VFS_XFER_REMOVESOURCE", "removesource" },
      { GNOME_VFS_XFER_USE_UNIQUE_NAMES, "GNOME_VFS_XFER_USE_UNIQUE_NAMES", "use-unique-names" },
      { GNOME_VFS_XFER_LINK_ITEMS, "GNOME_VFS_XFER_LINK_ITEMS", "link-items" },
      { GNOME_VFS_XFER_FOLLOW_LINKS_RECURSIVE, "GNOME_VFS_XFER_FOLLOW_LINKS_RECURSIVE", "follow-links-recursive" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GnomeVFSXferOptions", values);
  }
  return etype;
}

GType
gnome_vfs_xfer_progress_status_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_XFER_PROGRESS_STATUS_OK, "GNOME_VFS_XFER_PROGRESS_STATUS_OK", "ok" },
      { GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR, "GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR", "vfserror" },
      { GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE, "GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE", "overwrite" },
      { GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE, "GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE", "duplicate" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSXferProgressStatus", values);
  }
  return etype;
}

GType
gnome_vfs_xfer_overwrite_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_XFER_OVERWRITE_MODE_ABORT, "GNOME_VFS_XFER_OVERWRITE_MODE_ABORT", "abort" },
      { GNOME_VFS_XFER_OVERWRITE_MODE_QUERY, "GNOME_VFS_XFER_OVERWRITE_MODE_QUERY", "query" },
      { GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE, "GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE", "replace" },
      { GNOME_VFS_XFER_OVERWRITE_MODE_SKIP, "GNOME_VFS_XFER_OVERWRITE_MODE_SKIP", "skip" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSXferOverwriteMode", values);
  }
  return etype;
}

GType
gnome_vfs_xfer_overwrite_action_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT, "GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT", "abort" },
      { GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE, "GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE", "replace" },
      { GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE_ALL, "GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE_ALL", "replace-all" },
      { GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP, "GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP", "skip" },
      { GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP_ALL, "GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP_ALL", "skip-all" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSXferOverwriteAction", values);
  }
  return etype;
}

GType
gnome_vfs_xfer_error_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_XFER_ERROR_MODE_ABORT, "GNOME_VFS_XFER_ERROR_MODE_ABORT", "abort" },
      { GNOME_VFS_XFER_ERROR_MODE_QUERY, "GNOME_VFS_XFER_ERROR_MODE_QUERY", "query" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSXferErrorMode", values);
  }
  return etype;
}

GType
gnome_vfs_xfer_error_action_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_XFER_ERROR_ACTION_ABORT, "GNOME_VFS_XFER_ERROR_ACTION_ABORT", "abort" },
      { GNOME_VFS_XFER_ERROR_ACTION_RETRY, "GNOME_VFS_XFER_ERROR_ACTION_RETRY", "retry" },
      { GNOME_VFS_XFER_ERROR_ACTION_SKIP, "GNOME_VFS_XFER_ERROR_ACTION_SKIP", "skip" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSXferErrorAction", values);
  }
  return etype;
}

GType
gnome_vfs_xfer_phase_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GNOME_VFS_XFER_PHASE_INITIAL, "GNOME_VFS_XFER_PHASE_INITIAL", "phase-initial" },
      { GNOME_VFS_XFER_CHECKING_DESTINATION, "GNOME_VFS_XFER_CHECKING_DESTINATION", "checking-destination" },
      { GNOME_VFS_XFER_PHASE_COLLECTING, "GNOME_VFS_XFER_PHASE_COLLECTING", "phase-collecting" },
      { GNOME_VFS_XFER_PHASE_READYTOGO, "GNOME_VFS_XFER_PHASE_READYTOGO", "phase-readytogo" },
      { GNOME_VFS_XFER_PHASE_OPENSOURCE, "GNOME_VFS_XFER_PHASE_OPENSOURCE", "phase-opensource" },
      { GNOME_VFS_XFER_PHASE_OPENTARGET, "GNOME_VFS_XFER_PHASE_OPENTARGET", "phase-opentarget" },
      { GNOME_VFS_XFER_PHASE_COPYING, "GNOME_VFS_XFER_PHASE_COPYING", "phase-copying" },
      { GNOME_VFS_XFER_PHASE_MOVING, "GNOME_VFS_XFER_PHASE_MOVING", "phase-moving" },
      { GNOME_VFS_XFER_PHASE_READSOURCE, "GNOME_VFS_XFER_PHASE_READSOURCE", "phase-readsource" },
      { GNOME_VFS_XFER_PHASE_WRITETARGET, "GNOME_VFS_XFER_PHASE_WRITETARGET", "phase-writetarget" },
      { GNOME_VFS_XFER_PHASE_CLOSESOURCE, "GNOME_VFS_XFER_PHASE_CLOSESOURCE", "phase-closesource" },
      { GNOME_VFS_XFER_PHASE_CLOSETARGET, "GNOME_VFS_XFER_PHASE_CLOSETARGET", "phase-closetarget" },
      { GNOME_VFS_XFER_PHASE_DELETESOURCE, "GNOME_VFS_XFER_PHASE_DELETESOURCE", "phase-deletesource" },
      { GNOME_VFS_XFER_PHASE_SETATTRIBUTES, "GNOME_VFS_XFER_PHASE_SETATTRIBUTES", "phase-setattributes" },
      { GNOME_VFS_XFER_PHASE_FILECOMPLETED, "GNOME_VFS_XFER_PHASE_FILECOMPLETED", "phase-filecompleted" },
      { GNOME_VFS_XFER_PHASE_CLEANUP, "GNOME_VFS_XFER_PHASE_CLEANUP", "phase-cleanup" },
      { GNOME_VFS_XFER_PHASE_COMPLETED, "GNOME_VFS_XFER_PHASE_COMPLETED", "phase-completed" },
      { GNOME_VFS_XFER_NUM_PHASES, "GNOME_VFS_XFER_NUM_PHASES", "num-phases" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GnomeVFSXferPhase", values);
  }
  return etype;
}


/* Generated data ends here */

