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
/* (c) Copyright 1990, 1991, 1992, 1993 HEWLETT-PACKARD COMPANY */

#include <stdio.h>
#ifdef __apollo
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <X11/Xatom.h>
#include <Xm/IconGP.h>
#include <Xm/ManagerP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/BaseClassP.h>

#ifdef I18N_MSG
#include "XmMsgI.h"
#endif

/*-------------------------------------------------------------
**	Public Interface
**-------------------------------------------------------------
*/

WidgetClass	xmIconGadgetClass;


#define SPACING_DEFAULT		2
#define MARGIN_DEFAULT		2
#define UNSPECIFIED_DIMENSION	9999
#define UNSPECIFIED_CHAR	255
#define UNSPECIFIED_STRING      3

#define Max(x, y)       (((x) > (y)) ? (x) : (y))
#define Min(x, y)       (((x) < (y)) ? (x) : (y))

#define _XmManagerDrawShadow( m, d, x, y, w, h, ht, st, t) \
        _XmDrawShadows( m, d, x, y, (w - (2 * (ht))), (h - (2 * (ht))), st, t )

#define IG_XrmName(r)		(r -> object.xrm_name)
#define IG_Expose(r,e,reg) \
 (((RectObjClassRec *)r -> object.widget_class) -> rect_class.expose)(r,e,reg)
#define IG_X(r)			(r -> rectangle.x)
#define IG_Y(r)			(r -> rectangle.y)
#define IG_Width(r)		(r -> rectangle.width)
#define IG_Height(r)		(r -> rectangle.height)
#define IG_EventMask(g)		(g -> gadget.event_mask)
#define IG_Highlighted(g)	(g -> gadget.highlighted)
#define IG_UnitType(g)		(g -> gadget.unit_type)
#define M_Background(w)		(w -> core.background_pixel)
#define M_Foreground(m)		(m -> manager.foreground)

#define C_GetSize(wc)		(wc -> icon_class.get_size)
#define C_GetPositions(wc)	(wc -> icon_class.get_positions)
#define C_Draw(wc)		(wc -> icon_class.draw)
#define C_CallCallback(wc)	(wc -> icon_class.call_callback)
#define C_OptimizeRedraw(wc)	(wc -> icon_class.optimize_redraw)
#define C_UpdateGCs(wc)		(wc -> icon_class.update_gcs)

#define Icon_Cache(w)                   (((XmIconGadget)(w))-> \
                                           icon.cache)
#define Icon_ClassCachePart(w) \
        (((XmIconGadgetClass)xmIconGadgetClass)->gadget_class.cache_part)


static char _XmMsgIcon_0000[] =
   "Incorrect alignment.";

static char _XmMsgIcon_0001[] =
   "Incorrect behavior.";

static char _XmMsgIcon_0002[] =
   "Incorrect fill mode.";

static char _XmMsgIcon_0003[] =
   "Incorrect string or pixmap position.";

static char _XmMsgIcon_0004[] =
   "Incorrect margin width or height.";

static char _XmMsgIcon_0005[] =
   "Incorrect shadow type.";

#ifdef I18N_MSG
#define WARN_ALIGNMENT          catgets(Xm_catd,MS_IconG,MSG_IG_1,\ 
					_XmMsgIcon_0000)
#define WARN_BEHAVIOR           catgets(Xm_catd,MS_IconG,MSG_IG_2,\
					_XmMsgIcon_0001)
#define WARN_FILL_MODE          catgets(Xm_catd,MS_IconG,MSG_IG_3,\
					_XmMsgIcon_0002)
#define WARN_PIXMAP_POSITION    catgets(Xm_catd,MS_IconG,MSG_IG_4,\
					_XmMsgIcon_0003)
#define WARN_MARGIN             catgets(Xm_catd,MS_IconG,MSG_IG_5,\
					_XmMsgIcon_0004)
#define WARN_SHADOW_TYPE        catgets(Xm_catd,MS_IconG,MSG_IG_6,\
					_XmMsgIcon_0005)
#else
#define WARN_ALIGNMENT		_XmMsgIcon_0000
#define WARN_BEHAVIOR		_XmMsgIcon_0001
#define WARN_FILL_MODE		_XmMsgIcon_0002
#define WARN_PIXMAP_POSITION	_XmMsgIcon_0003
#define WARN_MARGIN		_XmMsgIcon_0004
#define WARN_SHADOW_TYPE	_XmMsgIcon_0005
#endif

#define MgrParent(g) (XmIsManager(XtParent(g)) ? (XtParent(g)) : (XtParent(XtParent(g))))

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static GC GetMaskGC() ;
static void GetDefaultBackground() ;
static void GetDefaultForeground() ;
static void GetDefaultFillMode() ;
static void GetSpacing() ;
static void GetString() ;
static void IconEventHandler() ;
static void ClickTimeout() ;
static void IconArm() ;
static void IconDisarm() ;
static void IconActivate() ;
static void IconDrag() ;
static void IconDrop() ;
static void IconPopup() ;
static void IconEnter() ;
static void IconLeave() ;
static void ClassInitialize() ;
static void ClassPartInitialize() ;
static void SecondaryObjectCreate() ;
static void InitializePosthook() ;
static Boolean SetValuesPrehook() ;
static void GetValuesPrehook() ;
static void GetValuesPosthook() ;
static Boolean SetValuesPosthook() ;
static Boolean QualifyIconLocalCache() ;
static void GetParentBackgroundGC() ;
static void Initialize() ;
static void Destroy() ;
static void Resize() ;
static void Redisplay() ;
static Boolean SetValues() ;
static void BorderHighlight() ;
static void BorderUnhighlight() ;
static void ArmAndActivate() ;
static void InputDispatch() ;
static Boolean VisualChange() ;
static void GetSize() ;
static void GetPositions() ;
static void Draw() ;
static void CallCallback() ;
static void UpdateGCs() ;
static Cardinal GetIconClassSecResData() ;
static XtPointer GetIconClassResBase() ;
static Boolean LoadPixmap() ;

#else

static GC GetMaskGC( 
                        XmIconGadget g,
#if NeedWidePrototypes
                        int x,
                        int y) ;
#else
                        Position x,
                        Position y) ;
#endif /* NeedWidePrototypes */
static void GetDefaultBackground( 
                        Widget g,
                        int offset,
                        XrmValue *value) ;
static void GetDefaultForeground( 
                        Widget g,
                        int offset,
                        XrmValue *value) ;
static void GetDefaultFillMode( 
                        Widget w,
                        int offset,
                        XrmValue *value) ;
static void GetSpacing( 
                        Widget w,
                        XrmQuark resource,
                        XtArgVal *value) ;
static void GetString( 
                        Widget w,
                        XrmQuark resource,
                        XtArgVal *value) ;
static void IconEventHandler( 
                        Widget w,
                        XtPointer client_data,
                        XEvent *event) ;
static void ClickTimeout( 
                        XtPointer cd,
                        XtIntervalId *id) ;
static void IconArm( 
                        Widget w,
                        XEvent *event) ;
static void IconDisarm( 
                        Widget w,
                        XEvent *event) ;
static void IconActivate( 
                        Widget w,
                        XEvent *event) ;
static void IconDrag( 
                        Widget w,
                        XEvent *event) ;
static void IconDrop( 
                        Widget w,
                        XEvent *event) ;
static void IconPopup( 
                        Widget w,
                        XEvent *event) ;
static void IconEnter( 
                        Widget w,
                        XEvent *event) ;
static void IconLeave( 
                        Widget w,
                        XEvent *event) ;
static void ClassInitialize( void ) ;
static void ClassPartInitialize( 
                        WidgetClass wc) ;
static void SecondaryObjectCreate( 
                        Widget req,
                        Widget w_new,
                        ArgList args,
                        Cardinal *num_args) ;
static void InitializePosthook( 
                        Widget req,
                        Widget w_new,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean SetValuesPrehook( 
                        Widget oldParent,
                        Widget refParent,
                        Widget newParent,
                        ArgList args,
                        Cardinal *num_args) ;
static void GetValuesPrehook( 
                        Widget newParent,
                        ArgList args,
                        Cardinal *num_args) ;
static void GetValuesPosthook( 
                        Widget w_new,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean SetValuesPosthook( 
                        Widget current,
                        Widget req,
                        Widget w_new,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean QualifyIconLocalCache( 
                        XmIconGadget g) ;
static void GetParentBackgroundGC( 
                        XmIconGadget g) ;
static void Initialize( 
                        Widget request_w,
                        Widget new_w,
                        ArgList args,
                        Cardinal *numArgs) ;
static void Destroy( 
                        Widget w) ;
static void Resize( 
                        Widget w) ;
static void Redisplay( 
                        Widget w,
                        XEvent *event,
                        Region region) ;
static Boolean SetValues( 
                        Widget current_w,
                        Widget request_w,
                        Widget new_w,
                        ArgList args,
                        Cardinal *num_args) ;
static void BorderHighlight( 
                        Widget gw) ;
static void BorderUnhighlight( 
                        Widget gw) ;
static void ArmAndActivate( 
                        Widget w,
                        XEvent *event,
                        String *params,
                        Cardinal *numParams) ;
static void InputDispatch( 
                        Widget w,
                        XEvent *event,
                        Mask event_mask) ;
static Boolean VisualChange( 
                        Widget w,
                        Widget current_w,
                        Widget new_w) ;
static void GetSize( 
                        Widget gw,
                        Dimension *w,
                        Dimension *h) ;
static void GetPositions( 
                        Widget gw,
#if NeedWidePrototypes
                        int w,
                        int h,
                        int h_t,
                        int s_t,
#else
                        Position w,
                        Position h,
                        Dimension h_t,
                        Dimension s_t,
#endif /* NeedWidePrototypes */
                        Position *pix_x,
                        Position *pix_y,
                        Position *str_x,
                        Position *str_y) ;
static void Draw( 
                        Widget gw,
                        Drawable drawable,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int w,
                        int h,
                        int h_t,
                        int s_t,
                        unsigned int s_type,
                        unsigned int fill_mode) ;
#else
                        Position x,
                        Position y,
                        Dimension w,
                        Dimension h,
                        Dimension h_t,
                        Dimension s_t,
                        unsigned char s_type,
                        unsigned char fill_mode) ;
#endif /* NeedWidePrototypes */
static void CallCallback( 
                        Widget g,
                        XtCallbackList cb,
                        int reason,
                        XEvent *event) ;
static void UpdateGCs( 
                        Widget gw) ;
static Cardinal GetIconClassSecResData( 
                        WidgetClass w_class,
                        XmSecondaryResourceData **data_rtn) ;
static XtPointer GetIconClassResBase( 
                        Widget widget,
                        XtPointer client_data) ;
static Boolean LoadPixmap( 
                        XmIconGadget w_new,
                        String pixmap) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/*-------------------------------------------------------------
**	Resource List
*/
#define I_Offset(field) \
	XtOffset (XmIconGadget, icon.field)

#define I_Cache_Offset(field) \
	XtOffset (XmIconCacheObject, icon_cache.field)

static XtResource resources[] = 
{
	{
		XmNset,
		XmCSet, XmRBoolean, sizeof (Boolean),
		I_Offset (set), XmRImmediate, (XtPointer) False
	},
	{
		XmNshadowType,
		XmCShadowType, XmRShadowType, sizeof (unsigned char),
		I_Offset (shadow_type),
		XmRImmediate, (XtPointer) XmSHADOW_ETCHED_OUT
	},
	{
		XmNborderType,
		XmCBorderType, XmRBorderType, sizeof (unsigned char),
		I_Offset (border_type),
		XmRImmediate, (XtPointer) XmRECTANGLE
	},
	{
		XmNcallback,
		XmCCallback, XmRCallback, sizeof (XtCallbackList),
		I_Offset (callback), XmRImmediate, (XtPointer) NULL
	},
	{
		XmNfontList,
		XmCFontList, XmRFontList, sizeof (XmFontList),
		I_Offset (font_list), XmRString, "Fixed"
	},
	{
		XmNimageName,
		XmCString, XmRString, sizeof (String),
		I_Offset (image_name), XmRImmediate, (XtPointer) NULL
	},
	{
		XmNpixmap,
		XmCPixmap, XmRPixmap, sizeof (Pixmap),
		I_Offset (pixmap),
		XmRImmediate, (XtPointer) XmUNSPECIFIED_PIXMAP
	},
	{
		XmNstring,
		XmCXmString, XmRXmString, sizeof (_XmString),
		I_Offset (string),
		XmRImmediate, (XtPointer) UNSPECIFIED_STRING
	},
	{
		XmNpixmapForeground,
		XmCForeground, XmRPixel, sizeof (Pixel),
		I_Offset (pixmap_foreground),
		XmRCallProc, (XtPointer) GetDefaultForeground
	},
	{
		XmNpixmapBackground,
		XmCBackground, XmRPixel, sizeof (Pixel),
		I_Offset (pixmap_background),
		XmRCallProc, (XtPointer) GetDefaultBackground
	},
	{
		XmNunderline,
		XmCUnderline, XmRBoolean, sizeof (Boolean),
		I_Offset (underline), XmRImmediate, (XtPointer) False
	}
};


static XtResource cache_resources[] =
{
	{
		XmNbehavior,
		XmCBehavior, XmRBehavior, sizeof (unsigned char),
		I_Cache_Offset (behavior),
		XmRImmediate, (XtPointer) XmICON_BUTTON
	},
	{
		XmNrecomputeSize,
		XmCRecomputeSize, XmRBoolean, sizeof (Boolean),
		I_Cache_Offset (recompute_size), XmRImmediate, (XtPointer) True
	},
	{
		XmNfillOnArm,
		XmCFillOnArm, XmRBoolean, sizeof (Boolean),
		I_Cache_Offset (fill_on_arm), XmRImmediate, (XtPointer) True
	},
	{
		XmNforeground,
		XmCForeground, XmRPixel, sizeof (Pixel),
		I_Cache_Offset (foreground),
		XmRCallProc, (XtPointer) GetDefaultForeground
	},
	{
		XmNbackground,
		XmCBackground, XmRPixel, sizeof (Pixel),
		I_Cache_Offset (background),
		XmRCallProc, (XtPointer) GetDefaultBackground
	},
	{
		XmNarmColor,
		XmCArmColor, XmRPixel, sizeof (Pixel),
		I_Cache_Offset (arm_color),
		XmRCallProc, (XtPointer) _XmSelectColorDefault
	},
	{
		XmNspacing,
		XmCSpacing, XmRDimension, sizeof (Dimension),
		I_Cache_Offset (spacing),
		XmRImmediate, (XtPointer) UNSPECIFIED_DIMENSION
	},
	{
		XmNmarginHeight,
		XmCMarginHeight, XmRDimension, sizeof (Dimension),
		I_Cache_Offset (margin_height),
		XmRImmediate, (XtPointer) UNSPECIFIED_DIMENSION
	},
	{
		XmNmarginWidth,
		XmCMarginWidth, XmRDimension, sizeof (Dimension),
		I_Cache_Offset (margin_width),
		XmRImmediate, (XtPointer) UNSPECIFIED_DIMENSION
	},
	{
		XmNpixmapPosition,
		XmCPixmapPosition, XmRPixmapPosition, sizeof (unsigned char),
		I_Cache_Offset (pixmap_position),
		XmRImmediate, (XtPointer) UNSPECIFIED_CHAR
	},
	{
		XmNstringPosition,
		XmCStringPosition, XmRStringPosition, sizeof (unsigned char),
		I_Cache_Offset (string_position),
		XmRImmediate, (XtPointer) UNSPECIFIED_CHAR
	},
	{
		XmNalignment,
		XmCAlignment, XmRAlignment, sizeof (unsigned char),
		I_Cache_Offset (alignment),
		XmRImmediate, (XtPointer) XmALIGNMENT_BEGINNING
	},
	{
		XmNfillMode,
		XmCFillMode, XmRFillMode, sizeof (unsigned char),
		I_Cache_Offset (fill_mode),
		XmRCallProc, (XtPointer) GetDefaultFillMode
	}
};


static XmSyntheticResource syn_resources[] =
{
	{
		XmNstring, sizeof (_XmString),
		I_Offset (string), GetString, NULL
	},
};


static XmSyntheticResource cache_syn_resources[] =
{
	{
		XmNspacing, sizeof (Dimension),
		I_Cache_Offset (spacing), GetSpacing, NULL
	},
	{
		XmNmarginWidth, sizeof (Dimension),
		I_Cache_Offset (margin_width),
		_XmFromHorizontalPixels, 
                _XmToHorizontalPixels
	},
	{
		XmNmarginHeight, sizeof (Dimension),
		I_Cache_Offset (margin_height),
		_XmFromVerticalPixels, 
                _XmToVerticalPixels, 
	}
};

#undef	I_Offset
#undef	I_Cache_Offset
	

/*-------------------------------------------------------------
**	Cache Class Record
*/

static XmCacheClassPart IconGClassCachePart = {
        {NULL, 0, 0},            /* head of class cache list */
        _XmCacheCopy,           /* Copy routine         */
        _XmCacheDelete,         /* Delete routine       */
        _XmIconGCacheCompare,       /* Comparison routine   */
};

/*-------------------------------------------------------------
**	Base Class Extension Record
*/
static XmBaseClassExtRec       iconGBaseClassExtRec = {
    NULL,                                     /* Next extension       */
    NULLQUARK,                                /* record type XmQmotif */
    XmBaseClassExtVersion,                    /* version              */
    sizeof(XmBaseClassExtRec),                /* size                 */
    XmInheritInitializePrehook,               /* initialize prehook   */
    SetValuesPrehook,                         /* set_values prehook   */
    InitializePosthook,                       /* initialize posthook  */
    SetValuesPosthook,                        /* set_values posthook  */
    (WidgetClass)&xmIconGCacheObjClassRec,    /* secondary class      */
    SecondaryObjectCreate,                    /* creation proc        */
    GetIconClassSecResData,                   /* getSecResData */
    {NULL},                                   /* fast subclass        */
    GetValuesPrehook,                         /* get_values prehook   */
    GetValuesPosthook,                        /* get_values posthook  */
};

/*-------------------------------------------------------------
**	Icon Cache Object Class Record
*/
externaldef (xmiconcacheobjclassrec)
XmIconGCacheObjClassRec xmIconGCacheObjClassRec =
{
  {
      /* superclass         */    (WidgetClass) &xmExtClassRec,
      /* class_name         */    "XmIcon",
      /* widget_size        */    sizeof(XmIconGCacheObjRec),
      /* class_initialize   */    NULL,
      /* chained class init */    NULL,
      /* class_inited       */    False,
      /* initialize         */    NULL,
      /* initialize hook    */    NULL,
      /* realize            */    NULL,
      /* actions            */    NULL,
      /* num_actions        */    0,
      /* resources          */    cache_resources,
      /* num_resources      */    XtNumber(cache_resources),
      /* xrm_class          */    NULLQUARK,
      /* compress_motion    */    False,
      /* compress_exposure  */    False,
      /* compress enter/exit*/    False,
      /* visible_interest   */    False,
      /* destroy            */    NULL,
      /* resize             */    NULL,
      /* expose             */    NULL,
      /* set_values         */    NULL,
      /* set values hook    */    NULL,
      /* set values almost  */    NULL,
      /* get values hook    */    NULL,
      /* accept_focus       */    NULL,
      /* version            */    XtVersion,
      /* callback offsetlst */    NULL,
      /* default trans      */    NULL,
      /* query geo proc     */    NULL,
      /* display accelerator*/    NULL,
      /* extension record   */    NULL,
   },

   {
      /* synthetic resources */   cache_syn_resources,
      /* num_syn_resources   */   XtNumber(cache_syn_resources),
      /* extension           */   NULL,
   },
};

/*-------------------------------------------------------------
**	Class Record
*/
externaldef (xmiconclassrec) XmIconGadgetClassRec xmIconGadgetClassRec =
{
	/*	Core Part
	*/
	{	
		(WidgetClass) &xmGadgetClassRec, /* superclass		*/
		"XmIcon",			/* class_name		*/
		sizeof (XmIconGadgetRec),		/* widget_size		*/
		ClassInitialize,		/* class_initialize	*/
		ClassPartInitialize,		/* class_part_initialize*/
		False,				/* class_inited		*/
		Initialize,             	/* initialize		*/
		NULL,				/* initialize_hook	*/
		NULL,				/* realize		*/
		NULL,				/* actions		*/
		0,				/* num_actions		*/
		resources,			/* resources		*/
		XtNumber (resources),		/* num_resources	*/
		NULLQUARK,			/* xrm_class		*/
		True,				/* compress_motion	*/
		True,				/* compress_exposure	*/
		True,				/* compress_enterleave	*/
		False,				/* visible_interest	*/	
		Destroy, 	                /* destroy		*/	
		Resize,		                /* resize		*/
		Redisplay,	                /* expose		*/	
		SetValues,	                /* set_values		*/	
		NULL,				/* set_values_hook	*/
		XtInheritSetValuesAlmost,	/* set_values_almost	*/
		NULL,				/* get_values_hook	*/
		NULL,				/* accept_focus		*/	
		XtVersion,			/* version		*/
		NULL,				/* callback private	*/
		NULL,				/* tm_table		*/
		NULL,				/* query_geometry	*/
		NULL,				/* display_accelerator	*/
		(XtPointer)&iconGBaseClassExtRec,/* extension		*/
	},

	/*	XmGadget Part
	*/
	{
	        BorderHighlight,	        /* border_highlight	*/
                BorderUnhighlight,	        /* border_unhighlight	*/
                ArmAndActivate,		        /* arm_and_activate	*/
		InputDispatch,	                /* input_dispatch	*/
		VisualChange,	                /* visual_change	*/
		syn_resources,			/* get_resources	*/
		XtNumber (syn_resources),	/* num_get_resources	*/
		&IconGClassCachePart,		/* class_cache_part	*/
		NULL,	 			/* extension		*/
	},

	/*	XmIconGadget Part
	*/
	{
		GetSize,			/* get_size		*/
		GetPositions,			/* get_positions	*/
		Draw,				/* draw			*/
		CallCallback,			/* call_callback	*/
		UpdateGCs,			/* update_gcs		*/
		True,				/* optimize_redraw	*/
		NULL,				/* class_cache_part	*/
		NULL,				/* extension		*/
	}
};


externaldef (xmicongadgetclass) WidgetClass xmIconGadgetClass = (WidgetClass) &xmIconGadgetClassRec;


/*-------------------------------------------------------------
**	Private Functions
**-------------------------------------------------------------
*/



/*-------------------------------------------------------------
**	GetMaskGC
**		Get normal and background graphics contexts.
*/
static GC
#ifdef _NO_PROTO
GetMaskGC( g, x, y )
    XmIconGadget g ;
    Position x, y;
#else
GetMaskGC(
        XmIconGadget g,
#if NeedWidePrototypes
        int x,
        int y)
#else
        Position x,
        Position y)
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    if (IG_Mask(g) != XmUNSPECIFIED_PIXMAP) {

	XSetClipOrigin(XtDisplay(g),
		       IG_ClipGC(g),
		       x, y);
	return IG_ClipGC(g);
    }
    else {
	return IG_NormalGC(g);
    }
}

