/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Author:
 *   Nat Friedman (nat@ximian.com)
 *
 * Copyright 2000, Ximian, Inc.
 */

#include "config.h"  
#include "pas-backend-file.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <db.h>
#include <sys/stat.h>

#include <e-util/e-db3-utils.h>

#if DB_VERSION_MAJOR != 3 || \
    DB_VERSION_MINOR != 1 || \
    DB_VERSION_PATCH != 17
#error Including wrong DB3.  Need libdb 3.1.17.
#endif

#include <gal/util/e-util.h>
#include <gal/widgets/e-unicode.h>

#include <ebook/e-card-simple.h>
#include <e-util/e-dbhash.h>
#include <e-util/e-db3-utils.h>
#include <libgnome/gnome-i18n.h>

#include "pas-book.h"
#include "pas-card-cursor.h"
#include "pas-backend-card-sexp.h"
#include "pas-backend-summary.h"

#define PAS_BACKEND_FILE_VERSION_NAME "PAS-DB-VERSION"
#define PAS_BACKEND_FILE_VERSION "0.2"

#define PAS_ID_PREFIX "pas-id-"
#define SUMMARY_FLUSH_TIMEOUT 5000

static PASBackendClass *pas_backend_file_parent_class;
typedef struct _PASBackendFileCursorPrivate PASBackendFileCursorPrivate;
typedef struct _PASBackendFileBookView PASBackendFileBookView;
typedef struct _PASBackendFileSearchContext PASBackendFileSearchContext;
typedef struct _PasBackendFileChangeContext PASBackendFileChangeContext;

struct _PASBackendFilePrivate {
	char     *uri;
	char     *filename;
	DB       *file_db;
	EList    *book_views;
	GHashTable *address_lists;
	PASBackendSummary *summary;
};

struct _PASBackendFileCursorPrivate {
	PASBackend *backend;
	PASBook    *book;

	GList      *elements;
	guint32    num_elements;
};

struct _PASBackendFileBookView {
	PASBookView                 *book_view;
	gchar                       *search;
	PASBackendCardSExp          *card_sexp;
	gchar                       *change_id;
	PASBackendFileChangeContext *change_context;
};

struct _PasBackendFileChangeContext {
	DB *db;

	GList *add_cards;
	GList *add_ids;
	GList *mod_cards;
	GList *mod_ids;
	GList *del_ids;
};

static void
string_to_dbt(const char *str, DBT *dbt)
{
	memset (dbt, 0, sizeof (*dbt));
	dbt->data = (void*)str;
	dbt->size = strlen (str) + 1;
}

static void
build_summary (PASBackendFilePrivate *bfpriv)
{
	DB             *db = bfpriv->file_db;
	DBC            *dbc;
	int            db_error;
	DBT  id_dbt, vcard_dbt;

	db_error = db->cursor (db, NULL, &dbc, 0);

	if (db_error != 0) {
		g_warning ("pas_backend_file_build_all_cards_list: error building list\n");
	}

	memset (&vcard_dbt, 0, sizeof (vcard_dbt));
	memset (&id_dbt, 0, sizeof (id_dbt));
	db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_FIRST);

	while (db_error == 0) {

		/* don't include the version in the list of cards */
		if (id_dbt.size != strlen(PAS_BACKEND_FILE_VERSION_NAME) + 1
		    || strcmp (id_dbt.data, PAS_BACKEND_FILE_VERSION_NAME)) {

			pas_backend_summary_add_card (bfpriv->summary, vcard_dbt.data);
		}

		db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_NEXT);

	}
}

static void
do_summary_query (PASBackendFile         *bf,
		  PASBackendFileBookView *view,
		  gboolean                completion_search)
{
	GPtrArray *ids = pas_backend_summary_search (bf->priv->summary, view->search);
	int     db_error = 0;
	GList   *cards = NULL;
	gint    card_count = 0, card_threshold = 20, card_threshold_max = 3000;
	DB      *db = bf->priv->file_db;
	DBT     id_dbt, vcard_dbt;
	int i;

	for (i = 0; i < ids->len; i ++) {
		char *id = g_ptr_array_index (ids, i);
		char *vcard = NULL;

#if SUMMARY_STORES_ENOUGH_INFO
		/* this is disabled for the time being because lists
		   can have more than 3 email addresses and the
		   summary only stores 3. */

		if (completion_search) {
			vcard = pas_backend_summary_get_summary_vcard (bf->priv->summary,
								       id);
		}
		else {
#endif
			string_to_dbt (id, &id_dbt);
			memset (&vcard_dbt, 0, sizeof (vcard_dbt));
				
			db_error = db->get (db, NULL, &id_dbt, &vcard_dbt, 0);

			if (db_error == 0)
				vcard = g_strdup (vcard_dbt.data);
#if SUMMARY_STORES_ENOUGH_INFO
		}
#endif

		if (vcard) {
			cards = g_list_prepend (cards, vcard);
			card_count ++;

			/* If we've accumulated a number of checks, pass them off to the client. */
			if (card_count >= card_threshold) {
				pas_book_view_notify_add (view->book_view, cards);
				/* Clean up the handed-off data. */
				g_list_foreach (cards, (GFunc)g_free, NULL);
				g_list_free (cards);
				cards = NULL;
				card_count = 0;

				/* Yeah, this scheme is overly complicated.  But I like it. */
				if (card_threshold < card_threshold_max) {
					card_threshold = MIN (2*card_threshold, card_threshold_max);
				}
			}
		}
		else
			continue; /* XXX */
	}

	g_ptr_array_free (ids, TRUE);

	if (card_count)
		pas_book_view_notify_add (view->book_view, cards);

	pas_book_view_notify_complete (view->book_view, GNOME_Evolution_Addressbook_BookViewListener_Success);

	g_list_foreach (cards, (GFunc)g_free, NULL);
	g_list_free (cards);
}

