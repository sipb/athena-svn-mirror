/*
  Copyright (C) 1991,1992 Adobe Systems Incorporated.
  All rights reserved
  RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/printpanel/PrintPanel.c,v 1.1.1.1 1996-10-07 20:25:54 ghudson Exp $
*/
/* RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 4.5  1993/12/01  21:05:18  snichols
 * pass feature requests like PageSize to enscript.
 *
 * Revision 4.4  1993/08/25  22:12:36  snichols
 * added ScrolledWindow widget to accomodate printers with lots of features.
 *
 * Revision 4.3  1993/08/23  22:59:07  snichols
 * fixed problems with Motif 1.2, and a problem with an uninitialized variable.
 *
 * Revision 4.2  1993/08/19  17:25:58  snichols
 * updated for Motif 1.2
 *
 * Revision 4.1  1992/12/15  17:36:24  snichols
 * handle constraints where all options are constrained.
 *
 * Revision 4.0  1992/08/21  16:24:13  snichols
 * Release 4.0
 *
 * Revision 1.18  1992/08/19  23:45:38  snichols
 * counter incremented one too far in BuildFeatures.
 *
 * Revision 1.17  1992/08/19  21:55:54  snichols
 * In InitList, if no item matches, shouldn't select any.
 *
 * Revision 1.16  1992/07/29  21:39:44  snichols
 * cleaned up several small bugs: dialog title on message box, size of file
 * selection boxes, don't unmanage null child when deselecting NONE filter,
 * and handle transition on printer features boxes better.
 *
 * Revision 1.15  1992/07/14  23:12:56  snichols
 * Updated copyright.
 *
 * Revision 1.14  1992/07/14  23:09:47  snichols
 * Added RCS header.
 *
 * Revision 1.13  1992/07/10  21:44:51  snichols
 * support for psdraft, plus graying out overtranslate and shrink to fit
 * unless landscape is selected.
 *
 * Revision 1.12  1992/06/23  17:13:20  snichols
 * fixed a couple of bugs with default choices on printer feature panel, with
 * graying things out, and added support for using PPD to determine fax
 * availability.
 *
 * Revision 1.11  1992/06/01  23:57:28  snichols
 * screwed up RCSID.
 *
 * Revision 1.10  1992/06/01  23:05:46  snichols
 * more support for command-line options, support for specifying
 * previewer in a file, and more SYSV support.
 *
 *
 */

#include <stdio.h>
#include <ctype.h>
#ifdef XPG3
#include <stdlib.h>
#endif
#include <string.h>
#include <sys/wait.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#if XmVersion > 1001
#include <Xm/ManagerP.h>
#else
#include <Xm/XmP.h>
#endif
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include <Xm/FileSB.h>
#include <Xm/Separator.h>
#include <Xm/MessageB.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include <Xm/DialogS.h>

#include "config.h"
#include "transcript.h"

#include "PrintPaneP.h"

#define CS(str) XmStringCreate(str, XmSTRING_DEFAULT_CHARSET)
static XmString CSempty;


/* need to figure out resources */
#define Offset(field) XtOffsetOf(PrintPanelRec, printpanel.field)

static XtResource resources[] = {
    {XtNinputFileName, XtCString, XtRString, sizeof(String), 
	 Offset(in_file_name), XtRString, (XtPointer) NULL}, 
    {XtNfilterName, XtCString, XtRString, sizeof(String), 
	 Offset(filter_name), XtRString, (XtPointer) NULL}, 
    {XtNprinterName, XtCString, XtRString, sizeof(String), 
	 Offset(printer_name), XtRString, (XtPointer) NULL}, 
    {XtNokCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	 Offset(ok_callback), XtRCallback, (XtPointer) NULL},
    {XtNcancelCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	 Offset(cancel_callback), XtRCallback, (XtPointer) NULL}
};

/* forward decls */

static void Initialize(), Destroy(), ChangeManaged(), Resize();
static Boolean SetValues();
static XtGeometryResult GeometryManager();


PrintPanelClassRec printPanelClassRec = {
    /* core class part */
    {
	(WidgetClass) &xmManagerClassRec, /* superclass */
	"PrintPanel",		/* class_name */
	sizeof(PrintPanelRec),	/* widget_size */
	NULL,			/* class_initialize */
	NULL,			/* class_part_initialize */
	FALSE,			/* class_inited */
	Initialize,		/* initialize */
	NULL,			/* initialize_hook */
	XtInheritRealize,	/* realize */
	NULL,			/* actions */
	0,			/* num_actions */
	resources,		/* resources */
	XtNumber(resources),	/* num_resources */
	NULLQUARK,		/* xrm_class */
	TRUE,			/* compress_motion */
	XtExposeCompressMultiple, /* compress_exposure */
	TRUE,			/* compress_enterleave */
	FALSE,			/* visible_interest */
	Destroy,		/* destroy */
	Resize,			/* resize */
	NULL,			/* expose */
	NULL,			/* set_values */
	NULL,			/* set_values_hook */
	XtInheritSetValuesAlmost, /* set_values_almost */
	NULL,			/* get_values_hook */
	NULL,			/* accept_focus */
	XtVersion,		/* version */
	NULL,			/* callback_offsets */
	NULL,			/* tm_table */
	XtInheritQueryGeometry,	/* query_geometry */
	NULL,			/* display_accelerator */
	NULL,			/* extension */
    },
    /* Composite class part */
    {
	GeometryManager,	/* geometry_manager */
	ChangeManaged,		/* change_managed */
	XtInheritInsertChild,	/* insert_child */
	XtInheritDeleteChild,	/* delete_child */
	NULL,			/* extension */
    },
    {
    /* Constraint class part */
	NULL,			/* resources */
	0,			/* num_resources */
	0,			/* constraint_size */
	NULL,			/* initialize */
	NULL,			/* destroy */
	NULL,			/* set_values */
	NULL,			/* extension */
    },
    /* Manager class part */
    {
	XtInheritTranslations,	/* translations */
	NULL,			/* syn_resources */
	0,			/* num_syn_resources */
	NULL,			/* syn_constraint_resources */
	0,			/* num_syn_constraint_resources */
	XmInheritParentProcess, /* parent_process */
	NULL,			/* extension */
    },
    /* PrintPanel class part */
    {
	NULL,			/* extension */
    }
};

WidgetClass printPanelWidgetClass = (WidgetClass) &printPanelClassRec;


/* forward dec'ls */
static void okCallback();
static void applyCallback();
static void resetCallback();
static void cancelCallback();
static void RadioCallback();
static void PrinterStatusCallback();
static void SecondaryCallback();
static void ListSelection();
static void RangeCallback();
static void IntTextFieldChange();
static void FilterFieldChange();
static void ChooseFileCallback();
static void CancelPopup();
static void ReadErrors();
static void CreateFSB();
static void CreateFeaturePanel();
static void CreateFaxPanel();
static void CleanupOldFeatures();
static void CleanupPhoneEntry();
static void CreateSpecialFeatures();
static void CreateDraftPanel();
static void BuildFeatures();
static void BuildPhoneEntry();
static void CreateGenericPanel();
static void CreatePtroffPanel();
static void CreatePsroffPanel();
static void CreatePlotPanel();
static void Create630Panel();
static void Create4014Panel();
static void CreateEnscriptPanel();
static void AddToCompoundList();
static int addarg();
static int GetPrinterList();
static int GetSysVPrinterList();
extern int GetPhonebookKeys();
static void SetFax();
static void StringTextFieldChange();

typedef struct _FilterData {
    char *name;
    int can_spool;
    int can_write;
    char *spool_flag;
    char *stdout_flag;
    char *file_flag;
} FilterData;

FilterData filter_array[] = {
    { "enscript", TRUE, TRUE, "-P", "-p-", "-p" },
    { "psroff", TRUE, FALSE, "-P", "-t", NULL },
    { "ptroff", TRUE, FALSE, "-P", "-t", NULL },
    { "ps630", FALSE, TRUE, NULL, NULL, "-p" },
    { "ps4014", FALSE, TRUE, NULL, NULL, "-p" },
    { "psplot", FALSE, FALSE, NULL, NULL, NULL },
    { "none", FALSE, FALSE, NULL, NULL, NULL }
};

static int num_filter_array = 7;

static char *truevalue = "True";
#ifdef BSD
static char *lprname = "lpr";
static char *printerstring = "-P";
#endif
#ifdef SYSV
static char *lprname = "lp";
static char *printerstring = "-d";
#endif



static void Destroy(widget)
    Widget widget;
{
    PrintPanelWidget p = (PrintPanelWidget) widget;

    /* this is just a place holder for the time being.  When I fix up the
       rest of the code so that there is alloc'ed storage in some places
       where there are currently arrays, I'll free them here */
}

static void Resize(widget)
    Widget widget;
{
    PrintPanelWidget p = (PrintPanelWidget) widget;

    XtResizeWidget(p->printpanel.panel_child, p->core.width,
		   p->core.height, 0);
}

static XtGeometryResult GeometryManager(w, desired, allowed)
    Widget w;
    XtWidgetGeometry *desired, *allowed;
{

#define WANTS(flag) (desired->request_mode & flag)

    if (WANTS(XtCWQueryOnly)) return XtGeometryYes;

    if (WANTS(CWWidth)) w->core.width = desired->width;
    if (WANTS(CWHeight)) w->core.height = desired->height;
    if (WANTS(CWX)) w->core.x = desired->x;
    if (WANTS(CWY)) w->core.y = desired->y;
    if (WANTS(CWBorderWidth)) {
	w->core.border_width = desired->border_width;
    }

    return XtGeometryYes;
#undef WANTS
}

static void ChangeManaged(w)
    Widget w;
{
    PrintPanelWidget p = (PrintPanelWidget) w;

    w->core.width = p->composite.children[0]->core.width;
    w->core.height = p->composite.children[0]->core.height;
}

int GetFilter(p, str)
    PrintPanelWidget p;
    char *str;
{
    int i;

    for (i = 0; i < p->printpanel.num_filters; i++) {
	if (strcmp(str, p->printpanel.filterlist[i].name) == 0)
	    return i;
    }
    return -1;
}

static void InitList(num, namelist, listchild, select)
    int num;
    char *namelist[];
    Widget listchild;
    char *select;
{
    int i;
    int sel = -1;
    XmString *names;

    names = (XmString *) XtCalloc(num, sizeof(XmString));

    for (i = 0; i < num; i++) {
	names[i] = CS(namelist[i]);
	if (strcmp(namelist[i], select) == 0)
	    sel = i;
    }

    XtVaSetValues(listchild, XmNitemCount, num, XmNitems, names, NULL);

    if (sel != -1) {
	XmListSelectItem(listchild, names[sel], TRUE);
    }
    XtFree(names);
}

void Cleanup(p, filt)
    PrintPanelWidget p;
    int filt;
{
    XtVaSetValues(p->printpanel.generic_label_child, XmNbottomWidget,
		p->printpanel.generic_separator_child, NULL);
    if (filt != NONE)
	XtUnmanageChildren(p->printpanel.option_children[filt].child,
			   p->printpanel.option_children[filt].num_children); 
}

static void ReadyOptions(p, filt, width, height, attach, labelstring)
    PrintPanelWidget p;
    int filt;
    int width, height;
    int attach;
    char *labelstring;
{
    p->printpanel.option_callback_data.which_widget =
	p->printpanel.generic_panel_child; 
    p->printpanel.option_callback_data.which_filter = filt;

    /* I don't have the slightest clue why you have to unmanage the label
       child, manage the rest of the children, change the constraint on the
       label child, then manage it, but this works */

    XtUnmanageChild(p->printpanel.generic_label_child);
    if (filt == PSROFF)
	XtManageChildren(p->printpanel.option_children[PTROFF].child,
			 p->printpanel.option_children[PTROFF].num_children); 
    XtManageChildren(p->printpanel.option_children[filt].child,
		     p->printpanel.option_children[filt].num_children); 

    XtVaSetValues(p->printpanel.generic_panel_child, XmNwidth, width,
		  XmNheight, height, NULL); 

    XtVaSetValues(p->printpanel.generic_label_child, XmNlabelString,
		  CS(labelstring), XmNbottomWidget,
		  p->printpanel.option_children[filt].child[attach], NULL);
    XtManageChild(p->printpanel.generic_label_child);
}


void SetupFilter(p, fil)
    PrintPanelWidget p;
    int fil;
{
    switch (fil) {
    case ENSCRIPT:
	ReadyOptions(p, fil, 500, 370, ROTATE, "Enscript Options");
	break;
    case PSROFF:
	ReadyOptions(p, fil, 300, 230, DIRFIELD, "Psroff Options");
	break;
    case PTROFF:
	ReadyOptions(p, fil, 300, 200, OPTFIELD, "Ptroff Options");
	break;
    case PSPLT:
	ReadyOptions(p, fil, 350, 150, PROBUTTON, "Psplot Options");
	break;
    case PS630:
	ReadyOptions(p, fil, 420, 187, BODYFIELD, "Ps630 Options");
	break;
    case PS4014:
	ReadyOptions(p, fil, 450, 280, NOLFTOG, "Ps4014 Options");
	break;
    default:
	break;
    }
}

static void CreateStandardButtons(form, ok, apply, reset, cancel, sep, name)
    Widget form;
    Widget *ok;
    Widget *apply;
    Widget *reset;
    Widget *cancel;
    Widget *sep;
    char *name;
{
    char tmpname[100];
    strcpy(tmpname, name);
    strcat(tmpname, "OK");
    *ok = XtVaCreateManagedWidget(tmpname, xmPushButtonWidgetClass, form,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNbottomAttachment, XmATTACH_FORM,
				 XmNlabelString, CS("OK"), XmNwidth, 60,
				 XmNheight, 30, XmNbottomOffset, 10,
				 XmNleftOffset, 10, NULL);

    strcpy(tmpname, name);
    strcat(tmpname, "Apply");
    *apply = XtVaCreateManagedWidget(tmpname, xmPushButtonWidgetClass, form, 
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget, *ok,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNlabelString, CS("Apply"),
				    XmNwidth, 60, XmNheight, 30,
				    XmNbottomOffset, 10, XmNleftOffset, 10,
				    NULL);

    strcpy(tmpname, name);
    strcat(tmpname, "Reset");
    *reset = XtVaCreateManagedWidget(tmpname, xmPushButtonWidgetClass, form, 
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget, *apply,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNlabelString, CS("Reset"), XmNwidth,
				    60, XmNheight, 30, XmNbottomOffset, 10, 
				    XmNleftOffset, 10, NULL);

    strcpy(tmpname, name);
    strcat(tmpname, "Cancel");
    *cancel = XtVaCreateManagedWidget(tmpname, xmPushButtonWidgetClass,
				     form, XmNleftAttachment,
				     XmATTACH_WIDGET, XmNleftWidget, *reset, 
				     XmNbottomAttachment, XmATTACH_FORM, 
				     XmNlabelString, CS("Cancel"),
				     XmNwidth, 60, XmNheight, 30,
				     XmNbottomOffset, 10, XmNleftOffset,
				     10, NULL);
    strcpy(tmpname, name);
    strcat(tmpname, "Separator");
    *sep = XtVaCreateManagedWidget(tmpname, xmSeparatorWidgetClass,
					form, XmNleftAttachment,
					XmATTACH_FORM, XmNrightAttachment,
					XmATTACH_FORM, XmNbottomAttachment,
					XmATTACH_WIDGET, XmNbottomWidget,
					*ok, XmNbottomOffset, 10, NULL);
}

