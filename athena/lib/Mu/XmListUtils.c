#include <Xm/Xm.h>
#include <Xm/ListP.h>  /* we need to get inside the list.  this is illegal */


void XmListGetSelectedPositions(w,return_table, return_count)
XmListWidget w;
int **return_table;
int *return_count;
{
    int i,j;
    int *table;

    if (w->list.selectedItemCount == 0) {
	*return_table = NULL;
	*return_count = 0;
	return;
    }
	
    table = (int *) XtMalloc(w->list.selectedItemCount * sizeof(int));
        
    for (i=j=0; i < w->list.itemCount; i++) {
	if (w->list.InternalList[i]->selected) table[j++] = i;
    }

    *return_table = table;
    *return_count = w->list.selectedItemCount;
}
    
void XmListSetSelectedPositions(w, items, num_items)
XmListWidget w;
int *items;
int num_items;
{
    register int i,j;

    /* free up the current selected items list */
    if (w->list.selectedItems && w->list.selectedItemCount) {
	for (i = 0; i < w->list.selectedItemCount; i++)
	    XmStringFree(w->list.selectedItems[i]);
	XtFree(w->list.selectedItems);    
	w->list.selectedItemCount = 0;
	w->list.selectedItems = NULL; 
    }

    /* select the new items */
    for(i=0; i < num_items; i++) 
	w->list.InternalList[items[i]]->selected = TRUE;

    /* build the new list of selected items */
    /* this broken code straight from List widget */
    for (i = 0; i < w->list.itemCount; i++)
	if (w->list.InternalList[i]->selected)
	{
            j = w->list.selectedItemCount++;
            w->list.selectedItems =(XmString *)XtRealloc(w->list.selectedItems,
			    (sizeof(XmString) * (w->list.selectedItemCount)));
            w->list.selectedItems[j] =  XmStringCopy(w->list.items[i]);
        }
    
    /* force a Redisplay */
    (XtClass(w)->core_class.expose)(w, (XEvent *)NULL, (Region)NULL);
}

int XmListGetTopPos(w)
XmListWidget w;
{
    /* we add one because a top_position of 0, means item 1 is on the top */
    return w->list.top_position + 1; 
}

int XmListGetBottomPos(w)
XmListWidget w;
{
    return w->list.top_position + 1 + w->list.visibleItemCount - 1;
}
