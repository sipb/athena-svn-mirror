/*
 * echo.c: Implements a Bonobo Echo server
 *
 * Author:
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * This file is here to show what are the basic steps into create a Bonobo Component.
 */
#include <config.h>
#include <bonobo.h>

/*
 * This pulls the CORBA definitions for the Demo::Echo server
 */
#include "Echo.h"

/*
 * This pulls the definition for the BonoboObject (Gtk Type)
 */
#include "echo.h"

/*
 * A pointer to our parent object class
 */
static BonoboObjectClass *echo_parent_class;

/*
 * The VEPV for the Demo Echo objecg
 */
static POA_Demo_Echo__vepv echo_vepv;

/*
 * Implemented GtkObject::destroy
 */
static void
echo_object_destroy (GtkObject *object)
{
	Echo *echo = ECHO (object);

	g_free (echo->instance_data);
	
	GTK_OBJECT_CLASS (echo_parent_class)->destroy (object);
}

/*
 * CORBA Demo::Echo::echo method implementation
 */
static void
impl_demo_echo_echo (PortableServer_Servant  servant,
		     const CORBA_char       *string,
		     CORBA_Environment      *ev)
{
	Echo *echo = ECHO (bonobo_object_from_servant (servant));
									 
	printf ("Echo message received: %s (echo instance data: %s)\n", string,
		echo->instance_data);
}

/*
 * If you want users to derive classes from your implementation
 * you need to support this method.
 */
POA_Demo_Echo__epv *
echo_get_epv (void)
{
	POA_Demo_Echo__epv *epv;

	epv = g_new0 (POA_Demo_Echo__epv, 1);

	/*
	 * This is the method invoked by CORBA
	 */
	epv->echo = impl_demo_echo_echo;

	return epv;
}

static void
init_echo_corba_class (void)
{
	echo_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	echo_vepv.Demo_Echo_epv      = echo_get_epv ();
}

static void
echo_class_init (EchoClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	echo_parent_class = gtk_type_class (bonobo_object_get_type ());

	object_class->destroy = echo_object_destroy;

	init_echo_corba_class ();
}

GtkType
echo_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"Echo",
			sizeof (Echo),
			sizeof (EchoClass),
			(GtkClassInitFunc) echo_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

Echo *
echo_construct (Echo *echo, Demo_Echo corba_echo)
{
	static int i;
	
	g_return_val_if_fail (echo != NULL, NULL);
	g_return_val_if_fail (IS_ECHO (echo), NULL);
	g_return_val_if_fail (corba_echo != CORBA_OBJECT_NIL, NULL);

	/*
	 * Call parent constructor
	 */
	if (!bonobo_object_construct (BONOBO_OBJECT (echo), (CORBA_Object) corba_echo))
		return NULL;

	/*
	 * Initialize echo
	 */
	echo->instance_data = g_strdup_printf ("Hello %d!", i++);
	
	/*
	 * Success: return the GtkType we were given
	 */
	return echo;
}

/*
 * This routine creates the ORBit CORBA server and initializes the
 * CORBA side of things
 */
static Demo_Echo
create_echo (BonoboObject *echo)
{
	POA_Demo_Echo *servant;
	CORBA_Environment ev;

	servant = (POA_Demo_Echo *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &echo_vepv;

	CORBA_exception_init (&ev);
	POA_Demo_Echo__init ((PortableServer_Servant) servant, &ev);
	ORBIT_OBJECT_KEY(servant->_private)->object = NULL;

	if (ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	/*
	 * Activates the CORBA object & binds to the servant
	 */
	return (Demo_Echo) bonobo_object_activate_servant (echo, servant);
}

Echo *
echo_new (void)
{
	Echo *echo;
	Demo_Echo corba_echo;

	echo = gtk_type_new (echo_get_type ());

	corba_echo = create_echo (BONOBO_OBJECT (echo));

	if (corba_echo == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (echo));
		return NULL;
	}
	
	return echo_construct (echo, corba_echo);
}