/*-------------------------------------------------------------
**	GetDefaultBackground
**		Copy background pixel from Manager parent.
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetDefaultBackground( g, offset, value )
        Widget g ;
        int offset ;
        XrmValue *value ;
#else
GetDefaultBackground(
        Widget g,
        int offset,
        XrmValue *value )
#endif /* _NO_PROTO */
{
	static Pixel		pixel;
	XmManagerWidget		parent = (XmManagerWidget) XtParent (g);

	value->addr = (XtPointer) &pixel;
	value->size = sizeof (Pixel);

	if (XmIsManager ((Widget) parent))
		pixel = M_Background (parent);
	else
		_XmBackgroundColorDefault (g, offset, value);
}


/*-------------------------------------------------------------
**	GetDefaultForeground
**		Copy foreground pixel from Manager parent.
*/
static void 
#ifdef _NO_PROTO
GetDefaultForeground( g, offset, value )
        Widget g ;
        int offset ;
        XrmValue *value ;
#else
GetDefaultForeground(
        Widget g,
        int offset,
        XrmValue *value )
#endif /* _NO_PROTO */
{
	static Pixel		pixel;
	XmManagerWidget		parent = (XmManagerWidget) XtParent (g);

	value->addr = (XtPointer) &pixel;
	value->size = sizeof (Pixel);

	if (XmIsManager ((Widget) parent))
		pixel = M_Foreground (parent);
	else
		_XmForegroundColorDefault (g, offset, value);
}



/*-------------------------------------------------------------
**	GetDefaultFillMode
**		Get default fill mode.
*/
static void 
#ifdef _NO_PROTO
GetDefaultFillMode( w, offset, value )
        Widget w ;
        int offset ;
        XrmValue *value ;
#else
GetDefaultFillMode(
        Widget w,
        int offset,
        XrmValue *value )
#endif /* _NO_PROTO */
{
	static unsigned char	fill_mode;
	XmIconGadget	g =	(XmIconGadget) w;

	value->addr = (XtPointer) &fill_mode;
	value->size = sizeof (unsigned char);

	if (G_ShadowThickness (g) == 0)
		fill_mode = XmFILL_PARENT;
	else
		fill_mode = XmFILL_SELF;
}


/*-------------------------------------------------------------
**	GetSpacing
**		Convert from pixels to horizontal or vertical units.
*/
static void 
#ifdef _NO_PROTO
GetSpacing( w, resource, value )
        Widget w ;
        XrmQuark resource ;
        XtArgVal *value ;
#else
GetSpacing(
        Widget w,
        XrmQuark resource,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
	XmIconGadget	g =	(XmIconGadget) w;

	if (IG_PixmapPosition (g) == XmPIXMAP_TOP ||
	    IG_PixmapPosition (g) == XmPIXMAP_BOTTOM)
		_XmFromVerticalPixels ((Widget)g, resource, value);
	else
		_XmFromHorizontalPixels ((Widget)g, resource, value);
}



/*-------------------------------------------------------------
**	GetString
**		Convert string from internal to external form.
*/
static void 
#ifdef _NO_PROTO
GetString( w, resource, value )
        Widget w ;
        XrmQuark resource ;
        XtArgVal *value ;
#else
GetString(
        Widget w,
        XrmQuark resource,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
	XmIconGadget	g =	(XmIconGadget) w;
	XmString	string;

	string = _XmStringCreateExternal (IG_FontList (g), IG_String (g));

	*value = (XtArgVal) string;
}


/*-------------------------------------------------------------
**	IconEventHandler
**		Event handler for middle button events.
*/
static void 
#ifdef _NO_PROTO
IconEventHandler( w, client_data, event )
        Widget w ;
        XtPointer client_data ;
        XEvent *event ;
#else
IconEventHandler(
        Widget w,
        XtPointer client_data,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadget		g =	NULL;

	g = (XmIconGadget) _XmInputInGadget (w, event->xbutton.x, event->xbutton.y);
	
	if (!g || !XmIsIconGadget ((Widget)g))
		return;

	if (event->xbutton.button == Button2 || event->xbutton.button == Button3)
	{
		if (event->xbutton.type == ButtonPress)
			InputDispatch ((Widget) g, event, XmARM_EVENT);
		else if (event->xbutton.type == ButtonRelease)
			InputDispatch ((Widget) g, event, XmACTIVATE_EVENT);
	}
}



/*-------------------------------------------------------------
**	ClickTimeout
**		Clear Click flags.
*/
static void
#ifdef _NO_PROTO
ClickTimeout( cd, id )
        XtPointer cd ;
        XtIntervalId *id ;
#else
ClickTimeout(
        XtPointer cd,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
	Widget		shell;
	XmIconGadget	g = (XmIconGadget) cd;
	Time		last_time, end_time;

	if (! IG_Armed (g))
	{
		IG_ClickTimerID (g) = 0;
		XtFree ((char *)IG_ClickEvent (g));
		IG_ClickEvent (g) = NULL;
		return;
	}

	last_time = XtLastTimestampProcessed (XtDisplay (g));
	end_time = IG_ClickEvent (g) -> time + (Time)
				XtGetMultiClickTime (XtDisplay (g)); 

/*	Sync and reset timer if server interval may not have elapsed.
*/
	if ((last_time < end_time) && IG_Sync (g))
	{
		IG_Sync (g) = False;
		XSync (XtDisplay (g), False);
		IG_ClickTimerID (g) = 
			XtAppAddTimeOut (XtWidgetToApplicationContext ((Widget)g),
					(unsigned long) 50, 
					ClickTimeout, 
					(XtPointer) g);
	}
/*	Handle Select action.
*/
	else
	{
		IG_ClickTimerID (g) = 0;
		IG_Armed (g) = False;
		IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_SELECT,
					(XEvent *)IG_ClickEvent (g));

	        if ((IG_Behavior (g) == XmICON_DRAG) &&
                    (G_ShadowThickness (g) == 0))
                {
                   /* Do nothing */
                }
                else
		   IG_Expose ((Widget)g, (XEvent*)IG_ClickEvent (g), NULL);
		XtFree ((char *)IG_ClickEvent (g));
		IG_ClickEvent (g) = NULL;
	}
}



/*-------------------------------------------------------------
**	Action Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	IconArm
**		Handle Arm action.
*/
static void 
#ifdef _NO_PROTO
IconArm( w, event )
        Widget w ;
        XEvent *event ;
#else
IconArm(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadget	g = 	(XmIconGadget) w;

	if (IG_Armed (g) || IG_Behavior (g) == XmICON_LABEL)
		return;

	IG_Armed (g) = True;

	if (IG_Behavior (g) == XmICON_DRAG)
		IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_ARM, event);

	if ((IG_Behavior (g) == XmICON_DRAG) &&
            (G_ShadowThickness (g) == 0))
        {
           /* Do nothing */
        }
        else
	   IG_Expose ((Widget)g, event, NULL);
}



/*-------------------------------------------------------------
**	IconDisarm
**		Handle Disarm action.
*/
static void 
#ifdef _NO_PROTO
IconDisarm( w, event )
        Widget w ;
        XEvent *event ;
#else
IconDisarm(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadget	g = 	(XmIconGadget) w;

	if (! IG_Armed (g) || IG_Behavior (g) == XmICON_LABEL)
		return;

	IG_Armed (g) = False;

if (IG_Behavior (g) == XmICON_DRAG)
	{
	IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_DISARM, event);
	IG_Expose ((Widget)g, event, NULL);
	}
}



/*-------------------------------------------------------------
**	IconActivate
**		Handle Activate action.
*/
static void 
#ifdef _NO_PROTO
IconActivate( w, event )
        Widget w ;
        XEvent *event ;
#else
IconActivate(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadget		g = 	(XmIconGadget) w;
	unsigned long		delay;
	XButtonEvent *		b_event = (XButtonEvent *) event;

	if (! IG_Armed (g))
		return;

	if (IG_Behavior (g) == XmICON_BUTTON)
	{
		IG_Armed (g) = False;
		IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_ACTIVATE, event);
		IG_Expose ((Widget)g, event, NULL);
	}

	else if (IG_Behavior (g) == XmICON_TOGGLE)
	{
		IG_Armed (g) = False;
		IG_Set (g) = ! IG_Set (g);

		IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_VALUE_CHANGED, event);
	}

	else if (IG_Behavior (g) == XmICON_DRAG)
	{
		if (IG_ClickTimerID (g))
		{
			IG_ClickTimerID (g) = 0;
			XtFree ((char *)IG_ClickEvent (g));
			IG_ClickEvent (g) = NULL;
			IG_Armed (g) = False;
			IG_CallCallback ((Widget) g, IG_Callback (g),
					XmCR_DEFAULT_ACTION, event);
		}
		else
		{
			delay = (unsigned long)
				XtGetMultiClickTime (XtDisplay (g)); 
			IG_ClickEvent (g) = (XButtonEvent *)
				XtMalloc (sizeof (XButtonEvent));
			*(IG_ClickEvent (g)) = *b_event;
			IG_Sync (g) = True;
			IG_ClickTimerID (g) = 
				XtAppAddTimeOut (
					XtWidgetToApplicationContext ((Widget)g),
					delay, ClickTimeout, 
					(XtPointer) g);
		}

                if (G_ShadowThickness (g) > 0)
		   IG_Expose ((Widget)g, event, NULL);
	}
}