static PASBackendFileBookView *
pas_backend_file_book_view_copy(const PASBackendFileBookView *book_view, void *closure)
{
	PASBackendFileBookView *new_book_view;
	new_book_view = g_new (PASBackendFileBookView, 1);
	new_book_view->book_view = book_view->book_view;

	new_book_view->search = g_strdup(book_view->search);
	new_book_view->card_sexp = book_view->card_sexp;
	if (new_book_view->card_sexp)
		g_object_ref(new_book_view->card_sexp);
	
	new_book_view->change_id = g_strdup(book_view->change_id);
	if (book_view->change_context) {
		new_book_view->change_context = g_new(PASBackendFileChangeContext, 1);
		new_book_view->change_context->db = book_view->change_context->db;
		new_book_view->change_context->add_cards = book_view->change_context->add_cards;
		new_book_view->change_context->add_ids = book_view->change_context->add_ids;
		new_book_view->change_context->mod_cards = book_view->change_context->mod_cards;
		new_book_view->change_context->mod_ids = book_view->change_context->mod_ids;
		new_book_view->change_context->del_ids = book_view->change_context->del_ids;
	} else
		new_book_view->change_context = NULL;
	
	return new_book_view;
}

static void
pas_backend_file_book_view_free(PASBackendFileBookView *book_view, void *closure)
{
	g_free(book_view->search);
	if (book_view->card_sexp)
		g_object_unref (book_view->card_sexp);

	g_free(book_view->change_id);
	if (book_view->change_context) {
		g_list_foreach (book_view->change_context->add_cards, (GFunc)g_free, NULL);
		g_list_foreach (book_view->change_context->add_ids, (GFunc)g_free, NULL);
		g_list_foreach (book_view->change_context->mod_cards, (GFunc)g_free, NULL);
		g_list_foreach (book_view->change_context->mod_ids, (GFunc)g_free, NULL);
		g_list_foreach (book_view->change_context->del_ids, (GFunc)g_free, NULL);
		g_list_free (book_view->change_context->add_cards);
		g_list_free (book_view->change_context->add_ids);
		g_list_free (book_view->change_context->mod_cards);
		g_list_free (book_view->change_context->mod_ids);
		g_list_free (book_view->change_context->del_ids);
	}
	g_free(book_view->change_context);

	g_free(book_view);
}

static long
get_length(PASCardCursor *cursor, gpointer data)
{
	PASBackendFileCursorPrivate *cursor_data = (PASBackendFileCursorPrivate *) data;

	return cursor_data->num_elements;
}

static char *
get_nth(PASCardCursor *cursor, long n, gpointer data)
{
	PASBackendFileCursorPrivate *cursor_data = (PASBackendFileCursorPrivate *) data;
	GList *nth_item = g_list_nth(cursor_data->elements, n);

	return g_strdup((char*)nth_item->data);
}

static void
cursor_destroy(gpointer data, GObject *where_object_was)
{
	CORBA_Environment ev;
	GNOME_Evolution_Addressbook_Book corba_book;
	PASBackendFileCursorPrivate *cursor_data = (PASBackendFileCursorPrivate *) data;

	corba_book = bonobo_object_corba_objref(BONOBO_OBJECT(cursor_data->book));

	CORBA_exception_init(&ev);

	GNOME_Evolution_Addressbook_Book_unref(corba_book, &ev);
	
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning("cursor_destroy: Exception unreffing "
			  "corba book.\n");
	}

	CORBA_exception_free(&ev);

	g_list_foreach(cursor_data->elements, (GFunc)g_free, NULL);
	g_list_free (cursor_data->elements);

	g_free(cursor_data);
}

static void
view_destroy(gpointer data, GObject *where_object_was)
{
	PASBook           *book = (PASBook *)data;
	PASBackendFile    *bf;
	EIterator         *iterator;
	gboolean success = FALSE;

	bf = PAS_BACKEND_FILE(pas_book_get_backend(book));
	for (iterator = e_list_get_iterator(bf->priv->book_views); e_iterator_is_valid(iterator); e_iterator_next(iterator)) {
		const PASBackendFileBookView *view = e_iterator_get(iterator);
		if (view->book_view == (PASBookView*)where_object_was) {
			e_iterator_delete(iterator);
			success = TRUE;
			break;
		}
	}
	if (!success)
		g_warning ("Failed to remove from book_views list");
	g_object_unref(iterator);

	bonobo_object_unref(BONOBO_OBJECT(book));
}

static char *
pas_backend_file_create_unique_id (char *vcard)
{
	/* use a 32 counter and the 32 bit timestamp to make an id.
	   it's doubtful 2^32 id's will be created in a second, so we
	   should be okay. */
	static guint c = 0;
	return g_strdup_printf (PAS_ID_PREFIX "%08lX%08X", time(NULL), c++);
}

static gboolean
vcard_matches_search (const PASBackendFileBookView *view, char *vcard_string)
{
	/* If this is not a search context view, it doesn't match be default */
	if (view->card_sexp == NULL)
		return FALSE;

	return pas_backend_card_sexp_match_vcard (view->card_sexp, vcard_string);
}

static gboolean
ecard_matches_search (const PASBackendFileBookView *view, ECard *card)
{
	/* If this is not a search context view, it doesn't match be default */
	if (view->card_sexp == NULL)
		return FALSE;

	return pas_backend_card_sexp_match_ecard (view->card_sexp, card);
}

typedef struct {
	PASBackendFile *bf;
	PASBook *book;
	const PASBackendFileBookView *view;
	DBC    *dbc;

	int    card_count;
	int    card_threshold;
	int    card_threshold_max;
	GList  *cards;

	gboolean done_first;
	gboolean search_needed;
} FileBackendSearchClosure;

static void
free_search_closure (FileBackendSearchClosure *closure)
{
	g_list_foreach (closure->cards, (GFunc)g_free, NULL);
	g_list_free (closure->cards);
	g_free (closure);
}

