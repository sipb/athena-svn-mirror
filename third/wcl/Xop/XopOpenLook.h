#ifndef _XopOpenLook_h_
#define _XopOpenLook_h_
#include <X11/Xop/COPY.h>

/* SCCS_data: %Z% %M%	%I% %E% %U%
*/

#ifdef XTTRACEMEMORY
#undef XTTRACEMEMORY	/* Thanks whoever wrote Xol/OlXlibExt.h  >:^p */
#endif

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint
*/
#include <X11/Intrinsic.h>

#include <Xol/OpenLookP.h>	/* need OL_VERSION */

/* include all the *.h files in heirarchical order */

/* Under Primitive */
#include <Xol/AbbrevMenu.h>
#include <Xol/AbbrevStac.h>
#include <Xol/Arrow.h>
#include <Xol/Button.h>
#include <Xol/MenuButton.h>
#include <Xol/ButtonStac.h>
#include <Xol/ListPane.h>
#include <Xol/Mag.h>
#include <Xol/OblongButt.h>
#if OL_VERSION > 2
#include <Xol/Pixmap.h>		/* OLIT 3 */
#include <Xol/DropTarget.h>	/* OLIT 3 */
#endif
#include <Xol/Pushpin.h>
#include <Xol/RectButton.h>
#include <Xol/Scrollbar.h>
#include <Xol/Slider.h>
#include <Xol/StaticText.h>
#include <Xol/Stub.h>
#include <Xol/TextEdit.h>
#include <Xol/TextPane.h>

#include <Xol/Flat.h>
#include <Xol/FExclusive.h>
#include <Xol/FNonexclus.h>
#include <Xol/FCheckBox.h>

/* Under TopLevelShell */
#include <Xol/BaseWindow.h>
#include <Xol/Menu.h>
#include <Xol/Notice.h>
#include <Xol/PopupWindo.h>

/* Under Constraint */
#include <Xol/Manager.h>
#include <X11/Xp/Table.h>

/* Under Manager */
#include <Xol/BulletinBo.h>
#include <Xol/Caption.h>
#include <Xol/CheckBox.h>
#include <Xol/ControlAre.h>
#if OL_VERSION > 2
#include <Xol/DrawArea.h>	/* OLIT 3 */
#endif
#include <Xol/Exclusives.h>
#include <Xol/FooterPane.h>
#include <Xol/Form.h>
#if OL_VERSION > 2
#include <Xol/Nonexclusi.h>	/* OLIT 3 */
#include <Xol/RubberTile.h>	/* OLIT 3 */
#include <Xol/Help.h>		/* OLIT 3 */
#else
#include <Xol/Help.h>		/* OLIT 2 */
#include <Xol/Nonexclusi.h>	/* OLIT 2 */
#endif
#include <Xol/ScrolledWi.h>
#include <Xol/ScrollingL.h>
#include <Xol/TextField.h>
#include <Xol/Text.h>

#endif
