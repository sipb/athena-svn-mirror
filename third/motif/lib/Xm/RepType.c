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
static char rcsid[] = "$RCSfile: RepType.c,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:56 $"
#endif
#endif
/*
*  (c) Copyright 1991, 1992 HEWLETT-PACKARD COMPANY */

#include "XmI.h"
#include "RepTypeI.h"
#include "MessagesI.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MESSAGE1 _XmMsgRepType_0001
#define MESSAGE2 _XmMsgRepType_0002


#ifdef I18N_MSG
#include "XmMsgI.h"
#endif


/* INSTRUCTIONS to add a statically-stored representation type:
 *    (For dynamically allocated/created representation types, see the
 *     man page for XmRepTypeRegister).
 *
 *  1) Determine whether or not the numerical values of the representation
 *     type can be enumerated with consecutive numerical values beginning 
 *     with value zero.  If this is the case, continue with step 2).
 *
 *     If this is not the case, the representation type needs an extra
 *     array in the data structure to map the numerical resource value to
 *     the array position of the value name in the representation type data
 *     structures.  If the representation type must be mapped in this way,
 *     go to step 2M).
 *
 *  2) Define a static array of the names of the values for the
 *     representation type in RepType.c.  Use the representation type name,
 *     plus the suffix "Names" for the name of the array (see existing name
 *     arrays for an example).  The ordering of the value names in this
 *     array determines the numercial value of each name, beginning with
 *     zero and incrementing consecutively.
 *
 *  3) Add an enumeration symbol for the ID number of the representation
 *     type in the enum statement in the RepTypeI.h module.  There are
 *     two sections to the enumerated list; add the new type (using the
 *     XmRID prefix) to the FIRST section in ALPHABETICAL ORDER!!!
 *     The beginning of the first section can be identified by the
 *     assignment using XmREP_TYPE_STD_TAG.
 *
 *  4) Add an element to the static array of representation type data
 *     structures named "_XmStandardRepTypes".  Add the new element to
 *     the array in ALPHABETICAL ORDER (according to the name of the
 *     representation type).  Use the same format as the other elements
 *     in the array; the fields which are initialized with FALSE and 
 *     NULL should be the same for all elements of the array.
 *
 *  5) You're done.  A generic "string to representation type" converter
 *     for the representation type that you just added will be automatically
 *     registered when all other Xm converters are registered.
 *
 ******** For "mapped" representation types: ********
 *
 *  2M) Define a static array of the numerical values for the
 *     representation type in RepType.c.  Use the enumerated symbols
 *     (generally defined in Xm.h) to initialize the array of numerical
 *     values.  Use the representation type name, plus the suffix "Map"
 *     for the name of the array (see existing map arrays for an example).
 *
 *  3M) Define a static array of the names of the values for the
 *     representation type in RepType.c.  Use the representation type name,
 *     plus the suffix "Names" for the name of the array (see existing name
 *     arrays for an example).  The ordering of the value names in this
 *     array determines the numercial value of each name, with the first
 *     element corresponding to the first element in the "Map" array, etc.
 *
 *  4M) Add an enumeration symbol for the ID number of the representation
 *     type in the enum statement in the RepTypeI.h module.  There are
 *     two sections to the enumerated list; add the new type (using the
 *     XmRID prefix) to the SECOND section in ALPHABETICAL ORDER!!!
 *     The beginning of the second section can be identified by the
 *     assignment using XmREP_TYPE_MAP_TAG.
 *
 *  5M) Add an element to the static array of representation type data
 *     structures named "_XmStandardMappedRepTypes".  Add the new element
 *     to the array in ALPHABETICAL ORDER (according to the name of the
 *     representation type).  Use the same format as the other elements
 *     in the array; the fields which are initialized with FALSE should
 *     be the same for all elements of the array.
 *
 *  6M) You're done.  A generic "string to representation type" converter
 *     for the representation type that you just added will be automatically
 *     registered when all other Xm converters are registered.
 */

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static String * CopyStringArray() ;
static Boolean ValuesConsecutive() ;
static XmRepTypeEntry GetRepTypeRecord() ;
static unsigned int GetByteDataSize() ;
static void CopyRecord() ;
static Boolean ConvertRepType() ;
static Boolean ReverseConvertRepType() ;

#else

static String * CopyStringArray( 
                        String *StrArray,
#if NeedWidePrototypes
                        unsigned int NumEntries,
                        int NullTerminate,
                        int UppercaseFormat) ;
#else
                        unsigned char NumEntries,
                        Boolean NullTerminate,
                        Boolean UppercaseFormat) ;
#endif /* NeedWidePrototypes */
static Boolean ValuesConsecutive( 
                        unsigned char *values,
#if NeedWidePrototypes
                        unsigned int num_values) ;
#else
                        unsigned char num_values) ;
#endif /* NeedWidePrototypes */
static XmRepTypeEntry GetRepTypeRecord( 
#if NeedWidePrototypes
                        int rep_type_id) ;
#else
                        XmRepTypeId rep_type_id) ;
#endif /* NeedWidePrototypes */
static unsigned int GetByteDataSize( 
                        XmRepTypeEntry Record) ;
static void CopyRecord( 
                        XmRepTypeEntry Record,
                        XmRepTypeEntry OutputEntry,
                        XtPointer *PtrDataArea,
                        XtPointer *ByteDataArea) ;
static Boolean ConvertRepType( 
                        Display *disp,
                        XrmValue *args,
                        Cardinal *n_args,
                        XrmValue *from,
                        XrmValue *to,
                        XtPointer *converter_data) ;
static Boolean ReverseConvertRepType( 
                        Display *disp,
                        XrmValue *args,
                        Cardinal *n_args,
                        XrmValue *from,
                        XrmValue *to,
                        XtPointer *converter_data) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


static String AlignmentNames[] =
{   "alignment_beginning", "alignment_center", "alignment_end"
    } ;
static String AnimationStyleNames[] =
{   "drag_under_none", "drag_under_pixmap", "drag_under_shadow_in",
    "drag_under_shadow_out", "drag_under_highlight"
    };
static String ArrowDirectionNames[] =
{   "arrow_up", "arrow_down", "arrow_left", "arrow_right"
    } ;
static String AttachmentNames[] =
{   "attach_none", "attach_form", "attach_opposite_form", "attach_widget",
    "attach_opposite_widget", "attach_position", "attach_self"
    } ;
static String AudibleWarningNames[] =
{   "none", "bell"
    } ;