static gboolean
pas_backend_file_search_timeout (gpointer data)
{
	FileBackendSearchClosure *closure = data;
	int     db_error = 0;
	DBT     id_dbt, vcard_dbt;
	int     file_version_name_len;
	DBC     *dbc = closure->dbc;

	file_version_name_len = strlen (PAS_BACKEND_FILE_VERSION_NAME);

	memset (&id_dbt, 0, sizeof (id_dbt));
	memset (&vcard_dbt, 0, sizeof (vcard_dbt));

	if (closure->done_first) {
		db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_NEXT);
	}
	else {
		db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_FIRST);
		closure->done_first = TRUE;
	}

	while (db_error == 0) {

		/* don't include the version in the list of cards */
		if (strcmp (id_dbt.data, PAS_BACKEND_FILE_VERSION_NAME)) {
			char *vcard_string = vcard_dbt.data;

			/* check if the vcard matches the search sexp */
			if ((!closure->search_needed) || vcard_matches_search (closure->view, vcard_string)) {
				closure->cards = g_list_prepend (closure->cards, g_strdup (vcard_string));
				closure->card_count ++;
			}

			/* If we've accumulated a number of checks, pass them off to the client. */
			if (closure->card_count >= closure->card_threshold) {
				pas_book_view_notify_add (closure->view->book_view, closure->cards);
				/* Clean up the handed-off data. */
				g_list_foreach (closure->cards, (GFunc)g_free, NULL);
				g_list_free (closure->cards);
				closure->cards = NULL;
				closure->card_count = 0;

				/* Yeah, this scheme is overly complicated.  But I like it. */
				if (closure->card_threshold < closure->card_threshold_max) {
					closure->card_threshold = MIN (2*closure->card_threshold, closure->card_threshold_max);
				}

				/* return here, we'll do the next lump in the next callback */
				g_timeout_add (200, pas_backend_file_search_timeout, closure);

				return FALSE;
			}
		}

		db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_NEXT);
	}

	dbc->c_close (dbc);

	if (db_error != DB_NOTFOUND) {
		g_warning ("pas_backend_file_search: error building list\n");
		free_search_closure (closure);
	}

	if (closure->card_count)
		pas_book_view_notify_add (closure->view->book_view, closure->cards);

	pas_book_view_notify_complete (closure->view->book_view, GNOME_Evolution_Addressbook_BookViewListener_Success);

	free_search_closure (closure);

	return FALSE;
}


static void
pas_backend_file_search (PASBackendFile  	      *bf,
			 PASBook         	      *book,
			 const PASBackendFileBookView *cnstview,
			 gboolean                      completion_search)
{
	PASBackendFileBookView *view = (PASBackendFileBookView *)cnstview;
	gboolean search_needed;

	search_needed = TRUE;

	if ( ! strcmp (view->search, "(contains \"x-evolution-any-field\" \"\")"))
		search_needed = FALSE;

	if (search_needed)
		pas_book_view_notify_status_message (view->book_view, _("Searching..."));
	else
		pas_book_view_notify_status_message (view->book_view, _("Loading..."));

	if (view->card_sexp) {
		g_object_unref (view->card_sexp);
		view->card_sexp = NULL;
	}

	view->card_sexp = pas_backend_card_sexp_new (view->search);
	
	if (!view->card_sexp) {
		pas_book_view_notify_complete (view->book_view, GNOME_Evolution_Addressbook_BookViewListener_InvalidQuery);
		return;
	}

	if (pas_backend_summary_is_summary_query (bf->priv->summary, view->search)) {
		do_summary_query (bf, view, completion_search);
	}
	else {
		FileBackendSearchClosure *closure = g_new0 (FileBackendSearchClosure, 1);
		DB  *db = bf->priv->file_db;
		int db_error;

		closure->card_threshold = 20;
		closure->card_threshold_max = 3000;
		closure->search_needed = search_needed;
		closure->view = view;
		closure->bf = bf;
		closure->book = book;

		db_error = db->cursor (db, NULL, &closure->dbc, 0);

		if (db_error != 0) {
			g_warning ("pas_backend_file_search: error building list\n");
		} else {
			g_idle_add (pas_backend_file_search_timeout, closure);
		}
	}
}

static void
pas_backend_file_changes_foreach_key (const char *key, gpointer user_data)
{
	PASBackendFileChangeContext *ctx = user_data;
	DB      *db = ctx->db;
	DBT     id_dbt, vcard_dbt;
	int     db_error = 0;
	
	string_to_dbt (key, &id_dbt);
	memset (&vcard_dbt, 0, sizeof (vcard_dbt));
	db_error = db->get (db, NULL, &id_dbt, &vcard_dbt, 0);
	
	if (db_error != 0) {
		char *id = id_dbt.data;
		
		ctx->del_ids = g_list_append (ctx->del_ids, g_strdup (id));
	}
}

static void
pas_backend_file_changes (PASBackendFile  	      *bf,
			  PASBook         	      *book,
			  const PASBackendFileBookView *cnstview)
{
	int     db_error = 0;
	DBT     id_dbt, vcard_dbt;
	char    *filename;
	EDbHash *ehash;
	GList *i, *v;
	DB      *db = bf->priv->file_db;
	DBC *dbc;
	PASBackendFileBookView *view = (PASBackendFileBookView *)cnstview;
	PASBackendFileChangeContext *ctx = cnstview->change_context;
	char *dirname, *slash;

	memset (&id_dbt, 0, sizeof (id_dbt));
	memset (&vcard_dbt, 0, sizeof (vcard_dbt));

	/* Find the changed ids */
	dirname = g_strdup (bf->priv->filename);
	slash = strrchr (dirname, '/');
	*slash = '\0';

	filename = g_strdup_printf ("%s/%s.db", dirname, view->change_id);
	ehash = e_dbhash_new (filename);
	g_free (filename);
	g_free (dirname);

	db_error = db->cursor (db, NULL, &dbc, 0);

	if (db_error != 0) {
		g_warning ("pas_backend_file_changes: error building list\n");
	} else {
		db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_FIRST);

		while (db_error == 0) {

			/* don't include the version in the list of cards */
			if (id_dbt.size != strlen(PAS_BACKEND_FILE_VERSION_NAME) + 1
			    || strcmp (id_dbt.data, PAS_BACKEND_FILE_VERSION_NAME)) {
				ECard *card;
				char *id = id_dbt.data;
				char *vcard_string;
				
				/* Remove fields the user can't change
				 * and can change without the rest of the
				 * card changing 
				 */
				card = e_card_new (vcard_dbt.data);
				g_object_set (card, "last_use", NULL, "use_score", 0.0, NULL);
				vcard_string = e_card_get_vcard_assume_utf8 (card);
				g_object_unref (card);
				
				/* check what type of change has occurred, if any */
				switch (e_dbhash_compare (ehash, id, vcard_string)) {
				case E_DBHASH_STATUS_SAME:
					break;
				case E_DBHASH_STATUS_NOT_FOUND:
					ctx->add_cards = g_list_append (ctx->add_cards, 
									vcard_string);
					ctx->add_ids = g_list_append (ctx->add_ids, g_strdup(id));
					break;
				case E_DBHASH_STATUS_DIFFERENT:
					ctx->mod_cards = g_list_append (ctx->mod_cards, 
									vcard_string);
					ctx->mod_ids = g_list_append (ctx->mod_ids, g_strdup(id));
					break;
				}
			}

			db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_NEXT);
		}
		dbc->c_close (dbc);
	}

   	e_dbhash_foreach_key (ehash, (EDbHashFunc)pas_backend_file_changes_foreach_key, view->change_context);

	/* Send the changes */
	if (db_error != DB_NOTFOUND) {
		g_warning ("pas_backend_file_changes: error building list\n");
	} else {
  		if (ctx->add_cards != NULL)
  			pas_book_view_notify_add (view->book_view, ctx->add_cards);
		
		if (ctx->mod_cards != NULL)
			pas_book_view_notify_change (view->book_view, ctx->mod_cards);

		for (v = ctx->del_ids; v != NULL; v = v->next){
			char *id = v->data;
			pas_book_view_notify_remove_1 (view->book_view, id);
		}
		
		pas_book_view_notify_complete (view->book_view, GNOME_Evolution_Addressbook_BookViewListener_Success);
	}

	/* Update the hash */
	for (i = ctx->add_ids, v = ctx->add_cards; i != NULL; i = i->next, v = v->next){
		char *id = i->data;
		char *vcard = v->data;

		e_dbhash_add (ehash, id, vcard);
		g_free (i->data);
		g_free (v->data);		
	}	
	for (i = ctx->mod_ids, v = ctx->mod_cards; i != NULL; i = i->next, v = v->next){
		char *id = i->data;
		char *vcard = v->data;

		e_dbhash_add (ehash, id, vcard);
		g_free (i->data);
		g_free (v->data);		
	}	
	for (i = ctx->del_ids; i != NULL; i = i->next){
		char *id = i->data;

		e_dbhash_remove (ehash, id);
		g_free (i->data);
	}

	e_dbhash_write (ehash);
  	e_dbhash_destroy (ehash);
}

