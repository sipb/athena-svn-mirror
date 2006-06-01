// Home page set-up
pref("browser.startup.homepage", "chrome://athena/content/athena.properties");
pref("browser.startup.homepage_reset", "chrome://athena/content/athena.properties");
pref("startup.homepage_override_url", "chrome://athena/content/athena.properties");

// Don't prompt the user for setting the default browser.
pref("browser.shell.checkDefaultBrowser", false);

// Use evolution for mailto links.
pref("network.protocol-handler.app.mailto", "evolution");
pref("network.protocol-handler.external.mailto", true);

// 0-Accept, 1-dontAcceptForeign, 2-dontUse, 3-p3p
pref("network.cookie.cookieBehavior", 1);

// Duplex printing set-up
pref("print.printer_PostScript/duplex.print_command", "lpr -Zduplex ");
pref("print.printer_PostScript/2upduplex.print_command", "psnup -q -n 2 | lpr -Zduplex ");
pref("print.printer_list", "default duplex 2upduplex");

// Font set-up for MathML
pref("font.mathfont-family", "CMSY10, CMEX10, Math1, Math2, Math4, MT Extra, Standard Symbols L");

// Use the Athena GSSAPI library.
pref("network.negotiate-auth.gsslib", "/usr/athena/lib/libgssapi_krb5.so");
