#ifndef _Xmp_h_
#define _Xmp_h_
#include <X11/Xmp/COPY.h>

/*
* SCCS_data: %Z% %M% %I% %E% %U%
*
*	This module contains declarations useful to clients of the
*	Xmp library.
*
*******************************************************************************
*/

#include <Xm/Xm.h>
#include <X11/Wc/PortableC.h>

BEGIN_NOT_Cxx

/* XmpRegisterMotif registers all Motif and Xmp widgets.
 * XmpRegisterAll and MriRegisterMotif are aliases for XmpRegisterMotif
 * for backward compatibility.
 */
void XmpRegisterMotif _(( XtAppContext ));
void XmpRegisterAll   _(( XtAppContext ));
void MriRegisterMotif _(( XtAppContext ));

/* These are called by the above functions.  If you want the callbacks
 * or converters but not the Widgets registered, then you can just call
 * these.  The XmpAddMotifConverters() adds those converters provided by
 * Motif, while XmpAddConverters() adds those converters provided by libXmp.
 */
void XmpAddActionsAndCallbacks _(( XtAppContext ));
void XmpAddMotifConverters     _(( XtAppContext ));
void XmpAddConverters          _(( XtAppContext ));

void XmpAddMwmCloseCallback _(( Widget, XtCallbackProc, XtPointer ));

void XmpChangeNavigationType _(( Widget ));

/* Support for sending WcPrint messages to an XmText widget.  The
 * following resources can be specified on this XmText widget:
 */
#define XmpNmessageMaxLines	"messageMaxLines"
#define XmpCMessageMaxLines	"MessageMaxLines"
#define XmpNmessageColumns	"messageColumns"
#define XmpCMessageColumns	"MessageColumns"
#define XmpNmessageLinesLostMsg	"messageLinesLostMsg"
#define XmpCMessageLinesLostMsg "MessageLinesLostMsg"
#define XmpNmessagePopup	"messagePopup"
#define XmpCMessagePopup	"MessagePopup"
#define XmpNmessageRaise	"messageRaise"
#define XmpCMessageRaise	"MessageRaise"
#define XmpNmessageBrokenXmTextShowPosition "messageBrokenXmTextShowPosition"
#define XmpCMessageBrokenXmTextShowPosition "MessageBrokenXmTextShowPosition"

/* resource		default
 * messageMaxLines	100		number of lines to be put into the
 *					XmText widget.  Once this limit is
 *					reached, then the first lines will
 *					be removed and replaced by the
 *					messageLinesLostMsg.
 * messageColumns	80		The messages will be broken into
 *					multiple lines at whitespace, each
 *					line with this maximum number of
 *					characters.
 * messageLinesLostMsg ... some lines lost...\n
 *					This message, which should be newline
 *					terminated, becomes the first line(s)
 *					when some earlier message(s) get
 *					eliminated due to exceeding
 *					messageMaxLines in the XmText widget.
 * messagePopup		True		The shell containing the XmText widget
 *					will be popped up when a new message
 *					arrives.
 * messageRaise		False		The shell will be raised to (try and)
 *					become the top window when a new
 *					message arrives.
 * messageBrokenXmTextShowPosition False You may be surprised to discover that
 *					XmTextShowPosition() is a VERY
 *					special case function, which often
 *					does not work.  XmpPrint would like
 *					to scroll the message window when a
 *					message comes in, and would like to
 *					use XmTextShowPosition() to do so.
 *					If it works, OK.  If not, set this
 *					resource...
 *
 * These resources are fetched from the XmText widget when the widget becomes
 * the message widget due to a call to XmpMessageWidget(), XmpMessageWidgetCB(),
 * or XmpMessageWidgetACT().
 */
extern void XmpMessageWidget _(( Widget ));


END_NOT_Cxx

#endif /* _Xmp_h_ */
