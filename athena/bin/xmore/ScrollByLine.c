/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: ScrollByL.c,v 1.5 89/01/06 18:41:40 kit Exp $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/ScrollByLine.c,v 1.3 1989-10-15 04:35:26 probe Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   December 5, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: ScrollByL.c,v 4.5 88/12/19 13:46:04 kit Exp $";
#endif

/*
 * I wrote this widget before I knew what form did, and it shows, since
 * the "right" way to do this widget would be to subclass it to form,
 * and do it much more like the Viewport widget in the Athena widget set.
 * But this works and time is short, so here it is.
 *
 *                                     Chris D. Peterson 1/30/88
 * 
 * I removed all the code for horizontal scrolling here, since is was a crock
 * anyway.
 * 
 *                                     Chris D. Peterson 11/13/88
 */

#include	<X11/IntrinsicP.h>
#include	"ScrollByLine.h"
#include	"ScrollByLineP.h"
#include        <X11/Scroll.h>
#include	<X11/StringDefs.h>
#include	<X11/XawMisc.h>

/* Default Translation Table */

static char defaultTranslations[] = 
  "<Btn1Down>:  Page(Forward) \n\
   <Btn3Down>:  Page(Back) \n\
   <Key>f:      Page(Forward) \n\
   <Key>b:      Page(Back) \n\
   <Key>\\ :    Page(Forward)";

      
/****************************************************************
 *
 * ScrollByLine Resources
 *
 ****************************************************************/

static XtResource resources[] = {
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	 XtOffset(ScrollByLineWidget, scroll_by_line.foreground), 
         XtRString, "XtDefaultForeground"},
    {XtNinnerWidth, XtCWidth, XtRInt, sizeof(int),
	 XtOffset(ScrollByLineWidget, scroll_by_line.inner_width), 
         XtRString, "100"},
    {XtNinnerHeight, XtCHeight, XtRInt, sizeof(int),
	 XtOffset(ScrollByLineWidget, scroll_by_line.inner_height),
         XtRString, "100"},
    {XtNforceBars, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset(ScrollByLineWidget, scroll_by_line.force_bars),
         XtRString, "FALSE"},
    {XtNallowVert, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset(ScrollByLineWidget, scroll_by_line.allow_vert),
         XtRString, "FALSE"},
    {XtNuseRight, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 XtOffset(ScrollByLineWidget, scroll_by_line.use_right),
         XtRString, "FALSE"},
    {XtNlines, XtCLine, XtRInt, sizeof(int), 
         XtOffset(ScrollByLineWidget, scroll_by_line.lines), 
         XtRString, "1"},
    {XtNfontHeight, XtCHeight,XtRInt, sizeof(int),
         XtOffset(ScrollByLineWidget, scroll_by_line.font_height), 
         XtRString, "0"},
    {XtNcallback, XtCCallback, XtRCallback, sizeof(caddr_t), 
       XtOffset(ScrollByLineWidget, scroll_by_line.callbacks), 
       XtRCallback, (caddr_t) NULL},
    {XtNformOnInner, XtCBoolean, XtRBoolean, sizeof(Boolean),
       XtOffset(ScrollByLineWidget, scroll_by_line.key),
       XtRString, "FALSE"},
};

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

static Boolean ScrollVerticalText();
static void VerticalThumb();
static void VerticalScroll();
static void Page();
static void InitializeHook();
static void Initialize();
static void Realize();
static void Resize();
static void ResetThumb();
static void Redisplay();
static void ChildExpose();
static Boolean SetValues();
static Boolean Layout();
static XtGeometryResult GeometryManager();
static void ChangeManaged();

static XtActionsRec actions[] = {
  { "Page",   Page},
  { NULL, NULL},
};

ScrollByLineClassRec scrollByLineClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &compositeClassRec,
    /* class_name         */    "ScrollByLine",
    /* widget_size        */    sizeof(ScrollByLineRec),
    /* class_initialize   */    NULL,
    /* class_part_init    */    NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */    InitializeHook,
    /* realize            */    Realize,
    /* actions            */    actions,
    /* num_actions	  */	XtNumber(actions),
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	FALSE,
    /* compress_enterleave*/    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    Resize,
    /* expose             */    Redisplay,
    /* set_values         */    SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    NULL,
    /* version            */    XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    defaultTranslations,
    /* query_geometry	  */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension	  */	NULL,
  },{
/* composite_class fields */
    /* geometry_manager   */    GeometryManager,
    /* change_managed     */    ChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */	NULL,
  },{
    /* mumble		  */	0	/* Make C compiler happy   */
  }
};

