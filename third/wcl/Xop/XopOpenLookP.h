#ifndef _XopOpenLookP_h_
#define _XopOpenLookP_h_
#include <X11/Xop/COPY.h>

/* SCCS_data: %Z% %M%	%I% %E% %U%
*/

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint
*/
#include <X11/IntrinsicP.h>
#include <Xol/OpenLookP.h>	/* only for OL_VERSION */

/* include all the *.h files in heirarchical order */

/* Xt Base Classes */
#include <X11/CoreP.h>
#include <X11/ObjectP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>

#include <Xol/EventObjP.h>
#include <Xol/PrimitiveP.h>

/* Under Primitive */
#include <Xol/AbbrevMenP.h>
#include <Xol/AbbrevStaP.h>
#include <Xol/ArrowP.h>
#include <Xol/ButtonP.h>
#include <Xol/MenuButtoP.h>
#include <Xol/ButtonStaP.h>
#include <Xol/ListPaneP.h>
#include <Xol/MagP.h>
#include <Xol/OblongButP.h>
#if OL_VERSION > 2
#include <Xol/PixmapP.h>	/* OLIT 3 */
#include <Xol/DropTargetP.h>	/* OLIT 3 */
#endif
#include <Xol/PushpinP.h>
#include <Xol/RectButtoP.h>
#include <Xol/ScrollbarP.h>
#include <Xol/SliderP.h>
#include <Xol/StaticTexP.h>
#include <Xol/StubP.h>
#include <Xol/TextEditP.h>
#include <Xol/TextPaneP.h>

#include <Xol/FlatP.h>
#include <Xol/FExclusivP.h>
#include <Xol/FNonexcluP.h>
#include <Xol/FCheckBoxP.h>

/* Under TopLevelShell */
#include <Xol/BaseWindoP.h>
#include <Xol/MenuP.h>
#include <Xol/NoticeP.h>
#include <Xol/PopupWindP.h>

/* Under Constraint */
#include <Xol/ManagerP.h>

/* Under Manager */
#include <Xol/BulletinBP.h>
#include <Xol/CaptionP.h>
#include <Xol/CheckBoxP.h>
#include <Xol/ControlArP.h>
#if OL_VERSION > 2
#include <Xol/DrawAreaP.h>	/* OLIT 3 */
#endif
#include <Xol/ExclusiveP.h>
#include <Xol/FooterPanP.h>
#include <Xol/FormP.h>
#if OL_VERSION > 2
#include <Xol/NonexclusP.h>	/* OLIT 3 */
#include <Xol/RubberTilP.h>	/* OLIT 3 */
#include <Xol/HelpP.h>		/* OLIT 3 */
#else
#include <Xol/HelpP.h>		/* OLIT 2 */
#include <Xol/NonexclusP.h>	/* OLIT 2 */
#endif
#include <Xol/ScrolledWP.h>
#include <Xol/ScrollingP.h>
#include <Xol/TextFieldP.h>
#include <Xol/TextP.h>

#endif