static String BlendModelNames[] =
{   "blend_all", "blend_state_source", "blend_just_source", "blend_none"
    } ;
#define ChildHorizontalAlignmentNames   AlignmentNames

static char *ChildPlacementNames[] =
{   "place_top", "place_above_selection", "place_below_selection"
    } ;
static char *ChildTypeNames[] =
{   "frame_generic_child", "frame_workarea_child", "frame_title_child"
    } ;
static String ChildVerticalAlignmentNames[] =
{   "alignment_baseline_top", "alignment_center", "alignment_baseline_bottom",
    "alignment_widget_top", "alignment_widget_bottom"
    } ;
static String CommandWindowLocationNames[] =
{   "command_above_workspace", "command_below_workspace"
    } ;
static String DialogTypeNames[] =
{   "dialog_template", "dialog_error", "dialog_information", "dialog_message",
    "dialog_question", "dialog_warning", "dialog_working"
    } ;
static String DragInitiatorProtocolStyleNames[] =
{   "drag_none", "drag_drop_only", "drag_prefer_preregister",
    "drag_preregister", "drag_prefer_dynamic", "drag_dynamic",
    "drag_prefer_receiver"
	};
static String DragReceiverProtocolStyleNames[] =
{   "drag_none", "drag_drop_only", "drag_prefer_preregister",
    "drag_preregister", "drag_prefer_dynamic", "drag_dynamic"
	};
static String DropSiteActivityNames[] =
{   "drop_site_active", "drop_site_inactive"
	};
static String DropSiteTypeNames[] =
{   "drop_site_simple",   "drop_site_composite"
	};
static String EditModeNames[] =
{   "multi_line_edit", "single_line_edit"
    } ;
static String IconAttachmentNames[] =
{   "attach_north_west", "attach_north", "attach_north_east", "attach_east",
    "attach_south_east", "attach_south", "attach_south_west", "attach_west",
    "attach_center", "attach_hot"
    } ;
static String ListSizePolicyNames[] =
{   "variable", "constant", "resize_if_possible"
    } ;
static String MultiClickNames[] =
{   "multiclick_discard", "multiclick_keep"
    } ;
static String NavigationTypeNames[] =
{   "none", "tab_group", "sticky_tab_group", "exclusive_tab_group"
    } ;
static String ProcessingDirectionNames[] =
{   "max_on_top", "max_on_bottom", "max_on_left", "max_on_right"
    } ;
static String ResizePolicyNames[] =
{   "resize_none", "resize_grow", "resize_any"
    } ;
static String RowColumnTypeNames[] =
{   "work_area", "menu_bar", "menu_pulldown", "menu_popup", "menu_option"
    } ;
static String ScrollBarDisplayPolicyNames[] =
{   "static", "as_needed"
    } ;
static String ScrollBarPlacementNames[] =
{   "bottom_right", "top_right", "bottom_left", "top_left"
    } ;
static String ScrollingPolicyNames[] =
{   "automatic", "application_defined"
    } ;
static String SelectionPolicyNames[] =
{   "single_select", "multiple_select", "extended_select", "browse_select"
    } ;
static String SelectionTypeNames[] =
{   "dialog_work_area", "dialog_prompt", "dialog_selection", "dialog_command",
    "dialog_file_selection"
    } ;
static String SeparatorTypeNames[] = 
{   "no_line", "single_line", "double_line", "single_dashed_line",
    "double_dashed_line", "shadow_etched_in", "shadow_etched_out",
    "shadow_etched_in_dash", "shadow_etched_out_dash"
    } ;
static String StringDirectionNames[] =
{   "string_direction_l_to_r", "string_direction_r_to_l"
    } ;
static String TearOffModelNames[] =
{   "tear_off_enabled", "tear_off_disabled"
    } ;
static String UnitTypeNames[] =
{   "pixels", "100th_millimeters", "1000th_inches", "100th_points",
    "100th_font_units"
    } ;
static String UnpostBehaviorNames[] =
{   "unpost", "unpost_and_replay"
    } ;
static String VerticalAlignmentNames[] =
{   "alignment_baseline_top", "alignment_center", "alignment_baseline_bottom",
    "alignment_contents_top", "alignment_contents_bottom"
    } ;
static String VisualPolicyNames[] =
{   "variable", "constant"
    } ;


