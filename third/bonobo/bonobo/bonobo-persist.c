/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-persist.c: a persistance interface
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-persist.h>

/* Parent GTK object class */
static BonoboObjectClass *bonobo_persist_parent_class;

#define CLASS(o) BONOBO_PERSIST_CLASS(GTK_OBJECT(o)->klass)

static inline BonoboPersist *
bonobo_persist_from_servant (PortableServer_Servant servant)
{
	return BONOBO_PERSIST (bonobo_object_from_servant (servant));
}

static Bonobo_Persist_ContentTypeList *
impl_Bonobo_Persist_getContentTypes (PortableServer_Servant servant,
				     CORBA_Environment     *ev)
{
	BonoboPersist *persist = bonobo_persist_from_servant (servant);

	return CLASS (persist)->get_content_types (persist, ev);
}

/**
 * bonobo_persist_get_epv:
 *
 * Returns: The EPV for the default BonoboPersist implementation.  
 */
POA_Bonobo_Persist__epv *
bonobo_persist_get_epv (void)
{
	POA_Bonobo_Persist__epv *epv;

	epv = g_new0 (POA_Bonobo_Persist__epv, 1);

	epv->getContentTypes = impl_Bonobo_Persist_getContentTypes;

	return epv;
}

static void
init_persist_corba_class (void)
{
}

static void
bonobo_persist_destroy (GtkObject *object)
{
	GTK_OBJECT_CLASS (bonobo_persist_parent_class)->destroy (object);
}

static void
bonobo_persist_class_init (BonoboPersistClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	bonobo_persist_parent_class = gtk_type_class (bonobo_object_get_type ());

	/*
	 * Override and initialize methods
	 */
	object_class->destroy = bonobo_persist_destroy;

	init_persist_corba_class ();
}

static void
bonobo_persist_init (BonoboPersist *persist)
{
}

BonoboPersist *
bonobo_persist_construct (BonoboPersist *persist,
			  Bonobo_Persist corba_persist)
{
	g_return_val_if_fail (persist != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PERSIST (persist), NULL);
	g_return_val_if_fail (corba_persist != CORBA_OBJECT_NIL, NULL);
	
	bonobo_object_construct (BONOBO_OBJECT (persist), corba_persist);

	return persist;
}

/**
 * bonobo_persist_get_type:
 *
 * Returns: the GtkType for the BonoboPersist class.
 */
GtkType
bonobo_persist_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboPersist",
			sizeof (BonoboPersist),
			sizeof (BonoboPersistClass),
			(GtkClassInitFunc) bonobo_persist_class_init,
			(GtkObjectInitFunc) bonobo_persist_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_persist_generate_content_types:
 * @num: the number of content types specified
 * @...: the content types (as strings)
 *
 * Returns: a ContentTypeList containing the given ContentTypes
 **/
Bonobo_Persist_ContentTypeList *
bonobo_persist_generate_content_types (int num, ...)
{
	Bonobo_Persist_ContentTypeList *types;
	va_list ap;
	char *type;
	int i;

	types = Bonobo_Persist_ContentTypeList__alloc ();
	CORBA_sequence_set_release (types, TRUE);
	types->_length = types->_maximum = num;
	types->_buffer = CORBA_sequence_Bonobo_Persist_ContentType_allocbuf (num);

	va_start (ap, num);
	for (i = 0; i < num; i++) {
		type = va_arg (ap, char *);
		types->_buffer[i] = CORBA_string_alloc (strlen (type) + 1);
		strcpy (types->_buffer[i], type);
	}
	va_end (ap);

	return types;
}
