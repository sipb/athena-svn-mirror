/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo control internal prototypes / helpers
 *
 * Author:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_CONTROL_INTERNAL_H_
#define _BONOBO_CONTROL_INTERNAL_H_

#include <bonobo/bonobo-plug.h>
#include <bonobo/bonobo-socket.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-control-frame.h>

G_BEGIN_DECLS

void     bonobo_control_add_listener            (CORBA_Object        object,
						 GCallback           fn,
						 gpointer            user_data,
						 CORBA_Environment  *ev);

void     bonobo_control_frame_get_remote_window (BonoboControlFrame *frame,
						 CORBA_Environment  *opt_ev);
gboolean bonobo_control_frame_focus             (BonoboControlFrame *frame,
						 GtkDirectionType    direction);
void     bonobo_control_frame_size_request      (BonoboControlFrame *frame,
						 GtkRequisition     *requisition,
						 CORBA_Environment  *opt_ev);
void     bonobo_control_frame_set_inproc_widget (BonoboControlFrame *frame,
						 BonoboPlug         *bonobo_plug,
						 GtkWidget          *control_widget);

BonoboSocket       *bonobo_control_frame_get_socket (BonoboControlFrame *frame);
BonoboControlFrame *bonobo_socket_get_control_frame (BonoboSocket       *socket);
void                bonobo_control_frame_set_socket (BonoboControlFrame *frame,
						     BonoboSocket       *socket);
void                bonobo_socket_set_control_frame (BonoboSocket       *socket,
						     BonoboControlFrame *frame);

BonoboControl      *bonobo_plug_get_control         (BonoboPlug         *plug);
void                bonobo_control_set_plug         (BonoboControl      *control,
						     BonoboPlug         *plug);
void                bonobo_plug_set_control         (BonoboPlug         *plug,
						     BonoboControl      *control);
gboolean            bonobo_socket_disposed          (BonoboSocket       *socket);

G_END_DECLS

#endif /* _BONOBO_CONTROL_INTERNAL_H_ */
