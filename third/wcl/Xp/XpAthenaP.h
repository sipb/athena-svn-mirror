#ifndef _XpAthenaP_h_
#define _XpAthenaP_h_
#include <X11/Xp/COPY.h>

/* SCCS_data: @(#) XpAthenaP.h	1.4 94/09/23 10:38:54
*/

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint.
*/
#include <X11/IntrinsicP.h>

#if defined(XtSpecificationRelease) 
 #if XtSpecificationRelease == 5
  #define R5
 #else
  #define R6
 #endif
#endif

/* include all the *P.h files in heirarchical order */

#include <X11/CoreP.h>
#include <X11/ObjectP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>

/* Core */
#include <X11/Xaw/SimpleP.h>
#ifndef R6
#include <X11/Xaw/LogoP.h>	/* went away in R6 */
#endif

/* Core with lotsa system dependencies: using these P.h files requires a
** very well configured Imake configuration due to the many OS dependencies,
** and - hey - we really only do this for kicks anyway!
*/ 
#ifndef R6
#include <X11/Xaw/Clock.h>	/* went away in R6 */
#ednfi
#ifdef R5
#include <X11/Xaw/Mailbox.h>	/* only in R5 */
#endif

/* Simple */
#include <X11/Xaw/GripP.h>
#include <X11/Xaw/LabelP.h>
#include <X11/Xaw/ListP.h>
#include <X11/Xaw/ScrollbarP.h>
#include <X11/Xaw/StripCharP.h>
#include <X11/Xaw/TextP.h>
#ifdef R5
#include <X11/Xaw/PannerP.h>
#endif /*R5*/

/* Label */
#include <X11/Xaw/CommandP.h>
#include <X11/Xaw/MenuButtoP.h>
#include <X11/Xaw/ToggleP.h>

/* Command */
#ifdef R5
#include <X11/Xaw/RepeaterP.h>
#endif

/* Sme */
#include <X11/Xaw/SmeP.h>
#include <X11/Xaw/SimpleMenP.h>
#include <X11/Xaw/SmeBSBP.h>
#include <X11/Xaw/SmeLineP.h>


/* Text */
#include <X11/Xaw/AsciiTextP.h>
#include <X11/Xaw/TextSrcP.h>
#include <X11/Xaw/AsciiSrcP.h>
#include <X11/Xaw/TextSinkP.h>
#include <X11/Xaw/AsciiSinkP.h>

/* Composite and Constraint */
#include <X11/Xaw/BoxP.h>
#include <X11/Xaw/FormP.h>
#include <X11/Xaw/PanedP.h>
#ifdef R5
#include <X11/Xaw/PortholeP.h>
#include <X11/Xaw/TreeP.h>
#endif /*R5*/
#include <X11/Xp/TableP.h>

/* Form */
#include <X11/Xaw/DialogP.h>
#include <X11/Xaw/ViewportP.h>

#undef R5

#endif /* _XpAthenaP_h_ */
