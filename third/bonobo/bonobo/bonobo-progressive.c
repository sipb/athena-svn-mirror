/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo::ProgressiveDataSink
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-progressive.h>

/* Parent GTK object class */
static BonoboObjectClass *bonobo_progressive_data_sink_parent_class;

POA_Bonobo_ProgressiveDataSink__vepv bonobo_progressive_data_sink_vepv;

static void
impl_Bonobo_ProgressiveDataSink_start (PortableServer_Servant servant,
	    CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboProgressiveDataSink *psink = BONOBO_PROGRESSIVE_DATA_SINK (object);
	int result;

	if (psink->start_fn != NULL)
		result = (*psink->start_fn) (psink, psink->closure);
	else
	{
		GtkObjectClass *oc = GTK_OBJECT (psink)->klass;
		BonoboProgressiveDataSinkClass *class = BONOBO_PROGRESSIVE_DATA_SINK_CLASS (oc);

		result = (*class->start_fn) (psink);
	}

	if (result != 0)
	{
		g_warning ("FIXME: should report an exception");
	}
	
} /* impl_Bonobo_ProgressiveDataSink_start */

static void
impl_Bonobo_ProgressiveDataSink_end (PortableServer_Servant servant,
	  CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboProgressiveDataSink *psink = BONOBO_PROGRESSIVE_DATA_SINK (object);
	int result;

	if (psink->end_fn != NULL)
		result = (*psink->end_fn) (psink, psink->closure);
	else
	{
		GtkObjectClass *oc = GTK_OBJECT (psink)->klass;
		BonoboProgressiveDataSinkClass *class = BONOBO_PROGRESSIVE_DATA_SINK_CLASS (oc);

		result = (*class->end_fn) (psink);
	}

	if (result != 0)
	{
		g_warning ("FIXME: should report an exception");
	}
	
} /* impl_Bonobo_ProgressiveDataSink_end */

static void
impl_Bonobo_ProgressiveDataSink_addData (PortableServer_Servant                 servant,
					 const Bonobo_ProgressiveDataSink_iobuf *buffer,
					 CORBA_Environment                      *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboProgressiveDataSink *psink = BONOBO_PROGRESSIVE_DATA_SINK (object);
	int result;

	if (psink->add_data_fn != NULL)
		result = (*psink->add_data_fn) (psink,
						buffer,
						psink->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (psink)->klass;
		BonoboProgressiveDataSinkClass *class =
			BONOBO_PROGRESSIVE_DATA_SINK_CLASS (oc);

		result = (*class->add_data_fn) (psink, buffer);
	}

	if (result != 0)
		g_warning ("FIXME: should report an exception");
	
} /* impl_Bonobo_ProgressiveDataSink_add_data */

static void
impl_Bonobo_ProgressiveDataSink_setSize (PortableServer_Servant servant,
					 CORBA_long             count,
					 CORBA_Environment     *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboProgressiveDataSink *psink = BONOBO_PROGRESSIVE_DATA_SINK (object);
	int result;

	if (psink->set_size_fn != NULL)
		result = (*psink->set_size_fn) (psink,
						count,
						psink->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (psink)->klass;
		BonoboProgressiveDataSinkClass *class =
			BONOBO_PROGRESSIVE_DATA_SINK_CLASS (oc);

		result = (*class->set_size_fn) (psink, count);
	}

	if (result != 0)
		g_warning ("FIXME: should report an exception");
} /* impl_Bonobo_ProgressiveDataSink_set_size */

/**
 * bonobo_progressive_get_epv:
 *
 * Returns: The EPV for the BonoboProgressive implementation.  
 */
POA_Bonobo_ProgressiveDataSink__epv *
bonobo_progressive_get_epv (void)
{
	POA_Bonobo_ProgressiveDataSink__epv *epv;

	epv = g_new0 (POA_Bonobo_ProgressiveDataSink__epv, 1);

	epv->start   = impl_Bonobo_ProgressiveDataSink_start;
	epv->end     = impl_Bonobo_ProgressiveDataSink_end;
	epv->addData = impl_Bonobo_ProgressiveDataSink_addData;
	epv->setSize = impl_Bonobo_ProgressiveDataSink_setSize;

	return epv;
}

static void
init_progressive_data_sink_corba_class (void)
{
	/*
	 * Initialize the entry point vector for Bonobo::ProgressiveDataSink.
	 */
	bonobo_progressive_data_sink_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_progressive_data_sink_vepv.Bonobo_ProgressiveDataSink_epv =
		bonobo_progressive_get_epv ();
}

static void
bonobo_progressive_data_sink_destroy (GtkObject *object)
{
}

static int
bonobo_progressive_data_sink_start_end_nop (BonoboProgressiveDataSink *psink)
{
	return 0;
}

static int
bonobo_progressive_data_sink_add_data_nop (BonoboProgressiveDataSink *psink,
					  const Bonobo_ProgressiveDataSink_iobuf *buffer)
{
	return 0;
}

static int
bonobo_progressive_data_sink_set_size_nop (BonoboProgressiveDataSink *psink,
					  const CORBA_long count)
{
	return 0;
}

static void
bonobo_progressive_data_sink_class_init (BonoboProgressiveDataSinkClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	bonobo_progressive_data_sink_parent_class =
		gtk_type_class (bonobo_object_get_type ());

	/*
	 * Override and initialize methods
	 */
	object_class->destroy = bonobo_progressive_data_sink_destroy;

	klass->start_fn    = bonobo_progressive_data_sink_start_end_nop;
	klass->end_fn      = bonobo_progressive_data_sink_start_end_nop;
	klass->add_data_fn = bonobo_progressive_data_sink_add_data_nop;
	klass->set_size_fn = bonobo_progressive_data_sink_set_size_nop;

	init_progressive_data_sink_corba_class ();
}