/*-------------------------------------------------------------
**	IconDrag
**		Handle Drag action.
*/
static void 
#ifdef _NO_PROTO
IconDrag( w, event )
        Widget w ;
        XEvent *event ;
#else
IconDrag(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadget	g = 	(XmIconGadget) w;

	if (IG_Behavior (g) == XmICON_DRAG)
		IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_DRAG, event);
}


/*-------------------------------------------------------------
**	IconDrop
**		Handle Drop action.
*/
static void 
#ifdef _NO_PROTO
IconDrop( w, event )
        Widget w ;
        XEvent *event ;
#else
IconDrop(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadget	g = 	(XmIconGadget) w;

	if (IG_Behavior (g) == XmICON_DRAG)
		IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_DROP, event);
}
 
/*-------------------------------------------------------------
**      IconPopup
**            Handle button 3 - popup's
*/
static void
#ifdef _NO_PROTO
IconPopup( w, event )
        Widget w ;
        XEvent *event ;
#else
IconPopup(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
        XmIconGadget   g =     (XmIconGadget) w;

        IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_POPUP, event);
}



/*-------------------------------------------------------------
**	IconEnter
**		Handle Enter action.
*/
static void 
#ifdef _NO_PROTO
IconEnter( w, event )
        Widget w ;
        XEvent *event ;
#else
IconEnter(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadget	g = 	(XmIconGadget) w;

	_XmEnterGadget (w, event, NULL, NULL);

	if (IG_Armed (g))
		{
		if ((IG_Behavior (g) == XmICON_BUTTON) ||
		    (IG_Behavior (g) == XmICON_TOGGLE))
			IG_Expose ((Widget)g, event, NULL);
		}
}


/*-------------------------------------------------------------
**	IconLeave
**		Handle Leave action.
*/
static void 
#ifdef _NO_PROTO
IconLeave( w, event )
        Widget w ;
        XEvent *event ;
#else
IconLeave(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadget	g = 	(XmIconGadget) w;

	_XmLeaveGadget (w, event, NULL, NULL);

	if (IG_Armed (g))
	{
	if ((IG_Behavior (g) == XmICON_BUTTON) ||
	    (IG_Behavior (g) == XmICON_TOGGLE))
		{
		IG_Armed (g) = False;
		IG_Expose ((Widget)g, event, NULL);
		IG_Armed (g) = True;
		}
	}
}



/*-------------------------------------------------------------
**	Core Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	ClassInitialize
**		Initialize gadget class.
*/
static void 
#ifdef _NO_PROTO
ClassInitialize()
#else
ClassInitialize( void )
#endif /* _NO_PROTO */
{
	iconGBaseClassExtRec.record_type = XmQmotif;
}

/*-------------------------------------------------------------
**	ClassPartInitialize
**		Initialize gadget class part.
*/
static void
#ifdef _NO_PROTO
ClassPartInitialize (wc)
	WidgetClass	wc;
#else
ClassPartInitialize (
	WidgetClass	wc)
#endif
{
	XmIconGadgetClass ic =	(XmIconGadgetClass) wc;
	XmIconGadgetClass super =	(XmIconGadgetClass) ic->rect_class.superclass;

	if (C_GetSize (ic) == (XmGetSizeProc) _XtInherit)
		C_GetSize (ic) = C_GetSize (super);
	if (C_GetPositions (ic) == (XmGetPositionProc) _XtInherit)
		C_GetPositions (ic) = C_GetPositions (super);
	if (C_Draw (ic) == (XmDrawProc) _XtInherit)
		C_Draw (ic) = C_Draw (super);
	if (C_CallCallback (ic) == (XmCallCallbackProc) _XtInherit)
		C_CallCallback (ic) = C_CallCallback (super);
	if (C_UpdateGCs (ic) == (XmUpdateGCsProc) _XtInherit)
		C_UpdateGCs (ic) = C_UpdateGCs (super);
}

/*-------------------------------------------------------------
**	Cache Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**      _XmIconGCacheCompare
**
*/
int 
#ifdef _NO_PROTO
_XmIconGCacheCompare( ii, ici )
        XtPointer ii ;
        XtPointer ici ;
#else
_XmIconGCacheCompare(
        XtPointer ii,
        XtPointer ici)
#endif /* _NO_PROTO */
{
        XmIconGCacheObjPart *icon_inst = (XmIconGCacheObjPart *) ii ;
        XmIconGCacheObjPart *icon_cache_inst = (XmIconGCacheObjPart *) ici ;

   if ((icon_inst->fill_on_arm == icon_cache_inst->fill_on_arm) &&
       (icon_inst->recompute_size== icon_cache_inst->recompute_size) &&
       (icon_inst->pixmap_position== icon_cache_inst->pixmap_position) &&
       (icon_inst->string_position== icon_cache_inst->string_position) &&
       (icon_inst->alignment == icon_cache_inst->alignment) &&
       (icon_inst->behavior == icon_cache_inst->behavior) &&
       (icon_inst->fill_mode == icon_cache_inst->fill_mode) &&
       (icon_inst->margin_width== icon_cache_inst->margin_width) &&
       (icon_inst->margin_height== icon_cache_inst->margin_height) &&
       (icon_inst->string_height== icon_cache_inst->string_height) &&
       (icon_inst->spacing== icon_cache_inst->spacing) &&
       (icon_inst->foreground== icon_cache_inst->foreground) &&
       (icon_inst->background== icon_cache_inst->background) &&
       (icon_inst->arm_color== icon_cache_inst->arm_color))
       return 1;
   else
       return 0;

}


/*-------------------------------------------------------------
**      SecondaryObjectCreate
**
*/
static void 
#ifdef _NO_PROTO
SecondaryObjectCreate( req, w_new, args, num_args )
        Widget req ;
        Widget w_new ;
        ArgList args ;
        Cardinal *num_args ;
#else
SecondaryObjectCreate(
        Widget req,
        Widget w_new,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
  XmBaseClassExt              *cePtr;
  Arg                         myArgs[2];
  ArgList                     mergedArgs;
  XmWidgetExtData             extData;

  XtSetArg(myArgs[0] ,XmNlogicalParent, w_new);
  XtSetArg(myArgs[1] ,XmNextensionType, XmCACHE_EXTENSION);

    if (*num_args)
      mergedArgs = XtMergeArgLists(args, *num_args, myArgs, XtNumber(myArgs));
    else
      mergedArgs = myArgs;

    cePtr = _XmGetBaseClassExtPtr(XtClass(w_new), XmQmotif);

    (void) XtCreateWidget(XtName(w_new),
                         (*cePtr)->secondaryObjectClass,
                         XtParent(w_new),
                         mergedArgs, *num_args + 2);

    extData = _XmGetWidgetExtData(w_new, XmCACHE_EXTENSION);

    if (mergedArgs != myArgs)
      XtFree ((char *)mergedArgs);

    /*
     * fill out cache pointers
     */
    Icon_Cache(w_new) = &(((XmIconCacheObject)extData->widget)->icon_cache);
    Icon_Cache(req) = &(((XmIconCacheObject)extData->reqWidget)->icon_cache);

}


/*-------------------------------------------------------------
**      InitializePostHook
**
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
InitializePosthook( req, w_new, args, num_args )
        Widget req ;
        Widget w_new ;
        ArgList args ;
        Cardinal *num_args ;
#else
InitializePosthook(
        Widget req,
        Widget w_new,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmWidgetExtData     ext;
    XmIconGadget lw = (XmIconGadget)w_new;
    XmIconGadget rw = (XmIconGadget)req;

    /*
     * - register parts in cache.
     * - update cache pointers
     * - and free req
     */

     Icon_Cache(lw) = (XmIconGCacheObjPart *)
           _XmCachePart( Icon_ClassCachePart(lw),
                         Icon_Cache(lw),
                         sizeof(XmIconGCacheObjPart));

    /*
     * might want to break up into per-class work that gets explicitly
     * chained. For right now, each class has to replicate all
     * superclass logic in hook routine
     */

    /*     * free the req subobject used for comparisons
     */
     ext = _XmGetWidgetExtData((Widget)lw, XmCACHE_EXTENSION);
     _XmExtObjFree((XtPointer)ext->reqWidget);
     XtDestroyWidget(ext->widget);
     /* extData gets freed at destroy */
}

/*-------------------------------------------------------------
**	SetValuesPrehook
**
*/
/* ARGSUSED */
static Boolean 
#ifdef _NO_PROTO
SetValuesPrehook( oldParent, refParent, newParent, args, num_args )
        Widget oldParent ;
        Widget refParent ;
        Widget newParent ;
        ArgList args ;
        Cardinal *num_args ;
#else
SetValuesPrehook(
        Widget oldParent,
        Widget refParent,
        Widget newParent,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmWidgetExtData             extData;
    XmBaseClassExt              *cePtr;
    WidgetClass                 ec;
    XmIconCacheObject         w_new;

    cePtr = _XmGetBaseClassExtPtr(XtClass(newParent), XmQmotif);
    ec = (*cePtr)->secondaryObjectClass;

    /* allocate copies and fill from cache */
    w_new = (XmIconCacheObject) _XmExtObjAlloc(ec->core_class.widget_size);

    w_new->object.self = (Widget)w_new;
    w_new->object.widget_class = ec;
    w_new->object.parent = XtParent(newParent);
    w_new->object.xrm_name = newParent->core.xrm_name;
    w_new->object.being_destroyed = False;
    w_new->object.destroy_callbacks = NULL;
    w_new->object.constraints = NULL;

    w_new->ext.logicalParent = newParent;
    w_new->ext.extensionType = XmCACHE_EXTENSION;

    memcpy((char *)&(w_new->icon_cache),
          (char *)Icon_Cache(newParent),
          sizeof(XmIconGCacheObjPart));

    extData = (XmWidgetExtData) XtCalloc(sizeof(XmWidgetExtDataRec), 1);
    _XmPushWidgetExtData(newParent, extData, XmCACHE_EXTENSION);

    XtSetValues((Widget)w_new, args, *num_args);

    Icon_Cache(newParent) = &(((XmIconCacheObject)w_new)->icon_cache);
    Icon_Cache(refParent) =
			&(((XmIconCacheObject)extData->reqWidget)->icon_cache);

    return FALSE;
}


/*-------------------------------------------------------------
**	GetValuesPrehook
**
*/
static void 
#ifdef _NO_PROTO
GetValuesPrehook( newParent, args, num_args )
        Widget newParent ;
        ArgList args ;
        Cardinal *num_args ;
#else
GetValuesPrehook(
        Widget newParent,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmIconGadget  lg = (XmIconGadget)newParent;

    XmWidgetExtData             extData;
    XmBaseClassExt              *cePtr;
    WidgetClass                 ec;
    XmIconCacheObject         w_new;

    cePtr = _XmGetBaseClassExtPtr(XtClass(newParent), XmQmotif);
    ec = (*cePtr)->secondaryObjectClass;

    /* allocate copies and fill from cache */
    w_new = (XmIconCacheObject) _XmExtObjAlloc(ec->core_class.widget_size);

    w_new->object.self = (Widget)w_new;
    w_new->object.widget_class = ec;
    w_new->object.parent = XtParent(newParent);
    w_new->object.xrm_name = newParent->core.xrm_name;
    w_new->object.being_destroyed = False;
    w_new->object.destroy_callbacks = NULL;
    w_new->object.constraints = NULL;

    w_new->ext.logicalParent = newParent;
    w_new->ext.extensionType = XmCACHE_EXTENSION;

    memcpy((char *)&(w_new->icon_cache),
          (char *)Icon_Cache(newParent),
          sizeof(XmIconGCacheObjPart));

    extData = (XmWidgetExtData) XtCalloc(sizeof(XmWidgetExtDataRec), 1);
    _XmPushWidgetExtData(newParent, extData, XmCACHE_EXTENSION);

    XtGetValues((Widget)w_new, args, *num_args);

}



/*-------------------------------------------------------------
**	GetValuesPosthook
**
*/
static void 
#ifdef _NO_PROTO
GetValuesPosthook( w_new, args, num_args )
        Widget w_new ;
        ArgList args ;
        Cardinal *num_args ;
#else
GetValuesPosthook(
        Widget w_new,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
 XmWidgetExtData             ext;

 _XmPopWidgetExtData(w_new, &ext, XmCACHE_EXTENSION);

 _XmExtObjFree((XtPointer)ext->widget);
 XtFree((char *)ext);
}



/*-------------------------------------------------------------
**	SetValuesPosthook
**
*/
static Boolean 
#ifdef _NO_PROTO
SetValuesPosthook( current, req, w_new, args, num_args )
        Widget current ;
        Widget req ;
        Widget w_new ;
        ArgList args ;
        Cardinal *num_args ;
#else
SetValuesPosthook(
        Widget current,
        Widget req,
        Widget w_new,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmIconGadget lg = (XmIconGadget)w_new;
    XmIconGadget cg = (XmIconGadget)current;

    XmWidgetExtData             ext;
    XmIconGCacheObjPart        *oldCachePtr, *newCachePtr;

    /*
     * - register parts in cache.
     * - update cache pointers
     * - and free req
     */


      /* assign if changed! */
      if (!_XmIconGCacheCompare(Icon_Cache(w_new),
                          Icon_Cache(current)))

      {
         _XmCacheDelete(Icon_Cache(current));  /* delete the old one */
          Icon_Cache(w_new) = (XmIconGCacheObjPart *)
              _XmCachePart(Icon_ClassCachePart(w_new),
                           Icon_Cache(w_new),
                           sizeof(XmIconGCacheObjPart));
      } else
          Icon_Cache(w_new) = Icon_Cache(current);

      _XmPopWidgetExtData(w_new, &ext, XmCACHE_EXTENSION);

      _XmExtObjFree((XtPointer)ext->widget);
      _XmExtObjFree((XtPointer)ext->reqWidget);
      _XmExtObjFree((XtPointer)ext->oldWidget);
      XtFree((char *)ext);

      return FALSE;
}


/*--------------------------------------------------------------------------
**      Cache Assignment Help
**     		These routines are for manager widgets that go into Icon's
**     		fields and set them, instead of doing a SetValues.
**--------------------------------------------------------------------------
*/

static XmIconGCacheObjPart local_cache;
static Boolean local_cache_inited = FALSE;


/*--------------------------------------------------------------------------
**	QualifyIconLocalCache
**		Checks to see if local cache is set up
*/
static Boolean 
#ifdef _NO_PROTO
QualifyIconLocalCache( g )
        XmIconGadget g ;
#else
QualifyIconLocalCache(
        XmIconGadget g )
#endif /* _NO_PROTO */
{
    if (!local_cache_inited)
    {
        local_cache_inited = TRUE;
        ClassCacheCopy(Icon_ClassCachePart(g))(Icon_Cache(g), &local_cache,
            sizeof(local_cache));
    }
}

/************************************************************************
 *
 * _XmReCacheIconG()
 * Check to see if ReCaching is necessary as a result of fields having
 * been set by a mananger widget. This routine is called by the
 * manager widget in their SetValues after a change is made to any
 * of Icon's cached fields.
 *
 ************************************************************************/

void 
#ifdef _NO_PROTO
_XmReCacheIconG( g )
        XmIconGadget g ;
#else
_XmReCacheIconG(
        XmIconGadget g )
