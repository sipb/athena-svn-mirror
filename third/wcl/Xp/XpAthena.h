#ifndef _XpAthena_h_
#define _XpAthena_h_
#include <X11/Xp/COPY.h>

/* SCCS_data: @(#) XpAthena.h	1.4 94/09/23 10:39:01
*/

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint.
*/
#include <X11/Intrinsic.h>

#if defined(XtSpecificationRelease) 
#if XtSpecificationRelease == 5
#define R5
#else
#define R6
#endif
#endif

/* include all the *.h files in heirarchical order */

#include <X11/Xaw/Simple.h>
#include <X11/Vendor.h>

/* Core */
#ifndef R6
/* went away in R6 */
#include <X11/Xaw/Clock.h>
#include <X11/Xaw/Logo.h>
#endif
#ifdef R5
/* only in R5 */
#include <X11/Xaw/Mailbox.h>
#endif

/* Simple */
#include <X11/Xaw/Grip.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xaw/Text.h>
#if defined(R5) || defined(R6)
#include <X11/Xaw/Panner.h>
#endif /*R5 or R6*/

/* Label */
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Toggle.h>

/* Command */
#if defined(R5) || defined(R6)
#include <X11/Xaw/Repeater.h>
#endif

/* Sme */
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>


/* Text */
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/TextSrc.h>
#include <X11/Xaw/AsciiSrc.h>
#include <X11/Xaw/TextSink.h>
#include <X11/Xaw/AsciiSink.h>

/* Composite and Constraint */
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#if defined(R5) || defined(R6)
#include <X11/Xaw/Porthole.h>
#include <X11/Xaw/Tree.h>
#endif /*R5 or R6*/
#include <X11/Xp/Table.h>

/* Form */
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Viewport.h>

#undef R5

#endif /* _XpAthena_h_ */