static void
bonobo_progressive_data_sink_init (BonoboProgressiveDataSink *psink)
{
}

/**
 * bonobo_progressive_data_sink_get_type:
 *
 * Returns: The GtkType for the BonoboProgressiveDataSink class type.
 */
GtkType
bonobo_progressive_data_sink_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboProgressiveDataSink",
			sizeof (BonoboProgressiveDataSink),
			sizeof (BonoboProgressiveDataSinkClass),
			(GtkClassInitFunc) bonobo_progressive_data_sink_class_init,
			(GtkObjectInitFunc) bonobo_progressive_data_sink_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
} 

/**
 * bonobo_progressive_data_sink_construct:
 * @psink: The #BonoboProgressiveDataSink object to initialize.
 * @corba_psink: The CORBA object for the #Bonobo_ProgressiveDataSink interface.
 * @start_fn: A callback which is invoked when the #start method is
 * called on the interface.  The #start method is used to indicate that
 * a new set of data is being loaded into the #Bonobo_ProgressiveDataSink interface.
 * @end_fn: A callback which is invoked when the #end method is called
 * on the interface.  The #end method is called after the entire data transmission
 * is complete.
 * @add_data_fn: A callback which is invoked when the #add_data method is called
 * on the interface.  This method is called whenever new data is available for
 * the current transmission.  The new data is passed as an argument to the method
 * @set_size_fn: A callback which is invoked when the #set_size method is called
 * on the interface.  The #set_size method is used by the caller to specify
 * the total size of the current transmission.  This method may be used
 * at any point after #start has been called.  Objects implementing #Bonobo_ProgressiveDataSink
 * may want to use this to know what percentage of the data has been received.
 * @closure: A closure which pis passed to all the callback functions.
 *
 * This function initializes @psink with the CORBA interface
 * provided in @corba_psink and the callback functions
 * specified by @start_fn, @end_fn, @add_data_fn and @set_size_fn.
 * 
 * Returns: the initialized BonoboProgressiveDataSink object.
 */
BonoboProgressiveDataSink *
bonobo_progressive_data_sink_construct (BonoboProgressiveDataSink *psink,
				       Bonobo_ProgressiveDataSink corba_psink,
				       BonoboProgressiveDataSinkStartFn start_fn,
				       BonoboProgressiveDataSinkEndFn end_fn,
				       BonoboProgressiveDataSinkAddDataFn add_data_fn,
				       BonoboProgressiveDataSinkSetSizeFn set_size_fn,
				       void *closure)
{
	g_return_val_if_fail (psink != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROGRESSIVE_DATA_SINK (psink), NULL);
	g_return_val_if_fail (corba_psink != CORBA_OBJECT_NIL, NULL);

	bonobo_object_construct (BONOBO_OBJECT (psink), corba_psink);

	psink->start_fn = start_fn;
	psink->end_fn = end_fn;
	psink->add_data_fn = add_data_fn;
	psink->set_size_fn = set_size_fn;

	psink->closure = closure;

	return psink;
} 

static Bonobo_ProgressiveDataSink
create_bonobo_progressive_data_sink (BonoboObject *object)
{
	POA_Bonobo_ProgressiveDataSink *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_ProgressiveDataSink *)g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_progressive_data_sink_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_ProgressiveDataSink__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return (Bonobo_ProgressiveDataSink) bonobo_object_activate_servant (object, servant);
} 

/**
 * bonobo_progressive_data_sink_new:
 * @start_fn: A callback which is invoked when the #start method is
 * called on the interface.  The #start method is used to indicate that
 * a new set of data is being loaded into the #Bonobo_ProgressiveDataSink interface.
 * @end_fn: A callback which is invoked when the #end method is called
 * on the interface.  The #end method is called after the entire data transmission
 * is complete.
 * @add_data_fn: A callback which is invoked when the #add_data method is called
 * on the interface.  This method is called whenever new data is available for
 * the current transmission.  The new data is passed as an argument to the method
 * @set_size_fn: A callback which is invoked when the #set_size method is called
 * on the interface.  The #set_size method is used by the caller to specify
 * the total size of the current transmission.  This method may be used
 * at any point after #start has been called.  Objects implementing #Bonobo_ProgressiveDataSink
 * may want to use this to know what percentage of the data has been received.
 * @closure: A closure which pis passed to all the callback functions.
 *
 * This function creates a new BonoboProgressiveDataSink object and the
 * corresponding CORBA interface object.  The new object is
 * initialized with the callback functionss specified by @start_fn,
 * @end_fn, @add_data_fn and @set_size_fn.
 * 
 * Returns: the newly-constructed BonoboProgressiveDataSink object.
 */
BonoboProgressiveDataSink *
bonobo_progressive_data_sink_new (BonoboProgressiveDataSinkStartFn start_fn,
				 BonoboProgressiveDataSinkEndFn end_fn,
				 BonoboProgressiveDataSinkAddDataFn add_data_fn,
				 BonoboProgressiveDataSinkSetSizeFn set_size_fn,
				 void *closure)
{
	BonoboProgressiveDataSink *psink;
	Bonobo_ProgressiveDataSink corba_psink;

	psink = gtk_type_new (bonobo_progressive_data_sink_get_type ());
	corba_psink = create_bonobo_progressive_data_sink (BONOBO_OBJECT (psink));
	if (corba_psink == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (psink));
		return NULL;
	}

	bonobo_progressive_data_sink_construct (psink, corba_psink, start_fn,
					       end_fn, add_data_fn,
					       set_size_fn, closure);

	return psink;
} 