#endif /* _NO_PROTO */
{
     if (local_cache_inited &&
        (!_XmIconGCacheCompare( &local_cache, Icon_Cache(g))))
     {
           _XmCacheDelete( (XtPointer) Icon_Cache(g));   /* delete the old one */
           Icon_Cache(g) = (XmIconGCacheObjPart *)_XmCachePart(
               Icon_ClassCachePart(g), (XtPointer) &local_cache, sizeof(local_cache));
     }
     local_cache_inited = FALSE;
}


/*--------------------------------------------------------------------------
**	_XmAssignIconG_StringHeight
**
*/
void 
#ifdef _NO_PROTO
_XmAssignIconG_StringHeight( g, value )
        XmIconGadget g ;
        Dimension value ;
#else
_XmAssignIconG_StringHeight(
        XmIconGadget g,
#if NeedWidePrototypes
        int value )
#else /* NeedWidePrototypes */
        Dimension value )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
       QualifyIconLocalCache(g);
       local_cache.string_height = value;
}



/*--------------------------------------------------------------------------
**	_XmAssignIconG_Foreground
**
*/
void 
#ifdef _NO_PROTO
_XmAssignIconG_Foreground( g, value )
        XmIconGadget g ;
        Pixel value ;
#else
_XmAssignIconG_Foreground(
        XmIconGadget g,
        Pixel value )
#endif /* _NO_PROTO */
{
       QualifyIconLocalCache(g);
       local_cache.foreground = value;
}


/*--------------------------------------------------------------------------
**	_XmAssignIconG_Background
**
*/
void 
#ifdef _NO_PROTO
_XmAssignIconG_Background( g, value )
        XmIconGadget g ;
        Pixel value ;
#else
_XmAssignIconG_Background(
        XmIconGadget g,
        Pixel value )
#endif /* _NO_PROTO */
{
       QualifyIconLocalCache(g);
       local_cache.background = value;
}

/*********************************************************************
 *
 *  GetParentBackgroundGC
 *      Get the graphics context used for erasing their highlight border.
 *
 *********************************************************************/
static void
#ifdef _NO_PROTO
GetParentBackgroundGC( g )
        XmIconGadget g ;
#else
GetParentBackgroundGC(
        XmIconGadget g )
#endif /* _NO_PROTO */
{
   XGCValues values;
   XtGCMask  valueMask;
   Widget    parent = XtParent((Widget)g);

   valueMask = GCForeground | GCBackground;
   values.foreground = parent->core.background_pixel;

   if (XmIsManager(parent))
      
      values.background = ((XmManagerWidget) parent)->manager.foreground;
   else
      values.background = ((XmPrimitiveWidget) parent)->primitive.foreground;

   if ((parent->core.background_pixmap != None) &&
       (parent->core.background_pixmap != XmUNSPECIFIED_PIXMAP))
   {
      valueMask |= GCFillStyle | GCTile;
      values.fill_style = FillTiled;
      values.tile = parent->core.background_pixmap;
   }

   IG_SavedParentBG(g) = parent->core.background_pixel;

   IG_ParentBackgroundGC(g) = XtGetGC (parent, valueMask, &values);
}


/*-------------------------------------------------------------
**	Initialize
**		Initialize a w_new gadget instance.
*/
static void 
#ifdef _NO_PROTO
Initialize( request_w, new_w )
        Widget request_w ;
        Widget new_w ;
        ArgList args ;
        Cardinal *numArgs ;
#else
Initialize(
        Widget request_w,
        Widget new_w,
        ArgList args,
        Cardinal *numArgs)
#endif /* _NO_PROTO */
{
	XmIconGadget	request =	(XmIconGadget) request_w,
			w_new =		(XmIconGadget) new_w;
	Window		root;
	int		int_x = 0, int_y = 0;
	unsigned int	int_w = 0, int_h = 0,
			int_bw, depth;
	Dimension	w, h;
	XmManagerWidget	mw = (XmManagerWidget) XtParent (w_new);
	EventMask	mask;
	XmString	string = NULL;
	String		name = NULL;

	IG_Sync (w_new) = False;

/*	Validate behavior.
*/
	if (IG_Behavior (w_new) != XmICON_LABEL &&
	    IG_Behavior (w_new) != XmICON_BUTTON &&
	    IG_Behavior (w_new) != XmICON_TOGGLE &&
	    IG_Behavior (w_new) != XmICON_DRAG)
	{
		_XmWarning ((Widget)w_new, WARN_BEHAVIOR);
		IG_Behavior (w_new) = XmICON_BUTTON;
	}

/*	Set the input mask for events handled by Manager.
*/
	IG_EventMask (w_new) = (XmARM_EVENT | XmACTIVATE_EVENT |
			XmMULTI_ARM_EVENT | XmMULTI_ACTIVATE_EVENT |
			XmHELP_EVENT | XmFOCUS_IN_EVENT | XmKEY_EVENT |
			XmFOCUS_OUT_EVENT | XmENTER_EVENT | XmLEAVE_EVENT);

/*	Add event handler for icon events.
*/
	if (IG_Behavior (w_new) == XmICON_DRAG)
	{
		mask = ButtonPressMask|ButtonReleaseMask;
		XtAddEventHandler (XtParent (w_new), mask, False,
				(XtEventHandler) IconEventHandler, 0);
	}

	IG_ClickTimerID (w_new) = 0;
	IG_ClickEvent (w_new) = NULL;

	IG_Armed (w_new) = False;

	IG_Mask (w_new) = NULL;

	if (IG_Pixmap (w_new) == XmUNSPECIFIED_PIXMAP)
		IG_Pixmap (w_new) = NULL;

	if (IG_ImageName (w_new) || IG_Pixmap (w_new))
	{
		if (IG_ImageName (w_new))
		{

/*	Try to get pixmap from image name.
*/
			IG_Pixmap (w_new) = 
				XmGetPixmap (XtScreen (w_new), IG_ImageName (w_new),
					IG_PixmapForeground (w_new),
					IG_PixmapBackground (w_new));
			if (IG_Pixmap (w_new) != XmUNSPECIFIED_PIXMAP) 
			  {
			      name = IG_ImageName (w_new);
			      IG_Mask (w_new) = 
				_XmGetMask(XtScreen (w_new), IG_ImageName
					   (w_new));
			  }
			else
			{
/* warning? */				
				name = NULL;
				IG_Pixmap (w_new) = NULL;
			}
		}

/*	Update width and height; copy image name.
*/
		if (IG_Pixmap (w_new))
		{
			XGetGeometry (XtDisplay (w_new), IG_Pixmap (w_new),
				&root, &int_x, &int_y, &int_w, &int_h,
				&int_bw, &depth);
		}
		if (name)
		{
			IG_ImageName (w_new) = XtNewString(name);
		}
		else
			IG_ImageName (w_new) = NULL;
	}
	IG_PixmapWidth (w_new) = (Dimension) int_w;
	IG_PixmapHeight (w_new) = (Dimension) int_h;

/*	Validate fill mode.
*/
	if (IG_FillMode (w_new) != XmFILL_NONE &&
	    IG_FillMode (w_new) != XmFILL_PARENT &&
	    IG_FillMode (w_new) != XmFILL_TRANSPARENT &&
	    IG_FillMode (w_new) != XmFILL_SELF)
	{
		_XmWarning ((Widget)w_new, WARN_FILL_MODE);
		if (G_ShadowThickness (w_new) > 0)
			IG_FillMode (w_new) = XmFILL_SELF;
		else
			IG_FillMode (w_new) = XmFILL_PARENT;
	}

/*	Validate pixmap position.
*/
	if (IG_StringPosition (w_new) != UNSPECIFIED_CHAR)
		IG_PixmapPosition (w_new) = IG_StringPosition (w_new);

	if (IG_PixmapPosition (w_new) == UNSPECIFIED_CHAR)
		IG_PixmapPosition (w_new) = XmPIXMAP_LEFT;
	else if (IG_PixmapPosition (w_new) != XmPIXMAP_LEFT &&
		 IG_PixmapPosition (w_new) != XmPIXMAP_RIGHT &&
		 IG_PixmapPosition (w_new) != XmPIXMAP_TOP &&
		 IG_PixmapPosition (w_new) != XmPIXMAP_BOTTOM)
	{
		_XmWarning ((Widget)w_new, WARN_PIXMAP_POSITION);
		IG_PixmapPosition (w_new) = XmPIXMAP_LEFT;
	}
	IG_StringPosition (w_new) = IG_PixmapPosition (w_new);

/*	Validate alignment.
*/
	if (IG_Alignment (w_new) != XmALIGNMENT_BEGINNING &&
	    IG_Alignment (w_new) != XmALIGNMENT_CENTER &&
	    IG_Alignment (w_new) != XmALIGNMENT_END)
	{
		_XmWarning ((Widget)w_new, WARN_ALIGNMENT);
		IG_Alignment (w_new) = XmALIGNMENT_BEGINNING;
	}

/*	Validate shadow type.
*/
	if (IG_ShadowType (w_new) != XmSHADOW_IN &&
	    IG_ShadowType (w_new) != XmSHADOW_OUT &&
	    IG_ShadowType (w_new) != XmSHADOW_ETCHED_IN &&
	    IG_ShadowType (w_new) != XmSHADOW_ETCHED_OUT)
	{
		_XmWarning ((Widget)w_new, WARN_SHADOW_TYPE);
		if (IG_Behavior (w_new) == XmICON_BUTTON)
			IG_ShadowType (w_new) = XmSHADOW_OUT;
		else if (IG_Behavior (w_new) == XmICON_TOGGLE)
			IG_ShadowType (w_new) = (IG_Set (w_new))
				? XmSHADOW_ETCHED_IN : XmSHADOW_ETCHED_OUT;
	}

/*	Copy fontlist.
*/
	if (IG_FontList (w_new) == NULL)
		IG_FontList (w_new) =
			_XmGetDefaultFontList ((Widget)w_new, XmBUTTON_FONTLIST);
	IG_FontList (w_new) = XmFontListCopy (IG_FontList (w_new));


/*	Get string from name if null.
*/
	if ((unsigned int)IG_String (w_new) == (unsigned int)UNSPECIFIED_STRING)
	{
		string = 
			XmStringLtoRCreate (XrmQuarkToString (IG_XrmName (w_new)),
				XmSTRING_DEFAULT_CHARSET);
		IG_String (w_new) = (_XmString) string;
	}
	if (IG_String (w_new))
	{
		IG_String (w_new) = _XmStringCreate ((XmString)IG_String (w_new));
		_XmStringExtent (IG_FontList (w_new), IG_String (w_new),
					&w, &h);
                if (IG_Underline(w_new))
                   h++;
	}
	else
		w = h = 0;
	
	IG_StringWidth (w_new) = w;
	IG_StringHeight (w_new) = h;
	if (string)
		XmStringFree (string);

/*	Convert margins to pixel units.
*/
	if (IG_UnitType (w_new) != XmPIXELS)
	{
		IG_MarginWidth (w_new) = 
			_XmToHorizontalPixels ((Widget)w_new, IG_UnitType (w_new),
					(XtArgVal *)IG_MarginWidth (w_new));
		IG_MarginHeight (w_new) = 
			_XmToVerticalPixels ((Widget)w_new, IG_UnitType (w_new),
					(XtArgVal *)IG_MarginHeight (w_new));
	}

/*	Check for unspecified margins.
*/
	if (IG_MarginWidth (request) == UNSPECIFIED_DIMENSION)
		IG_MarginWidth (w_new) = MARGIN_DEFAULT;
	if (IG_MarginHeight (request) == UNSPECIFIED_DIMENSION)
		IG_MarginHeight (w_new) = MARGIN_DEFAULT;

/*	Convert spacing.
*/
	if (IG_Spacing (w_new) == UNSPECIFIED_DIMENSION)
	{
		IG_Spacing (w_new) = IG_StringHeight (w_new) / 5;
		if (IG_Spacing (w_new) < SPACING_DEFAULT)
			IG_Spacing (w_new) = SPACING_DEFAULT;
	}
	else if (IG_Spacing (w_new) && IG_UnitType (w_new) != XmPIXELS)
	{
		IG_Spacing (w_new) = 
			(IG_PixmapPosition (w_new) == XmPIXMAP_LEFT ||
			 IG_PixmapPosition (w_new) == XmPIXMAP_RIGHT)
			? _XmToHorizontalPixels ((Widget)w_new, IG_UnitType (w_new),
					(XtArgVal *)IG_Spacing (w_new))
			: _XmToVerticalPixels ((Widget)w_new, IG_UnitType (w_new),
					(XtArgVal *)IG_Spacing (w_new));
	}

/*	Set width and height.
*/
	if (IG_Width (request) == 0 || IG_Height (request) == 0)
	{
		IG_GetSize (new_w, &w, &h);
		if (IG_Width (request) == 0)
			IG_Width (w_new) = w;
		if (IG_Height (request) == 0)
			IG_Height (w_new) = h;
	}

/*  	Get graphics contexts.
*/
	IG_NormalGC (w_new) = NULL;
	IG_ClipGC (w_new) = NULL;
	IG_BackgroundGC (w_new) = NULL;
	IG_ArmedGC (w_new) = NULL;
	IG_ArmedBackgroundGC (w_new) = NULL;
	IG_UpdateGCs (new_w);

        GetParentBackgroundGC(w_new);
}



/*-------------------------------------------------------------
**	Destroy
**		Release resources allocated for gadget.
*/
static void 
#ifdef _NO_PROTO
Destroy( w )
        Widget w ;
#else
Destroy(
        Widget w )
#endif /* _NO_PROTO */
{
	XmIconGadget	g =	(XmIconGadget) w;
	XmManagerWidget mw = (XmManagerWidget) XtParent(g);

	if (IG_ClickTimerID (g))
		XtRemoveTimeOut (IG_ClickTimerID (g));

	XtFree ((char *)IG_ClickEvent (g));

	if (IG_String (g) != NULL)
		_XmStringFree (IG_String (g));

	if (IG_ImageName (g) != NULL)
	{
		XtFree (IG_ImageName (g));
		if (IG_Mask (g) != XmUNSPECIFIED_PIXMAP)
		  XmDestroyPixmap (XtScreen(w),IG_Mask (g));
		XmDestroyPixmap (XtScreen(w),IG_Pixmap (g));
	}

	XmFontListFree (IG_FontList (g));

	_XmCacheDelete(Icon_Cache(w));

	XtReleaseGC ((Widget)mw, IG_NormalGC (g));
	XtReleaseGC ((Widget)mw, IG_ClipGC (g));
	XtReleaseGC ((Widget)mw, IG_BackgroundGC (g));
	XtReleaseGC ((Widget)mw, IG_ArmedGC (g));
	XtReleaseGC ((Widget)mw, IG_ArmedBackgroundGC (g));

/* remove event handler if last Icon in parent? */
}



/*-------------------------------------------------------------
**	Resize
**		Set clip rect?
*/
static void 
#ifdef _NO_PROTO
Resize( w )
        Widget w ;
#else
Resize(
        Widget w )
#endif /* _NO_PROTO */
{
	XmIconGadget	g =	(XmIconGadget) w;
}


/*-------------------------------------------------------------
**	Redisplay
**		Redisplay gadget.
*/
static void 
#ifdef _NO_PROTO
Redisplay( w, event, region )
        Widget w ;
        XEvent *event ;
        Region region ;
#else
Redisplay(
        Widget w,
        XEvent *event,
        Region region )
#endif /* _NO_PROTO */
{
	XmIconGadget	g =		(XmIconGadget) w;
	Dimension	s_t =		G_ShadowThickness (g);
	unsigned char	fill_mode =	IG_FillMode (g);

	if (! XtIsManaged (g))
		return;

/*	Draw gadget to window.
*/
	IG_Draw ((Widget) g, XtWindow (g), IG_X (g), IG_Y (g), IG_Width (g), IG_Height (g),
		G_HighlightThickness (g), s_t, IG_ShadowType (g), fill_mode);

/*	Draw highlight if highlighted.
*/
	if (IG_Highlighted (g))
                BorderHighlight( (Widget)g );
	else if (_XmDifferentBackground ((Widget)g, XtParent (g)))
                BorderUnhighlight( (Widget)g );
}