static char *
do_create(PASBackend *backend,
	  char       *vcard_req,
	  char      **vcard_ptr)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	DB             *db = bf->priv->file_db;
	DBT            id_dbt, vcard_dbt;
	int            db_error;
	char           *id;
	ECard          *card;
	char           *vcard;
	char           *ret_val;

	id = pas_backend_file_create_unique_id (vcard_req);

	string_to_dbt (id, &id_dbt);
	
	card = e_card_new(vcard_req);
	e_card_set_id(card, id);
	vcard = e_card_get_vcard_assume_utf8(card);

	string_to_dbt (vcard, &vcard_dbt);

	db_error = db->put (db, NULL, &id_dbt, &vcard_dbt, 0);

	if (0 == db_error) {
		db_error = db->sync (db, 0);
		if (db_error != 0)
			g_warning ("db->sync failed.\n");
		ret_val = id;

	}
	else {
		g_free (id);
		ret_val = NULL;
	}

	g_object_unref(card);
	card = NULL;

	if (vcard_ptr && ret_val)
		*vcard_ptr = vcard;
	else
		g_free (vcard);

	return ret_val;
}

static void
pas_backend_file_process_create_card (PASBackend *backend,
				      PASBook    *book,
				      PASCreateCardRequest *req)
{
	char *id;
	char *vcard;
	EIterator *iterator;
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);

	id = do_create(backend, req->vcard, &vcard);
	if (id) {
		for (iterator = e_list_get_iterator(bf->priv->book_views); e_iterator_is_valid(iterator); e_iterator_next(iterator)) {
			const PASBackendFileBookView *view = e_iterator_get(iterator);
			if (vcard_matches_search (view, vcard)) {
				bonobo_object_ref (BONOBO_OBJECT (view->book_view));
				pas_book_view_notify_add_1 (view->book_view, vcard);
				pas_book_view_notify_complete (view->book_view, GNOME_Evolution_Addressbook_BookViewListener_Success);
				bonobo_object_unref (BONOBO_OBJECT (view->book_view));
			}
		}
		g_object_unref(iterator);
		
		pas_book_respond_create (
			book,
			GNOME_Evolution_Addressbook_BookListener_Success,
			id);

		pas_backend_summary_add_card (bf->priv->summary, vcard);

		g_free(vcard);
		g_free(id);
	}
	else {
		/* XXX need a different call status for this case, i
                   think */
		pas_book_respond_create (
				 book,
				 GNOME_Evolution_Addressbook_BookListener_CardNotFound,
				 "");
	}
}

static void
pas_backend_file_process_remove_cards (PASBackend *backend,
				       PASBook    *book,
				       PASRemoveCardsRequest *req)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	DB             *db = bf->priv->file_db;
	DBT            id_dbt, vcard_dbt;
	int            db_error;
	EIterator     *iterator;
	const char    *id;
	GList         *l;
	GList         *removed_cards = NULL;
	GNOME_Evolution_Addressbook_BookListener_CallStatus rv = GNOME_Evolution_Addressbook_BookListener_Success;

	for (l = req->ids; l; l = l->next) {
		id = l->data;

		string_to_dbt (id, &id_dbt);
		memset (&vcard_dbt, 0, sizeof (vcard_dbt));

		db_error = db->get (db, NULL, &id_dbt, &vcard_dbt, 0);
		if (0 != db_error) {
			rv = GNOME_Evolution_Addressbook_BookListener_CardNotFound;
			continue;
		}
	
		db_error = db->del (db, NULL, &id_dbt, 0);
		if (0 != db_error) {
			rv = GNOME_Evolution_Addressbook_BookListener_CardNotFound;
			continue;
		}

		removed_cards = g_list_prepend (removed_cards, e_card_new (vcard_dbt.data));
	}

	/* if we actually removed some, try to sync */
	if (removed_cards) {
		db_error = db->sync (db, 0);
		if (db_error != 0)
			g_warning ("db->sync failed.\n");
	}

	for (iterator = e_list_get_iterator (bf->priv->book_views); e_iterator_is_valid(iterator); e_iterator_next(iterator)) {
		const PASBackendFileBookView *view = e_iterator_get(iterator);
		GList *view_removed = NULL;
		for (l = removed_cards; l; l = l->next) {
			ECard *removed_card = l->data;
			if (ecard_matches_search (view, removed_card)) {
				view_removed = g_list_prepend (view_removed, (char*)e_card_get_id (removed_card));
			}
		}
		if (view_removed) {
			bonobo_object_ref (BONOBO_OBJECT (view->book_view));
			pas_book_view_notify_remove (view->book_view, view_removed);
			pas_book_view_notify_complete (view->book_view, GNOME_Evolution_Addressbook_BookViewListener_Success);
			bonobo_object_unref (BONOBO_OBJECT (view->book_view));
			g_list_free (view_removed);
		}
	}
	g_object_unref(iterator);
	
	pas_book_respond_remove (book, rv);

	for (l = removed_cards; l; l = l->next) {
		ECard *c = l->data;
		pas_backend_summary_remove_card (bf->priv->summary, e_card_get_id (c));
		g_object_unref (c);
	}

	g_list_free (removed_cards);
}

