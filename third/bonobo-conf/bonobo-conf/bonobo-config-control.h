/**
 * bonobo-config-control.h: Property control implementation.
 *
 * Author:
 *      based ob bonobo-property-control.h by Iain Holmes <iain@helixcode.com>
 *      modified by Dietmar Maurer <dietmar@ximian.com>
 *
 * Copyright 2001, Ximian, Inc.
 */
#ifndef _BONOBO_CONFIG_CONTROL_H_
#define _BONOBO_CONFIG_CONTROL_H_

#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-property-control.h>

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_CONTROL_TYPE        (bonobo_config_control_get_type ())
#define BONOBO_CONFIG_CONTROL(o)          (GTK_CHECK_CAST ((o), BONOBO_CONFIG_CONTROL_TYPE, BonoboConfigControl))
#define BONOBO_CONFIG_CONTROL_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_CONTROL_TYPE, BonoboConfigControlClass))
#define BONOBO_IS_CONFIG_CONTROL(o)       (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_CONTROL_TYPE))
#define BONOBO_IS_CONFIG_CONTROL_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_CONTROL_TYPE))

typedef struct _BonoboConfigControl        BonoboConfigControl;
typedef struct _BonoboConfigControlPrivate BonoboConfigControlPrivate;

typedef GtkWidget *(* BonoboConfigControlGetControlFn) 
     (BonoboConfigControl *control,
      Bonobo_PropertyBag   pb,
      gpointer             closure,
      CORBA_Environment   *ev);

struct _BonoboConfigControl {
        BonoboXObject		    object;

	BonoboEventSource          *event_source;
	BonoboConfigControlPrivate *priv;
};

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_PropertyControl__epv epv;

	void (* action) (BonoboConfigControl *config_control, 
			 Bonobo_PropertyControl_Action action);
} BonoboConfigControlClass;

GtkType 
bonobo_config_control_get_type (void);

BonoboConfigControl *
bonobo_config_control_new      (BonoboEventSource   *opt_event_source);

void                   
bonobo_config_control_changed  (BonoboConfigControl *config_control,
				CORBA_Environment   *opt_ev);


void
bonobo_config_control_add_page (BonoboConfigControl             *cc,
				const char                      *name,
				Bonobo_PropertyBag               pb,
				BonoboConfigControlGetControlFn  opt_get_fn,
				gpointer                         closure);
				
END_GNOME_DECLS

#endif /* _BONOBO_CONFIG_CONTROL_H_ */