/*-------------------------------------------------------------
**	SetValues
**		
*/
static Boolean 
#ifdef _NO_PROTO
SetValues( current_w, request_w, new_w )
        Widget current_w ;
        Widget request_w ;
        Widget new_w ;
        ArgList args ;
        Cardinal *num_args ;
#else
SetValues(
        Widget current_w,
        Widget request_w,
        Widget new_w,
        ArgList args,
        Cardinal *num_args)
#endif /* _NO_PROTO */
{
	XmIconGadget	request =	(XmIconGadget) request_w,
			current =	(XmIconGadget) current_w,
			w_new =		(XmIconGadget) new_w;
	XmIconGadgetClass	wc =	(XmIconGadgetClass) XtClass (w_new);

	Window		root;
	int		int_x = 0, int_y = 0;
	unsigned int	int_w = 0, int_h = 0,
			int_bw, depth;
	Dimension	w, h;
	XmManagerWidget mw = (XmManagerWidget) XtParent (w_new);
	Boolean		new_image_name = False,
			redraw_flag = False,
			check_pixmap = False,
			draw_pixmap = False,
			draw_string = False,
			draw_shadow = False;
	Dimension	h_t = G_HighlightThickness (w_new),
			s_t = G_ShadowThickness (w_new),
			p_x, p_y, s_x, s_y;
	String		name = NULL;
        Position        adj_x, adj_y;
	XmManagerWidget	mgr =	(XmManagerWidget) XtParent (current_w);

/*	Validate behavior
*/
	if (IG_Behavior (w_new) != IG_Behavior (current))
	{
		if (IG_Behavior (w_new) != XmICON_LABEL &&
		    IG_Behavior (w_new) != XmICON_BUTTON &&
		    IG_Behavior (w_new) != XmICON_TOGGLE &&
		    IG_Behavior (w_new) != XmICON_DRAG)
		{
			_XmWarning ((Widget)w_new, WARN_BEHAVIOR);
			IG_Behavior (w_new) = IG_Behavior (current);
		}

		if (IG_Behavior (w_new) == XmICON_DRAG)
		{
			EventMask	mask;

			mask = ButtonPressMask|ButtonReleaseMask;
			XtAddEventHandler (XtParent (w_new), mask, False,
					(XtEventHandler) IconEventHandler, 0);
		}
	}

/*	Reset the interesting input types.
*/
	IG_EventMask (w_new) |= (XmARM_EVENT | XmACTIVATE_EVENT |
			XmMULTI_ARM_EVENT | XmMULTI_ACTIVATE_EVENT |
			XmHELP_EVENT | XmFOCUS_IN_EVENT | XmKEY_EVENT |
			XmFOCUS_OUT_EVENT | XmENTER_EVENT | XmLEAVE_EVENT);

/*	Check for w_new image name.
*/
	if (IG_ImageName (w_new) && (IG_ImageName (current) != IG_ImageName (w_new)))
		new_image_name = True;

/*	Validate shadow type.
*/
	if ((IG_ShadowType (w_new) != IG_ShadowType (current)) ||
	    (IG_Behavior (w_new) == XmICON_TOGGLE &&
	     IG_Set (w_new) != IG_Set (current)))
	{
		if (IG_ShadowType (w_new) != XmSHADOW_IN &&
		    IG_ShadowType (w_new) != XmSHADOW_OUT &&
		    IG_ShadowType (w_new) != XmSHADOW_ETCHED_IN &&
		    IG_ShadowType (w_new) != XmSHADOW_ETCHED_OUT)
		{
			_XmWarning ((Widget)w_new, WARN_SHADOW_TYPE);
			IG_ShadowType (w_new) = IG_ShadowType (current); 
		}

/*	Disallow change if conflict with set or armed state.
*/
		else if (((IG_Behavior (w_new) == XmICON_TOGGLE) &&
			  ((IG_Set (w_new) && ! IG_Armed (w_new)) ||
			   (! IG_Set (w_new) && IG_Armed (w_new)))) ||
			 ((IG_Behavior (w_new) == XmICON_BUTTON) &&
			  (IG_Armed (w_new))))
		{
			if (IG_ShadowType (w_new) == XmSHADOW_OUT)
				IG_ShadowType (w_new) = XmSHADOW_IN;
			else if (IG_ShadowType (w_new) == XmSHADOW_ETCHED_OUT)
				IG_ShadowType (w_new) = XmSHADOW_ETCHED_IN;
		}
		else if (((IG_Behavior (w_new) == XmICON_TOGGLE) &&
			  ((IG_Set (w_new) && IG_Armed (w_new)) ||
			   (! IG_Set (w_new) && ! IG_Armed (w_new)))) ||
			 ((IG_Behavior (w_new) == XmICON_BUTTON) &&
			  (! IG_Armed (w_new))))
		{
			if (IG_ShadowType (w_new) == XmSHADOW_IN)
				IG_ShadowType (w_new) = XmSHADOW_OUT;
			else if (IG_ShadowType (w_new) == XmSHADOW_ETCHED_IN)
				IG_ShadowType (w_new) = XmSHADOW_ETCHED_OUT;
		}

		if (IG_ShadowType (w_new) != IG_ShadowType (current))
			draw_shadow = True;
	}

/*	Validate alignment.
*/
	if (IG_Alignment (w_new) != IG_Alignment (current))
	{
		if (IG_Alignment (w_new) != XmALIGNMENT_BEGINNING &&
		    IG_Alignment (w_new) != XmALIGNMENT_CENTER &&
		    IG_Alignment (w_new) != XmALIGNMENT_END)
		{
			_XmWarning ((Widget)w_new, WARN_ALIGNMENT);
			IG_Alignment (w_new) = IG_Alignment (current);
		}
		else
			redraw_flag = True;
	}


/*	Validate fill mode.
*/
	if (IG_FillMode (w_new) != IG_FillMode (current))
	{
		if (IG_FillMode (w_new) != XmFILL_NONE &&
		    IG_FillMode (w_new) != XmFILL_PARENT &&
		    IG_FillMode (w_new) != XmFILL_TRANSPARENT &&
		    IG_FillMode (w_new) != XmFILL_SELF)
		{
			_XmWarning ((Widget)w_new, WARN_FILL_MODE);
			IG_FillMode (w_new) = IG_FillMode (current);
		}
	}

/*	Validate pixmap position.
*/
	if (IG_StringPosition (w_new) != IG_StringPosition (current))
		IG_PixmapPosition (w_new) = IG_StringPosition (w_new);

	if (IG_PixmapPosition (w_new) != IG_PixmapPosition (current))
	{
		if (IG_PixmapPosition (w_new) != XmPIXMAP_LEFT &&
		    IG_PixmapPosition (w_new) != XmPIXMAP_RIGHT &&
		    IG_PixmapPosition (w_new) != XmPIXMAP_TOP &&
		    IG_PixmapPosition (w_new) != XmPIXMAP_BOTTOM)
		{
			_XmWarning ((Widget)w_new, WARN_PIXMAP_POSITION);
			IG_PixmapPosition (w_new) = IG_PixmapPosition (current); 
		}
		else
			redraw_flag = True;

		IG_StringPosition (w_new) = IG_PixmapPosition (w_new);
	}

/*	Update pixmap if pixmap foreground or background changed.
*/
	if (IG_PixmapForeground (current) != IG_PixmapForeground (w_new) ||
	    IG_PixmapBackground (current) != IG_PixmapBackground (w_new))
	{
		if (IG_Pixmap (current) == IG_Pixmap (w_new) &&
		    (IG_ImageName (w_new) != NULL) &&
		    (! new_image_name))
		{
			draw_pixmap = True;
			if (IG_Mask(w_new) != XmUNSPECIFIED_PIXMAP) 
			  XmDestroyPixmap( XtScreen(w_new), IG_Mask(current));
			XmDestroyPixmap (XtScreen(w_new),IG_Pixmap (current));
			IG_Pixmap (w_new) = 
				XmGetPixmap (XtScreen (w_new), IG_ImageName (w_new),
					IG_PixmapForeground (w_new),
					IG_PixmapBackground (w_new));
			if (IG_Pixmap(w_new) != XmUNSPECIFIED_PIXMAP) 
			  IG_Mask (w_new) = 
			    _XmGetMask(XtScreen (w_new), IG_ImageName (w_new));
		}
	}


/*	Check for change in image name.
*/
	if (new_image_name)
	{

/*	Try to get pixmap from image name.
*/
		if (IG_ImageName (current) != NULL) 
		  {
		      if (IG_Mask(w_new) != XmUNSPECIFIED_PIXMAP)
			XmDestroyPixmap (XtScreen(w_new),IG_Mask(current));
		      XmDestroyPixmap (XtScreen(w_new),IG_Pixmap (current));
		  }
		IG_Pixmap (w_new) = 
			XmGetPixmap (XtScreen (w_new), IG_ImageName (w_new),
				IG_PixmapForeground (w_new),
				IG_PixmapBackground (w_new));

		if (IG_Pixmap (w_new) != XmUNSPECIFIED_PIXMAP)
		{
    		    IG_Mask(w_new) = (Pixmap)_XmGetMask(XtScreen(w_new), IG_ImageName(w_new));
		    XGetGeometry (XtDisplay (w_new), IG_Pixmap (w_new),
				  &root, &int_x, &int_y, &int_w, &int_h,
				  &int_bw, &depth);
		    name = IG_ImageName (w_new);
		    w = (Dimension) int_w;
		    h = (Dimension) int_h;
		}
		else
		{
			name = NULL;
			IG_Pixmap (w_new) = NULL;
			w = 0;
			h = 0;
		}

/*	If resetting to current image name, then reuse old copy.
*/
		if (name && IG_ImageName (current)
		    && (! strcmp (IG_ImageName (w_new), IG_ImageName (current))))
		{
			IG_ImageName (w_new) = IG_ImageName (current);
			name = IG_ImageName (current);
		}
		else 
		{
		    if (name)
		      IG_ImageName (w_new) = XtNewString(name);
		    else
		      IG_ImageName (w_new) = NULL;
		    if (IG_ImageName (current))
		      XtFree (IG_ImageName (current));
		}

		if (IG_Pixmap (w_new) != IG_Pixmap (current))
		{
			if ((IG_Pixmap (w_new) != NULL) &&
			    (IG_PixmapWidth (w_new) == w) &&
			    (IG_PixmapHeight (w_new) == h))
			{
				draw_pixmap = True;
			}
			else
			{
				redraw_flag = True;
				IG_PixmapWidth (w_new) = w;
				IG_PixmapHeight (w_new) = h;
			}
		}
	}

/*	Release image name and pixmap if name set to null.
*/
	else if (IG_Pixmap (w_new) == IG_Pixmap (current))
	{
		if ((IG_ImageName (current) != NULL) && 
		    (IG_ImageName (w_new) == NULL))
		{
			redraw_flag = True;
			XtFree (IG_ImageName (current));
			if (IG_Mask(w_new) != XmUNSPECIFIED_PIXMAP)
			  XmDestroyPixmap (XtScreen(w_new),IG_Mask (current));
			XmDestroyPixmap (XtScreen(w_new),IG_Pixmap (current));
			IG_Pixmap (w_new) = NULL;			
			IG_PixmapWidth (w_new) = 0;
			IG_PixmapHeight (w_new) = 0;
		}
	}

/*	Process change in pixmap.
*/
	else if (IG_Pixmap (w_new) != IG_Pixmap (current))
	{
		if (IG_Pixmap (w_new))
		{
			XGetGeometry (XtDisplay (w_new), IG_Pixmap (w_new), &root,
					&int_x, &int_y, &int_w, &int_h,
					&int_bw, &depth);
			w = (Dimension) int_w;
			h = (Dimension) int_h;
		}
		else
		{
			if (IG_ImageName (current) != NULL)
			{
				XtFree (IG_ImageName (current));
				if (IG_Mask(w_new) != XmUNSPECIFIED_PIXMAP)
				  XmDestroyPixmap (XtScreen(w_new),IG_Mask (current));
				XmDestroyPixmap (XtScreen(w_new),IG_Pixmap (current));
				IG_ImageName (w_new) = NULL;
			}
			w = h = 0;
		}

		if (IG_Pixmap (w_new) &&
		    (IG_PixmapWidth (w_new) == w) &&
		    (IG_PixmapHeight (w_new) == h))
		{
			draw_pixmap = True;
		}
		else
		{
			redraw_flag = True;
			IG_PixmapWidth (w_new) = w;
			IG_PixmapHeight (w_new) = h;
		}
	}
			
/*	Update GCs if foreground, background or mask changed.
*/
	if (IG_Foreground (current) != IG_Foreground (w_new) ||
	    IG_Background (current) != IG_Background (w_new) ||
	    ((IG_Mask (current) != IG_Mask(w_new)) &&
	     (IG_Pixmap (current) != IG_Pixmap (w_new))) ||
	    IG_ArmColor (current) != IG_ArmColor (w_new))
	{
		if (G_ShadowThickness (w_new) &&
		    IG_Background (current) != IG_Background (w_new))
			redraw_flag = True;
		else
			draw_string = True;
		IG_UpdateGCs (new_w);
	}

/*	Convert dimensions to pixel units.
*/
	if (IG_UnitType (w_new) != XmPIXELS)
	{
		IG_MarginWidth (w_new) = 
			_XmToHorizontalPixels ((Widget)w_new, IG_UnitType (w_new),
					(XtArgVal *)IG_MarginWidth (w_new));
		IG_MarginHeight (w_new) = 
			_XmToVerticalPixels ((Widget)w_new, IG_UnitType (w_new),
					(XtArgVal *)IG_MarginHeight (w_new));
	}

/*	Convert spacing.
*/
	if (IG_UnitType (w_new) != IG_UnitType (current) &&
	    IG_UnitType (w_new) != XmPIXELS)
	{
		IG_Spacing (w_new) = 
			(IG_PixmapPosition (w_new) == XmPIXMAP_LEFT ||
			 IG_PixmapPosition (w_new) == XmPIXMAP_RIGHT)
			? _XmToHorizontalPixels ((Widget)w_new, IG_UnitType (w_new),
					(XtArgVal *)IG_Spacing (w_new))
			: _XmToVerticalPixels ((Widget)w_new, IG_UnitType (w_new),
					(XtArgVal *)IG_Spacing (w_new));
	}

/*	Process change in string or font list.
*/

	if (IG_String (w_new) != IG_String (current) ||
	    IG_FontList (w_new) != IG_FontList (current) ||
	    IG_Underline (w_new) != IG_Underline (current))
	{
		if (IG_FontList (w_new) != IG_FontList (current))
		{
			if (IG_FontList (w_new) == NULL)
				IG_FontList (w_new) = IG_FontList (current);
			else
			{
				XmFontListFree (IG_FontList (current));
				IG_FontList (w_new) =
					XmFontListCopy (IG_FontList (w_new));
			}
		}
		if (IG_String (w_new))
		{
			if (IG_String (w_new) != IG_String (current))
			{
				if (IG_String (current))
					_XmStringFree (IG_String (current));
				IG_String (w_new) =
					_XmStringCreate ((XmString)IG_String (w_new));
			}
			else
				_XmStringUpdate (IG_FontList (w_new),
						IG_String (w_new));
			_XmStringExtent (IG_FontList (w_new), IG_String (w_new),
					&w, &h);
                        if (IG_Underline(w_new))
                           h++;
		}
		else
			w = h = 0;

		IG_StringWidth (w_new) = w;
		IG_StringHeight (w_new) = h;

		IG_Spacing (w_new) = (Dimension) IG_StringHeight (w_new) / 5;
		if (IG_Spacing (w_new) < SPACING_DEFAULT)
			IG_Spacing (w_new) = SPACING_DEFAULT;

		if ((IG_String (w_new) != NULL) &&
		    (IG_StringWidth (w_new) == IG_StringWidth (current)) &&
		    (IG_StringHeight (w_new) == IG_StringHeight (current)))
			draw_string = True;
		else
			redraw_flag = True;
	}

/*	Check for other changes requiring redraw.
*/
	if (G_HighlightThickness (w_new) != G_HighlightThickness (current) ||
	    G_ShadowThickness (w_new) != G_ShadowThickness (current) ||
	    IG_MarginWidth (w_new) != IG_MarginWidth (current) ||
	    IG_MarginHeight (w_new) != IG_MarginHeight (current) ||
	    IG_Spacing (w_new) != IG_Spacing (current))
	{
		redraw_flag = True;
	}

/*	Update size.
*/
	if (redraw_flag ||
	    (IG_RecomputeSize (w_new) && ! IG_RecomputeSize (current)))
	{
		if (IG_RecomputeSize (w_new) &&
		    (IG_Width (current) == IG_Width (w_new) ||
		     IG_Height (current) == IG_Height (w_new)))
		{
			IG_GetSize (new_w, &w, &h);
			if (IG_Width (current) == IG_Width (w_new))
				IG_Width (w_new) = w;
			if (IG_Height (current) == IG_Height (w_new))
				IG_Height (w_new) = h;
		}
	}

/*	Set redraw flag if this class doesn't optimize redraw.
*/
	else if (! C_OptimizeRedraw (wc))
	{
		if (draw_shadow || draw_pixmap || draw_string)
			redraw_flag = True;
	}	

/*	Optimize redraw if managed.
*/
	else if (XtIsManaged (w_new) && XtIsRealized(w_new))
	{
/*	Get string and pixmap positions if necessary.
*/
		if ((draw_pixmap && IG_Pixmap (w_new)) ||
		    (draw_string && IG_String (w_new)))
		{
			IG_GetPositions (new_w, IG_Width (w_new), IG_Height (w_new),
				h_t, G_ShadowThickness (w_new),
				(Position *)&p_x, (Position *)&p_y, 
				(Position *)&s_x, (Position *)&s_y);
		}
/*	Copy pixmap, clip if necessary.
*/
		if (draw_pixmap && IG_Pixmap (w_new) &&
		    IG_Pixmap (w_new) != XmUNSPECIFIED_PIXMAP)
		  {
		      w = (p_x + s_t + h_t >= IG_Width (w_new))
			? 0 : Min (IG_PixmapWidth (w_new),
				   IG_Width (w_new) - p_x - s_t - h_t);
		      h = (p_y + s_t + h_t >= IG_Height (w_new))
			? 0 : Min (IG_PixmapHeight (w_new),
				   IG_Height (w_new) - p_y - s_t - h_t);
		      if (w > 0 && h > 0) {
			  XCopyArea 
			    (XtDisplay (w_new), IG_Pixmap (w_new), XtWindow (w_new),
			     GetMaskGC(w_new, IG_X(w_new) + p_x, IG_Y(w_new) + p_y),
			     0, 0,
			     w, h, IG_X (w_new) + p_x, IG_Y (w_new) + p_y);
		      }
		}
/*	Draw string with normal or armed background; clip if necessary.
*/
		if (draw_string && IG_String (w_new))
		{
			GC		gc;
			XRectangle	clip;
			unsigned char	behavior = IG_Behavior (w_new);

			if ((behavior == XmICON_BUTTON ||
			     behavior == XmICON_DRAG) &&
			    IG_FillOnArm (w_new) && IG_Armed (w_new))
				gc = IG_ArmedGC (w_new);
			else if (behavior == XmICON_TOGGLE &&
				 IG_FillOnArm (w_new) &&
				 ((IG_Armed (w_new) && !IG_Set (w_new)) ||
				  (!IG_Armed (w_new) && IG_Set (w_new))))
				gc = IG_ArmedGC (w_new);
			else
				gc = IG_NormalGC (w_new);

			clip.x = IG_X (w_new) + s_x;
			clip.y = IG_Y (w_new) + s_y;
			clip.width = (s_x + s_t + h_t >= IG_Width (w_new))
				? 0 : Min (IG_StringWidth (w_new),
					IG_Width (w_new) - s_x - s_t - h_t);
			clip.height = (s_y + s_t + h_t >= IG_Height (w_new))
				? 0 : Min (IG_StringHeight (w_new),
					IG_Height (w_new) - s_y - s_t - h_t);
			if (clip.width > 0 && clip.height > 0)
                        {
				_XmStringDrawImage (XtDisplay (w_new),
					 XtWindow (w_new), IG_FontList (w_new),
					IG_String (w_new), gc,
					IG_X (w_new) + s_x, IG_Y (w_new) + s_y,
					clip.width, XmALIGNMENT_CENTER,
					XmSTRING_DIRECTION_L_TO_R, &clip);

				if (IG_Underline(w_new))
				{
				   _XmStringDrawUnderline (XtDisplay (w_new),
					    XtWindow (w_new), IG_FontList (w_new),
					   IG_String (w_new), gc,
					   IG_X (w_new) + s_x, IG_Y (w_new) + s_y,
					   clip.width, XmALIGNMENT_CENTER,
					   XmSTRING_DIRECTION_L_TO_R, &clip,
                                           IG_String(w_new));
				}
                        }
		}
/*	Draw shadow.
*/
		if (draw_shadow)
		{
                   if(IG_BorderType(w_new) == XmRECTANGLE || !IG_Pixmap(w_new))
		      _XmManagerDrawShadow (MgrParent (w_new), XtWindow (w_new),
                                IG_X (w_new), IG_Y (w_new),
				IG_Width (w_new), IG_Height (w_new), h_t,
				G_ShadowThickness (w_new), IG_ShadowType (w_new));
                   else
                      IG_CallCallback (new_w, IG_Callback (w_new),
                                                    XmCR_SHADOW, NULL);
		}
	}

	return (redraw_flag);
}