static void
pas_backend_file_process_modify_card (PASBackend *backend,
				      PASBook    *book,
				      PASModifyCardRequest *req)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	DB             *db = bf->priv->file_db;
	DBT            id_dbt, vcard_dbt;
	int            db_error;
	EIterator     *iterator;
	ECard         *card;
	const char    *id, *lookup_id;
	char          *old_vcard_string;

	/* create a new ecard from the request data */
	card = e_card_new(req->vcard);
	id = e_card_get_id(card);

	/* This is disgusting, but for a time cards were added with
           ID's that are no longer used (they contained both the uri
           and the id.) If we recognize it as a uri (file:///...) trim
           off everything before the last '/', and use that as the
           id.*/
	if (!strncmp (id, "file:///", strlen ("file:///"))) {
		lookup_id = strrchr (id, '/') + 1;
	}
	else
		lookup_id = id;

	string_to_dbt (lookup_id, &id_dbt);	
	memset (&vcard_dbt, 0, sizeof (vcard_dbt));

	/* get the old ecard - the one that's presently in the db */
	db_error = db->get (db, NULL, &id_dbt, &vcard_dbt, 0);
	if (0 != db_error) {
		pas_book_respond_modify (
				 book,
				 GNOME_Evolution_Addressbook_BookListener_CardNotFound);
		return;
	}
	old_vcard_string = g_strdup(vcard_dbt.data);

	string_to_dbt (req->vcard, &vcard_dbt);	

	db_error = db->put (db, NULL, &id_dbt, &vcard_dbt, 0);

	if (0 == db_error) {
		db_error = db->sync (db, 0);
		if (db_error != 0)
			g_warning ("db->sync failed.\n");

		for (iterator = e_list_get_iterator(bf->priv->book_views); e_iterator_is_valid(iterator); e_iterator_next(iterator)) {
			CORBA_Environment ev;
			const PASBackendFileBookView *view = e_iterator_get(iterator);
			gboolean old_match, new_match;

			CORBA_exception_init(&ev);

			bonobo_object_dup_ref(bonobo_object_corba_objref(BONOBO_OBJECT(view->book_view)), &ev);

			old_match = vcard_matches_search (view, old_vcard_string);
			new_match = vcard_matches_search (view, req->vcard);
			if (old_match && new_match)
				pas_book_view_notify_change_1 (view->book_view, req->vcard);
			else if (new_match)
				pas_book_view_notify_add_1 (view->book_view, req->vcard);
			else /* if (old_match) */
				pas_book_view_notify_remove_1 (view->book_view, id);

			pas_book_view_notify_complete (view->book_view, GNOME_Evolution_Addressbook_BookViewListener_Success);

			bonobo_object_release_unref(bonobo_object_corba_objref(BONOBO_OBJECT(view->book_view)), &ev);

			CORBA_exception_free (&ev);
		}

		g_object_unref(iterator);

		pas_book_respond_modify (
				 book,
				 GNOME_Evolution_Addressbook_BookListener_Success);

		pas_backend_summary_remove_card (bf->priv->summary, id);
		pas_backend_summary_add_card (bf->priv->summary, req->vcard);
	}
	else {
		pas_book_respond_modify (
				 book,
				 GNOME_Evolution_Addressbook_BookListener_CardNotFound);
	}

	g_free(old_vcard_string);

	g_object_unref(card);
}

static void
pas_backend_file_build_cards_list(PASBackend *backend,
				  PASBackendFileCursorPrivate *cursor_data,
				  char *search)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	DB             *db = bf->priv->file_db;
	DBC            *dbc;
	int            db_error;
	DBT  id_dbt, vcard_dbt;
	PASBackendCardSExp *card_sexp = NULL;
	gboolean search_needed;
	
	cursor_data->elements = NULL;

	search_needed = TRUE;

	if (!strcmp (search, "(contains \"x-evolution-any-field\" \"\")"))
		search_needed = FALSE;

	card_sexp = pas_backend_card_sexp_new (search);
	
	if (!card_sexp)
		g_warning ("pas_backend_file_build_all_cards_list: error building list\n");

	db_error = db->cursor (db, NULL, &dbc, 0);

	if (db_error != 0) {
		g_warning ("pas_backend_file_build_all_cards_list: error building list\n");
	}

	memset (&vcard_dbt, 0, sizeof (vcard_dbt));
	memset (&id_dbt, 0, sizeof (id_dbt));
	db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_FIRST);

	while (db_error == 0) {

		/* don't include the version in the list of cards */
		if (id_dbt.size != strlen(PAS_BACKEND_FILE_VERSION_NAME) + 1
		    || strcmp (id_dbt.data, PAS_BACKEND_FILE_VERSION_NAME)) {

			if ((!search_needed) || (card_sexp != NULL && pas_backend_card_sexp_match_vcard  (card_sexp, vcard_dbt.data))) {
				cursor_data->elements = g_list_prepend (cursor_data->elements, g_strdup (vcard_dbt.data));
			}
		}

		db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_NEXT);

	}

	if (db_error != DB_NOTFOUND) {
		g_warning ("pas_backend_file_build_all_cards_list: error building list\n");
	}
	else {
		cursor_data->num_elements = g_list_length (cursor_data->elements);
		cursor_data->elements = g_list_reverse (cursor_data->elements);
	}
}

static void
pas_backend_file_process_get_vcard (PASBackend *backend,
				    PASBook    *book,
				    PASGetVCardRequest *req)
{
	PASBackendFile *bf;
	DB             *db;
	DBT             id_dbt, vcard_dbt;
	int             db_error = 0;
	char           *card;
	GNOME_Evolution_Addressbook_BookListener_CallStatus status;	

	bf = PAS_BACKEND_FILE (pas_book_get_backend (book));
	db = bf->priv->file_db;

	string_to_dbt (req->id, &id_dbt);
	memset (&vcard_dbt, 0, sizeof (vcard_dbt));

	db_error = db->get (db, NULL, &id_dbt, &vcard_dbt, 0);

	if (db_error == 0) {
		card = vcard_dbt.data;
		status = GNOME_Evolution_Addressbook_BookListener_Success;
	} else {
		card = "";
		status = GNOME_Evolution_Addressbook_BookListener_CardNotFound;
	}

	pas_book_respond_get_vcard (book,
				    status,
				    card);
}