static XmRepTypeEntryRec _XmStandardRepTypes[] =
{   
    {	XmRAlignment, AlignmentNames, NULL,
	XtNumber( AlignmentNames), FALSE,
	XmRID_ALIGNMENT
	},
    {	XmRAnimationStyle, AnimationStyleNames, NULL,
	XtNumber( AnimationStyleNames), FALSE,
	XmRID_ANIMATION_STYLE
	},
    {	XmRArrowDirection, ArrowDirectionNames, NULL,
	XtNumber( ArrowDirectionNames), FALSE,
	XmRID_ARROW_DIRECTION
	},
    {	XmRAttachment, AttachmentNames, NULL,
	XtNumber( AttachmentNames), FALSE,
	XmRID_ATTACHMENT
	},
    {	XmRAudibleWarning, AudibleWarningNames, NULL,
	XtNumber( AudibleWarningNames), FALSE,
	XmRID_AUDIBLE_WARNING
	},
    {	XmRBlendModel, BlendModelNames, NULL,
	XtNumber( BlendModelNames), FALSE,
	XmRID_BLEND_MODEL
	},
    {	XmRChildHorizontalAlignment, ChildHorizontalAlignmentNames, NULL,
	XtNumber( ChildHorizontalAlignmentNames), FALSE,
	XmRID_CHILD_HORIZONTAL_ALIGNMENT
	},
    {	XmRChildPlacement, ChildPlacementNames, NULL,
	XtNumber( ChildPlacementNames), FALSE,
	XmRID_CHILD_PLACEMENT
	},
    {	XmRChildType, ChildTypeNames, NULL,
	XtNumber( ChildTypeNames), FALSE,
	XmRID_CHILD_TYPE
	},
    {	XmRChildVerticalAlignment, ChildVerticalAlignmentNames, NULL,
	XtNumber( ChildVerticalAlignmentNames), FALSE,
	XmRID_CHILD_VERTICAL_ALIGNMENT
	},
    {	XmRCommandWindowLocation, CommandWindowLocationNames, NULL,
	XtNumber( CommandWindowLocationNames), FALSE,
	XmRID_COMMAND_WINDOW_LOCATION
	},
    {	XmRDialogType, DialogTypeNames, NULL,
	XtNumber( DialogTypeNames), FALSE,
	XmRID_DIALOG_TYPE
	},
    {	XmRDragInitiatorProtocolStyle, DragInitiatorProtocolStyleNames, NULL,
	XtNumber( DragInitiatorProtocolStyleNames), FALSE,
	XmRID_DRAG_INITIATOR_PROTOCOL_STYLE
	},
    {	XmRDragReceiverProtocolStyle, DragReceiverProtocolStyleNames, NULL,
	XtNumber( DragReceiverProtocolStyleNames), FALSE,
	XmRID_DRAG_RECEIVER_PROTOCOL_STYLE
	},
    {	XmRDropSiteActivity, DropSiteActivityNames, NULL,
	XtNumber( DropSiteActivityNames), FALSE,
	XmRID_DROP_SITE_ACTIVITY
	},
    {	XmRDropSiteType, DropSiteTypeNames, NULL,
	XtNumber( DropSiteTypeNames), FALSE,
	XmRID_DROP_SITE_TYPE
	},
    {	XmREditMode, EditModeNames, NULL,
	XtNumber( EditModeNames), FALSE,
	XmRID_EDIT_MODE
	},
    {	XmRIconAttachment, IconAttachmentNames, NULL,
	XtNumber( IconAttachmentNames), FALSE,
	XmRID_ICON_ATTACHMENT
	},
    {	XmRListSizePolicy, ListSizePolicyNames, NULL,
	XtNumber( ListSizePolicyNames), FALSE,
	XmRID_LIST_SIZE_POLICY
	},
    {	XmRMultiClick, MultiClickNames, NULL,
	XtNumber( MultiClickNames), FALSE,
	XmRID_MULTI_CLICK
	},
    {	XmRNavigationType, NavigationTypeNames, NULL,
	XtNumber( NavigationTypeNames), FALSE,
	XmRID_NAVIGATION_TYPE
	},
    {	XmRProcessingDirection, ProcessingDirectionNames, NULL,
	XtNumber( ProcessingDirectionNames), FALSE,
	XmRID_PROCESSING_DIRECTION
	},
    {	XmRResizePolicy, ResizePolicyNames, NULL,
	XtNumber( ResizePolicyNames), FALSE,
	XmRID_RESIZE_POLICY
	},
    {	XmRRowColumnType, RowColumnTypeNames, NULL,
	XtNumber( RowColumnTypeNames), FALSE,
	XmRID_ROW_COLUMN_TYPE
	},
    {	XmRScrollBarDisplayPolicy, ScrollBarDisplayPolicyNames, NULL,
	XtNumber( ScrollBarDisplayPolicyNames), FALSE,
	XmRID_SCROLL_BAR_DISPLAY_POLICY
	},
    {	XmRScrollBarPlacement, ScrollBarPlacementNames, NULL,
	XtNumber( ScrollBarPlacementNames), FALSE,
	XmRID_SCROLL_BAR_PLACEMENT
	},
    {	XmRScrollingPolicy, ScrollingPolicyNames, NULL,
	XtNumber( ScrollingPolicyNames), FALSE,
	XmRID_SCROLLING_POLICY
	},
    {	XmRSelectionPolicy, SelectionPolicyNames, NULL,
	XtNumber( SelectionPolicyNames), FALSE,
	XmRID_SELECTION_POLICY
	},
    {	XmRSelectionType, SelectionTypeNames, NULL,
	XtNumber( SelectionTypeNames), FALSE,
	XmRID_SELECTION_TYPE
	},
    {	XmRSeparatorType, SeparatorTypeNames, NULL,
	XtNumber( SeparatorTypeNames), FALSE,
	XmRID_SEPARATOR_TYPE
	},
    {	XmRStringDirection, StringDirectionNames, NULL,
	XtNumber( StringDirectionNames), FALSE,
	XmRID_STRING_DIRECTION
	},
    {	XmRTearOffModel, TearOffModelNames, NULL,
	XtNumber( TearOffModelNames), FALSE,
	XmRID_TEAR_OFF_MODEL
	},
    {	XmRUnitType, UnitTypeNames, NULL,
	XtNumber( UnitTypeNames), FALSE,
	XmRID_UNIT_TYPE
	},
    {	XmRUnpostBehavior, UnpostBehaviorNames, NULL,
	XtNumber( UnpostBehaviorNames), FALSE,
	XmRID_UNPOST_BEHAVIOR
	},
    {	XmRVerticalAlignment, VerticalAlignmentNames, NULL,
	XtNumber( VerticalAlignmentNames), FALSE,
	XmRID_VERTICAL_ALIGNMENT
	},
    {	XmRVisualPolicy, VisualPolicyNames, NULL,
	XtNumber( VisualPolicyNames), FALSE,
	XmRID_VISUAL_POLICY
	}
    } ;

static String DefaultButtonTypeNames[] =
{   "dialog_none", "dialog_cancel_button", "dialog_ok_button",
    "dialog_help_button"
    } ;
static unsigned char DefaultButtonTypeMap[] = 
{   XmDIALOG_NONE, XmDIALOG_CANCEL_BUTTON, XmDIALOG_OK_BUTTON,
    XmDIALOG_HELP_BUTTON
    } ;
static String DialogStyleNames[] =
{   "dialog_modeless", "dialog_work_area", "dialog_primary_application_modal",
    "dialog_application_modal", "dialog_full_application_modal",
    "dialog_system_modal"
    } ;
static unsigned char DialogStyleMap[] =
{   XmDIALOG_MODELESS, XmDIALOG_WORK_AREA, XmDIALOG_PRIMARY_APPLICATION_MODAL,
    XmDIALOG_APPLICATION_MODAL, XmDIALOG_FULL_APPLICATION_MODAL,
    XmDIALOG_SYSTEM_MODAL
    } ;
static String FileTypeMaskNames[] =
{   "file_directory", "file_regular", "file_any_type"
    } ;
static unsigned char FileTypeMaskMap[] = 
{   XmFILE_DIRECTORY, XmFILE_REGULAR, XmFILE_ANY_TYPE
    } ;
static String IndicatorTypeNames[] =
{   "n_of_many", "one_of_many"
    } ;
static unsigned char IndicatorTypeMap[] = 
{   XmN_OF_MANY, XmONE_OF_MANY
    } ;
static String LabelTypeNames[] =
{   "pixmap", "string"
    } ;
static unsigned char LabelTypeMap[] = 
{   XmPIXMAP, XmSTRING
    } ;
