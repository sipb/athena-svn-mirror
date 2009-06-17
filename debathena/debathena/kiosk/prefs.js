// Enable the navigation bar, and do not auto-hide it.
user_pref("rkiosk.navbar", true);
user_pref("browser.fullscreen.autohide", false);

// Do not invoke external applications.
user_pref("network.protocol-handler.external.mailto", false);
user_pref("network.protocol-handler.external.news", false);
user_pref("network.protocol-handler.external.nntp", false);
user_pref("network.protocol-handler.external-default", false);

// Needed because r-kiosk's maxVersion is set to 3.0, not 3.0.*.
user_pref("extensions.checkCompatibility", false);

// Set the home page.
user_pref("browser.startup.homepage", "file:///usr/lib/debathena-kiosk/index.html");
user_pref("startup.homepage_override_url", "");
user_pref("startup.homepage_welcome_url", "");
