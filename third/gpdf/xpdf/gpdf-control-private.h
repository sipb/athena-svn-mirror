/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * gpdf bonobo control
 *
 * Author:
 *   Remi Cohen-Scali <rcoscali@cvs.gnome.org> 
 *
 * Copyright 2003 Remi Cohen-Scali
 */

#ifndef GPDF_CONTROL_PRIVATE_H
#define GPDF_CONTROL_PRIVATE_H

#include "gpdf-control.h"

G_BEGIN_DECLS

/* Set the status label without changing widget state */
void 		gpdf_control_private_set_status 	(GPdfControl * control,
                                                         const gchar * status);
/* Push a status text in status bar */
void		gpdf_control_private_push		(GPdfControl * control,
                                                         const gchar * status);

/* Pop a text from status stack */
void 		gpdf_control_private_pop		(GPdfControl * control);

/* Empty status stack */
void 		gpdf_control_private_clear_stack	(GPdfControl * control);

/* Set fraction on status progess bar */
void 		gpdf_control_private_set_fraction	(GPdfControl * control,
                                                         double        fraction);

/* Set pulse step value when progress used in pluse mode */
void		gpdf_control_private_set_pulse_step 	(GPdfControl * control,
                                                         gdouble       fraction);
/* Pulse progress one step */
void		gpdf_control_private_pulse 		(GPdfControl * control);

/* Refresh current state of stack/default. */
void 		gpdf_control_private_refresh		(GPdfControl * control);

/* Set cursor for whole component */
void		gpdf_control_private_set_cursor		(GPdfControl * control,
                                                         GdkCursor   * cursor);
/* Set wait cursor for whole component */
void		gpdf_control_private_set_wait_cursor	(GPdfControl * control);

/* Reset to default cursor for whole component */
void		gpdf_control_private_reset_cursor	(GPdfControl * control);

/* Get bookmarks view */
GtkWidget*	gpdf_control_private_get_bookmarks_view	(GPdfControl * control);

/* Get thumbnails view */
GtkWidget*	gpdf_control_private_get_thumbnails_view(GPdfControl * control);

/* Get annotations view */
GtkWidget*	gpdf_control_private_get_annots_view	(GPdfControl * control);

/* Raise an error dialog */
void		gpdf_control_private_error_dialog 	(GPdfControl * control,
							 const gchar *header_text,
							 const gchar *body_text,
							 gboolean modal,
							 gboolean ref_parent);
/* Raise a warning dialog */
void		gpdf_control_private_warn_dialog 	(GPdfControl * control,
							 const gchar *header_text,
							 const gchar *body_text,
							 gboolean modal);
/* Raise an info dialog */
void		gpdf_control_private_info_dialog 	(GPdfControl * control,
							 const gchar *header_text,
							 const gchar *body_text,
							 gboolean modal);

/* Display help at a given anchor or section id */
void		gpdf_control_private_display_help	(BonoboControl * control,
							 const gchar *section_id);

G_END_DECLS

#endif /* GPDF_CONTROL_PRIVATE_H */
