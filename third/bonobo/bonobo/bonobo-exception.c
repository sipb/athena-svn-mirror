/**
 * bonobo-exception.c: a generic exception -> user string converter.
 *
 * Authors:
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include <config.h>
#include <gnome.h>
#include <bonobo/bonobo-exception.h>

typedef enum {
	EXCEPTION_STR,
	EXCEPTION_FN
} ExceptionType;

typedef struct {
	ExceptionType     type;
	char             *repo_id;
	char             *str;
	BonoboExceptionFn fn;
	gpointer          user_data;
	GDestroyNotify    destroy_fn;
} ExceptionHandle;

static GHashTable *bonobo_exceptions = NULL;

static gboolean
except_destroy (gpointer dummy, ExceptionHandle *e, gpointer dummy2)
{
	if (e) {
		if (e->type == EXCEPTION_FN &&
		    e->destroy_fn)
			e->destroy_fn (e->user_data);
		e->destroy_fn = NULL;
		g_free (e->repo_id);
		g_free (e->str);
		g_free (e);
	}
	return TRUE;
}

static void
bonobo_exception_shutdown (void)
{
	g_hash_table_foreach_remove (
		bonobo_exceptions, (GHRFunc) except_destroy, NULL);
	g_hash_table_destroy (bonobo_exceptions);
	bonobo_exceptions = NULL;
}

static GHashTable *
get_hash (void)
{
	if (!bonobo_exceptions) {
		bonobo_exceptions = g_hash_table_new (
			g_str_hash, g_str_equal);
		g_atexit (bonobo_exception_shutdown);
	}

	return bonobo_exceptions;
}

void
bonobo_exception_add_handler_str (const char *repo_id, const char *str)
{
	ExceptionHandle *e;
	GHashTable *hash;

	g_return_if_fail (str != NULL);
	g_return_if_fail (repo_id != NULL);

	hash = get_hash ();

	e = g_new0 (ExceptionHandle, 1);

	e->type = EXCEPTION_STR;
	e->repo_id = g_strdup (repo_id);
	e->str = g_strdup (str);

	g_hash_table_insert (hash, e->repo_id, e);
}

void
bonobo_exception_add_handler_fn (const char *repo_id,
				 BonoboExceptionFn fn,
				 gpointer          user_data,
				 GDestroyNotify    destroy_fn)
{
	ExceptionHandle *e;
	GHashTable *hash;

	g_return_if_fail (fn != NULL);
	g_return_if_fail (repo_id != NULL);

	hash = get_hash ();

	e = g_new0 (ExceptionHandle, 1);

	e->type = EXCEPTION_STR;
	e->repo_id = g_strdup (repo_id);
	e->fn = fn;
	e->user_data = user_data;
	e->destroy_fn = destroy_fn;

	g_hash_table_insert (hash, e->repo_id, e);
}

/**
 * bonobo_exception_get_text:
 * @ev: the corba environment.
 * 
 * Returns a user readable description of the exception, busks
 * something convincing if it is not know.
 * 
 * Return value: a g_malloc'd description; needs freeing as,
 * and when. NULL is never returned.
 **/
char *
bonobo_exception_get_text (CORBA_Environment *ev)
{
	g_return_val_if_fail (ev != NULL, NULL);

	if (!BONOBO_EX (ev))
		return g_strdup (_("Error checking error; no exception"));

	/* Oaf */
/*	if (!strcmp (ev->_repo_id, "IDL:OAF/GeneralError:1.0")) {
		OAF_GeneralError *err = ev->_params;
		
		if (!err || !err->description)
			return g_strdup (_("General oaf error with no description"));
		else
			return g_strdup (err->description);

			}*/

	/* Bonobo::ItemContainer */
	if (!strcmp (ev->_repo_id, ex_Bonobo_ItemContainer_NotFound))
		return g_strdup (_("Object not found"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_ItemContainer_SyntaxError))
		return g_strdup (_("Syntax error in object description"));

	/* Bonobo::Embeddable */
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Embeddable_UserCancelledSave))
		return g_strdup (_("The User canceled the save"));

#if 0
	/* Bonobo::GenericFactory */
	else if (!strcmp (ev->_repo_id, ex_GNOME_ObjectFactory_CannotActivate))
		return g_strdup (_("Cannot activate object from factory"));
#endif

	/* Bonobo::Stream */
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Stream_NoPermission))
		return g_strdup (_("No permission to access stream"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Stream_NotSupported))
		return g_strdup (_("An unsupported stream action was attempted"));
	
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Stream_IOError))
		return g_strdup (_("IO Error on stream"));

	/* Bonobo::Storage */
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Storage_IOError))
		return g_strdup (_("IO Error on storage"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Storage_NameExists))
		return g_strdup (_("Name already exists in storage"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Storage_NotFound))
		return g_strdup (_("Object not found in storage"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Storage_NoPermission))
		return g_strdup (_("No permission to do operation on storage"));
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Storage_NotSupported))
		return g_strdup (_("An unsupported storage action was attempted"));
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Storage_NotStream))
		return g_strdup (_("Object is not a stream"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Storage_NotStorage))
		return g_strdup (_("Object is not a storage"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Storage_NotEmpty))
		return g_strdup (_("Storage is not empty"));

	/* Bonobo::UIContainer */
	else if (!strcmp (ev->_repo_id, ex_Bonobo_UIContainer_MalFormedXML))
		return g_strdup (_("malformed user interface XML description"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_UIContainer_InvalidPath))
		return g_strdup (_("invalid path to XML user interface element"));
		
	/* Bonobo::Persist */
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Persist_WrongDataType))
		return g_strdup (_("incorrect data type"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Persist_FileNotFound))
		return g_strdup (_("stream not found"));

	/* Bonobo::PropertyBag */
	else if (!strcmp (ev->_repo_id, ex_Bonobo_PropertyBag_NotFound))
		return g_strdup (_("property not found"));

	/* Bonobo::Moniker */
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Moniker_InterfaceNotFound))
		return g_strdup (_("Moniker interface cannot be found"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Moniker_TimeOut))
		return g_strdup (_("Moniker activation timed out"));
		
	else if (!strcmp (ev->_repo_id, ex_Bonobo_Moniker_InvalidSyntax))
		return g_strdup (_("Syntax error within moniker"));

	else if (!strcmp (ev->_repo_id, ex_Bonobo_Moniker_UnknownPrefix))
		return g_strdup (_("Moniker has an unknown moniker prefix"));

	else {
		ExceptionHandle *e;
		GHashTable *hash = get_hash ();
		char *str = NULL;

		if ((e = g_hash_table_lookup (hash, ev->_repo_id))) {
			if (e->type == EXCEPTION_STR)
				str = g_strdup (e->str);
			else
				str = e->fn (ev, e->user_data);
		}

		if (str)
			return str;
		else
			return g_strdup_printf (
				"Unknown CORBA exception id: '%s'", ev->_repo_id);
	}
}