/*-------------------------------------------------------------
**	Gadget Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	BorderHighlight
**
*/
static void 
#ifdef _NO_PROTO
BorderHighlight( gw )
        Widget gw ;
#else
BorderHighlight(
        Widget gw)
#endif /* _NO_PROTO */
{
   XmIconGadget g = (XmIconGadget) gw ;
   register int width;
   register int height;

   width = g->rectangle.width;
   height = g->rectangle.height;

   if (width == 0 || height == 0) return;

   if (g->gadget.highlight_thickness == 0) return;

   g->gadget.highlighted = True;
   g->gadget.highlight_drawn = True;

   if(IG_BorderType(g) == XmRECTANGLE || !IG_Pixmap(g))
      _XmHighlightBorder ((Widget)g);
   else
      IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_HIGHLIGHT, NULL);

}

/*-------------------------------------------------------------
**	BorderUnhighlight
**
*/
static void 
#ifdef _NO_PROTO
BorderUnhighlight( gw )
        Widget gw ;
#else
BorderUnhighlight( 
        Widget gw)
#endif /* _NO_PROTO */
{
   XmIconGadget g = (XmIconGadget) gw ;

   register int window_width;
   register int window_height;
   register highlight_width;

   window_width = g->rectangle.width;
   window_height = g->rectangle.height;

   if (window_width == 0 || window_height == 0) return;

   highlight_width = g->gadget.highlight_thickness;
   if (highlight_width == 0) return;

   g->gadget.highlighted = False;
   g->gadget.highlight_drawn = False;

   if(IG_BorderType(g) == XmRECTANGLE || !IG_Pixmap(g))
      _XmUnhighlightBorder ((Widget)g);
   else
      IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_UNHIGHLIGHT, NULL);

}

/*-------------------------------------------------------------
**	ArmAndActivate
**		Invoke Activate.
*/
static void 
#ifdef _NO_PROTO
ArmAndActivate( w, event, params, numParams )
        Widget w ;
        XEvent *event ;
        String *params ;
        Cardinal *numParams ;
#else
ArmAndActivate(
        Widget w,
        XEvent *event,
        String *params,
        Cardinal *numParams)
#endif /* _NO_PROTO */
{
	IconArm (w, event);
	IconActivate (w, event);
}



/*-------------------------------------------------------------
**	InputDispatch
**		Process event dispatched from parent or event handler.
*/
static void 
#ifdef _NO_PROTO
InputDispatch( w, event, event_mask )
        Widget w ;
        XEvent *event ;
        Mask event_mask ;
#else
InputDispatch(
        Widget w,
        XEvent *event,
        Mask event_mask )
#endif /* _NO_PROTO */
{
	XmIconGadget	g =	(XmIconGadget) w;

	if (event_mask & XmARM_EVENT || event_mask & XmMULTI_ARM_EVENT)
		if (event->xbutton.button == Button2)
			IconDrag (w, (XEvent*) event);	
                else if (event->xbutton.button == Button3)
                        IconPopup (w, (XEvent*) event);
		else
			IconArm (w, (XEvent*) event);
	else if (event_mask & XmACTIVATE_EVENT ||
		 event_mask & XmMULTI_ACTIVATE_EVENT)
	{
		if (event->xbutton.button == Button2)
			IconDrop (w, (XEvent*) event);	
                else if (event->xbutton.button == Button3)
                        ;
		else if (event->xbutton.x >= IG_X (g) &&
			 event->xbutton.x <= IG_X (g) + IG_Width (g) &&
			 event->xbutton.y >= IG_Y (g) &&
			 event->xbutton.y <= IG_Y (g) + IG_Height (g))
			IconActivate (w, (XEvent*) event);
		else
			IconDisarm (w, (XEvent*) event);
	}
	else if (event_mask & XmHELP_EVENT)
		_XmSocorro (w, event, NULL, NULL);
	else if (event_mask & XmENTER_EVENT)
		IconEnter (w, event);
	else if (event_mask & XmLEAVE_EVENT)
		IconLeave (w, event);
	else if (event_mask & XmFOCUS_IN_EVENT)
		_XmFocusInGadget (w, event, NULL, NULL);
	else if (event_mask & XmFOCUS_OUT_EVENT)
		_XmFocusOutGadget (w, event, NULL, NULL);
}



/*-------------------------------------------------------------
**	VisualChange
**		Update GCs when parent visuals change.
*/
static Boolean 
#ifdef _NO_PROTO
VisualChange( w, current_w, new_w )
        Widget w ;
        Widget current_w ;
        Widget new_w ;
#else
VisualChange(
        Widget w,
        Widget current_w,
        Widget new_w )
#endif /* _NO_PROTO */
{
    XmManagerWidget		current =	(XmManagerWidget) current_w;
    XmManagerWidget		w_new = 		(XmManagerWidget) new_w;
    XmIconGadget		g = 		(XmIconGadget) w;
    Boolean			update = 	False;
    
    /*	If foreground or background was the same as the parent, and parent
     **	foreground or background has changed, then update gcs and pixmap.
     */
    /* (can't really tell if explicitly set to be same as parent!
     **  -- could add flags set in dynamic default procs for fg and bg)
     */
    if (IG_Foreground (g) == M_Foreground (current) &&
	M_Foreground (current) != M_Foreground (w_new))
      {
	  _XmAssignIconG_Foreground(g, M_Foreground (w_new));
	  update = True;
      }
    
    if (IG_Background (g) == M_Background (current) &&
	M_Background (current) != M_Background (w_new))
      {
	  _XmAssignIconG_Background(g, M_Background (w_new));
	  update = True;
      }
    
    if (IG_PixmapForeground (g) == M_Foreground (current) &&
	M_Foreground (current) != M_Foreground (w_new))
      {
	  IG_PixmapForeground(g) =  M_Foreground (w_new);
	  update = True;
      }
    
    if (IG_PixmapBackground (g) == M_Background (current) &&
	M_Background (current) != M_Background (w_new))
      {
	  IG_PixmapBackground(g) = M_Background (w_new);
	  update = True;
      }
    
    if (update)
      {
	  _XmReCacheIconG(g);
	  IG_UpdateGCs ((Widget) g);
	  
	  if (IG_ImageName (g) != NULL)
	    {
		if (IG_Mask(g) != XmUNSPECIFIED_PIXMAP)
		  XmDestroyPixmap (XtScreen(g),IG_Mask(g));
		XmDestroyPixmap (XtScreen(w),IG_Pixmap (g));
		IG_Pixmap (g) = 
		  XmGetPixmap (XtScreen (g), IG_ImageName (g),
			       IG_PixmapForeground (g),
			       IG_PixmapBackground (g));
		if (IG_Pixmap (g) != XmUNSPECIFIED_PIXMAP)
		  IG_Mask(g) = (Pixmap)_XmGetMask(XtScreen(g), IG_ImageName(g));
		return (True);
	    }
	  else
	    return (False);
      }
}


/*-------------------------------------------------------------
**	Icon Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	GetSize
**		Compute size.
*/
static void 
#ifdef _NO_PROTO
GetSize( gw, w, h )
        Widget gw ;
        Dimension *w ;
        Dimension *h ;
#else
GetSize(
        Widget gw,
        Dimension *w,
        Dimension *h )
#endif /* _NO_PROTO */
{
    XmIconGadget g = (XmIconGadget) gw ;
	Dimension	s_t = G_ShadowThickness (g),
			h_t = G_HighlightThickness (g),
			p_w = IG_PixmapWidth (g),
			p_h = IG_PixmapHeight (g),
			m_w = IG_MarginWidth (g),
			m_h = IG_MarginHeight (g),
			s_w = IG_StringWidth (g),
			s_h = IG_StringHeight (g),
			v_pad = 2 * (s_t + h_t + m_h),
			h_pad = 2 * (s_t + h_t + m_w),
			spacing = IG_Spacing (g);
	
	if (((p_w == 0) && (p_h == 0)) || ((s_w == 0) && (s_h == 0)))
		spacing = 0;

/*	Get width and height.
*/
	switch ((int) IG_PixmapPosition (g))
	{
		case XmPIXMAP_TOP:
		case XmPIXMAP_BOTTOM:
			*w = Max (p_w, s_w) + h_pad;
			*h = p_h + s_h + v_pad + spacing;
			break;
		case XmPIXMAP_LEFT:
		case XmPIXMAP_RIGHT:
			*w = p_w + s_w + h_pad + spacing;
			*h = Max (p_h, s_h) + v_pad;
			break;
		default:
			break;
	}
}



/*-------------------------------------------------------------
**	GetPositions
**		Get positions of string and pixmap.
*/
static void 
#ifdef _NO_PROTO
GetPositions( gw, w, h, h_t, s_t, pix_x, pix_y, str_x, str_y )
        Widget gw ;
        Position w ;
        Position h ;
        Dimension h_t ;
        Dimension s_t ;
        Position *pix_x ;
        Position *pix_y ;
        Position *str_x ;
        Position *str_y ;
#else
GetPositions(
        Widget gw,
#if NeedWidePrototypes
        int w,
        int h,
        int h_t,
        int s_t,
#else /* NeedWidePrototypes */
        Position w,
        Position h,
        Dimension h_t,
        Dimension s_t,
#endif /* NeedWidePrototypes */
        Position *pix_x,
        Position *pix_y,
        Position *str_x,
        Position *str_y )