static String OrientationNames[] =
{   "vertical", "horizontal"
    } ;
static unsigned char OrientationMap[] = 
{   XmVERTICAL, XmHORIZONTAL
    } ;
static String PackingNames[] =
{   "pack_tight", "pack_column", "pack_none"
    } ;
static unsigned char PackingMap[] =
{   XmPACK_TIGHT, XmPACK_COLUMN, XmPACK_NONE
    } ;
static String ShadowTypeNames[] =
{   "shadow_etched_in", "shadow_etched_out", "shadow_in", "shadow_out"
    } ;
static unsigned char ShadowTypeMap[] = 
{   XmSHADOW_ETCHED_IN, XmSHADOW_ETCHED_OUT, XmSHADOW_IN, XmSHADOW_OUT
    } ;
static String WhichButtonNames[] =
{   "button1", "1", "button2", "2", "button3", "3", "button4", "4", 
    "button5", "5"
    } ;
static unsigned char WhichButtonMap[] = 
{   Button1, Button1, Button2, Button2, Button3, Button3, Button4, Button4,
    Button5, Button5
    } ;

static XmRepTypeEntryRec _XmStandardMappedRepTypes[] = 
{   
    {	XmRDefaultButtonType, DefaultButtonTypeNames, DefaultButtonTypeMap,
	XtNumber( DefaultButtonTypeNames), FALSE,
	XmRID_DEFAULT_BUTTON_TYPE
	},
    {	XmRDialogStyle, DialogStyleNames, DialogStyleMap,
	XtNumber( DialogStyleNames), FALSE,
	XmRID_DIALOG_STYLE
	},
    {	XmRFileTypeMask, FileTypeMaskNames, FileTypeMaskMap,
	XtNumber( FileTypeMaskNames), FALSE,
	XmRID_FILE_TYPE_MASK
	},
    {	XmRIndicatorType, IndicatorTypeNames, IndicatorTypeMap,
	XtNumber( IndicatorTypeNames), FALSE,
	XmRID_INDICATOR_TYPE
	},
    {	XmRLabelType, LabelTypeNames, LabelTypeMap,
	XtNumber( LabelTypeNames), FALSE,
	XmRID_LABEL_TYPE
	},
    {	XmROrientation, OrientationNames, OrientationMap,
	XtNumber( OrientationNames), FALSE,
	XmRID_ORIENTATION
	},
    {	XmRPacking, PackingNames, PackingMap,
	XtNumber( PackingNames), FALSE,
	XmRID_PACKING
	},
    {	XmRShadowType, ShadowTypeNames, ShadowTypeMap,
	XtNumber( ShadowTypeNames), FALSE,
	XmRID_SHADOW_TYPE
	},
    {	XmRWhichButton, WhichButtonNames, WhichButtonMap, 
	XtNumber( WhichButtonNames), FALSE,
	XmRID_WHICH_BUTTON
	}
    } ;

    static XmRepTypeEntryRec *_XmRepTypes = NULL;
    static unsigned short _XmRepTypeNumRecords = 0;



static String *
#ifdef _NO_PROTO
CopyStringArray( StrArray, NumEntries, NullTerminate, UppercaseFormat)
        String *StrArray ;
        unsigned char NumEntries ;
        Boolean NullTerminate ;
        Boolean UppercaseFormat ;
#else
CopyStringArray(
        String *StrArray,
#if NeedWidePrototypes
        unsigned int NumEntries,
        int NullTerminate,
        int UppercaseFormat)
#else
        unsigned char NumEntries,
        Boolean NullTerminate,
        Boolean UppercaseFormat)
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{   
    unsigned int AllocSize = 0 ;
    unsigned int Index ;
    char **TmpStr ;
    char *NextString ;
    char *SrcPtr ;

    Index = 0 ;
    while(    Index < NumEntries    )
      {   
	  AllocSize += strlen( StrArray[Index++]) + 1 ;
      } 
    AllocSize += NumEntries * sizeof( String) ;

    if(    NullTerminate    )
      {   AllocSize += sizeof( String *) ;
      } 
    if(    UppercaseFormat    )
      {   AllocSize += NumEntries << 1 ;
      } 
    TmpStr = (char **) XtMalloc( AllocSize) ;

    NextString = (char *) (TmpStr + NumEntries) ;

    if(    NullTerminate    )
      {   NextString += sizeof( String *) ;
      } 
    Index = 0 ;
    if(    UppercaseFormat    )
      {   
	  while(    Index < NumEntries    )
	    {   
		SrcPtr = StrArray[Index] ;
		TmpStr[Index] = NextString ;
		*NextString++ = 'X' ;
		*NextString++ = 'm' ;
		while ((*NextString++ =
			((unsigned char)islower((unsigned char)*SrcPtr)) ?
			toupper( (unsigned char)*SrcPtr++) : *SrcPtr++) != '\0')
		  { } 
		++Index ;
	    } 
      } 
    else
      {   while(    Index < NumEntries    )
	    {   
		SrcPtr = StrArray[Index] ;
		TmpStr[Index] = NextString ;
		while(    (*NextString++ = *SrcPtr++) != '\0'    ){ } 
		++Index ;
	    } 
        } 
    if(    NullTerminate    )
      {   TmpStr[Index] = NULL ;
      } 
    return( TmpStr) ;
} 

static Boolean
#ifdef _NO_PROTO
ValuesConsecutive( values, num_values)
        unsigned char *values ;
        unsigned char num_values ;
#else
ValuesConsecutive(
        unsigned char *values,
#if NeedWidePrototypes
        unsigned int num_values)
#else
        unsigned char num_values)
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{   
    if(    values    )
      {   while(    num_values--    )
	    {   if(    num_values != values[num_values]    )
		  {   return( FALSE) ;
		  } 
	      } 
        } 
    return( TRUE) ;
} 

XmRepTypeId
#ifdef _NO_PROTO
XmRepTypeRegister( rep_type, value_names, values, num_values)
        String rep_type ;
        String *value_names ;
        unsigned char *values ;
        unsigned char num_values ;
#else
XmRepTypeRegister(
        String rep_type,
        String *value_names,
        unsigned char *values,
#if NeedWidePrototypes
        unsigned int num_values)
