/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-storage.c: Storage manipulation.
 *
 * Authors:
 *   Miguel de Icaza (miguel@gnu.org)
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gmodule.h>

#include <bonobo/bonobo-storage.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-storage-plugin.h>

static BonoboObjectClass *bonobo_storage_parent_class;

static POA_Bonobo_Storage__vepv bonobo_storage_vepv;

#define CLASS(o) BONOBO_STORAGE_CLASS (GTK_OBJECT(o)->klass)

static inline BonoboStorage *
bonobo_storage_from_servant (PortableServer_Servant servant)
{
	return BONOBO_STORAGE (bonobo_object_from_servant (servant));
}

static Bonobo_StorageInfo*
impl_Bonobo_Storage_getInfo (PortableServer_Servant servant,
			     const CORBA_char * path,
			     const Bonobo_StorageInfoFields mask,
			     CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	return CLASS (storage)->get_info (storage, path, mask, ev);
}

static void          
impl_Bonobo_Storage_setInfo (PortableServer_Servant servant,
			     const CORBA_char * path,
			     const Bonobo_StorageInfo *info,
			     const Bonobo_StorageInfoFields mask,
			     CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	CLASS (storage)->set_info (storage, path, info, mask, ev);
}

static Bonobo_Stream
impl_Bonobo_Storage_openStream (PortableServer_Servant servant,
				const CORBA_char       *path,
				Bonobo_Storage_OpenMode mode,
				CORBA_Environment      *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	BonoboStream *stream;
	
	if ((stream = CLASS (storage)->open_stream (storage, path, mode, ev)))
		return (Bonobo_Stream) CORBA_Object_duplicate (
			bonobo_object_corba_objref (BONOBO_OBJECT (stream)), ev);
	else
		return CORBA_OBJECT_NIL;
}

static Bonobo_Storage
impl_Bonobo_Storage_openStorage (PortableServer_Servant  servant,
				 const CORBA_char       *path,
				 Bonobo_Storage_OpenMode mode,
				 CORBA_Environment      *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	BonoboStorage *open_storage;
	
	if ((open_storage = CLASS (storage)->open_storage (storage, path, 
							   mode, ev)))
		return (Bonobo_Storage) CORBA_Object_duplicate (
			bonobo_object_corba_objref (BONOBO_OBJECT (open_storage)), ev);
	else
		return CORBA_OBJECT_NIL;
}

static void
impl_Bonobo_Storage_copyTo (PortableServer_Servant servant,
			    Bonobo_Storage         target,
			    CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	Bonobo_Storage src = BONOBO_OBJECT (storage)->corba_objref;	

	if (CLASS (storage)->copy_to)
		CLASS (storage)->copy_to (storage, target, ev);
        else
		bonobo_storage_copy_to (src, target, ev);
}

static void
impl_Bonobo_Storage_rename (PortableServer_Servant servant,
			    const CORBA_char      *path_name,
			    const CORBA_char      *new_path_name,
			    CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->rename (storage, path_name, new_path_name, ev);
}

static void
impl_Bonobo_Storage_commit (PortableServer_Servant servant, 
			    CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->commit (storage, ev);
}

static void
impl_Bonobo_Storage_revert (PortableServer_Servant servant, 
			    CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->commit (storage, ev);
}

static Bonobo_Storage_DirectoryList *
impl_Bonobo_Storage_listContents (PortableServer_Servant servant,
				  const CORBA_char      *path,
				  Bonobo_StorageInfoFields mask,
				  CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	return CLASS (storage)->list_contents (storage, path, mask, ev);
}

static void
impl_Bonobo_Storage_erase (PortableServer_Servant servant, 
			   const CORBA_char      *path,
			   CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->erase (storage, path, ev);
}

/**
 * bonobo_storage_get_epv:
 *
 * Returns: The EPV for the default BonoboStorage implementation.  
 */
