// Enable the navigation bar, and do not auto-hide it.
user_pref("rkiosk.navbar", true);
user_pref("browser.fullscreen.autohide", false);

// Do not invoke external applications.
user_pref("network.protocol-handler.external.mailto", false);
user_pref("network.protocol-handler.external.news", false);
user_pref("network.protocol-handler.external.nntp", false);
user_pref("network.protocol-handler.external-default", false);

// "Disable" printing.  (Doesn't actually disable it, but gives 
// the illusion of doing so.  Users can't get their jobs anyway)
user_pref("print.always_print_silent",true);
user_pref("print.show_print_progress",false);

// Disable the compatibility check for the r-kiosk extension
// (the per-version preference is used in firefox 3.6+).
user_pref("extensions.checkCompatibility", false);
user_pref("extensions.checkCompatibility.3.6", false);

// Set the home page.
user_pref("browser.startup.homepage", "file:///usr/share/debathena-kiosk/index.html");
user_pref("startup.homepage_override_url", "");
user_pref("startup.homepage_welcome_url", "");