#else
        unsigned char num_values)
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{     
    XmRepTypeEntry NewRecord ;
    XtConvertArgRec convertArg;
    unsigned short NumRecords;

    if(    !num_values    )
      {   return( XmREP_TYPE_INVALID) ;
      } 

    NumRecords = _XmRepTypeNumRecords;
    _XmRepTypes = (XmRepTypeList) 
      XtRealloc( (char *) _XmRepTypes,
		(sizeof(XmRepTypeEntryRec) * (NumRecords + 1))) ;

    ++_XmRepTypeNumRecords;

    NewRecord = &_XmRepTypes[NumRecords] ;

    if(    ValuesConsecutive( values, num_values)    )
      {
	  NewRecord->values = NULL;
	  NewRecord->rep_type_id = NumRecords | XmREP_TYPE_RT_TAG ;
      } else {
	  if ((NewRecord->values = (unsigned char *) 
	       XtMalloc(sizeof(unsigned char) * num_values)) != NULL)
	    memcpy(NewRecord->values, values, (size_t)num_values);
	  NewRecord->rep_type_id = NumRecords | XmREP_TYPE_RT_MAP_TAG ;
      } 
    NewRecord->num_values = num_values ;
    NewRecord->rep_type_name = strcpy( XtMalloc( strlen( rep_type) + 1),
				      rep_type) ;
    NewRecord->value_names = CopyStringArray( value_names, num_values,
					     FALSE, FALSE) ;
    convertArg.address_mode = XtImmediate;
    convertArg.address_id   = (XPointer)(long)NewRecord->rep_type_id;
    convertArg.size         = sizeof(convertArg.address_id);

    XtSetTypeConverter( XmRString, NewRecord->rep_type_name, ConvertRepType,
		       &convertArg, 1, XtCacheNone, NULL) ;

    NewRecord->reverse_installed = FALSE ;

    return( NewRecord->rep_type_id) ;
}

static XmRepTypeEntry
#ifdef _NO_PROTO
GetRepTypeRecord( rep_type_id )
     XmRepTypeId rep_type_id ;
#else
GetRepTypeRecord(
#if NeedWidePrototypes
     int rep_type_id)
#else
     XmRepTypeId rep_type_id)
#endif
#endif /* _NO_PROTO */
{   
    if(    rep_type_id != XmREP_TYPE_INVALID    )
      {   
	  if(    XmREP_TYPE_STD( rep_type_id)    )
	    {
		return( &_XmStandardRepTypes[XmREP_TYPE_OFFSET( rep_type_id)]) ;
            } else {
		if(    XmREP_TYPE_STD_MAP( rep_type_id)    )
		  {   return( &_XmStandardMappedRepTypes[XmREP_TYPE_OFFSET( rep_type_id)]) ;
		  } else {
		      return( &_XmRepTypes[XmREP_TYPE_OFFSET( rep_type_id)]) ;
		  }
	    }
      }
    return (XmRepTypeEntry)NULL ;
} 

void
#ifdef _NO_PROTO
XmRepTypeAddReverse( rep_type_id )
     XmRepTypeId rep_type_id ;
#else
XmRepTypeAddReverse(
#if NeedWidePrototypes
     int rep_type_id)
#else
     XmRepTypeId rep_type_id)
#endif
#endif /* _NO_PROTO */
{     

    XtConvertArgRec convertArg;

    XmRepTypeEntry Record = GetRepTypeRecord( rep_type_id) ;

    if(    Record  &&  !Record->reverse_installed    )
      {   
	  convertArg.address_mode = XtImmediate;
	  convertArg.address_id   = (XPointer)(long)Record->rep_type_id;
	  convertArg.size         = sizeof(convertArg.address_id);
	  XtSetTypeConverter( Record->rep_type_name, XmRString,
			     ReverseConvertRepType, &convertArg,
			     1, XtCacheNone, NULL) ;
	  Record->reverse_installed = TRUE ;
      } 
    return ;
}

Boolean
#ifdef _NO_PROTO
XmRepTypeValidValue( rep_type_id, test_value, enable_default_warning )
     XmRepTypeId rep_type_id ;
     unsigned char test_value ;
     Widget enable_default_warning ;
#else
XmRepTypeValidValue(
#if NeedWidePrototypes
     int rep_type_id,
     unsigned int test_value,
#else
     XmRepTypeId rep_type_id,
     unsigned char test_value,
#endif
     Widget enable_default_warning)