POA_Bonobo_Storage__epv *
bonobo_storage_get_epv (void)
{
	POA_Bonobo_Storage__epv *epv;

	epv = g_new0 (POA_Bonobo_Storage__epv, 1);

	epv->getInfo      = impl_Bonobo_Storage_getInfo;
	epv->setInfo      = impl_Bonobo_Storage_setInfo;
	epv->openStream	  = impl_Bonobo_Storage_openStream;
	epv->openStorage  = impl_Bonobo_Storage_openStorage;
	epv->copyTo       = impl_Bonobo_Storage_copyTo;
	epv->rename       = impl_Bonobo_Storage_rename;
	epv->commit       = impl_Bonobo_Storage_commit;
	epv->revert       = impl_Bonobo_Storage_revert;
	epv->listContents = impl_Bonobo_Storage_listContents;
	epv->erase        = impl_Bonobo_Storage_erase;

	return epv;
}

static void
init_storage_corba_class (void)
{
	/* The VEPV */
	bonobo_storage_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_storage_vepv.Bonobo_Storage_epv = bonobo_storage_get_epv ();
}

static void
bonobo_storage_class_init (BonoboStorageClass *klass)
{
	bonobo_storage_parent_class = gtk_type_class (bonobo_object_get_type ());

	init_storage_corba_class ();
}

static void
bonobo_storage_init (BonoboObject *object)
{
}

/**
 * bonobo_storage_get_type:
 *
 * Returns: The GtkType for the BonoboStorage class.
 */
GtkType
bonobo_storage_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboStorage",
			sizeof (BonoboStorage),
			sizeof (BonoboStorageClass),
			(GtkClassInitFunc) bonobo_storage_class_init,
			(GtkObjectInitFunc) bonobo_storage_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_storage_construct:
 * @storage: The BonoboStorage object to be initialized
 * @corba_storage: The CORBA object for the Bonobo_Storage interface.
 *
 * Initializes @storage using the CORBA interface object specified
 * by @corba_storage.
 *
 * Returns: the initialized BonoboStorage object @storage.
 */
BonoboStorage *
bonobo_storage_construct (BonoboStorage *storage, Bonobo_Storage corba_storage)
{
	g_return_val_if_fail (storage != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_STORAGE (storage), NULL);
	g_return_val_if_fail (corba_storage != CORBA_OBJECT_NIL, NULL);

	bonobo_object_construct (
		BONOBO_OBJECT (storage),
		(CORBA_Object) corba_storage);

	
	return storage;
}

/**
 * bonobo_storage_open:
 * @driver: driver to use for opening.
 * @path: path where the base file resides
 * @flags: Bonobo Storage OpenMode
 * @mode: Unix open(2) mode
 *
 * Opens or creates the file named at @path with the stream driver @driver.
 *
 * @driver is one of: "efs", "vfs" or "fs" for now.
 *
 * Returns: a created BonoboStorage object.
 */
