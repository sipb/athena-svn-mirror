// Home page set-up
pref("browser.startup.homepage", "chrome://athena/content/athena.properties");
pref("browser.startup.homepage_reset", "chrome://athena/content/athena.properties");
pref("startup.homepage_override_url", "");
pref("startup.homepage_welcome_url", "");

// Don't prompt the user for setting the default browser.
pref("browser.shell.checkDefaultBrowser", false);

// Use evolution for mailto links.
pref("network.protocol-handler.app.mailto", "evolution");
pref("network.protocol-handler.external.mailto", true);

// 0-Accept, 1-dontAcceptForeign, 2-dontUse, 3-p3p
pref("network.cookie.cookieBehavior", 1);

// Trust the MIT IdP for HTTP/SPNEGO.
pref("network.negotiate-auth.trusted-uris", "https://idp.mit.edu");

// Do not delete files downloaded for helper applications upon exit,
// to give users a chance to retrieve them if desired.
pref("browser.helperApps.deleteTempFileOnExit", false);