#endif /* _NO_PROTO */
{
    XmRepTypeEntry Record = GetRepTypeRecord( rep_type_id) ;
    
    if( !Record ){
	if(    enable_default_warning )
#ifdef I18N_MSG
        {   _XmWarning( enable_default_warning,
		catgets(Xm_catd,MS_RepType,MSG_REP_1, MESSAGE1)) ;
#else
        {   _XmWarning( enable_default_warning, MESSAGE1) ;
#endif
	}
    } else {
	if (Record->values) {
	    unsigned int Index;
	    for (Index=0; Index < Record->num_values; Index++ ) {
		if (Record->values[Index] == test_value) {
		    return(TRUE) ;
		} 
	    }
	} else if( test_value < Record->num_values ){
	    return( TRUE) ;
	}
	if(    enable_default_warning    )
	  {   
	      char msg[256] ;

#ifdef I18N_MSG
            sprintf( msg, catgets(Xm_catd,MS_RepType,MSG_REP_2,
			MESSAGE2),
			test_value, Record->rep_type_name) ;
#else
            sprintf( msg, MESSAGE2, test_value,
                                                       Record->rep_type_name) ;
#endif
	      _XmWarning( enable_default_warning, msg) ;
	  } 
    }
    return FALSE ;
}

static unsigned int
#ifdef _NO_PROTO
GetByteDataSize( Record )
     XmRepTypeEntry Record ;
#else
GetByteDataSize(
        XmRepTypeEntry Record)
#endif /* _NO_PROTO */
{   
    String * valNames ;
    unsigned int numVal ;
    register unsigned int Index ;
    unsigned int ByteDataSize ;

    ByteDataSize = strlen( Record->rep_type_name) + 1 ; /* For rep type name.*/

    valNames = Record->value_names ;
    numVal = Record->num_values ;
    Index = 0 ;
    while(    Index < numVal    )
      {   
	  ByteDataSize += strlen( valNames[Index++]) + 1 ; /* For value names.*/
      } 
    ByteDataSize += numVal ;	/* For array of values. */

    return( ByteDataSize) ;
} 

static void
#ifdef _NO_PROTO
CopyRecord( Record, OutputEntry, PtrDataArea, ByteDataArea)
        XmRepTypeEntry Record ;
        XmRepTypeEntry OutputEntry ;
        XtPointer *PtrDataArea ;
        XtPointer *ByteDataArea ;
#else
CopyRecord(
        XmRepTypeEntry Record,
        XmRepTypeEntry OutputEntry,
        XtPointer *PtrDataArea,
        XtPointer *ByteDataArea)
#endif /* _NO_PROTO */
{   
    register String *PtrDataAreaPtr = (String *) *PtrDataArea ;
    register char *ByteDataAreaPtr = (char *) *ByteDataArea ;
    unsigned int NumValues = Record->num_values ;
    register char *NamePtr ;
    unsigned int ValueIndex ;

    OutputEntry->num_values = NumValues ;
    OutputEntry->reverse_installed = Record->reverse_installed ;
    OutputEntry->rep_type_id = Record->rep_type_id ;

    OutputEntry->rep_type_name = ByteDataAreaPtr ;
    NamePtr = Record->rep_type_name ;
    while(    (*ByteDataAreaPtr++ = *NamePtr++) != '\0'    ) /*EMPTY*/;

    OutputEntry->value_names = PtrDataAreaPtr ;
    ValueIndex = 0 ;
    while(    ValueIndex < NumValues    )
      {   
	  *PtrDataAreaPtr++ = ByteDataAreaPtr ;
	  NamePtr = Record->value_names[ValueIndex] ;
	  while(    (*ByteDataAreaPtr++ = *NamePtr++) != '\0'    ) /*EMPTY*/;
	  ++ValueIndex ;
      } 
    ValueIndex = 0 ;
    OutputEntry->values = (unsigned char *) ByteDataAreaPtr ;

    if(    XmREP_TYPE_MAPPED( Record->rep_type_id)    )
      {   
	  while(    ValueIndex < NumValues    )
	    {   *((unsigned char *) ByteDataAreaPtr++) =
		  Record->values[ValueIndex++] ;
            } 
      } 
    else
      {   while(    ValueIndex < NumValues    )
	    {   *((unsigned char *) ByteDataAreaPtr++) = (unsigned char)
		  ValueIndex++ ;
	      } 
        } 
    *PtrDataArea = (XtPointer) PtrDataAreaPtr ;
    *ByteDataArea = (XtPointer) ByteDataAreaPtr ;
    return ;
} 

XmRepTypeList
#ifdef _NO_PROTO
XmRepTypeGetRegistered()
#else
XmRepTypeGetRegistered( void )
#endif /* _NO_PROTO */
{
    /* In order allow the user to free allocated memory with a simple
     *    call to XtFree, the data from the record lists are copied into
     *    a single block of memory.  This can create alignment issues
     *    on some architectures.
     *  To resolve alignment issues, the data is layed-out in the memory
     *    block as shown below.
     *
     *   returned    ____________________________________________________
     *   pointer -> |                                                    |
     *              |   Array of XmRepTypeEntryRec structures            |
     *              |____________________________________________________|
     *              |                                                    |
     *              |   Pointer-size data (arrays of pointers to strings)|
     *              |____________________________________________________|
     *              |                                                    |
     *              |   Byte-size data (arrays of characters and values) |
     *              |____________________________________________________|
     *
     *  The XmRepTypeGetRegistered routine fills the fields of the
     *    XmRepTypeEntryRec with immediate values and with pointers
     *    to the appropriate arrays.  The entry->values field is set
     *    to point to an array of values in the byte-size data area,
     *    while the entry->value_names field is set to point to an
     *    array of pointers in the pointer-size data area.  This array
     *    of pointers is then set to point to character arrays in the
     *    byte-size data area.
     *  Since the first field of the XmRepTypeEntryRec is a pointer,
     *    it can be assumed that the section of arrays of pointers
     *    can be located immediately following the array of structures
     *    without concern for alignment.  Byte-size data is assumed
     *    to have no alignment requirements.
     */

    unsigned int TotalEntries ;

    unsigned int PtrDataSize = 0 ;
    unsigned int ByteDataSize = 0 ;
    XmRepTypeList OutputList ;
    XmRepTypeList ListPtr ;
    XtPointer PtrDataPtr ;
    XtPointer ByteDataPtr ;
    unsigned int Index ;

    /* Total up the data sizes of the static and run-time lists. */

    TotalEntries = XtNumber( _XmStandardRepTypes ) +
      XtNumber( _XmStandardMappedRepTypes ) + 
	_XmRepTypeNumRecords +
	  1 ;			/* One extra for null terminator.*/

    for ( Index = 0; Index < XtNumber( _XmStandardRepTypes); Index++ )
      {
	  PtrDataSize += 
	    _XmStandardRepTypes[Index].num_values * sizeof( String) ;
	  ByteDataSize += GetByteDataSize( &_XmStandardRepTypes[Index]) ;
      }
    for ( Index = 0; Index < XtNumber( _XmStandardMappedRepTypes); Index++ )
      {
	  PtrDataSize += 
	    _XmStandardMappedRepTypes[Index].num_values * sizeof( String) ;
	  ByteDataSize += GetByteDataSize( &_XmStandardMappedRepTypes[Index]) ;
      }
    for ( Index = 0; Index < _XmRepTypeNumRecords ; Index++ )
      {
	  PtrDataSize += _XmRepTypes[Index].num_values * sizeof( String) ;
	  ByteDataSize += GetByteDataSize( &_XmRepTypes[Index]) ;
      }

    OutputList = (XmRepTypeList) 
      XtMalloc( PtrDataSize + ByteDataSize +
	       (TotalEntries * sizeof( XmRepTypeListRec))) ;
    ListPtr = OutputList ;
    PtrDataPtr = (XtPointer) (ListPtr + TotalEntries) ;
    ByteDataPtr = (XtPointer) (((char *) PtrDataPtr) + PtrDataSize) ;

    for ( Index = 0; Index < XtNumber( _XmStandardRepTypes); Index++ )
      {
	  CopyRecord( &_XmStandardRepTypes[Index], ListPtr, &PtrDataPtr, 
		     &ByteDataPtr );
	  ++ListPtr ;
      }
    for ( Index = 0; Index < XtNumber( _XmStandardMappedRepTypes); Index++ )
      {
	  CopyRecord( &_XmStandardMappedRepTypes[Index], ListPtr, &PtrDataPtr, 
		     &ByteDataPtr );
	  ++ListPtr ;
      }
    for ( Index = 0; Index < _XmRepTypeNumRecords ; Index++ )
      {
	  CopyRecord( &_XmRepTypes[Index], ListPtr, &PtrDataPtr, 
		     &ByteDataPtr );
	  ++ListPtr ;
      }

    ListPtr->rep_type_name = NULL ;

    return( OutputList) ;
}

XmRepTypeEntry
#ifdef _NO_PROTO
XmRepTypeGetRecord( rep_type_id )
        XmRepTypeId rep_type_id ;
#else
XmRepTypeGetRecord(
#if NeedWidePrototypes
        int rep_type_id)
#else
        XmRepTypeId rep_type_id)
