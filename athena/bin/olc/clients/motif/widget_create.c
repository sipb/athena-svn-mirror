#include "xolc.h"

Widget			/* Widget ID's */
  widget_askbox,
  widget_motdbox,
  widget_motdframe,
  widget_motdlabel,
  widget_helpbox,
  widget_box_button[4],
  widget_errorbox,
  widget_replaybox,
  toplevel,
  main_form;

  
void widget_create (w, tag, callback_data)
     Widget w;
     int *tag;
     XmAnyCallbackStruct *callback_data;
{
  Arg arg;
    static char *motd =
"Welcome to OLC, Project Athena's On-Line Consulting system.\nCopyright (c) 1989 by the Massachusetts Institute of Technology.\n";
  static char string[1024];
    
  switch (*tag) {
  case ASKBOX:
    printf ("ask created\n");
    widget_askbox = w;
    break;
  case REPLAYBOX:
    widget_replaybox = w;
    fscanf(popen("olc replay", "r"), "%1023c", string);
    XmTextSetString(widget_replaybox, string);
    printf ("replay initialized\n");
    break;
  case HELPBOX:
    widget_helpbox = w;
    widget_box_button[0] =
      (Widget)(XmMessageBoxGetChild(widget_helpbox,
				    XmDIALOG_CANCEL_BUTTON));
    widget_box_button[1] =
      (Widget)(XmMessageBoxGetChild(widget_helpbox,
				    XmDIALOG_HELP_BUTTON));
    XtDestroyWidget(widget_box_button[0]);
    XtDestroyWidget(widget_box_button[1]);
    XtSetArg(arg, XmNmessageString,
	     XmStringLtoRCreate(
"This is the X version of the On-Line Consulting program.\n\nChoose a topic for your question and enter your question when\nprompted.  You will be connected automatically to the next available\nconsultant.  If you wish to send a message to the consultant, click on the\n'send' button.  When you are satisfied that your question has been answered\ncompletely, click on the 'done' button.  If, after entering a question, you\ndecide you no longer want to continue the question, or if you discover the\nanswer on your own, you can remove your question with the 'cancel' button.\n\nThe 'save log' button will allow you to keep a copy of this conversation in\na file, and the 'motd' button will show you the 'message of the day'.",
""));
    XtSetValues(widget_helpbox, &arg, 1);
    break;
  case ERRORBOX:
    widget_errorbox = w;
    widget_box_button[2] =
      (Widget)(XmMessageBoxGetChild(widget_errorbox,
				    XmDIALOG_HELP_BUTTON));
    widget_box_button[3] =
      (Widget)(XmMessageBoxGetChild(widget_errorbox,
				    XmDIALOG_CANCEL_BUTTON));
    XtDestroyWidget(widget_box_button[2]);
    XtDestroyWidget(widget_box_button[3]);
    break;
  case MOTDBOX:
    widget_motdbox = w;
    XtSetArg(arg, XmNmessageString, XmStringLtoRCreate(motd,""));
    XtSetValues(widget_motdbox, &arg, 1);
    printf ("motdbox initialized\n");
    break;
  case MOTDFRAME:
    widget_motdframe = w;
    printf ("motd frame creation\n");
    break;
  case MOTD:
    widget_motdlabel = w;
    printf ("motd label creation\n");
    break;
  }
}

