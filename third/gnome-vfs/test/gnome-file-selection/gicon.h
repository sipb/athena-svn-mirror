/* Icon loading support for the Midnight Commander
 *
 * Copyright (C) 1998-1999 The Free Software Foundation
 *
 * Authors: Miguel de Icaza <miguel@nuclecu.unam.mx>
 *          Federico Mena <federico@nuclecu.unam.mx>
 */

#ifndef GNOME_GICON_H
#define GNOME_GICON_H

#include <glib.h>
#include <gdk_imlib.h>

void gicon_init (void);

GdkImlibImage *gicon_get_icon_for_file (const gchar *file_name,
                                        gboolean do_quick);
const char *gicon_get_filename_for_icon (GdkImlibImage *image);
GdkImlibImage *gicon_get_directory_icon (void);

#endif