#endif /* _NO_PROTO */
{
        XmIconGadget g = (XmIconGadget) gw ;
	Dimension	p_w =		IG_PixmapWidth (g),
			p_h =		IG_PixmapHeight (g),
			s_w =		IG_StringWidth (g),
			s_h =		IG_StringHeight (g),
			m_w =		IG_MarginWidth (g),
			m_h =		IG_MarginHeight (g),
			spacing =	IG_Spacing (g),
			h_pad =		s_t + h_t + m_w,
			v_pad =		s_t + h_t + m_h,
			width =		w - 2 * h_pad,
			height =	h - 2 * v_pad;
	Position	p_x =		h_pad,
			p_y =		v_pad,
			s_x =		h_pad,
			s_y =		v_pad;
	unsigned char	align =		IG_Alignment (g);

	if (((p_w == 0) && (p_h == 0)) || ((s_w == 0) && (s_h == 0)))
		spacing = 0;

/*	Set positions
*/
	switch ((int) IG_PixmapPosition (g))
	{
		case XmPIXMAP_TOP:
			if (align == XmALIGNMENT_CENTER)
			{
				if (p_w && width > p_w)
					p_x += (width - p_w)/2;
				if (s_w && width > s_w)
					s_x += (width - s_w)/2;
			}
			else if (align == XmALIGNMENT_END)
			{
				if (p_w && width > p_w)
					p_x += width - p_w;
				if (s_w && width > s_w)
					s_x += width - s_w;
			}
			if (p_h && height > p_h + s_h + spacing)
				p_y += (height - p_h - s_h - spacing)/2;
			if (p_h)
				s_y = p_y + p_h + spacing;
			else
				s_y += (height - s_h)/2;
			break;
		case XmPIXMAP_BOTTOM:
			if (align == XmALIGNMENT_CENTER)
			{
				if (p_w && width > p_w)
					p_x += (width - p_w)/2;
				if (s_w && width > s_w)
					s_x += (width - s_w)/2;
			}
			else if (align == XmALIGNMENT_END)
			{
				if (p_w && width > p_w)
					p_x += width - p_w;
				if (s_w && width > s_w)
					s_x += width - s_w;
			}
			if (s_h && height > p_h + s_h + spacing)
				s_y += (height - p_h - s_h - spacing)/2;
			if (s_h)
				p_y = s_y + s_h + spacing;
			else
				p_y += (height - s_h)/2;
			break;
		case XmPIXMAP_LEFT:
			if (p_h && height > p_h)
				p_y += (height - p_h)/2;
			s_x += p_w + spacing;
			if (s_h && height > s_h)
				s_y += (height - s_h)/2;
			break;
		case XmPIXMAP_RIGHT:
			if (s_h && height > s_h)
				s_y += (height - s_h)/2;
			p_x += s_w + spacing;
			if (p_h && height > p_h)
				p_y += (height - p_h)/2;
			break;
		default:
			break;
	}

	*pix_x = p_x;
	*pix_y = p_y;
	*str_x = s_x;
	*str_y = s_y;
}



/*-------------------------------------------------------------
**	Draw
**		Draw gadget to drawable.
*/
static void 
#ifdef _NO_PROTO
Draw( gw, drawable, x, y, w, h, h_t, s_t, s_type, fill_mode )
        Widget gw ;
        Drawable drawable ;
        Position x ;
        Position y ;
        Dimension w ;
        Dimension h ;
        Dimension h_t ;
        Dimension s_t ;
        unsigned char s_type ;
        unsigned char fill_mode ;
#else
Draw(
        Widget gw,
        Drawable drawable,
#if NeedWidePrototypes
        int x,
        int y,
        int w,
        int h,
        int h_t,
        int s_t,
        unsigned int s_type,
        unsigned int fill_mode )
#else /* NeedWidePrototypes */
        Position x,
        Position y,
        Dimension w,
        Dimension h,
        Dimension h_t,
        Dimension s_t,
        unsigned char s_type,
        unsigned char fill_mode )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
        XmIconGadget g = (XmIconGadget) gw ;
	XmManagerWidget	mgr =	(XmManagerWidget) XtParent (g);
	Display *	d = 	XtDisplay (g);
	GC		gc;
	XRectangle	clip;
	Position	p_x, p_y, s_x, s_y;
	Dimension	width, height;
	unsigned char	behavior =	IG_Behavior (g);
        Position        adj_x, adj_y;

/*	Fill with icon or manager background or arm color.
*/
        if (IG_SavedParentBG(g) != XtParent(g)->core.background_pixel) {
	   XtReleaseGC((Widget)mgr, IG_ParentBackgroundGC(g));
	   GetParentBackgroundGC(g);
        }

	if (fill_mode == XmFILL_SELF)
	{
		if ((behavior == XmICON_BUTTON || behavior == XmICON_DRAG) &&
		     IG_FillOnArm (g) && IG_Armed (g))
			gc = IG_ArmedBackgroundGC (g);
		else if (behavior == XmICON_TOGGLE && IG_FillOnArm (g) &&
			 ((IG_Armed (g) && !IG_Set (g)) ||
			  (!IG_Armed (g) && IG_Set (g))))
			gc = IG_ArmedBackgroundGC (g);
		else
			gc = IG_BackgroundGC (g);
	}
	else if (fill_mode == XmFILL_PARENT)
		gc = IG_ParentBackgroundGC (g);

       if ((fill_mode != XmFILL_NONE) && (fill_mode != XmFILL_TRANSPARENT))
		XFillRectangle (d, drawable, gc, x + h_t, y + h_t,
				w - 2 * h_t, h - 2 * h_t);

/*	Get pixmap and string positions.
*/
	IG_GetPositions ((Widget) g, w, h, h_t, s_t, &p_x, &p_y, &s_x, &s_y);

/*	Copy pixmap.
*/
	if (IG_Pixmap (g))
	{
		width = (p_x + s_t + h_t >= IG_Width (g))
			? 0 : Min (IG_PixmapWidth (g),
				IG_Width (g) - p_x - s_t - h_t);
		height = (p_y + s_t + h_t >= IG_Height (g))
			? 0 : Min (IG_PixmapHeight (g),
				IG_Height (g) - p_y - s_t - h_t);
		if (width > 0 && height > 0)
		  {
		      if (fill_mode == XmFILL_TRANSPARENT)
                        {
			    adj_x = s_t + h_t + IG_MarginWidth(g);
			    adj_y = s_t + h_t + IG_MarginHeight(g);
                            switch (IG_PixmapPosition(g))
                            {
                               case XmPIXMAP_TOP:
                               case XmPIXMAP_BOTTOM:
			          XFillRectangle(d, drawable, 
						 IG_ParentBackgroundGC(g),
					         x + p_x - adj_x, 
                                                 y + p_y - adj_y,
					         width + (2 * adj_y),
        				         height + (2 * adj_x) -
                                                                 (s_t + h_t));
                                  break;
                               case XmPIXMAP_LEFT:
                               case XmPIXMAP_RIGHT:
			          XFillRectangle(d, drawable, 
                                                 IG_ParentBackgroundGC(g),
					         x + p_x - adj_x, 
                                                 y + p_y - adj_y,
					         width + (2 * adj_y) -
                                                                 (s_t + h_t),
        				         height + (2 * adj_x));
                                  break;
                            }
                        }
		      XCopyArea (d, IG_Pixmap (g), drawable, 
				 GetMaskGC(g, x + p_x, y + p_y),
				 0, 0, width, height, x + p_x, y + p_y);
		  }
	}

/*	Draw string.
*/
	if ((behavior == XmICON_BUTTON || behavior == XmICON_DRAG) &&
	     IG_FillOnArm (g) && IG_Armed (g))
		gc = IG_ArmedGC (g);
	else if (behavior == XmICON_TOGGLE && IG_FillOnArm (g) &&
		 ((IG_Armed (g) && !IG_Set (g)) ||
		  (!IG_Armed (g) && IG_Set (g))))
		gc = IG_ArmedGC (g);
	else
		gc = IG_NormalGC (g);

	if (IG_String (g))
	{
		clip.x = x + s_x;
		clip.y = y + s_y;
                switch (IG_PixmapPosition(g))
                {
                   case XmPIXMAP_TOP:
                   case XmPIXMAP_BOTTOM:
		      clip.width = (s_x + s_t + h_t >= IG_Width (g))
			      ? 0 : Min (IG_StringWidth (g),
           		            IG_Width (g) - s_x);
		      clip.height = (s_y + s_t + h_t >= IG_Height (g))
		      	      ? 0 : Min (IG_StringHeight (g),
				     IG_Height (g) - s_y - s_t - h_t);
                      break;
                   case XmPIXMAP_LEFT:
                   case XmPIXMAP_RIGHT:
		      clip.width = (s_x + s_t + h_t >= IG_Width (g))
			      ? 0 : Min (IG_StringWidth (g),
           		            IG_Width (g) - s_x - s_t - h_t);
		      clip.height = (s_y + s_t + h_t >= IG_Height (g))
		      	      ? 0 : Min (IG_StringHeight (g),
				     IG_Height (g) - s_y);
                      break;
                 }
		if (clip.width > 0 && clip.height > 0)
                {
                        if (fill_mode == XmFILL_TRANSPARENT)
                        {
                           adj_x = s_t + h_t + IG_MarginWidth(g);
                           adj_y = s_t + h_t + IG_MarginHeight(g);
                           switch (IG_PixmapPosition(g))
                           {
                              case XmPIXMAP_TOP:
                              case XmPIXMAP_BOTTOM:
                                 XFillRectangle(d, drawable, 
                                                IG_ParentBackgroundGC(g),
                                                clip.x - adj_x, 
                                                clip.y - adj_y + s_t + h_t,
                                                clip.width + (2 * adj_y),
                                                clip.height + (2 * adj_x) - 
                                                                (s_t + h_t));
                                 break;
                              case XmPIXMAP_RIGHT:
                              case XmPIXMAP_LEFT:
                                 XFillRectangle(d, drawable, 
                                                IG_ParentBackgroundGC(g),
                                                clip.x - adj_x + s_t + h_t, 
                                                clip.y - adj_y,
                                                clip.width + (2 * adj_y) -
                                                                (s_t + h_t),
                                                clip.height + (2 * adj_x));
                                 break;
                            }
                        }

			_XmStringDrawImage (d, drawable, IG_FontList (g),
				IG_String (g), gc, x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);

			if (IG_Underline(g))
			{
			   _XmStringDrawUnderline (d, drawable, IG_FontList (g),
				IG_String (g), gc, x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip,
                                IG_String(g));
			}
                }
	}

        /* Potentially fill the area between the label and the pixmap */
        if ((fill_mode == XmFILL_TRANSPARENT) && IG_Pixmap(g) && IG_String(g) &&
           (height > 0) && (width > 0) && (clip.width > 0) && (clip.height > 0))
        {
           switch (IG_PixmapPosition(g))
           {
              case XmPIXMAP_TOP:
                      XFillRectangle(d, drawable, IG_ParentBackgroundGC(g),
                                     x + Max(s_x, p_x), y + p_y + height,
                                     Min(clip.width, width),
                                     s_y - (p_y + height));
                      break;

              case XmPIXMAP_BOTTOM:
                      XFillRectangle(d, drawable, IG_ParentBackgroundGC(g),
                                     x + Max(s_x, p_x), y + s_y + clip.height,
                                     Min(clip.width, width),
                                     p_y - (s_y + clip.height));
                      break;

              case XmPIXMAP_RIGHT:
                      XFillRectangle(d, drawable, IG_ParentBackgroundGC(g),
                                     x + s_x + clip.width, y + Max(s_y, p_y),
                                     p_x - (s_x + clip.width),
                                     Min(clip.height, height));
                      break;
              case XmPIXMAP_LEFT:
                      XFillRectangle(d, drawable, IG_ParentBackgroundGC(g),
                                     x + p_x + width, y + Max(s_y, p_y),
                                     s_x - (p_x + width),
                                     Min(clip.height, height));
                      break;
           }
        }


/*	Draw shadow.
*/
	if (G_ShadowThickness (g) > 0)
           if(IG_BorderType(g) == XmRECTANGLE || !IG_Pixmap(g))
		{
		if (((IG_Behavior (g) == XmICON_BUTTON) && IG_Armed (g)) ||
		    ((IG_Behavior (g) == XmICON_TOGGLE) &&
		      (!IG_Set (g) && IG_Armed (g)) ||
		      (IG_Set (g) && !IG_Armed (g))))
			_XmManagerDrawShadow (MgrParent(g), drawable, x, y,
					w, h, h_t, s_t, XmSHADOW_IN);
		else
			_XmManagerDrawShadow (MgrParent(g), drawable, x, y,
					w, h, h_t, s_t, XmSHADOW_OUT);
		}
           else
              IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_SHADOW, NULL);
}



/*-------------------------------------------------------------
**	CallCallback
**		Call callback, if any, with reason and event.
*/
static void 
#ifdef _NO_PROTO
CallCallback( g, cb, reason, event )
        Widget g ;
        XtCallbackList cb ;
        int reason ;
        XEvent *event ;
#else
CallCallback(
        Widget g,
        XtCallbackList cb,
        int reason,
        XEvent *event )
#endif /* _NO_PROTO */
{
	XmIconGadgetCallbackStruct	cb_data;

	if (cb != NULL)
	{
		cb_data.reason = reason;
		cb_data.event = event;
		cb_data.set = IG_Set (((XmIconGadget) g));
		XtCallCallbackList (g, cb, &cb_data);
	}
}





/*-------------------------------------------------------------
**	UpdateGCs
**		Get normal and background graphics contexts.
**		Use standard mask to maximize caching opportunities.
*/
static void 
#ifdef _NO_PROTO
UpdateGCs( gw )
        Widget gw ;
#else
UpdateGCs(
        Widget gw )
#endif /* _NO_PROTO */
{
        XmIconGadget g = (XmIconGadget) gw ;
	XGCValues	values;
	XtGCMask	value_mask;
	XmManagerWidget	mw = (XmManagerWidget) XtParent(g);
	XFontStruct *	font;
	int		index;
	Boolean		font_rtn;

	if (IG_NormalGC (g))
		XtReleaseGC ((Widget)mw, IG_NormalGC (g));
	if (IG_ClipGC (g))
		XtReleaseGC ((Widget)mw, IG_ClipGC (g));
	if (IG_BackgroundGC (g))
		XtReleaseGC ((Widget)mw, IG_BackgroundGC (g));
	if (IG_ArmedGC (g))
		XtReleaseGC ((Widget)mw, IG_ArmedGC (g));
	if (IG_ArmedBackgroundGC (g))
		XtReleaseGC ((Widget)mw, IG_ArmedBackgroundGC (g));

/*	Get normal GC.
*/
	font_rtn = _XmFontListSearch (IG_FontList (g), "default", (short *)&index,
					&font);
	value_mask = GCForeground | GCBackground | GCFont | GCFillStyle;
	values.foreground = IG_Foreground (g);
	values.background = IG_Background (g);
	values.fill_style = FillSolid;
	values.font = font->fid;
	IG_NormalGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get background GC.
*/
	values.foreground = IG_Background (g);
	values.background = IG_Foreground (g);
	IG_BackgroundGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get armed GC.
*/
	values.foreground = IG_Foreground (g);
	values.background = IG_ArmColor (g);
	IG_ArmedGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get armed background GC.
*/
	values.foreground = IG_ArmColor (g);
	values.background = IG_Background (g);
	IG_ArmedBackgroundGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

	if (IG_Mask(g) != XmUNSPECIFIED_PIXMAP) {
	    value_mask |= GCClipMask;
	    values.clip_mask = IG_Mask(g);
	    values.foreground = IG_Foreground (g);
	    values.background = IG_Background (g);
	    IG_ClipGC (g) = XtGetGC ((Widget)mw, value_mask, &values);
	}
	else
	  IG_ClipGC (g) = NULL;
}


/*-------------------------------------------------------------
**	GetIconClassSecResData ( )
**		Class function to be called to copy secondary resource
**		for external use.  i.e. copy the cached resources and
**		send it back.
**-------------------------------------------------------------
*/
static Cardinal 
#ifdef _NO_PROTO
GetIconClassSecResData( w_class, data_rtn )
        WidgetClass w_class ;
        XmSecondaryResourceData **data_rtn ;
#else
GetIconClassSecResData(
        WidgetClass w_class,
        XmSecondaryResourceData **data_rtn )
