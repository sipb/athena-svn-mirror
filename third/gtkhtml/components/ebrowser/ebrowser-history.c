/*
 * ebrowser-history.c: Manages the history for EBrowser
 *
 * Author:
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * (C) 2000 Helix Code, Inc.
 */
#include <config.h>
#include <glib.h>
#include <stdio.h>
#include "ebrowser-history.h"

EBrowserHistory *
ebrowser_history_new (int max_size)
{
	EBrowserHistory *history = g_new0 (EBrowserHistory, 1);

	g_assert (max_size > 1);
	
	history->max_size = max_size;
	
	return history;
}

void
ebrowser_history_destroy (EBrowserHistory *history)
{
	GList *l;

	g_assert (history != NULL);

	for (l = history->history; l; l = l->next){
		g_free (l->data);
	}
	g_list_free (history->history);
	g_free (history);
}

void
ebrowser_dump_history (EBrowserHistory *history)
{
	int i;
	GList *l;
	
	printf ("Count: %d == %d\n", history->count, g_list_length (history->history));
	
	for (i = 0, l = history->history; l; l = l->next, i++) {
		printf ("   %d: %s\n", i, (char *) l->data);
	}
	
}

void
ebrowser_history_push (EBrowserHistory *history, const char *url)
{
	g_assert (history != NULL);
	g_assert (url != NULL);

	/*
	 * Kill forward history at this point
	 */
	if (history->current != history->history){
		GList *l;

		history->current->prev->next = NULL;
		history->current->prev = NULL;
		
		for (l = history->history; l; l = l->next){
			history->count--;
			g_free (l->data);
		}
		g_list_free (history->history);
		history->history = history->current;
	}

	/*
	 * Add more history
	 */
	history->history = g_list_prepend (history->history, g_strdup (url));
	history->count++;

	/*
	 * Limit population
	 */
	if (history->count > history->max_size){
		GList *tail;

		tail = g_list_last (history->history);
		if (tail->prev)
			tail->prev->next = NULL;
		history->count--;
		g_list_free (tail);
	}

	/*
	 * Dum de dum
	 */
	history->current = history->history;
}

const char *
ebrowser_history_prev (EBrowserHistory *history)
{
	g_assert (history != NULL);

	if (!history->current->next)
		return NULL;

	history->current = history->current->next;

	return (const char *) history->current->data;
}

int 
ebrowser_history_get_state (EBrowserHistory *history)
{
	int result = 0;

	g_assert (history != NULL);

	if (history->current->next)
		result |= EBROWSER_HISTORY_CAN_STEPBACK;
	if (history->current->prev)
		result |= EBROWSER_HISTORY_CAN_ADVANCE;

	return result;
}

const char *
ebrowser_history_peek (EBrowserHistory *history)
{
	g_assert (history != NULL);

	if (!history->current)
		return NULL;

	return (const char *) history->current->data;
}

const char *
ebrowser_history_next (EBrowserHistory *history)
{
	g_assert (history != NULL);

	if (history->current == NULL)
		return NULL;
			
	if (history->current == history->history)
		return NULL;

	history->current = history->current->prev;

	return (char *) history->current->data;
}

void
ebrowser_history_set_size (EBrowserHistory *history, int size)
{
	g_assert (history != NULL);
	g_assert (size > 1);

	if (size > history->max_size){
		history->max_size = size;
		return;
	}

	if (size < history->count){
		GList *l = g_list_nth (history->history, size);

		l->prev->next = NULL;
		g_list_free (l);
		history->count = size;
	}

	history->max_size = size;
}