WidgetClass scrollByLineWidgetClass = 
            (WidgetClass) &scrollByLineClassRec;


/****************************************************************
 *
 * Private Routines
 *
 ****************************************************************/


/*	Function Name: Layout
 *	Description: This function lays out the scroll_by_line widget.
 *	Arguments: w - the scroll_by_line widget.
 *                 key - a boolean: if true then resize the widget to the child
 *                                  if false the resize children to fit widget.
 *	Returns: TRUE if successful.
 */

static Boolean
Layout(w,key)
Widget w;
int key;
{    
  ScrollByLineWidget sblw = (ScrollByLineWidget) w;
  Dimension width,height;	/* The size that the widget would like to be */
  XtGeometryResult answer;	/* the answer from the parent. */
  Widget vbar,child;		/* The two children of this scrolled widget. */
  int vbar_x,vbar_y;		/* The locations of the various elements. */
  int child_x;
  int c_width;
  Boolean make_bar;

  vbar = sblw->composite.children[0];
  child = sblw->composite.children[1];
  height = sblw->core.height;
  width = sblw->core.width;

/* set the initial scroll bar positions. */

  vbar_x = vbar_y = - vbar->core.border_width;

/* Should I allow the vertical scrollbar to be seen */

   make_bar = FALSE;  
   if ( (!key) && (sblw->scroll_by_line.lines * 
	sblw->scroll_by_line.font_height > height) )
     make_bar = TRUE;

   else if ( (key) && (sblw->scroll_by_line.lines * 
	     sblw->scroll_by_line.font_height > child->core.height) )
     make_bar = TRUE;

   if (sblw->scroll_by_line.allow_vert && (sblw->scroll_by_line.force_bars || 
				       make_bar) ) {
 /*
  * Resize the outer window to fit the child, or visa versa.  Also make scroll
  * bar become placed in the correct location.
  */
     if (key)			
       width = child->core.width + vbar->core.width + 
	       2 * vbar->core.border_width;
     else
       c_width = width - vbar->core.width -2 * vbar->core.border_width;

     /* Put Scrollbar on right side if scrolled window? */

     if (sblw->scroll_by_line.use_right) {
       vbar_x = width - vbar->core.width - 2 * vbar->core.border_width;
       child_x = 0;
     }
     else 
       child_x = vbar->core.width + vbar->core.border_width;
   }
   else {
     /* Make the scroll bar dissappear, note how scroll bar is always there,
        sometimes it is just that we cannot see them. */
     vbar_x = - vbar->core.width - 2 * vbar->core.border_width - 10;
     child_x = 0;
     if (key)
       width = child->core.width;
     else
       c_width = width;
   }

 /* Move child and v_bar to correct location. */

  XtMoveWidget(vbar, vbar_x, vbar_y);
  XtMoveWidget(child, child_x, 0);

  /* resize the children to be the correct height or width. */

  XtResizeWidget(vbar, vbar->core.width, height, vbar->core.border_width);

  if (!key)
    XtResizeWidget(child, (Cardinal) (c_width - 20), (Cardinal) height,
		   child->core.border_width);

  /* set the thumb size to be correct. */

  ResetThumb( (Widget) sblw);

  answer = XtMakeResizeRequest( (Widget) sblw, width, height, &width, &height);

  switch(answer) {
  case XtGeometryYes:
    break;
  case XtGeometryNo:
    return(FALSE);
  case XtGeometryAlmost:
    (void) Layout( (Widget) sblw,FALSE);
  }
  return(TRUE);
}

/*	Function Name: ResetThumb
 *	Description: This function resets the thumb's shown percentage only.
 *	Arguments: w - the ScrollByLineWidget.
 *	Returns: none;
 */

