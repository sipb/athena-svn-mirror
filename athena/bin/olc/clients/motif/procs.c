#include "xolc.h"

extern Widget			/* Widget ID's */
  widget_askbox,
  widget_motdbox,
  widget_motdframe,
  widget_helpbox,
  widget_box_button[4],
  widget_errorbox,
  widget_replaybox,
  toplevel,
  main_form;

  /**************************************************************/
  /*  Procedures						*/
  /**************************************************************/

void olc_ask (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("ask button pressed\n");
#endif DEBUG
  if ( XtIsManaged(widget_motdframe) )
    XtUnmanageChild(widget_motdframe);
  if (! XtIsManaged(widget_askbox) )
    XtManageChild(widget_askbox);
}

void olc_send (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("send button pressed\n");
#endif DEBUG
}
  
void olc_done (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("done button pressed\n");
#endif DEBUG
}
  
void olc_cancel (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("cancel button pressed\n");
#endif DEBUG
}
  
void olc_save_log (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("save_log button pressed\n");
#endif DEBUG
}
  
void olc_stock (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("stock button pressed\n");
#endif DEBUG
}
  
void olc_motd (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  Arg arg;

#ifdef DEBUG
  printf ("motd button pressed\n");
#endif DEBUG
  if (! XtIsManaged(widget_motdbox) )
    XtManageChild(widget_motdbox);
}
  
void olc_update (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("update button pressed\n");
#endif DEBUG
}
  
void olc_help (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("help button pressed\n");
#endif DEBUG
  if (! XtIsManaged(widget_helpbox) )
    XtManageChild(widget_helpbox);
}

void olc_quit (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef DEBUG
  printf ("quit button pressed\n");
#endif DEBUG
  XtDestroyWidget(toplevel);
  exit(0);
}