static void LayoutPanel(p)
    PrintPanelWidget p;
{
    Arg args[10];
    int i;
    char *filterlist[20];

    p->printpanel.panel_child =
	XtVaCreateManagedWidget("ppanel",
				xmFormWidgetClass, (Widget) p,
				XmNwidth, 650, XmNheight, 485,
				NULL);
/*    
				XmNx, 100, XmNy, 80, NULL);
*/				


    /* secondary panels */
    CreateSpecialFeatures(p);
    CreateDraftPanel(p);
    CreateFeaturePanel(p);
    CreateFaxPanel(p);
    CreateGenericPanel(p);
    CreateEnscriptPanel(p);
    CreatePtroffPanel(p);
    CreatePsroffPanel(p);
    CreatePlotPanel(p);
    Create630Panel(p);
    Create4014Panel(p);

    /* for now, just the built in filters; later, want to allow filter list
       to be specified in a resource */
    for (i = 0; i < num_filter_array; i++) {
	strcpy(p->printpanel.filterlist[i].name, filter_array[i].name);
	p->printpanel.filterlist[i].index = i;
	p->printpanel.num_filters++;
    }
    if (p->printpanel.default_filter_name) {
	p->printpanel.filter =
	    GetFilter(p, p->printpanel.default_filter_name); 
	SetupFilter(p, p->printpanel.filter);
    }


    /* primary panel */
    p->printpanel.ok_button_child =
	XtVaCreateManagedWidget("okButton",
				xmPushButtonWidgetClass,
				p->printpanel.panel_child,
				XmNleftAttachment,
				XmATTACH_FORM,
				XmNbottomAttachment,
				XmATTACH_FORM,
				XmNbottomOffset, 10,
				XmNleftOffset, 10,
				XmNwidth, 60,
				XmNheight, 30,
				XmNlabelString, CS("OK"), NULL); 
    XtAddCallback(p->printpanel.ok_button_child, XmNactivateCallback,
		  okCallback, p); 

    p->printpanel.apply_button_child =
	XtVaCreateManagedWidget("applyButton",
			      xmPushButtonWidgetClass,  
			      p->printpanel.panel_child,
			      XmNleftAttachment,
			      XmATTACH_WIDGET,
			      XmNleftWidget,
			      p->printpanel.ok_button_child,
			      XmNbottomAttachment, XmATTACH_FORM,
			      XmNbottomOffset, 10, XmNleftOffset, 10,
			      XmNlabelString, CS("Apply"), XmNwidth, 60,
			      XmNheight, 30, NULL);
    XtAddCallback(p->printpanel.apply_button_child, XmNactivateCallback,
		  applyCallback, p); 
    
    p->printpanel.reset_button_child =
	XtVaCreateManagedWidget("resetButton",
				xmPushButtonWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.apply_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNbottomOffset, 10, XmNleftOffset, 10,
				XmNlabelString, CS("Reset"), XmNwidth, 60,
				XmNheight, 30, NULL);
    XtAddCallback(p->printpanel.reset_button_child, XmNactivateCallback,
		  resetCallback, p);

    p->printpanel.cancel_button_child =
	XtVaCreateManagedWidget("cancelButton",
				xmPushButtonWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.reset_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNbottomOffset, 10, XmNleftOffset, 10,
				XmNlabelString, CS("Cancel"), XmNwidth, 60,
				XmNheight, 30, NULL);

    XtAddCallback(p->printpanel.cancel_button_child, XmNactivateCallback,
		  cancelCallback, p);

    p->printpanel.bottom_separator_child =
	XtVaCreateManagedWidget("separator",
				xmSeparatorWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.ok_button_child,
				XmNbottomOffset, 10, NULL);
    

    i = 0;
    XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(args[i], XmNbottomWidget,
	     p->printpanel.bottom_separator_child); i++;
    p->printpanel.radio_box_child =
	XmCreateRadioBox(p->printpanel.panel_child, "radio", args, i); 

    XtAddCallback(p->printpanel.radio_box_child, XmNentryCallback,
		  RadioCallback, p);

    XtManageChild(p->printpanel.radio_box_child);

    p->printpanel.print_toggle_child =
	XtVaCreateManagedWidget("print",
				xmToggleButtonWidgetClass, 
				p->printpanel.radio_box_child,
				XmNlabelString, CS("Print"), NULL);
    p->printpanel.save_toggle_child =
	XtVaCreateManagedWidget("save",
				xmToggleButtonWidgetClass, 
				p->printpanel.radio_box_child,
				XmNlabelString, CS("Save"), NULL);
    p->printpanel.preview_toggle_child =
	XtVaCreateManagedWidget("preview",
				xmToggleButtonWidgetClass,
				p->printpanel.radio_box_child,
				XmNlabelString, CS("Preview"), NULL);
    p->printpanel.fax_toggle_child =
	XtVaCreateManagedWidget("fax",
				xmToggleButtonWidgetClass, 
				p->printpanel.radio_box_child,
				XmNlabelString, CS("Fax"), NULL); 

    p->printpanel.open_button_child =
	XtVaCreateManagedWidget("openFile",
				xmPushButtonWidgetClass, 
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.radio_box_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Open File..."),
				XmNbottomOffset, 10, XmNleftOffset, 10,
				XmNheight, 30, NULL);

    p->printpanel.open_field_child =
	XtVaCreateManagedWidget("openField",
				xmTextFieldWidgetClass, 
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.radio_box_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.open_button_child,
				XmNwidth, 300, XmNleftOffset, 10,
				XmNbottomOffset, 10, NULL);

    XtAddCallback(p->printpanel.open_field_child, XmNvalueChangedCallback,
		  StringTextFieldChange, &p->printpanel.input_file);

    p->printpanel.status_button_child =
	XtVaCreateManagedWidget("statusButton",
				xmPushButtonWidgetClass,
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.open_button_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Queue Status..."),
				XmNbottomOffset, 10, XmNleftOffset, 10,
				XmNheight, 30, NULL);


    XtAddCallback(p->printpanel.status_button_child, XmNactivateCallback,
		  PrinterStatusCallback, NULL);  

    
    p->printpanel.printer_option_button_child =
	XtVaCreateManagedWidget("printerOptions", 
				xmPushButtonWidgetClass,
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.status_button_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Printer Features..."),
				XmNbottomOffset, 10, XmNleftOffset, 10,
				XmNheight, 30, NULL);

    XtAddCallback(p->printpanel.printer_option_button_child,
		  XmNactivateCallback, SecondaryCallback,
		  &p->printpanel.feature_panel_child); 

    p->printpanel.filter_option_button_child =
	XtVaCreateManagedWidget("filterOptions", 
				xmPushButtonWidgetClass,
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.status_button_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.printer_option_button_child,
				XmNlabelString, CS("Filter Options..."),
				XmNbottomOffset, 10, XmNleftOffset, 10,
				XmNheight, 30, NULL);

    XtAddCallback(p->printpanel.filter_option_button_child,
		  XmNactivateCallback, SecondaryCallback,
		  &p->printpanel.generic_panel_child); 

    p->printpanel.special_button_child =
	XtVaCreateManagedWidget("specialFilter", 
				xmPushButtonWidgetClass,
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.open_button_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.status_button_child,
				XmNlabelString, CS("Special Features..."),
				XmNbottomOffset, 10, XmNleftOffset, 10,
				XmNheight, 30, NULL);

    XtAddCallback(p->printpanel.special_button_child, XmNactivateCallback,
		  SecondaryCallback, &(p->printpanel.special_form_child));

    p->printpanel.top_separator_child =
	XtVaCreateManagedWidget("topSeparator",
				xmSeparatorWidgetClass, 
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.printer_option_button_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomOffset, 10, NULL);

    p->printpanel.printer_field_child =
	XtVaCreateManagedWidget("printerField",
				xmTextFieldWidgetClass, 
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.top_separator_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNwidth, 200, NULL);

    p->printpanel.fax_label_child =
	XtVaCreateManagedWidget("faxToLabel",
				xmLabelWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.radio_box_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.bottom_separator_child,
				XmNbottomOffset, 7,
				XmNlabelString, CS("to"), NULL);
				
    XtSetSensitive(p->printpanel.fax_label_child, FALSE);

    p->printpanel.fax_text_child =
	XtVaCreateManagedWidget("faxTextField",
				xmTextFieldWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_label_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.bottom_separator_child,
				XmNbottomOffset, 4, XmNwidth, 300, NULL);

    XtSetSensitive(p->printpanel.fax_text_child, FALSE);

    p->printpanel.fax_button_child =
	XtVaCreateManagedWidget("faxButton",
				xmPushButtonWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_text_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.bottom_separator_child,
				XmNlabelString, CS("..."),
				XmNbottomOffset, 5, XmNwidth, 40, NULL);

    XtSetSensitive(p->printpanel.fax_button_child, FALSE);
    XtAddCallback(p->printpanel.fax_button_child, XmNactivateCallback,
		  SecondaryCallback, &(p->printpanel.fax_panel_child));

    p->printpanel.save_label_child =
	XtVaCreateManagedWidget("saveAsLabel",
				xmLabelWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.radio_box_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_label_child,
				XmNbottomOffset, 40,
				XmNlabelString, CS("as"), NULL);

    XtSetSensitive(p->printpanel.save_label_child, FALSE);

    p->printpanel.save_text_child =
	XtVaCreateManagedWidget("saveTextField",
				xmTextFieldWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.save_label_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_text_child,
				XmNbottomOffset, 27, XmNwidth, 300, NULL); 

    XtSetSensitive(p->printpanel.save_text_child, FALSE);
    XtAddCallback(p->printpanel.save_text_child, XmNvalueChangedCallback,
		  StringTextFieldChange, &p->printpanel.output_file);

    p->printpanel.save_button_child =
	XtVaCreateManagedWidget("saveButton",
				xmPushButtonWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.save_text_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_button_child,
				XmNbottomOffset, 30, XmNwidth, 40,
				XmNlabelString, CS("..."), NULL);

    XtSetSensitive(p->printpanel.save_button_child, FALSE);

    p->printpanel.printer_label_child =
	XtVaCreateManagedWidget("printerLabel", 
				xmLabelWidgetClass,
				p->printpanel.panel_child,
				XmNtopAttachment, XmATTACH_FORM,
				XmNtopOffset, 5,
				XmNleftAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Select Printer"), NULL);

    p->printpanel.filter_label_child =
	XtVaCreateManagedWidget("filterLabel",
				xmLabelWidgetClass, 
				p->printpanel.panel_child,
				XmNtopAttachment, XmATTACH_FORM,
				XmNtopOffset, 5,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, 50,
				XmNlabelString, CS("Select Filter"), NULL); 

    i = 0;
    XtSetArg(args[i], XmNitemCount, 1); i++;
    XtSetArg(args[i], XmNitems, &CSempty); i++;
    p->printpanel.printer_list_child =
	XmCreateScrolledList(p->printpanel.panel_child,
			     "printerList", args, i);  
    p->printpanel.filter_list_child =
	XmCreateScrolledList(p->printpanel.panel_child,
			     "filterList", args, i);  

    XtVaSetValues(XtParent(p->printpanel.printer_list_child),
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, p->printpanel.printer_label_child,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, p->printpanel.printer_field_child,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, 35, NULL);

    XtManageChild(p->printpanel.printer_list_child);
    XtAddCallback(p->printpanel.printer_list_child, XmNbrowseSelectionCallback,
		  ListSelection, p);

    InitList(p->printpanel.num_printers, p->printpanel.printers,
	     p->printpanel.printer_list_child,
	     p->printpanel.current_printer_name); 

    p->printpanel.page_label_child =
	XtVaCreateManagedWidget("pageLabel",
				xmLabelWidgetClass, 
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.open_field_child,
				XmNbottomOffset, 50,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.open_field_child,
				XmNlabelString, CS("Pages:"), NULL);

    i = 0;
    XtSetArg(args[i], XmNleftAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(args[i], XmNleftWidget, p->printpanel.page_label_child); i++;
    XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(args[i], XmNbottomWidget, p->printpanel.open_field_child); i++;
    XtSetArg(args[i], XmNbottomOffset, 7); i++;
    p->printpanel.page_box_child =
	XmCreateRadioBox(p->printpanel.panel_child, "page", args, i); 
    XtAddCallback(p->printpanel.page_box_child, XmNentryCallback,
		  RangeCallback, p); 
    XtManageChild(p->printpanel.page_box_child);

    p->printpanel.all_toggle_child =
	XtVaCreateManagedWidget("all",
				xmToggleButtonWidgetClass, 
				p->printpanel.page_box_child,
				XmNlabelString, CS("All"), NULL);

    p->printpanel.range_toggle_child =
	XtVaCreateManagedWidget("range",
				xmToggleButtonWidgetClass, 
				p->printpanel.page_box_child,
				XmNlabelString, CS("from"), NULL);

    p->printpanel.begin_field_child =
	XtVaCreateManagedWidget("rangeBegin",
				xmTextFieldWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.page_box_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.open_field_child,
				XmNbottomOffset, 7, XmNwidth, 40, NULL);

    XtSetSensitive(p->printpanel.begin_field_child, FALSE);
    XtAddCallback(p->printpanel.begin_field_child, XmNvalueChangedCallback,
		  IntTextFieldChange, &(p->printpanel.page_ranges[0].begin)); 

    p->printpanel.range_label_child =
	XtVaCreateManagedWidget("toLabel",
				xmLabelWidgetClass,
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.begin_field_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.open_field_child,
				XmNbottomOffset, 10,
				XmNlabelString, CS("to"), NULL);
				
    p->printpanel.end_field_child =
	XtVaCreateManagedWidget("rangeEnd",
				xmTextFieldWidgetClass, 
				p->printpanel.panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.range_label_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.open_field_child,
				XmNbottomOffset, 7, XmNwidth, 40, NULL); 

    XtSetSensitive(p->printpanel.end_field_child, FALSE);
    XtAddCallback(p->printpanel.end_field_child, XmNvalueChangedCallback,
		  IntTextFieldChange, &(p->printpanel.page_ranges[0].end)); 

    p->printpanel.copy_label_child =
	XtVaCreateManagedWidget("copyLabel",
				xmLabelWidgetClass, 
				p->printpanel.panel_child,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, p->printpanel.page_box_child,
				XmNtopOffset, 20,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.save_button_child,
				XmNleftOffset, 10,
				XmNlabelString, CS("Copies:"), NULL);

    p->printpanel.copy_field_child =
	XtVaCreateManagedWidget("copyField",
				xmTextFieldWidgetClass, 
				p->printpanel.panel_child,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, p->printpanel.page_box_child,
				XmNtopOffset, 15,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.copy_label_child,
				XmNvalue, "1", XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.copy_field_child, XmNvalueChangedCallback,
		  IntTextFieldChange, &p->printpanel.copies);

    p->printpanel.filter_field_child =
	XtVaCreateManagedWidget("filterField",
				xmTextFieldWidgetClass, 
				p->printpanel.panel_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.top_separator_child,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, 50, XmNwidth, 200, NULL);

    XtAddCallback(p->printpanel.filter_field_child, XmNvalueChangedCallback,
		  FilterFieldChange, p); 

    XtVaSetValues(XtParent(p->printpanel.filter_list_child),
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, p->printpanel.filter_label_child,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 50,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, p->printpanel.filter_field_child,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, 85, NULL);

    XtManageChild(p->printpanel.filter_list_child);
    XtAddCallback(p->printpanel.filter_list_child, XmNbrowseSelectionCallback,
		  ListSelection, p);

    for (i = 0; i < p->printpanel.num_filters; i++)
	filterlist[i] = p->printpanel.filterlist[i].name;
    InitList(p->printpanel.num_filters, filterlist,
	     p->printpanel.filter_list_child,
	     p->printpanel.filterlist[p->printpanel.filter].name); 

    XmToggleButtonSetState(p->printpanel.print_toggle_child, TRUE, TRUE);
    XmToggleButtonSetState(p->printpanel.all_toggle_child, TRUE, TRUE);

    CreateFSB(p);
    XtAddCallback(p->printpanel.open_button_child, XmNactivateCallback,
		  ChooseFileCallback,
		  p->printpanel.input_file_selection_child);  
    XtAddCallback(p->printpanel.save_button_child, XmNactivateCallback,
		  ChooseFileCallback,
		  p->printpanel.output_file_selection_child); 

    i = 0;
    XtSetArg(args[i], XmNdialogTitle, CS("Message Box")); i++;
    p->printpanel.message_dialog_child =
	XmCreateMessageDialog(p->printpanel.panel_child, "Message", args,
			      i);  
    XtAddCallback(p->printpanel.message_dialog_child, XmNokCallback,
		  CancelPopup, NULL);

    XtUnmanageChild(XmMessageBoxGetChild(p->printpanel.message_dialog_child,
					 XmDIALOG_CANCEL_BUTTON)); 
    XtUnmanageChild(XmMessageBoxGetChild(p->printpanel.message_dialog_child,
					 XmDIALOG_HELP_BUTTON)); 
}

static void Initialize(request, new, args, num_args)
    Widget request, new;
    ArgList args;
    Cardinal *num_args;
{
    int i;
    char *tmp;
    FILE *fp;
    char fname[1024];
    int gray = FALSE;

    PrintPanelWidget p = (PrintPanelWidget) new;

    p->printpanel.save_fd = -1;
    CSempty = CS("");
    p->printpanel.builtup_line[0] = '\0';
    p->printpanel.use_pslpr = FALSE;
    p->printpanel.use_psnup = FALSE;
    p->printpanel.copies = 1;
    p->printpanel.filter = NONE;
    p->printpanel.nup_info.rows = p->printpanel.nup_info.columns =
	p->printpanel.tmp_nup_info.rows =
	    p->printpanel.tmp_nup_info.columns = 2;
    p->printpanel.nup_info.gaudy = p->printpanel.nup_info.rotated = FALSE;
    p->printpanel.tmp_nup_info.gaudy =
	p->printpanel.tmp_nup_info.rotated = FALSE;
    for (i = 0; i < 10; i++) {
	p->printpanel.page_ranges[i].begin = -1;
	p->printpanel.page_ranges[i].end = -1;
    }
#ifdef BSD    
    if ((i = GetPrinterList(p->printpanel.printers,
			    &p->printpanel.num_printers, MAXPRINTERS,
			    "/etc/printcap")) != 0) { 
	fprintf(stderr, "Could not get list of printers, error %d.\n", i);
    }
#endif
#ifdef SYSV
    if (GetSysVPrinterList(p->printpanel.printers,
			   &p->printpanel.num_printers, MAXPRINTERS) ==
	FALSE) {
	fprintf(stderr, "ppanel: couldn't get list of printers.\n");
    }
#endif /* SYSV */

    p->printpanel.default_printer_name = getenv("PRINTER");
    if (p->printpanel.printer_name != NULL)
	p->printpanel.default_printer_name = p->printpanel.printer_name; 
    if (p->printpanel.default_printer_name)
	strncpy(p->printpanel.current_printer_name,
		p->printpanel.default_printer_name, 30); 
    p->printpanel.default_filter_name = getenv("TSFILTER");


    p->printpanel.enscript_info.font =
	p->printpanel.enscript_info.header_font = 
	p->printpanel.enscript_info.header = NULL; 
    p->printpanel.tmp_enscript_info.font =
	p->printpanel.tmp_enscript_info.header_font =
	    p->printpanel.tmp_enscript_info.header = NULL;
    p->printpanel.enscript_info.font_size =
	p->printpanel.enscript_info.header_font_size =  
	p->printpanel.tmp_enscript_info.font_size =
	    p->printpanel.tmp_enscript_info.header_font_size = -1; 
    p->printpanel.enscript_info.tab_width =
	p->printpanel.enscript_info.columns =
	    p->printpanel.enscript_info.lines = -1;  
    p->printpanel.tmp_enscript_info.tab_width =
	p->printpanel.tmp_enscript_info.columns = 
	    p->printpanel.tmp_enscript_info.lines = -1; 
    p->printpanel.enscript_info.rotated =
	p->printpanel.enscript_info.norotate =
	    p->printpanel.enscript_info.gaudy =  
		p->printpanel.enscript_info.lpt_mode =
		    p->printpanel.enscript_info.no_header =
			p->printpanel.enscript_info.truncate_lines = 0;  
    p->printpanel.tmp_enscript_info.rotated =
	p->printpanel.tmp_enscript_info.norotate = 
	    p->printpanel.tmp_enscript_info.gaudy =
		p->printpanel.tmp_enscript_info.lpt_mode = 
		    p->printpanel.tmp_enscript_info.no_header =
			p->printpanel.tmp_enscript_info.truncate_lines = 0; 
    p->printpanel.enscript_info.ignore_binary =
	p->printpanel.enscript_info.report_missing_chars =
	    p->printpanel.enscript_info.quiet_mode =  
		p->printpanel.enscript_info.no_burst_page = 0; 
    p->printpanel.tmp_enscript_info.ignore_binary =
	p->printpanel.tmp_enscript_info.report_missing_chars = 
	    p->printpanel.tmp_enscript_info.quiet_mode =
		p->printpanel.tmp_enscript_info.no_burst_page = 0;

    p->printpanel.num_feature_children = 0;
    p->printpanel.num_features = 0;
    p->printpanel.tmp_line[0] = '\0';
    if (p->printpanel.filter_name != NULL) {
	p->printpanel.default_filter_name = p->printpanel.filter_name;
    }
    LayoutPanel(p);
    if (p->printpanel.in_file_name != NULL) {
	p->printpanel.input_file = p->printpanel.in_file_name;
	XmTextFieldSetString(p->printpanel.open_field_child,
			     p->printpanel.input_file);
    }
    tmp = getenv("PREVIEWER");
    if (tmp)
	p->printpanel.preview_path = tmp;
    else {
	/* find info file */
	tmp = getenv("PSLIBDIR");
	if (tmp)
	    strcpy(fname, tmp);
	else
	    strcpy(fname, PSLibDir);
	strcat(fname, "/preview.info");
	if ((fp = fopen(fname, "r")) == NULL) {
	    gray = TRUE;
	}
	else {
	    if (fgets(fname, 1024, fp)) {
		tmp = strchr(fname, '\n');
		if (tmp) *tmp = '\0';
		/* assign preview path */
		p->printpanel.preview_path = (char *) malloc(strlen(fname)+1);
		strcpy(p->printpanel.preview_path, fname);
	    }
	    else gray = TRUE;
	}
    }
    if (gray) {
	/* gray out preview option */
	XtSetSensitive(p->printpanel.preview_toggle_child, FALSE);
    }
    tmp = getenv("PSFAX");
    if (tmp)
	p->printpanel.fax_path = tmp;
    else
	p->printpanel.fax_path = "psfax";
}

void DisplayError(w, msg)
    Widget w;
    char *msg;
{
    int i;
    Arg args[10];
    XmString errMsg;

    errMsg = CS(msg);
    i = 0;
    XtSetArg(args[i], XmNmessageString, errMsg); i++;
    XtSetValues(w, args, i);

    XtManageChild(w);
}

void DisplayCompound(w, xstr)
    Widget w;
    XmString xstr;
{
    Arg args[10];
    int i;

    i = 0;
    XtSetArg(args[i], XmNmessageString, xstr); i++;
    XtSetValues(w, args, i);
    XtManageChild(w);
}

static void GrabData(p, source, id)
    PrintPanelWidget p;
    int *source;
    XtInputId *id;
{
    int cnt;
    char buf[1024];
    FILE *fp;

    if (p->printpanel.save_fd == -1) {
	if ((fp = fopen(p->printpanel.output_file, "w")) == NULL) {
	    sprintf(buf, "Couldn't open output file %s",
		    p->printpanel.output_file);
	    DisplayError(p->printpanel.message_dialog_child, buf);
	    close(*source);
	    XtRemoveInput(*id);
	    p->printpanel.save_fd = -1;
	    return;
	}
	p->printpanel.save_fd = fileno(fp);
    }

    if ((cnt = read(*source, buf, 1024)) > 0) {
	if (write(p->printpanel.save_fd, buf, cnt) != cnt) {
	    DisplayError(p->printpanel.message_dialog_child,
			 "Failure in write");
	    close(*source);
	    XtRemoveInput(*id);
	    return;
	}
    }
    else {
	/* finish up */
	close(p->printpanel.save_fd);
	p->printpanel.save_fd = -1;
	close(*source);
	XtRemoveInput(*id);
	return;
    }
}
    

static void ExecuteProg(argvec)
    char *argvec[];
{
    int i;
    execvp(argvec[0], argvec);
}


static void Execute(p, nvec, argvec, catch, usestdin)
    PrintPanelWidget p;
    int nvec;
    char **argvec[];
    int catch;
    int usestdin;
{
    /* this assumes that the 0'th argv includes the file name, so a pipe
       isn't necessary for connecting to 0'th child's stdin, and further
       assumes that unless catch is true, a pipe isn't necessary for
       connecting to last child's stdout */

    int cpids[10];
    int datapipes[10][2];
    int errorpipe[2];
    int i;
    int j;
    char buf[256];
    int status;
    int cfd;

    if (pipe(errorpipe)) {
	DisplayError(p->printpanel.message_dialog_child,
		     "Error pipe failed.");
	return;
    }
    for (i = 0; i < nvec; i++) {
	if (pipe(datapipes[i])) {
	    sprintf(buf, "Data pipe %d failed", i);
	    DisplayError(p->printpanel.message_dialog_child, buf);
	    return;
	}
    }

    for (i = 0; i < nvec; i++) {
	if ((cpids[i] = fork()) < 0) {
	    sprintf(buf, "Fork %d failed.", i);
	    DisplayError(p->printpanel.message_dialog_child, buf);
	    return;
	}
	if (cpids[i] == 0) {
	    /* child */
	    if (i > 0) {
		/* don't need this for first child */
		if (dup2(datapipes[i-1][0], 0) < 0) {
		    sprintf(buf, "Dup %d failed.", i);
		    DisplayError(p->printpanel.message_dialog_child, buf);
		    return;
		}
	    }
	    if (i < nvec - 1 || catch) {
		if (dup2(datapipes[i][1], 1) < 0) {
		    sprintf(buf, "Dup %d failed.", i);
		    DisplayError(p->printpanel.message_dialog_child, buf);
		    return;
		}
	    }
	    if ((dup2(errorpipe[1], 2) < 0) || close(errorpipe[1]) ||
		close(errorpipe[0])) {
		perror("");
		sprintf(buf, "Error dup in child %d failed.", i);
		DisplayError(p->printpanel.message_dialog_child, buf);
		return;
	    }
	    for (j = 0; j < nvec; j++) {
		if (close(datapipes[j][0]) || close(datapipes[j][1])) {
		    sprintf(buf, "Data close in child %d failed.", i);
		    DisplayError(p->printpanel.message_dialog_child, buf);
		    return;
		}
	    }
	    ExecuteProg(argvec[i]);
	    exit(0);
	}
    }
    /* parent */
    if ((dup2(errorpipe[0], p->printpanel.error_fd) < 0) ||
	close(errorpipe[0]) || close(errorpipe[1])) {
	perror("");
	DisplayError(p->printpanel.message_dialog_child,
		     "Error dup in parent failed.");
	return;
    }
    if (catch) {
	cfd = dup(0);
	if ((dup2(datapipes[nvec -1][0], cfd) < 0)) {
	    DisplayError(p->printpanel.message_dialog_child,
			 "Capture dup in parent failed");
	    return;
	}
	XtAppAddInput(XtWidgetToApplicationContext(p), cfd,
		      XtInputReadMask, GrabData, p);
    }
    for (j = 0; j < nvec; j++) {
	if (close(datapipes[j][0]) || close(datapipes[j][1])) {
	    DisplayError(p->printpanel.message_dialog_child,
			 "Data close in parent failed.");
	    return;
	}
    }
}

static void FilterArgs(p, argvec, nargs)
    PrintPanelWidget p;
    char **argvec;
    int *nargs;
{
    char buf[1024];
    int n;
    int i, j;

    n = *nargs;

    addarg(argvec, n++,
	   p->printpanel.filterlist[p->printpanel.filter].name); 
    switch (p->printpanel.filter) {
    case ENSCRIPT:
	/* enscript */
	if (p->printpanel.enscript_info.font) {
	    if (p->printpanel.enscript_info.font_size != -1) {
		sprintf(buf, "-f%s%f", p->printpanel.enscript_info.font,
			p->printpanel.enscript_info.font_size); 
		addarg(argvec, n++, buf); 
	    }
	    else {
		DisplayError(p->printpanel.message_dialog_child,
			     "Must specify font size!"); 
		return;
	    }
	}
	if (p->printpanel.enscript_info.header_font) {
	    if (p->printpanel.enscript_info.header_font_size != -1) {
		sprintf(buf, "-F%s%f",
			p->printpanel.enscript_info.header_font, 
			p->printpanel.enscript_info.header_font_size); 
		addarg(argvec, n++, buf); 
	    }
	    else {
		DisplayError(p->printpanel.message_dialog_child,
			     "Must specify header font size!");
		return;
	    }
	}
	if (p->printpanel.enscript_info.header) {
	    sprintf(buf, "-b%s", p->printpanel.enscript_info.header);
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.enscript_info.tab_width != -1) {
	    sprintf(buf, "-T%d", p->printpanel.enscript_info.tab_width);
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.enscript_info.columns != -1) {
	    sprintf(buf, "-v%d", p->printpanel.enscript_info.columns);
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.enscript_info.lines != -1) {
	    sprintf(buf, "-L%d", p->printpanel.enscript_info.lines);
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.enscript_info.rotated)
	    addarg(argvec, n++, "-r"); 
	if (p->printpanel.enscript_info.norotate)
	    addarg(argvec, n++, "-R"); 
	if (p->printpanel.enscript_info.gaudy)
	    addarg(argvec, n++, "-G"); 
	if (p->printpanel.enscript_info.lpt_mode)
	    addarg(argvec, n++, "-l"); 
	if (p->printpanel.enscript_info.no_header)
	    addarg(argvec, n++, "-B"); 
	if (p->printpanel.enscript_info.truncate_lines)
	    addarg(argvec, n++, "-c"); 
	if (p->printpanel.enscript_info.ignore_binary)
	    addarg(argvec, n++, "-g"); 
	if (p->printpanel.enscript_info.report_missing_chars)
	    addarg(argvec, n++, "-o"); 
	if (p->printpanel.enscript_info.quiet_mode)
	    addarg(argvec, n++, "-q"); 
	if (p->printpanel.enscript_info.no_burst_page)
	    addarg(argvec, n++, "-h");
	for (i = 0; i <= p->printpanel.num_features; i++) {
	    for (j = 0; j < p->printpanel.feature_info[i].num; j++) {
		if (p->printpanel.feature_info[i].value[j]) {
		    sprintf(buf, "-S%s=%s",
			    p->printpanel.feature_info[i].name,
			    p->printpanel.feature_info[i].value[j]);
		    addarg(argvec, n++, buf);
		}
	    }
	}
	sprintf(buf, "%s%s", printerstring,
		p->printpanel.current_printer_name);
	addarg(argvec, n++, buf);
	break;
    case PSROFF:
	if (p->printpanel.roff_info.font_dir != NULL) {
	    addarg(argvec, n++, "-D"); 
	    addarg(argvec, n++, p->printpanel.roff_info.font_dir);  
	}
    case PTROFF:
	if (p->printpanel.roff_info.font != NULL) {
	    addarg(argvec, n++, "-F"); 
	    addarg(argvec, n++, p->printpanel.roff_info.font);  
	} 
	if (p->printpanel.roff_info.options != NULL) {
	    addarg(argvec, n++, p->printpanel.roff_info.options);  
	}
	break;
    case PSPLT:
	if (p->printpanel.plot_profile != NULL) {
	    sprintf(buf, "-g%s", p->printpanel.plot_profile);
	    addarg(argvec, n++, buf); 
	}
	break;
    case PS630:
	if (p->printpanel.ps630_info.bold_font != NULL) {
	    sprintf(buf, "-F%s", p->printpanel.ps630_info.bold_font);
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.ps630_info.body_font != NULL) {
	    sprintf(buf, "-f%s", p->printpanel.ps630_info.body_font);
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.ps630_info.pitch != NULL) {
	    sprintf(buf, "-s%s", p->printpanel.ps630_info.pitch);
	    addarg(argvec, n++, buf); 
	}
    case PS4014:
	if (p->printpanel.ps4014_info.scale != NULL) {
	    sprintf(buf, "-S%s", p->printpanel.ps4014_info.scale);
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.ps4014_info.left != NULL &&
	    p->printpanel.ps4014_info.bottom != NULL) { 
	    sprintf(buf, "-l%s,%s", p->printpanel.ps4014_info.left,
		    p->printpanel.ps4014_info.bottom); 
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.ps4014_info.width != NULL &&
	    p->printpanel.ps4014_info.height != NULL) { 
	    sprintf(buf, "-s%s,%s", p->printpanel.ps4014_info.width,
		    p->printpanel.ps4014_info.height); 
	    addarg(argvec, n++, buf); 
	}
	if (p->printpanel.ps4014_info.portrait) {
	    addarg(argvec, n++, "-R"); 
	}
	if (p->printpanel.ps4014_info.cr_no_lf) { 
	    addarg(argvec, n++, "-C"); 
	}
	if (p->printpanel.ps4014_info.lf_no_cr) {
	    addarg(argvec, n++, "-N"); 
	}
	if (p->printpanel.ps4014_info.margin_2) {
	    addarg(argvec, n++, "-m"); 
	}
    default:
	break;
    }
    *nargs = n;
}



static void PreviewArgs(p, argvec, nargs)
    PrintPanelWidget p;
    char **argvec;
    int *nargs;
{
    char buf[100];
    int n;

    n = *nargs;
    addarg(argvec, n++, p->printpanel.preview_path);
    *nargs = n;
}

static void FaxArgs(p, argvec, nargs)
    PrintPanelWidget p;
    char **argvec;
    int *nargs;
{
    char buf[100];
    int n;

    n = *nargs;
    addarg(argvec, n++, p->printpanel.fax_path);

    sprintf(buf, "-P%s", p->printpanel.current_printer_name);
    addarg(argvec, n++, buf);

    if (p->printpanel.fax_data.key[0] != '\0') {
	sprintf(buf, "-k%s", p->printpanel.fax_data.key);
	addarg(argvec, n++, buf);
    }
    else {
	fprintf(stderr, "%s %s\n", p->printpanel.fax_data.phone,
		p->printpanel.fax_data.name); 
	if (p->printpanel.fax_data.phone[0] != '\0') {
	    sprintf(buf, "-d%s", p->printpanel.fax_data.phone);
	    addarg(argvec, n++, buf);
	}
	if (p->printpanel.fax_data.name[0] != '\0') {
	    sprintf(buf, "-n%s", p->printpanel.fax_data.name);
	    addarg(argvec, n++, buf);
	}
    }
    if (p->printpanel.fax_data.sendps) {
	addarg(argvec, n++, "-t");
	if (p->printpanel.fax_data.passwd[0] != '\0') {
	    sprintf(buf, "-SPostScriptPassword=%s",
		    p->printpanel.fax_data.passwd);
	    addarg(argvec, n++, buf);
	}
    }
    if (p->printpanel.fax_data.save) {
	if (p->printpanel.fax_data.filename[0] != '\0') {
	    sprintf(buf, "-p%s", p->printpanel.fax_data.filename);
	    addarg(argvec, n++, buf);
	}
	else {
	    DisplayError(p->printpanel.message_dialog_child,
			 "Must specify a file name to save!");
	    return;
	}
    }
    *nargs = n;
}

static void PsdraftArgs(p, argvec, nargs)
    PrintPanelWidget p;
    char **argvec;
    int *nargs;
{
    char buf[100];
    int n;

    n = *nargs;

    addarg(argvec, n++, "psdraft");
    sprintf(buf, "-P%s", p->printpanel.current_printer_name);
    addarg(argvec, n++, buf);
    if (p->printpanel.psdraft_info.draftstring) {
	sprintf(buf, "-s%s", p->printpanel.psdraft_info.draftstring);
	addarg(argvec, n++, buf);
    }
    if (p->printpanel.psdraft_info.xpos) {
	sprintf(buf, "-x%f", p->printpanel.psdraft_info.xpos);
	addarg(argvec, n++, buf);
    }
    if (p->printpanel.psdraft_info.ypos) {
	sprintf(buf, "-y%f", p->printpanel.psdraft_info.ypos);
	addarg(argvec, n++, buf);
    }
    if (p->printpanel.psdraft_info.angle) {
	sprintf(buf, "-r%f", p->printpanel.psdraft_info.angle);
	addarg(argvec, n++, buf);
    }
    if (p->printpanel.psdraft_info.font) {
	sprintf(buf, "-f%s%f", p->printpanel.psdraft_info.font,
		p->printpanel.psdraft_info.size);
	addarg(argvec, n++, buf);
    }
    if (p->printpanel.psdraft_info.gray) {
	sprintf(buf, "-g%f", p->printpanel.psdraft_info.gray);
	addarg(argvec, n++, buf);
    }
    if (p->printpanel.psdraft_info.outline) {
	addarg(argvec, n++, "-o");
    }
}

    
	

static void PsnupArgs(p, argvec, nargs)
    PrintPanelWidget p;
    char **argvec;
    int *nargs;
{
    char buf[100];
    int n;

    n = *nargs;

    addarg(argvec, n++, "psnup");
    sprintf(buf, "-P%s", p->printpanel.current_printer_name);
    addarg(argvec, n++, buf);
    sprintf(buf, "-n%dx%d", p->printpanel.nup_info.rows,
	    p->printpanel.nup_info.columns); 
     addarg(argvec, n++, buf);
    if (p->printpanel.nup_info.rotated) {
	addarg(argvec, n++, "-r");
    }
    if (p->printpanel.nup_info.gaudy) {
	addarg(argvec, n++, "-G");
    }
    *nargs = n;
}
    
static void SpoolerArgs(p, argvec, nargs)
    PrintPanelWidget p;
    char **argvec;
    int *nargs;
{
    char buf[100];
    int i, j;
    int n;

    n = *nargs;

    if (p->printpanel.use_pslpr) {
	addarg(argvec, n++, "pslpr");
	if (p->printpanel.range == RANGE) {
	    sprintf(buf, "-i %d-%d", p->printpanel.page_ranges[0].begin,
		    p->printpanel.page_ranges[0].end); 
	    addarg(argvec, n++, buf);
	}
	if (p->printpanel.pslpr_info.showpage)
	    addarg(argvec, n++, "-e");
	if (p->printpanel.pslpr_info.landscape)
	    addarg(argvec, n++, "-L");
	if (p->printpanel.pslpr_info.squeeze)
	    addarg(argvec, n++, "-q");
	if (p->printpanel.pslpr_info.overtranslate)
	    addarg(argvec, n++, "-u");
	for (i = 0; i <= p->printpanel.num_features; i++) {
	    for (j = 0; j < p->printpanel.feature_info[i].num; j++) {
		if (p->printpanel.feature_info[i].value[j]) {
		    sprintf(buf, "-S%s=%s",
			    p->printpanel.feature_info[i].name, 
			    p->printpanel.feature_info[i].value[j]);
		    addarg(argvec, n++, buf);
		}
	    }
	}
    }
    else
	addarg(argvec, n++, lprname);
    sprintf(buf, "%s%s", printerstring, p->printpanel.current_printer_name);
    addarg(argvec, n++, buf);
    if (p->printpanel.copies > 1) {
	sprintf(buf, "-#%d", p->printpanel.copies);
	addarg(argvec, n++, buf);
    }
    *nargs = n;
}    

void PlanExecution(p)
    PrintPanelWidget p;
{
    int nprogs;
    char *filargs[100];
    int nfilargs = 0;
    char *draftargs[100];
    int ndraftargs = 0;
    int catch = FALSE;
    char *nupargs[100];
    int nnupargs = 0;
    char *pslprargs[100];
    int npslprargs = 0;
    char *spoolargs[100];
    int nspoolargs = 0;
    char *prevargs[100];
    int nprevargs = 0;
    char *faxargs[100];
    int nfaxargs = 0;
    int ndex;
    char buf[255];
    int nvectors = 0;
    char **vectors[10];
    int i, j;
    int usestdin = FALSE;

    if (p->printpanel.input_file[0] == '-')
	usestdin = TRUE;

    if (p->printpanel.filter != NONE) {
	/* we have a filter */
	FilterArgs(p, filargs, &nfilargs);
	ndex = p->printpanel.filterlist[p->printpanel.filter].index;
	if (p->printpanel.action == PRINT && !p->printpanel.use_pslpr &&
	    !p->printpanel.use_psnup && !p->printpanel.use_psdraft &&
	    filter_array[ndex].can_spool) {  
	    /* simple case, nothing special, just filter and spool */
	    sprintf(buf, "%s%s", filter_array[ndex].spool_flag,
		    p->printpanel.current_printer_name);
	    addarg(filargs, nfilargs++, buf);
	    if (!usestdin)
		addarg(filargs, nfilargs++, p->printpanel.input_file);
	    vectors[nvectors++] = filargs;

	    Execute(p, nvectors, vectors, FALSE, usestdin);
	    return;
	}
	/* handle all other cases */
	if (p->printpanel.use_psnup || p->printpanel.use_pslpr ||
	    p->printpanel.use_psdraft ||
	    (p->printpanel.action == SAVE &&
	     !filter_array[ndex].can_write) || p->printpanel.action ==
	    PREVIEW || p->printpanel.action == FAX) { 
	    /* need to send filter output to stdout */
	    if (filter_array[ndex].stdout_flag) {
		addarg(filargs, nfilargs++,
		       filter_array[ndex].stdout_flag); 
	    }
	    /* set catch to TRUE; we'll unset it if need be later */
	    /* catch tells us whether we need to handle catching the final
	       output or not */
	    catch = TRUE;
	}
	else if (p->printpanel.action == SAVE &&
		 filter_array[ndex].can_write) {
	    /* no more processes needed, filter can write to file directly 
	       */ 
	    sprintf(buf, "%s%s", filter_array[ndex].file_flag,
		    p->printpanel.output_file);
	    addarg(filargs, nfilargs++, buf);
	}
	if (!usestdin)
	    addarg(filargs, nfilargs++, p->printpanel.input_file);
	vectors[nvectors++] = filargs;
	if (p->printpanel.use_psdraft) {
	    PsdraftArgs(p, draftargs, &ndraftargs);
	    if (p->printpanel.action == SAVE && !p->printpanel.use_psnup &&
		!p->printpanel.use_pslpr) {
		/* if we're saving, and there are no more processes after
		   us, need to save to file here */
		sprintf(buf, "-p%s", p->printpanel.output_file);
		addarg(draftargs, ndraftargs++, buf);
		catch = FALSE;
	    }
	    vectors[nvectors++] = draftargs;
	}
	if (p->printpanel.use_psnup) {
	    PsnupArgs(p, nupargs, &nnupargs);
	    if (p->printpanel.use_pslpr) {
		/* if we need both pslpr and psnup, we want to run pslpr
		   first */
		SpoolerArgs(p, pslprargs, &npslprargs);
		addarg(pslprargs, npslprargs++, "-p-");
		vectors[nvectors++] = pslprargs;
		p->printpanel.use_pslpr = FALSE;
	    }
	    if (p->printpanel.action == SAVE) {
		/* psnup is always last in chain, have it write the
		   file */
		sprintf(buf, "-p%s", p->printpanel.output_file);
		addarg(nupargs, nnupargs++, buf);
		catch = FALSE;
	    }
	    vectors[nvectors++] = nupargs;
	}
	if (p->printpanel.action == PRINT) {
	    /* SpoolerArgs will handle the pslpr/lpr distinction */
	    SpoolerArgs(p, spoolargs, &nspoolargs);
	    vectors[nvectors++] = spoolargs;
	    p->printpanel.use_pslpr = FALSE;
	    catch = FALSE;
	}
	if (p->printpanel.use_pslpr) {
	    /* already handled psnup, and if we've already handled
	       pslpr, use_pslpr is now set to FALSE */
	    SpoolerArgs(p, pslprargs, &npslprargs);
	    /* since we've handled PRINT already, should either save to
	       file or send to stdout */
	    if (p->printpanel.action == SAVE) {
		sprintf(buf, "-p%s", p->printpanel.output_file);
		addarg(pslprargs, npslprargs++, buf);
		catch = FALSE;
	    }
	    else {
		addarg(pslprargs, npslprargs++, "-p-");
	    }
	    vectors[nvectors++] = pslprargs;
	    p->printpanel.use_pslpr = FALSE;
	}
	if (p->printpanel.action == PREVIEW) {
	    PreviewArgs(p, prevargs, &nprevargs);
	    addarg(prevargs, nprevargs++, "-");
	    vectors[nvectors++] = prevargs;
	    catch = FALSE;
	}
	if (p->printpanel.action == FAX) {
	    FaxArgs(p, faxargs, &nfaxargs);
	    vectors[nvectors++] = faxargs;
	    catch = FALSE;
	}
	Execute(p, nvectors, vectors, catch, usestdin);
    }
    else {
	/* no filter requested */
	if (p->printpanel.use_psdraft) {
	    PsdraftArgs(p, draftargs, &ndraftargs);
	    if (p->printpanel.action == SAVE && !p->printpanel.use_psnup &&
		!p->printpanel.use_pslpr) {
		/* if we're saving, and there are no more processes after
		   us, need to save to file here */
		sprintf(buf, "-p%s", p->printpanel.output_file);
		addarg(draftargs, ndraftargs++, buf);
		catch = FALSE;
	    }
	    /* this will be first in chain */
	    if (!usestdin)
		addarg(draftargs, ndraftargs++, p->printpanel.input_file);
	    vectors[nvectors++] = draftargs;
	}
	if (p->printpanel.use_psnup) {
	    PsnupArgs(p, nupargs, &nnupargs);
	    if (p->printpanel.action == SAVE) {
		sprintf(buf, "-p%s", p->printpanel.output_file);
		addarg(nupargs, nnupargs++, buf);
	    }
	    if (p->printpanel.use_pslpr) {
		/* it will be ahead of psnup  in chain if needed */
		SpoolerArgs(p, pslprargs, &npslprargs);
		addarg(pslprargs, npslprargs++, "-p-");
		if (!usestdin && !p->printpanel.use_psdraft)
		    addarg(pslprargs, npslprargs++,
			   p->printpanel.input_file); 
		p->printpanel.use_pslpr = FALSE;
		vectors[nvectors++] = pslprargs;
	    }
	    else {
		/* psnup is first in chain */
		if (!usestdin && !p->printpanel.use_psdraft)
		    addarg(nupargs, nnupargs++, p->printpanel.input_file); 
	    }
	    vectors[nvectors++] = nupargs;
	}
	if (p->printpanel.action == PRINT) {
	    SpoolerArgs(p, spoolargs, &nspoolargs);
	    if (!p->printpanel.use_psnup && !usestdin)
		addarg(spoolargs, nspoolargs++, p->printpanel.input_file); 
	    vectors[nvectors++] = spoolargs;
	    p->printpanel.use_pslpr = FALSE;
	}
	if (p->printpanel.use_pslpr) {
	    SpoolerArgs(p, pslprargs, &npslprargs);
	    if (p->printpanel.action == SAVE) {
		sprintf(buf, "-p%s", p->printpanel.output_file);
		addarg(pslprargs, npslprargs++, buf);
	    }
	    else {
		addarg(pslprargs, npslprargs++, "-p-");
	    }
	    if (!usestdin && !p->printpanel.use_psdraft)
		addarg(pslprargs, npslprargs++, p->printpanel.input_file);
	    vectors[nvectors++] = pslprargs;
	}
	if (p->printpanel.action == PREVIEW) {
	    PreviewArgs(p, prevargs, &nprevargs);
	    if (p->printpanel.use_pslpr || p->printpanel.use_psnup ||
		p->printpanel.use_psdraft || usestdin) { 
		addarg(prevargs, nprevargs++, "-");
	    }
	    else 
		addarg(prevargs, nprevargs++, p->printpanel.input_file);
	    vectors[nvectors++] = prevargs;
	}
	if (p->printpanel.action == FAX) {
	    FaxArgs(p, faxargs, &nfaxargs);
	    if (!p->printpanel.use_pslpr && !p->printpanel.use_psnup &&
		!p->printpanel.use_psdraft && !usestdin) { 
		addarg(faxargs, nfaxargs++, p->printpanel.input_file);
	    }
	    vectors[nvectors++] = faxargs;
	}
	Execute(p, nvectors, vectors, catch, stdin);
    }
}

static void DoWork(p)
    PrintPanelWidget p;
{
    int i, j;

    p->printpanel.use_pslpr = FALSE;
    if (p->printpanel.range == RANGE) {
	if (p->printpanel.page_ranges[0].begin == -1 ||
	    p->printpanel.page_ranges[0].end == -1) { 
	    DisplayError(p->printpanel.message_dialog_child,
			 "Must specify page numbers for range."); 
	    return; 
	}
	p->printpanel.use_pslpr = TRUE; 
    }
    if (p->printpanel.pslpr_info.showpage ||
	p->printpanel.pslpr_info.landscape ||
	p->printpanel.pslpr_info.squeeze ||
	p->printpanel.pslpr_info.overtranslate) 
	p->printpanel.use_pslpr = TRUE;
    for (i = 0; i <= p->printpanel.num_features; i++) {
	for (j = 0; j < p->printpanel.feature_info[i].num; j++) {
	    if (p->printpanel.feature_info[i].value[j] != NULL) {
		p->printpanel.use_pslpr = TRUE;
		break;
	    }
	    if (j < p->printpanel.feature_info[i].num)
		break;
	}
    }
    p->printpanel.error_fd = dup(0);
    XtAppAddInput(XtWidgetToApplicationContext(p), p->printpanel.error_fd,
		  XtInputReadMask, ReadErrors, p); 

    /* for now, our only input source is a file name, so complain if there
       isn't one */
    if (p->printpanel.input_file == NULL) {
	DisplayError(p->printpanel.message_dialog_child,
		     "You must specify an input file!");
	return;
    }
    if (p->printpanel.action == SAVE && p->printpanel.output_file == NULL) {
	DisplayError(p->printpanel.message_dialog_child,
		     "You must specify an output file!");
	return;
    }
    PlanExecution(p);
}

/* callbacks */

static void okCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    DoWork(p);
    XtCallCallbackList(w, p->printpanel.ok_callback, (XtPointer) NULL);
}

static void applyCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    DoWork(p);
}

