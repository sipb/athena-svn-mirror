/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef ECHO_H_
#define ECHO_H_

#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define ECHO_TYPE        (echo_get_type ())
#define ECHO(o)          (GTK_CHECK_CAST ((o), ECHO_TYPE, Echo))
#define ECHO_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), ECHO_TYPE, EchoClass))
#define IS_ECHO(o)       (GTK_CHECK_TYPE ((o), ECHO_TYPE))
#define IS_ECHO_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), ECHO_TYPE))

typedef struct {
	BonoboObject parent;

	char *instance_data;
} Echo;

typedef struct {
	BonoboObjectClass parent_class;
} EchoClass;

GtkType   	    echo_get_type  (void);
Echo      	   *echo_construct (Echo *echo, Bonobo_Sample_Echo corba_echo);
Echo      	   *echo_new       (void);

POA_Bonobo_Sample_Echo__epv *echo_get_epv (void);

END_GNOME_DECLS

#endif /* ECHO_H_ */
