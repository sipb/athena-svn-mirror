#ifndef SAVE_BUDDY_H
#define SAVE_BUDDY_H

#include <gtk/gtkwindow.h>

#define BB_SAVE_ERROR (bb_save_error_quark ())
GQuark bb_save_error_quark (void);

#if 0
gboolean bb_write_buffer_to_fd      (GtkWindow *parent, const char *wait_msg, int fd, int pid, const char *buffer, gssize buflen, GError **error);
#endif
gboolean bb_write_buffer_to_file    (GtkWindow *parent, const char *wait_msg, const char *filename, const char *buffer, gssize buflen, GError **error);
gboolean bb_write_buffer_to_command (GtkWindow *parent, const char *wait_msg, char **argv, const char *buffer, gssize buflen, GError **error);

#endif /* SAVE_BUDDY_H */
