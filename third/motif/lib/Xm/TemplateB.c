/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: TemplateB.c,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:17:03 $"
#endif
#endif
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/TemplateBP.h>
#include <Xm/DialogS.h>

#include <Xm/RowColumnP.h>
#include <Xm/LabelG.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include "MessagesI.h"


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void ClassPartInitialize() ;
static void Initialize() ;
static Boolean SetValues() ;
static Boolean SetupRow() ;

#else

static void ClassPartInitialize( 
                        WidgetClass wc) ;
static void Initialize( 
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean SetValues( 
                        Widget cw,
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean SetupRow( 
                        Widget tb,
                        XmKidGeometry *boxes,
                        XmGeoRowLayout layoutPtr,
                        Widget *widgets,
                        int nwidgets,
                        int *no_space_above) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


static XtResource resources[] = {

    {   XmNminimizeButtons,
        XmCMinimizeButtons,
        XmRBoolean,
        sizeof(Boolean),
        XtOffsetOf( struct _XmTemplateBoxRec, template_box.minimize_buttons),
        XmRImmediate,
        (XtPointer) False}
};



/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

externaldef( xmtemplateboxclassrec) XmTemplateBoxClassRec xmTemplateBoxClassRec =
{
   {                                            /* core_class fields  */
      (WidgetClass) &xmBulletinBoardClassRec,   /* superclass         */
      "XmTemplateBox",                          /* class_name         */
      sizeof(XmTemplateBoxRec),                 /* widget_size        */
      NULL,                                     /* class_initialize   */
      ClassPartInitialize,                      /* class_part_init    */
      FALSE,                                    /* class_inited       */
      Initialize,                               /* initialize         */
      NULL,                                     /* initialize_hook    */
      XtInheritRealize,                         /* realize            */
      NULL,                                     /* actions            */
      0,                                        /* num_actions        */
      resources,                                /* resources          */
      XtNumber(resources),                      /* num_resources      */
      NULLQUARK,                                /* xrm_class          */
      TRUE,                                     /* compress_motion    */
      XtExposeCompressMaximal,                  /* compress_exposure  */
      FALSE,                                    /* compress_enterlv   */
      FALSE,                                    /* visible_interest   */
      NULL,                                     /* destroy            */
      XtInheritResize,                          /* resize             */
      XtInheritExpose,                          /* expose             */
      SetValues,                                /* set_values         */
      NULL,                                     /* set_values_hook    */
      XtInheritSetValuesAlmost,                 /* set_values_almost  */
      NULL,                                     /* get_values_hook    */
      XtInheritAcceptFocus,                     /* enter_focus        */
      XtVersion,                                /* version            */
      NULL,                                     /* callback_private   */
      XtInheritTranslations,                    /* tm_table           */
      XtInheritQueryGeometry,                   /* query_geometry     */
      NULL,                                     /* display_accelerator*/
      NULL,                                     /* extension          */
   },

   {                                            /* composite_class fields */
      XtInheritGeometryManager,                 /* geometry_manager   */
      XtInheritChangeManaged,                   /* change_managed     */
      XtInheritInsertChild,                     /* insert_child       */
      XtInheritDeleteChild,                     /* delete_child       */
      NULL,                                     /* extension          */
   },

   {                                            /* constraint_class fields */
      NULL,                                     /* resource list        */   
      0,                                        /* num resources        */   
      0,                                        /* constraint size      */   
      NULL,                                     /* init proc            */   
      NULL,                                     /* destroy proc         */   
      NULL,                                     /* set values proc      */   
      NULL,                                     /* extension            */
   },

   {                                            /* manager_class fields   */
      XmInheritTranslations,                    /* translations           */
      NULL,                                     /* syn_resources          */
      0,                                        /* num_syn_resources      */
      NULL,                                     /* syn_cont_resources     */
      0,                                        /* num_syn_cont_resources */
      XmInheritParentProcess,                   /* parent_process         */
      NULL,                                     /* extension              */
   },

   {                                            /* bulletinBoard class  */
      TRUE,                                     /*always_install_accelerators*/
      _XmTemplateBoxGeoMatrixCreate,             /* geo__matrix_create */
      XmInheritFocusMovedProc,                  /* focus_moved_proc */
      NULL                                      /* extension */
   },   

   {                                            /* messageBox class - none */
      0                                         /* mumble */
   }    
};

externaldef( xmmessageboxwidgetclass) WidgetClass xmTemplateBoxWidgetClass
                                        = (WidgetClass) &xmTemplateBoxClassRec ;


/****************************************************************/
static void 
#ifdef _NO_PROTO
ClassPartInitialize( wc )
        WidgetClass wc ;
#else
ClassPartInitialize(
        WidgetClass wc )
#endif /* _NO_PROTO */
{
/****************/

/*    _XmFastSubclassInit (wc, XmTEMPLATE_BOX_BIT); */

    return ;
}
/****************************************************************
 * TemplateBox widget specific initialization
 ****************/
static void 
#ifdef _NO_PROTO
Initialize( rw, nw, args, num_args )
        Widget rw ;
        Widget nw ;
        ArgList args ;
        Cardinal *num_args ;
#else
Initialize(
        Widget rw,
        Widget nw,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
/****************/

    return ;
    }

/****************************************************************
 * Set attributes of a message widget
 ****************/
static Boolean 
#ifdef _NO_PROTO
SetValues( cw, rw, nw, args, num_args )
        Widget cw ;
        Widget rw ;
        Widget nw ;
        ArgList args ;
        Cardinal *num_args ;
#else
SetValues(
        Widget cw,
        Widget rw,
        Widget nw,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
#if 0
            XmTemplateBoxWidget current = (XmTemplateBoxWidget) cw ;
            XmTemplateBoxWidget new = (XmTemplateBoxWidget) nw ;
            Boolean         need_layout = FALSE ;
#endif
/****************/

#if 0
    /* "in_set_values" means the GeometryManager won't try to resize
    *    and/or re-layout subwidgets.
    */
    BB_InSetValues( nw) = True;


    BB_InSetValues( nw) = False;

    if(    need_layout
        && (XtClass( nw) == xmTemplateBoxWidgetClass)    )
    {
        _XmBulletinBoardSizeUpdate( nw) ;
        }
#endif
    return( FALSE) ;
    }

/****************************************************************/
static Boolean
#ifdef _NO_PROTO
SetupRow( tb, boxes, layoutPtr, widgets, nwidgets, no_space_above)
        Widget tb ;
        XmKidGeometry *boxes ;
        XmGeoRowLayout layoutPtr ;
        Widget *widgets ;
        int nwidgets ;
        int *no_space_above ;
#else
SetupRow( 
        Widget tb,
        XmKidGeometry *boxes,
        XmGeoRowLayout layoutPtr,
        Widget *widgets,
        int nwidgets,
        int *no_space_above)
#endif /* _NO_PROTO */
{   
    if(    nwidgets    )
    {   
            register XmKidGeometry boxPtr = *boxes ;
            int Index ;

        for(    Index = 0 ; Index < nwidgets ; ++Index    )
        {   if(    _XmGeoSetupKid( boxPtr, widgets[Index])    )
            {   ++boxPtr ;
                } 
            } 
        if(    boxPtr != *boxes    )
        {   
            if(    *no_space_above    )
            {   *no_space_above = FALSE ;
                } 
            else
            {   layoutPtr->space_above = BB_MarginHeight( tb) ;
                } 
            layoutPtr->space_between = BB_MarginWidth( tb) ;

            *boxes = boxPtr + 1 ;

            return( TRUE) ;
            } 
        }
    return( FALSE) ;
    } 

/****************************************************************/
XmGeoMatrix 
#ifdef _NO_PROTO
_XmTemplateBoxGeoMatrixCreate( wid, instigator, desired )
        Widget wid ;
        Widget instigator ;
        XtWidgetGeometry *desired ;
#else
_XmTemplateBoxGeoMatrixCreate(
        Widget wid,
        Widget instigator,
        XtWidgetGeometry *desired )
#endif /* _NO_PROTO */
{
            XmTemplateBoxWidget tb = (XmTemplateBoxWidget) wid ;
            XmGeoMatrix     geoSpec ;
    register XmGeoRowLayout  layoutPtr ;
            XmKidGeometry   boxPtr ;
            Widget menubar = NULL;
            Widget separator = NULL;
            Widget *pbuttons ;
            Widget *labels ;
            Widget *misc ;
            Widget *layoutChildren ;
            int npbuttons = 0 ;
            int nlabels = 0 ;
            int nmisc = 0 ;
            int fix_menubar = False ;
            int nchildren = tb->composite.num_children;
            int i;
/****************/

    /* Create 3 arrays, each large enough to hold all children (rather than
    *  traversing the child list an extra time to figure out how much space
    *  is actually needed).
    */
    layoutChildren = (Widget *) XtMalloc( sizeof( Widget) * nchildren * 3) ;
    pbuttons = layoutChildren ;
    labels = &pbuttons[nchildren] ;
    misc = &labels[nchildren] ;

    for (i=0; i < nchildren; i++)
    {
            register Widget w = tb->composite.children[i];
  
        if(    XmIsPushButton(w) || XmIsPushButtonGadget(w)    )
        {
            pbuttons[npbuttons++] = w ;
            }
        else
        {   if(    XmIsLabel(w)  ||  XmIsLabelGadget(w)    )
            {   
                labels[nlabels++] = w ;
                } 
            else
            {   if(    XmIsRowColumn(w)
                    && ((XmRowColumnWidget)w)->row_column.type == XmMENU_BAR)
                {
                    menubar = w;
                    } 
                else
                {   if(    XmIsSeparator(w)  ||  XmIsSeparatorGadget(w)    )
                    {   
                        separator = w ;
                        } 
                    else
                    {   misc[nmisc++] = w ;
                        } 
                    } 
                }
            } 
        }
    geoSpec = _XmGeoMatrixAlloc( 5, nchildren, 0) ;
    geoSpec->composite = (Widget) tb ;
    geoSpec->instigator = (Widget) instigator ;
    if(    desired    )
    {   geoSpec->instig_request = *desired ;
        }
    geoSpec->margin_w = BB_MarginWidth( tb) + tb->manager.shadow_thickness ;
    geoSpec->margin_h = BB_MarginHeight( tb) + tb->manager.shadow_thickness ;
    geoSpec->no_geo_request = _XmTemplateBoxNoGeoRequest ;

    layoutPtr = geoSpec->layouts ;
    boxPtr = geoSpec->boxes ;

    if(    menubar && _XmGeoSetupKid( boxPtr, menubar)    )
    {   layoutPtr->fix_up = _XmMenuBarFix ;
        fix_menubar = True ;
        boxPtr += 2;       /* For new row, add 2. */
        ++layoutPtr;       /* For new row. */
        } 
    if(    SetupRow( tb, &boxPtr, layoutPtr, labels, nlabels, &fix_menubar)   )
    {   ++layoutPtr ;
        } 
    if(    SetupRow( tb, &boxPtr, layoutPtr, misc, nmisc, &fix_menubar)    )
    {   ++layoutPtr ;
        } 
    if(    SetupRow( tb, &boxPtr, layoutPtr, &separator, 1, &fix_menubar)    )
    {   layoutPtr->fix_up = _XmSeparatorFix ;
        ++layoutPtr ;
        } 
    if(    SetupRow( tb, &boxPtr, layoutPtr, pbuttons, npbuttons,
                                                             &fix_menubar)    )
    {   layoutPtr->space_between = 0 ; /* Override what was set in SetupRow. */
        layoutPtr->fill_mode = XmGEO_CENTER ;
        layoutPtr->fit_mode = XmGEO_WRAP ;
        if(    !tb->template_box.minimize_buttons    )
        {   layoutPtr->even_width = 1 ;
            } 
        layoutPtr->even_height = 1 ;
        ++layoutPtr ;
        } 
    layoutPtr->space_above = BB_MarginHeight( tb) ;
    layoutPtr->end = TRUE ;        /* Mark the last row. */
    XtFree( (char *) layoutChildren) ;

    return( geoSpec) ;
    }
/****************************************************************/
Boolean 
#ifdef _NO_PROTO
_XmTemplateBoxNoGeoRequest( geoSpec )
        XmGeoMatrix geoSpec ;
#else
_XmTemplateBoxNoGeoRequest(
        XmGeoMatrix geoSpec )
#endif /* _NO_PROTO */
{
/****************/

    if(    BB_InSetValues( geoSpec->composite)
        && (XtClass( geoSpec->composite) == xmTemplateBoxWidgetClass)    )
    {   
        return( TRUE) ;
        } 
    return( FALSE) ;
    }

/****************************************************************/
Widget 
#ifdef _NO_PROTO
XmCreateTemplateBox( parent, name, al, ac )
        Widget parent ;
        char *name ;
        ArgList al ;
        Cardinal ac ;
#else
XmCreateTemplateBox(
        Widget parent,
        char *name,
        ArgList al,
        Cardinal ac )
#endif /* _NO_PROTO */
{
/****************/

    return( XtCreateWidget( name, xmTemplateBoxWidgetClass, parent, al, ac)) ;
}
/****************************************************************/
Widget 
#ifdef _NO_PROTO
XmCreateTemplateDialog( parent, name, al, ac )
        Widget parent ;
        char *name ;
        ArgList al ;
        Cardinal ac ;
#else
XmCreateTemplateDialog(
        Widget parent,
        char *name,
        ArgList al,
        Cardinal ac )
#endif /* _NO_PROTO */
{
    Widget shell;
    Widget w;
    ArgList  argsNew;
    char *ds_name;
/****************/

    ds_name = XtMalloc((strlen(name)+XmDIALOG_SUFFIX_SIZE+1) * sizeof(char));
    strcpy(ds_name,name);
    strcat(ds_name,XmDIALOG_SUFFIX);

    argsNew = (ArgList) XtMalloc( sizeof( Arg) * (ac + 1)) ;
    memcpy( argsNew, al, (sizeof( Arg) * ac)) ;
    XtSetArg (argsNew[ac], XmNallowShellResize, TRUE); ac ;

    shell = XmCreateDialogShell (parent, ds_name, argsNew, (ac + 1));

    XtFree( (char *) argsNew) ;
    XtFree(ds_name);

    w = XtCreateWidget( name, xmTemplateBoxWidgetClass, shell, al, ac);

    XtAddCallback (w, XmNdestroyCallback, _XmDestroyParentCallback, NULL);

    return w;
}