#endif
#endif /* _NO_PROTO */
{
    XmRepTypeEntry Record = GetRepTypeRecord( rep_type_id) ;
    XmRepTypeEntry OutputRecord ;
    XtPointer PtrDataArea ;
    XtPointer ByteDataArea ;
    unsigned int PtrDataSize ;
    unsigned int ByteDataSize ;

    if(    Record    )
      {   
	  PtrDataSize = Record->num_values * sizeof( String) ;
	  ByteDataSize = GetByteDataSize( Record) ;

	  OutputRecord = (XmRepTypeEntry) XtMalloc( sizeof( XmRepTypeEntryRec)
						   + PtrDataSize + ByteDataSize) ;
	  PtrDataArea = (XtPointer) (OutputRecord + 1) ;
	  ByteDataArea = (XtPointer) (((char *) PtrDataArea) + PtrDataSize) ;

	  CopyRecord( Record, OutputRecord, &PtrDataArea, &ByteDataArea) ;

	  return( OutputRecord) ;
      } 
    return( NULL) ;
}

XmRepTypeId
#ifdef _NO_PROTO
GetIdFromSortedList( rep_type, List, ListSize )
     String rep_type;
     XmRepTypeList List;
     unsigned short ListSize;
#else
GetIdFromSortedList( 
    String rep_type,
    XmRepTypeList List,
    unsigned short ListSize )
#endif
{   
    int Index ;
    int Lower = 0 ;
    int Upper = ListSize - 1;
    int TestResult ;

    while(    Upper >= Lower    )
      {   
	  Index = ((Upper - Lower) >> 1) + Lower ;
	  TestResult = strcmp( rep_type, List[Index].rep_type_name) ;
	  if(    TestResult > 0    )
	    {
		Lower = Index + 1 ;
	    } 
	  else
	    {
		if(    TestResult < 0    )
		  {
		      Upper = Index - 1 ;
		  } 
		else
		  {
		      return List[Index].rep_type_id ;
		  }
	    }
      }

    return XmREP_TYPE_INVALID ;
}

XmRepTypeId
#ifdef _NO_PROTO
XmRepTypeGetId( rep_type)
        String rep_type ;
#else
XmRepTypeGetId(
        String rep_type)
#endif /* _NO_PROTO */
{
    XmRepTypeId rep_type_id;
    int Index ;

    /* First look in the statically defined lists */
    
    if ( ( rep_type_id = 
	  GetIdFromSortedList( rep_type, _XmStandardRepTypes, 
			      XtNumber( _XmStandardRepTypes )) ) != 
	XmREP_TYPE_INVALID )
      return rep_type_id ;

    if( ( rep_type_id = 
	 GetIdFromSortedList( rep_type, _XmStandardMappedRepTypes,
			     XtNumber( _XmStandardMappedRepTypes )) ) !=
       XmREP_TYPE_INVALID )
      return rep_type_id ;


    /* Not in the static lists; look in the run-time list. */

    for ( Index = 0; Index < _XmRepTypeNumRecords; Index++ )
      {
	  if( !strcmp( rep_type, _XmRepTypes[Index].rep_type_name ))
	    return _XmRepTypes[Index].rep_type_id ;
      }

    return( XmREP_TYPE_INVALID) ;
}

String *
#ifdef _NO_PROTO
XmRepTypeGetNameList( rep_type_id, use_uppercase_format )
        XmRepTypeId rep_type_id ;
        Boolean use_uppercase_format ;
#else
XmRepTypeGetNameList(
#if NeedWidePrototypes
        int rep_type_id,
        int use_uppercase_format)
#else
        XmRepTypeId rep_type_id,
        Boolean use_uppercase_format)
#endif /* NeedWidePrototypes */
#endif
{
    XmRepTypeEntry Record = GetRepTypeRecord( rep_type_id) ;

    if(    Record    )
      {
	  return( CopyStringArray( Record->value_names, Record->num_values,
				  TRUE, use_uppercase_format)) ;
      } 
    return( NULL) ;
}

/*ARGSUSED*/
static Boolean
#ifdef _NO_PROTO
ConvertRepType( disp, args, n_args, from, to, converter_data )
        Display *disp ;
        XrmValue *args ;
        Cardinal *n_args ;
        XrmValue *from ;
        XrmValue *to ;
        XtPointer *converter_data ;
#else
ConvertRepType(
        Display *disp,
        XrmValue *args,
        Cardinal *n_args,
        XrmValue *from,
        XrmValue *to,
        XtPointer *converter_data)
#endif /* _NO_PROTO */
{   
    char *in_str = (char *) (from->addr) ;
    XtPointer argvalue = *((XtPointer*)args[0].addr);
    XmRepTypeId RepTypeID = (XmRepTypeId)(long)argvalue;
    XmRepTypeEntry Record = GetRepTypeRecord( RepTypeID) ;

    unsigned int NumValues = Record->num_values ;
    unsigned int Index = 0 ;
    static unsigned char conversion_buffer ;
    static unsigned int EditModeBuffer ;
    static int WhichButtonBuffer ;
    char first, second;

    /*
     * Fix for 5330 - To ensure OS compatibility, always check a character with
     *                isupper before converting it with tolower.
     */
    if (isupper((unsigned char)in_str[0]))
      first = tolower((unsigned char)in_str[0]);
    else
      first = in_str[0];
    if (isupper((unsigned char)in_str[1]))
      second = tolower((unsigned char)in_str[1]);
    else
      second = in_str[1];
 
    if(    (first == 'x')  &&  (second == 'm')    )
      {   in_str += 2 ;
      } 
    while(    Index < NumValues    )
      {   
	  if(    _XmStringsAreEqual( in_str, Record->value_names[Index])    )
	    {   
		unsigned char OutValue = XmREP_TYPE_MAPPED( RepTypeID)
		  ? (Record->values[Index])
		    : ((unsigned char) Index) ;
		if(    (RepTypeID == XmRID_EDIT_MODE)
		   || (RepTypeID == XmRID_WHICH_BUTTON)    )
		  {   
		      /* Assuming sizeof( unsigned int) == sizeof( int) */
		      if(    to->addr    )
			{   
			    if(    to->size < sizeof( unsigned int)    )
			      {   
				  /* Insufficient space, so set needed size and return.*/
				  to->size = sizeof( unsigned int) ;
				  return( FALSE) ;
			      }
			}
		      else
			{   to->addr = (RepTypeID == XmRID_EDIT_MODE)
			      ? ((XPointer) &EditModeBuffer)
				: ((XPointer) &WhichButtonBuffer) ;
			  } 
		      to->size = sizeof( unsigned int);
		      *((unsigned int *) to->addr) = (unsigned int) OutValue ;
		  }
		else
		  {
		      if(    to->addr    )
			{   
			    if(    to->size < sizeof( unsigned char)    )
			      {   
				  /* Insufficient space, so set needed size and return.*/
				  to->size = sizeof( unsigned char) ;
				  return( FALSE) ;
			      }
			}
		      else
			{
			    to->addr = (XPointer) &conversion_buffer ;
			}
		      to->size = sizeof( unsigned char);
		      *((unsigned char *) to->addr) = OutValue ;
		  }
		return( TRUE) ;
            }
	  ++Index ;
      } 
    XtDisplayStringConversionWarning( disp, in_str, Record->rep_type_name) ;
    
    return( FALSE) ;
}

