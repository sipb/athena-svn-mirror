include "LoadDriver.think";
include "Kerberos.rsrc";

type 'sysz' {             			/* This is the type definition */
	longint;						/* size requested	(see IM V, page 352) */
};

resource 'sysz' ( 0 ,"", 0 ) {		/* and this is the declaration */
	0x00010000						/* bytes for sysz resource */
};

/* Set the resource bits on these... */

type 'DCOD' { hex string; };
type 'DATA' { hex string; };
type 'DRVR' { hex string; };

resource 'DRVR' (1, ".kerberos", sysheap, locked, nonpurgeable) {
	/*
	 * This directive inserts the contents of the DRVR resource
	 */
	$$resource("DriverShell.think", 'DRVR', 1)
};

resource 'DCOD' (-16350, "Krb Libs 1", sysheap, locked, nonpurgeable) {
	/*
	 * This directive inserts the contents of the code resource
	 */
	$$resource("DriverShell.think", 'DCOD', -16350)
};

resource 'DCOD' (-16349, "Krb Libs 2", sysheap, locked, nonpurgeable) {
	/*
	 * This directive inserts the contents of the code resource
	 */
	$$resource("DriverShell.think", 'DCOD', -16349)
};

resource 'DATA' (-16352, "Globals", sysheap, locked, nonpurgeable) {
	/*
	 * This directive inserts the contents of the data resource
	 */
	$$resource("DriverShell.think", 'DATA', -16352)
};


