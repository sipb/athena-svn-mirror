/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Bonobo PersistFile
 *
 * Author:
 *   Matt Loper (matt@gnome-support.com)
 *
 * Copyright 1999,2000 Helix Code, Inc.
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-persist-file.h>

/* Parent GTK object class */
static BonoboPersistClass *bonobo_persist_file_parent_class;

/* The CORBA entry point vectors */
POA_Bonobo_PersistFile__vepv bonobo_persist_file_vepv;

static CORBA_char *
impl_get_current_file (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistFile *pfile = BONOBO_PERSIST_FILE (object);

	/* if our persist_file has a filename with any length, return it */
	if (pfile->filename && strlen (pfile->filename))
		return CORBA_string_dup ((CORBA_char*)pfile->filename);
	else
	{
		/* otherwise, raise a `NoCurrentName' exception */
		Bonobo_PersistFile_NoCurrentName *exception;
		exception = Bonobo_PersistFile_NoCurrentName__alloc ();
		
		exception->extension = CORBA_string_dup ("");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PersistFile_NoCurrentName,
				     exception);
		return NULL;
	}	
}


static CORBA_boolean
impl_is_dirty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistFile *pfile = BONOBO_PERSIST_FILE (object);

	return pfile->is_dirty;
}

static void
impl_load (PortableServer_Servant servant,
	   const CORBA_char      *filename,
	   CORBA_Environment     *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistFile *pf = BONOBO_PERSIST_FILE (object);
	int result;
	
	if (pf->load_fn != NULL)
		result = (*pf->load_fn)(pf, filename, ev, pf->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (pf)->klass;
		BonoboPersistFileClass *class = BONOBO_PERSIST_FILE_CLASS (oc);

		if (class->load)
			result = (*class->load)(pf, filename, ev);
		else {
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_NotSupported, NULL);
			return;
		}
			
	}
	if (result != 0) {
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_Bonobo_IOError, NULL);
	}
}

static void
impl_save (PortableServer_Servant servant,
	   const CORBA_char      *filename,
	   CORBA_Environment     *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistFile *pf = BONOBO_PERSIST_FILE (object);
	int result;
	
	if (pf->save_fn != NULL)
		result = (*pf->save_fn)(pf, filename, ev, pf->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (pf)->klass;
		BonoboPersistFileClass *class = BONOBO_PERSIST_FILE_CLASS (oc);

		if (class->save)
			result = (*class->save)(pf, filename, ev);
		else {
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_NotSupported, NULL);
			return;
		}
	}
	
	if (result != 0){
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_Bonobo_Persist_FileNotFound, NULL);
	}
	pf->is_dirty = FALSE;
}

/**
 * bonobo_persist_file_get_epv:
 *
 * Returns: The EPV for the default BonoboPersistFile implementation.  
 */
POA_Bonobo_PersistFile__epv *
bonobo_persist_file_get_epv (void)
{
	POA_Bonobo_PersistFile__epv *epv;

	epv = g_new0 (POA_Bonobo_PersistFile__epv, 1);

	epv->load           = impl_load;
	epv->save           = impl_save;
	epv->isDirty        = impl_is_dirty;
	epv->getCurrentFile = impl_get_current_file;

	return epv;
}

static void
init_persist_file_corba_class (void)
{
	bonobo_persist_file_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_persist_file_vepv.Bonobo_Persist_epv = bonobo_persist_get_epv ();
	bonobo_persist_file_vepv.Bonobo_PersistFile_epv = bonobo_persist_file_get_epv ();
}

static CORBA_char *
bonobo_persist_file_get_current_file (BonoboPersistFile *pf,
				      CORBA_Environment *ev)
{
	if (pf->filename)
		return pf->filename;
	return "";
}

static void
bonobo_persist_file_class_init (BonoboPersistFileClass *klass)
{
	bonobo_persist_file_parent_class = gtk_type_class (bonobo_persist_get_type ());

	/*
	 * Override and initialize methods
	 */

	klass->save = NULL;
	klass->load = NULL;
	klass->get_current_file = bonobo_persist_file_get_current_file;
	
	init_persist_file_corba_class ();
}

static void
bonobo_persist_file_init (BonoboPersistFile *pf)
{
}

GtkType
bonobo_persist_file_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboPersistFile",
			sizeof (BonoboPersistFile),
			sizeof (BonoboPersistFileClass),
			(GtkClassInitFunc) bonobo_persist_file_class_init,
			(GtkObjectInitFunc) bonobo_persist_file_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_persist_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_persist_file_construct:
 * @pf: A BonoboPersistFile
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Initializes the BonoboPersistFile object.  The @load_fn and @save_fn
 * parameters might be NULL.  If this is the case, the load and save 
 * operations are performed by the class load and save methods
 */
BonoboPersistFile *
bonobo_persist_file_construct (BonoboPersistFile *pf,
			      Bonobo_PersistFile corba_pf,
			      BonoboPersistFileIOFn load_fn,
			      BonoboPersistFileIOFn save_fn,
			      void *closure)
{
	g_return_val_if_fail (pf != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PERSIST_FILE (pf), NULL);
	g_return_val_if_fail (corba_pf != CORBA_OBJECT_NIL, NULL);

	bonobo_persist_construct (BONOBO_PERSIST (pf), corba_pf);
	
	pf->load_fn = load_fn;
	pf->save_fn = save_fn;
	pf->closure = closure;
		
	return pf;
}

static Bonobo_PersistFile
create_bonobo_persist_file (BonoboObject *object)
{
	POA_Bonobo_PersistFile *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_PersistFile *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_persist_file_vepv;
	CORBA_exception_init (&ev);
	POA_Bonobo_PersistFile__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return (Bonobo_PersistFile) bonobo_object_activate_servant (object, servant);
}


/**
 * bonobo_persist_file_new:
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Creates a BonoboPersistFile object.  The @load_fn and @save_fn
 * parameters might be NULL.  If this is the case, the load and save 
 * operations are performed by the class load and save methods
 */
BonoboPersistFile *
bonobo_persist_file_new (BonoboPersistFileIOFn load_fn,
			BonoboPersistFileIOFn save_fn,
			void *closure)
{
	BonoboPersistFile *pf;
	Bonobo_PersistFile corba_pf;

	pf = gtk_type_new (bonobo_persist_file_get_type ());
	corba_pf = create_bonobo_persist_file (
		BONOBO_OBJECT (pf));
	if (corba_pf == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (pf));
		return NULL;
	}

	pf->filename = NULL;

	bonobo_persist_file_construct (pf, corba_pf, load_fn, save_fn, closure);

	return pf;
}