static void resetCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{

    p->printpanel.action = PRINT;
    XmToggleButtonSetState(p->printpanel.print_toggle_child, TRUE, TRUE);
    XmListSelectItem(p->printpanel.filter_list_child,
		     CS(p->printpanel.default_filter_name), TRUE);
    XmListSelectItem(p->printpanel.printer_list_child,
		     CS(p->printpanel.default_printer_name), TRUE);
    XmToggleButtonSetState(p->printpanel.all_toggle_child, TRUE, TRUE);
    p->printpanel.use_pslpr = FALSE;
    p->printpanel.use_psnup = FALSE;
    p->printpanel.copies = 1;
    XmTextFieldSetString(p->printpanel.copy_field_child, "1");
    XmTextFieldSetString(p->printpanel.begin_field_child, "");
    XmTextFieldSetString(p->printpanel.end_field_child, "");
    XmTextFieldSetString(p->printpanel.open_field_child, "");
    XmTextFieldSetString(p->printpanel.save_text_child, "");
    XmTextFieldSetString(p->printpanel.fax_text_child, "");
    
    
}
static void cancelCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    XtCallCallbackList(w, p->printpanel.cancel_callback, (XtPointer) NULL);
}

static void ListSelection(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmListCallbackStruct *call_data;
{
    char *text;
    FILE *fp;
    int ret;

    if (XmStringGetLtoR(call_data->item, XmSTRING_DEFAULT_CHARSET,
			&text)) {
	if (w == p->printpanel.printer_list_child) {
	    XmTextFieldSetString(p->printpanel.printer_field_child, text);
	    strncpy(p->printpanel.current_printer_name, text, 30);
	    fp = (FILE *) GetPPD(p->printpanel.current_printer_name);
	    ret = ReadPPD(p, fp);
	    if (ret == FALSE) {
		CleanupOldFeatures(p);
		if (p->printpanel.message_dialog_child)
		    DisplayError(p->printpanel.message_dialog_child,
				 "Error reading PPD file\n");
		else
		    fprintf(stderr, "Error reading PPD file\n");
		return;
	    }
	    CleanupOldFeatures(p);
	    BuildFeatures(p);
	    if (p->printpanel.features.supports_fax != TRUE)
		XtSetSensitive(p->printpanel.fax_toggle_child, FALSE);
	}
	else if (w == p->printpanel.filter_list_child) {
	    XmTextFieldSetString(p->printpanel.filter_field_child, text);
	    p->printpanel.old_filter = p->printpanel.filter;
	    p->printpanel.filter = GetFilter(p, text);
	    /* clean up old filter */
	    if (p->printpanel.old_filter != p->printpanel.filter) {
		Cleanup(p, p->printpanel.old_filter);
		if (p->printpanel.old_filter == PSROFF)
		    Cleanup(p, PTROFF);
		SetupFilter(p, p->printpanel.filter);
	    }
	}
	else if (w == p->printpanel.fax_phone_list_child) {
	    strcpy(p->printpanel.tmp_fax_data.key, text);
	    CleanupPhoneEntry(p);
	    BuildPhoneEntry(p);
	}
    }
}


void SensitizeFax(p, value)
    PrintPanelWidget p;
    int value;
{
    XtSetSensitive(p->printpanel.fax_label_child, value);
    XtSetSensitive(p->printpanel.fax_text_child, value);
    XtSetSensitive(p->printpanel.fax_button_child, value);
}

void SensitizeSave(p, value)
    PrintPanelWidget p;
    int value;
{
    XtSetSensitive(p->printpanel.save_label_child, value);
    XtSetSensitive(p->printpanel.save_text_child, value);
    XtSetSensitive(p->printpanel.save_button_child, value);
}

static void ToggleCallback(w, data, call_data)
    Widget w;
    int *data;
    XmToggleButtonCallbackStruct *call_data;
{
    *data = call_data->set;
}

static void DraftToggleCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmToggleButtonCallbackStruct *call_data;
{
    p->printpanel.tmp_psdraft = call_data->set;

    if (p->printpanel.tmp_psdraft) {
	XtSetSensitive(p->printpanel.special_draft_button_child, TRUE);
    }
    else {
	XtSetSensitive(p->printpanel.special_draft_button_child, FALSE);
    }
}

static void LandscapeToggleCallback(w, data, call_data)
    Widget w;
    int *data;
    XmToggleButtonCallbackStruct *call_data;
{
    PrintPanelWidget p = (PrintPanelWidget)
	XtParent(XtParent(XtParent(XtParent(w)))); 
    *data= call_data->set;

    if (call_data->set) {
	XtSetSensitive(p->printpanel.special_overtranslate_toggle_child,
		      TRUE); 
	XtSetSensitive(p->printpanel.special_squeeze_toggle_child, TRUE);
    }
    else {
	XtSetSensitive(p->printpanel.special_overtranslate_toggle_child,
		      FALSE); 
	XtSetSensitive(p->printpanel.special_squeeze_toggle_child, FALSE);
    }
}

static void FaxToggleCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmToggleButtonCallbackStruct *call_data;
{
    p->printpanel.tmp_fax_data.save = call_data->set;

    if (p->printpanel.tmp_fax_data.save)
	XtSetSensitive(p->printpanel.fax_save_button_child, TRUE);
    else 
	XtSetSensitive(p->printpanel.fax_save_button_child, FALSE);
}

static void NupToggleCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmToggleButtonCallbackStruct *call_data;
{

    p->printpanel.tmp_psnup = call_data->set;

    if (p->printpanel.tmp_psnup) {
	XtSetSensitive(p->printpanel.special_gaudy_toggle_child, TRUE);
	XtSetSensitive(p->printpanel.special_rotate_toggle_child, TRUE);
	XtSetSensitive(p->printpanel.special_nup_column_label_child, TRUE);
	XtSetSensitive(p->printpanel.special_nup_column_field_child, TRUE);
	XtSetSensitive(p->printpanel.special_nup_row_label_child, TRUE);
	XtSetSensitive(p->printpanel.special_nup_row_field_child, TRUE);
    }
    else {
	XtSetSensitive(p->printpanel.special_gaudy_toggle_child, FALSE);
	XtSetSensitive(p->printpanel.special_rotate_toggle_child, FALSE);
	XtSetSensitive(p->printpanel.special_nup_column_label_child, FALSE);
	XtSetSensitive(p->printpanel.special_nup_column_field_child, FALSE);
	XtSetSensitive(p->printpanel.special_nup_row_label_child, FALSE);
	XtSetSensitive(p->printpanel.special_nup_row_field_child, FALSE);
    }
}

static void RadioCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmRowColumnCallbackStruct *call_data;
{
    XmToggleButtonCallbackStruct *cdata;

    cdata = (XmToggleButtonCallbackStruct *) call_data->callbackstruct;
    if (call_data->widget == p->printpanel.print_toggle_child &&
	cdata->set)	{    
        p->printpanel.action = PRINT;
    }
    else if (call_data->widget == p->printpanel.preview_toggle_child &&
	     cdata->set ) { 
	p->printpanel.action = PREVIEW;
    }
    else if (call_data->widget == p->printpanel.save_toggle_child) {
	if (cdata->set) {
	    p->printpanel.action = SAVE;
	    SensitizeSave(p, TRUE);
	}
	else SensitizeSave(p, FALSE);
    }
    else if (call_data->widget == p->printpanel.fax_toggle_child) {
	if (cdata->set) {
	    p->printpanel.action = FAX;
	    SensitizeFax(p, TRUE);
	}
	else SensitizeFax(p, FALSE);
    }
}

static void FaxFormatCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmRowColumnCallbackStruct *call_data;
{

    XmToggleButtonCallbackStruct *cdata;

    cdata = (XmToggleButtonCallbackStruct *)call_data->callbackstruct;

    if (call_data->widget == p->printpanel.fax_teleps_toggle_child) {
	if (cdata->set) {
	    XtSetSensitive(p->printpanel.fax_teleps_field_child, TRUE);
	    XtSetSensitive(p->printpanel.fax_teleps_label_child, TRUE);
	    p->printpanel.tmp_fax_data.sendps = TRUE;
	}
	else {
	    XtSetSensitive(p->printpanel.fax_teleps_field_child, FALSE);
	    XtSetSensitive(p->printpanel.fax_teleps_label_child, FALSE);
	    p->printpanel.tmp_fax_data.sendps = FALSE;
	}
    }
}
	    

static void FieldChange(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    char *value;

    value = XmTextFieldGetString(w);
    if (*value == '\0')
	return;
    if (w == p->printpanel.fax_rec_field_child) {
	strcpy(p->printpanel.tmp_fax_data.name, value);
    }
    else if (w == p->printpanel.fax_num_field_child) {
	strcpy(p->printpanel.tmp_fax_data.phone, value);
    }
}

static void FilterFieldChange(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    char *value;
    int i;

    value = XmTextFieldGetString(w);
    if (*value == '\0')
	XmListDeselectAllItems(p->printpanel.filter_list_child);
    else {
	for (i = 0; i < p->printpanel.num_filters; i++) {
	    if (strncmp(value, p->printpanel.filterlist[i].name,
			strlen(value)) == 0) { 
		XmListSelectItem(p->printpanel.filter_list_child,
				 CS(p->printpanel.filterlist[i].name),
				    FALSE);  
		break;
	    }
	}
	if (i == 7)
	    XmListDeselectAllItems(p->printpanel.filter_list_child);
    }
}

static void ChooseFileCallback(w, whichfile, call_data)
    Widget w;
    Widget whichfile;
    XmAnyCallbackStruct *call_data;
{
    XtManageChild(whichfile);
}

static void FileOKCallback(w, mydata, call_data)
    Widget w;
    FileSBData *mydata;
    XmAnyCallbackStruct *call_data;
{
    PrintPanelWidget p = (PrintPanelWidget) mydata->head;
    XmString val;
    int i;
    Arg args[10];

    i = 0;
    XtSetArg(args[i], XmNtextString, &val); i++;
    XtGetValues(mydata->which_sb, args, i);
    if (!XmStringGetLtoR(val, XmSTRING_DEFAULT_CHARSET, mydata->name))
	DisplayError(p->printpanel.message_dialog_child,
		     "Value of field unreadable.");
    if (mydata->which_field != NULL)
	XmTextFieldSetString(mydata->which_field, *(mydata->name));
    XtUnmanageChild(mydata->which_sb);
}

static void FileCancelCallback(w, mydata, call_data)
    Widget w;
    FileSBData *mydata;
    XmAnyCallbackStruct *call_data;
{
    XtUnmanageChild(mydata->which_sb);
}

static void RangeCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmRowColumnCallbackStruct *call_data;
{
    XmToggleButtonCallbackStruct *cdata;

    cdata = (XmToggleButtonCallbackStruct *) call_data->callbackstruct;

    if (call_data->widget == p->printpanel.all_toggle_child && cdata->set) {
	p->printpanel.range = ALL;
	XtSetSensitive(p->printpanel.begin_field_child, FALSE);
	XtSetSensitive(p->printpanel.end_field_child, FALSE);
    }
    else if (call_data->widget == p->printpanel.range_toggle_child &&
	     cdata->set) { 
	p->printpanel.range = RANGE;
	XtSetSensitive(p->printpanel.begin_field_child, TRUE);
	XtSetSensitive(p->printpanel.end_field_child, TRUE);
    }
}

static void IntTextFieldChange(w, data, call_data)
    Widget w;
    int *data;
    XmAnyCallbackStruct *call_data;
{
    char *value;

    value = XmTextFieldGetString(w);
    *data = atoi(value);
}

static void RealTextFieldChange(w, data, call_data)
    Widget w;
    float *data;
    XmAnyCallbackStruct *call_data;
{
    char *value;

    value = XmTextFieldGetString(w);
    *data = strtod(value, NULL);
}

static void StringTextFieldChange(w, data, call_data)
    Widget w;
    char **data;
    XmAnyCallbackStruct *call_data;
{
    *data = XmTextFieldGetString(w);
    if (**data == '\0')
	*data = NULL;
}

static void FindLines(str, carryover, list)
    char *str;
    char *carryover;
    XmString *list;
{
    char *p, *q;

    q = str;
    while ((p = strchr(q, '\n')) != NULL) {
	*p = '\0';
	p++;
	if (*carryover != '\0') {
	    /* tack new stuff onto old stuff */
	    strcat(carryover, q);
	    AddToCompoundList(list, carryover);
	    *carryover = '\0';
	}
	else
	    AddToCompoundList(list, q);
	q = p;
	if (*q == '\0')
	    break;
    }
    if (*q != '\0') 
	/* save the fragment */
	strcat(carryover, q);
}

static void ReadErrors(p, source, id)
    PrintPanelWidget p;
    int *source;
    XtInputId *id;
{
    int cnt;
    FILE *fp;
    char buf[1024];


    buf[0] = '\0';
    cnt = 0;
    if ((cnt = read(*source, buf, 1024)) > 0) {
	buf[cnt] = '\0';
	FindLines(buf, p->printpanel.tmp_line, &p->printpanel.err_list);
    }
    else {
	if (p->printpanel.tmp_line[0] != '\0')
	    AddToCompoundList(&p->printpanel.err_list, p->printpanel.tmp_line);
	if (p->printpanel.err_list) {
	    DisplayCompound(p->printpanel.message_dialog_child,
			    p->printpanel.err_list); 
	    XtFree(p->printpanel.err_list);
	    p->printpanel.err_list = (XmString) NULL;
	}
	close(*source);
	XtRemoveInput(*id);
	p->printpanel.tmp_line[0] = '\0';
    }
}

static void CancelPopup(w, call_data)
    Widget w;
    XmAnyCallbackStruct *call_data;
{
    XtUnmanageChild(w);
}

static XtInputCallbackProc ReadStatus(client_data, source, id)
    XtPointer client_data;
    int *source;
    XtInputId *id;
{
    int cnt;
    FILE *fp;
    char buf[1024];
    PrintPanelWidget p = (PrintPanelWidget) client_data;
    

    if ((cnt = read(*source, buf, 1024)) > 0) {
	buf[cnt] = '\0';
	FindLines(buf, p->printpanel.tmp_line, &p->printpanel.status_list);
    }
    else {
	if (p->printpanel.tmp_line[0] != '\0')
	    AddToCompoundList(&p->printpanel.status_list, p->printpanel.tmp_line);
	DisplayCompound(p->printpanel.message_dialog_child,
			p->printpanel.status_list); 
	close(*source);
	XtRemoveInput(*id);
	p->printpanel.status_list = (XmString) NULL;
	XtFree(p->printpanel.status_list);
	p->printpanel.tmp_line[0] = '\0';
    }
}

static void PrinterStatusCallback(w, call_data)
    Widget w;
    XmAnyCallbackStruct *call_data;
{
    int statuspipe[2];
    int fpid;
    char *argstr[10];
    int nargs = 0;
    char buf[100];
    PrintPanelWidget p = (PrintPanelWidget) XtParent(XtParent(w));
#ifdef SYSV
#define PSTAT "lpstat"
#else
#define PSTAT "lpq"
#endif    

    addarg(argstr, nargs++, PSTAT);
#ifdef SYSV
    sprintf(buf, "-p%s", p->printpanel.current_printer_name);
#else    
    sprintf(buf, "-P%s", p->printpanel.current_printer_name);
#endif
    addarg(argstr, nargs++, buf);

    p->printpanel.status_fd = dup(0);
    XtAppAddInput(XtWidgetToApplicationContext(p), p->printpanel.status_fd,
		  XtInputReadMask, ReadStatus, p); 
    
    if (pipe(statuspipe)) {
	DisplayError(p->printpanel.message_dialog_child,
		     "Pipe creation failed.\n");
	return;
    }

    if ((fpid = fork()) < 0) {
	DisplayError(p->printpanel.message_dialog_child,
		     "Process fork failed.\n");
	return;
    }

    if (fpid == 0) {
	/* child process */
	if ((dup2(statuspipe[1], 1) < 0) || close(statuspipe[0]) ||
	    close(statuspipe[1])) { 
	    DisplayError(p->printpanel.message_dialog_child,
			 "Pipe error in child process.\n");
	    exit(1);
	}
	execvp(PSTAT, argstr);
	DisplayError(p->printpanel.message_dialog_child,
		     "Could not get printer status");
	exit(1);
    }
    /* parent */
    if ((dup2(statuspipe[0], p->printpanel.status_fd) < 0) ||
	close(statuspipe[0]) || close(statuspipe[1])) {
	DisplayError(p->printpanel.message_dialog_child,
		     "Pipe error in parent process.");
	return;
    }
}

static void ResetOptions(p)
    PrintPanelWidget p;
{
    int i;
    int num;
    WidgetClass wclass;
    Widget w;

    num = p->printpanel.option_children[p->printpanel.filter].num_children;
    for (i = 0; i < num; i++) {
	w = p->printpanel.option_children[p->printpanel.filter].child[i];
	wclass = XtClass(w);

	if (wclass == xmTextFieldWidgetClass) {
	    /* text field; set to null */
	    XmTextFieldSetString(w, "");
	}
	else if (wclass == xmToggleButtonWidgetClass) {
	    /* toggle button; set to false */
	    XmToggleButtonSetState(w, FALSE, TRUE);
	}
    }
}
	
static void SecondaryResetCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    if (w == p->printpanel.special_reset_button_child) {
	XmToggleButtonSetState(p->printpanel.special_nup_toggle_child,
			       FALSE, TRUE);
	XmTextFieldSetString(p->printpanel.special_nup_row_field_child,
			     "2"); 
	XmTextFieldSetString(p->printpanel.special_nup_column_field_child,
			     "2"); 
	XmToggleButtonSetState(p->printpanel.special_gaudy_toggle_child,
			       FALSE, TRUE);
	XmToggleButtonSetState(p->printpanel.special_rotate_toggle_child,
			       FALSE, TRUE);
	XmToggleButtonSetState(p->printpanel.special_overtranslate_toggle_child, 
			       FALSE, TRUE);
	XmToggleButtonSetState(p->printpanel.special_squeeze_toggle_child,
			       FALSE, TRUE);
	XmToggleButtonSetState(p->printpanel.special_showpage_toggle_child,
			       FALSE, TRUE);
	XmToggleButtonSetState(p->printpanel.special_landscape_toggle_child,
			       FALSE, TRUE);
    }
    else if (w == p->printpanel.generic_reset_button_child) {
	ResetOptions(p);
    }
}
 