static void
pas_backend_file_process_get_cursor (PASBackend *backend,
				     PASBook    *book,
				     PASGetCursorRequest *req)
{
	/*
	  PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	  DB             *db = bf->priv->file_db;
	  DBT            id_dbt, vcard_dbt;
	*/
	CORBA_Environment ev;
	int            db_error = 0;
	PASBackendFileCursorPrivate *cursor_data;
	PASCardCursor *cursor;
	GNOME_Evolution_Addressbook_Book corba_book;

	cursor_data = g_new(PASBackendFileCursorPrivate, 1);
	cursor_data->backend = backend;
	cursor_data->book = book;

	pas_backend_file_build_cards_list(backend, cursor_data, req->search);

	corba_book = bonobo_object_corba_objref(BONOBO_OBJECT(book));

	CORBA_exception_init(&ev);

	GNOME_Evolution_Addressbook_Book_ref(corba_book, &ev);
	
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning("pas_backend_file_process_get_cursor: Exception reffing "
			  "corba book.\n");
	}

	CORBA_exception_free(&ev);
	
	cursor = pas_card_cursor_new(get_length,
				     get_nth,
				     cursor_data);

	g_object_weak_ref (G_OBJECT (cursor),
			   cursor_destroy, cursor_data);
	
	pas_book_respond_get_cursor (
		book,
		(db_error == 0 
		 ? GNOME_Evolution_Addressbook_BookListener_Success 
		 : GNOME_Evolution_Addressbook_BookListener_CardNotFound),
		cursor);
}

static void
pas_backend_file_process_get_book_view (PASBackend *backend,
					PASBook    *book,
					PASGetBookViewRequest *req)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	PASBookView       *book_view;
	PASBackendFileBookView view;
	EIterator *iterator;

	bonobo_object_ref(BONOBO_OBJECT(book));

	book_view = pas_book_view_new (req->listener);

	g_object_weak_ref (G_OBJECT (book_view), view_destroy, book);

	view.book_view = book_view;
	view.search = g_strdup (req->search);
	view.card_sexp = NULL;
	view.change_id = NULL;
	view.change_context = NULL;	

	e_list_append(bf->priv->book_views, &view);

	pas_book_respond_get_book_view (book,
		   (book_view != NULL
		    ? GNOME_Evolution_Addressbook_BookListener_Success 
		    : GNOME_Evolution_Addressbook_BookListener_CardNotFound /* XXX */),
		   book_view);

	if (!pas_backend_is_loaded (backend))
		return;

	iterator = e_list_get_iterator(bf->priv->book_views);
	e_iterator_last(iterator);
	pas_backend_file_search (bf, book, e_iterator_get(iterator), FALSE);
	g_object_unref(iterator);
}

static void
pas_backend_file_process_get_completion_view (PASBackend *backend,
					      PASBook    *book,
					      PASGetCompletionViewRequest *req)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	PASBookView       *book_view;
	PASBackendFileBookView view;
	EIterator *iterator;

	bonobo_object_ref(BONOBO_OBJECT(book));

	book_view = pas_book_view_new (req->listener);

	g_object_weak_ref (G_OBJECT (book_view), view_destroy, book);

	view.book_view = book_view;
	view.search = g_strdup (req->search);
	view.card_sexp = NULL;
	view.change_id = NULL;
	view.change_context = NULL;	

	e_list_append(bf->priv->book_views, &view);

	pas_book_respond_get_completion_view (book,
		   (book_view != NULL
		    ? GNOME_Evolution_Addressbook_BookListener_Success 
		    : GNOME_Evolution_Addressbook_BookListener_CardNotFound /* XXX */),
		   book_view);

	if (!pas_backend_is_loaded (backend))
		return;

	iterator = e_list_get_iterator(bf->priv->book_views);
	e_iterator_last(iterator);
	pas_backend_file_search (bf, book, e_iterator_get(iterator), TRUE);
	g_object_unref(iterator);
}

static void
pas_backend_file_process_get_changes (PASBackend *backend,
				      PASBook    *book,
				      PASGetChangesRequest *req)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	PASBookView       *book_view;
	PASBackendFileBookView view;
	PASBackendFileChangeContext ctx;
	EIterator *iterator;

	bonobo_object_ref(BONOBO_OBJECT(book));

	book_view = pas_book_view_new (req->listener);

	g_object_weak_ref (G_OBJECT (book_view), view_destroy, book);

	pas_book_respond_get_changes (book,
		   (book_view != NULL
		    ? GNOME_Evolution_Addressbook_BookListener_Success 
		    : GNOME_Evolution_Addressbook_BookListener_CardNotFound /* XXX */),
		   book_view);

	view.book_view = book_view;
	view.change_id = req->change_id;
	view.change_context = &ctx;
	ctx.db = bf->priv->file_db;
	ctx.add_cards = NULL;
	ctx.add_ids = NULL;
	ctx.mod_cards = NULL;
	ctx.mod_ids = NULL;
	ctx.del_ids = NULL;
	view.search = NULL;
	view.card_sexp = NULL;
	
	e_list_append(bf->priv->book_views, &view);

	if (!pas_backend_is_loaded (backend))
		return;

	iterator = e_list_get_iterator(bf->priv->book_views);
	e_iterator_last(iterator);
	pas_backend_file_changes (bf, book, e_iterator_get(iterator));
	g_object_unref(iterator);
}

static void
pas_backend_file_process_check_connection (PASBackend *backend,
					   PASBook    *book,
					   PASCheckConnectionRequest *req)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);

	pas_book_report_connection (book, bf->priv->file_db != NULL);
}

static char *
pas_backend_file_extract_path_from_uri (const char *uri)
{
	g_assert (strncasecmp (uri, "file:", 5) == 0);

	return g_strdup (uri + 5);
}

static void
pas_backend_file_process_authenticate_user (PASBackend *backend,
					    PASBook    *book,
					    PASAuthenticateUserRequest *req)
{
	pas_book_respond_authenticate_user (book,
					    GNOME_Evolution_Addressbook_BookListener_Success);
}

