#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "ebrowser-history.h"

static void
do_test (int init_size, char *ops, int last_size)
{
	EBrowserHistory *history = ebrowser_history_new (init_size);
	const char *p, *v;
	int num = 0;

	v = NULL;

	printf ("Beginnin test\n");
	for (p = ops; *p; p++, num++){
		char *str = g_strdup_printf ("operation: %d", num);

		switch (*p){
		case '+':
			ebrowser_history_push (history, str);
			break;

		case '<':
			v = ebrowser_history_prev (history);
			break;

		case '>':
			v = ebrowser_history_next (history);
			break;
				
		case 'p':
			printf ("    %s\n", ebrowser_history_peek (history));
			break;

		case 'S':
			ebrowser_history_set_size (history, 2);
			printf ("Count Items: %d (%d)\n", g_list_length (history->history), history->count);
			break;
			
		default:
			printf ("Unknown op: %c\n", *p);
			exit (1);
		}
		g_free (str);
	}
	if (last_size != -1)
		if (history->count != last_size){
			fprintf (stderr, "Count (%d) != last_size (%d)\n", history->count, last_size);
			exit (1);
		}
	printf ("Items: %d\n", g_list_length (history->history));
	ebrowser_dump_history (history);
	ebrowser_history_destroy (history);
	return;
}

int
main (int argc, char **argv)
{
	do_test (5, "++++++++S++++S", 2);
	do_test (4, "++++++++++++++++++++", 4);
	do_test (4, "+<", 1);
	do_test (4, "+<++<<++++<<<<", 4);
	do_test (4, "+++++", 4);
	do_test (4, ">>>>>>", 0);
	do_test (4, "+<>p", 1);
	do_test (4, "+++<<<p>>>p", -1);
	do_test (4, "+<p", -1);
	do_test (4, "+<p+p", -1);
	do_test (4, "+++<<+p<<<<<p+p<p", -1);
	do_test (4, "+++++++++<<+p<<<<<p+p<>p", -1);
	printf ("Test ok\n");
	exit (0);
}

