/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-container.h: The server side CORBA impl. for BonoboWindow.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_UI_CONTAINER_H_
#define _BONOBO_UI_CONTAINER_H_

#include <bonobo/bonobo-win.h>

#define BONOBO_UI_CONTAINER_TYPE        (bonobo_ui_container_get_type ())
#define BONOBO_UI_CONTAINER(o)          (GTK_CHECK_CAST ((o), BONOBO_UI_CONTAINER_TYPE, BonoboUIContainer))
#define BONOBO_UI_CONTAINER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_UI_CONTAINER_TYPE, BonoboUIContainerClass))
#define BONOBO_IS_UI_CONTAINER(o)       (GTK_CHECK_TYPE ((o), BONOBO_UI_CONTAINER_TYPE))
#define BONOBO_IS_UI_CONTAINER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_UI_CONTAINER_TYPE))

typedef struct {
	BonoboObject base;

	int          flags;
	BonoboWindow   *win;
} BonoboUIContainer;

typedef struct {
	BonoboObjectClass parent;
} BonoboUIContainerClass;

GtkType                      bonobo_ui_container_get_type            (void);
POA_Bonobo_UIContainer__epv *bonobo_ui_container_get_epv             (void);
Bonobo_UIContainer           bonobo_ui_container_corba_object_create (BonoboObject       *object);
BonoboUIContainer           *bonobo_ui_container_construct           (BonoboUIContainer  *container,
								      Bonobo_UIContainer  corba_container);

BonoboUIContainer           *bonobo_ui_container_new                 (void);

void                         bonobo_ui_container_set_win             (BonoboUIContainer  *container,
								      BonoboWindow       *win);
BonoboWindow                *bonobo_ui_container_get_win             (BonoboUIContainer  *container);

#endif /* _BONOBO_UI_CONTAINER_H_ */