static void
pas_backend_file_process_get_supported_fields (PASBackend *backend,
					       PASBook    *book,
					       PASGetSupportedFieldsRequest *req)
{
	EList *fields = e_list_new ((EListCopyFunc)g_strdup, (EListFreeFunc)g_free, NULL);
	ECardSimple *simple;
	ECard *card;
	int i;

	/* we support everything, so instantiate an e-card, and loop
           through all fields, adding their ecard_fields. */

	card = e_card_new ("");
	simple = e_card_simple_new (card);

	for (i = 0; i < E_CARD_SIMPLE_FIELD_LAST; i ++)
		e_list_append (fields, e_card_simple_get_ecard_field (simple, i));

	g_object_unref (card);
	g_object_unref (simple);

	pas_book_respond_get_supported_fields (book,
					       GNOME_Evolution_Addressbook_BookListener_Success,
					       fields);
}

/*
** versions:
**
** 0.0 just a list of cards
**
** 0.1 same as 0.0, but with the version tag
**
** 0.2 not a real format upgrade, just a hack to fix broken ids caused
**     by a bug in early betas, but we only need to convert them if
**     the previous version is 0.1, since the bug existed after 0.1
**     came about.
*/
static gboolean
pas_backend_file_upgrade_db (PASBackendFile *bf, char *old_version)
{
	DB  *db = bf->priv->file_db;
	int db_error;
	DBT version_name_dbt, version_dbt;
	
	if (strcmp (old_version, "0.0")
	    && strcmp (old_version, "0.1")) {
		g_warning ("unsupported version '%s' found in PAS backend file\n",
			   old_version);
		return FALSE;
	}

	if (!strcmp (old_version, "0.1")) {
		/* we just loop through all the cards in the db,
                   giving them valid ids if they don't have them */
		DBT  id_dbt, vcard_dbt;
		DBC *dbc;
		int  card_failed = 0;

		db_error = db->cursor (db, NULL, &dbc, 0);
		if (db_error != 0) {
			g_warning ("unable to get cursor");
			return FALSE;
		}

		memset (&id_dbt, 0, sizeof (id_dbt));
		memset (&vcard_dbt, 0, sizeof (vcard_dbt));

		db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_FIRST);

		while (db_error == 0) {
			if (id_dbt.size != strlen(PAS_BACKEND_FILE_VERSION_NAME) + 1
			    || strcmp (id_dbt.data, PAS_BACKEND_FILE_VERSION_NAME)) {
				ECard *card;

				card = e_card_new (vcard_dbt.data);

				/* the cards we're looking for are
				   created with a normal id dbt, but
				   with the id field in the vcard set
				   to something that doesn't match.
				   so, we need to modify the card to
				   have the same id as the the dbt. */
				if (strcmp (id_dbt.data, e_card_get_id (card))) {
					char *vcard;

					e_card_set_id (card, id_dbt.data);

					vcard = e_card_get_vcard (card);
					string_to_dbt (vcard, &vcard_dbt);

					db_error = db->put (db, NULL,
							    &id_dbt, &vcard_dbt, 0);

					g_free (vcard);

					if (db_error != 0)
						card_failed++;
				}

				g_object_unref (card);
			}

			db_error = dbc->c_get(dbc, &id_dbt, &vcard_dbt, DB_NEXT);
		}

		if (card_failed) {
			g_warning ("failed to update %d cards\n", card_failed);
			return FALSE;
		}
	}

	string_to_dbt (PAS_BACKEND_FILE_VERSION_NAME, &version_name_dbt);
	string_to_dbt (PAS_BACKEND_FILE_VERSION, &version_dbt);

	db_error = db->put (db, NULL, &version_name_dbt, &version_dbt, 0);
	if (db_error == 0)
		return TRUE;
	else
		return FALSE;
}

static gboolean
pas_backend_file_maybe_upgrade_db (PASBackendFile *bf)
{
	DB   *db = bf->priv->file_db;
	DBT  version_name_dbt, version_dbt;
	int  db_error;
	char *version;
	gboolean ret_val = TRUE;

	string_to_dbt (PAS_BACKEND_FILE_VERSION_NAME, &version_name_dbt);
	memset (&version_dbt, 0, sizeof (version_dbt));

	db_error = db->get (db, NULL, &version_name_dbt, &version_dbt, 0);
	if (db_error == 0) {
		/* success */
		version = g_strdup (version_dbt.data);
	}
	else {
		/* key was not in file */
		version = g_strdup ("0.0");
	}

	if (strcmp (version, PAS_BACKEND_FILE_VERSION))
		ret_val = pas_backend_file_upgrade_db (bf, version);

	g_free (version);

	return ret_val;
}

#define INITIAL_VCARD "BEGIN:VCARD\n\
X-EVOLUTION-FILE-AS:Ximian, Inc.\n\
LABEL;WORK;QUOTED-PRINTABLE:401 Park Drive  3 West=0ABoston, MA 02215=0AUSA\n\
TEL;WORK;VOICE:(617) 375-3800\n\
TEL;WORK;FAX:(617) 236-8630\n\
EMAIL;INTERNET:hello@ximian.com\n\
URL:www.ximian.com/\n\
ORG:Ximian, Inc.;\n\
NOTE:Welcome to the Ximian Addressbook.\n\
END:VCARD"