static void
ResetThumb(w)
Widget w;
{
  float shown;
  ScrollByLineWidget sblw = (ScrollByLineWidget) w;
  Widget child,vbar;

  vbar = sblw->composite.children[0];
  child = sblw->composite.children[1];

/* vertical */

  shown = (float) child->core.height /(float) (sblw->scroll_by_line.lines *
       				  sblw->scroll_by_line.font_height);
  if (shown > 1.0)
    shown = 1.0;

  XtScrollBarSetThumb( vbar, (float) -1, shown );
}

/*
 *
 * Geometry Manager - If the height of width is changed then try a new layout.
 *                    else dissallow the requwest.
 *
 */

/*ARGSUSED*/
static XtGeometryResult GeometryManager(w, request, reply)
    Widget		w;
    XtWidgetGeometry	*request;
    XtWidgetGeometry	*reply;	/* RETURN */

{
  ScrollByLineWidget sblw;

  sblw = (ScrollByLineWidget) w->core.parent;
  if ( request->width != 0 && request->height != 0 && 
      (request->request_mode && (CWWidth || CWHeight)) ) {
    w->core.height = request->height;
    w->core.width = request->width;
    (void) Layout( (Widget) sblw,TRUE);
    return(XtGeometryYes);
  }
  return(XtGeometryNo);

} /* Geometery Manager */

/* ARGSUSED */
static void ChildExpose(w,junk,event)
Widget w;
caddr_t junk;
XEvent *event;
{

/* 
 * since we are realy concerned with the expose events that happen
 * to the child, we have selected expose events on this window, and 
 * then I call the redisplay routine.
 */

  if ((event->type == Expose) || (event->type == GraphicsExpose))
    Redisplay(w->core.parent, event, NULL);

} /* ChildExpose */

/*
 * Repaint the widget's child Window Widget.
 */

/* ARGSUSED */
static void Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
  ScrollByLineWidget sblw = (ScrollByLineWidget) w;

  int top,bottom;		/* the locations of the top and
       			  bottom of the region that needs to be
       			  repainted. */
  ScrollByLineStruct sblw_struct;

/*
 * This routine tells the client which sections of the window to 
 * repaint in his callback function which does the actual repainting.
 */

  if (event->type == Expose) {
    top = event->xexpose.y;
    bottom = event->xexpose.height + top;
  }
  else {
    top = event->xgraphicsexpose.y;
    bottom  = event->xgraphicsexpose.height + top;
  }
  
  sblw_struct.start_line = top / sblw->scroll_by_line.font_height + 
                            sblw->scroll_by_line.line_pointer;
/*
 * If an expose event is called on a region that has no text assoicated
 * with it then do not redisplay. Only nescessary for very short file.
 */
  if (sblw_struct.start_line > sblw->scroll_by_line.lines)
    return;
  sblw_struct.num_lines = (bottom - top) / 
                            sblw->scroll_by_line.font_height + 1;
  sblw_struct.location =  top / sblw->scroll_by_line.font_height *
                            sblw->scroll_by_line.font_height;

  XtCallCallbacks( (Widget) sblw, XtNcallback, (caddr_t) &sblw_struct);

} /* redisplay (expose) */

static void
Resize(w)
Widget w;
{

  (void) Layout(w,FALSE);

} /* Resize */

static void ChangeManaged(w)
Widget w;
{
  /* note how we ignore bonus children, but since we control all children,
   there should never be a problem anyway. */
  ScrollByLineWidget sblw = (ScrollByLineWidget) w;;

  if (sblw->composite.num_children == 3) {
    (void) Layout( w,sblw->scroll_by_line.key);
  }

} /* Change Managed */

/*	Function Name: Page
 *	Description: This function pages the widget, by the amount it recieves
 *                   from the translation Manager.
 *	Arguments: w - the ScrollByLineWidget.
 *                 event - the event that caused this return.
 *                 params - the parameters passed to it.
 *                 num_params - the number of parameters.
 *	Returns: none.
 */