static void SecondaryCancelCallback(w, whichone, call_data)
    Widget w;
    Widget *whichone;
    XmAnyCallbackStruct *call_data;
{
    XtUnmanageChild(*whichone);
}

static void SecondaryCallback(w, whichone, call_data)
    Widget w;
    Widget *whichone;
    XmAnyCallbackStruct *call_data;
{
    int i;
    Arg args[10];
    Dimension width;
    Dimension height;
    int test;
    PrintPanelWidget p = (PrintPanelWidget) XtParent(XtParent(w));
	
    if (*whichone == p->printpanel.generic_panel_child &&
	p->printpanel.filter == -1) 
	return;
    if (*whichone == NULL)
	return;

    XtRealizeWidget(*whichone);
    XtManageChild(*whichone);
}

static void SetOptions(p, filt)
    PrintPanelWidget p;
    int filt;
{
    switch (filt) {
    case  ENSCRIPT:
	p->printpanel.enscript_info.font =
	    p->printpanel.tmp_enscript_info.font; 
	p->printpanel.enscript_info.header_font =
	    p->printpanel.tmp_enscript_info.header_font; 
	p->printpanel.enscript_info.font_size =
	    p->printpanel.tmp_enscript_info.font_size; 
	p->printpanel.enscript_info.header_font_size =
	    p->printpanel.tmp_enscript_info.header_font_size; 
	p->printpanel.enscript_info.header =
	    p->printpanel.tmp_enscript_info.header; 
	p->printpanel.enscript_info.tab_width =
	    p->printpanel.tmp_enscript_info.tab_width; 
	p->printpanel.enscript_info.columns =
	    p->printpanel.tmp_enscript_info.columns; 
	p->printpanel.enscript_info.lines =
	    p->printpanel.tmp_enscript_info.lines; 
	p->printpanel.enscript_info.rotated =
	    p->printpanel.tmp_enscript_info.rotated; 
	p->printpanel.enscript_info.norotate =
	    p->printpanel.tmp_enscript_info.norotate; 
	p->printpanel.enscript_info.gaudy =
	    p->printpanel.tmp_enscript_info.gaudy; 
	p->printpanel.enscript_info.lpt_mode =
	    p->printpanel.tmp_enscript_info.lpt_mode; 
	p->printpanel.enscript_info.no_header =
	    p->printpanel.tmp_enscript_info.no_header; 
	p->printpanel.enscript_info.truncate_lines =
	    p->printpanel.tmp_enscript_info.truncate_lines; 
	p->printpanel.enscript_info.ignore_binary =
	    p->printpanel.tmp_enscript_info.ignore_binary; 
	p->printpanel.enscript_info.report_missing_chars =
	    p->printpanel.tmp_enscript_info.report_missing_chars; 
	p->printpanel.enscript_info.quiet_mode =
	    p->printpanel.tmp_enscript_info.quiet_mode; 
	p->printpanel.enscript_info.no_burst_page =
	    p->printpanel.tmp_enscript_info.no_burst_page; 
	break;
    case PSROFF:
	p->printpanel.roff_info.font_dir =
	    p->printpanel.tmp_roff_info.font_dir; 
    case PTROFF:
	p->printpanel.roff_info.font = p->printpanel.tmp_roff_info.font;
	p->printpanel.roff_info.options =
	    p->printpanel.tmp_roff_info.options ;
	break;
    case PS4014:
	p->printpanel.ps4014_info.scale =
	    p->printpanel.tmp_ps4014_info.scale; 
	p->printpanel.ps4014_info.left =
	    p->printpanel.tmp_ps4014_info.left; 
	p->printpanel.ps4014_info.bottom =
	    p->printpanel.tmp_ps4014_info.bottom; 
	p->printpanel.ps4014_info.width =
	    p->printpanel.tmp_ps4014_info.width; 
	p->printpanel.ps4014_info.height =
	    p->printpanel.tmp_ps4014_info.height; 
	p->printpanel.ps4014_info.portrait =
	    p->printpanel.tmp_ps4014_info.portrait; 
	p->printpanel.ps4014_info.cr_no_lf =
	    p->printpanel.tmp_ps4014_info.cr_no_lf; 
	p->printpanel.ps4014_info.lf_no_cr =
	    p->printpanel.tmp_ps4014_info.lf_no_cr; 
	p->printpanel.ps4014_info.margin_2 =
	    p->printpanel.tmp_ps4014_info.margin_2; 
	break;
    case PSPLT:
	p->printpanel.plot_profile = p->printpanel.tmp_plot_profile;
	break;
    case PS630:
	p->printpanel.ps630_info.bold_font =
	    p->printpanel.tmp_ps630_info.bold_font; 
	p->printpanel.ps630_info.body_font =
	    p->printpanel.tmp_ps630_info.body_font; 
	p->printpanel.ps630_info.pitch =
	    p->printpanel.tmp_ps630_info.pitch; 
	break;
    case NONE:
    default:
	break;
    }
}

static void SetFeatures(p)
    PrintPanelWidget p;
{
    int i;
    int j;
    char **ptr;

    p->printpanel.use_pslpr = FALSE;
    for (i = 0; i <= p->printpanel.num_features; i++) {
	for (j = 0; j < p->printpanel.feature_info[i].num; j++) {
	    if (p->printpanel.feature_info[i].tmpvalue[j]) {
		p->printpanel.feature_info[i].value[j] =
		    p->printpanel.feature_info[i].tmpvalue[j]; 
	    }
	}
    }
}

static void SetDraftFields(p)
    PrintPanelWidget p;
{
    p->printpanel.psdraft_info.font = p->printpanel.tmp_psdraft_info.font;
    p->printpanel.psdraft_info.size = p->printpanel.tmp_psdraft_info.size;
    p->printpanel.psdraft_info.angle = p->printpanel.tmp_psdraft_info.angle;
    p->printpanel.psdraft_info.gray = p->printpanel.tmp_psdraft_info.gray;
    p->printpanel.psdraft_info.xpos = p->printpanel.tmp_psdraft_info.xpos;
    p->printpanel.psdraft_info.ypos = p->printpanel.tmp_psdraft_info.ypos;
    p->printpanel.psdraft_info.draftstring =
	p->printpanel.tmp_psdraft_info.draftstring; 
    p->printpanel.psdraft_info.outline =
	p->printpanel.tmp_psdraft_info.outline;
}
    

static void DraftOKCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    SetDraftFields(p);
    XtUnmanageChild(p->printpanel.draft_form_child);
}

static void DraftApplyCallback(w, p, call_data) 
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    SetDraftFields(p);
}

static void DraftResetCallback(w, p, call_data) 
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
}