BonoboStorage *
bonobo_storage_open_full (const char *driver, const char *path,
			  gint flags, gint mode,
			  CORBA_Environment *opt_ev)
{
	BonoboStorage *storage = NULL;
	StoragePlugin *p;
	CORBA_Environment ev, *my_ev;
	
	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (!driver || !path)
		CORBA_exception_set (my_ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);

	else if (!(p = bonobo_storage_plugin_find (driver)) ||
		 !p->storage_open)
		CORBA_exception_set (my_ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
	else
		storage = p->storage_open (path, flags, mode, my_ev);

	if (!opt_ev) {
		if (BONOBO_EX (my_ev))
			g_warning ("bonobo_storage_open failed '%s'",
				   bonobo_exception_get_text (my_ev));
		CORBA_exception_free (&ev);
	}

	return storage;
}

BonoboStorage *
bonobo_storage_open (const char *driver, const char *path, gint flags, 
		     gint mode)
{
	return bonobo_storage_open_full (driver, path, flags, mode, NULL);
}

/**
 * bonobo_storage_corba_object_create:
 * @object: the GtkObject that will wrap the CORBA object
 *
 * Creates and activates the CORBA object that is wrapped by the
 * @object BonoboObject.
 *
 * Returns: An activated object reference to the created object
 * or %CORBA_OBJECT_NIL in case of failure.
 */
Bonobo_Storage
bonobo_storage_corba_object_create (BonoboObject *object)
{
        POA_Bonobo_Storage *servant;
        CORBA_Environment ev;

        servant = (POA_Bonobo_Storage *) g_new0 (BonoboObjectServant, 1);
        servant->vepv = &bonobo_storage_vepv;

        CORBA_exception_init (&ev);

        POA_Bonobo_Storage__init ((PortableServer_Servant) servant, &ev);
        if (BONOBO_EX (&ev)){
                g_free (servant);
                CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

        CORBA_exception_free (&ev);
        return (Bonobo_Storage) bonobo_object_activate_servant (object,
								servant);
}

static void
copy_stream (Bonobo_Stream src, Bonobo_Stream dest, CORBA_Environment *ev) 
{
	Bonobo_Stream_iobuf *buf;

	do {
		Bonobo_Stream_read (src, 4096, &buf, ev);
		if (BONOBO_EX (ev)) 
			break;

		if (buf->_length == 0) {
			CORBA_free (buf);
			break;
		}

		Bonobo_Stream_write (dest, buf, ev);
		CORBA_free (buf);
		if (BONOBO_EX (ev)) 
			break;

	} while (1);

	if (BONOBO_EX (ev)) /* we must return a Bonobo_Storage exception */
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);

}

void
bonobo_storage_copy_to (Bonobo_Storage src, Bonobo_Storage dest,
			CORBA_Environment *ev) 
{
	Bonobo_Storage new_src, new_dest;
	Bonobo_Stream src_stream, dest_stream;
	Bonobo_Storage_DirectoryList *list;
	gint i;

	if ((src == CORBA_OBJECT_NIL) || (dest == CORBA_OBJECT_NIL) || !ev) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);
		return;
	}

	list = Bonobo_Storage_listContents (src, "", 
					    Bonobo_FIELD_CONTENT_TYPE |
					    Bonobo_FIELD_TYPE,
					    ev);
	if (BONOBO_EX (ev))
		return;

	for (i = 0; i <list->_length; i++) {

		if (list->_buffer[i].type == Bonobo_STORAGE_TYPE_DIRECTORY) {

			new_dest = Bonobo_Storage_openStorage
				(dest, list->_buffer[i].name, 
				 Bonobo_Storage_CREATE | 
				 Bonobo_Storage_FAILIFEXIST, ev);

			if (BONOBO_EX (ev)) 
				break;

			Bonobo_Storage_setInfo (new_dest, "",
						&list->_buffer[i],
						Bonobo_FIELD_CONTENT_TYPE,
						ev);

			if (BONOBO_EX (ev)) {
				bonobo_object_release_unref (new_dest, NULL);
				break;
			}

			new_src = Bonobo_Storage_openStorage
				(src, list->_buffer[i].name, 
				 Bonobo_Storage_READ, ev);
			
			if (BONOBO_EX (ev)) {
				bonobo_object_release_unref (new_dest, NULL);
				break;
			}

			bonobo_storage_copy_to (new_src, new_dest, ev);
			
			bonobo_object_release_unref (new_src, NULL);
			bonobo_object_release_unref (new_dest, NULL);

			if (BONOBO_EX (ev))
				break;

		} else {
			dest_stream = Bonobo_Storage_openStream 
				(dest, list->_buffer[i].name,
				 Bonobo_Storage_CREATE | 
				 Bonobo_Storage_FAILIFEXIST, ev);

			if (BONOBO_EX (ev))
				break;

			Bonobo_Stream_setInfo (dest_stream,
					       &list->_buffer[i],
					       Bonobo_FIELD_CONTENT_TYPE,
					       ev);

			if (BONOBO_EX (ev)) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
						     ex_Bonobo_Storage_IOError,
						     NULL);
				bonobo_object_release_unref (dest_stream,
							     NULL);
				break;
			}

			src_stream = Bonobo_Storage_openStream 
				(src, list->_buffer[i].name,
				 Bonobo_Storage_READ, ev);

			if (BONOBO_EX (ev)) {
				bonobo_object_release_unref (dest_stream, 
							     NULL);
				break;
			}

			copy_stream (src_stream, dest_stream, ev);

			bonobo_object_release_unref (src_stream, NULL);
			bonobo_object_release_unref (dest_stream, NULL);

			if (BONOBO_EX (ev))
				break;
		}
	}

	CORBA_free (list);
}