static void 
Page(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal *num_params;
{
   ScrollByLineWidget sblw = (ScrollByLineWidget) w;
   Widget vbar = sblw->composite.children[0];
   char direction;

   if (*num_params > 0)
     direction = *params[0];
   else
     return;

/*
 * If no scroll bar is visible then do not page, as the entire window is shown,
 * of scrolling has been turned off. 
 */

   if (vbar->core.x < - vbar->core.border_width)
     return;

   switch ( direction ) {
   case 'f':
   case 'F':
     /* move one page forward */
     VerticalScroll(vbar,NULL, (int) vbar->core.height);
     break;
   case 'b':
   case 'B':
     /* move one page backward */
     VerticalScroll(vbar,NULL,  - (int) vbar->core.height);
     break;
   case 'L':
   case 'l':
     /* move one line forward */
     VerticalScroll(vbar,NULL, (int) sblw->scroll_by_line.font_height);
     break;
   default:
     return;
   }
}

/*	Function Name: ScrollVerticalText
 *	Description: This accomplished the actual movement of the text.
 *	Arguments: w - the ScrollByLine Widget.
 *                 new_line - the new location for the line pointer
 *                 force_redisplay - should we force this window to get 
 *                                   redisplayed?
 *	Returns: True if the thumb needs to be moved.
 */

static Boolean
ScrollVerticalText(w,new_line,force_redisp)
Widget w;
int new_line;
Boolean force_redisp;
{
  ScrollByLineWidget sblw = (ScrollByLineWidget) w;
  int max_lines,		/* The location of top of the last screen. */
    num_lines,			/* The number of lines in one screen of text */
    y_pos,			/* The location to start displaying text. */
    num_lines_disp,		/* The number of lines to display. */
    start_line,			/* The line to start displaying text. */
    y_location,			/* The y_location to for storing copy area. */
    lines_to_move;		/* The number of lines to copy. */
  GC gc;
  Widget child,vbar;	/* Widgets. */
  ScrollByLineStruct sblw_struct;
  Boolean move_thumb = FALSE;

  vbar =  sblw->composite.children[0];
  child = sblw->composite.children[1];

  num_lines =  child->core.height / sblw->scroll_by_line.font_height;

  gc = XCreateGC(XtDisplay( (Widget) sblw),XtWindow(child),NULL,0);
  XSetGraphicsExposures(XtDisplay( (Widget) sblw),gc,TRUE);

 /* do not let the window extend out of bounds */

  if ( new_line < 0) {
    move_thumb = TRUE;
    new_line = 0;
  }
  else {
    max_lines = sblw->scroll_by_line.lines -
     child->core.height / sblw->scroll_by_line.font_height;

    if (max_lines < 0) max_lines = 0;

    if ( new_line > max_lines ) {
      new_line = max_lines;
      move_thumb = TRUE;
    }
  }

  if ( new_line == sblw->scroll_by_line.line_pointer && !force_redisp)
/* No change in postion, and no action is nescessary */
    return(move_thumb);
  else if ( new_line <= sblw->scroll_by_line.line_pointer) { /* scroll back. */
    if ( sblw->scroll_by_line.line_pointer - new_line >= num_lines
	|| force_redisp) {
/*
 * We have moved so far that no text that is currently on the screen can
 * be saved thus there is no need to be clever just clear
 * the window and display a full screen of text.
 */
      XClearWindow(XtDisplay(child),XtWindow(child));
      y_pos = 0;
      start_line = new_line;
      num_lines_disp = num_lines;
    }
    else {
/*
 * Move text that is to remain on the screen to its new location, and then
 * set up the proper callback values to display the rest of the text.
 */
      lines_to_move = num_lines - sblw->scroll_by_line.line_pointer + new_line;
      y_location = sblw->scroll_by_line.line_pointer - new_line;
      XCopyArea(XtDisplay(vbar),XtWindow(child),XtWindow(child),
	 gc,0,0,child->core.width,
	 (lines_to_move + 1) * sblw->scroll_by_line.font_height,
	 0,y_location * sblw->scroll_by_line.font_height);
      XClearArea( XtDisplay(vbar),XtWindow(child),0,0,0,
	  (num_lines - lines_to_move) 
	  * sblw->scroll_by_line.font_height,
	  FALSE );
      y_pos = 0;
      start_line = new_line;
      num_lines_disp = num_lines - lines_to_move;
    }
  }
  else {     /* scrolling forward */
    if ( new_line - sblw->scroll_by_line.line_pointer >= num_lines
	|| force_redisp) {
/*
 * We have moved so far that no text that is currently on the screen can
 * be saved thus there is no need to be clever just clear
 * the window and display a full screen of text.
 */
      XClearWindow(XtDisplay(child),XtWindow(child));
      y_pos = 0;
      start_line = new_line;
      num_lines_disp = num_lines;
    }
    else {
/*
 * Move text that is to remain on the screen to its new location, and then
 * set up the proper callback values to display the rest of the text.
 */
      lines_to_move = num_lines - new_line + sblw->scroll_by_line.line_pointer;
      y_location = new_line - sblw->scroll_by_line.line_pointer;
      XCopyArea(XtDisplay(vbar),XtWindow(child),XtWindow(child),gc,0,
		y_location * sblw->scroll_by_line.font_height,
		child->core.width,
		(lines_to_move)* sblw->scroll_by_line.font_height,0,0);

      lines_to_move--;		/* make sure that we get the last,
        			    (possibly) partial line, fully painted. */

      /* we add 10% of a font height here to the vertical position
       because some characters extend a little bit below the fontheight */

      XClearArea(XtDisplay(vbar),XtWindow(child),0,lines_to_move *
		 sblw->scroll_by_line.font_height + (int)
		 (.1 * (float) sblw->scroll_by_line.font_height),0,0,FALSE);

      y_pos = lines_to_move * sblw->scroll_by_line.font_height;
      start_line = new_line + lines_to_move;
      num_lines_disp = num_lines - lines_to_move;
    }
  }
  
  sblw->scroll_by_line.line_pointer = new_line;

/*
 * call the callbacks, this is the callback to the application to do the 
 * actual painting of the text. 
 */

  sblw_struct.location = y_pos;
  sblw_struct.start_line = start_line;
  sblw_struct.num_lines = num_lines_disp;

  XtCallCallbacks( (Widget) sblw,XtNcallback, (caddr_t) &sblw_struct);

/* Save that memory */

  XFreeGC(XtDisplay( (Widget) sblw), gc);

  return(move_thumb);
}

/*	Function Name: VerticalThumb
 *	Description: This function moves the postition of the interior window
 *                   as the vertical scroll bar is moved.
 *	Arguments: w - the scrollbar widget.
 *                 junk - not used.
 *                 percent - the position of the scrollbar.
 *	Returns: none.
 */

static void
VerticalThumb(w,junk,percent)
Widget w;
caddr_t junk;
float *percent;
{
  int new_line;			/* The new location for the line pointer. */
  float location;		/* The location of the thumb. */
  Widget vbar;

  ScrollByLineWidget sblw = (ScrollByLineWidget) w->core.parent;

  vbar =  sblw->composite.children[0];

  new_line = (int) ((float) sblw->scroll_by_line.lines * (*percent));

  if (ScrollVerticalText( (Widget) sblw, new_line, FALSE)) {
/* reposition the thumb */
    location = (float) sblw->scroll_by_line.line_pointer / 
               (float) sblw->scroll_by_line.lines; 
    XtScrollBarSetThumb( vbar, location , (float) -1 );
  }

}

/*	Function Name: VerticalScroll
 *	Description: This function moves the postition of the interior window
 *                   as the vertical scroll bar is moved.
 *	Arguments: w - the scrollbar widget.
 *                 junk - not used.
 *                 pos - the position of the cursor.
 *	Returns: none.
 */

static void
VerticalScroll(w,junk,pos)
Widget w;
caddr_t junk;
int pos;
{
  int new_line;			/* The new location for the line pointer. */
  float location;		/* The new location of the thumb. */
  Widget vbar;

  ScrollByLineWidget sblw = (ScrollByLineWidget) w->core.parent;

  vbar =  sblw->composite.children[0];

  new_line = sblw->scroll_by_line.line_pointer;
  new_line += (int) pos / sblw->scroll_by_line.font_height;

  (void) ScrollVerticalText( (Widget) sblw,new_line,FALSE);

/* reposition the thumb */

  location = (float) sblw->scroll_by_line.line_pointer / 
             (float) sblw->scroll_by_line.lines; 
  XtScrollBarSetThumb( vbar, location , (float) -1 );
  

}

static void 
Initialize(req, new)
Widget req, new;
{
  ScrollByLineWidget sblw = (ScrollByLineWidget) new;
  
  sblw->scroll_by_line.line_pointer = 0; /* initially point to line 0. */

  if (sblw->core.height <= 0)
    sblw->core.height = DEFAULT_HEIGHT;
  if (sblw->core.width <= 0)
    sblw->core.width = DEFAULT_WIDTH;
} /* Initialize. */

static void 
InitializeHook(new, args, num_args)
ScrollByLineWidget new;
ArgList args;
Cardinal *num_args;
{
  ScrollByLineWidget sblw = (ScrollByLineWidget) new;
  Widget window, s_bar;		/* Window widget, and scollbar. */
  Arg arglist[10];		/* an arglist. */
  ArgList merged_list, XtMergeArgLists(); /* The merged arglist. */
  Cardinal window_num, merged_num; /* The number of window args. */

  s_bar = XtCreateManagedWidget("verticalScrollBar", scrollbarWidgetClass,
				(Widget) sblw, args, *num_args);
  XtAddCallback(s_bar, XtNjumpProc, VerticalThumb, NULL);
  XtAddCallback(s_bar, XtNscrollProc, VerticalScroll, NULL);

  window_num = 0;  
  XtSetArg(arglist[window_num], XtNwidth, sblw->scroll_by_line.inner_width); 
  window_num++;
  XtSetArg(arglist[window_num], XtNheight, sblw->scroll_by_line.inner_height);
  window_num++;
  XtSetArg(arglist[window_num], XtNborderWidth, 0);
  window_num++;  

/* 
 * I hope this will cause my args to override those passed to the SBL widget.
 */
  
  merged_list = XtMergeArgLists(args, *num_args, arglist, window_num);
  merged_num = *num_args + window_num;

  window = XtCreateWidget("windowWithFile",widgetClass,(Widget) sblw,
			  merged_list, merged_num);
  XtManageChild(window);
  XtFree(merged_list);		/* done, free it. */

/*
 * We want expose (and graphic exposuer) events for this window also. 
 */

  XtAddEventHandler(window, (Cardinal) ExposureMask, TRUE, ChildExpose, NULL);
  
} /* InitializeHook */

static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    XtCreateWindow( w, (Cardinal) InputOutput, (Visual *)CopyFromParent,
	*valueMask, attributes);
} /* Realize */

