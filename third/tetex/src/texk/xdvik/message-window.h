#ifndef MESSAGE_WINDOW_H
#define MESSAGE_WINDOW_H

#ifdef TOOLKIT

#define MSG_HELP 0
#define MSG_INFO 1
#define MSG_WARN 2
#define MSG_ERR  3


/* XawtextScrollWhenNeeded has been ditched in Xaw7
   (e.g. Xfree >= 4.x); from the man page:
   
   The value XawtextScrollWhenNeeded (and whenNeeded, recognized by
   the converter), is accepted for backwards compatibilty with
   resource specifications written for the Xaw6 Text widget, but
   ignored (effectively treated as XawtextScrollNever).

   So we're forced to use `scrollAlways' here.
*/
#define XAW_SCROLL_ALWAYS XawtextScrollAlways

/* the reconfig stuff has been fixed
   in XFree 4.1.0, vendor release 6510 (Slackware 8.0).
   In RedHat 7.1, XFree 4.1.0 has vendor release
   40100000; dunno about SuSE ... dang, why can't there
   be a uniform scheme for XFree?
*/
#define BROKEN_RECONFIG ((                              \
        (strstr(ServerVendor(DISP), "XFree") != NULL)   \
        && VendorRelease(DISP) >= 4000                  \
        && (VendorRelease(DISP) > 10000000              \
            ? VendorRelease(DISP) < 40020000            \
            : VendorRelease(DISP) < 4002)               \
        ))

#endif

#endif /* MESSAGE_WINDOW_H */