#endif /* _NO_PROTO */
{   int arrayCount = 0;
    int resNum;
    XmSecondaryResourceData secData, *sd;
    XmBaseClassExt  bcePtr;
    String  resource_class, resource_name;
    XtPointer  client_data;

    bcePtr = &( iconGBaseClassExtRec);
    client_data = NULL;
    resource_class = NULL;
    resource_name = NULL;
    arrayCount =
      _XmSecondaryResourceData ( bcePtr, data_rtn, client_data,
                                resource_name, resource_class,
                            (XmResourceBaseProc) (GetIconClassResBase));

    return (arrayCount);
}


/*-------------------------------------------------------------
**	GetIconClassResBase ()
**		retrun the address of the base of resources.
**		- Not yet implemented.
**-------------------------------------------------------------
*/
static XtPointer 
#ifdef _NO_PROTO
GetIconClassResBase( widget, client_data )
        Widget widget ;
        XtPointer client_data ;
#else
GetIconClassResBase(
        Widget widget,
        XtPointer client_data )
#endif /* _NO_PROTO */
{   XtPointer  widgetSecdataPtr;
    int  icon_cache_size = sizeof (XmIconGCacheObjPart);
    char *cp;

    widgetSecdataPtr = (XtPointer) (XtMalloc ( icon_cache_size +1));

    if (widgetSecdataPtr)
      { cp = (char *) widgetSecdataPtr;
        memcpy( (char *) cp, (char *) ( Icon_Cache(widget)), icon_cache_size);
          }

    return (widgetSecdataPtr);
}




/*-------------------------------------------------------------
**	_XmIconGadgetGetState
**		Return state of Icon.
**-------------------------------------------------------------
*/
Boolean 
#ifdef _NO_PROTO
_XmIconGadgetGetState( w )
        Widget w ;
#else
_XmIconGadgetGetState(
        Widget w )
#endif /* _NO_PROTO */
{
	XmIconGadget g =	(XmIconGadget) w;

	return (IG_Set (g));
}


/*-------------------------------------------------------------
**	_XmIconGadgetSetState
**		Set state of Icon.
**-------------------------------------------------------------
*/
void 
#ifdef _NO_PROTO
_XmIconGadgetSetState( w, state, notify )
        Widget w ;
        Boolean state ;
        Boolean notify ;
#else
_XmIconGadgetSetState(
        Widget w,
#if NeedWidePrototypes
        int state,
        int notify )
#else /* NeedWidePrototypes */
        Boolean state,
        Boolean notify )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
	XmIconGadget g =	(XmIconGadget) w;

	if (IG_Behavior (g) != XmICON_TOGGLE || state == IG_Set (g))
		return;

	IG_Set (g) = state;
	IG_Expose ((Widget)g, NULL, NULL);

	if (notify)
		IG_CallCallback ((Widget) g, IG_Callback (g), XmCR_VALUE_CHANGED, NULL);
}



/*-------------------------------------------------------------
**	_XmIconGadgetDraw
**		Render gadget to drawable without highlight.
**-------------------------------------------------------------
*/
Drawable 
#ifdef _NO_PROTO
_XmIconGadgetDraw( widget, drawable, x, y, fill )
        Widget widget ;
        Drawable drawable ;
        Position x ;
        Position y ;
        Boolean fill ;
#else
_XmIconGadgetDraw(
        Widget widget,
        Drawable drawable,
#if NeedWidePrototypes
        int x,
        int y,
        int fill )
#else /* NeedWidePrototypes */
        Position x,
        Position y,
        Boolean fill )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
	XmIconGadget	g =		(XmIconGadget) widget;
	Dimension	h_t =		G_HighlightThickness (g),
			w =		IG_Width (g) - 2 * h_t,
			h =		IG_Height (g) - 2 * h_t;
	unsigned char	fill_mode;

	if (!drawable || drawable == XmUNSPECIFIED_PIXMAP)
		drawable = (Drawable)
			XCreatePixmap (XtDisplay (g),
				RootWindowOfScreen (XtScreen (g)),
				w, h, DefaultDepthOfScreen (XtScreen (g)));

	fill_mode = (fill) ? XmFILL_SELF : XmFILL_PARENT;
	
	IG_Draw ((Widget) g, drawable, x, y, w, h,
		0, G_ShadowThickness (g), IG_ShadowType (g), fill_mode);

	return (drawable);
}



/***************************************************************************/


/* Pixmap cache information */
typedef struct {
   Display * d;
   Pixmap id;
   Dimension h;
   Dimension w;
} PmCache, *PmCachePtr;

static int numCache = 0;
static int sizeCache = 0;
static PmCachePtr cache = NULL;


/*
 * Load the specified pixmap; use our cached size, if possible, to
 * save a server round trip.
 */

static Boolean 
#ifdef _NO_PROTO
LoadPixmap( w_new, pixmap )
        XmIconGadget w_new ;
        String pixmap ;
#else
LoadPixmap(
        XmIconGadget w_new,
        String pixmap )
#endif /* _NO_PROTO */

{
   Display * display = XtDisplay(w_new);
   Window root;
   unsigned int int_h, int_w, int_bw, depth;
   int x, y;
   int i;
   Pixmap pm = XmGetPixmap(XtScreen(w_new), pixmap, IG_PixmapForeground(w_new),
                           IG_PixmapBackground(w_new));
   Pixmap mask;

   if (pm != XmUNSPECIFIED_PIXMAP)
     mask = _XmGetMask(XtScreen(w_new), pixmap);

   if (pm == XmUNSPECIFIED_PIXMAP)
      return(True);

   IG_Pixmap(w_new) = pm;
   IG_Mask(w_new) = mask;
   IG_ImageName(w_new) = XtNewString(pixmap);

   /* Have we cached this pixmap already? */
   for (i = 0; i < numCache; i++)
   {
      if ((cache[i].id == pm) && (cache[i].d == display))
      {
         /* Found a match */
         IG_PixmapWidth(w_new) = cache[i].w;
         IG_PixmapHeight(w_new) = cache[i].h;
         return(False);
      }
   }

   /* Not in the cache; add it */
   if (numCache >= sizeCache)
   {
      sizeCache += 10;
      cache = (PmCachePtr)XtRealloc((char*)cache, sizeof(PmCache) * sizeCache);
   }
   XGetGeometry(XtDisplay(w_new), pm, &root, &x, &y, &int_w, &int_h,
                &int_bw, &depth);
   cache[numCache].d = display;
   cache[numCache].id = pm;
   cache[numCache].h = int_h;
   cache[numCache].w = int_w;
   numCache++;
   IG_PixmapWidth(w_new) = int_w;
   IG_PixmapHeight(w_new) = int_h;
   return(False);
}



Widget 
#ifdef _NO_PROTO
_XmDuplicateIconG( parent, widget, string, pixmap, user_data, underline )
        Widget parent ;
        Widget widget ;
        XmString string ;
        String pixmap ;
        XtPointer user_data ;
        Boolean underline;
#else
_XmDuplicateIconG(
        Widget parent,
        Widget widget,
        XmString string,
        String pixmap,
        XtPointer user_data,
#if NeedWidePrototypes
        int underline )
#else /* NeedWidePrototypes */
        Boolean underline )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
   XmIconGadget gadget;
   int size;
   XmIconGadget w_new;
   Dimension h, w;

   /* Create the w_new instance structure */
   gadget = (XmIconGadget) widget;
   size = XtClass(gadget)->core_class.widget_size;
   w_new = (XmIconGadget)XtMalloc(size);

   /* Copy the master into the duplicate */
   memcpy( (char *)w_new, (char *)gadget, (int)size);

   Icon_Cache(w_new) = (XmIconGCacheObjPart *)
                       _XmCachePart(Icon_ClassCachePart(w_new),
                                    Icon_Cache(w_new),
                                    sizeof(XmIconGCacheObjPart));

   /* Certain fields need to be updated */
   w_new->object.parent = parent;
   w_new->object.self = (Widget)w_new;
   IG_FontList(w_new) = XmFontListCopy(IG_FontList(gadget));

   /* Certain fields should not be inherited by the clone */
   w_new->object.destroy_callbacks = NULL;
   w_new->object.constraints = NULL;
   w_new->gadget.help_callback = NULL;
   w_new->rectangle.managed = False;
   IG_Callback(w_new) = NULL;

   /* Set the user_data field */
   w_new->gadget.user_data = user_data;

   /* Process the optional pixmap name */
   if ((pixmap == NULL) || LoadPixmap(w_new, pixmap))
   {
      /* No pixmap to load */
      IG_ImageName(w_new) = NULL;
      IG_Pixmap(w_new) = NULL;
      IG_PixmapWidth(w_new) = 0;
      IG_PixmapHeight(w_new) = 0;
   }

   /* Process the required label string */
   IG_String(w_new) = _XmStringCreate(string);
   _XmStringExtent(IG_FontList(w_new), IG_String(w_new), &w, &h);
   IG_Underline(w_new) = underline;
   if (IG_Underline(w_new))
      h++;
   IG_StringWidth(w_new) = w;
   _XmAssignIconG_StringHeight(w_new, h);

   _XmReCacheIconG(w_new);

   /* Get copies of the GC's */
   IG_NormalGC(w_new) = NULL;
   IG_BackgroundGC(w_new) = NULL;
   IG_ArmedGC(w_new) = NULL;
   IG_ArmedBackgroundGC(w_new) = NULL;
   IG_ClipGC(w_new) = NULL;
   UpdateGCs((Widget) w_new);

   /* Size the gadget */
   IG_GetSize((Widget) w_new, &w, &h);
   IG_Width(w_new) = w;
   IG_Height(w_new) = h;

   /* Insert the duplicate into the parent's child list */
   (*(((CompositeWidgetClass)XtClass(XtParent(w_new)))->composite_class.
       insert_child))((Widget)w_new);

   return ((Widget) w_new);
}


Boolean 
#ifdef _NO_PROTO
_XmIconGSelectInTitle( widget, pt_x, pt_y )
        Widget widget ;
        Position pt_x ;
        Position pt_y ;
#else
_XmIconGSelectInTitle(
        Widget widget,
#if NeedWidePrototypes
        int pt_x,
        int pt_y )
#else /* NeedWidePrototypes */
        Position pt_x,
        Position pt_y )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */

{
   XmIconGadget	g = 	(XmIconGadget) widget;
   Position	x, y;
   Dimension	w, h, h_t, s_t;
   XRectangle	clip;
   Position	p_x, p_y, s_x, s_y;

   h_t = 0;
   s_t = G_ShadowThickness(g);
   x = IG_X(g);
   y = IG_Y(g);
   w = IG_Width (g); 
   h = IG_Height (g);
   IG_GetPositions ((Widget) g, w, h, h_t, s_t, &p_x, &p_y, &s_x, &s_y);

   if (IG_String (g))
   {
      clip.x = x + s_x;
      clip.y = y + s_y;
      clip.width = (s_x + s_t + h_t >= IG_Width (g))
         ? 0 : Min (IG_StringWidth (g), IG_Width (g) - s_x - s_t - h_t);
      clip.height = (s_y + s_t + h_t >= IG_Height (g))
         ? 0 : Min (IG_StringHeight (g), IG_Height (g) - s_y - s_t - h_t);
      if (clip.width <= 0 || clip.height <= 0)
         return(False);
      else
      {
         if ((pt_x >= clip.x) && (pt_y >= clip.y) &&
             (pt_x <= clip.x + clip.width) && (pt_y <= clip.y + clip.height))
            return(True);
         else
            return(False);
      }
   }
   else
      return(False);
}


/*
 * Returns a pointer to a static storage area; must not be freed.
 */

XRectangle * 
#ifdef _NO_PROTO
_XmIconGGetTextExtent( widget )
        Widget widget ;
#else
_XmIconGGetTextExtent(
        Widget widget )
#endif /* _NO_PROTO */

{
   XmIconGadget	g = 	(XmIconGadget) widget;
   Position	x, y;
   Dimension	w, h, h_t, s_t;
   static XRectangle	clip;
   Position	p_x, p_y, s_x, s_y;

   h_t = 0;
   s_t = G_ShadowThickness(g);
   x = IG_X(g);
   y = IG_Y(g);
   w = IG_Width (g); 
   h = IG_Height (g);
   IG_GetPositions ((Widget) g, w, h, h_t, s_t, &p_x, &p_y, &s_x, &s_y);

   if (IG_String (g))
   {
      clip.x = x + s_x;
      clip.y = y + s_y;
      clip.width = (s_x + s_t + h_t >= IG_Width (g))
         ? 0 : Min (IG_StringWidth (g), IG_Width (g) - s_x - s_t - h_t);
      clip.height = (s_y + s_t + h_t >= IG_Height (g))
         ? 0 : Min (IG_StringHeight (g), IG_Height (g) - s_y - s_t - h_t);

      if (clip.width <= 0)
         clip.width = 0;

      if (clip.height <= 0)
         clip.height = 0;
   }
   else
   {
      clip.x = 0;
      clip.y = 0;
      clip.height = 0;
      clip.width = 0;
   }

   return(&clip);
}

/*-------------------------------------------------------------
**	_XmIconGGetIconRects
**		Returns rects occupied by label and pixmap
*/
void 
#ifdef _NO_PROTO
_XmIconGGetIconRects( gw, flags, rect1, rect2 )
        Widget gw ;
        unsigned char *flags ;
        XRectangle *rect1 ;
        XRectangle *rect2 ;
#else
_XmIconGGetIconRects(
        Widget gw,
        unsigned char *flags,
        XRectangle *rect1,
        XRectangle *rect2 )
#endif /* _NO_PROTO */

{
   XmIconGadget g = (XmIconGadget) gw ;
   Position	p_x, p_y, s_x, s_y;
   Dimension	width, height;
   Position     adj_x, adj_y;
   Dimension    h_t, s_t;

   h_t = G_HighlightThickness(g);
   s_t = G_ShadowThickness(g);

   adj_x = IG_MarginWidth(g);
   adj_y = IG_MarginHeight(g);
   IG_GetPositions((Widget) g, IG_Width(g), IG_Height(g), h_t, s_t, &p_x, &p_y, &s_x, &s_y);
   *flags = 0;

   if (IG_Pixmap (g))
   {
      width = (p_x + s_t + h_t >= IG_Width (g)) ? 0 : 
                 Min (IG_PixmapWidth (g), IG_Width (g) - p_x - s_t - h_t);
      height = (p_y + s_t + h_t >= IG_Height (g)) ? 0 : 
                 Min (IG_PixmapHeight (g), IG_Height (g) - p_y - s_t - h_t);
      if (width > 0 && height > 0)
      {
         rect1->x = IG_X(g) + p_x - adj_x; 
         rect1->y = IG_Y(g) + p_y - adj_y;
         rect1->width = width + (2 * adj_y);
         rect1->height = height + (2 * adj_x);
         *flags |= XmPIXMAP_RECT;
      }
   }

   if (IG_String(g))
   {
      width = (s_x + s_t + h_t >= IG_Width (g)) ? 0 : 
                    Min (IG_StringWidth (g), IG_Width (g) - s_x - s_t - h_t);
      height = (s_y + s_t + h_t >= IG_Height (g)) ? 0 : 
                    Min (IG_StringHeight (g), IG_Height (g) - s_y - s_t - h_t);
      if (width > 0 && height > 0)
      {
         rect2->x = IG_X(g) + s_x - adj_x;
         rect2->y = IG_Y(g) + s_y - adj_y;
         rect2->width = width + (2 * adj_y);
         rect2->height = height + (2 * adj_x);
         *flags |= XmLABEL_RECT;
      }
   }
}

/*-------------------------------------------------------------
**	Public Entry Points
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	XmCreateIconGadget
**		Create a w_new gadget instance.
**-------------------------------------------------------------
*/
Widget 
#ifdef _NO_PROTO
XmCreateIconGadget( parent, name, arglist, argcount )
        Widget parent ;
        String name ;
        ArgList arglist ;
        Cardinal argcount ;
#else
XmCreateIconGadget(
        Widget parent,
        String name,
        ArgList arglist,
        Cardinal argcount )
#endif /* _NO_PROTO */
{
	return (XtCreateWidget (name, xmIconGadgetClass, 
			parent, arglist, argcount));
}