static void SetSpecialOptions(p)
    PrintPanelWidget p;
{
    p->printpanel.use_psnup = p->printpanel.tmp_psnup;
    if (p->printpanel.use_psnup) {
	p->printpanel.nup_info.rows = p->printpanel.tmp_nup_info.rows; 
	p->printpanel.nup_info.columns =
	    p->printpanel.tmp_nup_info.columns; 
	p->printpanel.nup_info.gaudy =
	    p->printpanel.tmp_nup_info.gaudy; 
	p->printpanel.nup_info.rotated =
	    p->printpanel.tmp_nup_info.rotated; 
    }
    p->printpanel.use_psdraft = p->printpanel.tmp_psdraft;
    p->printpanel.pslpr_info.showpage =
	p->printpanel.tmp_pslpr_info.showpage;
    p->printpanel.pslpr_info.landscape =
	p->printpanel.tmp_pslpr_info.landscape;
    p->printpanel.pslpr_info.squeeze =
	p->printpanel.tmp_pslpr_info.squeeze;
    p->printpanel.pslpr_info.overtranslate =
	p->printpanel.tmp_pslpr_info.overtranslate;
}
    

static void SecondaryOKCallback(w, mycalldata, call_data)
    Widget w;
    SecondaryData *mycalldata;
    XmAnyCallbackStruct *call_data;
{
    PrintPanelWidget p = (PrintPanelWidget)
	XtParent(XtParent(XtParent(XtParent(w)))); 

    if (mycalldata->which_widget ==  p->printpanel.generic_panel_child) {
	SetOptions(p, mycalldata->which_filter);
    }
    if (mycalldata->which_widget == p->printpanel.special_form_child) {
	SetSpecialOptions(p);
    }
    if (mycalldata->which_widget == p->printpanel.feature_panel_child) {
	SetFeatures(p);
    }
    if (mycalldata->which_widget == p->printpanel.fax_panel_child) {
	SetFax(p);
    }
    XtUnmanageChild(mycalldata->which_widget);
}    

static void SecondaryApplyCallback(w, mycalldata, call_data)
    Widget w;
    SecondaryData *mycalldata;
    XmAnyCallbackStruct *call_data;
{
    PrintPanelWidget p = (PrintPanelWidget)
	XtParent(XtParent(XtParent(XtParent(w)))); 

    if (mycalldata->which_widget == p->printpanel.generic_panel_child) {
	SetOptions(p, mycalldata->which_filter);
    }
    if (mycalldata->which_widget == p->printpanel.special_form_child) {
	SetSpecialOptions(p);
    }
    if (mycalldata->which_widget == p->printpanel.feature_panel_child) {
	SetFeatures(p);
    }
    if (mycalldata->which_widget == p->printpanel.fax_panel_child) {
	SetFax(p);
    }
}

static void SetFaxFields(p)
    PrintPanelWidget p;
{
    if (p->printpanel.tmp_fax_data.name[0] != '\0')
	XmTextFieldSetString(p->printpanel.fax_rec_field_child,
			     p->printpanel.tmp_fax_data.name);
    if (p->printpanel.tmp_fax_data.phone[0] != '\0')
	XmTextFieldSetString(p->printpanel.fax_num_field_child,
			     p->printpanel.tmp_fax_data.phone);

}

static void SetFax(p)
    PrintPanelWidget p;
{
    strcpy(p->printpanel.fax_data.key, p->printpanel.tmp_fax_data.key);
    strcpy(p->printpanel.fax_data.name, p->printpanel.tmp_fax_data.name);
    strcpy(p->printpanel.fax_data.phone, p->printpanel.tmp_fax_data.phone);
    strcpy(p->printpanel.fax_data.passwd, p->printpanel.tmp_fax_data.passwd);
    p->printpanel.fax_data.filename = p->printpanel.tmp_fax_data.filename;
    p->printpanel.fax_data.sendps = p->printpanel.tmp_fax_data.sendps;
    p->printpanel.fax_data.save = p->printpanel.tmp_fax_data.save;

    XmTextFieldSetString(p->printpanel.fax_text_child,
			 p->printpanel.fax_data.phone); 

}    
    
static void PhoneOKCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    SetFaxFields(p);
    XtUnmanageChild(p->printpanel.fax_phone_panel_child);
}

static void PhoneApplyCallback(w, p, call_data)
    Widget w;
    PrintPanelWidget p;
    XmAnyCallbackStruct *call_data;
{
    SetFaxFields(p);
}

static void SetPitch10(w, callData)
    Widget w;
    XmAnyCallbackStruct *callData;
{
    PrintPanelWidget p = (PrintPanelWidget)
	XtParent(XtParent(XtParent(XtParent(XtParent(XtParent(w))))));
    p->printpanel.tmp_ps630_info.pitch = "10";
}

static void SetPitch12(w, callData)
    Widget w;
    XmAnyCallbackStruct *callData;
{
    PrintPanelWidget p = (PrintPanelWidget)
	XtParent(XtParent(XtParent(XtParent(XtParent(XtParent(w))))));
    p->printpanel.tmp_ps630_info.pitch = "12";
}

static void SetPitch15(w, callData)
    Widget w;
    XmAnyCallbackStruct *callData;
{
    PrintPanelWidget p = (PrintPanelWidget)
	XtParent(XtParent(XtParent(XtParent(XtParent(XtParent(w))))));
    p->printpanel.tmp_ps630_info.pitch = "15";
}

static void PickManyCallback(w, mycalldata, callData)
    Widget w;
    FeatureValueData *mycalldata;
    XmToggleButtonCallbackStruct *callData;
{
    PrintPanelWidget p = (PrintPanelWidget) mycalldata->head;
    int i, j;
    int ndex = mycalldata->which_option;
    OptionData *op;
    OptionData *noopt;

    op =
	&p->printpanel.features.uis[mycalldata->which_key].options[mycalldata->which_option];


    if (callData->set) {
	p->printpanel.feature_info[mycalldata->which_feature].tmpvalue[ndex]
	    = mycalldata->choice;

	for (i = 0; i < op->num_constraints; i++) {
	    if (op->constraints[i].no_key->type == BOOL) {
		XtSetSensitive(op->constraints[i].no_key->imp, FALSE);
	    }
	    else {
		if (op->constraints[i].no_option) {
		    noopt = op->constraints[i].no_option;
		    if (noopt->imp) {
			XtSetSensitive(noopt->imp, FALSE);
			XmToggleButtonSetState(noopt->imp, FALSE, TRUE);
		    }
		}
		else {
		    /* all options are constrained */
		    for (j = 0; j < op->constraints[i].no_key->num_options;
			 j++) {
			noopt = &op->constraints[i].no_key->options[j];
			if (noopt->imp) {
			    XtSetSensitive(noopt->imp, FALSE);
			    XmToggleButtonSetState(noopt->imp, FALSE, TRUE);
			}
		    }
		}
	    }
	}
    }
    else {
	p->printpanel.feature_info[mycalldata->which_feature].tmpvalue[ndex]
	    = NULL; 
	for (i = 0; i < op->num_constraints; i++) {
	    if (op->constraints[i].no_key->type == BOOL) {
		if (op->constraints[i].no_key->imp)
		    XtSetSensitive(op->constraints[i].no_key->imp, TRUE);
	    }
	    else {
		if (op->constraints[i].no_option) {
		    if (op->constraints[i].no_option->imp &&
			!op->constraints[i].no_option->gray) 
			XtSetSensitive(op->constraints[i].no_option->imp,
				       TRUE);
		}
		else {
		    /* all are constrained */
		    for (j = 0; j < op->constraints[i].no_key->num_options;
			 j++) {
			noopt = &op->constraints[i].no_key->options[j];
			if (noopt->imp && !noopt->gray) {
			    XtSetSensitive(noopt->imp, TRUE);
			}
		    }
		}
	    }
	}
    }
}
		
static void SetFeatureValue(w, mycalldata, callData)
    Widget w;
    FeatureValueData *mycalldata;
    XmToggleButtonCallbackStruct *callData;
{
    int i, j;
    PrintPanelWidget p = (PrintPanelWidget) mycalldata->head;
    OptionData *op;
    OptionData *noopt;

    op =
	&p->printpanel.features.uis[mycalldata->which_key].options[mycalldata->which_option];

    if (callData->set) {
	p->printpanel.feature_info[mycalldata->which_feature].tmpvalue[0] =
	    mycalldata->choice;

	for (i = 0; i < op->num_constraints; i++) {
	    if (op->constraints[i].no_key->type == BOOL) {
		if (op->constraints[i].no_key->imp)
		    XtSetSensitive(op->constraints[i].no_key->imp, FALSE);
	    }
	    else {
		if (op->constraints[i].no_option) {
		    noopt = op->constraints[i].no_option;
		    if (noopt->imp) {
			XtSetSensitive(noopt->imp, FALSE);
			XmToggleButtonSetState(noopt->imp, FALSE, TRUE);
		    }
		}
		else {
		    /* all options are constrained */
		    for (j = 0; j < op->constraints[i].no_key->num_options;
			 j++) {
			noopt = &op->constraints[i].no_key->options[j];
			if (noopt->imp) {
			    XtSetSensitive(noopt->imp, FALSE);
			    XmToggleButtonSetState(noopt->imp, FALSE, TRUE);
			}
		    }
		}
	    }
	}
    }
    else {
	p->printpanel.feature_info[mycalldata->which_feature].tmpvalue[0] =
	    NULL; 
	for (i = 0; i < op->num_constraints; i++) {
	    if (op->constraints[i].no_key->type == BOOL) {
		if (op->constraints[i].no_key->imp)
		    XtSetSensitive(op->constraints[i].no_key->imp, TRUE);
	    }
	    else {
		if (op->constraints[i].no_option) {
		    if (op->constraints[i].no_option->imp &&
			!op->constraints[i].no_option->gray) 
			XtSetSensitive(op->constraints[i].no_option->imp,
				       TRUE);
		}
		else {
		    /* all are constrained */
		    for (j = 0; j < op->constraints[i].no_key->num_options;
			 j++) {
			noopt = &op->constraints[i].no_key->options[j];
			if (noopt->imp && !noopt->gray) {
			    XtSetSensitive(noopt->imp, TRUE);
			}
		    }
		}
	    }
	}
    }
}

static void SetToggleFeature(w, mycalldata, call_data)
    Widget w;
    FeatureValueData *mycalldata;
    XmToggleButtonCallbackStruct *call_data;
{
    PrintPanelWidget p = (PrintPanelWidget) mycalldata->head;
    int i, j;
    UIData *uiptr;
    OptionData *op;
    OptionData *noopt;

    uiptr = &p->printpanel.features.uis[mycalldata->which_key];
    op = &uiptr->options[!mycalldata->which_option];

    if (call_data->set) {
	p->printpanel.feature_info[mycalldata->which_feature].tmpvalue[0] =
	    truevalue;
	for (i = 0; i < op->num_constraints; i++) {
	    if (op->constraints[i].no_key->type == BOOL) {
		XtSetSensitive(op->constraints[i].no_key->imp, FALSE);
	    }
	    else {
		if (op->constraints[i].no_option) {
		    noopt = op->constraints[i].no_option;
		    if (noopt->imp) {
			XtSetSensitive(noopt->imp, FALSE);
			XmToggleButtonSetState(noopt->imp, FALSE, TRUE);
		    }
		}
		else {
		    /* all options are constrained */
		    for (j = 0; j < op->constraints[i].no_key->num_options;
			 j++) {
			noopt = &op->constraints[i].no_key->options[j];
			if (noopt->imp) {
			    XtSetSensitive(noopt->imp, FALSE);
			    XmToggleButtonSetState(noopt->imp, FALSE, TRUE);
			}
		    }
		}
	    }
	}
    }
    else {
	p->printpanel.feature_info[mycalldata->which_feature].tmpvalue[0] =
	    NULL;
	for (i = 0; i < op->num_constraints; i++) {
	    if (op->constraints[i].no_key->type == BOOL) {
		if (op->constraints[i].no_key->imp)
		    XtSetSensitive(op->constraints[i].no_key->imp, TRUE);
	    }
	    else {
		if (op->constraints[i].no_option) {
		    if (op->constraints[i].no_option->imp &&
			!op->constraints[i].no_option->gray)
			XtSetSensitive(op->constraints[i].no_option->imp,
				       TRUE);
		}
		else {
		    /* all are constrained */
		    for (j = 0; j < op->constraints[i].no_key->num_options;
			 j++) {
			noopt = &op->constraints[i].no_key->options[j];
			if (!noopt->gray && noopt->imp) {
			    XtSetSensitive(noopt->imp, TRUE);
			}
		    }
		}
	    }
	}
    }
}

static void CreateFSB(p)
    PrintPanelWidget p;
{
    Widget tmp;
    int i;
    Arg args[10];

    i = 0;
    XtSetArg(args[i], XmNdialogTitle, CS("Input File Selection")); i++;
    XtSetArg(args[i], XmNwidth, 325); i++;
    XtSetArg(args[i], XmNresizePolicy, XmRESIZE_NONE); i++;
    p->printpanel.input_file_selection_child =
	XmCreateFileSelectionDialog(p->printpanel.panel_child,
				    "Input File", args, i);

    tmp =
	XmFileSelectionBoxGetChild(p->printpanel.input_file_selection_child,
				   XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(tmp);

    p->printpanel.input_sb_data.head = (Widget) p;
    p->printpanel.input_sb_data.which_sb =
	p->printpanel.input_file_selection_child; 
    p->printpanel.input_sb_data.which_field =
	p->printpanel.open_field_child;
    p->printpanel.input_sb_data.name = &p->printpanel.input_file;
    XtAddCallback(p->printpanel.input_file_selection_child, XmNokCallback,
		  FileOKCallback, &p->printpanel.input_sb_data); 
    XtAddCallback(p->printpanel.input_file_selection_child,
		  XmNcancelCallback, FileCancelCallback,
		  &p->printpanel.input_sb_data);  

    i = 0;
    XtSetArg(args[i], XmNdialogTitle, CS("Output File Selection")); i++;
    XtSetArg(args[i], XmNwidth, 325); i++;
    XtSetArg(args[i], XmNresizePolicy, XmRESIZE_NONE); i++;
    p->printpanel.output_file_selection_child =
	XmCreateFileSelectionDialog(p->printpanel.panel_child,
				    "Save to File", args, i);
    tmp =
	XmFileSelectionBoxGetChild(p->printpanel.output_file_selection_child,
				   XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(tmp);

    p->printpanel.output_sb_data.head = (Widget) p;
    p->printpanel.output_sb_data.which_sb =
	p->printpanel.output_file_selection_child; 
    p->printpanel.output_sb_data.which_field = p->printpanel.save_text_child;
    p->printpanel.output_sb_data.name = &p->printpanel.output_file;
    XtAddCallback(p->printpanel.output_file_selection_child, XmNokCallback,
		  FileOKCallback, &p->printpanel.output_sb_data); 
    XtAddCallback(p->printpanel.output_file_selection_child,
		  XmNcancelCallback, FileCancelCallback,
		  &p->printpanel.output_sb_data);  
}

static void CreateDraftPanel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];

    i = 0;
    XtSetArg(args[i], XmNautoUnmanage, FALSE); i++;
    XtSetArg(args[i], XmNdialogTitle, CS("Psdraft Options")); i++;
    XtSetArg(args[i], XmNwidth, 360); i++;
    XtSetArg(args[i], XmNheight, 280); i++;
    p->printpanel.draft_form_child =
	XmCreateFormDialog(p->printpanel.special_form_child, "draftForm",
			   args, i);

    CreateStandardButtons(p->printpanel.draft_form_child,
			  &p->printpanel.draft_ok_button_child,
			  &p->printpanel.draft_apply_button_child,
			  &p->printpanel.draft_reset_button_child,
			  &p->printpanel.draft_cancel_button_child,
			  &p->printpanel.draft_separator_child,
			  "draft");

    XtAddCallback(p->printpanel.draft_ok_button_child, XmNactivateCallback,
		  DraftOKCallback, p);
    XtAddCallback(p->printpanel.draft_apply_button_child,
		  XmNactivateCallback, DraftApplyCallback, p);
    XtAddCallback(p->printpanel.draft_reset_button_child,
		  XmNactivateCallback, DraftResetCallback, p);
    XtAddCallback(p->printpanel.draft_cancel_button_child,
		  XmNactivateCallback, SecondaryCancelCallback,
		  &p->printpanel.draft_form_child);

    p->printpanel.draft_angle_label_child =
	XtVaCreateManagedWidget("draftAngleLabel", xmLabelWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_separator_child,
				XmNbottomOffset, 10, 
				XmNlabelString, CS("Rotation (degrees)"),
				NULL);

    p->printpanel.draft_angle_field_child =
	XtVaCreateManagedWidget("draftAngleField", xmTextFieldWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_angle_label_child,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_separator_child,
				XmNbottomOffset, 10,
				XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.draft_angle_field_child,
		  XmNvalueChangedCallback, RealTextFieldChange,
		  &p->printpanel.tmp_psdraft_info.angle); 
    XmTextFieldSetString(p->printpanel.draft_angle_field_child, "90");

    p->printpanel.draft_gray_label_child =
	XtVaCreateManagedWidget("draftGrayLabel", xmLabelWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_angle_field_child,
				XmNbottomOffset, 10, 
				XmNlabelString, CS("Gray level (0-1)"),
				NULL);

    p->printpanel.draft_gray_field_child =
	XtVaCreateManagedWidget("draftGrayField", xmTextFieldWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_gray_label_child,
				XmNleftOffset, 23, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_angle_field_child,
				XmNbottomOffset, 10,
				XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.draft_gray_field_child,
		  XmNvalueChangedCallback, RealTextFieldChange,
		  &p->printpanel.tmp_psdraft_info.gray); 
    XmTextFieldSetString(p->printpanel.draft_gray_field_child, "0");

    p->printpanel.draft_outline_toggle_child =
	XtVaCreateManagedWidget("draftOutlineToggle",
				xmToggleButtonWidgetClass,
				p->printpanel.draft_form_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.draft_angle_field_child,
				XmNbottomOffset, 10, XmNleftAttachment,
				XmATTACH_WIDGET, XmNleftWidget,
				p->printpanel.draft_gray_field_child,
				XmNleftOffset, 10, 
				XmNlabelString, CS("Outlined letters"),
				NULL);
    XtAddCallback(p->printpanel.draft_outline_toggle_child,
		  XmNvalueChangedCallback, ToggleCallback,
		  &p->printpanel.tmp_psdraft_info.outline); 

    p->printpanel.draft_x_label_child =
	XtVaCreateManagedWidget("draftXLabel", xmLabelWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_gray_field_child,
				XmNbottomOffset, 10,
				XmNlabelString, CS("X (pts)"),
				NULL);

    p->printpanel.draft_x_field_child =
	XtVaCreateManagedWidget("draftXField", xmTextFieldWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_x_label_child,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_gray_field_child,
				XmNbottomOffset, 10, XmNwidth, 50, NULL); 

    XtAddCallback(p->printpanel.draft_x_field_child,
		  XmNvalueChangedCallback, RealTextFieldChange,
		  &p->printpanel.tmp_psdraft_info.xpos); 
    XmTextFieldSetString(p->printpanel.draft_x_field_child, "575");

    p->printpanel.draft_y_label_child =
	XtVaCreateManagedWidget("draftYLabel", xmLabelWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_x_field_child, 
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_gray_field_child,
				XmNbottomOffset, 10, 
				XmNlabelString, CS("Y (pts)"),
				NULL);

    p->printpanel.draft_y_field_child =
	XtVaCreateManagedWidget("draftYField", xmTextFieldWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_y_label_child,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_gray_field_child,
				XmNbottomOffset, 10, XmNwidth, 50, NULL); 

    XtAddCallback(p->printpanel.draft_y_field_child,
		  XmNvalueChangedCallback, RealTextFieldChange,
		  &p->printpanel.tmp_psdraft_info.ypos); 
    XmTextFieldSetString(p->printpanel.draft_y_field_child, "300");

    p->printpanel.draft_font_label_child =
	XtVaCreateManagedWidget("draftFontLabel", xmLabelWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_x_field_child,
				XmNbottomOffset, 10, 
				XmNlabelString, CS("Font"),
				NULL);
    p->printpanel.draft_font_field_child =
	XtVaCreateManagedWidget("draftFontField", xmTextFieldWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_font_label_child,
				XmNleftOffset, 20, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_x_field_child,
				XmNbottomOffset, 10, NULL);

    XtAddCallback(p->printpanel.draft_font_field_child,
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_psdraft_info.font); 
    XmTextFieldSetString(p->printpanel.draft_font_field_child,
			 "Times-Roman"); 

    p->printpanel.draft_size_label_child =
	XtVaCreateManagedWidget("draftSizeLabel", xmLabelWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_font_field_child, 
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_x_field_child,
				XmNbottomOffset, 10, 
				XmNlabelString, CS("Size"),
				NULL);

    p->printpanel.draft_size_field_child =
	XtVaCreateManagedWidget("draftSizeField", xmTextFieldWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_size_label_child,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_x_field_child,
				XmNbottomOffset, 10, XmNwidth, 40, NULL); 

    XtAddCallback(p->printpanel.draft_size_field_child,
		  XmNvalueChangedCallback, RealTextFieldChange,
		  &p->printpanel.tmp_psdraft_info.size); 
    XmTextFieldSetString(p->printpanel.draft_size_field_child, "30");

    p->printpanel.draft_string_label_child =
	XtVaCreateManagedWidget("draftStringLabel", xmLabelWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_font_field_child,
				XmNbottomOffset, 10, 
				XmNlabelString, CS("String"),
				NULL);

    p->printpanel.draft_string_field_child =
	XtVaCreateManagedWidget("draftStringField", xmTextFieldWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.draft_string_label_child,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.draft_font_field_child,
				XmNbottomOffset, 10,
				XmNrightAttachment, XmATTACH_FORM,
				XmNrightOffset, 20, NULL);

    XtAddCallback(p->printpanel.draft_string_field_child,
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_psdraft_info.draftstring); 
    XmTextFieldSetString(p->printpanel.draft_string_field_child, "DRAFT");
/*
    p->printpanel.draft_label_child =
	XtVaCreateManagedWidget("draftLabel", xmLabelWidgetClass,
				p->printpanel.draft_form_child,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, 35, XmNtopAttachment,
				XmATTACH_FORM, XmNtopOffset, 20,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.draft_string_field_child,
				XmNbottomOffset, 20, XmNlabelString,
				CS("Psdraft Options"), NULL);
*/				
}

	     


