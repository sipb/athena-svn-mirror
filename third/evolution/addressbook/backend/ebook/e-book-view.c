/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * The Evolution addressbook client object.
 *
 * Author:
 *   Nat Friedman (nat@ximian.com)
 *
 * Copyright 1999, 2000, Ximian, Inc.
 */

#include <config.h>

#include "addressbook.h"
#include "e-card-cursor.h"
#include "e-book-view-listener.h"
#include "e-book-view.h"
#include "e-book.h"
#include "e-book-marshal.h"

static GObjectClass *parent_class;

struct _EBookViewPrivate {
	GNOME_Evolution_Addressbook_BookView     corba_book_view;

	EBook                 *book;
	
	EBookViewListener     *listener;

	int                    responses_queued_id;
};

enum {
	CARD_CHANGED,
	CARD_REMOVED,
	CARD_ADDED,
	SEQUENCE_COMPLETE,
	STATUS_MESSAGE,
	LAST_SIGNAL
};

static guint e_book_view_signals [LAST_SIGNAL];

static void
add_book_iterator (gpointer data, gpointer closure)
{
	ECard *card = E_CARD (data);
	EBook *book = E_BOOK (closure);

	e_card_set_book (card, book);
}

static void
e_book_view_do_added_event (EBookView                 *book_view,
			    EBookViewListenerResponse *resp)
{
	if (book_view->priv->book)
		g_list_foreach (resp->cards, add_book_iterator, book_view->priv->book);

	g_signal_emit (book_view, e_book_view_signals [CARD_ADDED], 0,
		       resp->cards);

	g_list_foreach (resp->cards, (GFunc) g_object_unref, NULL);
	g_list_free (resp->cards);
}

static void
e_book_view_do_modified_event (EBookView                 *book_view,
			       EBookViewListenerResponse *resp)
{
	if (book_view->priv->book)
		g_list_foreach (resp->cards, add_book_iterator, book_view->priv->book);

	g_signal_emit (book_view, e_book_view_signals [CARD_CHANGED], 0,
		       resp->cards);

	g_list_foreach (resp->cards, (GFunc) g_object_unref, NULL);
	g_list_free (resp->cards);
}

static void
e_book_view_do_removed_event (EBookView                 *book_view,
			      EBookViewListenerResponse *resp)
{
	g_signal_emit (book_view, e_book_view_signals [CARD_REMOVED], 0,
		       resp->ids);

	g_list_foreach (resp->ids, (GFunc) g_free, NULL);
	g_list_free (resp->ids);
}

static void
e_book_view_do_complete_event (EBookView                 *book_view,
			       EBookViewListenerResponse *resp)
{
	g_signal_emit (book_view, e_book_view_signals [SEQUENCE_COMPLETE], 0,
		       resp->status);
}

static void
e_book_view_do_status_message_event (EBookView                 *book_view,
				     EBookViewListenerResponse *resp)
{
	g_signal_emit (book_view, e_book_view_signals [STATUS_MESSAGE], 0,
		       resp->message);
	g_free(resp->message);
}


/*
 * Reading notices out of the EBookViewListener's queue.
 */
static void
e_book_view_check_listener_queue (EBookViewListener *listener, EBookView *book_view)
{
	EBookViewListenerResponse *resp;

	resp = e_book_view_listener_pop_response (listener);
	
	if (resp == NULL)
		return;

	switch (resp->op) {
	case CardAddedEvent:
		e_book_view_do_added_event (book_view, resp);
		break;
	case CardModifiedEvent:
		e_book_view_do_modified_event (book_view, resp);
		break;
	case CardsRemovedEvent:
		e_book_view_do_removed_event (book_view, resp);
		break;
	case SequenceCompleteEvent:
		e_book_view_do_complete_event (book_view, resp);
		break;
	case StatusMessageEvent:
		e_book_view_do_status_message_event (book_view, resp);
		break;
	default:
		g_error ("EBookView: Unknown operation %d in listener queue!\n",
			 resp->op);
		break;
	}

	g_free (resp);
}

static gboolean
e_book_view_construct (EBookView *book_view, GNOME_Evolution_Addressbook_BookView corba_book_view, EBookViewListener *listener)
{
	CORBA_Environment  ev;
	g_return_val_if_fail (book_view != NULL,     FALSE);
	g_return_val_if_fail (E_IS_BOOK_VIEW (book_view), FALSE);

	/*
	 * Copy in the corba_book_view.
	 */
	CORBA_exception_init (&ev);

	book_view->priv->corba_book_view = bonobo_object_dup_ref(corba_book_view, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("e_book_view_construct: Exception duplicating corba_book_view.\n");
		CORBA_exception_free (&ev);
		book_view->priv->corba_book_view = NULL;
		return FALSE;
	}

	CORBA_exception_free (&ev);

	/*
	 * Create our local BookListener interface.
	 */
	book_view->priv->listener = listener;
	book_view->priv->responses_queued_id = g_signal_connect (book_view->priv->listener, "responses_queued",
								 G_CALLBACK (e_book_view_check_listener_queue), book_view);

	bonobo_object_ref(BONOBO_OBJECT(book_view->priv->listener));

	return TRUE;
}

