#ifndef _EBROWSER_HISTORY_H_
#define _EBROWSER_HISTORY_H_

typedef struct {
	GList *history;
	GList *current;
	GList *last;
	
	short  count;
	short  max_size;
} EBrowserHistory;

EBrowserHistory *ebrowser_history_new      (int max_size);
void             ebrowser_history_destroy  (EBrowserHistory *history);
void             ebrowser_history_set_size (EBrowserHistory *history, int size); 
void        	 ebrowser_history_push     (EBrowserHistory *history, const char *url);
const char  	*ebrowser_history_prev     (EBrowserHistory *history);
const char  	*ebrowser_history_next     (EBrowserHistory *history);
void             ebrowser_dump_history     (EBrowserHistory *history);
const char      *ebrowser_history_peek     (EBrowserHistory *history);

/* Flags */
enum {
	EBROWSER_HISTORY_CAN_STEPBACK = 1,
	EBROWSER_HISTORY_CAN_ADVANCE  = 2,
} EBrowserHistoryState;

int ebrowser_history_get_state (EBrowserHistory *history);

#endif /* _EBROWSER_HISTORY_H_ */