static void CreateSpecialFeatures(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];

    i = 0;
    XtSetArg(args[i], XmNheight, 240); i++;
    XtSetArg(args[i], XmNwidth, 380); i++;
    XtSetArg(args[i], XmNautoUnmanage, FALSE); i++;
    XtSetArg(args[i], XmNdialogTitle, CS("Special Features")); i++;
    p->printpanel.special_form_child =
	XmCreateFormDialog(p->printpanel.panel_child,
			   "specialForm", args, i);

    p->printpanel.special_call_data.which_widget =
	p->printpanel.special_form_child;
    p->printpanel.special_call_data.which_filter = NONE;

    CreateStandardButtons(p->printpanel.special_form_child,
			  &p->printpanel.special_ok_button_child,
			  &p->printpanel.special_apply_button_child,
			  &p->printpanel.special_reset_button_child,
			  &p->printpanel.special_cancel_button_child,
			  &p->printpanel.special_separator_child,
			  "special"); 

    XtAddCallback(p->printpanel.special_ok_button_child, XmNactivateCallback,
		  SecondaryOKCallback, &p->printpanel.special_call_data);
    XtAddCallback(p->printpanel.special_apply_button_child,
		  XmNactivateCallback, SecondaryApplyCallback,
		  &p->printpanel.special_call_data);
    XtAddCallback(p->printpanel.special_reset_button_child,
		  XmNactivateCallback, SecondaryResetCallback, p);
    XtAddCallback(p->printpanel.special_cancel_button_child,
		  XmNactivateCallback, SecondaryCancelCallback,
		  &p->printpanel.special_form_child);

    p->printpanel.special_gaudy_toggle_child =
	XtVaCreateManagedWidget("specialGaudyToggle",
				xmToggleButtonWidgetClass,
				p->printpanel.special_form_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.special_separator_child,
				XmNbottomOffset, 10,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10,
				XmNlabelString, CS("Gaudy"), NULL);

    XtAddCallback(p->printpanel.special_gaudy_toggle_child,
		   XmNvalueChangedCallback, ToggleCallback,
		   &(p->printpanel.tmp_nup_info.gaudy)); 
    XtSetSensitive(p->printpanel.special_gaudy_toggle_child, FALSE);

    p->printpanel.special_rotate_toggle_child =
	XtVaCreateManagedWidget("specialRotateToggle",
				 xmToggleButtonWidgetClass,
				 p->printpanel.special_form_child,
				 XmNbottomAttachment, XmATTACH_WIDGET,
				 XmNbottomWidget,
				 p->printpanel.special_separator_child,
				 XmNbottomOffset, 10,
				 XmNleftAttachment, XmATTACH_WIDGET,
				 XmNleftWidget,
				 p->printpanel.special_gaudy_toggle_child,
				 XmNlabelString, CS("Rotated"), NULL);

    XtAddCallback(p->printpanel.special_rotate_toggle_child,
		   XmNvalueChangedCallback, ToggleCallback,
		   &(p->printpanel.tmp_nup_info.rotated)); 
    XtSetSensitive(p->printpanel.special_rotate_toggle_child, FALSE);

    p->printpanel.special_nup_column_label_child =
	XtVaCreateManagedWidget("columnLabel",
				 xmLabelWidgetClass,
				 p->printpanel.special_form_child,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNleftOffset, 10,
				 XmNbottomAttachment, XmATTACH_WIDGET,
				 XmNbottomWidget,
				 p->printpanel.special_gaudy_toggle_child,
				 XmNbottomOffset, 25,
				 XmNlabelString, CS("Columns"), NULL);

    XtSetSensitive(p->printpanel.special_nup_column_label_child, FALSE); 

    p->printpanel.special_nup_column_field_child =
	XtVaCreateManagedWidget("columnField",
				 xmTextFieldWidgetClass,
				 p->printpanel.special_form_child,
				 XmNleftAttachment, XmATTACH_WIDGET,
				 XmNleftWidget,
				 p->printpanel.special_nup_column_label_child,
				 XmNbottomAttachment, XmATTACH_WIDGET,
				 XmNbottomWidget,
				 p->printpanel.special_gaudy_toggle_child,
				 XmNbottomOffset, 20, XmNwidth, 40, NULL); 

    XmTextFieldSetString(p->printpanel.special_nup_column_field_child, "2");
    XtAddCallback(p->printpanel.special_nup_column_field_child,
		  XmNvalueChangedCallback, IntTextFieldChange,
		  &(p->printpanel.tmp_nup_info.columns)); 
    XtSetSensitive(p->printpanel.special_nup_column_field_child, FALSE);

    p->printpanel.special_nup_row_label_child =
	XtVaCreateManagedWidget("rowLabel",
				xmLabelWidgetClass,
				p->printpanel.special_form_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.special_nup_column_field_child,
				XmNbottomOffset, 25, XmNlabelString,
				CS("Rows"), NULL);

    XtSetSensitive(p->printpanel.special_nup_row_label_child, FALSE);

    p->printpanel.special_nup_row_field_child =
	XtVaCreateManagedWidget("rowField",
				xmTextFieldWidgetClass,
				p->printpanel.special_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.special_nup_row_label_child,
				XmNleftOffset, 15,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.special_nup_column_field_child,
				XmNbottomOffset, 20, XmNwidth, 40, NULL);

    XmTextFieldSetString(p->printpanel.special_nup_row_field_child, "2");
    XtAddCallback(p->printpanel.special_nup_row_field_child,
		  XmNvalueChangedCallback, IntTextFieldChange,
		  &(p->printpanel.tmp_nup_info.rows)); 
    XtSetSensitive(p->printpanel.special_nup_row_field_child, FALSE);

    p->printpanel.special_draft_toggle_child =
	XtVaCreateManagedWidget("draftToggle", xmToggleButtonWidgetClass,
				p->printpanel.special_form_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget, 
				p->printpanel.special_separator_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.special_rotate_toggle_child,
				XmNleftOffset, 40, XmNbottomOffset, 10,
				XmNlabelString, CS("Add draft string"),
				NULL);
    XtAddCallback(p->printpanel.special_draft_toggle_child,
		  XmNvalueChangedCallback, DraftToggleCallback, p);

    p->printpanel.special_draft_button_child =
	XtVaCreateManagedWidget("draftButton", xmPushButtonWidgetClass,
				p->printpanel.special_form_child,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.special_separator_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.special_draft_toggle_child,
				XmNleftOffset, 10, XmNbottomOffset, 10,
				XmNheight, 30, XmNwidth, 40,
				XmNlabelString, CS("..."),
				NULL);

    XtAddCallback(p->printpanel.special_draft_button_child,
		  XmNactivateCallback, SecondaryCallback,
		  &p->printpanel.draft_form_child);
    XtSetSensitive(p->printpanel.special_draft_button_child, FALSE);

    p->printpanel.special_showpage_toggle_child =
	XtVaCreateManagedWidget("showpageToggle",
				xmToggleButtonWidgetClass,
				p->printpanel.special_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.special_rotate_toggle_child,
				XmNleftOffset, 40, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.special_draft_button_child, 
				XmNbottomOffset, 10, XmNlabelString,
				CS("Add showpage"), NULL);

    XtAddCallback(p->printpanel.special_showpage_toggle_child,
		  XmNvalueChangedCallback, ToggleCallback,
		  &p->printpanel.tmp_pslpr_info.showpage); 

    p->printpanel.special_squeeze_toggle_child =
	XtVaCreateManagedWidget("squeezeToggle",
				xmToggleButtonWidgetClass,
				p->printpanel.special_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.special_rotate_toggle_child,
				XmNleftOffset, 40, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.special_showpage_toggle_child,
				XmNbottomOffset, 10, XmNlabelString,
				CS("ShrinkToFit"), NULL);

    XtAddCallback(p->printpanel.special_squeeze_toggle_child,
		  XmNvalueChangedCallback, ToggleCallback,
		  &p->printpanel.tmp_pslpr_info.squeeze);

    XtSetSensitive(p->printpanel.special_squeeze_toggle_child, FALSE);

    p->printpanel.special_overtranslate_toggle_child =
	XtVaCreateManagedWidget("overtranslateToggle",
				xmToggleButtonWidgetClass,
				p->printpanel.special_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.special_rotate_toggle_child,
				XmNleftOffset, 40, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.special_squeeze_toggle_child,
				XmNbottomOffset, 10, XmNlabelString,
				CS("Overtranslate"), NULL);

    XtAddCallback(p->printpanel.special_overtranslate_toggle_child,
		  XmNvalueChangedCallback, ToggleCallback,
		  &p->printpanel.tmp_pslpr_info.overtranslate); 

    XtSetSensitive(p->printpanel.special_overtranslate_toggle_child, FALSE);

    p->printpanel.special_landscape_toggle_child =
	XtVaCreateManagedWidget("landscapeToggle",
				xmToggleButtonWidgetClass,
				p->printpanel.special_form_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.special_rotate_toggle_child,
				XmNleftOffset, 40, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.special_overtranslate_toggle_child, 
				XmNbottomOffset, 10, XmNlabelString,
				CS("Landscape"), NULL);

    XtAddCallback(p->printpanel.special_landscape_toggle_child,
		  XmNvalueChangedCallback, LandscapeToggleCallback,
		  &p->printpanel.tmp_pslpr_info.landscape); 

    p->printpanel.special_nup_toggle_child =
	XtVaCreateManagedWidget("n-upToggle",
				xmToggleButtonWidgetClass,
				p->printpanel.special_form_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.special_nup_row_label_child,
				XmNbottomOffset, 30,
				XmNlabelString, CS("N-up Printing"), NULL);

    XtAddCallback(p->printpanel.special_nup_toggle_child,
		  XmNvalueChangedCallback, NupToggleCallback, p);
}

static void CreateFeaturePanel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];

    i = 0;
    XtSetArg(args[i], XmNautoUnmanage, FALSE); i++;
    XtSetArg(args[i], XmNdialogTitle, CS("Printer Features")); i++;
    p->printpanel.feature_panel_child =
	XmCreateFormDialog(p->printpanel.panel_child, "featurePanel", args,
			   i); 

    p->printpanel.feature_ok_button_child =
	XtVaCreateManagedWidget("featureOK",
				xmPushButtonWidgetClass, 
				p->printpanel.feature_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("OK"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.feature_apply_button_child =
	XtVaCreateManagedWidget("featureApply",
				xmPushButtonWidgetClass,
				p->printpanel.feature_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.feature_ok_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Apply"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.feature_cancel_button_child =
	XtVaCreateManagedWidget("featureCancel",
				xmPushButtonWidgetClass,
				p->printpanel.feature_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.feature_apply_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Cancel"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);
				

    XtAddCallback(p->printpanel.feature_cancel_button_child,
		  XmNactivateCallback, SecondaryCancelCallback,
		  &p->printpanel.feature_panel_child); 
    XtAddCallback(p->printpanel.feature_ok_button_child, XmNactivateCallback,
		  SecondaryOKCallback, &p->printpanel.feature_callback_data);
    XtAddCallback(p->printpanel.feature_apply_button_child,
		  XmNactivateCallback, SecondaryApplyCallback,
		  &p->printpanel.feature_callback_data); 
    
    p->printpanel.feature_separator_child =
	XtVaCreateManagedWidget("featureSeparator",
				xmSeparatorWidgetClass,
				p->printpanel.feature_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.feature_ok_button_child,
				XmNbottomOffset, 10, NULL);

    i = 0;
    XtSetArg(args[i], XmNscrollingPolicy, XmAUTOMATIC); i++;
    XtSetArg(args[i], XmNheight, 450); i++;
    XtSetArg(args[i], XmNwidth, 250); i++;
    XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(args[i], XmNbottomWidget,
	     p->printpanel.feature_separator_child); i++;
    p->printpanel.feature_window_child =
	XmCreateScrolledWindow(p->printpanel.feature_panel_child,
			       "featureWindow", args, i);
    XtManageChild(p->printpanel.feature_window_child);
    i = 0;
    XtSetArg(args[i], XmNorientation, XmVERTICAL); i++;
    XtSetArg(args[i], XmNpacking, XmPACK_TIGHT); i++;
    p->printpanel.feature_form_child =
	XmCreateForm(p->printpanel.feature_window_child,
			   "featureForm", args, i);
    XtManageChild(p->printpanel.feature_form_child);

    i = 0;
    XtSetArg(args[i], XmNworkWindow,
	     p->printpanel.feature_form_child); i++;
    XtSetValues(p->printpanel.feature_window_child, args, i);
    
    p->printpanel.feature_callback_data.which_widget =
	p->printpanel.feature_panel_child; 
    p->printpanel.feature_callback_data.which_filter = NONE;

}

static void CleanupPhoneEntry(p)
    PrintPanelWidget p;
{
    int i;

    for (i = 0; i < p->printpanel.fax_phone_num_children; i++) {
	XtDestroyWidget(p->printpanel.fax_phone_children[i]);
	p->printpanel.fax_phone_children[i] = NULL;
    }
    p->printpanel.fax_phone_num_children = 0;
}


static void CleanupOldFeatures(p)
    PrintPanelWidget p;
{
    int i;
    int j;

    for (i = 0; i < p->printpanel.features.num_uis; i++) {
	p->printpanel.features.uis[i].imp = 0;
	for (j = 0; j < p->printpanel.features.uis[i].num_options; j++) {
	    p->printpanel.features.uis[i].options[j].imp = 0;
	}
    }
    for (i = p->printpanel.num_feature_children - 1; i >= 0; i--) {
	XtUnmanageChild(p->printpanel.feature_children[i]);
	XtDestroyWidget(p->printpanel.feature_children[i]);
	p->printpanel.feature_children[i] = NULL;
    }
    p->printpanel.num_feature_children = 0;
#if XmVersion <= 1001    
    XtUnrealizeWidget(p->printpanel.feature_panel_child);
#endif    
}

struct FaxPhoneEntry {
    char option[200];
    char value[200];
};

static void BuildPhoneEntry(p)
    PrintPanelWidget p;
{
    struct FaxPhoneEntry entrylist[10];
    int nelist = 0;
    int maxlist = 10;
    int k;
    int j;
    char buf[1024];
    int i;
    Arg args[20];
    int prev;
    char *q;

    
    if (!FindPhoneEntry(p->printpanel.tmp_fax_data.key, entrylist, &nelist,
		       &maxlist, p->printpanel.faxdb)) {
	sprintf(buf, "Couldn't find entry for key %s",
		p->printpanel.tmp_fax_data.key); 
	DisplayError(p->printpanel.message_dialog_child, buf);
	return;
    }

    prev = -1;

    i = 0;
    XtSetArg(args[i], XmNautoUnmanage, FALSE); i++;
    XtSetArg(args[i], XmNdialogTitle, CS("Fax Phonebook Entry")); i++;
    p->printpanel.fax_phone_children[0] =
	XmCreateFormDialog(p->printpanel.fax_phone_panel_child,
			   "entryPanel", args, i);

    p->printpanel.fax_phone_children[1] =
	XtVaCreateManagedWidget("entryOK", xmPushButtonWidgetClass,
				p->printpanel.fax_phone_children[0],
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, 40, XmNbottomAttachment,
				XmATTACH_FORM, 
				XmNlabelString, CS("OK"), NULL);

    XtAddCallback(p->printpanel.fax_phone_children[1], XmNactivateCallback,
		  SecondaryCancelCallback,
		  &p->printpanel.fax_phone_children[0]);

    j = 2;
    prev = 1;

    for (k = nelist - 1; k > -1; k--) {
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNbottomWidget,
		 p->printpanel.fax_phone_children[prev]); 
	i++;
	sprintf(buf, "%s:  ", entrylist[k].option);
	XtSetArg(args[i], XmNlabelString, CS(buf)); i++;
	sprintf(buf, "faxPhone%dLabel", j);
	p->printpanel.fax_phone_children[j++] =
	    XtCreateManagedWidget(buf, xmLabelWidgetClass,
				  p->printpanel.fax_phone_children[0],
				  args, i);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNleftWidget,
		 p->printpanel.fax_phone_children[j-1]); i++;
	XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNbottomWidget,
		 p->printpanel.fax_phone_children[prev]);
	i++;
	XtSetArg(args[i], XmNlabelString, CS(entrylist[k].value)); i++;
	sprintf(buf, "faxPhone%dLabel", j);
	prev = j;
	p->printpanel.fax_phone_children[j++] =
	    XtCreateManagedWidget(buf, xmLabelWidgetClass,
				  p->printpanel.fax_phone_children[0],
				  args, i);

	if (strcmp(entrylist[k].option, "RecipientName") == 0)
	    strcpy(p->printpanel.tmp_fax_data.name, entrylist[k].value);
	if (strcmp(entrylist[k].option, "DialCallee") == 0)
	    strcpy(p->printpanel.tmp_fax_data.phone, entrylist[k].value);

    }
    q = strchr(p->printpanel.tmp_fax_data.key, '|');
    if (q) *q = '\0';
    p->printpanel.fax_phone_num_children = j;
    XtManageChild(p->printpanel.fax_phone_children[0]);
}

									
static void BuildFeatures(p)
    PrintPanelWidget p;
{
    int i, j, k;
    Arg args[20];
    char buf[1024];
    int def;
    int pull;
    int lastone = -1;
    char *q;
    int test;
    PrintPanelPart *h;


    h = &p->printpanel;
    h->num_feature_children = 0;
    p->printpanel.num_features = -1;

    for (j = 0; j < p->printpanel.features.num_uis; j++) {
	def = -1;
	if (strcmp(p->printpanel.features.uis[j].keyword, "*PageRegion") == 0)
	    continue;
	if (p->printpanel.features.uis[j].display == FALSE)
	    continue;
	q = p->printpanel.features.uis[j].keyword;
	q++;
	p->printpanel.feature_info[++p->printpanel.num_features].name = q;
	switch (p->printpanel.features.uis[j].type) {
	case PICKONE:
	    /* option menu */
	    sprintf(buf, "radio%d", j);
	    i = 0;
	    XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
	    if (lastone == -1) {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
	    }
	    else {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
		XtSetArg(args[i], XmNbottomWidget,
			 h->feature_children[lastone]); i++;
	    }
	    XtSetArg(args[i], XmNradioAlwaysOne, FALSE); i++;
	    sprintf(buf, "%sRadioBox",
		    p->printpanel.feature_info[p->printpanel.num_features].name);  
	    h->feature_children[h->num_feature_children] =
		XmCreateRadioBox(h->feature_form_child, buf, args, i);
	    pull = h->num_feature_children;
	    XtManageChild(h->feature_children[h->num_feature_children]);
	    h->features.uis[j].imp =
		h->feature_children[h->num_feature_children]; 
	    lastone = h->num_feature_children;
	    h->num_feature_children++;
	    p->printpanel.feature_info[p->printpanel.num_features].num = 1;
	    for (k = 0; k < p->printpanel.features.uis[j].num_options; k++) {
		i = 0;
		if (h->features.uis[j].options[k].name_tran[0] != '\0')
		    q = h->features.uis[j].options[k].name_tran;
		else
		    q = h->features.uis[j].options[k].name;
		h->feature_children[h->num_feature_children] =
		    XmCreateToggleButton(h->feature_children[pull], q,
					       args, i);
		h->features.uis[j].options[k].imp =
		    h->feature_children[h->num_feature_children]; 
		h->choice_callback_data[h->num_feature_children].choice =
		    p->printpanel.features.uis[j].options[k].name;  
		h->choice_callback_data[h->num_feature_children].which_feature
		    = p->printpanel.num_features;
		h->choice_callback_data[h->num_feature_children].head =
		    (Widget) p;
		h->choice_callback_data[h->num_feature_children].which_key
		    = j;
		h->choice_callback_data[h->num_feature_children].which_option
		    = k;
		XtAddCallback(h->feature_children[h->num_feature_children],
			      XmNvalueChangedCallback, SetFeatureValue,
			      &h->choice_callback_data[h->num_feature_children]); 
		if (k == p->printpanel.features.uis[j].default_option)
		    def = h->num_feature_children;
		if (h->features.uis[j].options[k].gray) {
		    XtSetSensitive(h->feature_children[h->num_feature_children],
				  FALSE);
		}
		XtManageChild(h->feature_children[h->num_feature_children]);
		h->num_feature_children++;
	    }
	    if (def > -1)
		XmToggleButtonSetState(h->feature_children[def], TRUE,
				       TRUE); 
	    /* create label */
	    i = 0;
	    XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
	    if (lastone == -1) {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
	    }
	    else {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
		XtSetArg(args[i], XmNbottomWidget,
			 h->feature_children[lastone]); i++;
	    }
	    if (p->printpanel.features.uis[j].key_tran[0] != '\0') {
		XtSetArg(args[i], XmNlabelString,
			 CS(p->printpanel.features.uis[j].key_tran)); 
		i++;
	    }
	    else {
		XtSetArg(args[i], XmNlabelString,
			 CS(h->feature_info[h->num_features].name)); 
		i++;
	    }
	    sprintf(buf, "%sLabel",
		    p->printpanel.feature_info[p->printpanel.num_features].name); 
	    h->feature_children[h->num_feature_children] =
		XtCreateWidget(buf, xmLabelWidgetClass, 
			       h->feature_form_child, args, i);
	    XtManageChild(h->feature_children[h->num_feature_children]);
	    lastone = h->num_feature_children;
	    h->num_feature_children++;
	    break;
	case BOOL:
	    /* toggle button */
	    p->printpanel.feature_info[p->printpanel.num_features].num = 1;
	    sprintf(buf, "toggle%d", j);
	    i = 0;
	    XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
	    if (lastone == -1) {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
	    }
	    else {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
		XtSetArg(args[i], XmNbottomWidget,
			 h->feature_children[lastone]); i++;
	    }
	    if (p->printpanel.features.uis[j].key_tran[0] != '\0') {
		XtSetArg(args[i], XmNlabelString,
			 CS(p->printpanel.features.uis[j].key_tran)); 
		i++;
	    }
	    else {
		XtSetArg(args[i], XmNlabelString,
			 CS(h->feature_info[h->num_features].name)); 
		i++;
	    }
	    h->feature_children[h->num_feature_children] =
		XtCreateManagedWidget(buf, xmToggleButtonWidgetClass,
				      h->feature_form_child, args, i);
	    h->features.uis[j].imp =
		h->feature_children[h->num_feature_children]; 
	    def = p->printpanel.features.uis[j].default_option;
	    if (def > -1)  {
		if (strcmp(p->printpanel.features.uis[j].options[def].name,
			   "True") == 0) {
		    XmToggleButtonSetState(h->feature_children[h->num_feature_children], TRUE, TRUE); 
		}
	    }
	    h->choice_callback_data[h->num_feature_children].head =
		(Widget) p;
	    h->choice_callback_data[h->num_feature_children].which_feature
		= p->printpanel.num_features;
	    h->choice_callback_data[h->num_feature_children].which_key = j;
	    for (k = 0; k < h->features.uis[j].num_options; k++) {
		if (strcmp(h->features.uis[j].options[k].name, "False") ==
		    0) {
		    h->choice_callback_data[h->num_feature_children].choice
			= h->features.uis[j].options[k].name;
		    h->choice_callback_data[h->num_feature_children].which_option = k;
		}
		if (h->features.uis[j].options[k].gray) {
		    XtSetSensitive(h->feature_children[h->num_feature_children], 
				   FALSE);
		}
	    }
	    XtAddCallback(h->feature_children[h->num_feature_children],
			  XmNvalueChangedCallback, SetToggleFeature,
			  &h->choice_callback_data[h->num_feature_children]);
	    lastone = h->num_feature_children;
	    h->num_feature_children++;
	    break;
	case PICKMANY:
	    /* rowcolumn */
	    sprintf(buf, "radio%d", j);
	    i = 0;
	    i = 0;
	    XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
	    if (lastone == -1) {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
	    }
	    else {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
		XtSetArg(args[i], XmNbottomWidget,
			 h->feature_children[lastone]); i++;
	    }
	    XtSetArg(args[i], XmNpacking, XmPACK_COLUMN); i++;
	    XtSetArg(args[i], XmNisHomogeneous, TRUE); i++;
	    XtSetArg(args[i], XmNentryClass, xmToggleButtonWidgetClass); i++;
	    pull = h->num_feature_children;
	    h->feature_children[h->num_feature_children] =
		XmCreateRowColumn(h->feature_form_child, buf, args, i);
	    XtManageChild(h->feature_children[h->num_feature_children]);
	    h->features.uis[j].imp =
		h->feature_children[h->num_feature_children]; 
	    lastone = h->num_feature_children;
	    h->num_feature_children++;
	    p->printpanel.feature_info[p->printpanel.num_features].num = 0;
	    for (k = 0; k < h->features.uis[j].num_options; k++) {
		i = 0;
		sprintf(buf, "button%d", k);
		if (h->features.uis[j].options[k].name_tran[0] != '\0') {
		    XtSetArg(args[i], XmNlabelString,
			     CS(h->features.uis[j].options[k].name_tran)); i++;
		}
		else {
		    XtSetArg(args[i], XmNlabelString,
			     CS(h->features.uis[j].options[k].name)); i++;
		}
		h->feature_children[h->num_feature_children] =
		    XtCreateManagedWidget(buf, xmToggleButtonWidgetClass,
					  h->feature_children[pull], args,
					  i);
		h->features.uis[j].options[k].imp =
		    h->feature_children[h->num_feature_children]; 
		h->choice_callback_data[h->num_feature_children].which_key = j;
		h->choice_callback_data[h->num_feature_children].which_option
		    = k;
		h->choice_callback_data[h->num_feature_children].choice =
		    h->features.uis[j].options[k].name; 
		h->choice_callback_data[h->num_feature_children].which_feature
		    = p->printpanel.num_features;
		h->choice_callback_data[h->num_feature_children].head =
		    (Widget) p;
		XtAddCallback(h->feature_children[h->num_feature_children],
			      XmNvalueChangedCallback, PickManyCallback,
			      &h->choice_callback_data[h->num_feature_children]); 
		if (h->features.uis[j].options[k].gray) {
		    XtSetSensitive(h->feature_children[h->num_feature_children],
				  FALSE);
		}
		h->num_feature_children++;
		p->printpanel.feature_info[p->printpanel.num_features].num++;
	    }
	    i = 0;
	    XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
	    if (lastone == -1)  {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
	    }
	    else {
		XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
		XtSetArg(args[i], XmNbottomWidget,
			 h->feature_children[lastone]); i++;
	    }
	    if (p->printpanel.features.uis[j].key_tran[0] != '\0') {
		XtSetArg(args[i], XmNlabelString,
			 CS(p->printpanel.features.uis[j].key_tran)); 
		i++;
	    }
	    else {
		XtSetArg(args[i], XmNlabelString,
			 CS(h->feature_info[h->num_features].name)); 
		i++;
	    }
	    sprintf(buf, "%sLabel", h->features.uis[j].keyword);
	    h->feature_children[h->num_feature_children] =
		XtCreateManagedWidget(buf, xmLabelWidgetClass,
				      h->feature_form_child, args, i);
	    lastone = h->num_feature_children;
	    h->num_feature_children++;
	    break;
	default:
	    break;
	}
    }
}