/**
 * e_book_view_new:
 */
EBookView *
e_book_view_new (GNOME_Evolution_Addressbook_BookView corba_book_view, EBookViewListener *listener)
{
	EBookView *book_view;

	book_view = g_object_new (E_TYPE_BOOK_VIEW, NULL);

	if (! e_book_view_construct (book_view, corba_book_view, listener)) {
		g_object_unref (book_view);
		return NULL;
	}

	return book_view;
}

void
e_book_view_set_book (EBookView *book_view, EBook *book)
{
	g_return_if_fail (book_view && E_IS_BOOK_VIEW (book_view));
	g_return_if_fail (book && E_IS_BOOK (book));
	g_return_if_fail (book_view->priv->book == NULL);

	book_view->priv->book = book;
	g_object_ref (book);
}

void
e_book_view_stop (EBookView *book_view)
{
	g_return_if_fail (book_view && E_IS_BOOK_VIEW (book_view));
	if (book_view->priv->listener)
		e_book_view_listener_stop (book_view->priv->listener);
}

static void
e_book_view_init (EBookView *book_view)
{
	book_view->priv                      = g_new0 (EBookViewPrivate, 1);
	book_view->priv->book                = NULL;
	book_view->priv->corba_book_view     = CORBA_OBJECT_NIL;
	book_view->priv->listener            = NULL;
	book_view->priv->responses_queued_id = 0;
}

static void
e_book_view_dispose (GObject *object)
{
	EBookView             *book_view = E_BOOK_VIEW (object);
	CORBA_Environment  ev;

	if (book_view->priv) {
		if (book_view->priv->book) {
			g_object_unref (book_view->priv->book);
		}

		if (book_view->priv->corba_book_view) {
			CORBA_exception_init (&ev);

			bonobo_object_release_unref (book_view->priv->corba_book_view, &ev);

			if (ev._major != CORBA_NO_EXCEPTION) {
				g_warning ("EBookView: Exception while releasing BookView\n");
			}

			CORBA_exception_free (&ev);
		}

		if (book_view->priv->listener) {
			if (book_view->priv->responses_queued_id)
				g_signal_handler_disconnect(book_view->priv->listener,
							    book_view->priv->responses_queued_id);
			e_book_view_listener_stop (book_view->priv->listener);
			bonobo_object_unref (BONOBO_OBJECT(book_view->priv->listener));
		}

		g_free (book_view->priv);
		book_view->priv = NULL;
	}

	G_OBJECT_CLASS(parent_class)->dispose (object);
}

static void
e_book_view_class_init (EBookViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_ref (G_TYPE_OBJECT);

	e_book_view_signals [CARD_CHANGED] =
		g_signal_new ("card_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EBookViewClass, card_changed),
			      NULL, NULL,
			      e_book_marshal_NONE__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);

	e_book_view_signals [CARD_ADDED] =
		g_signal_new ("card_added",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EBookViewClass, card_added),
			      NULL, NULL,
			      e_book_marshal_NONE__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);

	e_book_view_signals [CARD_REMOVED] =
		g_signal_new ("card_removed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EBookViewClass, card_removed),
			      NULL, NULL,
			      e_book_marshal_NONE__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);

	e_book_view_signals [SEQUENCE_COMPLETE] =
		g_signal_new ("sequence_complete",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EBookViewClass, sequence_complete),
			      NULL, NULL,
			      e_book_marshal_NONE__INT,
			      G_TYPE_NONE, 1,
			      G_TYPE_INT);

	e_book_view_signals [STATUS_MESSAGE] =
		g_signal_new ("status_message",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EBookViewClass, status_message),
			      NULL, NULL,
			      e_book_marshal_NONE__STRING,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRING);

	object_class->dispose = e_book_view_dispose;
}

/**
 * e_book_view_get_type:
 */
GType
e_book_view_get_type (void)
{
	static GType type = 0;

	if (! type) {
		GTypeInfo info = {
			sizeof (EBookViewClass),
			NULL, /* base_class_init */
			NULL, /* base_class_finalize */
			(GClassInitFunc)  e_book_view_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (EBookView),
			0,    /* n_preallocs */
			(GInstanceInitFunc) e_book_view_init
		};

		type = g_type_register_static (G_TYPE_OBJECT, "EBookView", &info, 0);
	}

	return type;
}