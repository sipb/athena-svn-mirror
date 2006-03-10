#include <sysdep.h>
#include "include/libjwgc.h"

/* $Id: JDebug.c,v 1.1.1.1 2006-03-10 15:32:45 ghudson Exp $ */

int debug_enabled[dNumZones];
char *debug_zones[dNumZones];

void 
dinit()
{
#ifndef NODEBUG
	dZone i;

	for (i = 0; i < dNumZones; i++) {
		debug_enabled[i] = 0;
	}

	debug_zones[dJWG] = "jwg";
	debug_zones[dParser] = "parser";
	debug_zones[dJAB] = "jab";
	debug_zones[dOutput] = "output";
	debug_zones[dEval] = "eval";
	debug_zones[dPoll] = "poll";
	debug_zones[dExecution] = "execution";
	debug_zones[dVars] = "vars";
	debug_zones[dMatch] = "match";
	debug_zones[dXML] = "xml";
	debug_zones[dGPG] = "gpg";
#endif /* NODEBUG */
}

void 
dflagon(dZone zone)
{
#ifndef NODEBUG
	debug_enabled[zone] = 1;
#endif /* NODEBUG */
}

void 
dflagoff(dZone zone)
{
#ifndef NODEBUG
	debug_enabled[zone] = 0;
#endif /* NODEBUG */
}

void 
dprintf(dZone zone, const char *msgfmt, ...)
{
#ifndef NODEBUG
	va_list ap;

	if (!debug_enabled[zone]) {
		return;
	}

	va_start(ap, msgfmt);
	printf("[%s] ", dzoneitos(zone));
	vprintf(msgfmt, ap);
	va_end(ap);
#endif /* NODEBUG */
}

void
dprinttypes()
{
#ifndef NODEBUG
	dZone i;

        fprintf(stderr, "\
Debugging usage:\n\
  <flags> can contain any number of the following flags, separated by commas.\n\
  If you wish to use 'all' to enable all flags, and then disable a couple,\n\
  you can prepend the flag with a - (Ex: all,-poll,-eval).  The available\n\
  flags are as follows:\n\
\n\
");
	for (i = 0; i < dNumZones; i++) {
		printf("     %s\n", debug_zones[i]);
	}

	exit(1);
#endif /* NODEBUG */
}

char *
dzoneitos(dZone zone)
{
#ifndef NODEBUG
	return debug_zones[zone];
#endif /* NODEBUG */
}

dZone 
dzonestoi(char *zone)
{
#ifndef NODEBUG
	dZone i;

	for (i = 0; i < dNumZones; i++) {
		if (!strcmp(zone, debug_zones[i])) {
			return i;
		}
	}

	return -1;
#endif /* NODEBUG */
}

void
dparseflags(char *flags)
{
#ifndef NODEBUG
	char *ptr;
	int retval;
	dZone i;

	if (!flags) { return; }

	ptr = strtok(flags, ",");
	while (ptr != NULL) {
		if (!strcmp(ptr, "-all")) {
			for (i = 0; i < dNumZones; i++) {
				debug_enabled[i] = 0;
			}
		}
		else if (!strcmp(ptr, "all")) {
			for (i = 0; i < dNumZones; i++) {
				debug_enabled[i] = 1;
			}
		}
		else {
			if (*ptr == '-') {
				retval = dzonestoi(++ptr);
				if (retval != -1) {
					dflagoff(retval);
				}
			}
			else {
				retval = dzonestoi(ptr);
				if (retval != -1) {
					dflagon(retval);
				}
			}
		}
		
		ptr = strtok(NULL, ",");
	}
#endif /* NODEBUG */
}