/*
 *
 * Set Values
 *
 */

static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
  ScrollByLineWidget sblw_new, sblw_current;
  Boolean ret = FALSE;

  sblw_current = (ScrollByLineWidget) current;
  sblw_new = (ScrollByLineWidget) new;

  if (sblw_current->scroll_by_line.lines != sblw_new->scroll_by_line.lines) {
    ResetThumb(new);
    ret = TRUE;
  }
  if (sblw_current->scroll_by_line.font_height != 
      sblw_new->scroll_by_line.font_height) {
    ResetThumb(new);
    ret = TRUE;
  }
  return(ret);

} /* Set Values */

/* Public Routines. */

/*	Function Name: XtScrollByLineWidget()
 *	Description: This function returns the window widget that the 
 *                   ScrollByLine widget uses to display its text.
 *	Arguments: w - the ScrollByLine Widget.
 *	Returns: the widget to display the text into.
 */

extern Widget XtScrollByLineWidget(w)
Widget w;
{
  ScrollByLineWidget sblw = (ScrollByLineWidget) w; /* the sblw widget. */
  
  return(sblw->composite.children[1]);
}

/*	Function Name: XtResetScrollByLine
 *	Description: This function resets the scroll by line widget.
 *	Arguments: w - the sblw widget.
 *	Returns: none.
 */

extern void
XtResetScrollByLine(w)
Widget w;
{
  float location;		/* the location of the thumb. */
  ScrollByLineWidget sblw = (ScrollByLineWidget) w; /* the sblw widget. */
  Widget vbar;

  vbar =  sblw->composite.children[0];

  (void) ScrollVerticalText( w, 0, TRUE);
  
  /* reposition the thumb */

  location = (float) sblw->scroll_by_line.line_pointer / 
             (float) sblw->scroll_by_line.lines; 
  XtScrollBarSetThumb( vbar, location , (float) -1 );

  Layout(w, FALSE);		/* see if new layout is required. */
}