static void CreateGenericPanel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];

    i = 0;
    XtSetArg(args[i], XmNautoUnmanage, FALSE); i++;
    XtSetArg(args[i], XmNdialogTitle, CS("Filter Options")); i++;
    p->printpanel.generic_panel_child =
	XmCreateFormDialog(p->printpanel.panel_child, 
			   "genericPanel", args, i);

    CreateStandardButtons(p->printpanel.generic_panel_child,
			  &p->printpanel.generic_ok_button_child,
			  &p->printpanel.generic_apply_button_child,
			  &p->printpanel.generic_reset_button_child,
			  &p->printpanel.generic_cancel_button_child,
			  &p->printpanel.generic_separator_child,
			  "generic");
    
/*
    p->printpanel.generic_ok_button_child =
	XtVaCreateManagedWidget("genericOK", 
				xmPushButtonWidgetClass, 
				p->printpanel.generic_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("OK"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.generic_apply_button_child =
	XtVaCreateManagedWidget("genericApply", 
				xmPushButtonWidgetClass,
				p->printpanel.generic_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.generic_ok_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Apply"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.generic_reset_button_child =
	XtVaCreateManagedWidget("genericReset", 
				xmPushButtonWidgetClass,
				p->printpanel.generic_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.generic_apply_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Reset"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.generic_cancel_button_child =
	XtVaCreateManagedWidget("genericCancel", 
				xmPushButtonWidgetClass,
				p->printpanel.generic_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.generic_reset_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Cancel"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);
*/

    XtAddCallback(p->printpanel.generic_cancel_button_child,
		  XmNactivateCallback, SecondaryCancelCallback,
		  &p->printpanel.generic_panel_child); 
    XtAddCallback(p->printpanel.generic_ok_button_child, XmNactivateCallback,
		  SecondaryOKCallback,
		  &p->printpanel.option_callback_data); 
    XtAddCallback(p->printpanel.generic_apply_button_child,
		  XmNactivateCallback, SecondaryApplyCallback,
		  &p->printpanel.option_callback_data);
    XtAddCallback(p->printpanel.generic_reset_button_child,
		  XmNactivateCallback, SecondaryResetCallback, p); 
    
/*
    p->printpanel.generic_separator_child =
	XtVaCreateManagedWidget("genericSeparator", 
				xmSeparatorWidgetClass,
				p->printpanel.generic_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.generic_ok_button_child,
				XmNbottomOffset, 10, NULL);
*/
    p->printpanel.generic_label_child =
	XtVaCreateManagedWidget("genericLabel", 
				xmLabelWidgetClass,
				p->printpanel.generic_panel_child,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, 35, XmNtopAttachment,
				XmATTACH_FORM, XmNtopOffset, 20,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.generic_separator_child,
				XmNbottomOffset, 20, XmNwidth, 100, NULL);
}

static void InitPhoneList(p)
    PrintPanelWidget p;
{
    int i;
    XmString *k_names;

    k_names = (XmString *) XtCalloc(p->printpanel.num_keys,
				    sizeof(XmString));

    for (i = 0; i < p->printpanel.num_keys; i++) {
	k_names[i] = CS(p->printpanel.keys[i]);
    }

    XtVaSetValues(p->printpanel.fax_phone_list_child, XmNitemCount,
		  p->printpanel.num_keys, XmNitems, k_names, NULL);
    
    XtFree(k_names);
}


static void CreatePhonebookPanel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];
    char *faxdb;

    faxdb = getenv("PSFAXDB");
    if (faxdb == NULL) {
	faxdb = getenv("HOME");
	if (faxdb) {
	    strcpy(p->printpanel.faxdb, faxdb);
	    strcat(p->printpanel.faxdb, "/.faxdb");
	}
    }
    else {
	strcpy(p->printpanel.faxdb, faxdb);
    }

    p->printpanel.max_keys = 20;
    p->printpanel.num_keys = 0;
    if ((i = GetPhonebookKeys(p->printpanel.keys,
			      &p->printpanel.num_keys,
			      &p->printpanel.max_keys,
			      p->printpanel.faxdb)) != 0) {
	fprintf(stderr, "Could not get list of keys in fax phonebook\n");
    }

    i = 0;
    XtSetArg(args[i], XmNautoUnmanage, FALSE); i++;
    XtSetArg(args[i], XmNdialogTitle, CS("Fax Phonebook")); i++;
    p->printpanel.fax_phone_panel_child =
	XmCreateFormDialog(p->printpanel.fax_panel_child, "faxPhonePanel",
			   args, i);

    CreateStandardButtons(p->printpanel.fax_phone_panel_child,
			  &p->printpanel.fax_phone_ok_button_child,
			  &p->printpanel.fax_phone_apply_button_child,
			  &p->printpanel.fax_phone_reset_button_child,
			  &p->printpanel.fax_phone_cancel_button_child,
			  &p->printpanel.fax_phone_separator_child,
			  "faxPhone");
    
/*    
    p->printpanel.fax_phone_ok_button_child =
	XtVaCreateManagedWidget("faxOK", 
				xmPushButtonWidgetClass, 
				p->printpanel.fax_phone_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("OK"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.fax_phone_apply_button_child =
	XtVaCreateManagedWidget("faxApply", 
				xmPushButtonWidgetClass,
				p->printpanel.fax_phone_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_phone_ok_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Apply"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.fax_phone_reset_button_child =
	XtVaCreateManagedWidget("faxReset", 
				xmPushButtonWidgetClass,
				p->printpanel.fax_phone_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_phone_apply_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Reset"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.fax_phone_cancel_button_child =
	XtVaCreateManagedWidget("faxCancel", 
				xmPushButtonWidgetClass,
				p->printpanel.fax_phone_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_phone_reset_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Cancel"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNrightOffset, 10, 
				XmNleftOffset, 10, NULL);
*/

    XtAddCallback(p->printpanel.fax_phone_cancel_button_child,
		  XmNactivateCallback, SecondaryCancelCallback,
		  &p->printpanel.fax_phone_panel_child);
    XtAddCallback(p->printpanel.fax_phone_ok_button_child, XmNactivateCallback,
		  PhoneOKCallback, p);
    XtAddCallback(p->printpanel.fax_phone_apply_button_child,
		  XmNactivateCallback, PhoneApplyCallback, p);

/*
    p->printpanel.fax_phone_separator_child =
	XtVaCreateManagedWidget("faxSeparator", 
				xmSeparatorWidgetClass,
				p->printpanel.fax_phone_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_phone_ok_button_child,
				XmNbottomOffset, 10, NULL);
*/

    p->printpanel.fax_phone_label_child =
	XtVaCreateManagedWidget("faxPhoneLabel", xmLabelWidgetClass,
				p->printpanel.fax_phone_panel_child,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNtopOffset, 10, XmNleftOffset, 10,
				XmNlabelString, CS("Keys from phonebook"),
				NULL);
    i = 0;
    XtSetArg(args[i], XmNitemCount, 1); i++;
    XtSetArg(args[i], XmNitems, &CSempty); i++;
    XtSetArg(args[i], XmNheight, 100); i++;
    p->printpanel.fax_phone_list_child =
	XmCreateScrolledList(p->printpanel.fax_phone_panel_child,
			     "phoneList", args, i);

    XtVaSetValues(XtParent(p->printpanel.fax_phone_list_child),
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, p->printpanel.fax_phone_label_child,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, p->printpanel.fax_phone_separator_child,
		  XmNbottomOffset, 10,
		  NULL);

    XtManageChild(p->printpanel.fax_phone_list_child);
    XtAddCallback(p->printpanel.fax_phone_list_child,
		  XmNbrowseSelectionCallback, ListSelection, p);
    InitPhoneList(p);
}
    
static void CreateFaxPanel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];
    Widget tmp;

    i = 0;
    XtSetArg(args[i], XmNautoUnmanage, FALSE); i++;
    XtSetArg(args[i], XmNdialogTitle, CS("Fax")); i++;
    p->printpanel.fax_panel_child =
	XmCreateFormDialog(p->printpanel.panel_child, "faxPanel", args, i);

    p->printpanel.fax_callback_data.which_widget =
	p->printpanel.fax_panel_child; 
    p->printpanel.fax_callback_data.which_filter = NONE;


    CreateStandardButtons(p->printpanel.fax_panel_child,
			  &p->printpanel.fax_ok_button_child,
			  &p->printpanel.fax_apply_button_child,
			  &p->printpanel.fax_reset_button_child,
			  &p->printpanel.fax_cancel_button_child,
			  &p->printpanel.fax_separator_child,
			  "fax");

/*
    p->printpanel.fax_ok_button_child =
	XtVaCreateManagedWidget("faxOK", 
				xmPushButtonWidgetClass, 
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("OK"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.fax_apply_button_child =
	XtVaCreateManagedWidget("faxApply", 
				xmPushButtonWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_ok_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Apply"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.fax_reset_button_child =
	XtVaCreateManagedWidget("faxReset", 
				xmPushButtonWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_apply_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Reset"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

    p->printpanel.fax_cancel_button_child =
	XtVaCreateManagedWidget("faxCancel", 
				xmPushButtonWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_reset_button_child,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNlabelString, CS("Cancel"), XmNwidth, 60,
				XmNheight, 30, XmNbottomOffset, 10,
				XmNleftOffset, 10, NULL);

*/
    XtAddCallback(p->printpanel.fax_cancel_button_child,
		  XmNactivateCallback, SecondaryCancelCallback,
		  &p->printpanel.fax_panel_child); 
    XtAddCallback(p->printpanel.fax_ok_button_child, XmNactivateCallback,
		  SecondaryOKCallback,
		  &p->printpanel.fax_callback_data); 
    XtAddCallback(p->printpanel.fax_apply_button_child,
		  XmNactivateCallback, SecondaryApplyCallback,
		  &p->printpanel.fax_callback_data); 
    
/*
    p->printpanel.fax_separator_child =
	XtVaCreateManagedWidget("faxSeparator", 
				xmSeparatorWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_ok_button_child,
				XmNbottomOffset, 10, NULL);
*/

    p->printpanel.fax_save_toggle_child =
	XtVaCreateManagedWidget("faxSaveToggle", xmToggleButtonWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10, 
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_separator_child,
				XmNlabelString, CS("Save to file"),
				XmNbottomOffset, 10, NULL);
    XtAddCallback(p->printpanel.fax_save_toggle_child,
		  XmNvalueChangedCallback, FaxToggleCallback, p);

    p->printpanel.fax_save_button_child =
	XtVaCreateManagedWidget("faxSaveButton", xmPushButtonWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_save_toggle_child, 
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.fax_separator_child,
				XmNwidth, 40,
				XmNlabelString, CS("..."), XmNbottomOffset,
				10, NULL);
    XtSetSensitive(p->printpanel.fax_save_button_child, FALSE);

    i = 0;
    XtSetArg(args[i], XmNdialogTitle, CS("Fax Save File Selection")); i++;
    XtSetArg(args[i], XmNwidth, 325); i++;
    XtSetArg(args[i], XmNresizePolicy, XmRESIZE_NONE); i++;
    p->printpanel.fax_save_selection_child =
	XmCreateFileSelectionDialog(p->printpanel.fax_panel_child,
				    "faxSaveFile", args, i);

    tmp =
	XmFileSelectionBoxGetChild(p->printpanel.fax_save_selection_child,
				   XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(tmp);

    p->printpanel.fax_file_data.head = (Widget) p;
    p->printpanel.fax_file_data.which_sb =
	p->printpanel.fax_save_selection_child;
    p->printpanel.fax_file_data.which_field = NULL;
    p->printpanel.fax_file_data.name = &p->printpanel.tmp_fax_data.filename;
    XtAddCallback(p->printpanel.fax_save_selection_child, XmNokCallback,
		  FileOKCallback, &p->printpanel.fax_file_data);
    XtAddCallback(p->printpanel.fax_save_selection_child,
		  XmNcancelCallback, FileCancelCallback,
		  &p->printpanel.fax_file_data); 

    XtAddCallback(p->printpanel.fax_save_button_child, XmNactivateCallback,
		  ChooseFileCallback,
		  p->printpanel.fax_save_selection_child); 

    p->printpanel.fax_phone_button_child =
	XtVaCreateManagedWidget("faxPhoneButton", xmPushButtonWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_save_toggle_child,
				XmNlabelString, CS("Phonebook..."),
				XmNheight, 30,
				XmNbottomOffset, 10, NULL);

    XtAddCallback(p->printpanel.fax_phone_button_child,
		  XmNactivateCallback, SecondaryCallback,
		  &(p->printpanel.fax_phone_panel_child));
    
/*
    p->printpanel.fax_add_button_child =
	XtVaCreateManagedWidget("faxAddButton", xmPushButtonWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_phone_button_child,
				XmNleftOffset, 10, 
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_save_toggle_child,
				XmNlabelString, CS("Add to Phonebook"),
				XmNheight, 30,
				XmNbottomOffset, 10, NULL);
*/

/*
    p->printpanel.fax_extra_button_child =
	XtVaCreateManagedWidget("faxExtraButton", xmPushButtonWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_add_button_child, 
				XmNleftOffset, 10, 
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_save_toggle_child,
				XmNlabelString, CS("Extra Options..."),
				XmNheight, 30,
				XmNbottomOffset, 10, NULL);
*/
    i = 0;
    XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
    XtSetArg(args[i], XmNleftOffset, 10); i++;
    XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(args[i], XmNbottomWidget,
	     p->printpanel.fax_phone_button_child); i++;
    XtSetArg(args[i], XmNbottomOffset, 10); i++;
    XtSetArg(args[i], XmNorientation, XmHORIZONTAL); i++;
    XtSetArg(args[i], XmNpacking, XmPACK_TIGHT); i++;
    p->printpanel.fax_format_radio_child =
	XmCreateRadioBox(p->printpanel.fax_panel_child, "formatRadio",
			 args, i);
    XtAddCallback(p->printpanel.fax_format_radio_child, XmNentryCallback,
		  FaxFormatCallback, p);

    XtManageChild(p->printpanel.fax_format_radio_child);

    p->printpanel.fax_standard_toggle_child =
	XtVaCreateManagedWidget("faxStandardToggle",
				xmToggleButtonWidgetClass, 
				p->printpanel.fax_format_radio_child,
				XmNlabelString, CS("Standard"), NULL);

    p->printpanel.fax_fine_toggle_child =
	XtVaCreateManagedWidget("faxFineToggle", xmToggleButtonWidgetClass,
				p->printpanel.fax_format_radio_child, 
				XmNlabelString, CS("Fine"), NULL);


    p->printpanel.fax_teleps_toggle_child =
	XtVaCreateManagedWidget("faxPSToggle", xmToggleButtonWidgetClass,
				p->printpanel.fax_format_radio_child,
				XmNlabelString, CS("PostScript"), NULL);

    p->printpanel.fax_teleps_label_child =
	XtVaCreateManagedWidget("faxPSLabel", xmLabelWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_phone_button_child,
				XmNleftOffset, 30, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.fax_save_toggle_child, 
				XmNbottomOffset, 10, XmNlabelString,
				CS("Password"), NULL);

    XtSetSensitive(p->printpanel.fax_teleps_label_child, FALSE);

    p->printpanel.fax_teleps_field_child =
	XtVaCreateManagedWidget("faxPSField", xmTextFieldWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget,
				p->printpanel.fax_teleps_label_child,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.fax_save_toggle_child, 
				XmNbottomOffset, 10, NULL);

    XtSetSensitive(p->printpanel.fax_teleps_field_child, FALSE);
    XtAddCallback(p->printpanel.fax_teleps_field_child,
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &(p->printpanel.tmp_fax_data.passwd)); 
    
    p->printpanel.fax_rec_label_child =
	XtVaCreateManagedWidget("faxRecLabel", 
				xmLabelWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget, 
				p->printpanel.fax_format_radio_child,
				XmNbottomOffset, 10, XmNlabelString,
				CS("Recipient Name"), NULL);

    p->printpanel.fax_rec_field_child =
	XtVaCreateManagedWidget("faxRecField",
				xmTextFieldWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftOffset, 10, XmNleftWidget, 
				p->printpanel.fax_rec_label_child,
				XmNrightAttachment, XmATTACH_FORM,
				XmNrightOffset, 10,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomOffset, 10, XmNbottomWidget,
				p->printpanel.fax_format_radio_child,
				NULL);

    XtAddCallback(p->printpanel.fax_rec_field_child,
		  XmNvalueChangedCallback, FieldChange, p);

    p->printpanel.fax_num_label_child =
	XtVaCreateManagedWidget("faxNumLabel", 
				xmLabelWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 10, XmNbottomAttachment,
				XmATTACH_WIDGET, XmNbottomWidget,
				p->printpanel.fax_rec_field_child,
				XmNbottomOffset, 10, XmNlabelString,
				CS("Fax Phone Number"), NULL);

    p->printpanel.fax_num_field_child =
	XtVaCreateManagedWidget("faxNumField", 
				xmTextFieldWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftOffset, 10, XmNleftWidget, 
				p->printpanel.fax_num_label_child,
				XmNrightAttachment, XmATTACH_FORM,
				XmNrightOffset, 10,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomOffset, 10, XmNbottomWidget,
				p->printpanel.fax_rec_field_child, NULL); 

    XtAddCallback(p->printpanel.fax_num_field_child,
		  XmNvalueChangedCallback, FieldChange, p);

    p->printpanel.fax_panel_label_child =
	XtVaCreateManagedWidget("faxPanelLabel", 
				xmLabelWidgetClass,
				p->printpanel.fax_panel_child,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, 35, XmNtopAttachment,
				XmATTACH_FORM, XmNtopOffset, 20,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget,
				p->printpanel.fax_num_field_child,
				XmNlabelString, CS("Fax Options"),
				XmNbottomOffset, 20, XmNwidth, 100, NULL);

    CreatePhonebookPanel(p);
}