/*ARGSUSED*/
static Boolean
#ifdef _NO_PROTO
ReverseConvertRepType( disp, args, n_args, from, to, converter_data )
        Display *disp ;
        XrmValue *args ;
        Cardinal *n_args ;
        XrmValue *from ;
        XrmValue *to ;
        XtPointer *converter_data ;
#else
ReverseConvertRepType(
        Display *disp,
        XrmValue *args,
        Cardinal *n_args,
        XrmValue *from,
        XrmValue *to,
        XtPointer *converter_data)
#endif /* _NO_PROTO */
{   
    XtPointer argvalue = *(XtPointer *)args[0].addr;
    XmRepTypeId RepTypeID = (XmRepTypeId)(long)argvalue;
    XmRepTypeEntry Record = GetRepTypeRecord( RepTypeID) ;
    unsigned char in_value = ((RepTypeID == XmRID_EDIT_MODE)
			      || (RepTypeID == XmRID_WHICH_BUTTON))
      ? ((unsigned char) *((unsigned int *) (from->addr)))
	: *((unsigned char *) (from->addr)) ;
    unsigned short NumValues = Record->num_values ;
    char **OutValue = NULL ;
    char *params[2];
    Cardinal num_params = 2;


    if(    XmREP_TYPE_MAPPED( RepTypeID)    )
      {   
	  unsigned short Index = 0 ;

	  while(    Index < NumValues    )
	    {   
		if(    in_value == Record->values[Index]    )
		  {   
		      OutValue = (char **) &Record->value_names[Index] ;
		      break ;
		  }
		++Index ;
            } 
      } 
    else
      {
	  if(    in_value < NumValues    )
	    {   
		OutValue = (char **) &Record->value_names[in_value] ;
            } 
      } 
    if(    OutValue    )
      {  
	  if(    to->addr    )
	    {   
		if(    to->size < sizeof( char *)    )
		  {   
		      to->size = sizeof( char *) ;
		      return( FALSE) ;
		  } 
		*((char **) to->addr) = *OutValue ;
            }
	  else
	    {
		to->addr = (XPointer) OutValue ;
            } 
	  to->size = sizeof( char *) ;

	  return( TRUE) ;
      } 
    params[0] = Record->rep_type_name;
    params[1] = (char *) ((long) in_value);
    XtAppWarningMsg( XtDisplayToApplicationContext( disp), "conversionError",
		    Record->rep_type_name, "XtToolkitError", 
		    "Cannot convert %s value %d to type String",
		    params, &num_params);
    return( FALSE) ;
}

void
#ifdef _NO_PROTO
_XmRepTypeInstallConverters()
#else
_XmRepTypeInstallConverters( void )
#endif /* _NO_PROTO */
{   
    unsigned short Index;

    /* Install the static consecutive-valued converters. */
    for ( Index = 0; Index < XtNumber( _XmStandardRepTypes ); Index ++ )
      {
#if 1
	  /* Maybe in Motif 1.3, this line will be removed so as to
	   * always install the tear-off model converter.
	   */
	  if( _XmStandardRepTypes[Index].rep_type_id != XmRID_TEAR_OFF_MODEL )
#endif
	    {   
		XtConvertArgRec convertArg;
		convertArg.address_mode = XtImmediate;
		convertArg.address_id   = 
		  (XPointer)(long)_XmStandardRepTypes[Index].rep_type_id;
		convertArg.size         = sizeof(convertArg.address_id);

		XtSetTypeConverter( XmRString, _XmStandardRepTypes[Index].rep_type_name,
				   ConvertRepType, &convertArg, 1,
				   XtCacheNone, NULL) ;
	    } 
      } 
    /* Install the static mapped-value converters. */
    for ( Index = 0; Index < XtNumber( _XmStandardMappedRepTypes ); Index ++ )
      {
	  XtConvertArgRec convertArg;
	  convertArg.address_mode = XtImmediate;
	  convertArg.address_id   = 
	    (XPointer)(long)_XmStandardMappedRepTypes[Index].rep_type_id;
	  convertArg.size         = sizeof(convertArg.address_id);

	  XtSetTypeConverter( XmRString, _XmStandardMappedRepTypes[Index].rep_type_name,
			     ConvertRepType, &convertArg, 1,
			     XtCacheNone, NULL) ;

      }
    return ;
}

void
#ifdef _NO_PROTO
XmRepTypeInstallTearOffModelConverter()
#else
XmRepTypeInstallTearOffModelConverter( void )
#endif /* _NO_PROTO */
{
  /* XmRepTypeInstallTearOffModelConverter convenience function.
   * Provide a way for the application to easily dynamically install the
   * TearOffModel converter.
   */
  XtConvertArgRec convertArg;
  convertArg.address_mode = XtImmediate;
  convertArg.address_id   = 
    (XPointer)(long)(GetRepTypeRecord(XmRID_TEAR_OFF_MODEL)->rep_type_id);
  convertArg.size         = sizeof(convertArg.address_id);
  XtSetTypeConverter( XmRString, XmRTearOffModel, ConvertRepType,
		     &convertArg, 1,
                      XtCacheNone, NULL) ;
}