static GNOME_Evolution_Addressbook_BookListener_CallStatus
pas_backend_file_load_uri (PASBackend             *backend,
			   const char             *uri)
{
	PASBackendFile *bf = PAS_BACKEND_FILE (backend);
	char           *filename;
	gboolean        writable = FALSE;
	int             db_error;
	DB *db;
	int major, minor, patch;
	time_t db_mtime;
	struct stat sb;
	char *summary_filename;

	g_free(bf->priv->uri);
	bf->priv->uri = g_strdup (uri);

	db_version (&major, &minor, &patch);

	if (major != 3 ||
	    minor != 1 ||
	    patch != 17) {
		g_warning ("Wrong version of libdb.");
		return GNOME_Evolution_Addressbook_BookListener_OtherError;
	}

	filename = pas_backend_file_extract_path_from_uri (uri);

	db_error = e_db3_utils_maybe_recover (filename);
	if (db_error != 0)
		return GNOME_Evolution_Addressbook_BookListener_OtherError;

	db_error = db_create (&db, NULL, 0);
	if (db_error != 0)
		return GNOME_Evolution_Addressbook_BookListener_OtherError;

	db_error = db->open (db, filename, NULL, DB_HASH, 0, 0666);

	if (db_error == DB_OLD_VERSION) {
		db_error = e_db3_utils_upgrade_format (filename);

		if (db_error != 0)
			return GNOME_Evolution_Addressbook_BookListener_OtherError;

		db_error = db->open (db, filename, NULL, DB_HASH, 0, 0666);
	}

	bf->priv->file_db = db;

	if (db_error == 0) {
		writable = TRUE;
	} else {
		db_error = db->open (db, filename, NULL, DB_HASH, DB_RDONLY, 0666);

		if (db_error != 0) {
			db_error = db->open (db, filename, NULL, DB_HASH, DB_CREATE, 0666);

			if (db_error == 0) {
				char *create_initial_file;
				char *dir;

				dir = g_path_get_dirname(filename);
				create_initial_file = g_build_filename (dir, "create-initial", NULL);

				if (g_file_test(create_initial_file, G_FILE_TEST_EXISTS)) {
					char *id;
					id = do_create(backend, INITIAL_VCARD, NULL);
					g_free (id);
				}

				g_free(create_initial_file);
				g_free(dir);

				writable = TRUE;
			}
		}
	}

	if (db_error != 0) {
		bf->priv->file_db = NULL;
		return GNOME_Evolution_Addressbook_BookListener_OtherError;
	}

	if (!pas_backend_file_maybe_upgrade_db (bf)) {
		db->close (db, 0);
		bf->priv->file_db = NULL;
		return GNOME_Evolution_Addressbook_BookListener_OtherError;
	}

	g_free (bf->priv->filename);
	bf->priv->filename = filename;

	if (stat (bf->priv->filename, &sb) == -1) {
		db->close (db, 0);
		bf->priv->file_db = NULL;
		return GNOME_Evolution_Addressbook_BookListener_OtherError;
	}
	db_mtime = sb.st_mtime;

	summary_filename = g_strconcat (bf->priv->filename, ".summary", NULL);
	bf->priv->summary = pas_backend_summary_new (summary_filename, SUMMARY_FLUSH_TIMEOUT);
	g_free (summary_filename);

	if (pas_backend_summary_is_up_to_date (bf->priv->summary, db_mtime) == FALSE
	    || pas_backend_summary_load (bf->priv->summary) == FALSE ) {
		build_summary (bf->priv);
	}

	pas_backend_set_is_loaded (backend, TRUE);
	pas_backend_set_is_writable (backend, writable);
	return GNOME_Evolution_Addressbook_BookListener_Success;
}

/* Get_uri handler for the addressbook file backend */
static const char *
pas_backend_file_get_uri (PASBackend *backend)
{
	PASBackendFile *bf;

	bf = PAS_BACKEND_FILE (backend);

	g_assert (bf->priv->uri != NULL);

	return bf->priv->uri;
}

static char *
pas_backend_file_get_static_capabilities (PASBackend *backend)
{
	return g_strdup("local,do-initial-query,bulk-removes");
}

static gboolean
pas_backend_file_construct (PASBackendFile *backend)
{
	g_assert (backend != NULL);
	g_assert (PAS_IS_BACKEND_FILE (backend));

	if (! pas_backend_construct (PAS_BACKEND (backend)))
		return FALSE;

	return TRUE;
}

/**
 * pas_backend_file_new:
 */
PASBackend *
pas_backend_file_new (void)
{
	PASBackendFile *backend;

	backend = g_object_new (PAS_TYPE_BACKEND_FILE, NULL);

	if (! pas_backend_file_construct (backend)) {
		g_object_unref (backend);

		return NULL;
	}

	return PAS_BACKEND (backend);
}

static void
pas_backend_file_dispose (GObject *object)
{
	PASBackendFile *bf;

	bf = PAS_BACKEND_FILE (object);

	if (bf->priv) {
		g_object_unref(bf->priv->book_views);
		if (bf->priv->summary)
			g_object_unref(bf->priv->summary);
		g_free (bf->priv->uri);
		g_free (bf->priv->filename);

		g_free (bf->priv);
		bf->priv = NULL;
	}

	G_OBJECT_CLASS (pas_backend_file_parent_class)->dispose (object);	
}

static void
pas_backend_file_class_init (PASBackendFileClass *klass)
{
	GObjectClass    *object_class = G_OBJECT_CLASS (klass);
	PASBackendClass *parent_class;

	pas_backend_file_parent_class = g_type_class_peek_parent (klass);

	parent_class = PAS_BACKEND_CLASS (klass);

	/* Set the virtual methods. */
	parent_class->load_uri                = pas_backend_file_load_uri;
	parent_class->get_uri                 = pas_backend_file_get_uri;
	parent_class->get_static_capabilities = pas_backend_file_get_static_capabilities;

	parent_class->create_card             = pas_backend_file_process_create_card;
	parent_class->remove_cards            = pas_backend_file_process_remove_cards;
	parent_class->modify_card             = pas_backend_file_process_modify_card;
	parent_class->check_connection        = pas_backend_file_process_check_connection;
	parent_class->get_vcard               = pas_backend_file_process_get_vcard;
	parent_class->get_cursor              = pas_backend_file_process_get_cursor;
	parent_class->get_book_view           = pas_backend_file_process_get_book_view;
	parent_class->get_completion_view     = pas_backend_file_process_get_completion_view;
	parent_class->get_changes             = pas_backend_file_process_get_changes;
	parent_class->authenticate_user       = pas_backend_file_process_authenticate_user;
	parent_class->get_supported_fields    = pas_backend_file_process_get_supported_fields;

	object_class->dispose = pas_backend_file_dispose;
}

static void
pas_backend_file_init (PASBackendFile *backend)
{
	PASBackendFilePrivate *priv;

	priv             = g_new0 (PASBackendFilePrivate, 1);
	priv->book_views = e_list_new((EListCopyFunc) pas_backend_file_book_view_copy, (EListFreeFunc) pas_backend_file_book_view_free, NULL);
	priv->uri        = NULL;

	backend->priv = priv;
}

/**
 * pas_backend_file_get_type:
 */
GType
pas_backend_file_get_type (void)
{
	static GType type = 0;

	if (! type) {
		GTypeInfo info = {
			sizeof (PASBackendFileClass),
			NULL, /* base_class_init */
			NULL, /* base_class_finalize */
			(GClassInitFunc)  pas_backend_file_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (PASBackendFile),
			0,    /* n_preallocs */
			(GInstanceInitFunc) pas_backend_file_init
		};

		type = g_type_register_static (PAS_TYPE_BACKEND, "PASBackendFile", &info, 0);
	}

	return type;
}