static void CreatePtroffPanel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];

    p->printpanel.option_children[PTROFF].child[FAMLAB] =
	XtVaCreateWidget("familyLabel", 
			 xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, XmNwidth, 85, XmNlabelString,
			 CS("Font Family"), NULL);

    p->printpanel.option_children[PTROFF].num_children++;

    p->printpanel.option_children[PTROFF].child[FAMFIELD] =
	XtVaCreateWidget("familyField", 
			 xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET,
			 XmNleftOffset, 10, XmNleftWidget,
			 p->printpanel.option_children[PTROFF].child[FAMLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNrightAttachment, XmATTACH_FORM, XmNrightOffset,
			 20, NULL); 

    XtAddCallback(p->printpanel.option_children[PTROFF].child[FAMFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_roff_info.font); 

    p->printpanel.option_children[PTROFF].num_children++;

    p->printpanel.option_children[PTROFF].child[OPTLAB] =
	XtVaCreateWidget("macroLabel", 
			 xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PTROFF].child[FAMFIELD],
			 XmNbottomOffset, 10, XmNwidth, 85, XmNlabelString,
			 CS("Troff Options"),  NULL);

    p->printpanel.option_children[PTROFF].num_children++;

    p->printpanel.option_children[PTROFF].child[OPTFIELD] =
	XtVaCreateWidget("macroField", 
			 xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PTROFF].child[OPTLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.option_children[PTROFF].child[FAMFIELD],
			 XmNwidth, 85, XmNrightAttachment, XmATTACH_FORM,
			 XmNrightOffset, 20, NULL);

    XtAddCallback(p->printpanel.option_children[PTROFF].child[OPTFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_roff_info.options); 

    p->printpanel.option_children[PTROFF].num_children++;

}

static void CreatePsroffPanel(p)
    PrintPanelWidget p;
{

    p->printpanel.option_children[PSROFF].child[DIRLAB] =
	XtVaCreateWidget("dirLabel", 
			 xmLabelWidgetClass,
			 p->printpanel.generic_panel_child, 
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PTROFF].child[OPTFIELD],
			 XmNbottomOffset, 10, XmNwidth, 85, XmNlabelString,
			 CS("Font Directory"), NULL);

    p->printpanel.option_children[PSROFF].num_children++;

    p->printpanel.option_children[PSROFF].child[DIRFIELD] =
	XtVaCreateWidget("dirField", 
			 xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child, 
			 XmNleftAttachment, XmATTACH_WIDGET,
			 XmNleftOffset, 10, XmNleftWidget,
			 p->printpanel.option_children[PSROFF].child[DIRLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.option_children[PTROFF].child[OPTFIELD],
			 XmNrightAttachment, XmATTACH_FORM, XmNrightOffset,
			 20, NULL); 

    XtAddCallback(p->printpanel.option_children[PSROFF].child[DIRFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_roff_info.font_dir); 

    p->printpanel.option_children[PSROFF].num_children++;
}

static void CreatePlotPanel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];
    Widget tmp;

    p->printpanel.option_children[PSPLT].child[PROBUTTON] =
	XtVaCreateWidget("prologButton", 
			 xmPushButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNlabelString, CS("..."), XmNrightAttachment,
			 XmATTACH_FORM, XmNrightOffset, 20, NULL);

    p->printpanel.option_children[PSPLT].num_children++;

    p->printpanel.option_children[PSPLT].child[PROLAB] =
	XtVaCreateWidget("prologLabel", 
			 xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child ,
			 XmNbottomOffset, 10, XmNwidth, 85, XmNlabelString,
			 CS("Prolog File"), NULL);

    p->printpanel.option_children[PSPLT].num_children++;

    p->printpanel.option_children[PSPLT].child[PROFIELD] =
	XtVaCreateWidget("prologField", 
			 xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PSPLT].child[PROLAB],
			 XmNrightAttachment, XmATTACH_WIDGET,
			 XmNrightWidget,
			 p->printpanel.option_children[PSPLT].child[PROBUTTON],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.generic_separator_child, NULL);

    p->printpanel.option_children[PSPLT].num_children++;

    i = 0;
    XtSetArg(args[i], XmNdialogTitle, CS("Prologue File Selection")); i++;
    XtSetArg(args[i], XmNwidth, 325); i++;
    XtSetArg(args[i], XmNresizePolicy, XmRESIZE_NONE); i++;
    p->printpanel.plot_selection_box =
	XmCreateFileSelectionDialog(p->printpanel.generic_panel_child, 
				    "Plot Prologue", args, i);
    tmp =
	XmFileSelectionBoxGetChild(p->printpanel.plot_selection_box,
				   XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(tmp);
    
    p->printpanel.plot_data.head = (Widget) p;
    p->printpanel.plot_data.which_sb = p->printpanel.plot_selection_box;
    p->printpanel.plot_data.which_field =
	p->printpanel.option_children[PSPLT].child[PROFIELD]; 
    p->printpanel.plot_data.name = &p->printpanel.tmp_plot_profile;
    XtAddCallback(p->printpanel.plot_selection_box, XmNokCallback,
		  FileOKCallback, &p->printpanel.plot_data); 
    XtAddCallback(p->printpanel.plot_selection_box, XmNcancelCallback,
		  FileCancelCallback, &p->printpanel.plot_data); 

    XtAddCallback(p->printpanel.option_children[PSPLT].child[PROBUTTON],
		  XmNactivateCallback, ChooseFileCallback,
		  p->printpanel.plot_selection_box); 
}

static void Create630Panel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[20];

    i = 0;
    p->printpanel.pulldown_menu_child =
	XmCreatePulldownMenu(p->printpanel.generic_panel_child, "sizeMenu",
			     args, i);  

    i = 0;
    XtSetArg(args[i], XmNwidth, 50); i++;
    XtSetArg(args[i], XmNheight, 50); i++;
    p->printpanel.pulldown_buttons[0] =
	XmCreatePushButtonGadget(p->printpanel.pulldown_menu_child, "10", 
				 args, i);  
    XtAddCallback(p->printpanel.pulldown_buttons[0], XmNactivateCallback,
		  SetPitch10, NULL); 
    p->printpanel.pulldown_buttons[1] =
	XmCreatePushButtonGadget(p->printpanel.pulldown_menu_child, "12",
				 args, i);  
    XtAddCallback(p->printpanel.pulldown_buttons[1], XmNactivateCallback,
		  SetPitch12, NULL); 
    p->printpanel.pulldown_buttons[2] =
	XmCreatePushButtonGadget(p->printpanel.pulldown_menu_child, "15",
				 args, i);  
    XtAddCallback(p->printpanel.pulldown_buttons[2], XmNactivateCallback,
		  SetPitch15, NULL); 
    XtManageChildren(p->printpanel.pulldown_buttons, 3);

    i = 0;
    XtSetArg(args[i], XmNrightAttachment, XmATTACH_FORM); i++;
    XtSetArg(args[i], XmNrightOffset, 20); i++;
    XtSetArg(args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
    XtSetArg(args[i], XmNbottomWidget,
	     p->printpanel.generic_separator_child); i++; 
    XtSetArg(args[i], XmNbottomOffset, 10); i++;
    XtSetArg(args[i], XmNsubMenuId, p->printpanel.pulldown_menu_child);
    i++; 
    XtSetArg(args[i], XmNmenuHistory, p->printpanel.pulldown_buttons[1]);
    i++; 
    p->printpanel.option_children[PS630].child[OPTIONSIZE] =
	XmCreateOptionMenu(p->printpanel.generic_panel_child, 
			   "sizeOptionMenu", args, i);

    p->printpanel.option_children[PS630].num_children++;

    p->printpanel.option_children[PS630].child[SIZEOPTLAB] =
	XtVaCreateWidget("optLabel", xmLabelWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNrightAttachment, XmATTACH_WIDGET,
			 XmNrightWidget,
			 p->printpanel.option_children[PS630].child[OPTIONSIZE],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, XmNlabelString, CS("Size"),
			 NULL); 

    p->printpanel.option_children[PS630].num_children++;

    p->printpanel.option_children[PS630].child[BOLDLAB] =
	XtVaCreateWidget("boldLabel", 
			 xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, XmNwidth, 85, XmNlabelString,
			 CS("Bold Font"), NULL);

    p->printpanel.option_children[PS630].num_children++;

    p->printpanel.option_children[PS630].child[BOLDFIELD] =
	XtVaCreateWidget("boldField", 
			 xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS630].child[BOLDLAB],
			 XmNrightAttachment, XmATTACH_WIDGET,
			 XmNrightWidget,
			 p->printpanel.option_children[PS630].child[SIZEOPTLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.generic_separator_child, NULL);

    XtAddCallback(p->printpanel.option_children[PS630].child[BOLDFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_ps630_info.bold_font); 

    p->printpanel.option_children[PS630].num_children++;

    p->printpanel.option_children[PS630].child[BODYLAB] =
	XtVaCreateWidget("fontLabel",  
			 xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PS630].child[BOLDFIELD],
			 XmNbottomOffset, 10, XmNwidth, 85, XmNlabelString,
			 CS("Body Font"), NULL);

    p->printpanel.option_children[PS630].num_children++;

    p->printpanel.option_children[PS630].child[BODYFIELD] =
	XtVaCreateWidget("bodyField", 
			 xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS630].child[BODYLAB],
			 XmNrightAttachment, XmATTACH_FORM, XmNrightOffset,
			 115, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.option_children[PS630].child[BOLDFIELD],
			 NULL); 

    XtAddCallback(p->printpanel.option_children[PS630].child[BODYFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_ps630_info.body_font);  

    p->printpanel.option_children[PS630].num_children++;
}


static void Create4014Panel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[10];

    p->printpanel.option_children[PS4014].child[SCALELAB] =
	XtVaCreateWidget("scaleLabel",
			 xmLabelWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, XmNwidth, 120,
			 XmNlabelString, CS("Scaled Width (in)"), NULL);

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[SCALEFIELD] =
	XtVaCreateWidget("scaleField", 
			 xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[SCALELAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, XmNwidth, 40, NULL);
    
    XtAddCallback(p->printpanel.option_children[PS4014].child[SCALEFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_ps4014_info.scale); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[WIDLAB] =
	XtVaCreateWidget("widthLabel", xmLabelWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[SCALEFIELD],
			 XmNbottomOffset, 10, XmNwidth, 120,
			 XmNlabelString, CS("Width of image (in)"), NULL);

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[WIDFIELD] =
	XtVaCreateWidget("widthField", 
			 xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[WIDLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[SCALEFIELD],
			 XmNbottomOffset, 10, XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.option_children[PS4014].child[WIDFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_ps4014_info.width); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[HGTLAB] =
	XtVaCreateWidget("heightLabel", xmLabelWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[WIDFIELD],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[SCALEFIELD],
			 XmNbottomOffset, 10, XmNwidth, 120,
			 XmNlabelString, CS("Height of image (in)"), NULL); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[HGTFIELD] =
	XtVaCreateWidget("heightField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[HGTLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[SCALEFIELD],
			 XmNbottomOffset, 10, XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.option_children[PS4014].child[HGTFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_ps4014_info.height); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[LEFTLAB] =
	XtVaCreateWidget("leftLabel", xmLabelWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[WIDFIELD],
			 XmNbottomOffset, 10, XmNwidth, 120,
			 XmNlabelString, CS("Left margin (in)"), NULL);

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[LEFTFIELD] =
	XtVaCreateWidget("leftField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[LEFTLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[WIDFIELD],
			 XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.option_children[PS4014].child[LEFTFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_ps4014_info.left); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[BOTLAB] =
	XtVaCreateWidget("bottomLabel", xmLabelWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[LEFTFIELD],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[WIDFIELD],
			 XmNwidth, 120, XmNlabelString,
			 CS("Bottom Margin (in)"), NULL);

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[BOTFIELD] =
	XtVaCreateWidget("bottomField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset,
			 10, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[BOTLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomOffset, 10, XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[WIDFIELD],
			 XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.option_children[PS4014].child[BOTFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &p->printpanel.tmp_ps4014_info.bottom); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[NOLFTOG] =
	XtVaCreateWidget("noLFToggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[LEFTFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("No LF on CR"), NULL);

    XtAddCallback(p->printpanel.option_children[PS4014].child[NOLFTOG],
		  XmNvalueChangedCallback, ToggleCallback,
		  &p->printpanel.tmp_ps4014_info.cr_no_lf); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[NOCRTOG] =
	XtVaCreateWidget("noCRToggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[NOLFTOG],
			 XmNleftOffset, 10, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[LEFTFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("No CR on LF"), NULL);

    XtAddCallback(p->printpanel.option_children[PS4014].child[NOCRTOG],
		  XmNvalueChangedCallback, ToggleCallback,
		  &p->printpanel.tmp_ps4014_info.lf_no_cr); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[MARTOG] =
	XtVaCreateWidget("margin2Toggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[NOCRTOG],
			 XmNleftOffset, 10, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[LEFTFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Margin 2 Mode"), NULL);

    XtAddCallback(p->printpanel.option_children[PS4014].child[MARTOG],
		  XmNvalueChangedCallback, ToggleCallback,
		  &p->printpanel.tmp_ps4014_info.margin_2); 

    p->printpanel.option_children[PS4014].num_children++;

    p->printpanel.option_children[PS4014].child[PORTTOG] =
	XtVaCreateWidget("portraitToggle", xmToggleButtonWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[PS4014].child[MARTOG],
			 XmNleftOffset, 10, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[PS4014].child[LEFTFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Portrait"), NULL);

    XtAddCallback(p->printpanel.option_children[PS4014].child[PORTTOG],
		  XmNvalueChangedCallback, ToggleCallback,
		  &p->printpanel.tmp_ps4014_info.portrait); 

    p->printpanel.option_children[PS4014].num_children++;
}
    
static void CreateEnscriptPanel(p)
    PrintPanelWidget p;
{
    int i;
    Arg args[20];

    p->printpanel.option_children[ENSCRIPT].child[HSIZEFIELD] =
	XtVaCreateWidget("headerSizeField", xmTextFieldWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, XmNrightAttachment,
			 XmATTACH_FORM, XmNrightOffset, 110,
			 XmNwidth, 40, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[HSIZEFIELD],
		  XmNvalueChangedCallback, RealTextFieldChange,
		  &(p->printpanel.tmp_enscript_info.header_font_size)); 

    p->printpanel.option_children[ENSCRIPT].child[HSIZELAB] =
	XtVaCreateWidget("headerSizeLabel", 
			 xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNrightAttachment, XmATTACH_WIDGET,
			 XmNrightWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HSIZEFIELD], 
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Point Size"), NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[HFONTLAB] =
	XtVaCreateWidget("headerFontLabel", xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Header Font"), XmNwidth, 85, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[HFONTFIELD] =
	XtVaCreateWidget("headerFontField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HFONTLAB],
			 XmNrightAttachment, XmATTACH_WIDGET,
			 XmNrightWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HSIZELAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.generic_separator_child,
			 XmNbottomOffset, 10, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;
			 
    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[HFONTFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &(p->printpanel.tmp_enscript_info.header_font)); 

    p->printpanel.option_children[ENSCRIPT].child[SIZEFIELD] =
	XtVaCreateWidget("sizeField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNrightAttachment, XmATTACH_FORM, XmNrightOffset,
			 110, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HFONTFIELD],
			 XmNbottomOffset, 10, XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[SIZEFIELD],
		  XmNvalueChangedCallback, RealTextFieldChange,
		  &(p->printpanel.tmp_enscript_info.font_size)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[SIZELAB] =
	XtVaCreateWidget("sizeLabel", xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNrightAttachment, XmATTACH_WIDGET,
			 XmNrightWidget,
			 p->printpanel.option_children[ENSCRIPT].child[SIZEFIELD],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HFONTFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Point Size"), NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[FONTLAB] =
	XtVaCreateWidget("fontLabel", xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HFONTFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Body Font"), XmNwidth, 85, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[FONTFIELD] =
	XtVaCreateWidget("fontField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[FONTLAB],
			 XmNrightAttachment, XmATTACH_WIDGET,
			 XmNrightWidget,
			 p->printpanel.option_children[ENSCRIPT].child[SIZELAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HFONTFIELD],
			 XmNbottomOffset, 10, NULL); 

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[FONTFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &(p->printpanel.tmp_enscript_info.font)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[HEADLAB] =
	XtVaCreateWidget("headerLabel", xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[FONTFIELD],
			 XmNbottomOffset, 10, XmNlabelString, CS("Header"),
			 XmNwidth, 85, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[HEADFIELD] =
	XtVaCreateWidget("headerField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HEADLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[FONTFIELD],
			 XmNbottomOffset, 10, XmNrightAttachment,
			 XmATTACH_FORM, XmNrightOffset, 20, XmNwidth, 150,
			 NULL); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[HEADFIELD],
		  XmNvalueChangedCallback, StringTextFieldChange,
		  &(p->printpanel.tmp_enscript_info.header)); 

    p->printpanel.option_children[ENSCRIPT].child[TABLAB] =
	XtVaCreateWidget("tabLabel", xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HEADFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Tab Width"), XmNwidth, 85, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[TABFIELD] =
	XtVaCreateWidget("tabField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[TABLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HEADFIELD],
			 XmNbottomOffset, 10, XmNwidth, 40, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[TABFIELD],
		  XmNvalueChangedCallback, IntTextFieldChange,
		  &(p->printpanel.tmp_enscript_info.tab_width)); 
    
    p->printpanel.option_children[ENSCRIPT].child[LINELAB] =
	XtVaCreateWidget("linesLabel", xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[TABFIELD],
			 XmNbottomOffset, 10, XmNlabelString, CS("Lines"),
			 XmNwidth, 85, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[LINEFIELD] =
	XtVaCreateWidget("linesField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[LINELAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[TABFIELD],
			 XmNbottomOffset, 10, XmNwidth, 40, NULL);
			 

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[LINEFIELD],
		  XmNvalueChangedCallback, IntTextFieldChange,
		  &(p->printpanel.tmp_enscript_info.lines)); 
    
    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[COLLAB] =
	XtVaCreateWidget("columnsLabel", xmLabelWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset,
			 10, XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[LINEFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Columns"), XmNwidth, 85, NULL);

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[COLFIELD] =
	XtVaCreateWidget("columnsField", xmTextFieldWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[COLLAB],
			 XmNbottomAttachment, XmATTACH_WIDGET,
			 XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[LINEFIELD],
			 XmNbottomOffset, 10, XmNwidth, 40, NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[COLFIELD],
		  XmNvalueChangedCallback, IntTextFieldChange,
		  &(p->printpanel.tmp_enscript_info.columns)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[NOHEAD] =
	XtVaCreateWidget("noHeadingsToggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[COLFIELD],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HEADFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("No page headings"), NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[NOHEAD],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.no_header)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[LP] =
	XtVaCreateWidget("linePrinterToggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[COLFIELD],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[NOHEAD],
			 XmNlabelString, CS("Line Printer Mode"), NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[LP],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.lpt_mode)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[GAUDY] =
	XtVaCreateWidget("gaudyToggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[COLFIELD],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[LP],
			 XmNlabelString, CS("Gaudy Mode"), NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[GAUDY],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.gaudy)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[OVERRIDE] =
	XtVaCreateWidget("noRotateToggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[COLFIELD],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[GAUDY],
			 XmNlabelString, CS("No rotation"), NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[OVERRIDE],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.norotate)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[ROTATE] =
	XtVaCreateWidget("rotateToggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[COLFIELD],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[OVERRIDE],
			 XmNlabelString, CS("Rotate"), NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[ROTATE],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.rotated)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[NOBURST] =
	XtVaCreateWidget("noburstToggle", xmToggleButtonWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[LP],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[HEADFIELD],
			 XmNbottomOffset, 10, XmNlabelString,
			 CS("Suppress burst page"), NULL);


    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[NOBURST],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.no_burst_page)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[QUIET] =
	XtVaCreateWidget("quietToggle", xmToggleButtonWidgetClass,
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[LP],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[NOBURST],
			 XmNlabelString, CS("Quiet Mode"), NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[QUIET],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.quiet_mode)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[MISSCHAR] =
	XtVaCreateWidget("missingCharsToggle", xmToggleButtonWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[LP],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[QUIET],
			 XmNlabelString, CS("List missing characters"),
			 NULL); 

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[MISSCHAR],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.report_missing_chars)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[IGNORE] =
	XtVaCreateWidget("ignoreToggle", xmToggleButtonWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget, 
			 p->printpanel.option_children[ENSCRIPT].child[LP],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[MISSCHAR],
			 XmNlabelString, CS("Ignore garbage"), NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[IGNORE],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.ignore_binary)); 

    p->printpanel.option_children[ENSCRIPT].num_children++;

    p->printpanel.option_children[ENSCRIPT].child[TRUNC] =
	XtVaCreateWidget("truncateToggle", xmToggleButtonWidgetClass, 
			 p->printpanel.generic_panel_child,
			 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget,
			 p->printpanel.option_children[ENSCRIPT].child[LP],
			 XmNleftOffset, 30, XmNbottomAttachment,
			 XmATTACH_WIDGET, XmNbottomWidget,
			 p->printpanel.option_children[ENSCRIPT].child[IGNORE],
			 XmNlabelString, CS("Truncate"), NULL);

    XtAddCallback(p->printpanel.option_children[ENSCRIPT].child[TRUNC],
		  XmNvalueChangedCallback, ToggleCallback,
		  &(p->printpanel.tmp_enscript_info.truncate_lines));

    p->printpanel.option_children[ENSCRIPT].num_children++;
}

#define TRUE 1
#define FALSE 0

#ifdef BSD
static int GetPrinterList(plist, nprinters, maxprinters, filename)
    char *plist[];
    int *nprinters;
    int maxprinters;
    char *filename;
{
    FILE *fp;
    char buf[1024];
    char *p;
    int i;
    char *pname;

    if ((fp = fopen(filename, "r")) == NULL) 
        return -1;

    i = 0;

    while (fgets(buf, 1024, fp)) {
	if (buf[0] == '#')
	    continue;
	if (buf[0] == '\t' || buf[0] == ':')
	    continue;
	if (buf[0] == '\n' || buf[0] == ' ')
	    continue;
	pname = buf;
	
	if ((p = strchr(buf,'|')) != NULL) 
	    *p = '\0';
	else if ((p = strchr(buf, ':')) != NULL)
	    *p = '\0';
	else if ((p = strchr(buf, '\n')) != NULL)
	    *p = '\0';
	else {
	    fclose(fp);
	    return -2;
	}
	if (i == maxprinters) {
	    fclose(fp);
	    return -3;
	}
	plist[i] = (char *) malloc(strlen(pname)+1);
	strcpy (plist[i], pname);
	i++;
    }
    *nprinters = i;
    fclose(fp);
    return 0;
}
#endif /* BSD */

#ifdef SYSV
static int GetSysVPrinterList(plist, nprinters, maxprinters)
    char *plist[];
    int *nprinters;
    int maxprinters;
{
    FILE *fp;
    char fname[255];
    char buf[1024];
    char *pname, *p;
    int i;


    p = getenv("PSLIBDIR");
    if (p)
	strcpy(fname, p);
    else
	strcpy(fname, PSLibDir);
    strcat(fname, "/printer.list");
    if ((fp = fopen(fname, "r")) == NULL)
	return FALSE;

    i = 0;
    while (fgets(buf, 1024, fp)) {
	if (buf[0] == '#')
	    continue;
	pname = buf;
	if (p = strchr(buf, '\n')) *p = '\0';
	if (i == maxprinters) {
	    fclose(fp);
	    return FALSE;
	}
	plist[i] = (char *) malloc(strlen(pname)+1);
	strcpy(plist[i], pname);
	i++;
    }
    *nprinters = i;
    fclose(fp);
    return TRUE;
}
#endif /* SYSV */
    

static int addarg(argstr, nargs, arg)
    char *argstr[];
    int nargs;
    char *arg;
{
    char *p;

    if ((p = (char *) malloc(strlen(arg) + 1)) == NULL) {
	return -1;
    }
    strcpy(p, arg);
    argstr[nargs] = p;
    argstr[nargs+1] = NULL;
    return 1;
}


static void AddToCompoundList(xstr, msg)
    XmString *xstr;
    char *msg;
{
    if (*xstr != NULL)
	*xstr = XmStringConcat(*xstr, XmStringSeparatorCreate());
    *xstr = XmStringConcat(*xstr, CS(msg));
}
